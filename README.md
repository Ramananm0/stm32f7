# STM32F7 Workspace

Top-level workspace for the STM32F7 terrain bot effort.

## Main local components

- `stm32f7disco/`: STM32F746G-DISCO firmware project
- `_terrain_ws_check/`: ROS 2 simulation workspace reference copy
- `_terrain_bot_source/`: STM32/ESP32 source reference repo
- `_raspberry_pi4b_check/`: Raspberry Pi ROS 2 stack reference repo
- `prompts/`, `data/`, `agents/`, `evals/`: requested workspace sections

## Notes

- The three `_...` directories above remain separate git repositories.
- This parent repo is intended to capture the overall workspace layout, scripts, docs, and firmware tree without rewriting child repo history.
