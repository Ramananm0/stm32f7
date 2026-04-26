# micro-ROS integration status

The terrain bot firmware is structured so it builds without micro-ROS by default.
The ROS code is still present in `Src/app.c` and `Src/microros_transport.c`, guarded by:

```c
#ifdef USE_MICROROS
```

## Current build mode

Default CubeIDE build:

- IMU driver enabled
- ESP32 encoder reader enabled
- motor driver enabled
- terrain safety layer enabled
- LCD dashboard enabled
- micro-ROS disabled

This mode builds cleanly without `rcl`, `rclc`, `rmw_microros`, `uxr`, or message headers.

## To enable micro-ROS

1. Install Docker Desktop.
2. Add the official `micro_ros_stm32cubemx_utils` package to the project.
3. Generate the STM32 static library for Cortex-M7 hard-float.
4. Add the generated include path to CubeIDE:

```text
Middlewares/micro_ros_stm32cubemx_utils/microros_static_library/libmicroros/include
```

5. Add the generated library path to CubeIDE:

```text
Middlewares/micro_ros_stm32cubemx_utils/microros_static_library/libmicroros
```

6. Link:

```text
libmicroros.a
```

7. Add this compiler symbol:

```text
USE_MICROROS
```

8. Rebuild.

## Host transport status

The checked-in firmware currently binds the micro-ROS custom transport to
`USART6` in [Src/app.c](Src/app.c) and uses the UART interrupt transport in
`Src/microros_transport.c`.

The Raspberry Pi side can already consume either:

- native USB CDC ACM as `/dev/ttyACM*`
- the older USB-TTL adapter as `/dev/ttyUSB*`

Both should map to `/dev/stm32` through udev.

## USB CDC note

Native USB CDC is not a one-line switch in this repo's current STM32 project.
Two concrete gaps remain:

- the CubeMX project does not currently have the USB device middleware checked in
- `PA11` and `PA12` are assigned to LTDC in `stm32f7disco.ioc`, which conflicts
  with USB FS on STM32F746

So the Pi-side repo can be prepared for CDC now, but the STM32 firmware cannot
truthfully expose a CDC device from this repository alone until those two
project-level issues are resolved.

## UART

The current CubeMX project configures USART6 on:

```text
PC6 = USART6_TX
PC7 = USART6_RX
Baud = 115200
```

Run the Raspberry Pi / host agent with the same baud rate unless CubeMX is changed.
If `/dev/stm32` points to a `ttyACM` CDC device, the agent still opens it via
the serial backend; the baud setting is simply retained for CLI compatibility.

```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/stm32 -b 115200
```
