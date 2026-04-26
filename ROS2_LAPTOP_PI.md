# Laptop <-> Pi ROS 2

This repo's robot runtime is split like this:

- Raspberry Pi `100.88.254.75` runs the robot-side ROS 2 stack
- Raspberry Pi also runs the `micro_ros_agent` process that bridges STM32 `USART6`
  into ROS 2
- laptop runs operator tools such as `rviz2`, topic inspection, and teleop

## STM32 <-> Pi link

- STM32 UART: `USART6`
- Baud: `115200`
- STM32 pins:
  - `D1 / PC6 = USART6_TX`
  - `D0 / PC7 = USART6_RX`
- TTL cross wiring to Pi-side USB serial adapter:
  - adapter `RX -> STM32 D1 / PC6`
  - adapter `TX -> STM32 D0 / PC7`
  - `GND -> GND`

## Current state

- Pi LiDAR is publishing `/scan` at about `7.6 Hz`
- STM32 micro-ROS is connected and publishing `/imu/data`, `/wheel_ticks`, and `/base_status`
- camera is not publishing because `Picamera2()` currently sees no attached camera on the Pi

## Laptop setup

Use the repo-local Fast DDS peer profile:

```bash
source /home/ramana/stm32f7/ros2/use_pi_fastdds.sh
ros2 daemon stop
ros2 daemon start
ros2 topic list
```

If discovery succeeds, run:

```bash
rviz2
ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args -r /cmd_vel:=/cmd_vel_raw
```

## Pi-side checks

```bash
ssh ubuntu@100.88.254.75
source /opt/ros/humble/setup.bash
source /home/ubuntu/raspberry_pi4b/ros2_ws/install/setup.bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/stm32 -b 115200
ros2 topic hz /scan
ros2 topic info /camera/image_raw
journalctl -u micro-ros-agent.service -n 40 --no-pager
```

## Camera failure currently observed

The camera node now imports `libcamera`, but fails here:

```text
Picamera2() -> IndexError: list index out of range
```

That means the Pi camera stack currently sees zero cameras. This is no longer a Python dependency problem; it is now a camera detection problem on the Pi.
