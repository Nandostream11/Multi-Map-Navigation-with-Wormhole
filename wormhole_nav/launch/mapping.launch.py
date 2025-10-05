from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():

    # Paths
    pkg_tb3_desc = get_package_share_directory('turtlebot3_description')
    pkg_slam = get_package_share_directory('slam_toolbox')
    pkg_gz = get_package_share_directory('ros_gz_sim')

    world_file = os.path.join(
        get_package_share_directory('wormhole_nav'),
        'worlds',
        'multi_room.sdf'
    )

    # Arguments
    use_sim_time = LaunchConfiguration('use_sim_time', default='true')

    # Determine URDF dynamically from environment variable
    turtlebot3_model = os.environ.get('TURTLEBOT3_MODEL', 'waffle')
    urdf_file = os.path.join(pkg_tb3_desc, 'urdf', f'turtlebot3_{turtlebot3_model}.urdf')

    # Timer-wrapped spawn node to ensure Gazebo is ready
    spawn_tb3 = TimerAction(
        period=2.0,  # wait 2 seconds
        actions=[
            Node(
                package='ros_gz_sim',
                executable='create',
                arguments=[
                    '-name', 'turtlebot3',
                    '-x', '0', '-y', '0', '-z', '0.0',
                    '-file', urdf_file
                ],
                output='screen'
            )
        ]
    )

    return LaunchDescription([

        # Load Gazebo (Ignition) with world
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(pkg_gz, 'launch', 'gz_sim.launch.py')
            ),
            launch_arguments={'gz_args': world_file}.items(),
        ),

        # Spawn TurtleBot3 Waffle into Gazebo
        spawn_tb3,

        # Bridge TF, cmd_vel, laser scan between ROS2 <-> Gazebo
        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            arguments=[
                '/cmd_vel@geometry_msgs/msg/Twist@gz.msgs.Twist',
                '/odom@nav_msgs/msg/Odometry@gz.msgs.Odometry',
                '/scan@sensor_msgs/msg/LaserScan@gz.msgs.LaserScan',
                '/tf@tf2_msgs/msg/TFMessage@gz.msgs.Pose_V',
                '/tf_static@tf2_msgs/msg/TFMessage@gz.msgs.Pose_V'
            ],
            output='screen'
        ),

        # Start slam_toolbox in online mode
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(pkg_slam, 'launch', 'online_async_launch.py')
            ),
            launch_arguments={'use_sim_time': use_sim_time}.items(),
        ),
    ])
