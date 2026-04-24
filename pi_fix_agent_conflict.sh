python3 - <<'PY'
from pathlib import Path

path = Path('/home/ubuntu/raspberry_pi4b/start_bringup.sh')
text = path.read_text()
old = "exec ros2 launch terrain_bringup bringup.launch.py\n"
new = "exec ros2 launch terrain_bringup bringup.launch.py enable_microros:=false enable_base_status:=true\n"
if old in text and "enable_microros:=false" not in text:
    text = text.replace(old, new)
path.write_text(text)
print(path.read_text())
PY

sudo systemctl restart micro-ros-agent.service
sudo systemctl restart terrain-bringup.service
sleep 12

echo "=== SERVICE STATE ==="
systemctl is-active micro-ros-agent.service || true
systemctl is-active terrain-bringup.service || true

echo "=== MICRO ROS PROCS ==="
pgrep -af micro_ros_agent || true

echo "=== BRINGUP STATUS ==="
systemctl --no-pager --full status terrain-bringup.service | sed -n '1,60p' || true

echo "=== MICRO ROS STATUS ==="
systemctl --no-pager --full status micro-ros-agent.service | sed -n '1,60p' || true

echo "=== TOPICS ==="
. /opt/ros/humble/setup.bash >/dev/null 2>&1 || true
. /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash >/dev/null 2>&1 || true
ros2 topic list | grep -E "^/base_status$|^/base_status_text$|^/wheel_ticks$|^/wheel_velocity$|^/imu/data$" || true

echo "=== TOPIC INFO ==="
ros2 topic info /wheel_ticks || true
ros2 topic info /base_status || true

echo "=== TICKS ONCE ==="
timeout 8s ros2 topic echo /wheel_ticks --once || true

echo "=== BASE STATUS ONCE ==="
timeout 8s ros2 topic echo /base_status --once || true
