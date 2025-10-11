#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "wormhole_nav/db_interface.hpp"
#include "wormhole_nav/map_manager.hpp"
#include "wormhole_nav/nav_controller.hpp"
#include "wormhole_nav/action/multimapnavigate.hpp"

class MultiMapActionServer : public rclcpp::Node {
public:
    using MultiMapNavigate = wormhole_nav::action::MultiMapNavigate;
    using GoalHandle = rclcpp_action::ServerGoalHandle<MultiMapNavigate>;

    MultiMapActionServer();

private:
    rclcpp_action::Server<MultiMapNavigate>::SharedPtr server_;
    std::shared_ptr<DBInterface> db_;
    std::shared_ptr<MapManager> map_manager_;
    std::shared_ptr<NavController> nav_;

    void handleGoal(const rclcpp_action::GoalUUID &, std::shared_ptr<const MultiMapNavigate::Goal>);
    void execute(const std::shared_ptr<GoalHandle> goal_handle);
};