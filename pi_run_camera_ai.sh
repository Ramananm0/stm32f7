source /opt/ros/humble/setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash

export PYTHONPATH="/usr/local/lib/python3/dist-packages:/usr/local/lib/python3.10/dist-packages:${PYTHONPATH}"

nohup ros2 run terrain_ai terrain_camera_node > /tmp/terrain_camera.log 2>&1 &
nohup ros2 run terrain_ai terrain_ai_node --ros-args -p rate_hz:=0.2 > /tmp/terrain_ai.log 2>&1 &

sleep 8

echo ---PROCS---
ps -ef | grep -E 'terrain_camera_node|terrain_ai_node' | grep -v grep || true
echo ---TOPICS---
timeout 5s ros2 topic list | grep -E 'camera|terrain' || true
echo ---AI_LOG---
tail -n 40 /tmp/terrain_ai.log || true
echo ---CAM_LOG---
tail -n 40 /tmp/terrain_camera.log || true
