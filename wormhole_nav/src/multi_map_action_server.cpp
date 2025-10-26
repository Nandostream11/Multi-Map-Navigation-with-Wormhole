#include "wormhole_nav/multi_map_action_server.hpp"
#include <thread>
#include <cmath>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

MultiMapActionServer::MultiMapActionServer() : Node("multi_map_action_server")
{
    std::string db_path = this->declare_parameter<std::string>("db_path", "wormholes.db");
    db_ = std::make_shared<DBInterface>(db_path);
    if (!db_->open()){
        RCLCPP_ERROR(this->get_logger(), "Failed to open DB at %s", db_path.c_str());
        throw std::runtime_error("Database initialization failed");
    }

    // Use a timer to delay initialization until after construction is complete
    // This prevents the std::bad_weak_ptr error
    init_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(100),
        [this]() {
            this->init_timer_->cancel();  // Run only once
            
            // Now it's safe to use shared_from_this() since construction is complete
            auto node_ptr = this->shared_from_this();
            map_manager_ = std::make_shared<MapManager>(node_ptr);
            nav_ = std::make_shared<NavController>(node_ptr);

            server_ = rclcpp_action::create_server<MultiMapNavigate>(
                this,
                "multi_map_navigate",
                std::bind(&MultiMapActionServer::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
                std::bind(&MultiMapActionServer::handle_cancel, this, std::placeholders::_1),
                std::bind(&MultiMapActionServer::handle_accepted, this, std::placeholders::_1)
            );

            RCLCPP_INFO(this->get_logger(), "MultiMapActionServer is ready.");
        }
    );
}

rclcpp_action::GoalResponse MultiMapActionServer::handle_goal(
    const rclcpp_action::GoalUUID &uuid,
    std::shared_ptr<const MultiMapNavigate::Goal> goal) 
{
    RCLCPP_INFO(this->get_logger(), "Received goal for map: %s", goal->target_map.c_str());
    (void)uuid;  // Unused parameter
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse MultiMapActionServer::handle_cancel(
    const std::shared_ptr<GoalHandle> goal_handle) 
{
    RCLCPP_INFO(this->get_logger(), "Received cancel request");
    (void)goal_handle;  // Unused parameter
    return rclcpp_action::CancelResponse::ACCEPT;
}

void MultiMapActionServer::handle_accepted(const std::shared_ptr<GoalHandle> goal_handle) 
{
    std::thread{std::bind(&MultiMapActionServer::execute, this, std::placeholders::_1), goal_handle}.detach();
}

void MultiMapActionServer::execute(const std::shared_ptr<GoalHandle> goal_handle)
{
    auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<MultiMapNavigate::Feedback>();
    auto result = std::make_shared<MultiMapNavigate::Result>();

    std::string current_map = map_manager_->getCurrentMap();
    if(current_map.empty()) {
        current_map = "room1"; // default starting map
    }

    if(goal->target_map == current_map){
        RCLCPP_INFO(this->get_logger(), "Target is on current map, navigating directly");
        feedback->current_state = "Navigating on same map";
        goal_handle->publish_feedback(feedback);
        
        bool ok = nav_->sendGoal(goal->target_pose);
        result->success = ok;
        result->message = ok ? "Navigated to target on same map" : "Navigation failed";
        
        if (ok) {
            goal_handle->succeed(result);
        } else {
            goal_handle->abort(result);
        }
        return;
    }
    
    auto list = db_->getWormholes(current_map, goal->target_map);
    if (list.empty()) {
        result->success = false;
        result->message = "No wormhole found from " + current_map + " to " + goal->target_map;
        goal_handle->abort(result);
        return;
    }

    Wormhole wh = list.front();  // Use the first wormhole found

    geometry_msgs::msg::PoseStamped wh_pose;
    wh_pose.header.frame_id = "map";
    wh_pose.header.stamp = this->now();
    wh_pose.pose.position.x = wh.x;
    wh_pose.pose.position.y = wh.y;

    // Convert yaw to quaternion
    tf2::Quaternion quat;
    quat.setRPY(0, 0, wh.yaw);
    wh_pose.pose.orientation = tf2::toMsg(quat);

    RCLCPP_INFO(this->get_logger(), "Navigating to wormhole in map: %s", current_map.c_str());
    feedback->current_state = "Navigating to wormhole...";
    goal_handle->publish_feedback(feedback);

    nav_->cancelGoal(); // cancel any existing goal

    bool to_wormhole_ok = nav_->sendGoal(wh_pose);
    if (!to_wormhole_ok) {
        result->success = false;
        result->message = "Failed to reach wormhole in " + current_map;
        goal_handle->abort(result);
        return;
    }

    // Switch map
    feedback->current_state = "Reached wormhole, switching map...";
    goal_handle->publish_feedback(feedback);
    
    // Get map path from parameters
    std::string map_path = this->get_parameter("maps." + goal->target_map).as_string();
    if (map_path.empty()) {
        result->success = false;
        result->message = "No map path defined for: " + goal->target_map;
        goal_handle->abort(result);
        return;
    }

    if (!map_manager_->switchToMap(goal->target_map, map_path)) {
        result->success = false;
        result->message = "Failed to switch to map " + goal->target_map;
        goal_handle->abort(result);
        return;
    }

    // Navigate to final target in new map
    RCLCPP_INFO(this->get_logger(), "Switch success, moving to final target...");
    feedback->current_state = "Navigating to final target...";
    goal_handle->publish_feedback(feedback);
    
    bool final_nav_ok = nav_->sendGoal(goal->target_pose);
    result->success = final_nav_ok;
    result->message = final_nav_ok ? "Navigation succeeded" : "Navigation failed";
    
    if (final_nav_ok) {
        goal_handle->succeed(result);
    } else {
        goal_handle->abort(result);
    }
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