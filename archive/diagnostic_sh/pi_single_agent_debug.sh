echo "=== START ==="
. /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/microros_ws/install/setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1 || true

pkill -f "micro_ros_agent serial --dev /dev/stm32 -b 115200" 2>/dev/null || true
sleep 2

nohup ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/stm32 -b 115200 -v4 >/tmp/microros_agent_debug.log 2>&1 &
sleep 8

echo "=== AGENT PROCESSES ==="
pgrep -af micro_ros_agent || true

echo "=== AGENT LOG ==="
tail -n 80 /tmp/microros_agent_debug.log || true

echo "=== TOPIC INFO ==="
ros2 topic info /imu/data || true
ros2 topic info /wheel_ticks || true
ros2 topic info /wheel_velocity || true
ros2 topic info /base_status || true

echo "=== IMU ONCE ==="
timeout 8s ros2 topic echo /imu/data --once || true

echo "=== TICKS ONCE ==="
timeout 8s ros2 topic echo /wheel_ticks --once || true
