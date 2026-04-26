echo "=== SERVICES ==="
systemctl is-active terrain-bringup.service 2>/dev/null || true
systemctl is-active micro-ros-agent.service 2>/dev/null || true
echo "=== TERRAIN BRINGUP STATUS ==="
systemctl --no-pager --full status terrain-bringup.service 2>/dev/null | sed -n '1,80p' || true
echo "=== MICRO ROS STATUS ==="
systemctl --no-pager --full status micro-ros-agent.service 2>/dev/null | sed -n '1,80p' || true
