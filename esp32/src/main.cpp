#include <Arduino.h>
#include <Wire.h>
#include "driver/pcnt.h"

#define I2C_SLAVE_ADDR 0x30
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_FREQ 400000

#define ENC_NUM 4
#define REG_READ_LEN 36

#define STATUS_ESP32_ALIVE   (1u << 0)
#define STATUS_WHEELS_VALID  (1u << 1)

/*
 * RMCS-3070 gearmotor encoder defaults. Adjust these to match the exact motor
 * variant and the STM32 odometry constants. PCNT mode below counts both edges
 * of channel A, so effective ticks per wheel revolution are encoder CPR * 2
 * after gearbox multiplication.
 */
#define WHEEL_DIAMETER_MM 85.0f
#define ENCODER_TICKS_PER_REV 2264.0f

static const int ENC_A_PINS[ENC_NUM] = {16, 18, 25, 32};
static const int ENC_B_PINS[ENC_NUM] = {17, 19, 26, 33};
static const pcnt_unit_t PCNT_UNITS[ENC_NUM] = {
    PCNT_UNIT_0, PCNT_UNIT_1, PCNT_UNIT_2, PCNT_UNIT_3
};

static uint8_t g_tx_buf[REG_READ_LEN];
static volatile uint32_t g_request_count;
static volatile uint32_t g_receive_count;
static uint8_t g_last_dirs[ENC_NUM];
static int32_t g_ticks[ENC_NUM];
static float g_speeds_mm_s[ENC_NUM];
static int16_t g_last_pcnt[ENC_NUM];
static uint32_t g_last_update_ms;
static portMUX_TYPE g_packet_mux = portMUX_INITIALIZER_UNLOCKED;

static void build_packet(void)
{
    /* STATUS_WHEELS_VALID set only after STM32 has polled at least once,
       confirming the I2C link is live and encoders are being read */
    const uint32_t status = STATUS_ESP32_ALIVE |
                            (g_request_count > 0u ? STATUS_WHEELS_VALID : 0u);

    portENTER_CRITICAL(&g_packet_mux);
    memcpy(&g_tx_buf[0], &status, sizeof(status));
    memcpy(&g_tx_buf[4], g_ticks, sizeof(g_ticks));
    memcpy(&g_tx_buf[20], g_speeds_mm_s, sizeof(g_speeds_mm_s));
    portEXIT_CRITICAL(&g_packet_mux);
}

static void configure_encoder_pcnt(int index)
{
    pcnt_config_t cfg = {};
    cfg.pulse_gpio_num = ENC_A_PINS[index];
    cfg.ctrl_gpio_num = ENC_B_PINS[index];
    cfg.channel = PCNT_CHANNEL_0;
    cfg.unit = PCNT_UNITS[index];
    cfg.pos_mode = PCNT_COUNT_INC;
    cfg.neg_mode = PCNT_COUNT_DEC;
    cfg.lctrl_mode = PCNT_MODE_REVERSE;
    cfg.hctrl_mode = PCNT_MODE_KEEP;
    cfg.counter_h_lim = 30000;
    cfg.counter_l_lim = -30000;

    pinMode(ENC_A_PINS[index], INPUT_PULLUP);
    pinMode(ENC_B_PINS[index], INPUT_PULLUP);

    pcnt_unit_config(&cfg);
    pcnt_set_filter_value(PCNT_UNITS[index], 1000);
    pcnt_filter_enable(PCNT_UNITS[index]);
    pcnt_counter_pause(PCNT_UNITS[index]);
    pcnt_counter_clear(PCNT_UNITS[index]);
    pcnt_counter_resume(PCNT_UNITS[index]);
}

static void update_encoder_state(void)
{
    const uint32_t now = millis();
    const uint32_t elapsed_ms = now - g_last_update_ms;

    if (elapsed_ms < 20u) {
        return;
    }

    const float dt_s = (float)elapsed_ms / 1000.0f;
    const float mm_per_tick =
        (PI * WHEEL_DIAMETER_MM) / ENCODER_TICKS_PER_REV;

    for (int i = 0; i < ENC_NUM; i++) {
        int16_t count = 0;
        pcnt_get_counter_value(PCNT_UNITS[i], &count);

        int32_t delta = (int32_t)count - (int32_t)g_last_pcnt[i];
        /* Handle PCNT counter wrap at ±30000 */
        if (delta > 15000)  delta -= 60000;
        if (delta < -15000) delta += 60000;
        g_last_pcnt[i] = count;

        /* Apply direction mapping from STM32: dir=1 means motor is reverse-mounted */
        if (g_last_dirs[i] != 0u) {
            delta = -delta;
        }

        g_ticks[i] += delta;
        g_speeds_mm_s[i] = ((float)delta * mm_per_tick) / dt_s;
    }

    g_last_update_ms = now;
    build_packet();
}

static void on_request(void)
{
    g_request_count++;
    portENTER_CRITICAL(&g_packet_mux);
    Wire.write(g_tx_buf, REG_READ_LEN);
    portEXIT_CRITICAL(&g_packet_mux);
}

static void on_receive(int num_bytes)
{
    g_receive_count++;

    for (int i = 0; i < ENC_NUM && Wire.available(); i++) {
        g_last_dirs[i] = Wire.read();
    }

    while (Wire.available()) {
        Wire.read();
    }

    (void)num_bytes;
}

void setup(void)
{
    Serial.begin(115200);
    delay(200);

    memset(g_last_dirs, 0, sizeof(g_last_dirs));
    memset(g_ticks, 0, sizeof(g_ticks));
    memset(g_speeds_mm_s, 0, sizeof(g_speeds_mm_s));
    memset(g_last_pcnt, 0, sizeof(g_last_pcnt));

    for (int i = 0; i < ENC_NUM; i++) {
        configure_encoder_pcnt(i);
    }

    g_last_update_ms = millis();
    build_packet();

    Wire.begin(I2C_SLAVE_ADDR, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ);
    Wire.onRequest(on_request);
    Wire.onReceive(on_receive);

    Serial.println("ESP32 PCNT I2C encoder slave");
    Serial.printf("addr=0x%02X SDA=GPIO%d SCL=GPIO%d\n",
                  I2C_SLAVE_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.printf("wheel_diameter_mm=%.2f ticks_per_rev=%.2f\n",
                  WHEEL_DIAMETER_MM, ENCODER_TICKS_PER_REV);
}

void loop(void)
{
    static uint32_t last_print = 0;

    update_encoder_state();

    if (millis() - last_print >= 1000u) {
        last_print = millis();
        Serial.printf("i2c requests=%lu receives=%lu dirs=%u,%u,%u,%u ticks=%ld,%ld,%ld,%ld speed=%.1f,%.1f,%.1f,%.1f\n",
                      (unsigned long)g_request_count,
                      (unsigned long)g_receive_count,
                      g_last_dirs[0], g_last_dirs[1],
                      g_last_dirs[2], g_last_dirs[3],
                      (long)g_ticks[0], (long)g_ticks[1],
                      (long)g_ticks[2], (long)g_ticks[3],
                      g_speeds_mm_s[0], g_speeds_mm_s[1],
                      g_speeds_mm_s[2], g_speeds_mm_s[3]);
    }

    delay(2);
}
