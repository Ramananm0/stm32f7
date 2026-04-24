source /opt/ros/humble/setup.bash
cd ~/raspberry_pi4b/ros2_ws || exit 1
colcon build --packages-select terrain_bringup --symlink-install

sudo systemctl disable terrain-camera-ai.service || true
sudo systemctl stop terrain-camera-ai.service || true
sudo systemctl enable terrain-bringup.service
sudo systemctl restart terrain-bringup.service
sleep 12

echo ---SERVICES---
sudo systemctl --no-pager --full status terrain-bringup.service | sed -n '1,60p' || true
echo ---
sudo systemctl --no-pager --full status terrain-camera-ai.service | sed -n '1,30p' || true

source ~/raspberry_pi4b/ros2_ws/install/setup.bash
echo ---TOPICS---
timeout 8s ros2 topic list | grep -E 'imu|wheel|scan|odom|terrain|camera' || true
