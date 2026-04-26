# Rover Live — Launch & SLAM

## Folder Structure

```
rover_live/
├── laptop/
│   ├── start.sh          ← Run this on laptop to launch RViz + Teleop + Monitor
│   └── rviz_rover.rviz   ← RViz config (odom frame, map, lidar, odometry)
├── pi/
│   └── start_pi.sh       ← Run on Pi to check/restart all services
└── orbslam3/
    ├── install_orbslam3_pi.sh   ← One-time install on Pi
    ├── calibrate_camera.sh      ← Calibrate Pi Camera Module 3
    ├── cam3_calibration.yaml    ← Camera params for ORB-SLAM3
    └── run_orbslam3.sh          ← Run ORB-SLAM3 monocular on Pi
```

---

## Quick Start (SLAM Toolbox — works now)

### 1. Boot Pi, check services
```bash
ssh ubuntu@100.88.254.75
bash ~/rover_live/pi/start_pi.sh
```

### 2. Launch laptop UI
```bash
bash ~/stm32f7/rover_live/laptop/start.sh
```

This opens 3 windows:
- **RViz** — map + lidar + odometry
- **Teleop** — keyboard drive (i=fwd, k=stop, j/l=turn)
- **Monitor** — live status feed

---

## ORB-SLAM3 with Pi Camera Module 3 (setup required)

### Step 1 — Install on Pi (one time, ~20 min)
```bash
ssh ubuntu@100.88.254.75
bash ~/rover_live/orbslam3/install_orbslam3_pi.sh
```

### Step 2 — Calibrate camera (one time)
```bash
bash ~/rover_live/orbslam3/calibrate_camera.sh
```
Use a printed 9×6 checkerboard. Move it around, collect 30 frames, save.

### Step 3 — Run ORB-SLAM3
```bash
bash ~/rover_live/orbslam3/run_orbslam3.sh
```

### Step 4 — View in RViz on laptop
- In RViz, Camera display is already added (disabled by default)
- Enable "Camera (ORB-SLAM3)" display in RViz Displays panel
- ORB-SLAM3 publishes `/orb_slam3/map_points` for point cloud view

---

## Camera Wiring — Pi Camera Module 3

- Connect via **CSI ribbon cable** to Pi 4B CSI-2 port (next to HDMI ports)
- No USB needed — direct CSI interface
- Enable with: `sudo raspi-config → Interface Options → Camera → Enable`
- Test: `libcamera-hello` should show preview

---

## SLAM Toolbox vs ORB-SLAM3

| Feature | SLAM Toolbox (current) | ORB-SLAM3 |
|---------|----------------------|-----------|
| Sensor | LiDAR (/scan) | Camera Module 3 |
| Map type | 2D occupancy grid | Sparse 3D point cloud |
| Loop closure | Yes | Yes |
| Works now | **Yes** | After install |
| Best for | Navigation, path planning | Visual odometry, 3D mapping |
| Can fuse IMU | No | Yes (monocular-inertial) |
