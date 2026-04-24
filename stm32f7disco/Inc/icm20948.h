#ifndef ICM20948_H
#define ICM20948_H

#include "stm32f7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define ICM20948_ADDR_68    (0x68 << 1)
#define ICM20948_ADDR_69    (0x69 << 1)
#define REG_BANK_SEL        0x7F

#define B0_WHO_AM_I         0x00
#define B0_USER_CTRL        0x03
#define B0_PWR_MGMT_1       0x06
#define B0_PWR_MGMT_2       0x07
#define B0_INT_PIN_CFG      0x0F
#define B0_INT_ENABLE_1     0x11
#define B0_ACCEL_XOUT_H     0x2D

#define B2_GYRO_SMPLRT_DIV  0x00
#define B2_GYRO_CONFIG_1    0x01
#define B2_ACCEL_SMPLRT_2   0x11
#define B2_ACCEL_CONFIG     0x14

#define B3_I2C_MST_CTRL     0x01
#define B3_SLV0_ADDR        0x03
#define B3_SLV0_REG         0x04
#define B3_SLV0_CTRL        0x05
#define B3_SLV0_DO          0x06

#define AK_ADDR             0x0C
#define AK_CNTL2            0x31
#define AK_CNTL2_100HZ      0x08
#define AK_ST1              0x10

#define ACCEL_SCALE_MS2     (4.0f / 32768.0f * 9.80665f)
#define GYRO_SCALE_RADS     (500.0f / 32768.0f * 0.017453293f)
#define MAG_SCALE_UT        0.15f
#define TEMP_SCALE          (1.0f / 333.87f)
#define TEMP_OFFSET         21.0f

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
    float temp_c;
    float roll_rad;
    float pitch_rad;
    uint32_t timestamp_ms;
    bool data_ready;
} ICM20948_Data;

typedef struct {
    float gx_bias, gy_bias, gz_bias;
    float gravity;
    bool done;
    uint16_t count;
} ICM20948_Calib;

HAL_StatusTypeDef ICM20948_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef ICM20948_Read(I2C_HandleTypeDef *hi2c, ICM20948_Data *data);
bool ICM20948_Calibrate(const ICM20948_Data *data,
                        ICM20948_Calib *calib,
                        uint16_t n_samples);
void ICM20948_Filter(ICM20948_Data *data,
                     const ICM20948_Calib *calib,
                     float dt_s,
                     float alpha);

#endif /* ICM20948_H */
