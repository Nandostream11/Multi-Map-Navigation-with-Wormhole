# Multi-Map-Navigation-with-Wormhole
Multi-Map Navigation and Wormhole Implementation- Assignment

Download Sqlite3:
```
sudo apt install sqlite3 libsqlite3-dev
```

mkdir -p ros2_ws/src
cd ros2_ws
git clone https://github.com/Nandostream11/Quadrupedal-Robot-Jack.git ./src

colcon build 
source install/setup.bash
export GAZEBO_MODEL_PATH=~/ros2_ws/install/wbot_description/share:$GAZEBO_MODEL_PATH

ros2 launch wbot_description gazebo.launch.py