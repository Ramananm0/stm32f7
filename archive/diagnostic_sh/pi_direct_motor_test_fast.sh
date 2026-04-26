#!/bin/sh
. /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/microros_ws/install/local_setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1 || true

echo "=== CMD_VEL INFO ==="
ros2 topic info /cmd_vel || true

echo "=== FAST FORWARD DIRECT ==="
timeout 6s ros2 topic echo /wheel_velocity >/tmp/direct_wheel_velocity_fast.txt 2>/dev/null &
E1=$!
ros2 topic pub -r 10 /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.12, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/tmp/direct_cmd_vel_fast_pub.txt 2>&1 &
P1=$!
sleep 3
kill $P1 2>/dev/null || true
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/dev/null 2>&1 || true
wait $E1 2>/dev/null || true

echo "=== WHEEL VELOCITY SAMPLE ==="
tail -n 80 /tmp/direct_wheel_velocity_fast.txt 2>/dev/null || true

echo "=== BASE STATUS ==="
timeout 5s ros2 topic echo /base_status --once || true
