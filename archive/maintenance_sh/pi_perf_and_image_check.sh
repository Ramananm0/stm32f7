echo ---LOAD---
uptime
free -h
echo ---TOP---
top -b -n 1 | sed -n '1,20p'

source /opt/ros/humble/setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash

echo ---TOPICS---
timeout 5s ros2 topic list | grep -E 'camera|terrain'
echo ---HZ_CAMERA---
timeout 10s ros2 topic hz /camera/image_raw || true
echo ---HZ_ANNOTATED---
timeout 10s ros2 topic hz /terrain/annotated_image || true
echo ---HZ_DETECTED---
timeout 10s ros2 topic hz /terrain/detected || true

echo ---IMAGE_FIELDS_CAMERA---
timeout 8s ros2 topic echo /camera/image_raw --once | sed -n '1,40p' || true
echo ---IMAGE_FIELDS_ANNOTATED---
timeout 8s ros2 topic echo /terrain/annotated_image --once | sed -n '1,40p' || true

echo ---AI_LOG---
journalctl -u terrain-bringup.service -n 80 --no-pager | grep -E 'terrain_ai_node|terrain_camera_node|Image conversion failed|Model loaded|Camera started' || true
