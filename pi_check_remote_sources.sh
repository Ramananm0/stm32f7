set -e
echo "=== SOURCE FILES ==="
ls -l ~/raspberry_pi4b/ros2_ws/src/terrain_odom/terrain_odom || true
ls -l ~/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch || true

echo "=== SETUP.PY ==="
sed -n '1,220p' ~/raspberry_pi4b/ros2_ws/src/terrain_odom/setup.py || true

echo "=== BASE STATUS NODE ==="
sed -n '1,220p' ~/raspberry_pi4b/ros2_ws/src/terrain_odom/terrain_odom/base_status_node.py || true

echo "=== MICROROS LAUNCH ==="
sed -n '1,220p' ~/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py || true

echo "=== BRINGUP LAUNCH ==="
sed -n '1,260p' ~/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/bringup.launch.py || true
