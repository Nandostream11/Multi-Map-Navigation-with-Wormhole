#include "wormhole_nav/multi_map_action_server.hpp"
#include <thread>
#include <cmath>

MultiMapActionServer::MultiMapActionServer() : Node("multi map action server"){

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
            std::thread{&MultiMapNavigate::execute, this, goal_handle}.detach();
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
    MultiMapNavigate::Feedback feedback;
    MultiMapNavigate::Result result;

    std::string curr_map= map_manager_->getCurrentMap();
    if(current_map.empty())  curr_map="default_map(roomA)"; //default starting map

    if(goal->target_map==curr_map){
        RCLCPP_INFO(this->get_logger(), "Executing goal...");
        bool ok=nav_->sendGoal(goal_pose);
        result.success=ok;
        goal_handle->succeed(result);
        return;
    }

    auto list= db_->getWormholes(curr_map, goal->target_map);
    if(list.empty()){
        resullt.success=false;
        result.message="No wormhole found from "+curr_map+" to "+goal->target_map;
        goal_handle->abort(result);
        return;
    }

    Wormhole wh=list.front();
    geometry_msgs::msg::PoseStamped wh_pose;
    wh_pose.header.frame_id="map";
    wh_pose.pose.position.x=wh.x;
    wh_pose.pose.position.y=wh.y;
    wh_pose.pose.orientation.w=1.0;

    RCLCPP_INFO(this->get_logger(), "Navigating to wormhole in map: %s", curr_map.c_str());
    nav_->cancelGoal(); //cancel any existing goal

    std::string map_path=this->get_parameter("maps."+ goal->target_map).as_string();
    if(!map_manager_->switchToMap(goal->target_map,map_path)){
        result.success=false;
        result.message="Failed to switch to map "+goal->target_map;
        goal_handle->abort(result);
        return;
    }

    RCLCPP_INFO(this->get_logger(), "Switch success, moving to final target...");
    bool ok=nav_->sendGoal(goal->target_pose);
    result.success=ok;
    result.message=ok ? "Navigation succeeded" : "Navigation failed";
    goal_handle->succeed(result);

}    