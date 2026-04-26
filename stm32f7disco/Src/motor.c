#include "motor.h"
#include "encoder.h"
#include <math.h>
#include <string.h>

static TIM_HandleTypeDef *g_tim10;
static TIM_HandleTypeDef *g_tim11;
static TIM_HandleTypeDef *g_tim12;
static TIM_HandleTypeDef *g_tim13;
static TIM_HandleTypeDef *g_tim14;
static TIM_HandleTypeDef *g_tim2;
static TIM_HandleTypeDef *g_tim3;

static uint8_t g_ready;
static int8_t g_dir[MOTOR_NUM];
static uint32_t g_current_duty[MOTOR_NUM];
/*
 * Per-wheel trim derived from the known-good startup test duties.
 * FR and RR are used as the baseline; FL and RL are scaled down to
 * match their observed breakaway/drive strength.
 */
static const uint8_t g_trim_percent[MOTOR_NUM] = {94u, 95u, 100u, 98u};

static void set_pwm(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t duty)
{
    __HAL_TIM_SET_COMPARE(htim, channel, duty);
}

static uint32_t timer_safe_max_duty(TIM_HandleTypeDef *htim)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(htim);

    /* Leave a little headroom below ARR for consistent PWM behavior. */
    return (arr * 85u) / 100u;
}

static uint32_t max_duty_for_motor(Motor_ID id)
{
    switch (id) {
    case MOTOR_FL:
        return timer_safe_max_duty(g_tim10);
    case MOTOR_FR:
        return timer_safe_max_duty(g_tim13);
    case MOTOR_RL:
        return timer_safe_max_duty(g_tim12);
    case MOTOR_RR: {
        uint32_t tim2_max = timer_safe_max_duty(g_tim2);
        uint32_t tim3_max = timer_safe_max_duty(g_tim3);
        return tim2_max < tim3_max ? tim2_max : tim3_max;
    }
    default:
        return 0u;
    }
}

static uint32_t apply_trim(Motor_ID id, uint32_t duty)
{
    if ((uint8_t)id >= MOTOR_NUM) {
        return duty;
    }

    return (duty * g_trim_percent[id]) / 100u;
}

static uint32_t duty_from_percent(Motor_ID id, uint8_t duty_percent)
{
    uint32_t max_duty = max_duty_for_motor(id);

    if (duty_percent > 100u) {
        duty_percent = 100u;
    }

    return ((uint32_t)duty_percent * max_duty) / 100u;
}

static uint32_t clamp_duty(Motor_ID id, uint32_t duty)
{
    uint32_t max_duty = max_duty_for_motor(id);

    return duty > max_duty ? max_duty : duty;
}

static uint32_t ramp_duty(uint32_t current, uint32_t target)
{
    uint32_t step;

    if (target > current) {
        step = target - current;
        if (step > MOTOR_RAMP_STEP) {
            step = MOTOR_RAMP_STEP;
        }
        return current + step;
    }

    if (target < current) {
        step = current - target;
        if (step > MOTOR_RAMP_STEP) {
            step = MOTOR_RAMP_STEP;
        }
        return current - step;
    }

    return current;
}

static void write_motor_channels(Motor_ID id, uint32_t duty, uint8_t forward)
{
    switch (id) {
    case MOTOR_FL:
        set_pwm(g_tim10, TIM_CHANNEL_1, forward ? 0u : duty);
        set_pwm(g_tim11, TIM_CHANNEL_1, forward ? duty : 0u);
        break;
    case MOTOR_FR:
        set_pwm(g_tim13, TIM_CHANNEL_1, forward ? duty : 0u);
        set_pwm(g_tim14, TIM_CHANNEL_1, forward ? 0u : duty);
        break;
    case MOTOR_RL:
        set_pwm(g_tim12, TIM_CHANNEL_1, forward ? duty : 0u);
        set_pwm(g_tim12, TIM_CHANNEL_2, forward ? 0u : duty);
        break;
    case MOTOR_RR:
        set_pwm(g_tim3, TIM_CHANNEL_1, forward ? 0u : duty);
        set_pwm(g_tim2, TIM_CHANNEL_1, forward ? duty : 0u);
        break;
    default:
        break;
    }
}

static void set_motor_raw(Motor_ID id, uint32_t duty, uint8_t forward)
{
    duty = clamp_duty(id, duty);
    duty = ramp_duty(g_current_duty[id], duty);
    g_current_duty[id] = duty;
    Encoder_SetDirection((uint8_t)id, forward);
    write_motor_channels(id, duty, forward);
}

void Motor_Init(TIM_HandleTypeDef *htim10,
                TIM_HandleTypeDef *htim11,
                TIM_HandleTypeDef *htim12,
                TIM_HandleTypeDef *htim13,
                TIM_HandleTypeDef *htim14,
                TIM_HandleTypeDef *htim2,
                TIM_HandleTypeDef *htim3)
{
    g_tim10 = htim10;
    g_tim11 = htim11;
    g_tim12 = htim12;
    g_tim13 = htim13;
    g_tim14 = htim14;
    g_tim2 = htim2;
    g_tim3 = htim3;

    HAL_TIM_PWM_Start(g_tim10, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(g_tim11, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(g_tim12, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(g_tim12, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(g_tim13, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(g_tim14, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(g_tim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(g_tim3, TIM_CHANNEL_1);

    memset(g_current_duty, 0, sizeof(g_current_duty));
    memset(g_dir, 0, sizeof(g_dir));
    g_ready = 1u;
    Motor_StopAll();
}

void Motor_Set(Motor_ID id, float vel_mmps)
{
    float abs_vel;
    uint32_t duty;
    uint32_t max_duty;
    uint8_t forward;

    if (!g_ready || (uint8_t)id >= MOTOR_NUM) {
        return;
    }

    if (vel_mmps > MOTOR_MAX_SPEED_MMPS) {
        vel_mmps = MOTOR_MAX_SPEED_MMPS;
    }
    if (vel_mmps < -MOTOR_MAX_SPEED_MMPS) {
        vel_mmps = -MOTOR_MAX_SPEED_MMPS;
    }

    forward = vel_mmps >= 0.0f ? 1u : 0u;
    abs_vel = fabsf(vel_mmps);
    max_duty = max_duty_for_motor(id);
    max_duty = (max_duty * MOTOR_COMMAND_DUTY_LIMIT_PCT) / 100u;
    duty = (uint32_t)((abs_vel / MOTOR_MAX_SPEED_MMPS) * (float)max_duty);
    duty = apply_trim(id, duty);

    g_dir[id] = vel_mmps > 0.0f ? 1 : (vel_mmps < 0.0f ? -1 : 0);
    set_motor_raw(id, duty, forward);
}

void Motor_SetTestDuty(Motor_ID id, uint8_t duty_percent, uint8_t forward)
{
    uint32_t duty;

    if (!g_ready || (uint8_t)id >= MOTOR_NUM) {
        return;
    }

    duty = duty_from_percent(id, duty_percent);
    duty = clamp_duty(id, duty);

    g_current_duty[id] = duty;
    g_dir[id] = (duty_percent == 0u) ? 0 : (forward ? 1 : -1);
    Encoder_SetDirection((uint8_t)id, forward);
    write_motor_channels(id, duty, forward);
}

void Motor_StopAll(void)
{
    if (!g_ready) {
        return;
    }

    set_pwm(g_tim10, TIM_CHANNEL_1, 0);
    set_pwm(g_tim11, TIM_CHANNEL_1, 0);
    set_pwm(g_tim12, TIM_CHANNEL_1, 0);
    set_pwm(g_tim12, TIM_CHANNEL_2, 0);
    set_pwm(g_tim13, TIM_CHANNEL_1, 0);
    set_pwm(g_tim14, TIM_CHANNEL_1, 0);
    set_pwm(g_tim2, TIM_CHANNEL_1, 0);
    set_pwm(g_tim3, TIM_CHANNEL_1, 0);

    memset(g_current_duty, 0, sizeof(g_current_duty));
    memset(g_dir, 0, sizeof(g_dir));
}

int8_t Motor_GetDirection(Motor_ID id)
{
    return (uint8_t)id < MOTOR_NUM ? g_dir[id] : 0;
}

uint32_t Motor_GetDuty(Motor_ID id)
{
    return (uint8_t)id < MOTOR_NUM ? g_current_duty[id] : 0u;
}
