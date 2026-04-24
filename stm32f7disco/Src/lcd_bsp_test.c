#include "lcd_bsp_test.h"
#include "stm32746g_discovery_lcd.h"

volatile uint32_t g_lcd_bsp_test_step;

static void show(uint32_t step, uint32_t color, const char *label)
{
    g_lcd_bsp_test_step = step;
    BSP_LCD_Clear(color);
    BSP_LCD_SetBackColor(color);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 112, (uint8_t *)label, CENTER_MODE);
}

void LCD_BSP_Test_Run(void)
{
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(0);
    BSP_LCD_DisplayOn();
    BSP_LCD_SetFont(&Font24);

    while (1) {
        show(1u, LCD_COLOR_RED, "BSP RED");
        HAL_Delay(5000);
        show(2u, LCD_COLOR_GREEN, "BSP GREEN");
        HAL_Delay(5000);
        show(3u, LCD_COLOR_BLUE, "BSP BLUE");
        HAL_Delay(5000);
        show(4u, LCD_COLOR_WHITE, "BSP WHITE");
        HAL_Delay(5000);
        show(5u, LCD_COLOR_BLACK, "BSP BLACK");
        HAL_Delay(5000);
        show(6u, LCD_COLOR_CYAN, "BSP CYAN");
        HAL_Delay(5000);
        show(7u, LCD_COLOR_MAGENTA, "BSP MAGENTA");
        HAL_Delay(5000);
        show(8u, LCD_COLOR_YELLOW, "BSP YELLOW");
        HAL_Delay(5000);
    }
}
