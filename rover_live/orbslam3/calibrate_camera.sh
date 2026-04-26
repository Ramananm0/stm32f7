#!/bin/bash
# Calibrate Pi Camera Module 3 for ORB-SLAM3
# Run ON THE PI with a 9x6 checkerboard (print one from opencv docs)
# Result updates cam3_calibration.yaml

echo "=== Pi Camera Module 3 Calibration ==="
echo ""
echo "You need a 9x6 checkerboard pattern."
echo "Print from: https://calib.io/pages/camera-calibration-pattern-generator"
echo "Or use OpenCV sample: opencv/doc/pattern.png"
echo ""
echo "Hold the checkerboard in front of the camera and move it around."
echo "Collect ~30 images from different angles."
echo ""

source /opt/ros/humble/setup.bash

# Start camera
ros2 run camera_ros camera_node \
    --ros-args -p width:=1280 -p height:=720 &
CAM_PID=$!
sleep 2

# Run calibration
ros2 run camera_calibration cameracalibrator \
    --size 9x6 \
    --square 0.025 \
    --ros-args \
    --remap image:=/camera/image_raw \
    --remap camera:=/camera

kill $CAM_PID 2>/dev/null
echo ""
echo "After calibration, copy fx,fy,cx,cy,k1,k2,p1,p2 into cam3_calibration.yaml"
