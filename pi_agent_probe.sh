source /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
source /home/ubuntu/microros_ws/install/setup.bash >/dev/null 2>&1 || true
pkill -f micro_ros_agent || true
rm -f /tmp/microros_agent.log
nohup ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/stm32 -b 115200 -v6 >/tmp/microros_agent.log 2>&1 &
sleep 6
echo ---AGENT_LOG---
tail -n 120 /tmp/microros_agent.log || true
echo ---TOPICS---
ros2 topic list || true
