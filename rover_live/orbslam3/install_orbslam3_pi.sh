#!/bin/bash
# Install ORB-SLAM3 + ROS2 wrapper on Raspberry Pi 4B
# Camera: Pi Camera Module 3 (Sony IMX708, CSI)
# Mode: Monocular (or Monocular-Inertial with ICM20948 IMU from STM32)
# Run ON THE PI: bash ~/rover_live/orbslam3/install_orbslam3_pi.sh

set -e
echo "=== ORB-SLAM3 Pi Install ==="

# --- Dependencies ---
echo "[1/6] Installing dependencies..."
sudo apt-get update -qq
sudo apt-get install -y \
    cmake git \
    libglew-dev libboost-all-dev libssl-dev \
    libpython3-dev python3-dev \
    libeigen3-dev \
    libopencv-dev \
    libpangolin-dev 2>/dev/null || true

# Pangolin (if not via apt)
if ! pkg-config --exists pangolin 2>/dev/null; then
    echo "[1b] Building Pangolin..."
    cd /tmp
    git clone --depth 1 https://github.com/stevenlovegrove/Pangolin.git
    mkdir -p Pangolin/build && cd Pangolin/build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j2
    sudo make install
    cd /home/ubuntu
fi

# --- ORB-SLAM3 ---
echo "[2/6] Cloning ORB-SLAM3..."
cd /home/ubuntu
git clone --depth 1 https://github.com/UZ-SLAMLab/ORB_SLAM3.git

echo "[3/6] Building ORB-SLAM3 (takes 10-15 min on Pi)..."
cd /home/ubuntu/ORB_SLAM3
chmod +x build.sh
# Limit to 2 jobs so Pi doesn't OOM
sed -i 's/make -j/make -j2/' build.sh
./build.sh

# --- ROS2 Wrapper ---
echo "[4/6] Installing ROS2 ORB-SLAM3 wrapper..."
mkdir -p /home/ubuntu/orbslam_ws/src
cd /home/ubuntu/orbslam_ws/src
git clone --depth 1 https://github.com/zang09/ORB-SLAM3-ROS2.git orbslam3_ros2

echo "[5/6] Building ROS2 wrapper..."
cd /home/ubuntu/orbslam_ws
source /opt/ros/humble/setup.bash
colcon build --symlink-install --cmake-args \
    -DORB_SLAM3_DIR=/home/ubuntu/ORB_SLAM3 \
    -DCMAKE_BUILD_TYPE=Release

# --- Camera (Pi Camera Module 3 via libcamera) ---
echo "[6/6] Installing libcamera ROS2 bridge..."
sudo apt-get install -y libcamera-dev libcamera-apps 2>/dev/null || true
cd /home/ubuntu/orbslam_ws/src
git clone --depth 1 https://github.com/christianrauch/camera_ros.git || true
cd /home/ubuntu/orbslam_ws
colcon build --symlink-install --packages-select camera_ros 2>/dev/null || true

echo ""
echo "=== ORB-SLAM3 install complete ==="
echo "Next step: calibrate Pi Camera Module 3"
echo "Run: bash ~/rover_live/orbslam3/calibrate_camera.sh"
