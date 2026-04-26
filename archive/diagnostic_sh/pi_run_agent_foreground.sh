source /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
source /home/ubuntu/microros_ws/install/local_setup.bash >/dev/null 2>&1 || true
timeout 15s ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/stm32 -b 115200 -v6
