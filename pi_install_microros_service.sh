if [ -f /tmp/microros_agent_v6.pid ]; then
  pid="$(cat /tmp/microros_agent_v6.pid 2>/dev/null || true)"
  if [ -n "$pid" ]; then
    kill "$pid" 2>/dev/null || true
  fi
fi
pkill -f "micro_ros_agent serial --dev /dev/stm32 -b 115200" 2>/dev/null || true

python3 - <<'PY'
from pathlib import Path

paths = [
    Path('/home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/bringup.launch.py'),
    Path('/home/ubuntu/raspberry_pi4b/ros2_ws/install/terrain_bringup/share/terrain_bringup/launch/bringup.launch.py'),
]

for path in paths:
    text = path.read_text()
    text = text.replace(
        "    microros,\n",
        "",
    )
    path.write_text(text)
PY

cat <<'EOF' | sudo tee /etc/systemd/system/micro-ros-agent.service >/dev/null
[Unit]
Description=Micro-ROS Agent for STM32 terrain bot
After=network.target
Wants=network.target

[Service]
Type=simple
User=ubuntu
WorkingDirectory=/home/ubuntu/microros_ws
ExecStartPre=/bin/bash -lc 'until [ -e /dev/stm32 ]; do sleep 1; done'
ExecStart=/bin/bash -lc 'source /opt/ros/humble/setup.bash && source /home/ubuntu/microros_ws/install/local_setup.bash && exec ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/stm32 -b 115200 -v4'
Restart=always
RestartSec=2

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable micro-ros-agent.service
sudo systemctl restart micro-ros-agent.service
sudo systemctl restart terrain-bringup.service
systemctl is-active micro-ros-agent.service
systemctl is-active terrain-bringup.service
