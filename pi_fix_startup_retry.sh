cp /home/ubuntu/.bashrc /home/ubuntu/.bashrc.bak_codex_retry
sed -i '/source ~\/microros_ws\/install\/setup.bash/d' /home/ubuntu/.bashrc
cat >> /home/ubuntu/.bashrc <<'EOF'
if [ -f /opt/ros/humble/setup.bash ]; then
    source /opt/ros/humble/setup.bash
fi
if [ -f ~/microros_ws/install/setup.bash ]; then
    source ~/microros_ws/install/setup.bash
fi
if [ -f ~/raspberry_pi4b/ros2_ws/install/setup.bash ]; then
    source ~/raspberry_pi4b/ros2_ws/install/setup.bash
fi
EOF

sed -i "s/Baud rate  : 2 000 000/Baud rate  : 115200/g" /home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py
sed -i "s/'-b',   '2000000'/'-b',   '115200'/g" /home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py
sed -i "s/# 2 Mbaud — must match STM32 UART1/# must match STM32 USART6/g" /home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py

echo "=== BASHRC SOURCES ==="
grep -n "setup.bash" /home/ubuntu/.bashrc
echo "=== LAUNCH BAUD ==="
grep -n "115200\\|2000000\\|USART6\\|Mbaud" /home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py
