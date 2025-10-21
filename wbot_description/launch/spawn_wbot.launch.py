import os
from pathlib import Path
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable, ExecuteProcess, RegisterEventHandler
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.event_handlers import OnProcessStart
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    # --- Package paths ---
    wbot_description = get_package_share_directory("wbot_description")
    urdf_file = os.path.join(wbot_description, "urdf", "wbot.urdf.xacro")
    controllers_yaml = os.path.join(wbot_description, "config", "ros2_controllers.yaml")

    # Optional: Gazebo world
    world_file = os.path.join(
        get_package_share_directory("wormhole_nav"), "worlds", "flat.world"
    )

    # --- Declare URDF/Xacro argument ---
    model_arg = DeclareLaunchArgument(
        "model",
        default_value=urdf_file,
        description="Absolute path to robot URDF/Xacro file",
    )

    # --- Set GAZEBO_MODEL_PATH to include meshes + share ---
    gazebo_model_path = SetEnvironmentVariable(
        name="GAZEBO_MODEL_PATH",
        value=str(Path(wbot_description, "meshes").resolve()) + ":" +
              str(Path(wbot_description, "share").resolve()) + ":" +
              os.environ.get("GAZEBO_MODEL_PATH", ""),
    )

    # --- Robot description from xacro ---
    robot_description = ParameterValue(
        Command(["xacro ", LaunchConfiguration("model"), " is_classic:=True"]),
        value_type=str,
    )

    # --- Robot State Publisher ---
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[{"robot_description": robot_description, "use_sim_time": True}],
    )

    # --- Gazebo server and client ---
    gazebo_server = ExecuteProcess(
        cmd=["gzserver", "--verbose", world_file, "-s", "libgazebo_ros_factory.so"],
        output="screen",
    )

    gazebo_client = ExecuteProcess(
        cmd=["gzclient"],
        output="screen",
    )

    # --- Spawn WBot in Gazebo ---
    spawn_entity = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        arguments=["-topic", "robot_description", "-entity", "wbot"],
        output="screen",
    )

    # --- Spawn diff_drive_controller after robot is spawned ---
    spawn_controller = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diff_drive_controller", "--controller-manager", "/controller_manager", "--param-file", controllers_yaml],
        output="screen",
    )

    # --- Event handlers to enforce sequence ---
    start_gazebo_after_rsp = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=robot_state_publisher_node,
            on_start=[gazebo_server],
        )
    )

    spawn_after_gazebo = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=gazebo_server,
            on_start=[spawn_entity],
        )
    )

    start_controller_after_spawn = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=spawn_entity,
            on_start=[spawn_controller],
        )
    )

    start_gzclient_after_spawn = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=spawn_entity,
            on_start=[gazebo_client],
        )
    )

    return LaunchDescription([
        model_arg,
        gazebo_model_path,
        robot_state_publisher_node,
        start_gazebo_after_rsp,
        spawn_after_gazebo,
        start_controller_after_spawn,
        start_gzclient_after_spawn,
    ])
