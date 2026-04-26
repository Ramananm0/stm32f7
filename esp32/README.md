# ESP32-WROOM — Encoder Co-processor

This folder contains firmware for the **ESP32-WROOM-32** running as an
I2C slave encoder co-processor for the STM32F746G-DISCO terrain bot.

---

## What it does

The ESP32 uses its built-in **hardware PCNT (Pulse Counter)** peripheral
to count quadrature encoder pulses from all 4 RMCS-3070 motors with
zero CPU overhead. It serves the tick counts and wheel velocities to the
STM32 master over I2C on demand.

```
STM32 (master, addr 0x68 IMU already)
  │
  ├── I2C read  → ESP32 (0x30) → 36 bytes: status + ticks + speeds
  └── I2C write → ESP32 (0x30) →  4 bytes: motor directions
```

---

## Hardware Setup

### Power
```
STM32 3V3 pin  →  ESP32 3V3
STM32 GND      →  ESP32 GND
```

### I2C (shared bus with ICM-20948)
```
STM32 D15 (PB8)  →  ESP32 GPIO22 (SCL)
STM32 D14 (PB9)  →  ESP32 GPIO21 (SDA)
```

STM32 `I2C1` is configured as open-drain with `GPIO_PULLUP` in firmware. If the
shared bus is unstable, add or verify external pull-ups on SDA and SCL.

### Encoder connections
```
Motor   CH_A (green wire)   CH_B (white wire)
─────   ─────────────────   ─────────────────
FL      GPIO4               GPIO13
FR      GPIO18              GPIO19
RL      GPIO25              GPIO26
RR      GPIO32              GPIO33

Red  wire → 5V
Black wire → GND
```

Current firmware uses `GPIO4` and `GPIO13` for the front-left encoder, even
though older notes used `GPIO16` and `GPIO17`. Treat `esp32/src/main.cpp` as
the source of truth if the harness and this README ever diverge again.

`GPIO4` is a boot strapping pin on common ESP32 DevKit boards. If flashing or
boot becomes unreliable with the encoder connected, disconnect that channel
during upload or move the front-left encoder back to a non-strapping pin pair
and update the firmware to match.

---

## I2C Protocol

### STM32 reads 36 bytes (encoder data + status)
```
Bytes  0-3  : status bitmask (uint32, little-endian)
              bit0 = ESP32 alive
              bit1 = wheel data valid
Bytes  4-7  : FL ticks  (int32, little-endian)
Bytes  8-11 : FR ticks  (int32, little-endian)
Bytes 12-15 : RL ticks  (int32, little-endian)
Bytes 16-19 : RR ticks  (int32, little-endian)
Bytes 20-23 : FL speed  (float, mm/s)
Bytes 24-27 : FR speed  (float, mm/s)
Bytes 28-31 : RL speed  (float, mm/s)
Bytes 32-35 : RR speed  (float, mm/s)
```

### STM32 writes 4 bytes (direction info)
```
Byte 0 : FL direction (0=forward, 1=reverse)
Byte 1 : FR direction
Byte 2 : RL direction
Byte 3 : RR direction
```

---

## PCNT Quadrature Mode

The ESP32 PCNT counts ×4 quadrature:
```
CH_A rising  + CH_B LOW  → count UP   (forward)
CH_A rising  + CH_B HIGH → count DOWN (reverse)
CH_B rising  + CH_A HIGH → count UP
CH_B rising  + CH_A LOW  → count DOWN
```

This gives 2264 ticks per wheel revolution:
```
PPR (motor shaft) = 11
Gear ratio        = 51.45
Quadrature        = ×4
Ticks per rev     = 11 × 51.45 × 4 = 2264
mm per tick       = 26.70mm / 2264  = 0.01179mm
```

---

## Build & Flash

Uses PlatformIO:

```bash
# Install PlatformIO CLI
pip install platformio

# Build
cd esp32
pio run

# Flash (connect ESP32 via USB)
pio run --target upload

# Monitor serial output
pio device monitor
```

Or use **VS Code + PlatformIO extension**:
1. Open the `esp32/` folder in VS Code
2. Click the PlatformIO upload button

---

## Serial Debug Output

When running, the ESP32 prints periodic I2C activity:
```
i2c requests=123 receives=77 dirs=0,0,0,0 ticks=10,20,30,40 speed=1.2,1.3,1.4,1.5
```

---

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| STM32 I2C fails | Address conflict | Confirm ESP32 is at 0x30 |
| Encoders count wrong | CH_A/CH_B swapped | Swap green/white wires |
| Speed always 0 | PCNT not started | Check GPIO pin numbers |
| Counts drift | Noise on encoder lines | Increase PCNT filter value |
| I2C timeout | Weak or missing bus pull-ups | Verify shared-bus pull-ups on SDA and SCL |
| Flash upload fails | Boot strapping conflict on FL encoder | Disconnect GPIO4 encoder lead during upload or reassign the FL pair in firmware |
