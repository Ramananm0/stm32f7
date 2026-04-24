python3 - <<'PY'
from pathlib import Path

paths = [
    Path('/home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/bringup.launch.py'),
    Path('/home/ubuntu/raspberry_pi4b/ros2_ws/install/terrain_bringup/share/terrain_bringup/launch/bringup.launch.py'),
]

for path in paths:
    text = path.read_text()
    if "        microros,\n" not in text:
        raise SystemExit(f"expected microros launch entry in {path}")
    text = text.replace("        microros,\n", "")
    path.write_text(text)
    print(f"patched {path}")
PY

grep -n microros /home/ubuntu/raspberry_pi4b/ros2_ws/src/terrain_bringup/launch/bringup.launch.py || true
echo ---
grep -n microros /home/ubuntu/raspberry_pi4b/ros2_ws/install/terrain_bringup/share/terrain_bringup/launch/bringup.launch.py || true
