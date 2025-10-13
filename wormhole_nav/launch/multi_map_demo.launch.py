from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    nav2_launch_dir = os.path.join(get_package_share_directory('nav2_bringup'), 'launch')
    wormhole_nav_dir = get_package_share_directory('wormhole_nav')

    return LaunchDescription([
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(nav2_launch_dir, 'bringup_launch.py')
            ),
            launch_arguments={
                'map': os.path.join(wormhole_nav_dir, 'maps', 'custom_map.yaml'),
                'params_file': os.path.join(wormhole_nav_dir, 'config', 'nav2_params.yaml'),
                'use_sim_time': 'false'
            }.items()
        ),
        Node(
            package='wormhole_nav',
            executable='multi_map_action_server',
            name='multi_map_action_server',
            output='screen',
            parameters=[os.path.join(wormhole_nav_dir, 'config', 'params.yaml')]
        )
    ])
