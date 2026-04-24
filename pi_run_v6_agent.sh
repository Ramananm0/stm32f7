sudo systemctl stop terrain-bringup.service
rm -f /tmp/microros_agent_v6.log /tmp/microros_agent_v6.pid
bash -lc 'nohup bash -lc "source /opt/ros/humble/setup.bash; source ~/microros_ws/install/local_setup.bash; ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/stm32 -b 115200 -v6" >/tmp/microros_agent_v6.log 2>&1 < /dev/null & echo $! > /tmp/microros_agent_v6.pid'
sleep 2
cat /tmp/microros_agent_v6.pid
ps -p "$(cat /tmp/microros_agent_v6.pid)" -o pid=,cmd=
