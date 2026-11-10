cmake_minimum_required(VERSION 3.8)
project(arm_system_tests)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(arm_interfaces REQUIRED)
find_package(arm_moveit_config REQUIRED)

add_executable(pickup_test src/pickup_test.cpp)
ament_target_dependencies(pickup_test rclcpp moveit_ros_planning_interface arm_interfaces)
add_executable(pickup_n_sort src/pickup_n_sort.cpp)
ament_target_dependencies(pickup_n_sort rclcpp moveit_ros_planning_interface arm_interfaces arm_moveit_config)

install(TARGETS
  pickup_test
  pickup_n_sort
  DESTINATION lib/${PROJECT_NAME}/
)

ament_package()
