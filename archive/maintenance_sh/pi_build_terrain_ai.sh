source /opt/ros/humble/setup.bash
cd ~/raspberry_pi4b/ros2_ws || exit 1
colcon build --packages-select terrain_ai --symlink-install
source ~/raspberry_pi4b/ros2_ws/install/setup.bash
echo ---EXECUTABLES---
ros2 pkg executables terrain_ai
