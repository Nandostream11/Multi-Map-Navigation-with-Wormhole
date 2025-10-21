import os
from pathlib import Path
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    SetEnvironmentVariable,
    ExecuteProcess,
    RegisterEventHandler,
)
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.event_handlers import OnProcessStart


def generate_launch_description():
    # --- Package paths ---
    wbot_description = get_package_share_directory("wbot_description")
    urdf_file = os.path.join(wbot_description, "urdf", "wbot.urdf.xacro")

    # --- Declare URDF/Xacro argument ---
    model_arg = DeclareLaunchArgument(
        "model",
        default_value=urdf_file,
        description="Absolute path to robot URDF/Xacro file",
    )

    # --- Minimal GAZEBO_MODEL_PATH (only URDF folder) ---
    gazebo_model_path = SetEnvironmentVariable(
        name="GAZEBO_MODEL_PATH",
        value=str(Path(wbot_description, "meshes").resolve())        
        + ":"
        + str(Path(wbot_description).resolve())     
        + ":"   
        + os.environ.get("GAZEBO_MODEL_PATH", ""),
    )

    # --- Robot description (from xacro) ---
    robot_description = ParameterValue(
        Command(["xacro ", LaunchConfiguration("model"), " is_classic:=True"]),
        value_type=str,
    )

    # --- Robot State Publisher (for TF) ---
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[{"robot_description": robot_description, "use_sim_time": True}],
    )

    # --- Start Gazebo Classic server (physics engine only) ---
    gazebo_server = ExecuteProcess(
        cmd=["gzserver", "--verbose", "-s", "libgazebo_ros_factory.so"],
        output="screen",
    )

    # --- Spawn robot into Gazebo ---
    spawn_entity = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        arguments=["-topic", "robot_description", "-entity", "wbot"],
        output="screen",
    )

    # --- gzclient (GUI) ---
    gzclient = ExecuteProcess(cmd=["gzclient"], output="screen")

    # --- Event handlers to enforce sequence:
    # 1) start gazebo_server once robot_state_publisher is up
    # 2) spawn robot once gazebo_server is up
    # 3) start gzclient once spawn_entity starts
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

    start_gzclient_after_spawn = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=spawn_entity,
            on_start=[gzclient],
        )
    )

    return LaunchDescription(
        [
            model_arg,
            gazebo_model_path,
            # start robot_state_publisher first
            robot_state_publisher_node,
            # event handlers drive the rest in order
            start_gazebo_after_rsp,
            spawn_after_gazebo,
            start_gzclient_after_spawn,
        ]
    )