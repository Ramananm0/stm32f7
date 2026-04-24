echo ---HOST---
hostname
echo ---UPTIME---
uptime
echo ---DEVICES---
ls -l /dev/stm32 /dev/rplidar /dev/video0 2>/dev/null || true
echo ---PROCS---
ps -ef | grep -E 'micro_ros_agent|terrain_|rplidar|ros2 launch|image_view|rqt' | grep -v grep || true
echo ---ROS_ENV---
if [ -f /opt/ros/humble/setup.bash ]; then
  source /opt/ros/humble/setup.bash
fi
if [ -f ~/raspberry_pi4b/ros2_ws/install/setup.bash ]; then
  source ~/raspberry_pi4b/ros2_ws/install/setup.bash
fi
echo ---TOPICS---
timeout 5s ros2 topic list 2>/dev/null || true
echo ---AI_TOPICS---
timeout 5s ros2 topic list 2>/dev/null | grep -E 'camera|terrain|imu|wheel|scan' || true
