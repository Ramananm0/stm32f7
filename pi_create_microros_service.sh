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
systemctl status micro-ros-agent.service --no-pager
