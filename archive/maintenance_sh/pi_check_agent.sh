source /opt/ros/humble/setup.bash
source /home/ubuntu/microros_ws/install/setup.bash
pkill -f micro_ros_agent || true
nohup ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/stm32 -b 115200 -v4 >/tmp/microros_agent.log 2>&1 &
sleep 5
echo "=== AGENT LOG ==="
tail -n 40 /tmp/microros_agent.log || true
echo "=== TOPICS ==="
ros2 topic list
echo "=== IMU ONCE ==="
timeout 5s ros2 topic echo /imu/data --once || true
echo "=== TICKS ONCE ==="
timeout 5s ros2 topic echo /wheel_ticks --once || true
