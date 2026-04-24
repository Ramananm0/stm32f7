#include "imu_min_test.h"
#include "i2c.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>

#define ICM_ADDR_68       (0x68u << 1)
#define ICM_ADDR_69       (0x69u << 1)
#define REG_BANK_SEL      0x7Fu
#define B0_WHO_AM_I       0x00u
#define B0_PWR_MGMT_1     0x06u
#define B0_PWR_MGMT_2     0x07u
#define B0_ACCEL_XOUT_H   0x2Du
#define I2C_TO_MS         50u

volatile uint32_t g_imu_min_stage;
volatile uint8_t g_imu_min_addr;
volatile uint8_t g_imu_min_who;
volatile uint8_t g_imu_min_ready68;
volatile uint8_t g_imu_min_ready69;
volatile int16_t g_imu_min_ax;
volatile int16_t g_imu_min_ay;
volatile int16_t g_imu_min_az;
volatile int16_t g_imu_min_gx;
volatile int16_t g_imu_min_gy;
volatile int16_t g_imu_min_gz;

static void draw(uint16_t x, uint16_t y, const char *text, uint32_t color)
{
    BSP_LCD_SetTextColor(color);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_DisplayStringAt(x, y, (uint8_t *)text, LEFT_MODE);
}

static HAL_StatusTypeDef wr(uint8_t dev, uint8_t reg, uint8_t val)
{
    uint8_t b[2] = {reg, val};
    return HAL_I2C_Master_Transmit(&hi2c1, dev, b, sizeof(b), I2C_TO_MS);
}

static HAL_StatusTypeDef rd(uint8_t dev, uint8_t reg, uint8_t *buf, uint16_t len)
{
    HAL_StatusTypeDef s = HAL_I2C_Master_Transmit(&hi2c1, dev, &reg, 1, I2C_TO_MS);
    if (s != HAL_OK) {
        return s;
    }
    return HAL_I2C_Master_Receive(&hi2c1, dev, buf, len, I2C_TO_MS);
}

static HAL_StatusTypeDef bank0(uint8_t dev)
{
    return wr(dev, REG_BANK_SEL, 0x00u);
}

static uint8_t detect_imu(void)
{
    uint8_t who = 0;

    g_imu_min_ready68 = (HAL_I2C_IsDeviceReady(&hi2c1, ICM_ADDR_68, 2, I2C_TO_MS) == HAL_OK) ? 1u : 0u;
    g_imu_min_ready69 = (HAL_I2C_IsDeviceReady(&hi2c1, ICM_ADDR_69, 2, I2C_TO_MS) == HAL_OK) ? 1u : 0u;

    if (g_imu_min_ready68 && bank0(ICM_ADDR_68) == HAL_OK &&
        rd(ICM_ADDR_68, B0_WHO_AM_I, &who, 1) == HAL_OK) {
        g_imu_min_addr = 0x68u;
        g_imu_min_who = who;
        return ICM_ADDR_68;
    }

    if (g_imu_min_ready69 && bank0(ICM_ADDR_69) == HAL_OK &&
        rd(ICM_ADDR_69, B0_WHO_AM_I, &who, 1) == HAL_OK) {
        g_imu_min_addr = 0x69u;
        g_imu_min_who = who;
        return ICM_ADDR_69;
    }

    g_imu_min_addr = 0u;
    g_imu_min_who = 0u;
    return 0u;
}

static HAL_StatusTypeDef init_min(uint8_t dev)
{
    HAL_StatusTypeDef s;

    s = bank0(dev);
    if (s != HAL_OK) {
        return s;
    }
    s = wr(dev, B0_PWR_MGMT_1, 0x01u);
    if (s != HAL_OK) {
        return s;
    }
    HAL_Delay(10);
    return wr(dev, B0_PWR_MGMT_2, 0x00u);
}

void IMU_Min_Test_Run(void)
{
    char line[64];
    uint8_t dev = 0;
    uint8_t b[14];
    uint32_t last = 0;

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font16);
    draw(8, 8, "IMU MIN TEST", LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font12);
    draw(8, 42, "I2C1 PB8=SCL PB9=SDA", LCD_COLOR_LIGHTGRAY);

    while (1) {
        if (HAL_GetTick() - last < 250u) {
            continue;
        }
        last = HAL_GetTick();
        g_imu_min_stage++;

        dev = detect_imu();

        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(8, 70, 464, 170);
        BSP_LCD_SetFont(&Font12);

        snprintf(line, sizeof(line), "ACK 0x68:%u  0x69:%u", g_imu_min_ready68, g_imu_min_ready69);
        draw(8, 76, line, LCD_COLOR_WHITE);
        snprintf(line, sizeof(line), "ADDR:0x%02X  WHO:0x%02X", g_imu_min_addr, g_imu_min_who);
        draw(8, 100, line, (g_imu_min_who == 0xEAu) ? LCD_COLOR_GREEN : LCD_COLOR_YELLOW);

        if (dev == 0u) {
            draw(8, 130, "IMU NOT FOUND", LCD_COLOR_RED);
            draw(8, 154, "Check CS=3V3 AD0=GND", LCD_COLOR_YELLOW);
            continue;
        }

        if (g_imu_min_who != 0xEAu) {
            draw(8, 130, "DEVICE ACKS BUT WHO != EA", LCD_COLOR_YELLOW);
            continue;
        }

        if (init_min(dev) != HAL_OK || bank0(dev) != HAL_OK ||
            rd(dev, B0_ACCEL_XOUT_H, b, sizeof(b)) != HAL_OK) {
            draw(8, 130, "READ FAILED AFTER INIT", LCD_COLOR_RED);
            continue;
        }

        g_imu_min_ax = (int16_t)((b[0] << 8) | b[1]);
        g_imu_min_ay = (int16_t)((b[2] << 8) | b[3]);
        g_imu_min_az = (int16_t)((b[4] << 8) | b[5]);
        g_imu_min_gx = (int16_t)((b[8] << 8) | b[9]);
        g_imu_min_gy = (int16_t)((b[10] << 8) | b[11]);
        g_imu_min_gz = (int16_t)((b[12] << 8) | b[13]);

        snprintf(line, sizeof(line), "ACC %6d %6d %6d", g_imu_min_ax, g_imu_min_ay, g_imu_min_az);
        draw(8, 130, line, LCD_COLOR_GREEN);
        snprintf(line, sizeof(line), "GYR %6d %6d %6d", g_imu_min_gx, g_imu_min_gy, g_imu_min_gz);
        draw(8, 154, line, LCD_COLOR_GREEN);
        snprintf(line, sizeof(line), "TICK %lu", (unsigned long)g_imu_min_stage);
        draw(8, 190, line, LCD_COLOR_LIGHTGRAY);
    }
}
