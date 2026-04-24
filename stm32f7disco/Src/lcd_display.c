#include "lcd_display.h"
#include "encoder.h"
#include "motor.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stm32f7xx_hal.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern volatile uint32_t g_debug_stage;
extern volatile int32_t g_ros_error;
extern volatile uint32_t g_ros_rx_bytes;
extern volatile uint32_t g_ros_tx_bytes;
extern volatile uint8_t g_ros_rx_sample[4];
extern volatile uint8_t g_ros_rx_sample_len;
extern volatile uint32_t g_ros_retry_count;

#define COL_BG      LCD_COLOR_BLACK
#define COL_TEXT    LCD_COLOR_WHITE
#define COL_MUTED   LCD_COLOR_LIGHTGRAY
#define COL_OK      LCD_COLOR_WHITE
#define COL_WARN    LCD_COLOR_YELLOW
#define COL_LINE    LCD_COLOR_DARKGRAY
#define COL_BAD     LCD_COLOR_RED
#define COL_ACCENT  LCD_COLOR_CYAN
#define COL_FILL    LCD_COLOR_GREEN
#define COL_PANEL   LCD_COLOR_DARKGRAY

#define LCD_W       480U
#define LCD_H       272U
#define NAV_Y       228U
#define NAV_H       44U
#define IMU_X       328U
#define IMU_Y       78U
#define IMU_W       136U
#define IMU_H       52U
#define MOTOR_BTN_X 328U
#define MOTOR_BTN_Y 138U
#define MOTOR_BTN_W 136U
#define MOTOR_BTN_H 52U
#define BACK_X      8U
#define BACK_Y      230U
#define BACK_W      126U
#define BACK_H      34U

typedef enum
{
    LCD_PAGE_HOME = 0,
    LCD_PAGE_IMU = 1,
    LCD_PAGE_MOTORS = 2
} LcdPage_t;

static LcdPage_t g_page = LCD_PAGE_HOME;
static LcdPage_t g_drawn_page = (LcdPage_t)0xff;
static uint8_t g_touch_ok = 0;
static uint8_t g_touch_was_down = 0;

static void draw_text(uint16_t x, uint16_t y, const char *text, uint32_t color)
{
    BSP_LCD_SetTextColor(color);
    BSP_LCD_SetBackColor(COL_BG);
    BSP_LCD_DisplayStringAt(x, y, (uint8_t *)text, LEFT_MODE);
}

static void draw_text_bg(uint16_t x, uint16_t y, const char *text, uint32_t color, uint32_t bg)
{
    BSP_LCD_SetTextColor(color);
    BSP_LCD_SetBackColor(bg);
    BSP_LCD_DisplayStringAt(x, y, (uint8_t *)text, LEFT_MODE);
}

static void clear_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    BSP_LCD_SetTextColor(COL_BG);
    BSP_LCD_FillRect(x, y, w, h);
}

static void draw_panel(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *title)
{
    BSP_LCD_SetTextColor(COL_LINE);
    BSP_LCD_DrawRect(x, y, w, h);
    BSP_LCD_SetFont(&Font12);
    draw_text((uint16_t)(x + 8U), (uint16_t)(y + 8U), title, COL_MUTED);
}

static void draw_state_box(uint16_t x, uint16_t y, const char *label, uint8_t ok)
{
    BSP_LCD_SetTextColor(ok ? COL_FILL : COL_BAD);
    BSP_LCD_FillRect(x, y, 14U, 14U);
    BSP_LCD_SetTextColor(COL_LINE);
    BSP_LCD_DrawRect(x, y, 14U, 14U);
    draw_text((uint16_t)(x + 22U), (uint16_t)(y - 1U), label, ok ? COL_OK : COL_WARN);
}

static void draw_bar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float value, uint32_t color)
{
    uint16_t fill;

    if (value < 0.0f)
    {
        value = 0.0f;
    }
    if (value > 1.0f)
    {
        value = 1.0f;
    }

    fill = (uint16_t)((float)(w - 2U) * value);
    BSP_LCD_SetTextColor(COL_LINE);
    BSP_LCD_DrawRect(x, y, w, h);
    BSP_LCD_SetTextColor(COL_BG);
    BSP_LCD_FillRect((uint16_t)(x + 1U), (uint16_t)(y + 1U), (uint16_t)(w - 2U), (uint16_t)(h - 2U));
    BSP_LCD_SetTextColor(color);
    BSP_LCD_FillRect((uint16_t)(x + 1U), (uint16_t)(y + 1U), fill, (uint16_t)(h - 2U));
}

static void draw_motor_bar(uint16_t x, uint16_t y, const char *label, float velocity, uint32_t duty)
{
    char buf[32];
    float mag = fabsf(velocity) / 600.0f;
    long velocity_rounded;

    BSP_LCD_SetFont(&Font12);
    draw_text(x, y, label, COL_TEXT);
    draw_bar((uint16_t)(x + 38U), (uint16_t)(y + 2U), 150U, 14U, mag, COL_ACCENT);
    if (velocity < 0.0f)
    {
        velocity_rounded = -(long)(-velocity + 0.5f);
    }
    else
    {
        velocity_rounded = (long)(velocity + 0.5f);
    }
    snprintf(buf, sizeof(buf), "%+5ld", velocity_rounded);
    draw_text((uint16_t)(x + 198U), y, buf, COL_TEXT);
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)duty);
    draw_text((uint16_t)(x + 286U), y, buf, COL_MUTED);
}

static void draw_button(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *label, uint32_t color)
{
    BSP_LCD_SetTextColor(color);
    BSP_LCD_DrawRect(x, y, w, h);
    BSP_LCD_SetFont(&Font16);
    draw_text((uint16_t)(x + 14U), (uint16_t)(y + 12U), label, COL_TEXT);
}

static void fmt_fixed(char *buf,
                      size_t len,
                      const char *label,
                      float value,
                      uint8_t decimals,
                      uint8_t show_plus)
{
    uint32_t scale = (decimals == 1U) ? 10U : 100U;
    uint32_t scaled;
    uint32_t whole;
    uint32_t frac;
    char sign = ' ';

    if (value < 0.0f)
    {
        sign = '-';
        value = -value;
    }
    else if (show_plus)
    {
        sign = '+';
    }

    scaled = (uint32_t)(value * (float)scale + 0.5f);
    whole = scaled / scale;
    frac = scaled % scale;

    if (decimals == 1U)
    {
        snprintf(buf, len, "%s%c%lu.%01lu", label, sign,
                 (unsigned long)whole, (unsigned long)frac);
    }
    else
    {
        snprintf(buf, len, "%s%c%lu.%02lu", label, sign,
                 (unsigned long)whole, (unsigned long)frac);
    }
}

static void fmt_home_rpy(char *buf, size_t len, float roll, float pitch, float yaw)
{
    uint32_t rs = (uint32_t)(fabsf(roll) * 10.0f + 0.5f);
    uint32_t ps = (uint32_t)(fabsf(pitch) * 10.0f + 0.5f);
    uint32_t ys = (uint32_t)(fabsf(yaw) * 10.0f + 0.5f);

    snprintf(buf, len, "R%c%lu.%01lu P%c%lu.%01lu Y%c%lu.%01lu",
             (roll < 0.0f) ? '-' : '+', (unsigned long)(rs / 10U), (unsigned long)(rs % 10U),
             (pitch < 0.0f) ? '-' : '+', (unsigned long)(ps / 10U), (unsigned long)(ps % 10U),
             (yaw < 0.0f) ? '-' : '+', (unsigned long)(ys / 10U), (unsigned long)(ys % 10U));
}

static void fmt_risk_line(char *buf, size_t len, const char *label, float value)
{
    uint32_t scaled = (uint32_t)(value * 100.0f + 0.5f);
    snprintf(buf, len, "%s%lu.%02lu", label,
             (unsigned long)(scaled / 100U), (unsigned long)(scaled % 100U));
}

static void fmt_ros_sample(char *buf, size_t len)
{
    if (g_ros_rx_sample_len == 0u)
    {
        snprintf(buf, len, "----");
    }
    else
    {
        snprintf(buf, len, "%02X%02X%02X%02X",
                 (unsigned int)g_ros_rx_sample[0],
                 (unsigned int)g_ros_rx_sample[1],
                 (unsigned int)g_ros_rx_sample[2],
                 (unsigned int)g_ros_rx_sample[3]);
    }
}

static uint8_t point_in_rect(uint16_t px, uint16_t py,
                             uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    return (px >= x) && (px < (uint16_t)(x + w)) &&
           (py >= y) && (py < (uint16_t)(y + h));
}

static void poll_touch(void)
{
    TS_StateTypeDef state;
    uint8_t is_down = 0;

    if (!g_touch_ok)
    {
        return;
    }

    if (BSP_TS_GetState(&state) != TS_OK)
    {
        return;
    }

    is_down = (state.touchDetected > 0U) ? 1U : 0U;
    if (is_down && !g_touch_was_down)
    {
        uint16_t x = state.touchX[0];
        uint16_t y = state.touchY[0];

        if ((g_page == LCD_PAGE_HOME) && point_in_rect(x, y, IMU_X, IMU_Y, IMU_W, IMU_H))
        {
            g_page = LCD_PAGE_IMU;
            g_drawn_page = (LcdPage_t)0xff;
        }
        else if ((g_page == LCD_PAGE_HOME) && point_in_rect(x, y, MOTOR_BTN_X, MOTOR_BTN_Y, MOTOR_BTN_W, MOTOR_BTN_H))
        {
            g_page = LCD_PAGE_MOTORS;
            g_drawn_page = (LcdPage_t)0xff;
        }
        else if ((g_page != LCD_PAGE_HOME) && point_in_rect(x, y, BACK_X, BACK_Y, BACK_W, BACK_H))
        {
            g_page = LCD_PAGE_HOME;
            g_drawn_page = (LcdPage_t)0xff;
        }
    }

    g_touch_was_down = is_down;
}

static void draw_home_static(void)
{
    BSP_LCD_Clear(COL_BG);
    BSP_LCD_SetFont(&Font16);
    draw_text(8, 8, "TERRAIN BOT", COL_TEXT);
    draw_text(8, 36, "STATUS", COL_TEXT);

    draw_panel(8, 68, 308, 150, "Links");
    draw_button(IMU_X, IMU_Y, IMU_W, IMU_H, "IMU", COL_ACCENT);
    draw_button(MOTOR_BTN_X, MOTOR_BTN_Y, MOTOR_BTN_W, MOTOR_BTN_H, "MOTOR", COL_ACCENT);
    draw_panel(328, 198, 136, 20, "Touch");

    BSP_LCD_SetTextColor(COL_LINE);
    BSP_LCD_DrawHLine(0, NAV_Y, LCD_W);
}

static void draw_imu_static(void)
{
    BSP_LCD_Clear(COL_BG);
    BSP_LCD_SetFont(&Font16);
    draw_text(8, 8, "TERRAIN BOT", COL_TEXT);
    draw_text(8, 42, "IMU DATA", COL_TEXT);

    draw_panel(8, 72, 150, 142, "AHRS");
    draw_panel(166, 72, 150, 142, "Accel/Gyro");
    draw_panel(324, 72, 146, 142, "Mag/State");

    BSP_LCD_SetTextColor(COL_LINE);
    BSP_LCD_DrawHLine(0, NAV_Y, LCD_W);
    draw_button(BACK_X, BACK_Y, BACK_W, BACK_H, "< BACK", COL_LINE);
}

static uint32_t risk_color(float risk, uint8_t emergency_stop)
{
    if (emergency_stop || risk >= 0.75f)
    {
        return COL_BAD;
    }
    if (risk >= 0.35f)
    {
        return COL_WARN;
    }
    return COL_FILL;
}

static const char *risk_label(float risk, uint8_t emergency_stop)
{
    if (emergency_stop || risk >= 0.75f)
    {
        return emergency_stop ? "EMERGENCY STOP" : "STOP";
    }
    if (risk >= 0.35f)
    {
        return "MODERATE";
    }
    return "SAFE";
}

static void draw_motor_static(void)
{
    BSP_LCD_Clear(COL_BG);
    BSP_LCD_SetFont(&Font16);
    draw_text(8, 8, "TERRAIN BOT", COL_TEXT);
    draw_text(8, 42, "MOTORS", COL_TEXT);

    BSP_LCD_SetFont(&Font12);
    draw_panel(8, 72, 460, 142, "Velocity mm/s                 PWM");

    BSP_LCD_SetTextColor(COL_LINE);
    BSP_LCD_DrawHLine(0, NAV_Y, LCD_W);
    draw_button(BACK_X, BACK_Y, BACK_W, BACK_H, "< BACK", COL_LINE);
}

static void ensure_page_static(void)
{
    if (g_drawn_page == g_page)
    {
        return;
    }

    if (g_page == LCD_PAGE_HOME)
    {
        draw_home_static();
    }
    else if (g_page == LCD_PAGE_IMU)
    {
        draw_imu_static();
    }
    else
    {
        draw_motor_static();
    }

    g_drawn_page = g_page;
}

void LCD_Display_Init(void)
{
    g_page = LCD_PAGE_HOME;
    g_drawn_page = (LcdPage_t)0xff;
    g_touch_was_down = 0;
    g_touch_ok = (BSP_TS_Init(LCD_W, LCD_H) == TS_OK) ? 1U : 0U;
    ensure_page_static();
}

void LCD_Display_BootStatus(uint8_t imu_ok, uint8_t esp32_ok, uint8_t ros_ok)
{
    g_drawn_page = (LcdPage_t)0xff;
    BSP_LCD_Clear(COL_BG);
    BSP_LCD_SetFont(&Font16);
    draw_text(8, 8, "TERRAIN BOT", COL_TEXT);
    BSP_LCD_SetFont(&Font12);
    draw_text(20, 58, "Startup check", COL_TEXT);
    draw_text(20, 92, imu_ok ? "IMU: OK" : "IMU: FAIL",
              imu_ok ? COL_OK : COL_WARN);
    draw_text(20, 116, esp32_ok ? "ESP32: OK" : "ESP32: FAIL",
              esp32_ok ? COL_OK : COL_WARN);
    draw_text(20, 140, ros_ok ? "ROS: OK" : "ROS: WAIT",
              ros_ok ? COL_OK : COL_WARN);
    draw_text(20, 184, "Missing devices allowed", COL_MUTED);
}

void LCD_Display_BootMotorTest(const char *label)
{
    char buf[32];

    g_drawn_page = (LcdPage_t)0xff;
    BSP_LCD_Clear(COL_BG);
    BSP_LCD_SetFont(&Font16);
    draw_text(8, 8, "TERRAIN BOT", COL_TEXT);
    BSP_LCD_SetFont(&Font12);
    draw_text(20, 58, "Startup motor test", COL_TEXT);
    snprintf(buf, sizeof(buf), "TESTING: %s", label);
    draw_text(20, 96, buf, COL_ACCENT);
    draw_text(20, 140, "Check which wheel moves", COL_MUTED);
}

void LCD_Display_Update(const Madgwick_t *ahrs,
                        const ICM20948_Data *imu,
                        uint8_t calib_done,
                        float risk,
                        uint8_t emergency_stop,
                        uint8_t imu_ok,
                        uint8_t esp32_ok,
                        uint8_t wheels_ok,
                        uint8_t ros_ok,
                        uint8_t host_ok)
{
    char buf[48];
    float roll = ahrs->roll * (180.0f / 3.14159265f);
    float pitch = ahrs->pitch * (180.0f / 3.14159265f);
    float yaw = ahrs->yaw * (180.0f / 3.14159265f);
    float accel_mag = sqrtf(imu->ax * imu->ax + imu->ay * imu->ay + imu->az * imu->az);
    uint32_t state_color = risk_color(risk, emergency_stop);
    const char *imu_state = imu_ok ? (calib_done ? "CONNECTED" : "CALIBRATING") : "NOT CONNECTED";

    poll_touch();
    ensure_page_static();
    BSP_LCD_SetFont(&Font12);

    if (g_page == LCD_PAGE_HOME)
    {
        clear_area(18, 84, 298, 136);
        clear_area(270, 8, 198, 48);

        BSP_LCD_SetTextColor(state_color);
        BSP_LCD_FillRect(270, 10, 194U, 26U);
        BSP_LCD_SetFont(&Font12);
        draw_text_bg(280, 16, risk_label(risk, emergency_stop), LCD_COLOR_BLACK, state_color);

        BSP_LCD_SetFont(&Font12);
        snprintf(buf, sizeof(buf), "IMU   : %s", imu_state);
        draw_text(18, 86, buf, imu_ok ? COL_FILL : COL_BAD);
        if (ros_ok)
        {
            snprintf(buf, sizeof(buf), "ROS   : CONNECTED");
        }
        else if (g_debug_stage >= 200u)
        {
            char sample[9];

            fmt_ros_sample(sample, sizeof(sample));
            snprintf(buf, sizeof(buf), "R%lu S%lu E%ld T%lu R%lu %s",
                     (unsigned long)g_ros_retry_count,
                     (unsigned long)g_debug_stage, (long)g_ros_error,
                     (unsigned long)g_ros_tx_bytes,
                     (unsigned long)g_ros_rx_bytes,
                     sample);
        }
        else
        {
            snprintf(buf, sizeof(buf), "ROS   : WAITING");
        }
        draw_text(18, 108, buf, ros_ok ? COL_FILL : COL_WARN);
        snprintf(buf, sizeof(buf), "ESP32 : %s", esp32_ok ? "CONNECTED" : "NOT CONNECTED");
        draw_text(18, 130, buf, esp32_ok ? COL_FILL : COL_BAD);
        snprintf(buf, sizeof(buf), "WHEEL : %s", wheels_ok ? "CONNECTED" : "NOT CONNECTED");
        draw_text(18, 152, buf, wheels_ok ? COL_FILL : COL_WARN);
        fmt_home_rpy(buf, sizeof(buf), roll, pitch, yaw);
        draw_text(18, 182, buf, imu_ok ? COL_TEXT : COL_WARN);
        fmt_fixed(buf, sizeof(buf), "|A| ", accel_mag, 2U, 0U);
        draw_text(240, 86, buf, COL_ACCENT);
        fmt_risk_line(buf, sizeof(buf), "Risk ", risk);
        draw_text(240, 108, buf, state_color);
        draw_bar(240, 130, 74U, 16U, risk, state_color);
        snprintf(buf, sizeof(buf), "%s %lu",
                 host_ok ? "CMD OK" : "NO CMD",
                 (unsigned long)g_debug_stage);
        draw_text(338, 202, buf, host_ok ? COL_FILL : COL_WARN);
    }
    else if (g_page == LCD_PAGE_IMU)
    {
        clear_area(14, 92, 138, 112);
        clear_area(172, 92, 138, 112);
        clear_area(330, 92, 134, 112);

        BSP_LCD_SetTextColor(state_color);
        BSP_LCD_FillRect(238, 6, 230U, 24U);
        BSP_LCD_SetFont(&Font16);
        draw_text_bg(248, 10, risk_label(risk, emergency_stop), LCD_COLOR_BLACK, state_color);

        BSP_LCD_SetFont(&Font12);
        fmt_fixed(buf, sizeof(buf), "Roll  ", roll, 2U, 1U);
        draw_text(18, 96, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "Pitch ", pitch, 2U, 1U);
        draw_text(18, 116, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "Yaw   ", yaw, 2U, 1U);
        draw_text(18, 136, buf, COL_TEXT);
        draw_text(18, 160, calib_done ? "AHRS RUN" : "AHRS CAL", calib_done ? COL_FILL : COL_WARN);
        fmt_fixed(buf, sizeof(buf), "Temp ", imu->temp_c, 1U, 0U);
        snprintf(&buf[0] + strlen(buf), sizeof(buf) - strlen(buf), " C");
        draw_text(18, 184, buf, COL_MUTED);

        fmt_fixed(buf, sizeof(buf), "AX ", imu->ax, 2U, 1U);
        draw_text(176, 96, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "AY ", imu->ay, 2U, 1U);
        draw_text(176, 112, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "AZ ", imu->az, 2U, 1U);
        draw_text(176, 128, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "|A|", accel_mag, 2U, 0U);
        draw_text(176, 144, buf, COL_ACCENT);
        fmt_fixed(buf, sizeof(buf), "GX ", imu->gx, 2U, 1U);
        draw_text(176, 166, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "GY ", imu->gy, 2U, 1U);
        draw_text(176, 182, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "GZ ", imu->gz, 2U, 1U);
        draw_text(176, 198, buf, COL_TEXT);

        fmt_fixed(buf, sizeof(buf), "MX ", imu->mx, 1U, 1U);
        draw_text(334, 96, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "MY ", imu->my, 1U, 1U);
        draw_text(334, 112, buf, COL_TEXT);
        fmt_fixed(buf, sizeof(buf), "MZ ", imu->mz, 1U, 1U);
        draw_text(334, 128, buf, COL_TEXT);
        fmt_risk_line(buf, sizeof(buf), "Risk ", risk);
        draw_text(334, 152, buf, state_color);
        draw_bar(334, 172, 112U, 14U, risk, state_color);
        draw_state_box(334, 188, "ROS", ros_ok);
        draw_state_box(334, 204, "HOST", host_ok);
    }
    else
    {
        clear_area(18, 100, 432, 104);
        if (esp32_ok && wheels_ok)
        {
            draw_motor_bar(28, 104, "FL", Encoder_VelMmps(ENC_FL), Motor_GetDuty(MOTOR_FL));
            draw_motor_bar(28, 130, "FR", Encoder_VelMmps(ENC_FR), Motor_GetDuty(MOTOR_FR));
            draw_motor_bar(28, 156, "RL", Encoder_VelMmps(ENC_RL), Motor_GetDuty(MOTOR_RL));
            draw_motor_bar(28, 182, "RR", Encoder_VelMmps(ENC_RR), Motor_GetDuty(MOTOR_RR));
        }
        else
        {
            draw_text(28, 118, "ENCODER FEEDBACK UNAVAILABLE", COL_WARN);
            draw_text(28, 146, esp32_ok ? "ESP32 LINK OK" : "ESP32 LINK DOWN",
                      esp32_ok ? COL_MUTED : COL_BAD);
            draw_text(28, 170, wheels_ok ? "WHEEL DATA OK" : "WHEEL DATA INVALID",
                      wheels_ok ? COL_MUTED : COL_BAD);
        }
    }

}
