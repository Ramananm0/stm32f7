#!/bin/sh
. /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/microros_ws/install/local_setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1 || true

echo "=== TOPIC INFO ==="
ros2 topic info /cmd_vel_raw || true
ros2 topic info /cmd_vel || true
ros2 topic info /terrain/speed_factor || true

echo "=== SPEED FACTOR ==="
timeout 5s ros2 topic echo /terrain/speed_factor --once || true

echo "=== WATCH CMD_VEL ==="
timeout 8s ros2 topic echo /cmd_vel >/tmp/cmd_vel_seen.txt 2>/dev/null &
E1=$!
timeout 8s ros2 topic echo /cmd_vel_raw >/tmp/cmd_vel_raw_seen.txt 2>/dev/null &
E2=$!

ros2 topic pub -r 10 /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.04, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/tmp/cmd_vel_pub_direct.txt 2>&1 &
P1=$!
ros2 topic pub -r 10 /cmd_vel_raw geometry_msgs/msg/Twist "{linear: {x: 0.04, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/tmp/cmd_vel_pub_raw.txt 2>&1 &
P2=$!
sleep 3
kill $P1 $P2 2>/dev/null || true
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/dev/null 2>&1 || true
ros2 topic pub --once /cmd_vel_raw geometry_msgs/msg/Twist "{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/dev/null 2>&1 || true
wait $E1 2>/dev/null || true
wait $E2 2>/dev/null || true

echo "=== SEEN /cmd_vel ==="
tail -n 40 /tmp/cmd_vel_seen.txt 2>/dev/null || true
echo "=== SEEN /cmd_vel_raw ==="
tail -n 40 /tmp/cmd_vel_raw_seen.txt 2>/dev/null || true

echo "=== POST WHEEL VELOCITY ==="
timeout 5s ros2 topic echo /wheel_velocity --once || true
