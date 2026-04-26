#!/bin/bash
# Terrain Rover — Laptop launcher
# Usage: bash ~/stm32f7/rover_live/laptop/start.sh
#   DIRECT=1  bash ...  → publish /cmd_vel directly (bypass terrain_risk_node)
#   USE_FASTDDS_PEER=0  → disable FastDDS unicast peer manually

PI_IP="${PI_IP:-100.88.254.75}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
FASTDDS_FILE="$REPO_ROOT/ros2/fastdds_pi_peer.xml"

# Tailscale/FastDDS peer is the default discovery path for laptop ↔ Pi.
if [ -n "${USE_FASTDDS_PEER:-}" ]; then
    USE_FASTDDS_PEER="${USE_FASTDDS_PEER}"
elif [[ "$PI_IP" == 100.* ]]; then
    USE_FASTDDS_PEER=1
else
    USE_FASTDDS_PEER=0
fi

# Direct mode: publish to /cmd_vel instead of /cmd_vel_raw
# Use when terrain_risk_node is NOT running on Pi
DIRECT="${DIRECT:-0}"

echo "================================================"
echo "   TERRAIN ROVER — LIVE LAPTOP LAUNCH"
echo "   FastDDS peer: $USE_FASTDDS_PEER  Direct: $DIRECT"
echo "================================================"

# Check Pi reachable
echo -n "Pinging Pi ($PI_IP)... "
if ! ping -c 1 -W 3 "$PI_IP" > /dev/null 2>&1; then
    echo "FAIL — Pi not reachable. Check Tailscale / network."
    exit 1
fi
echo "OK"

source /opt/ros/humble/setup.bash
export ROS_LOCALHOST_ONLY=0
export ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-0}"
export RMW_IMPLEMENTATION=rmw_fastrtps_cpp
if [ "$USE_FASTDDS_PEER" = "1" ] && [ -f "$FASTDDS_FILE" ]; then
    export FASTRTPS_DEFAULT_PROFILES_FILE="$FASTDDS_FILE"
    echo "FastDDS peer profile: $FASTDDS_FILE"
else
    unset FASTRTPS_DEFAULT_PROFILES_FILE
fi

echo -n "Checking ROS topics from Pi... "
TOPICS=$(timeout 6 ros2 topic list 2>/dev/null | wc -l)
if [ "$TOPICS" -lt 3 ]; then
    echo "WARNING: only $TOPICS topics visible."
    echo "Pi services may be starting or Tailscale/FastDDS is not ready."
    echo "Try: ssh ubuntu@$PI_IP 'bash ~/rover_live/pi/start_pi.sh'"
    echo "Continuing anyway..."
else
    echo "OK ($TOPICS topics)"
fi

# Determine teleop target topic
if [ "$DIRECT" = "1" ]; then
    TELEOP_TOPIC="/cmd_vel"
    TELEOP_NOTE="DIRECT MODE — bypassing terrain_risk_node"
else
    TELEOP_TOPIC="/cmd_vel_raw"
    TELEOP_NOTE="publishing to /cmd_vel_raw (terrain_risk_node must be running on Pi)"
fi

# Common env block passed into each terminal
ENV_BLOCK="source /opt/ros/humble/setup.bash
export ROS_LOCALHOST_ONLY=0
export ROS_DOMAIN_ID='${ROS_DOMAIN_ID:-0}'
export RMW_IMPLEMENTATION=rmw_fastrtps_cpp
$([ "$USE_FASTDDS_PEER" = "1" ] && echo "export FASTRTPS_DEFAULT_PROFILES_FILE='$FASTDDS_FILE'" || echo "unset FASTRTPS_DEFAULT_PROFILES_FILE")"

# Window 1: RViz
gnome-terminal --title="RViz" -- bash -c "
$ENV_BLOCK
rviz2 -d '$SCRIPT_DIR/rviz_rover.rviz'
exec bash"

sleep 2

# Window 2: Teleop
gnome-terminal --title="Teleop" -- bash -c "
$ENV_BLOCK
echo ''
echo '========================================='
echo '  i=forward   ,=backward   k=STOP'
echo '  j=left       l=right'
echo '  u=fwd-left   o=fwd-right'
echo '  q/z = speed up/down'
echo ''
echo '  $TELEOP_NOTE'
echo '========================================='
echo ''
ros2 run teleop_twist_keyboard teleop_twist_keyboard \
    --ros-args --remap cmd_vel:=$TELEOP_TOPIC
exec bash"

sleep 1

# Window 3: Live status monitor
gnome-terminal --title="Rover Monitor" -- bash -c "
$ENV_BLOCK
echo '=== ROVER LIVE MONITOR ==='
echo 'Press Ctrl+C to exit'
echo ''
while true; do
    clear
    echo '=== ROVER STATUS ===' && date
    echo ''
    echo '-- Base Status --'
    timeout 1 ros2 topic echo /base_status_text --once 2>/dev/null || echo '  (no data — check micro-ros-agent on Pi)'
    echo ''
    echo '-- Roll / Pitch (deg) --'
    echo -n '  roll:  '; timeout 1 ros2 topic echo /roll_deg  --once 2>/dev/null | grep 'data:' | head -1 || echo '?'
    echo -n '  pitch: '; timeout 1 ros2 topic echo /pitch_deg --once 2>/dev/null | grep 'data:' | head -1 || echo '?'
    echo ''
    echo '-- Wheel Velocity (mm/s) --'
    timeout 1 ros2 topic echo /wheel_velocity --once 2>/dev/null | head -6 || echo '  (no data)'
    echo ''
    echo '-- cmd_vel (what STM32 receives) --'
    timeout 1 ros2 topic echo /cmd_vel --once 2>/dev/null | head -6 || echo '  (no data — terrain_risk_node may be down; try DIRECT=1)'
    sleep 2
done
exec bash"

echo ""
echo "================================================"
echo "  3 windows launched: RViz | Teleop | Monitor"
echo ""
echo "  If motors don't respond:"
echo "  1. Check Pi: ssh ubuntu@$PI_IP 'bash ~/rover_live/pi/start_pi.sh'"
echo "  2. Direct mode (bypass terrain_risk): DIRECT=1 bash $0"
echo "  3. Check Monitor window — base_status shows STM32 connection"
echo "================================================"
