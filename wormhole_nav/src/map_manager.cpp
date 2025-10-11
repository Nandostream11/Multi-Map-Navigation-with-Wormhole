#include "wormhole_nav/map_manager.hpp"
#include <nav_msgs/srv/load_map.hpp>

MapManager::MapManager(rclcpp::Node::SharedPtr node): node_(node){}

bool MapManager::switchToMap(const std::string &map_name, const std::string &map_file_path){
    auto client = node_->create_client<nav_msgs::srv::LoadMap>("map_server/load_map");
    if(!client->wait_for_service(std::chrono::seconds(5))){
        RCLCPP_ERROR(node_->get_logger(), "Service map_server/load_map not available");
        return false;
    }
    
    auto req= std::make_shared<nav_msgs::srv::LoadMap::Request>();
    req->map_url=map_file_path;

    auto future= client->async_send_request(req);
    if(rclcpp::spin_until_future_complete(node_, future)!= rclcpp::FutureReturnCode::SUCCESS){
        RCLCPP_ERROR(node_->get_logger(), "Failed to call map_server/load_map");
        return false;
    }
    
    auto response= future.get();
    if(!response->map.info.width || !response->map.info.height){
        RCLCPP_ERROR(node_->get_logger(), "Loaded map is invalid");
        return false;
    }

    current_map_=map_name;
    RCLCPP_INFO(node_->get_logger(), "Switched to map: %s", map_name.c_str());
    return true;
}

std::string MapManager::getCurrentMap() const { return current_map_;}