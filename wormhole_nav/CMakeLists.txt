cmake_minimum_required(VERSION 3.8)
project(wormhole_nav)

# Set C++ standard
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

find_package(wormhole_action_interfaces REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

# Direct SQLite3 detection
find_library(SQLite3_LIBRARY
  NAMES sqlite3
  PATHS /usr/lib /usr/lib/x86_64-linux-gnu
)

if(SQLite3_LIBRARY)
  message(STATUS "Found SQLite3: ${SQLite3_LIBRARY}")
  set(SQLite3_LIBRARIES ${SQLite3_LIBRARY})
else()
  message(FATAL_ERROR "SQLite3 library not found. Install with: sudo apt-get install libsqlite3-dev")
endif()

include_directories(include)

# Add executable
add_executable(multi_map_action_server
  src/db_interface.cpp 
  src/map_manager.cpp 
  src/nav_controller.cpp 
  src/multi_map_action_server.cpp
)

# Link dependencies - FIXED SQLite3 linking
target_link_libraries(multi_map_action_server
  ${rclcpp_LIBRARIES}
  ${rclcpp_action_LIBRARIES}
  ${geometry_msgs_LIBRARIES}
  ${nav2_msgs_LIBRARIES}
  ${nav_msgs_LIBRARIES}
  ${tf2_LIBRARIES}
  tf2::tf2
  ${SQLite3_LIBRARIES}  # This should now work
)

# Target dependencies
ament_target_dependencies(multi_map_action_server 
  rclcpp
  rclcpp_action
  nav2_msgs
  nav_msgs
  geometry_msgs
  tf2
  tf2_geometry_msgs
  wormhole_action_interfaces
)

# Install headers
install(DIRECTORY include/${PROJECT_NAME}/  
  DESTINATION include/${PROJECT_NAME}/
)

# Install configuration files
install(DIRECTORY config 
  DESTINATION share/${PROJECT_NAME}/
)

# Install maps directory
install(DIRECTORY maps
  DESTINATION share/${PROJECT_NAME}/
)

# Install executable
install(TARGETS 
  multi_map_action_server
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)

# Install worlds
install(
  DIRECTORY worlds
  DESTINATION share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()