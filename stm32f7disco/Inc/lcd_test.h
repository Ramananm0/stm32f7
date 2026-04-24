#ifndef LCD_TEST_H
#define LCD_TEST_H

#include <stdint.h>

void LCD_Test_Run(void);

extern volatile uint32_t g_lcd_test_step;

#endif /* LCD_TEST_H */
