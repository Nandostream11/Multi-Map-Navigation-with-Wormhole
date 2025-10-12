cmake_minimum_required(VERSION 3.8)
project(wormhole_nav)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(wormhole_action_interfaces REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

include_directories(include)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

add_executable( multi_map_action_server
  src/db_interface.cpp 
  src/map_manager.cpp 
  src/nav_controller.cpp 
  src/multi_map_action_server.cpp
)
ament_target_dependencies( multi_map_action_server 
  rclcpp 
  rclcpp_action 
  nav_msgs 
  nav2_msgs 
  geometry_msgs
  wormhole_action_interfaces
)

# Link SQLite manually
target_link_libraries(multi_map_action_server sqlite3)

install(DIRECTORY include/${PROJECT_NAME}/  DESTINATION include/${PROJECT_NAME}/)
install(DIRECTORY config DESTINATION share/${PROJECT_NAME}/)


install(TARGETS 
  multi_map_action_server
  DESTINATION lib/${PROJECT_NAME}
)
# Install launch files
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)
install(
  DIRECTORY worlds
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
