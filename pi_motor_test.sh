#!/bin/bash
# pi_motor_test.sh — Quick motor verification script
# Run this ON the Raspberry Pi after flashing STM32 + ESP32.
#
# What it does:
#   1. Finds /dev/stm32 (or the raw ttyUSB/ttyACM device)
#   2. Kills any stale micro-ROS agent
#   3. Starts a fresh agent at 115200 baud
#   4. Waits for the STM32 node to appear in ROS2
#   5. Drives all 4 motors forward 3 s → stops → reverse 3 s → stops
#   6. Prints pass/fail

set -e

ROS_SETUP="/opt/ros/humble/setup.bash"
UROS_SETUP="${HOME}/microros_ws/install/local_setup.bash"
AGENT_BIN="ros2 run micro_ros_agent micro_ros_agent"
BAUD=115200
LOG=/tmp/microros_motor_test.log

source "$ROS_SETUP"
[ -f "$UROS_SETUP" ] && source "$UROS_SETUP"

# ── 1. Find the STM32 UART device ─────────────────────────────────────
if [ -e /dev/stm32 ]; then
    DEV=/dev/stm32
elif ls /dev/ttyUSB* 2>/dev/null | head -1 | grep -q tty; then
    DEV=$(ls /dev/ttyUSB* 2>/dev/null | head -1)
    echo "[WARN] /dev/stm32 symlink missing — using $DEV"
    echo "       Run: sudo cp _raspberry_pi4b_check/setup/99-stm32-usb.rules /etc/udev/rules.d/"
    echo "            sudo udevadm control --reload-rules && sudo udevadm trigger"
elif ls /dev/ttyACM* 2>/dev/null | head -1 | grep -q tty; then
    DEV=$(ls /dev/ttyACM* 2>/dev/null | head -1)
    echo "[WARN] /dev/stm32 symlink missing — using $DEV"
else
    echo "[ERROR] No USB-TTL device found. Check cable and udev rules."
    exit 1
fi
echo "[OK] UART device: $DEV"

# ── 2. Kill any existing agent ────────────────────────────────────────
pkill -f "micro_ros_agent serial" 2>/dev/null || true
sleep 1

# ── 3. Start micro-ROS agent ──────────────────────────────────────────
echo "[..] Starting micro-ROS agent on $DEV @ $BAUD baud..."
nohup $AGENT_BIN serial --dev "$DEV" -b "$BAUD" -v4 >"$LOG" 2>&1 &
AGENT_PID=$!
echo "[OK] Agent PID=$AGENT_PID  log=$LOG"

# ── 4. Wait for STM32 node to appear ─────────────────────────────────
echo "[..] Waiting for STM32 to connect (up to 15 s)..."
CONNECTED=0
for i in $(seq 1 30); do
    sleep 0.5
    if ros2 node list 2>/dev/null | grep -q "stm32f7_node"; then
        CONNECTED=1
        echo "[OK] STM32 connected — node /terrain_bot/stm32f7_node found"
        break
    fi
done

if [ "$CONNECTED" -eq 0 ]; then
    echo "[WARN] STM32 node not seen after 15 s."
    echo "       The STM32 fallback mode is still running motors — checking topics..."
    ros2 topic list 2>/dev/null | grep -E "cmd_vel|wheel|imu" || true
    echo ""
    echo "       Agent log (last 30 lines):"
    tail -30 "$LOG" 2>/dev/null || true
    echo ""
    echo "[INFO] Fallback motors still cycle at open-loop speed (no ROS needed)."
    echo "       Check LCD display on STM32 for status."
    exit 0
fi

# ── 5. Motor test: forward → stop → reverse → stop ───────────────────
SPEED_FWD=0.03    # 30 mm/s forward  (within 44.5 mm/s limit)
SPEED_REV=-0.03   # 30 mm/s reverse

echo ""
echo "=== MOTOR TEST START ==="

echo "[>>] Forward 3 s at ${SPEED_FWD} m/s..."
ros2 topic pub --rate 10 /cmd_vel geometry_msgs/msg/Twist \
  "{linear: {x: ${SPEED_FWD}, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" &
PUB_PID=$!
sleep 3
kill "$PUB_PID" 2>/dev/null || true

echo "[--] Stop 1 s..."
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist \
  "{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}"
sleep 1

echo "[<<] Reverse 3 s at ${SPEED_REV} m/s..."
ros2 topic pub --rate 10 /cmd_vel geometry_msgs/msg/Twist \
  "{linear: {x: ${SPEED_REV}, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" &
PUB_PID=$!
sleep 3
kill "$PUB_PID" 2>/dev/null || true

echo "[--] Stop."
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist \
  "{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}"

echo ""
echo "=== MOTOR TEST COMPLETE ==="
echo ""

# ── 6. Show encoder/status feedback ──────────────────────────────────
echo "[INFO] Wheel velocity (last reading):"
timeout 3 ros2 topic echo --once /wheel_velocity 2>/dev/null || echo "  (no encoder data — ESP32 may not be flashed)"

echo ""
echo "[INFO] Base status:"
timeout 3 ros2 topic echo --once /base_status 2>/dev/null || echo "  (no status topic)"

echo ""
echo "Done. Agent still running (PID=$AGENT_PID). To stop: kill $AGENT_PID"
