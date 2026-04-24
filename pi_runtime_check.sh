source /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1 || true
echo "=== PROCS ==="
ps -ef | grep -E "micro_ros_agent|rplidar|terrain_|ekf_node|slam_toolbox|teleop_twist_keyboard" | grep -v grep || true
echo "=== TOPICS ==="
ros2 topic list || true
echo "=== HZ ==="
timeout 6s ros2 topic hz /imu/data /wheel_ticks /wheel_velocity /scan 2>/dev/null || true
echo "=== AGENT LOG ==="
tail -n 80 /tmp/microros_115200.log 2>/dev/null || tail -n 80 /tmp/microros_agent.log 2>/dev/null || echo "no_agent_log"
echo "=== SYSTEM ==="
uptime
free -h
df -h /
echo "=== SERIAL ERRORS ==="
dmesg 2>/dev/null | tail -n 80 | grep -E "ttyUSB|pl2303|cp210|overrun|overflow|error" || true
