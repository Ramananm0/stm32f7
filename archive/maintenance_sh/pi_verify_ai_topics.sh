source /opt/ros/humble/setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash
echo ---DETECTED---
timeout 12s ros2 topic echo /terrain/detected --once || true
echo ---SAFETY---
timeout 12s ros2 topic echo /terrain/safety_level --once || true
echo ---ANNOTATED---
timeout 12s ros2 topic info /terrain/annotated_image || true
