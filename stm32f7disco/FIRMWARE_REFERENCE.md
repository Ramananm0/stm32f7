# STM32F746G-DISCO Terrain Rover — Firmware Reference

This file is a technical reference for Claude and developers. It captures the full
hardware mapping, data flow, known issues, and fixes applied so far.

---

## 1. Hardware Overview

- **MCU**: STM32F746NG on STM32F746G-DISCO board
- **Motor driver**: 2× H-bridge (IBT-2 or equivalent), RPWM/LPWM control
- **Encoder slave**: ESP32 (I2C address 0x30) running PCNT quadrature counting
- **IMU**: ICM20948 on I2C1 (address 0x68 or 0x69)
- **UART**: USART6 @ 115200 baud — PC6=TX, PC7=RX — micro-ROS transport to Pi
- **Display**: LCD on BSP (built-in 4.3" touchscreen)
- **Raspberry Pi 4B**: runs micro_ros_agent, LiDAR, EKF, SLAM, terrain_risk_node

---

## 2. Motor → Timer → PWM Channel Mapping

Each motor is an H-bridge controlled by two PWM signals: RPWM (forward) and LPWM (reverse).
Only one signal is active at a time. Setting the active channel to 0 is coast/brake.

| Motor | ID | FWD timer/ch | REV timer/ch | ARR  | PWM freq |
|-------|----|--------------|--------------|------|----------|
| FL    | 0  | TIM10 CH1    | TIM11 CH1    | 10799| 20 kHz   |
| FR    | 1  | TIM14 CH1    | TIM13 CH1    | 5399 | 40 kHz   |
| RL    | 2  | TIM12 CH2    | TIM12 CH1    | 5399 | 40 kHz   |
| RR    | 3  | TIM2  CH1    | TIM3  CH1    | 5399 | 40 kHz   |

**Important direction note for FR and RR**: Their FWD/REV channels are swapped relative to
FL and RL because the right-side motors are mounted mirrored. In `write_motor_channels()`:
- FR forward = TIM14 active (not TIM13)
- RR forward = TIM2 active (not TIM3)

**Important fix applied**: FL and RL had their FWD/REV channels swapped in code to match
physical wiring. See `motor.c:write_motor_channels()`.

---

## 3. PWM GPIO Pin Assignments

These are the physical STM32 pins that carry PWM to the H-bridges:

| Motor | Direction     | Timer     | GPIO Pin | Notes                        |
|-------|---------------|-----------|----------|------------------------------|
| FL    | FORWARD RPWM  | TIM10 CH1 | PF6      | CN4 on Discovery board       |
| FL    | REVERSE LPWM  | TIM11 CH1 | PF7      | CN4                          |
| FR    | FORWARD RPWM  | TIM14 CH1 | PF9      | CN4                          |
| FR    | REVERSE LPWM  | TIM13 CH1 | PF8      | CN4                          |
| RL    | FORWARD RPWM  | TIM12 CH2 | PB15     | CN7                          |
| RL    | REVERSE LPWM  | TIM12 CH1 | PH6      | CN8                          |
| RR    | FORWARD RPWM  | TIM2  CH1 | PA15     | CN4                          |
| RR    | REVERSE LPWM  | TIM3  CH1 | PB4      | CN7                          |

**H-bridge checklist (IBT-2 / BTS7960):**
- R_EN and L_EN must be pulled HIGH (5V or 3.3V) — if floating, output is disabled
- Motor power rail (12V/24V) must be present — logic 3.3V alone won't drive motors
- Verify each PWM pin is connected from Discovery connector → H-bridge RPWM/LPWM

---

## 4. PWM Duty Calculation

`Motor_Set(id, vel_mmps)` path:

```
abs_vel = |vel_mmps| clamped to MOTOR_MAX_SPEED_MMPS (44.5 mm/s)
max_duty = timer_safe_max_duty(timer)           → ARR * 85 / 100
         (after MOTOR_COMMAND_DUTY_LIMIT_PCT)   → max_duty * 100 / 100  [= unchanged at 100%]
duty = (abs_vel / 44.5) * max_duty
duty = apply_trim(id, duty)                     → duty * trim[id] / 100
duty = ramp_duty(current, duty)                 → step limited to MOTOR_RAMP_STEP per PID cycle
```

**Effective max duty counts at full speed (44.5 mm/s), after trim:**

| Motor | ARR   | safe_max (85%) | trim | final duty | effective % |
|-------|-------|----------------|------|------------|-------------|
| FL    | 10799 | 9179           | 94%  | 8628       | 79.9%       |
| FR    | 5399  | 4589           | 95%  | 4360       | 80.8%       |
| RL    | 5399  | 4589           | 100% | 4589       | 85.0%       |
| RR    | 5399  | 4589           | 98%  | 4497       | 83.3%       |

**Trim rationale**: RL is the physically weakest motor (lowest free-spin speed). Trim is
set so faster motors (FR, RR) are reduced to match RL. RL stays at 100%. Measured free-spin
speeds at 0.04 m/s cmd with final trim: FL≈861, FR≈843, RL≈896, RR≈849 mm/s (~6% spread).

---

## 4. Motor Ramp (Acceleration)

`MOTOR_RAMP_STEP = 200` (counts per 10 ms PID cycle).

Time to reach full speed from stop:

| Motor | final duty | time to full speed |
|-------|------------|--------------------|
| FL    | 8628       | ~431 ms            |
| FR    | 4589       | ~230 ms            |
| RL    | 3717       | ~186 ms            |
| RR    | 4589       | ~230 ms            |

**History**: was 25 counts/cycle → FL took 3.45 seconds, FR/RR 1.83 seconds. Motors
appeared unresponsive under teleop. Fixed to 200.

---

## 5. Encoder → ESP32 → STM32 I2C Protocol

**ESP32 I2C address**: 0x30 (8-bit: 0x60)

**Write (4 bytes)**: permanent per-motor phase-reversal flags sent once at init
```
dirs[0..3] = {1, 0, 0, 1}   // FL and RR have A/B quadrature phase inverted
```
These flags tell ESP32 PCNT to negate the delta for FL and RR. They are PERMANENT
constants, not dynamic direction flags. `Encoder_SetDirection()` is now a no-op.

**Read (36 bytes)**:
```
[0..3]   uint32  status   bit0=esp32_alive, bit1=wheels_valid
[4..19]  int32×4 ticks    cumulative ticks per motor [FL,FR,RL,RR]
[20..35] float×4 vel_mmps velocity in mm/s per motor [FL,FR,RL,RR]
```

**Constants**:
- `ENC_TICKS_PER_REV = 2264`
- `ENC_WHEEL_CIRC_MM = 267.04` (π × 85 mm diameter)
- `ENC_MM_PER_TICK = 0.118 mm`
- `ENC_WHEELBASE_MM = 200 mm`

**FR encoder**: Hardware issue — GPIO pin wiring suspected wrong (GPIO15 vs GPIO19 for
FR_B on ESP32). FR always reads 0. Not blocking; open-loop control still works for 3 wheels.

---

## 6. Application Loop Timing (app.c)

| Task             | Period  | Notes                                       |
|------------------|---------|---------------------------------------------|
| IMU read         | 5 ms    | ICM20948 via I2C1                           |
| Encoder poll     | 20 ms   | ESP32 via I2C1; 500 ms if disconnected      |
| Motor/PID update | 10 ms   | calls motor_control()                       |
| IMU publish      | 100 ms  | /imu/data @ 10 Hz                           |
| Encoder publish  | 200 ms  | /wheel_ticks, /wheel_velocity, /base_status |
| LCD refresh      | 50 ms   |                                             |
| Command watchdog | 2000 ms | g_host_ok=0 if no /cmd_vel for 2s           |
| ROS ping         | 5000 ms | verify agent still alive                    |
| ROS retry        | 500 ms  | re-init micro-ROS if disconnected           |

---

## 7. Motor Control Safety Gate (app.c)

Motor targets are set to 0 and motors stop if ANY of these are true:

1. `g_ros_ok = 0` — micro-ROS not connected to Pi agent
2. `g_host_ok = 0` — no `/cmd_vel` received within 2 seconds
3. `g_emergency_stop = 1` — tilt exceeds threshold (pitch≥45° or roll≥55°)

**g_scale** attenuates motor speed based on tilt risk:
```
g_scale = (1 - g_tilt_risk)²
```
On flat ground: g_tilt_risk≈0 → g_scale≈1.0 (full speed).

**IMU failure behaviour** (fixed):
- Previously: IMU read failure set `g_scale=0.0` → motors stopped hard
- Now: IMU failure sets `g_scale=1.0` (consistent with g_tilt_risk=0, g_risk=0)
- IMU is retried every 500 ms via I2C recover + re-init

---

## 8. micro-ROS Topics

**Published by STM32** (node: `/terrain_bot/stm32f7_node`):
- `/imu/data` — sensor_msgs/Imu @ 10 Hz
- `/wheel_ticks` — std_msgs/Int32MultiArray[4] @ 5 Hz
- `/wheel_velocity` — std_msgs/Float32MultiArray[4] mm/s @ 5 Hz
- `/base_status` — std_msgs/Int32MultiArray[7] @ 5 Hz
  - [0]=esp32_ok [1]=wheels_ok [2]=imu_ok [3]=host_ok [4]=ros_ok [5]=estop [6]=cmd_count

**Subscribed by STM32**:
- `/cmd_vel` — geometry_msgs/Twist

**Transport**: USART6, 115200 baud, custom micro-ROS UART transport
(`Src/microros_transport.c`). Agent runs on Pi at `/dev/stm32`.

---

## 9. Full Command Chain (Laptop → Motors)

```
[Laptop]
  teleop_twist_keyboard
      ↓ /cmd_vel_raw

[Pi — terrain_risk_node]
  applies IMU+LiDAR safety scaling
  scale = (1 - risk)²
      ↓ /cmd_vel

[Pi — micro_ros_agent]
  UART bridge at 115200
      ↓ USART6 serial

[STM32 — cmd_cb()]
  converts Twist to per-wheel mm/s
  vx (m/s) × 1000 = mm/s
  left  = vx - wz × (wheelbase/2)
  right = vx + wz × (wheelbase/2)
      ↓ g_target[FL,RL] = left
        g_target[FR,RR] = right

[STM32 — motor_control() @ 10ms]
  Motor_Set(id, target × g_scale)
      ↓

[STM32 — Motor_Set()]
  duty = (vel / 44.5) × max_duty
  duty = apply_trim(id, duty)
  duty = ramp_duty(current, duty)  ← 200 counts/10ms step
      ↓

[PWM → H-bridge → Motor]
```

**Bypass mode** (direct to STM32, skip terrain_risk_node):
```bash
DIRECT=1 bash ~/stm32f7/rover_live/laptop/start.sh
```

---

## 10. Known Issues and Status

| Issue                           | Status     | Fix                                           |
|---------------------------------|------------|-----------------------------------------------|
| MOTOR_COMMAND_DUTY_LIMIT_PCT=30 | FIXED      | Changed to 100 in motor.h, reflashed          |
| RAMP_STEP=25 (too slow)         | FIXED      | Changed to 200 in motor.h, reflashed          |
| g_scale=0 on IMU read failure   | FIXED      | Changed to g_scale=1.0 in app.c, reflashed    |
| FL/RL FWD/REV channels swapped  | FIXED      | Swapped in motor.c:write_motor_channels()     |
| FL/FR encoder phase inverted    | FIXED      | g_enc_reversed={0,1,0,1} — measured 2026-04-26|
| Encoder_SetDirection resetting  | FIXED      | Motor_StopAll no longer calls SendDirections  |
| FR encoder dead (0 ticks)       | OPEN       | GPIO19 pin suspected wrong, hardware issue    |
| /imu/data not publishing        | OPEN       | IMU initializes but publish path needs verify |
| start.sh FastDDS off by default | FIXED      | Defaulted USE_FASTDDS_PEER=1 in start.sh      |

---

## 11. Build and Flash

```bash
# Build (from Debug/ directory)
make -j$(nproc) -C /home/ramana/stm32f7/stm32f7disco/Debug all

# Generate binary
arm-none-eabi-objcopy -O binary \
    /home/ramana/stm32f7/stm32f7disco/Debug/stm32f7disco.elf \
    /home/ramana/stm32f7/stm32f7disco/Debug/stm32f7disco.bin

# Flash
st-flash --reset write /home/ramana/stm32f7/stm32f7disco/Debug/stm32f7disco.bin 0x08000000
```

USE_MICROROS is defined in `Debug/Src/subdir.mk` as a compiler flag (`-DUSE_MICROROS`).

---

## 12. Key Files

| File                          | Role                                              |
|-------------------------------|---------------------------------------------------|
| Src/app.c                     | Main loop, safety, micro-ROS orchestration        |
| Src/motor.c                   | PWM duty calculation, ramping, H-bridge write     |
| Inc/motor.h                   | Motor constants (RAMP_STEP, MAX_SPEED, etc.)      |
| Src/encoder.c                 | ESP32 I2C encoder interface                       |
| Inc/encoder.h                 | I2C protocol offsets, tick/mm constants           |
| Src/icm20948.c                | IMU driver (read, calibrate, complementary filter)|
| Src/madgwick.c                | Madgwick AHRS filter for orientation              |
| Src/microros_transport.c      | UART DMA transport for micro-ROS                  |
| Src/lcd_display.c             | LCD status dashboard                              |
| Src/tim.c                     | CubeMX timer init (ARR, prescaler values)         |
| rover_live/laptop/start.sh    | Laptop launcher (RViz, teleop, monitor)           |
| rover_live/pi/start_pi.sh     | Pi service checker/restarter                      |
| bringup.launch.pi.py          | Pi full ROS 2 stack launcher                      |
| ros2/fastdds_pi_peer.xml      | FastDDS unicast peer config for Pi (Tailscale)    |
