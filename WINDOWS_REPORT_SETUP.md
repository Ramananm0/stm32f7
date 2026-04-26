# Windows Report Setup

This branch is suitable for reading code, diagrams, notes, and `graphify-out` on Windows.

## Get the Snapshot

If cloning fresh:

```bash
git clone -b windows-report-snapshot git@github.com:Ramananm0/stm32f7.git
```

If you already have the repo:

```bash
git fetch origin
git checkout windows-report-snapshot
git pull
```

## What To Open

For report writing, start with:

- `README.md`
- `UBUNTU_HANDOFF.md`
- `ROS2_LAPTOP_PI.md`
- `FUNCTIONALITY_COMPARISON_2026-04-25.md`
- `STM32F7_Terrain_Bot_Detailed_Notes.txt`
- `graphify-out/GRAPH_REPORT.md`
- `graphify-out/graph.json`
- `stm32f7disco/FIRMWARE_REFERENCE.md`

## Best Windows Workflow

- Open the folder in VS Code.
- Use the Markdown preview for `.md` files.
- Use the file explorer to read firmware source under `stm32f7disco/`.
- Use `graphify-out/GRAPH_REPORT.md` as the main architecture summary.

## Important Scope Note

This snapshot is for documentation, code reading, and report writing.

It does not include local build caches or generated test/build output such as:

- `.platformio_core/`
- `esp32/.pio/`
- `.codex/`

## If You Need A Quick Report Structure

Suggested sections:

1. System overview
2. STM32 firmware responsibilities
3. Raspberry Pi ROS 2 responsibilities
4. Laptop/operator tooling
5. Communication architecture
6. Current working status
7. Known issues and next steps

