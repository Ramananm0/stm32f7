echo "=== PROCESSES ==="
ps -ef | grep -E "micro_ros_agent|terrain_|rplidar|ekf_node|slam_toolbox|ros2 launch terrain_bringup" | grep -v grep || true

echo "=== TOPICS ==="
source /opt/ros/humble/setup.bash
source /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash
ros2 topic list | grep -E "^/base_status$|^/base_status_text$|^/wheel_ticks$|^/wheel_velocity$|^/imu/data$|^/odom$|^/scan$" || true

echo "=== TOPIC INFO ==="
ros2 topic info /wheel_ticks || true
ros2 topic info /wheel_velocity || true
ros2 topic info /base_status || true
ros2 topic info /base_status_text || true

echo "=== TICKS ONCE ==="
timeout 8s ros2 topic echo /wheel_ticks --once || true

echo "=== VELOCITY ONCE ==="
timeout 8s ros2 topic echo /wheel_velocity --once || true

echo "=== BASE STATUS ONCE ==="
timeout 8s ros2 topic echo /base_status --once || true

echo "=== BASE STATUS TEXT ONCE ==="
timeout 8s ros2 topic echo /base_status_text --once || true

echo "=== BRINGUP LOG ==="
tail -n 120 /tmp/terrain_bringup.log 2>/dev/null || echo "no bringup log"
