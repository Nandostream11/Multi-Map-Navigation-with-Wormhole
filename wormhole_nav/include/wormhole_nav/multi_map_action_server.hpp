#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "wormhole_nav/db_interface.hpp"
#include "wormhole_nav/map_manager.hpp"
#include "wormhole_nav/nav_controller.hpp"
#include "wormhole_action_interfaces/action/multi_map_navigate.hpp"

class MultiMapActionServer : public rclcpp::Node {
public:
    using MultiMapNavigate = wormhole_action_interfaces::action::MultiMapNavigate;
    using GoalHandle = rclcpp_action::ServerGoalHandle<MultiMapNavigate>;

    MultiMapActionServer();

private:
    rclcpp_action::Server<MultiMapNavigate>::SharedPtr server_;
    std::shared_ptr<DBInterface> db_;
    std::shared_ptr<MapManager> map_manager_;
    std::shared_ptr<NavController> nav_;

    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID &uuid,
        std::shared_ptr<const MultiMapNavigate::Goal> goal);
        
    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandle> goal_handle);

    void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle);

    void execute(const std::shared_ptr<GoalHandle> goal_handle);
};