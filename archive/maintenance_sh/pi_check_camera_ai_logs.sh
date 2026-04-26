source /opt/ros/humble/setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash
echo ---PROCS---
ps -ef | grep -E 'terrain_camera_node|terrain_ai_node' | grep -v grep || true
echo ---TOPICS---
timeout 5s ros2 topic list | grep -E 'camera|terrain' || true
echo ---AI_LOG---
tail -n 80 /tmp/terrain_ai.log || true
echo ---CAM_LOG---
tail -n 80 /tmp/terrain_camera.log || true
