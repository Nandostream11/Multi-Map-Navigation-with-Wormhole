#pragma once
#include <string>
#include <rclcpp/rclcpp.hpp>

class MapManager {
    public:
        explicit MapManager(rclcpp::Node::SharedPtr node);
        //switch to the specified map, return true if successful
        bool switchToMap(const std::string &map_name, const std::string &map_file_path);
        std::string getCurrentMap() const;
    private:
        rclcpp::Node::SharedPtr node_;
        std::string current_map_;
};