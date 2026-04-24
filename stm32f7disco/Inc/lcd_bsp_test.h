#ifndef LCD_BSP_TEST_H
#define LCD_BSP_TEST_H

#include <stdint.h>

void LCD_BSP_Test_Run(void);

extern volatile uint32_t g_lcd_bsp_test_step;

#endif /* LCD_BSP_TEST_H */
