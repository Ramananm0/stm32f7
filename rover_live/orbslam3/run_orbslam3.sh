#!/bin/bash
# Run ORB-SLAM3 monocular on Pi with Pi Camera Module 3
# Run ON THE PI after install_orbslam3_pi.sh completes

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
VOCAB=/home/ubuntu/ORB_SLAM3/Vocabulary/ORBvoc.txt
SETTINGS="$SCRIPT_DIR/cam3_calibration.yaml"

source /opt/ros/humble/setup.bash
source /home/ubuntu/orbslam_ws/install/setup.bash

echo "=== Starting Pi Camera Module 3 ==="
# Launch camera node (libcamera → /camera/image_raw)
ros2 run camera_ros camera_node \
    --ros-args \
    -p width:=1280 \
    -p height:=720 \
    -p framerate:=30.0 \
    &
CAM_PID=$!
sleep 3

echo "=== Starting ORB-SLAM3 Monocular ==="
ros2 run orbslam3_ros2 mono \
    "$VOCAB" "$SETTINGS" \
    --ros-args \
    --remap image_raw:=/camera/image_raw

# Cleanup
kill $CAM_PID 2>/dev/null
