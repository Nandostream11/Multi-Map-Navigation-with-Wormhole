import os
from pathlib import Path
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable, ExecuteProcess
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    # --- Package paths ---
    wbot_description = get_package_share_directory("wbot_description")
    urdf_file = os.path.join(wbot_description, "urdf", "wbot.urdf.xacro")

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

    # --- Set GAZEBO_MODEL_PATH to a folder with actual Gazebo models (optional) ---
    gazebo_model_path = SetEnvironmentVariable(
        name="GAZEBO_MODEL_PATH",
        value=str(Path(wbot_description, "urdf").resolve()),  # Only URDF folder
    )

    # --- Robot description (from xacro) ---
    robot_description = ParameterValue(
        Command(["xacro ", LaunchConfiguration("model")]), value_type=str
    )

    # --- Robot State Publisher ---
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[{"robot_description": robot_description, "use_sim_time": True}],
    )

    # --- Start Gazebo Classic ---
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

    return LaunchDescription([
        model_arg,
        gazebo_model_path,
        robot_state_publisher_node,
        gazebo_server,
        gazebo_client,
        spawn_entity,
    ])
