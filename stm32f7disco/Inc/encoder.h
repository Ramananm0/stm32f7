#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f7xx_hal.h"
#include <stdint.h>

#define ESP32_ENCODER_ADDR   (0x30 << 1)
#define ESP32_WRITE_LEN      4

#define ENC_TICK_BYTES       4u
#define ENC_VEL_BYTES        4u
#define ENC_STATUS_OFFSET    0u
#define ENC_TICKS_OFFSET     4u
#define ENC_VELS_OFFSET      (ENC_TICKS_OFFSET + (ENC_NUM * ENC_TICK_BYTES))
#define ESP32_READ_LEN       (ENC_VELS_OFFSET + (ENC_NUM * ENC_VEL_BYTES))

#define ENC_STATUS_ESP32_ALIVE   (1u << 0)
#define ENC_STATUS_WHEELS_VALID  (1u << 1)

#define ENC_FL     0
#define ENC_FR     1
#define ENC_RL     2
#define ENC_RR     3
#define ENC_NUM    4

#define ENC_LEFT   ENC_FL
#define ENC_RIGHT  ENC_FR

#define ENC_TICKS_PER_REV    2264u
#define ENC_WHEEL_CIRC_MM    267.04f   /* π × 85 mm wheel diameter */
#define ENC_MM_PER_TICK      (ENC_WHEEL_CIRC_MM / (float)ENC_TICKS_PER_REV)
#define ENC_WHEELBASE_MM     200.0f    /* left-to-right wheel centre distance — measure and update */

#if (ESP32_READ_LEN != 36u)
#error "Encoder frame size must remain 36 bytes"
#endif

typedef struct {
    int32_t ticks;
    float vel_mmps;
    float dist_mm;
    uint8_t direction;
} Encoder_t;

extern Encoder_t g_enc[ENC_NUM];

void Encoder_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef Encoder_Update(void);
void Encoder_SendDirections(uint8_t directions[ENC_NUM]);
void Encoder_SetDirection(uint8_t enc_id, uint8_t forward);
float Encoder_VelMmps(uint8_t idx);
float Encoder_DistMm(uint8_t idx);
int32_t Encoder_Ticks(uint8_t idx);
uint32_t Encoder_Status(void);
uint8_t Encoder_Esp32Connected(void);
uint8_t Encoder_WheelsConnected(void);
void Encoder_Reset(uint8_t idx);

#endif /* ENCODER_H */
