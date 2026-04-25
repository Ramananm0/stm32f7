#!/bin/bash
# Run this ON THE PI after boot to ensure all services are up.
# ssh ubuntu@100.88.254.75 "bash ~/rover_live/pi/start_pi.sh"

echo "=== Pi Rover Services ==="

check() {
    local name=$1
    if systemctl is-active --quiet "$name"; then
        echo "  [OK]  $name"
    else
        echo "  [DOWN] $name — restarting..."
        sudo systemctl restart "$name"
        sleep 2
        systemctl is-active --quiet "$name" && echo "  [OK]  $name restarted" || echo "  [FAIL] $name failed to start"
    fi
}

check micro-ros-agent
check rplidar
check terrain-bringup 2>/dev/null || true

echo ""
echo "=== Serial Devices ==="
ls -la /dev/stm32 /dev/rplidar 2>/dev/null || echo "Check USB connections!"

echo ""
echo "=== Active ROS Topics ==="
source /opt/ros/humble/setup.bash
source ~/microros_ws/install/setup.bash
timeout 5 ros2 topic list 2>/dev/null | grep -E "imu|wheel|base|scan|odom|cmd" || echo "No topics yet"

echo ""
echo "Pi ready."
