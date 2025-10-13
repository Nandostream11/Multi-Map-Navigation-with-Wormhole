#include "wormhole_nav/multi_map_action_server.hpp"
#include <thread>
#include <cmath>

MultiMapActionServer::MultiMapActionServer() : Node("multi_map_action_server"){

    std::string db_path=this->declare_parameter<std::string>("db_path", "wormholes.db");
    db_->open();

    map_manager_=std::make_shared<MapManager>(shared_from_this());
    nav_=std::make_shared<NavController>(shared_from_this());

    server_=rclcpp_action::create_server<MultiMapNavigate>(
        this,
        "multi_map_navigate",
        [this](const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const MultiMapNavigate::Goal> goal){
            RCLCPP_INFO(this->get_logger(), "Received goal for map %s", goal->target_map.c_str());
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        },
        [](const std::shared_ptr<GoalHandle>){ return rclcpp_action::CancelResponse::ACCEPT;
        },
        [this](const std::shared_ptr<GoalHandle> goal_handle){
            using namespace std::placeholders;
            std::thread{std::bind(&MultiMapActionServer::execute, this, _1), goal_handle}.detach();
        }
    );

    /* lambda used to bind the handleGoal method, etc
    {
        this->handleGoal(_1, _2);
    },
    */
    RCLCPP_INFO(this->get_logger(), "MultiMapActionServer is ready.");
}

void MultiMapActionServer::execute(const std::shared_ptr<GoalHandle> goal_handle){
    auto goal= goal_handle->get_goal();
    auto feedback = std::make_shared<MultiMapNavigate::Feedback>();
    auto result = std::make_shared<MultiMapNavigate::Result>();

    std::string current_map= map_manager_->getCurrentMap();
    if(current_map.empty())  current_map="default_map(roomA)"; //default starting map

    if(goal->target_map==current_map){
        RCLCPP_INFO(this->get_logger(), "Executing goal...");
        bool ok=nav_->sendGoal(goal->target_pose);
        result->success=ok;
        result->message = ok ? "Navigated to target on same map" : "Navigation failed";
        goal_handle->succeed(result);
        return;
    }

    auto list= db_->getWormholes(current_map, goal->target_map);
    if(list.empty()){
        result->success=false;
        result->message = "No wormhole found from " + current_map + " to " + goal->target_map;
        goal_handle->abort(result);
        return;
    }

    Wormhole wh=list.front();
    geometry_msgs::msg::PoseStamped wh_pose;
    wh_pose.header.frame_id="map";
    wh_pose.pose.position.x=wh.x;
    wh_pose.pose.position.y=wh.y;
    wh_pose.pose.orientation.w=1.0;

    RCLCPP_INFO(this->get_logger(), "Navigating to wormhole in map: %s", current_map.c_str());
    feedback->current_state = "Navigating to wormhole...";
    goal_handle->publish_feedback(feedback);

    nav_->cancelGoal(); //cancel any existing goal

    bool to_wormhole_ok = nav_->sendGoal(wh_pose);
    if (!to_wormhole_ok) {
        result->success = false;
        result->message = "Failed to reach wormhole in " + current_map;
        goal_handle->abort(result);
        return;
    }

    feedback->current_state = "Reached wormhole, switching map...";
    goal_handle->publish_feedback(feedback);

    std::string map_path=this->get_parameter("maps."+ goal->target_map).as_string();
    if(!map_manager_->switchToMap(goal->target_map,map_path)){
        result->success=false;
        result->message="Failed to switch to map "+goal->target_map;
        goal_handle->abort(result);
        return;
    }

    RCLCPP_INFO(this->get_logger(), "Switch success, moving to final target...");
    bool ok=nav_->sendGoal(goal->target_pose);
    result->success=ok;
    result->message=ok ? "Navigation succeeded" : "Navigation failed";
    goal_handle->succeed(result);

}

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<MultiMapActionServer>();
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();
  
  rclcpp::shutdown();
  return 0;
}