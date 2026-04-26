#!/bin/sh
. /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/microros_ws/install/local_setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1 || true

pkill -f micro_ros_agent 2>/dev/null || true
sleep 2

nohup /home/ubuntu/microros_ws/install/micro_ros_agent/lib/micro_ros_agent/micro_ros_agent serial --dev /dev/stm32 -b 115200 -v6 >/tmp/microros_hold.log 2>&1 &
sleep 12

echo "=== AGENT PROCS ==="
pgrep -af micro_ros_agent || true

echo "=== STM NODE ==="
ros2 node list 2>/dev/null | grep stm32 || true

echo "=== TOPIC INFO ==="
ros2 topic info /base_status || true
ros2 topic info /wheel_ticks || true
ros2 topic info /imu/data || true

echo "=== AGENT LOG ==="
tail -n 120 /tmp/microros_hold.log 2>/dev/null || true
