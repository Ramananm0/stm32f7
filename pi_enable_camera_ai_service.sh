cat <<'EOF' > /home/ubuntu/raspberry_pi4b/start_camera_ai.sh
#!/usr/bin/env bash
set -e
source /opt/ros/humble/setup.bash
source /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash
export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera/ipa
export PYTHONPATH=/usr/local/lib/python3/dist-packages:/usr/local/lib/python3.10/dist-packages:${PYTHONPATH}
ros2 run terrain_ai terrain_camera_node --ros-args -p width:=640 -p height:=480 -p fps:=10 &
CAM_PID=$!
ros2 run terrain_ai terrain_ai_node --ros-args -p rate_hz:=0.2
wait $CAM_PID
EOF

chmod +x /home/ubuntu/raspberry_pi4b/start_camera_ai.sh

cat <<'EOF' | sudo tee /etc/systemd/system/terrain-camera-ai.service >/dev/null
[Unit]
Description=Terrain Camera + AI Overlay
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=ubuntu
WorkingDirectory=/home/ubuntu/raspberry_pi4b
ExecStart=/home/ubuntu/raspberry_pi4b/start_camera_ai.sh
Restart=on-failure
RestartSec=5
Environment=HOME=/home/ubuntu

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl disable terrain-bringup.service || true
sudo systemctl stop terrain-bringup.service || true
sudo systemctl enable terrain-camera-ai.service
sudo systemctl restart terrain-camera-ai.service
sleep 10

echo ---SERVICES---
sudo systemctl --no-pager --full status terrain-camera-ai.service | sed -n '1,60p' || true
echo ---
sudo systemctl --no-pager --full status terrain-bringup.service | sed -n '1,30p' || true

source /opt/ros/humble/setup.bash
source /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash
echo ---TOPICS---
timeout 8s ros2 topic list | grep -E 'camera|terrain' || true
echo ---HZ_CAMERA---
timeout 10s ros2 topic hz /camera/image_raw || true
