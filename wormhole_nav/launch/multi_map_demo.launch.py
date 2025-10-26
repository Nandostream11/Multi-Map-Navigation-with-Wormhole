from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory
import os
from pathlib import Path

def generate_launch_description():
    pkg_bumperbot = get_package_share_directory('bumperbot_description')
    pkg_wormhole = get_package_share_directory('wormhole_nav')
    pkg_ros_gz_sim = get_package_share_directory('ros_gz_sim')

    world = os.path.join(pkg_wormhole, 'worlds', 'multi_room.sdf')
    nav2_params = os.path.join(pkg_wormhole, 'config', 'nav2_params.yaml')
    wormhole_params = os.path.join(pkg_wormhole, 'config', 'params.yaml')
    
    model_arg = DeclareLaunchArgument(
        name="model", 
        default_value=os.path.join(pkg_bumperbot, "urdf", "bumperbot.urdf.xacro"),
        description="Absolute path to robot urdf file"
    )

    gazebo_resource_path = SetEnvironmentVariable(
        name="GZ_SIM_RESOURCE_PATH",
        value=[str(Path(pkg_bumperbot).parent.resolve())]
    )

    ros_distro = os.environ["ROS_DISTRO"]
    is_ignition = "True" if ros_distro == "humble" else "False"
    
    robot_description = ParameterValue(Command([
        "xacro ",
        LaunchConfiguration("model"),
        " is_ignition:=",
        is_ignition
    ]), value_type=str)

    return LaunchDescription([
        model_arg,
        gazebo_resource_path,

        # 1. Start Gazebo with your world
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([
                os.path.join(pkg_ros_gz_sim, "launch", "gz_sim.launch.py")
            ]),
            launch_arguments=[
                ("gz_args", [f" -v 4 -r {world}"])
            ]
        ),

        # 2. Robot description and state
        Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            parameters=[{
                "robot_description": robot_description,
                "use_sim_time": True
            }]
        ),

        Node(
            package='joint_state_publisher',
            executable='joint_state_publisher',
            name='joint_state_publisher',
            parameters=[{'use_sim_time': True}]
        ),

        # 3. Spawn robot
        Node(
            package="ros_gz_sim",
            executable="create",
            output="screen",
            arguments=["-topic", "robot_description", "-name", "bumperbot"],
        ),

        # 4. CRITICAL: Odometry bridge from Gazebo
        Node(
            package="ros_gz_bridge",
            executable="parameter_bridge",
            arguments=[
                "/model/bumperbot/odometry@nav_msgs/msg/Odometry[ignition.msgs.Odometry",
            ],
            remappings=[
                ("/model/bumperbot/odometry", "/odom")
            ],
            output="screen"
        ),

        # 5. IMU bridge
        Node(
            package="ros_gz_bridge",
            executable="parameter_bridge",
            arguments=["/imu@sensor_msgs/msg/Imu[gz.msgs.IMU"],
            remappings=[("/imu","/imu/out")]
        ),

        # 6. Optional: Static transform if Gazebo doesn't publish odom frame
        # This creates the odom frame, but the transform will be updated by the odometry bridge
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='odom_frame_publisher',
            arguments=['0', '0', '0', '0', '0', '0', 'odom', 'base_footprint'],
            parameters=[{'use_sim_time': True}]
        ),

        # 7. Complete Nav2 Stack (with delayed start to ensure TF is ready)
        Node(
            package='nav2_controller',
            executable='controller_server',
            name='controller_server',
            output='screen',
            parameters=[nav2_params],
        ),

        Node(
            package='nav2_planner',
            executable='planner_server',
            name='planner_server',
            output='screen',
            parameters=[nav2_params],
        ),

        Node(
            package='nav2_behaviors',
            executable='behavior_server',
            name='behavior_server',
            output='screen',
            parameters=[nav2_params],
        ),

        Node(
            package='nav2_bt_navigator',
            executable='bt_navigator',
            name='bt_navigator',
            output='screen',
            parameters=[nav2_params],
        ),

        Node(
            package='nav2_waypoint_follower',
            executable='waypoint_follower',
            name='waypoint_follower',
            output='screen',
            parameters=[nav2_params],
        ),

        Node(
            package='nav2_amcl',
            executable='amcl',
            name='amcl',
            output='screen',
            parameters=[nav2_params],
        ),

        Node(
            package='nav2_lifecycle_manager',
            executable='lifecycle_manager',
            name='lifecycle_manager_navigation',
            output='screen',
            parameters=[{
                'use_sim_time': True,
                'autostart': True,
                'node_names': [
                    'controller_server',
                    'planner_server',
                    'behavior_server',
                    'bt_navigator',
                    'waypoint_follower',
                    'amcl'
                ]
            }]
        ),

        # 8. Your action server
        Node(
            package='wormhole_nav',
            executable='multi_map_action_server',
            name='multi_map_action_server',
            output='screen',
            parameters=[wormhole_params]
        ),
    ])