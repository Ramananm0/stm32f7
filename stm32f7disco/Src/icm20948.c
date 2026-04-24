#include "icm20948.h"
#include <math.h>

#define I2C_TO_MS 50u

static uint8_t g_icm20948_addr = ICM20948_ADDR_68;

static HAL_StatusTypeDef bank(I2C_HandleTypeDef *hi2c, uint8_t bank_id)
{
    uint8_t buf[2] = {REG_BANK_SEL, bank_id};
    return HAL_I2C_Master_Transmit(hi2c, g_icm20948_addr, buf, 2, I2C_TO_MS);
}

static HAL_StatusTypeDef wr(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    return HAL_I2C_Master_Transmit(hi2c, g_icm20948_addr, buf, 2, I2C_TO_MS);
}

static HAL_StatusTypeDef rd(I2C_HandleTypeDef *hi2c,
                            uint8_t reg,
                            uint8_t *buf,
                            uint16_t len)
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Transmit(hi2c, g_icm20948_addr, &reg, 1, I2C_TO_MS);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_I2C_Master_Receive(hi2c, g_icm20948_addr, buf, len, I2C_TO_MS);
}

static HAL_StatusTypeDef detect(I2C_HandleTypeDef *hi2c)
{
    const uint8_t addrs[] = {ICM20948_ADDR_68, ICM20948_ADDR_69};
    uint8_t who = 0;

    for (uint8_t i = 0; i < sizeof(addrs); i++) {
        g_icm20948_addr = addrs[i];
        if (HAL_I2C_IsDeviceReady(hi2c, g_icm20948_addr, 3, I2C_TO_MS) != HAL_OK) {
            continue;
        }
        if (bank(hi2c, 0x00) == HAL_OK &&
            rd(hi2c, B0_WHO_AM_I, &who, 1) == HAL_OK &&
            who == 0xEA) {
            return HAL_OK;
        }
    }

    g_icm20948_addr = ICM20948_ADDR_68;
    return HAL_ERROR;
}

static void ak_write(I2C_HandleTypeDef *hi2c, uint8_t ak_reg, uint8_t value)
{
    bank(hi2c, 0x30);
    wr(hi2c, B3_SLV0_ADDR, AK_ADDR);
    wr(hi2c, B3_SLV0_REG, ak_reg);
    wr(hi2c, B3_SLV0_DO, value);
    wr(hi2c, B3_SLV0_CTRL, 0x81);
    HAL_Delay(2);
    bank(hi2c, 0x00);
}

HAL_StatusTypeDef ICM20948_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t who = 0;

    if (detect(hi2c) != HAL_OK) {
        return HAL_ERROR;
    }

    if (bank(hi2c, 0x00) != HAL_OK) {
        return HAL_ERROR;
    }
    if (wr(hi2c, B0_PWR_MGMT_1, 0x80) != HAL_OK) {
        return HAL_ERROR;
    }
    HAL_Delay(100);
    if (wr(hi2c, B0_PWR_MGMT_1, 0x01) != HAL_OK) {
        return HAL_ERROR;
    }
    HAL_Delay(10);

    if (rd(hi2c, B0_WHO_AM_I, &who, 1) != HAL_OK || who != 0xEA) {
        return HAL_ERROR;
    }

    if (wr(hi2c, B0_PWR_MGMT_2, 0x00) != HAL_OK) {
        return HAL_ERROR;
    }

    if (bank(hi2c, 0x20) != HAL_OK ||
        wr(hi2c, B2_GYRO_SMPLRT_DIV, 10) != HAL_OK ||
        wr(hi2c, B2_GYRO_CONFIG_1, 0x0B) != HAL_OK ||
        wr(hi2c, B2_ACCEL_SMPLRT_2, 10) != HAL_OK ||
        wr(hi2c, B2_ACCEL_CONFIG, 0x0B) != HAL_OK) {
        return HAL_ERROR;
    }

    if (bank(hi2c, 0x30) != HAL_OK ||
        wr(hi2c, B3_I2C_MST_CTRL, 0x07) != HAL_OK ||
        bank(hi2c, 0x00) != HAL_OK ||
        wr(hi2c, B0_USER_CTRL, 0x20) != HAL_OK) {
        return HAL_ERROR;
    }
    HAL_Delay(10);

    ak_write(hi2c, AK_CNTL2, 0x01);
    HAL_Delay(10);
    ak_write(hi2c, AK_CNTL2, AK_CNTL2_100HZ);
    HAL_Delay(10);

    if (bank(hi2c, 0x30) != HAL_OK ||
        wr(hi2c, B3_SLV0_ADDR, AK_ADDR | 0x80) != HAL_OK ||
        wr(hi2c, B3_SLV0_REG, AK_ST1) != HAL_OK ||
        wr(hi2c, B3_SLV0_CTRL, 0x89) != HAL_OK ||
        bank(hi2c, 0x00) != HAL_OK) {
        return HAL_ERROR;
    }

    if (wr(hi2c, B0_INT_PIN_CFG, 0x12) != HAL_OK ||
        wr(hi2c, B0_INT_ENABLE_1, 0x01) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef ICM20948_Read(I2C_HandleTypeDef *hi2c, ICM20948_Data *data)
{
    uint8_t b[23];

    if (rd(hi2c, B0_ACCEL_XOUT_H, b, sizeof(b)) != HAL_OK) {
        return HAL_ERROR;
    }

    data->ax = (float)(int16_t)((b[0] << 8) | b[1]) * ACCEL_SCALE_MS2;
    data->ay = (float)(int16_t)((b[2] << 8) | b[3]) * ACCEL_SCALE_MS2;
    data->az = (float)(int16_t)((b[4] << 8) | b[5]) * ACCEL_SCALE_MS2;
    data->temp_c = (float)(int16_t)((b[6] << 8) | b[7]) * TEMP_SCALE + TEMP_OFFSET;
    data->gx = (float)(int16_t)((b[8] << 8) | b[9]) * GYRO_SCALE_RADS;
    data->gy = (float)(int16_t)((b[10] << 8) | b[11]) * GYRO_SCALE_RADS;
    data->gz = (float)(int16_t)((b[12] << 8) | b[13]) * GYRO_SCALE_RADS;

    if (b[14] & 0x01u) {
        data->mx = (float)(int16_t)((b[16] << 8) | b[15]) * MAG_SCALE_UT;
        data->my = (float)(int16_t)((b[18] << 8) | b[17]) * MAG_SCALE_UT;
        data->mz = (float)(int16_t)((b[20] << 8) | b[19]) * MAG_SCALE_UT;
    }

    data->timestamp_ms = HAL_GetTick();
    data->data_ready = true;
    return HAL_OK;
}

bool ICM20948_Calibrate(const ICM20948_Data *data,
                        ICM20948_Calib *calib,
                        uint16_t n_samples)
{
    if (calib->done) {
        return true;
    }

    calib->gx_bias += data->gx;
    calib->gy_bias += data->gy;
    calib->gz_bias += data->gz;
    calib->gravity += sqrtf(data->ax * data->ax + data->ay * data->ay + data->az * data->az);
    calib->count++;

    if (calib->count >= n_samples) {
        float n = (float)n_samples;
        calib->gx_bias /= n;
        calib->gy_bias /= n;
        calib->gz_bias /= n;
        calib->gravity /= n;
        calib->done = true;
    }

    return calib->done;
}

void ICM20948_Filter(ICM20948_Data *data,
                     const ICM20948_Calib *calib,
                     float dt_s,
                     float alpha)
{
    float gx;
    float gy;
    float roll_acc;
    float pitch_acc;

    if (!calib->done || dt_s <= 0.0f || dt_s > 0.5f) {
        return;
    }

    gx = data->gx - calib->gx_bias;
    gy = data->gy - calib->gy_bias;
    roll_acc = atan2f(data->ay, data->az);
    pitch_acc = atan2f(-data->ax, sqrtf(data->ay * data->ay + data->az * data->az));

    data->roll_rad = alpha * (data->roll_rad + gx * dt_s) + (1.0f - alpha) * roll_acc;
    data->pitch_rad = alpha * (data->pitch_rad + gy * dt_s) + (1.0f - alpha) * pitch_acc;
}
