#include "encoder.h"
#include <string.h>

Encoder_t g_enc[ENC_NUM];

static I2C_HandleTypeDef *g_hi2c;
static uint8_t g_dirs[ENC_NUM];
static uint8_t g_dirs_dirty;
static uint8_t g_initialised;
static uint32_t g_status;

static void clear_samples(void)
{
    for (uint8_t i = 0; i < ENC_NUM; i++) {
        g_enc[i].ticks = 0;
        g_enc[i].vel_mmps = 0.0f;
        g_enc[i].dist_mm = 0.0f;
    }
}

void Encoder_Init(I2C_HandleTypeDef *hi2c)
{
    g_hi2c = hi2c;
    memset(g_enc, 0, sizeof(g_enc));
    memset(g_dirs, 0, sizeof(g_dirs));
    g_dirs_dirty = 1u;
    g_initialised = 1u;
    g_status = 0u;
}

HAL_StatusTypeDef Encoder_Update(void)
{
    uint8_t buf[ESP32_READ_LEN];
    HAL_StatusTypeDef status;

    if (!g_initialised || g_hi2c == NULL) {
        clear_samples();
        g_status = 0u;
        return HAL_ERROR;
    }

    if (g_dirs_dirty) {
        status = HAL_I2C_Master_Transmit(g_hi2c, ESP32_ENCODER_ADDR,
                                         g_dirs, ESP32_WRITE_LEN, 10);
        if (status == HAL_OK) {
            g_dirs_dirty = 0u;
        }
    }

    status = HAL_I2C_Master_Receive(g_hi2c, ESP32_ENCODER_ADDR,
                                    buf, ESP32_READ_LEN, 20);
    if (status != HAL_OK) {
        clear_samples();
        g_status = 0u;
        g_dirs_dirty = 1u;
        return status;
    }

    memcpy(&g_status, &buf[ENC_STATUS_OFFSET], sizeof(g_status));

    for (uint8_t i = 0; i < ENC_NUM; i++) {
        int32_t ticks;
        memcpy(&ticks, &buf[ENC_TICKS_OFFSET + i * sizeof(ticks)], sizeof(ticks));
        g_enc[i].ticks = ticks;
        g_enc[i].dist_mm = (float)ticks * ENC_MM_PER_TICK;
    }

    for (uint8_t i = 0; i < ENC_NUM; i++) {
        float vel;
        memcpy(&vel, &buf[ENC_VELS_OFFSET + i * sizeof(vel)], sizeof(vel));
        g_enc[i].vel_mmps = vel;
    }

    return HAL_OK;
}

void Encoder_SetDirection(uint8_t enc_id, uint8_t forward)
{
    uint8_t dir;

    if (enc_id >= ENC_NUM) {
        return;
    }

    dir = forward ? 0u : 1u;
    if (g_dirs[enc_id] != dir) {
        g_dirs[enc_id] = dir;
        g_enc[enc_id].direction = dir;
        g_dirs_dirty = 1u;
    }
}

void Encoder_SendDirections(uint8_t directions[ENC_NUM])
{
    memcpy(g_dirs, directions, ENC_NUM);
    g_dirs_dirty = 1u;
}

float Encoder_VelMmps(uint8_t idx)
{
    return idx < ENC_NUM ? g_enc[idx].vel_mmps : 0.0f;
}

float Encoder_DistMm(uint8_t idx)
{
    return idx < ENC_NUM ? g_enc[idx].dist_mm : 0.0f;
}

int32_t Encoder_Ticks(uint8_t idx)
{
    return idx < ENC_NUM ? g_enc[idx].ticks : 0;
}

uint32_t Encoder_Status(void)
{
    return g_status;
}

uint8_t Encoder_Esp32Connected(void)
{
    return (g_status & ENC_STATUS_ESP32_ALIVE) ? 1u : 0u;
}

uint8_t Encoder_WheelsConnected(void)
{
    return (g_status & ENC_STATUS_WHEELS_VALID) ? 1u : 0u;
}

void Encoder_Reset(uint8_t idx)
{
    if (idx >= ENC_NUM) {
        return;
    }

    g_enc[idx].ticks = 0;
    g_enc[idx].vel_mmps = 0.0f;
    g_enc[idx].dist_mm = 0.0f;
}
