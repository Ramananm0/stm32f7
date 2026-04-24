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

## UART

The current CubeMX project configures USART6 on:

```text
PC6 = USART6_TX
PC7 = USART6_RX
Baud = 115200
```

Run the Raspberry Pi / host agent with the same baud rate unless CubeMX is changed.

```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 -b 115200
```

