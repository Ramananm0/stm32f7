echo ---SERVICE---
sudo systemctl --no-pager --full status terrain-bringup.service | sed -n '1,80p'

echo ---PROCS---
ps -ef | grep -E 'terrain_camera_node|terrain_ai_node|micro_ros_agent|rplidar_node' | grep -v grep || true

source /opt/ros/humble/setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash

echo ---TOPICS---
timeout 8s ros2 topic list | grep -E 'camera|terrain|imu|wheel|scan' || true

echo ---TOPIC_INFO_CAMERA---
timeout 8s ros2 topic info /camera/image_raw || true
echo ---TOPIC_INFO_ANNOTATED---
timeout 8s ros2 topic info /terrain/annotated_image || true

echo ---HZ_CAMERA---
timeout 10s ros2 topic hz /camera/image_raw || true
echo ---HZ_DETECTED---
timeout 12s ros2 topic hz /terrain/detected || true

echo ---SAFETY_ONCE---
timeout 10s ros2 topic echo /terrain/safety_level --once || true
echo ---DETECTED_ONCE---
timeout 12s ros2 topic echo /terrain/detected --once || true

echo ---AI_LOG---
journalctl -u terrain-bringup.service -n 120 --no-pager | grep -E 'terrain_ai_node|terrain_camera_node|Model loaded|annotated|message|STOP|SAFE|CAUTION|UNSAFE' || true
