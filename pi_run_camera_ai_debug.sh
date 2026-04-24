set -x
source /opt/ros/humble/setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash
echo after_source
pkill -f terrain_camera_node || true
pkill -f terrain_ai_node || true
echo after_kill
sleep 1
export PYTHONPATH="/usr/local/lib/python3/dist-packages:/usr/local/lib/python3.10/dist-packages:${PYTHONPATH}"
echo after_env
nohup ros2 run terrain_ai terrain_camera_node > /tmp/terrain_camera.log 2>&1 &
echo after_camera
nohup ros2 run terrain_ai terrain_ai_node --ros-args -p rate_hz:=0.2 > /tmp/terrain_ai.log 2>&1 &
echo after_ai
sleep 5
ls -l /tmp/terrain_camera.log /tmp/terrain_ai.log || true
