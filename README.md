# Multi-Map-Navigation-with-Wormhole
Multi-Map Navigation: Wormhole Implementation- Assignment
In large environments with limited edge resources, it is common to encounter instances of compute utilization by unused assets. Thus, dividing the 
Download Sqlite3:
```bash
sudo apt install sqlite3 libsqlite3-dev
```

1. Set up the workspace
```bash
mkdir -p wormhole_ws
cd ~/wormhole_ws
git clone https://github.com/Nandostream11/Quadrupedal-Robot-Jack.git ./src
```
2. Build the packages
```bash
cd ~/wormhole_ws
colcon build 
source install/setup. bash
export GAZEBO_MODEL_PATH=~/wormhole_ws/install/wbot_description/share:$GAZEBO_MODEL_PATH
```
3. Launch the robot
```bash
ros2 launch wbot_description gazebo.launch.py
```
