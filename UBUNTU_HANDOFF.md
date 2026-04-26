# Ubuntu Handoff

## Current status

- STM32 board: `STM32F746G-DISCO`
- STM firmware project: [`stm32f7disco`](./stm32f7disco)
- Raspberry Pi address used for remote work: `100.88.254.75`
- Pi user: `ubuntu`
- UART link between STM and Pi is working through micro-ROS at `115200`

## What is working now

- Dedicated Pi service for micro-ROS agent is running:
  - `micro-ros-agent.service`
- Main Pi ROS bringup is running:
  - `terrain-bringup.service`
- STM micro-ROS session can establish successfully
- STM topics seen on Pi:
  - `/wheel_ticks`
  - `/wheel_velocity`
  - `/base_status`
- STM subscribes to:
  - `/cmd_vel`
- Pi-side nodes confirmed running:
  - `rplidar_node`
  - `terrain_odom`
  - `ekf_filter_node`
  - `slam_toolbox`
  - `terrain_traversability`
  - `terrain_risk_node`
  - `data_logger`

## Known remaining issue

- `/imu/data` is not publishing currently from STM
- This is now the main functional blocker on the robot side
- Pi communication is no longer the main issue

## Important architecture decision

The Raspberry Pi is too resource-constrained for full planning plus visualization.

Recommended split:

- STM32:
  - IMU read
  - wheel encoder read
  - motor control
  - micro-ROS transport
- Raspberry Pi:
  - micro-ROS agent
  - lidar
  - wheel odometry
  - EKF
  - SLAM
  - terrain risk / local safety
- Ubuntu laptop:
  - RViz
  - map visualization
  - waypoint planning
  - global planning / Nav2 or custom planner
  - send goals or velocity commands back to Pi over ROS 2 network

SSH should be used for:

- login
- debugging
- restarting services
- copying files

ROS data should flow over ROS 2 networking, not through SSH forwarding.

## Raspberry Pi fixes already made

Earlier, the agent repeatedly crashed when launched inside the main bringup path. The stable fix was:

1. move micro-ROS agent into its own systemd service
2. remove agent launch responsibility from the main bringup path
3. keep bringup and agent independent

Current Pi services:

- `micro-ros-agent.service`
- `terrain-bringup.service`

## Raspberry Pi service details

Micro-ROS service was installed as a dedicated systemd unit on the Pi.

Expected checks on Pi:

```bash
systemctl status micro-ros-agent.service
systemctl status terrain-bringup.service
```

Useful ROS checks on Pi:

```bash
source /opt/ros/humble/setup.bash
source ~/microros_ws/install/local_setup.bash
source ~/raspberry_pi4b/ros2_ws/install/setup.bash

ros2 topic list
ros2 topic info -v /wheel_ticks
ros2 topic info -v /imu/data
ros2 topic info -v /cmd_vel
ros2 node list
```

## UART / micro-ROS details

- STM UART baud: `115200`
- STM transport in source: `USART6`
- STM UART pins:
  - `D1 / PC6 = USART6_TX`
  - `D0 / PC7 = USART6_RX`
- Correct TTL cross wiring:
  - adapter `RX` -> STM `D1`
  - adapter `TX` -> STM `D0`
  - `GND` -> `GND`

## Important STM project edits already made

These edits were made locally in this workspace:

- `.cproject`: BSP include paths fixed for LCD/BSP headers
- `lcd_display.c`:
  - fixed float-format warning
  - fixed invisible IMU/MOTOR button repaint bug
  - added temporary ROS debug display
- `app.c`:
  - added extra micro-ROS debug counters and retry tracking
  - adjusted ping timing
- `microros_transport.c`:
  - moved to interrupt-backed UART RX handling
  - transport debugging added
- `usart.c`, `stm32f7xx_it.c`, `stm32f7xx_it.h`:
  - enabled `USART6` interrupt path

Some of these are debug-oriented and may be cleaned later after the system is stable.

## Paths in this workspace

- STM project:
  - [`stm32f7disco`](./stm32f7disco)
- Local Pi reference repo copy:
  - [`_raspberry_pi4b_check`](./_raspberry_pi4b_check)
- Local helper scripts used during debugging:
  - [`pi_check_agent.sh`](./pi_check_agent.sh)
  - [`pi_uart_probe.sh`](./pi_uart_probe.sh)
  - [`pi_agent_probe.sh`](./pi_agent_probe.sh)

## Recommended Ubuntu 22.04 next steps

1. Install ROS 2 Humble on Ubuntu laptop
2. Put laptop and Pi on the same ROS 2 network
3. Match `ROS_DOMAIN_ID` on both machines
4. Verify laptop can see Pi topics:

```bash
ros2 topic list
```

5. Start with laptop-side tools:
  - `rviz2`
  - topic inspection
  - map visualization
  - teleop or goal sending
6. Then add planning:
  - waypoint planner
  - Nav2 or custom terrain planner

## Best immediate technical next step

Before full planning, fix STM IMU publishing:

- verify IMU driver read path on STM
- verify STM actually publishes `/imu/data`
- then re-check EKF and terrain nodes on Pi

Without `/imu/data`, the robot stack is only partially complete.

## If continuing from Ubuntu

First checks to run:

```bash
ssh ubuntu@100.88.254.75
systemctl status micro-ros-agent.service
systemctl status terrain-bringup.service
```

Then:

```bash
source /opt/ros/humble/setup.bash
ros2 topic list
```

Then confirm:

- `/wheel_ticks` present
- `/wheel_velocity` present
- `/base_status` present
- `/cmd_vel` present
- `/imu/data` status

## Summary

- Pi stack is working and stable enough for robot-side runtime
- full planning should move to Ubuntu laptop
- SSH is for management, not the main data path
- ROS 2 networking should connect laptop and Pi
- main remaining robot-side issue is STM IMU publishing
