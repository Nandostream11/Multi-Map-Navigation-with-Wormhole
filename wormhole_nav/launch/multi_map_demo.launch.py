from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, ExecuteProcess
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # --- Paths ---
    pkg_nav2 = get_package_share_directory('nav2_bringup')
    pkg_bumperbot = get_package_share_directory('bumperbot_description')
    pkg_wormhole = get_package_share_directory('wormhole_nav')

    world = os.path.join(pkg_wormhole, 'worlds', 'flat.world')
    map_file = os.path.join(pkg_wormhole, 'maps', 'custom_map.yaml')
    nav2_params = os.path.join(pkg_wormhole, 'config', 'nav2_params.yaml')
    wormhole_params = os.path.join(pkg_wormhole, 'config', 'params.yaml')

    urdf_file = os.path.join(pkg_bumperbot, 'urdf', 'bumperbot.urdf.xacro')

    # --- Launch description ---
    return LaunchDescription([
        # 1. Launch Gazebo Classic
        ExecuteProcess(
            cmd=['gazebo', '--verbose', world,
                 '-s', 'libgazebo_ros_factory.so'],
            output='screen'
        ),

        # 2. Robot state publisher
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{'use_sim_time': True}],  # Changed to True for Gazebo
            arguments=[urdf_file]
        ),

        # 3. Spawn the robot into Gazebo
        Node(
            package='gazebo_ros',
            executable='spawn_entity.py',
            arguments=['-topic', 'robot_description', '-entity', 'bumperbot'],
            output='screen'
        ),

        # 4. Nav2 bringup
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(pkg_nav2, 'launch', 'bringup_launch.py')
            ),
            launch_arguments={
                'map': map_file,
                'params_file': nav2_params,
                'use_sim_time': 'true'  # Changed to true for Gazebo
            }.items()
        ),

        # 5. Wormhole navigation action server
        Node(
            package='wormhole_nav',
            executable='multi_map_action_server',
            name='multi_map_action_server',
            output='screen',
            parameters=[wormhole_params],
            remappings=[
                ('/map_server/load_map', '/map_server/load_map')  # Ensure correct service name
            ]
        ),

        # 6. RViz2 visualization
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen',
            parameters=[{'use_sim_time': True}]  # Added use_sim_time
        ),
    ])