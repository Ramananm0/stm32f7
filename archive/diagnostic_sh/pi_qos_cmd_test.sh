#!/bin/sh
. /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/microros_ws/install/local_setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1 || true

echo "=== WATCH /cmd_vel ==="
timeout 8s ros2 topic echo /cmd_vel >/tmp/qos_cmd_seen.txt 2>/dev/null &
E1=$!

echo "=== PUBLISH BEST_EFFORT /cmd_vel ==="
ros2 topic pub --qos-reliability best_effort -r 10 /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.04, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/tmp/qos_cmd_pub.txt 2>&1 &
P1=$!
sleep 3
kill $P1 2>/dev/null || true
ros2 topic pub --qos-reliability best_effort --once /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" >/dev/null 2>&1 || true
wait $E1 2>/dev/null || true

echo "=== SEEN /cmd_vel ==="
tail -n 60 /tmp/qos_cmd_seen.txt 2>/dev/null || true

echo "=== BASE STATUS ==="
timeout 5s ros2 topic echo /base_status --once || true

echo "=== WHEEL VELOCITY ==="
timeout 5s ros2 topic echo /wheel_velocity --once || true
