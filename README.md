# STM32F7 Workspace

Top-level workspace for the STM32F7 terrain bot effort.

## Main local components

- `stm32f7disco/`: STM32F746G-DISCO firmware project
- `_terrain_ws_check/`: ROS 2 simulation workspace reference copy
- `_terrain_bot_source/`: STM32/ESP32 source reference repo
- `_raspberry_pi4b_check/`: Raspberry Pi ROS 2 stack reference repo
- `prompts/`, `data/`, `agents/`, `evals/`: requested workspace sections

## Current hardware links

- STM32 `I2C1` talks to the ESP32 encoder coprocessor on `PB8/PB9`
  - `D15 / PB8 -> ESP32 GPIO22` (`SCL`)
  - `D14 / PB9 -> ESP32 GPIO21` (`SDA`)
  - ESP32 I2C address: `0x30`
- STM32 `USART6` talks to the Raspberry Pi micro-ROS agent at `115200`
  - `D1 / PC6 = USART6_TX`
  - `D0 / PC7 = USART6_RX`
- STM32 motor PWM outputs are routed on Arduino Uno V3 headers as:
  - `A5 / PF6 = TIM10_CH1`
  - `A4 / PF7 = TIM11_CH1`
  - `A3 / PF8 = TIM13_CH1`
  - `A2 / PF9 = TIM14_CH1`
  - `D6 / PH6 = TIM12_CH1`
  - `D11 / PB15 = TIM12_CH2`
  - `D3 / PB4 = TIM3_CH1`
  - `D9 / PA15 = TIM2_CH1`

## Motor mapping in Arduino headers

- Front left motor:
  - `A5` and `A4`
- Front right motor:
  - `A3` and `A2`
- Rear left motor:
  - `D11` and `D6`
- Rear right motor:
  - `D3` and `D9`

## Notes

- The three `_...` directories above remain separate git repositories.
- This parent repo is intended to capture the overall workspace layout, scripts, docs, and firmware tree without rewriting child repo history.
