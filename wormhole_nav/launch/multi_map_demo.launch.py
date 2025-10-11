from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='nav2_bringup',
            executable='bringup_launch.py',
            name='nav2_bringup',
            output='screen',
            parameters=[{'use_sim_time': False}]
        ),
        Node(
            package='wormhole_nav',
            executable='multi_map_action_server',
            name='multi_map_action_server',
            output='screen',
            parameters=['config/params.yaml']
        )
    ])