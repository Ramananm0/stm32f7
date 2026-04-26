#!/usr/bin/env bash
set -euo pipefail

# Single-entry laptop launcher:
# 1. Verify Pi reachability over Tailscale
# 2. Recover Pi-side rover services over SSH
# 3. Refresh local ROS 2 discovery
# 4. Launch the existing laptop UI/tools
#
# Usage:
#   bash ~/stm32f7/rover_live/laptop/host_connect.sh
#   PI_IP=100.88.254.75 bash ~/stm32f7/rover_live/laptop/host_connect.sh
#   DIRECT=1 PI_IP=100.88.254.75 bash ~/stm32f7/rover_live/laptop/host_connect.sh

PI_IP="${PI_IP:-100.88.254.75}"
PI_USER="${PI_USER:-ubuntu}"
ROS_TOPIC_WAIT_TRIES="${ROS_TOPIC_WAIT_TRIES:-8}"
ROS_TOPIC_WAIT_SECONDS="${ROS_TOPIC_WAIT_SECONDS:-3}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
START_SCRIPT="$SCRIPT_DIR/start.sh"

if [ ! -f "$START_SCRIPT" ]; then
    echo "Missing launcher: $START_SCRIPT"
    exit 1
fi

echo "================================================"
echo "   TERRAIN ROVER — HOST CONNECT"
echo "   Pi: ${PI_USER}@${PI_IP}"
echo "================================================"

# Tailscale is the default transport for this launcher. Keep the peer profile
# enabled unless the caller explicitly overrides it.
USE_FASTDDS_PEER="${USE_FASTDDS_PEER:-1}"

echo "Discovery mode: Tailscale/FastDDS peer"
echo "Tailscale service is not restarted by this script"

echo -n "Pinging Pi (${PI_IP})... "
if ! ping -c 1 -W 3 "$PI_IP" >/dev/null 2>&1; then
    echo "FAIL"
    echo "Pi is not reachable. Check Tailscale and the Pi network."
    exit 1
fi
echo "OK"

echo "Recovering Pi rover services over SSH..."
REMOTE_RECOVERY_CMD='
set -e
for candidate in \
    "${PI_START_SCRIPT:-$HOME/start_pi.sh}" \
    "${PI_START_SCRIPT:-$HOME/raspberry_pi4b/rover_live/pi/start_pi.sh}" \
    "$HOME/stm32f7/rover_live/pi/start_pi.sh" \
    "$HOME/rover_live/pi/start_pi.sh"
do
    if [ -f "$candidate" ]; then
        echo "Using Pi start script: $candidate"
        exec bash "$candidate"
    fi
done
echo "No Pi start script found in expected locations." >&2
exit 1
'
if ! ssh -o BatchMode=yes -o ConnectTimeout=8 "${PI_USER}@${PI_IP}" "$REMOTE_RECOVERY_CMD"; then
    echo "SSH recovery failed."
    echo "Make sure SSH keys are set up for ${PI_USER}@${PI_IP} and the Pi start script exists."
    exit 1
fi

echo "Refreshing local ROS 2 discovery..."
source /opt/ros/humble/setup.bash
export ROS_LOCALHOST_ONLY=0
export ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-0}"
ros2 daemon stop >/dev/null 2>&1 || true
ros2 daemon start >/dev/null 2>&1 || true

topic_count=0
for ((i = 1; i <= ROS_TOPIC_WAIT_TRIES; i++)); do
    topic_count="$(timeout 6 ros2 topic list 2>/dev/null | wc -l | tr -d ' ')"
    if [ "${topic_count}" -ge 3 ]; then
        echo "ROS topics visible from Pi: ${topic_count}"
        break
    fi
    echo "Waiting for Pi topics (${i}/${ROS_TOPIC_WAIT_TRIES})..."
    sleep "${ROS_TOPIC_WAIT_SECONDS}"
done

if [ "${topic_count}" -lt 3 ]; then
    echo "Warning: only ${topic_count} ROS topics visible after Pi recovery."
    echo "Launching laptop tools anyway."
fi

echo "Starting laptop launcher..."
exec env PI_IP="$PI_IP" USE_FASTDDS_PEER="$USE_FASTDDS_PEER" DIRECT="${DIRECT:-0}" \
    bash "$START_SCRIPT"
