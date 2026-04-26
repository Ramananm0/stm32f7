# STM32F7 Functionality Comparison - 2026-04-25

Compared:

- Existing local folder: `/home/ramana/stm32f7`
- Clean GitHub clone: `/home/ramana/stm32f7_github_compare`
- Source repo: `https://github.com/Ramananm0/stm32f7.git`

## Project Baseline Decision

Use `/home/ramana/stm32f7_github_compare` as the active project baseline.

Reason:

- It is a real Git clone.
- It has the cleaner requested structure: `agents/`, `data/`, `evals/`, and `prompts/`.
- Its firmware code is functionally newer than `/home/ramana/stm32f7`.
- The old local folder contains useful runtime artifacts, generated build outputs, live debug captures, and nested reference repos, but it is not a clean source baseline.

Do not overwrite `/home/ramana/stm32f7` blindly. Pull useful local-only scripts, notes, and runtime artifacts into the clean repo only when they are still relevant.

## Code-Level Summary

The clean GitHub clone is not only reorganized. It has newer runtime behavior in the STM32 firmware.

The largest functional changes are in:

- `stm32f7disco/Src/app.c`
- `stm32f7disco/Src/motor.c`
- `stm32f7disco/Src/encoder.c`
- `stm32f7disco/Src/lcd_display.c`
- `stm32f7disco/Inc/app.h`
- `stm32f7disco/Inc/motor.h`
- `stm32f7disco/Inc/encoder.h`
- `stm32f7disco/Inc/lcd_display.h`

Many other diffs are line-ending or generated-file churn and are less important for behavior.

## Main Functional Differences

### 1. Local firmware still contains temporary open-loop motor test code

In `/home/ramana/stm32f7/stm32f7disco/Src/app.c`, the PID loop section still directly drives three motors:

```c
Motor_Set(MOTOR_FL, 20.0f);
Motor_Set(MOTOR_RL, 20.0f);
Motor_Set(MOTOR_RR, 20.0f);
/* MOTOR_FR skipped */
```

This is bring-up code, not final rover behavior.

The GitHub clone removes that temporary behavior and runs the normal target/PID path.

### 2. GitHub firmware adds encoder-aware motor control

The GitHub version:

- Tracks `g_encoder_feedback_ok`
- Falls back to open-loop target application only when encoder feedback is unavailable
- Filters measured wheel velocity
- Uses PID plus wheel synchronization terms
- Stops cleanly when targets are near zero

This makes it the better baseline for closed-loop rover behavior.

### 3. GitHub firmware adds slip-risk logic

The GitHub version computes both:

- tilt risk from IMU attitude and vibration
- slip risk from wheel target/measured mismatch

The combined risk uses the dominant of tilt and slip risk, then applies squared speed scaling.

The older local code only has the tilt/IMU side of this behavior.

### 4. GitHub firmware adds startup motor/encoder validation

The GitHub version has `startup_motor_check()` in `app.c`.

It:

- reads baseline encoder ticks
- applies a controlled test duty
- checks whether each wheel moved enough
- stops all motors
- displays pass/fail on the LCD

This is important for early hardware detection before normal runtime.

### 5. GitHub firmware adds a dedicated motor/ESP32 diagnostic mode

The GitHub version adds:

- `App_MotorEsp32Diag_Run()` in `stm32f7disco/Src/app.c`
- `MOTOR_ESP32_DIAG_MODE` branch in `stm32f7disco/Src/main.c`
- `LCD_Display_BootMotorTest()` for visible wheel-test state

This gives a clean path for motor/encoder bring-up without polluting normal `App_Run()`.

### 6. GitHub motor driver is safer and more calibrated

The GitHub `motor.c` adds:

- safe max duty derived from timer auto-reload value
- 85 percent PWM headroom
- per-wheel trim values
- `Motor_SetTestDuty()` for controlled diagnostics

The local version uses fixed APB1/APB2 max-duty constants.

### 7. GitHub encoder parsing is clearer and safer

The GitHub `encoder.h` replaces hardcoded offsets with named constants:

- `ENC_STATUS_OFFSET`
- `ENC_TICKS_OFFSET`
- `ENC_VELS_OFFSET`
- `ESP32_READ_LEN`

It also compile-checks that the ESP32 frame remains 36 bytes.

`encoder.c` clears stale samples on I2C failure, which avoids using old velocity/tick data after a failed read.

### 8. GitHub micro-ROS traffic is reduced for 115200 baud

Local timing:

- IMU publish: 10 ms
- encoder publish: 20 ms
- watchdog: 500 ms

GitHub timing:

- IMU publish: 100 ms
- encoder publish: 200 ms
- watchdog: 2000 ms

This is more realistic for USART6 at 115200 baud.

### 9. GitHub status message includes command counter

Local `/base_status` buffer has 4 fields:

1. ESP32 OK
2. wheels OK
3. IMU OK
4. host OK

GitHub `/base_status` buffer has 5 fields:

1. ESP32 OK
2. wheels OK
3. IMU OK
4. host OK
5. received command count

The command counter is useful for confirming whether `/cmd_vel` reaches the STM32.

## Local-Only Items Worth Preserving

The old `/home/ramana/stm32f7` folder includes artifacts that may still be useful:

- `terrain_rover_handoff_2026-04-25.txt`
- `MASTER_SYSTEM_DOCUMENTATION_2026-04-24.md`
- `_pi_live_20260425/`
- `_terrain_ws_check/`
- `_raspberry_pi4b_check/`
- `_terrain_bot_source/`
- `pi_camera_pull_node.py`
- `run_laptop_pi_camera_ai.sh`
- `run_laptop_terrain_ai.sh`
- `pi_deploy_lean_bringup.sh`
- `pi_enable_camera_b3_lowrate.sh`

Review these before copying them into the clean repo. Do not import generated folders such as `Debug/`, `.metadata/`, `.platformio_core/`, `__pycache__/`, or live image snapshots unless explicitly needed.

## Recommended Project Start

Work from:

```bash
cd /home/ramana/stm32f7_github_compare
```

First priorities:

1. Build the STM32 firmware from the clean clone.
2. Verify whether the clean clone contains the needed micro-ROS middleware/static library locally.
3. Flash the GitHub firmware to STM32.
4. Run the motor/ESP32 diagnostic mode if encoder/motor behavior is uncertain.
5. Start the Pi micro-ROS agent and verify `/base_status`, `/wheel_ticks`, `/wheel_velocity`, and `/cmd_vel`.
6. Only then merge useful local-only scripts or docs from `/home/ramana/stm32f7`.

## grafify Status

`grafify` has been cloned to:

```bash
/home/ramana/grafify
```

It is an R package. This machine currently does not have `R` or `Rscript`, so installation needs system R first:

```bash
sudo apt-get update
sudo apt-get install -y r-base r-cran-ggplot2 r-cran-car r-cran-dplyr r-cran-emmeans r-cran-hmisc r-cran-lme4 r-cran-lmertest r-cran-magrittr r-cran-mgcv r-cran-patchwork r-cran-purrr r-cran-tidyr r-cran-knitr r-cran-rmarkdown r-cran-pbkrtest r-cran-testthat r-cran-rlang
R CMD INSTALL /home/ramana/grafify
```
