#!/bin/bash
# Terrain Rover — Laptop launcher
# Usage: bash ~/stm32f7/rover_live/laptop/start.sh
# Pi must be powered on and on same network first.

PI_IP="100.88.254.75"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "================================================"
echo "   TERRAIN ROVER — LIVE LAPTOP LAUNCH"
echo "================================================"

# Check Pi reachable
echo -n "Pinging Pi ($PI_IP)... "
if ! ping -c 1 -W 3 "$PI_IP" > /dev/null 2>&1; then
    echo "FAIL — Pi not reachable. Check network."
    exit 1
fi
echo "OK"

# Check ROS
source /opt/ros/humble/setup.bash
echo -n "Checking ROS topics from Pi... "
TOPICS=$(timeout 6 ros2 topic list 2>/dev/null | wc -l)
if [ "$TOPICS" -lt 5 ]; then
    echo "WARNING: only $TOPICS topics visible."
    echo "Pi services may still be starting. Continuing anyway..."
else
    echo "OK ($TOPICS topics)"
fi

# Window 1: RViz
gnome-terminal --title="RViz" -- bash -c "
source /opt/ros/humble/setup.bash
rviz2 -d '$SCRIPT_DIR/rviz_rover.rviz'
exec bash"

sleep 2

# Window 2: Teleop
gnome-terminal --title="Teleop" -- bash -c "
source /opt/ros/humble/setup.bash
echo ''
echo '========================================='
echo '  i=forward   ,=backward   k=STOP'
echo '  j=left       l=right'
echo '  u=fwd-left   o=fwd-right'
echo '  q/z = speed up/down'
echo '========================================='
echo ''
ros2 run teleop_twist_keyboard teleop_twist_keyboard \
    --ros-args --remap cmd_vel:=/cmd_vel
exec bash"

sleep 1

# Window 3: Live status monitor
gnome-terminal --title="Rover Monitor" -- bash -c "
source /opt/ros/humble/setup.bash
echo '=== ROVER LIVE MONITOR ==='
echo 'Press Ctrl+C to exit'
echo ''
while true; do
    clear
    echo '=== ROVER STATUS ===' && date
    echo ''
    echo '-- Base --'
    timeout 1 ros2 topic echo /base_status_text --once 2>/dev/null || echo '  (no data)'
    echo ''
    echo '-- Roll/Pitch --'
    echo -n '  roll: ';  timeout 1 ros2 topic echo /roll_deg  --once 2>/dev/null | grep data | head -1 || echo '?'
    echo -n '  pitch: '; timeout 1 ros2 topic echo /pitch_deg --once 2>/dev/null | grep data | head -1 || echo '?'
    echo ''
    echo '-- Wheel Velocity (mm/s) --'
    timeout 1 ros2 topic echo /wheel_velocity --once 2>/dev/null | head -6 || echo '  (no data)'
    sleep 2
done
exec bash"

echo ""
echo "================================================"
echo "  3 windows launched: RViz | Teleop | Monitor"
echo ""
echo "  RViz tip: Fixed Frame = odom"
echo "  To switch to map frame once SLAM builds a map,"
echo "  change Fixed Frame to 'map' in RViz Global Options"
echo "================================================"
