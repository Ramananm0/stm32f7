source /opt/ros/humble/setup.bash
cd ~/raspberry_pi4b/ros2_ws || exit 1
colcon build --symlink-install
source ~/raspberry_pi4b/ros2_ws/install/setup.bash
echo ---NODES---
ros2 pkg executables terrain_ai
ros2 pkg executables terrain_bringup || true
