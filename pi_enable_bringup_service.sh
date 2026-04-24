cat <<'EOF' > /home/ubuntu/raspberry_pi4b/start_bringup.sh
#!/usr/bin/env bash
set -e
source /opt/ros/humble/setup.bash
source /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash
export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera/ipa
export PYTHONPATH=/usr/local/lib/python3/dist-packages:/usr/local/lib/python3.10/dist-packages:${PYTHONPATH}
exec ros2 launch terrain_bringup bringup.launch.py
EOF

chmod +x /home/ubuntu/raspberry_pi4b/start_bringup.sh

cat <<'EOF' | sudo tee /etc/systemd/system/terrain-bringup.service >/dev/null
[Unit]
Description=Terrain Bot ROS2 Bringup
After=network-online.target systemd-udev-settle.service
Wants=network-online.target systemd-udev-settle.service

[Service]
Type=simple
User=ubuntu
WorkingDirectory=/home/ubuntu/raspberry_pi4b
ExecStart=/home/ubuntu/raspberry_pi4b/start_bringup.sh
Restart=on-failure
RestartSec=5
Environment=HOME=/home/ubuntu

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable terrain-bringup.service
sudo systemctl restart terrain-bringup.service
sleep 10
echo ---SERVICE---
sudo systemctl --no-pager --full status terrain-bringup.service || true
echo ---TOPICS---
source /opt/ros/humble/setup.bash
source /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash
timeout 8s ros2 topic list | grep -E 'camera|terrain|imu|wheel|scan|odom' || true
