#include "lcd_test.h"
#include "main.h"

#define LCD_W  480u
#define LCD_H  272u
#define LCD_FB ((volatile uint16_t *)0xC0000000u)

#define RGB565(r, g, b) (uint16_t)((((r) & 0xF8u) << 8) | (((g) & 0xFCu) << 3) | (((b) & 0xF8u) >> 3))

volatile uint32_t g_lcd_test_step;

static void lcd_enable_panel_pins(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOK_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    gpio.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOI, &gpio);
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET);

    gpio.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOK, &gpio);
    HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_SET);
}

static void fill(uint16_t color)
{
    for (uint32_t i = 0; i < (LCD_W * LCD_H); i++) {
        LCD_FB[i] = color;
    }
    __DSB();
}

static void bars(void)
{
    static const uint16_t colors[] = {
        RGB565(255, 255, 255),
        RGB565(255, 255, 0),
        RGB565(0, 255, 255),
        RGB565(0, 255, 0),
        RGB565(255, 0, 255),
        RGB565(255, 0, 0),
        RGB565(0, 0, 255),
        RGB565(0, 0, 0),
    };
    const uint32_t bar_w = LCD_W / 8u;

    for (uint32_t y = 0; y < LCD_H; y++) {
        for (uint32_t x = 0; x < LCD_W; x++) {
            uint32_t idx = x / bar_w;
            if (idx >= 8u) {
                idx = 7u;
            }
            LCD_FB[y * LCD_W + x] = colors[idx];
        }
    }
    __DSB();
}

static void checker(void)
{
    for (uint32_t y = 0; y < LCD_H; y++) {
        for (uint32_t x = 0; x < LCD_W; x++) {
            uint8_t block = (uint8_t)(((x >> 4) ^ (y >> 4)) & 1u);
            LCD_FB[y * LCD_W + x] = block ? RGB565(255, 255, 255) : RGB565(0, 0, 0);
        }
    }
    __DSB();
}

void LCD_Test_Run(void)
{
    lcd_enable_panel_pins();

    while (1) {
        g_lcd_test_step = 1u;
        fill(RGB565(255, 0, 0));
        HAL_Delay(5000);
        g_lcd_test_step = 2u;
        fill(RGB565(0, 255, 0));
        HAL_Delay(5000);
        g_lcd_test_step = 3u;
        fill(RGB565(0, 0, 255));
        HAL_Delay(5000);
        g_lcd_test_step = 4u;
        fill(RGB565(255, 255, 255));
        HAL_Delay(5000);
        g_lcd_test_step = 5u;
        fill(RGB565(0, 0, 0));
        HAL_Delay(5000);
        g_lcd_test_step = 6u;
        bars();
        HAL_Delay(8000);
        g_lcd_test_step = 7u;
        checker();
        HAL_Delay(8000);
    }
}
