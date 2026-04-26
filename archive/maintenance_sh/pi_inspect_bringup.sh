cd ~/raspberry_pi4b/ros2_ws/src || exit 1
echo ---FILES---
find . -maxdepth 2 -type f | sort | sed -n '1,200p'
echo ---BRINGUP---
sed -n '1,260p' ~/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/bringup.launch.py
echo ---AI_BRINGUP---
sed -n '1,260p' ~/raspberry_pi4b/ros2_ws/src/terrain_ai/terrain_ai/bringup.launch.py
