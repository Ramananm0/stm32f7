set -e
source /opt/ros/humble/setup.bash
cd ~/raspberry_pi4b/ros2_ws
colcon build --symlink-install
source ~/raspberry_pi4b/ros2_ws/install/setup.bash

pkill -f "micro_ros_agent serial --dev /dev/stm32 -b 115200" 2>/dev/null || true
pkill -f "ros2 launch terrain_bringup bringup.launch.py" 2>/dev/null || true
sleep 2

nohup ros2 launch terrain_bringup bringup.launch.py >/tmp/terrain_bringup.log 2>&1 &
sleep 15

echo "=== PROCESSES ==="
ps -ef | grep -E "micro_ros_agent|terrain_|rplidar|ekf_node|slam_toolbox|ros2 launch terrain_bringup" | grep -v grep || true

echo "=== TOPICS ==="
ros2 topic list | grep -E "^/base_status$|^/base_status_text$|^/wheel_ticks$|^/wheel_velocity$|^/imu/data$|^/odom$|^/scan$" || true

echo "=== TOPIC INFO ==="
ros2 topic info /wheel_ticks || true
ros2 topic info /wheel_velocity || true
ros2 topic info /base_status || true

echo "=== WHEEL TICKS ONCE ==="
timeout 8s ros2 topic echo /wheel_ticks --once || true

echo "=== WHEEL VELOCITY ONCE ==="
timeout 8s ros2 topic echo /wheel_velocity --once || true

echo "=== BASE STATUS ONCE ==="
timeout 8s ros2 topic echo /base_status --once || true

echo "=== IMU ONCE ==="
timeout 8s ros2 topic echo /imu/data --once || true

echo "=== BRINGUP LOG ==="
tail -n 80 /tmp/terrain_bringup.log || true
