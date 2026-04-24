hostname
echo ---VIDEO---
ls -l /dev/video* 2>/dev/null || true
echo ---CAMERA---
which libcamera-hello >/dev/null 2>&1 && timeout 8s libcamera-hello --list-cameras || true
echo ---ROSNODES---
ps -ef | grep -E "ros2|camera|ai_node|rplidar|terrain" | grep -v grep || true
echo ---TOPICS---
source /opt/ros/humble/setup.bash >/dev/null 2>&1
[ -f ~/raspberry_pi4b/ros2_ws/install/setup.bash ] && source ~/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1
timeout 5s ros2 topic list 2>/dev/null || true
echo ---PKGS---
timeout 5s ros2 pkg executables terrain_ai 2>/dev/null || true
