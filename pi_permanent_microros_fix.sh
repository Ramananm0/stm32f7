python3 - <<'PY'
from pathlib import Path

files = [
    Path('/home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/microros_agent.launch.py'),
    Path('/home/ubuntu/raspberry_pi4b/ros2_ws/install/terrain_bringup/share/terrain_bringup/launch/microros_agent.launch.py'),
    Path('/home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/bringup.launch.py'),
    Path('/home/ubuntu/raspberry_pi4b/ros2_ws/install/terrain_bringup/share/terrain_bringup/launch/bringup.launch.py'),
]

for path in files:
    text = path.read_text()
    orig = text
    if path.name == 'microros_agent.launch.py':
        if "respawn=True" not in text:
            text = text.replace(
                "        output='screen',\n        name='micro_ros_agent',\n",
                "        output='screen',\n        name='micro_ros_agent',\n        respawn=True,\n        respawn_delay=5.0,\n",
            )
    else:
        text = text.replace(
            "    _stm32_present = (\n        os.path.exists('/dev/stm32') and\n        not _same_device('/dev/stm32', '/dev/rplidar')\n    )\n",
            "",
        )
        text = text.replace(
            "        *([microros] if _stm32_present else []),\n",
            "        microros,\n",
        )
    if text != orig:
        path.write_text(text)
        print(f'updated {path}')
    else:
        print(f'nochange {path}')
PY

sudo systemctl restart terrain-bringup.service
sleep 8
echo ---SERVICE---
systemctl status terrain-bringup.service --no-pager | sed -n '1,60p'
echo ---MICROROS---
ps -ef | grep micro_ros_agent | grep -v grep || true
