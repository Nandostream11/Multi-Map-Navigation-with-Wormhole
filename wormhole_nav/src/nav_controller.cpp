#include "wormhole_nav/nav_controller.hpp"

NavController::NavController(std::shared_ptr<rclcpp::Node> node) : node_(node){
    client_=rclcpp_action::create_client<NavigateToPose>(node_, "navigate_to_pose");
}

bool NavController::sendGoal(const geometry_msgs::msg::PoseStamped &goal){
    if(!client_->wait_for_action_server(std::chrono::seconds(5))){
        RCLCPP_ERROR(node_->get_logger(), "Nav2 Action server not available after waiting 5 sec");
        return false;
    }

    NavigateToPose::Goal nav_goal;
    nav_goal.pose=goal;

    auto future_goal_handle=client_->async_send_goal(nav_goal);
    if(rclcpp::spin_until_future_complete(node_, future_goal_handle)!=rclcpp::FutureReturnCode::SUCCESS){
        RCLCPP_ERROR(node_->get_logger(), "Failed to send goal");
        return false;
    }

    auto handle=future_goal_handle.get();
    if(!handle){
        RCLCPP_ERROR(node_->get_logger(), "Goal was rejected by server");
        return false;   
    }

    RCLCPP_INFO(node_->get_logger(), "Server: Goal accepted, waiting for result...");
    auto future_res=client_->async_get_result(handle);
    rclcpp::spin_until_future_complete(node_, future_res);
    auto res=future_res.get();
    RCLCPP_INFO(node_->get_logger(), "Nav finished with code: %d", static_cast<int>(res.code));
    return res.code==rclcpp_action::ResultCode::SUCCEEDED;
}

void NavController::cancelGoal(){
    client_->async_cancel_all_goals();
    RCLCPP_INFO(node_->get_logger(), "Cancelled all goals");
}