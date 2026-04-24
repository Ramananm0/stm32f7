cp /home/ubuntu/.bashrc /home/ubuntu/.bashrc.bak_codex
python3 - <<'PY'
from pathlib import Path
p = Path('/home/ubuntu/.bashrc')
text = p.read_text()
old = "source ~/microros_ws/install/setup.bash\nsource ~/microros_ws/install/setup.bash\n"
new = (
    "if [ -f /opt/ros/humble/setup.bash ]; then\n"
    "    source /opt/ros/humble/setup.bash\n"
    "fi\n"
    "if [ -f ~/microros_ws/install/setup.bash ]; then\n"
    "    source ~/microros_ws/install/setup.bash\n"
    "fi\n"
    "if [ -f ~/raspberry_pi4b/ros2_ws/install/setup.bash ]; then\n"
    "    source ~/raspberry_pi4b/ros2_ws/install/setup.bash\n"
    "fi\n"
)
if old in text:
    text = text.replace(old, new)
else:
    text += "\n" + new
p.write_text(text)

p2 = Path('/home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py')
text2 = p2.read_text()
text2 = text2.replace('Baud rate  : 2 000 000', 'Baud rate  : 115200')
text2 = text2.replace('# 2 Mbaud — must match STM32 UART1', '# must match STM32 USART6')
text2 = text2.replace(\"'-b',   '2000000'\", \"'-b',   '115200'\")
p2.write_text(text2)
print('updated bashrc and microros_agent.launch.py')
PY
echo "=== BASHRC SOURCES ==="
grep -n "setup.bash" /home/ubuntu/.bashrc
echo "=== LAUNCH BAUD ==="
grep -n "115200\\|2000000" /home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py
