pkill -f micro_ros_agent || true
nohup /home/ubuntu/microros_ws/install/micro_ros_agent/lib/micro_ros_agent/micro_ros_agent serial --dev /dev/stm32 -b 115200 -v6 >/tmp/microros_agent_v6.log 2>&1 &
sleep 6
sed -n '1,220p' /tmp/microros_agent_v6.log || true
