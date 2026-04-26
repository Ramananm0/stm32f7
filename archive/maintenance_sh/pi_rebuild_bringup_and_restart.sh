source /opt/ros/humble/setup.bash
cd ~/raspberry_pi4b/ros2_ws || exit 1
colcon build --packages-select terrain_bringup --symlink-install
sudo systemctl restart terrain-bringup.service
sleep 12
echo ---SERVICE---
sudo systemctl --no-pager --full status terrain-bringup.service | sed -n '1,80p'
echo ---TOPICS---
source ~/raspberry_pi4b/ros2_ws/install/setup.bash
timeout 8s ros2 topic list | grep -E 'camera|terrain|imu|wheel|scan|odom' || true
echo ---SCAN_ONCE---
timeout 12s ros2 topic echo /scan --once || true
