#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "icm20948.h"
#include "madgwick.h"
#include <stdint.h>

void LCD_Display_Init(void);
void LCD_Display_BootStatus(uint8_t imu_ok, uint8_t esp32_ok, uint8_t ros_ok);
void LCD_Display_BootMotorTest(const char *label);
void LCD_Display_Update(const Madgwick_t *ahrs,
                        const ICM20948_Data *imu,
                        uint8_t calib_done,
                        float risk,
                        uint8_t emergency_stop,
                        uint8_t imu_ok,
                        uint8_t esp32_ok,
                        uint8_t wheels_ok,
                        uint8_t ros_ok,
                        uint8_t host_ok);

#endif /* LCD_DISPLAY_H */
