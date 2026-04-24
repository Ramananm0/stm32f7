# CLAUDE.md

## Project Summary

This workspace is a terrain-robot development workspace centered on an STM32F746G-DISCO firmware project, with supporting ROS 2 simulation and Raspberry Pi runtime reference repos.

The top-level workspace contains:

- `stm32f7disco/`: main STM32 firmware project
- `_terrain_ws_check/`: ROS 2 simulation and terrain-risk evaluation workspace
- `_terrain_bot_source/`: reference STM32/ESP32 source repo
- `_raspberry_pi4b_check/`: Raspberry Pi ROS 2 runtime stack
- `pi_*.sh`: helper scripts for remote Pi inspection, rebuild, service management, and bringup
- `prompts/`, `data/`, `agents/`, `evals/`: workspace sections requested for prompt/data/agent/eval organization

The parent repository tracks the workspace layout and the STM32 firmware tree. The `_terrain_ws_check`, `_terrain_bot_source`, and `_raspberry_pi4b_check` directories remain separate nested repositories.

## High-Level Goal

The system is a four-wheel terrain robot with this intended split:

- STM32:
  - low-level hardware initialization
  - motor control
  - IMU acquisition
  - encoder acquisition
  - safety/risk computation
  - LCD status display
  - micro-ROS transport over UART
- Raspberry Pi:
  - micro-ROS agent
  - LiDAR driver
  - wheel odometry / EKF / SLAM
  - terrain analysis / AI supervisor nodes
  - robot-side bringup services
- Ubuntu laptop or workstation:
  - RViz
  - map visualization
  - planning
  - operator control and debugging

## Project Flow

### 1. Firmware startup flow

Main firmware entry is `stm32f7disco/Src/main.c`.

Startup sequence:

1. Configure MPU
2. Run `HAL_Init()`
3. Configure clocks in `SystemClock_Config()`
4. Initialize peripherals:
   - GPIO
   - I2C1
   - TIM1, TIM3, TIM10, TIM11, TIM12, TIM13, TIM14
   - USART6
   - DMA2D
   - BSP LCD init
   - TIM2
5. Enter `App_Run()` from `stm32f7disco/Src/app.c`

Important conditional paths in `main.c`:

- `BSP_LCD_TEST_MODE` runs LCD BSP tests instead of the robot app
- `IMU_MIN_TEST_MODE` runs a minimal IMU test path
- `LCD_TEST_MODE` runs LCD test mode
- `MOTOR_ESP32_DIAG_MODE` runs `App_MotorEsp32Diag_Run()`
- otherwise normal runtime enters `App_Run()`

### 2. STM32 application flow

`App_Run()` is the main firmware loop.

Initialization inside `App_Run()`:

1. Initialize LCD dashboard
2. Initialize ICM20948 IMU over I2C
3. Initialize encoder interface over I2C
4. Initialize motor PWM outputs
5. Initialize Madgwick attitude filter
6. Optionally initialize micro-ROS transport and node setup if `USE_MICROROS` is enabled
7. Run IMU calibration
8. Seed attitude filter from accelerometer
9. Start the main timed loop

Main loop timing from the current `stm32f7disco/Src/app.c`:

- IMU update: 5 ms
- Encoder update: 20 ms
- Motor/PID update: 10 ms
- IMU publish: 100 ms
- Encoder publish: 200 ms
- LCD refresh: 50 ms
- Watchdog/health timing: 2000 ms

Older notes in `STM32F7_Terrain_Bot_Detailed_Notes.txt` describe an earlier timing layout. For current behavior, trust the constants in `stm32f7disco/Src/app.c`.

### 3. STM32 control/data flow

Core runtime flow on the MCU:

1. Read IMU from `icm20948.c`
2. Read encoder data from `encoder.c`
3. Estimate roll/pitch/yaw with `madgwick.c`
4. Compute terrain/safety risk inside `app.c`
5. Scale or stop motor output if unsafe
6. Drive motors through `motor.c`
7. Update LCD via `lcd_display.c`
8. Exchange ROS-facing messages over UART transport in `microros_transport.c`

Primary firmware modules:

- `Src/app.c`: application loop, safety logic, micro-ROS orchestration
- `Src/motor.c`: motor speed to PWM mapping
- `Src/encoder.c`: wheel encoder I2C interface
- `Src/icm20948.c`: IMU driver
- `Src/madgwick.c`: orientation estimation
- `Src/microros_transport.c`: UART micro-ROS transport
- `Src/lcd_display.c`: status/dashboard rendering

### 4. ROS 2 / Pi flow

There are two separate Pi-side references in this workspace and they should not be mixed up:

- `UBUNTU_HANDOFF.md`: current top-level handoff for the live STM32 + Pi debugging effort
- `_raspberry_pi4b_check/README.md`: a separate Raspberry Pi reference repo with a broader ROS 2 stack description

The current STM32 code and the top-level handoff are aligned on:

- `USART6`
- `PC6 = TX`
- `PC7 = RX`
- baud `115200`

The `_raspberry_pi4b_check` repo documents a different runtime snapshot and mentions `2 Mbaud`. Treat that repo as reference material, not as the authoritative current STM32 configuration.

The intended robot-side runtime flow from the handoff and current firmware is:

1. STM32 publishes wheel and IMU-related data over UART
2. Raspberry Pi runs `micro_ros_agent`
3. Pi exposes ROS 2 topics such as:
   - `/wheel_ticks`
   - `/wheel_velocity`
   - `/base_status`
   - `/imu/data` when working correctly
4. Pi also subscribes to `/cmd_vel`
5. Additional Pi-side nodes handle:
   - LiDAR
   - wheel odometry
   - EKF
   - SLAM
   - terrain risk
   - data logging

From the handoff notes, a known blocker was:

- `/imu/data` was not publishing correctly from the STM side

### 5. Simulation flow

The simulation workspace under `_terrain_ws_check/` models the same robot and terrain-risk pipeline in ROS 2 / Gazebo.

Main packages:

- `terrain_ai`: AI/image/path layer for terrain decisions
- `terrain_bot_description`: URDF/xacro robot model
- `terrain_bot_gazebo`: Gazebo worlds and simulation launcher
- `terrain_bot_bringup`: SLAM, RViz, launch orchestration
- `terrain_risk_layer`: terrain risk node and CSV logger

Typical simulation flow:

1. Launch Gazebo world
2. Spawn robot model
3. Bridge simulator topics to ROS 2
4. Run `terrain_risk_node`
5. Run `terrain_ai_node` and `terrain_path_node`
6. Feed raw teleop/planner/path commands into `/cmd_vel_raw`
7. `terrain_risk_node` computes risk from IMU + LiDAR and forwards safe commands to `/cmd_vel`
8. SLAM toolbox builds the map
9. `data_logger` writes CSV logs

## Development Workflow

### A. Workspace workflow

When changing this project:

1. Decide which layer you are working on:
   - STM32 firmware
   - Pi runtime
   - ROS 2 simulation
   - helper scripts / deployment flow
2. Avoid mixing generated artifacts into source commits
3. Preserve nested repos as separate repos unless explicitly restructuring
4. Use the top-level docs first before changing behavior

### B. Firmware workflow

For STM32 firmware work:

1. Read:
   - `STM32F7_Terrain_Bot_Detailed_Notes.txt`
   - `UBUNTU_HANDOFF.md`
   - `stm32f7disco/MICROROS_INTEGRATION.md`
2. Identify the affected module in `stm32f7disco/Src` and `stm32f7disco/Inc`
3. Confirm whether the change affects:
   - IMU path
   - encoder path
   - motor safety logic
   - LCD behavior
   - micro-ROS transport
4. Keep CubeMX-generated initialization files stable unless hardware config actually changes
5. Prefer targeted edits in application-layer files over broad generated-file changes

### C. Build workflow

The STM board does not need to be connected to compile firmware.

Build prerequisites:

- STM32CubeIDE toolchain is installed locally
- `arm-none-eabi-gcc.exe` exists under `C:\ST\STM32CubeIDE_1.18.1\...`
- `make.exe` exists under `C:\ST\STM32CubeIDE_1.18.1\...`

Important distinction:

- board connection is needed for flashing/testing on hardware
- board connection is not needed for local compilation

If building from shell, use the full toolchain paths or add them to `PATH` first.

### D. micro-ROS workflow

micro-ROS in this firmware is optional and guarded by `USE_MICROROS`.

Two operating modes matter:

1. Default build:
   - firmware builds without micro-ROS dependencies
   - robot logic, IMU, encoders, motors, LCD remain active
2. micro-ROS enabled build:
   - requires generated micro-ROS static library and include paths
   - requires correct UART transport path
   - used for live STM32 <-> Pi ROS 2 integration

### E. Pi / bringup workflow

The helper scripts at workspace root are mainly for Raspberry Pi operations over SSH.

Common tasks handled by these scripts:

- inspect services
- rebuild workspace
- patch bringup
- verify topics
- test motors carefully
- inspect serial agent state

Use these scripts as operational tools, not as the primary source of architecture truth.

### F. Simulation / evaluation workflow

Use `_terrain_ws_check/` when:

- validating terrain-risk logic in simulation
- checking world behavior
- testing SLAM + risk + logging
- testing the AI/path layer that publishes to `/cmd_vel_raw`
- reviewing package architecture

Typical evaluation path:

1. Build the ROS 2 workspace
2. Launch `terrain_bot_bringup`
3. Drive with teleop into different worlds
4. Inspect:
   - `/terrain_risk`
   - `/speed_scale`
   - `/roll_deg`
   - `/pitch_deg`
5. Review generated CSV logs

## Current Known State

- Top-level workspace is pushed to `https://github.com/Ramananm0/stm32f7.git`
- Requested folders `prompts/`, `data/`, `agents/`, and `evals/` exist
- Graphify is installed as Python package `graphifyy`
- Graphify Codex integration has been installed
- STM32 toolchain binaries exist locally under `C:\ST\STM32CubeIDE_1.18.1\...`
- The main live-system issue noted in the handoff is missing STM-side `/imu/data`
- Current STM32 code is configured for `115200` baud on `USART6`
- `_raspberry_pi4b_check` contains useful reference material but is not the authoritative source for the current STM32 UART configuration

## Practical Rules For Future Work

1. Read the firmware notes and handoff docs before changing control logic.
2. Treat `stm32f7disco/Src/app.c` as the main behavior file and trust its constants over older prose notes when they differ.
3. Do not assume the Pi issue is the same as the STM issue; keep the boundary clear.
4. Do not flatten nested repos into the parent repo accidentally.
5. Separate build problems from hardware-connection problems.
6. Prefer small, traceable firmware edits and verify impact on data flow.
7. When documenting or debugging, always state whether the context is:
   - simulation
   - STM32 firmware only
   - live STM32 + Pi integration

## Fast Orientation For A New Session

If starting fresh, read in this order:

1. `CLAUDE.md`
2. `STM32F7_Terrain_Bot_Detailed_Notes.txt`
3. `UBUNTU_HANDOFF.md`
4. `stm32f7disco/MICROROS_INTEGRATION.md`
5. `_terrain_ws_check/README.md`
6. `_terrain_ws_check/FILES.md`

That gives the runtime architecture, current blocker, firmware responsibilities, and simulation structure quickly.
