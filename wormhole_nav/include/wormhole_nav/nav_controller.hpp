#pragma once
#include <rclcpp/rclcpp.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

class NavController{
public:
    using NavigateToPose = nav2_msgs::action::NavigateToPose;
    using GoalHandleNavToPose = rclcpp_action::ClientGoalHandle<NavigateToPose>;

    explicit NavController(std::shared_ptr<rclcpp::Node> node);
    bool sendGoal(const geometry_msgs::msg::PoseStamped &goal);
    void cancelGoal();

private:
    std::shared_ptr<rclcpp::Node> node_;
    rclcpp_action::Client<NavigateToPose>::SharedPtr client_;
};