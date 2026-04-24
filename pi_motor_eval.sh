#!/bin/sh
. /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/microros_ws/install/local_setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1 || true

echo "=== TOPIC INFO ==="
ros2 topic info /wheel_ticks || true
ros2 topic info /wheel_velocity || true
ros2 topic info /imu/data || true
ros2 topic info /cmd_vel || true

echo "=== PRECHECK ==="
timeout 5s ros2 topic echo /base_status --once || true
timeout 5s ros2 topic echo /wheel_velocity --once || true

echo "=== FORWARD TEST ==="
timeout 5s ros2 topic echo /wheel_velocity >/tmp/wheel_velocity_eval.txt 2>/dev/null &
ECHO_PID=$!
ros2 topic pub -r 10 /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.03, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/tmp/cmd_vel_eval.txt 2>&1 &
PUB_PID=$!
sleep 3
kill $PUB_PID 2>/dev/null || true
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/dev/null 2>&1 || true
wait $ECHO_PID 2>/dev/null || true

echo "=== WHEEL VELOCITY SAMPLE ==="
tail -n 80 /tmp/wheel_velocity_eval.txt 2>/dev/null || true

echo "=== TICKS ONCE ==="
timeout 5s ros2 topic echo /wheel_ticks --once || true

echo "=== IMU ONCE ==="
timeout 5s ros2 topic echo /imu/data --once || true
