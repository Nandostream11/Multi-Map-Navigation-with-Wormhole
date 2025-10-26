#pragma once
#include <string>
#include <rclcpp/rclcpp.hpp>
#include <nav2_msgs/srv/load_map.hpp>

class MapManager {
    public:
        explicit MapManager(std::shared_ptr<rclcpp::Node> node);
        //switch to the specified map, return true if successful
        bool switchToMap(const std::string &map_name, const std::string &map_file_path);
        std::string getCurrentMap() const;
    private:
        std::shared_ptr<rclcpp::Node> node_;
        std::string current_map_;
};