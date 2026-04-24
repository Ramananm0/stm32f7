#include "app.h"
#include "encoder.h"
#include "icm20948.h"
#include "i2c.h"
#include "lcd_display.h"
#include "madgwick.h"
#include "microros_transport.h"
#include "motor.h"
#include "main.h"
#include "usart.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_MICROROS
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>
#include <sensor_msgs/msg/imu.h>
#include <std_msgs/msg/float32_multi_array.h>
#include <std_msgs/msg/int32_multi_array.h>
#include <geometry_msgs/msg/twist.h>
#include <uxr/client/transport.h>
#endif

extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim13;
extern TIM_HandleTypeDef htim14;

#define IMU_MS       5u
#define ENC_MS       20u
#define ENC_RETRY_MS 500u
#define PID_MS       10u
#define PUB_IMU_MS   100u   /* 10 Hz — 115200 baud safe (3700 B/s) */
#define PUB_ENC_MS   200u   /* 5 Hz  — encoder/status, low priority */
#define LCD_MS      50u
#define I2C_RECOVER_MS 100u
#define ENC_FAIL_DISABLE_COUNT 3u
#define IMU_RETRY_MS 500u
#define ROS_RETRY_MS 500u
#define ROS_SPIN_MS  5u
#define CALIB_N     200u
#define UROS_PING_TIMEOUT_MS 300u
#define UROS_PING_ATTEMPTS   10u
#define UROS_STARTUP_DELAY_MS 500u
#define MOTOR_BOOT_TEST_RUN_MS     800u
#define MOTOR_BOOT_TEST_STOP_MS    300u
#define MOTOR_BOOT_NORMALIZE_DUTY_PERCENT 40u
#define MOTOR_BOOT_NORMALIZE_POLL_MS      50u
#define MOTOR_BOOT_NORMALIZE_TIMEOUT_MS  5000u
#define MOTOR_BOOT_NORMALIZE_TICKS        48
#define DIAG_ENC_MS               100u
#define DIAG_MOTOR_RUN_MS       15000u
#define DIAG_MOTOR_STOP_MS        500u
#define DIAG_MOTOR_DUTY_PERCENT    55u

#define MADGWICK_BETA   0.033f
#define CF_ALPHA        0.98f

#define S_W_PITCH   0.7f
#define S_W_ROLL    0.3f
#define S_SAFE      10.0f
#define S_CRIT      30.0f
#define S_W_INCL    0.7f
#define S_W_SHAKE   0.3f
#define S_LPF       0.90f
#define S_ESTOP_P   45.0f
#define S_ESTOP_R   55.0f
#define S_FLIP_RATE (120.0f * 0.017453293f)
#define S_FLIP_P    (25.0f * 0.017453293f)
#define S_ESTOP_CONFIRM 5u

#define KP          0.5f
#define KI          0.1f
#define ENC_VEL_LPF_ALPHA        0.65f
#define SYNC_PAIR_K              0.20f
#define SYNC_STRAIGHT_K          0.10f
#define SYNC_DEADBAND_MMPS       1.0f
#define TARGET_STOP_EPS_MMPS     0.5f
#define SLIP_MIN_TARGET_MMPS     6.0f
#define SLIP_TARGET_EXCESS_MMPS  8.0f
#define SLIP_PAIR_DELTA_MMPS     10.0f
#define SLIP_SIDE_DELTA_MMPS     8.0f
#define SLIP_RISK_LPF_ALPHA      0.80f
#define SLIP_ALERT_THRESHOLD     0.45f
#define WDG_MS     2000u
#define NORMAL_FALLBACK_SPEED_MMPS  22.0f
#define NORMAL_FALLBACK_RUN_MS    3000u
#define NORMAL_FALLBACK_STOP_MS    750u

#define GY_COV      (0.00262f * 0.00262f)
#define AC_COV      (0.02256f * 0.02256f)
#define OR_COV      0.0001f

#ifdef USE_MICROROS
#define UROS_TRY_STAGE(stage_code, fn) do { \
    rcl_ret_t rc__ = (fn); \
    g_debug_stage = (stage_code); \
    if (rc__ != RCL_RET_OK) { \
        g_ros_ok = 0u; \
        g_ros_error = (int32_t)rc__; \
        return; \
    } \
} while (0)

static rcl_allocator_t alloc;
static rclc_support_t support;
static rcl_node_t node;
static rclc_executor_t exec;
static rcl_publisher_t pub_imu;
static rcl_publisher_t pub_ticks;
static rcl_publisher_t pub_vel;
static rcl_publisher_t pub_status;
static rcl_subscription_t sub_cmd;

static sensor_msgs__msg__Imu msg_imu;
static std_msgs__msg__Int32MultiArray msg_ticks;
static std_msgs__msg__Float32MultiArray msg_vel;
static std_msgs__msg__Int32MultiArray msg_status;
static geometry_msgs__msg__Twist msg_cmd;

static int32_t tick_buf[MOTOR_NUM];
static float vel_buf[MOTOR_NUM];
static int32_t status_buf[5];
static char frame_id[] = "imu_link";
#endif

static ICM20948_Data imu;
static ICM20948_Calib calib;
static Madgwick_t ahrs;

static float g_tilt_risk;
static float g_slip_risk;
static float g_risk;
static float g_scale = 1.0f;
static float g_target[MOTOR_NUM];
static float g_integral[MOTOR_NUM];
static float g_measured_filt[MOTOR_NUM];
static float g_wheel_slip[MOTOR_NUM];
static uint32_t g_cmd_ms;
static uint32_t g_cmd_count;
static uint8_t g_esp32_ok;
static uint8_t g_wheels_ok;
static uint8_t g_encoder_feedback_ok;
static uint8_t g_host_ok;
static uint8_t g_imu_ok;
static uint8_t g_ros_ok;
static uint8_t g_emergency_stop;
static uint8_t g_estop_count;
static uint8_t g_slip_alert;
volatile int32_t g_ros_error;
volatile uint32_t g_ros_rx_bytes;
volatile uint32_t g_ros_tx_bytes;
volatile uint8_t g_ros_rx_sample[4];
volatile uint8_t g_ros_rx_sample_len;
volatile uint32_t g_ros_retry_count;

volatile uint32_t g_debug_stage;
volatile uint32_t g_debug_heartbeat;

#ifdef USE_MICROROS
static void cmd_cb(const void *msg_in)
{
    const geometry_msgs__msg__Twist *tw = (const geometry_msgs__msg__Twist *)msg_in;
    float vx = (float)tw->linear.x * 1000.0f;
    float wz = (float)tw->angular.z;
    float half_track = ENC_WHEELBASE_MM * 0.5f;
    float left = vx - wz * half_track;
    float right = vx + wz * half_track;

    g_target[MOTOR_FL] = left;
    g_target[MOTOR_RL] = left;
    g_target[MOTOR_FR] = right;
    g_target[MOTOR_RR] = right;
    g_cmd_ms = HAL_GetTick();
    g_cmd_count++;
    g_host_ok = 1u;
}
#endif

static float clamp01f(float value)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

static void update_combined_risk(void)
{
    float dominant_risk = g_tilt_risk > g_slip_risk ? g_tilt_risk : g_slip_risk;
    float one_minus_risk;

    g_risk = clamp01f(dominant_risk);
    one_minus_risk = 1.0f - g_risk;
    g_scale = one_minus_risk * one_minus_risk;
}

static void update_safety(void)
{
    float pitch_deg = ahrs.pitch * (180.0f / 3.14159265f);
    float roll_deg = ahrs.roll * (180.0f / 3.14159265f);
    float pitch_abs = fabsf(pitch_deg);
    float roll_abs = fabsf(roll_deg);
    float pitch_rate = fabsf(imu.gy - calib.gy_bias);
    float weighted;
    float r_incl;
    float accel_mag;
    float g_ref;
    float r_shake;
    float r_raw;

    if (pitch_abs >= S_ESTOP_P || roll_abs >= S_ESTOP_R ||
        (pitch_rate > S_FLIP_RATE && ahrs.pitch > S_FLIP_P)) {
        if (g_estop_count < S_ESTOP_CONFIRM) {
            g_estop_count++;
            return;
        }
        g_tilt_risk = 1.0f;
        g_scale = 0.0f;
        g_emergency_stop = 1u;
        Motor_StopAll();
        return;
    }
    g_estop_count = 0u;
    g_emergency_stop = 0u;

    weighted = S_W_PITCH * pitch_abs + S_W_ROLL * roll_abs;
    r_incl = (weighted - S_SAFE) / (S_CRIT - S_SAFE);
    if (r_incl < 0.0f) r_incl = 0.0f;
    if (r_incl > 1.0f) r_incl = 1.0f;

    accel_mag = sqrtf(imu.ax * imu.ax + imu.ay * imu.ay + imu.az * imu.az);
    g_ref = calib.gravity > 9.0f ? calib.gravity : 9.80665f;
    r_shake = fabsf(accel_mag - g_ref) / g_ref;
    if (r_shake > 1.0f) r_shake = 1.0f;

    r_raw = S_W_INCL * r_incl + S_W_SHAKE * r_shake;
    if (r_raw > 1.0f) r_raw = 1.0f;

    g_tilt_risk = S_LPF * g_tilt_risk + (1.0f - S_LPF) * r_raw;
    update_combined_risk();
}

static void update_slip_risk(void)
{
    float side_avg[2];
    uint8_t active_wheels = 0u;
    float slip_sum = 0.0f;

    if (!g_encoder_feedback_ok) {
        memset(g_wheel_slip, 0, sizeof(g_wheel_slip));
        g_slip_risk = 0.0f;
        g_slip_alert = 0u;
        update_combined_risk();
        return;
    }

    side_avg[0] = 0.5f * (fabsf(g_measured_filt[MOTOR_FL]) + fabsf(g_measured_filt[MOTOR_RL]));
    side_avg[1] = 0.5f * (fabsf(g_measured_filt[MOTOR_FR]) + fabsf(g_measured_filt[MOTOR_RR]));

    for (uint8_t i = 0; i < MOTOR_NUM; i++) {
        float target_mag = fabsf(g_target[i] * g_scale);
        float measured_mag = fabsf(g_measured_filt[i]);
        float side_mag = (i == MOTOR_FL || i == MOTOR_RL) ? side_avg[0] : side_avg[1];
        float target_excess;
        float pair_mismatch;
        float wheel_slip;

        if (target_mag < SLIP_MIN_TARGET_MMPS) {
            g_wheel_slip[i] = 0.0f;
            continue;
        }

        active_wheels++;
        target_excess = clamp01f((measured_mag - target_mag) / SLIP_TARGET_EXCESS_MMPS);
        pair_mismatch = clamp01f(fabsf(measured_mag - side_mag) / SLIP_PAIR_DELTA_MMPS);
        wheel_slip = (0.6f * target_excess) + (0.4f * pair_mismatch);
        g_wheel_slip[i] = clamp01f(wheel_slip);
        slip_sum += g_wheel_slip[i];
    }

    if (active_wheels == 0u) {
        memset(g_wheel_slip, 0, sizeof(g_wheel_slip));
        g_slip_risk = 0.0f;
        g_slip_alert = 0u;
        update_combined_risk();
        return;
    }

    {
        float side_delta = clamp01f(fabsf(side_avg[0] - side_avg[1]) / SLIP_SIDE_DELTA_MMPS);
        float slip_mean = slip_sum / (float)active_wheels;
        float raw_slip_risk = clamp01f((0.7f * slip_mean) + (0.3f * side_delta));
        g_slip_risk = (SLIP_RISK_LPF_ALPHA * g_slip_risk) +
                      ((1.0f - SLIP_RISK_LPF_ALPHA) * raw_slip_risk);
    }

    g_slip_alert = g_slip_risk >= SLIP_ALERT_THRESHOLD ? 1u : 0u;
    update_combined_risk();
}

static void motor_pid(float dt)
{
    float side_avg[2];
    float straight_avg = 0.0f;
    uint8_t straight_mode;

    if (!g_encoder_feedback_ok) {
        memset(g_integral, 0, sizeof(g_integral));
        memset(g_measured_filt, 0, sizeof(g_measured_filt));
        for (uint8_t i = 0; i < MOTOR_NUM; i++) {
            Motor_Set((Motor_ID)i, g_target[i] * g_scale);
        }
        return;
    }

    for (uint8_t i = 0; i < MOTOR_NUM; i++) {
        float measured = Encoder_VelMmps(i);
        g_measured_filt[i] = (ENC_VEL_LPF_ALPHA * g_measured_filt[i]) +
                             ((1.0f - ENC_VEL_LPF_ALPHA) * measured);
    }

    update_slip_risk();

    side_avg[0] = 0.5f * (g_measured_filt[MOTOR_FL] + g_measured_filt[MOTOR_RL]);
    side_avg[1] = 0.5f * (g_measured_filt[MOTOR_FR] + g_measured_filt[MOTOR_RR]);
    straight_mode =
        (fabsf(g_target[MOTOR_FL] - g_target[MOTOR_FR]) < 2.0f) &&
        ((g_target[MOTOR_FL] * g_target[MOTOR_FR]) >= 0.0f);

    if (straight_mode) {
        for (uint8_t i = 0; i < MOTOR_NUM; i++) {
            straight_avg += g_measured_filt[i];
        }
        straight_avg *= 0.25f;
    }

    for (uint8_t i = 0; i < MOTOR_NUM; i++) {
        float target = g_target[i] * g_scale;
        float measured = g_measured_filt[i];
        float pair_error;
        float sync_term = 0.0f;
        float error = target - measured;
        float command;

        if (i == MOTOR_FL || i == MOTOR_RL) {
            pair_error = side_avg[0] - measured;
        } else {
            pair_error = side_avg[1] - measured;
        }

        if (fabsf(pair_error) >= SYNC_DEADBAND_MMPS) {
            sync_term += SYNC_PAIR_K * pair_error;
        }

        if (straight_mode) {
            float straight_error = straight_avg - measured;
            if (fabsf(straight_error) >= SYNC_DEADBAND_MMPS) {
                sync_term += SYNC_STRAIGHT_K * straight_error;
            }
        }

        if (fabsf(target) < TARGET_STOP_EPS_MMPS) {
            g_integral[i] = 0.0f;
            Motor_Set((Motor_ID)i, 0.0f);
            continue;
        }

        g_integral[i] += error * dt;
        if (g_integral[i] > MOTOR_ILIMIT) g_integral[i] = MOTOR_ILIMIT;
        if (g_integral[i] < -MOTOR_ILIMIT) g_integral[i] = -MOTOR_ILIMIT;

        command = target + KP * error + KI * g_integral[i] + sync_term;
        Motor_Set((Motor_ID)i, command);
    }
}

static void refresh_encoder_status(void)
{
    if (Encoder_Update() == HAL_OK) {
        g_esp32_ok = Encoder_Esp32Connected();
        g_wheels_ok = Encoder_WheelsConnected();
        g_encoder_feedback_ok = (g_esp32_ok && g_wheels_ok) ? 1u : 0u;
    } else {
        g_esp32_ok = 0u;
        g_wheels_ok = 0u;
        g_encoder_feedback_ok = 0u;
    }
}

static void update_normal_mode_targets(uint32_t now)
{
    (void)now;

    if (g_ros_ok) {
        return;
    }

    if (g_emergency_stop) {
        memset(g_target, 0, sizeof(g_target));
        return;
    }

    /* Keep the rover stationary until ROS is online and sending commands. */
    memset(g_target, 0, sizeof(g_target));
}

static void startup_motor_check(void)
{
    int32_t baseline[MOTOR_NUM];
    uint32_t start_ms;
    uint32_t poll_ms;
    uint8_t motion_ok = 0u;

    refresh_encoder_status();
    for (uint8_t i = 0; i < MOTOR_NUM; i++) {
        baseline[i] = Encoder_Ticks(i);
        Motor_SetTestDuty((Motor_ID)i, MOTOR_BOOT_NORMALIZE_DUTY_PERCENT, 1u);
    }

    LCD_Display_BootMotorTest("ENC NORM FWD");
    start_ms = HAL_GetTick();
    poll_ms = start_ms;

    while ((HAL_GetTick() - start_ms) < MOTOR_BOOT_NORMALIZE_TIMEOUT_MS) {
        uint32_t now = HAL_GetTick();
        uint8_t moved = 1u;

        if (now - poll_ms < MOTOR_BOOT_NORMALIZE_POLL_MS) {
            HAL_Delay(5u);
            continue;
        }

        poll_ms = now;
        refresh_encoder_status();
        if (!g_encoder_feedback_ok) {
            continue;
        }

        for (uint8_t i = 0; i < MOTOR_NUM; i++) {
            if (labs(Encoder_Ticks(i) - baseline[i]) < MOTOR_BOOT_NORMALIZE_TICKS) {
                moved = 0u;
                break;
            }
        }

        if (moved) {
            motion_ok = 1u;
            break;
        }
    }

    Motor_StopAll();
    HAL_Delay(MOTOR_BOOT_TEST_STOP_MS);
    refresh_encoder_status();
    LCD_Display_BootMotorTest(motion_ok ? "ENC NORM OK" : "ENC NORM FAIL");
    HAL_Delay(MOTOR_BOOT_TEST_STOP_MS);
}

#ifdef USE_MICROROS
static void uros_setup(void)
{
    uint8_t agent_ok = 0u;

    g_ros_retry_count++;
    g_ros_ok = 0u;
    g_ros_error = 0;
    g_debug_stage = 200u;

    rmw_uros_set_custom_transport(true, (void *)&huart6,
                                  transport_open,
                                  transport_close,
                                  transport_write,
                                  transport_read);

    g_debug_stage = 201u;
    transport_reset_debug();
    transport_drain_rx(20u);
    HAL_Delay(UROS_STARTUP_DELAY_MS);
    alloc = rcutils_get_default_allocator();
    if (rmw_uros_ping_agent(UROS_PING_TIMEOUT_MS, UROS_PING_ATTEMPTS) == RMW_RET_OK) {
        agent_ok = 1u;
    }

    if (!agent_ok) {
        g_ros_error = -1;
        return;
    }

    g_debug_stage = 202u;
    sensor_msgs__msg__Imu__init(&msg_imu);
    std_msgs__msg__Int32MultiArray__init(&msg_ticks);
    std_msgs__msg__Float32MultiArray__init(&msg_vel);
    std_msgs__msg__Int32MultiArray__init(&msg_status);
    geometry_msgs__msg__Twist__init(&msg_cmd);

    UROS_TRY_STAGE(210u, rclc_support_init(&support, 0, NULL, &alloc));
    UROS_TRY_STAGE(211u, rclc_node_init_default(&node, "stm32f7_node", "terrain_bot", &support));

    UROS_TRY_STAGE(212u, rclc_publisher_init_best_effort(&pub_imu, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu), "/imu/data"));
    UROS_TRY_STAGE(213u, rclc_publisher_init_best_effort(&pub_ticks, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray), "/wheel_ticks"));
    UROS_TRY_STAGE(214u, rclc_publisher_init_best_effort(&pub_vel, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32MultiArray), "/wheel_velocity"));
    UROS_TRY_STAGE(215u, rclc_publisher_init_best_effort(&pub_status, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray), "/base_status"));
    UROS_TRY_STAGE(216u, rclc_subscription_init_default(&sub_cmd, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "/cmd_vel"));

    UROS_TRY_STAGE(217u, rclc_executor_init(&exec, &support.context, 1, &alloc));
    UROS_TRY_STAGE(218u, rclc_executor_add_subscription(&exec, &sub_cmd, &msg_cmd, cmd_cb, ON_NEW_DATA));

    g_debug_stage = 219u;
    msg_ticks.data.data = tick_buf;
    msg_ticks.data.size = MOTOR_NUM;
    msg_ticks.data.capacity = MOTOR_NUM;
    msg_vel.data.data = vel_buf;
    msg_vel.data.size = MOTOR_NUM;
    msg_vel.data.capacity = MOTOR_NUM;
    msg_status.data.data = status_buf;
    msg_status.data.size = 5;
    msg_status.data.capacity = 5;

    msg_imu.header.frame_id.data = frame_id;
    msg_imu.header.frame_id.size = 8;
    msg_imu.header.frame_id.capacity = sizeof(frame_id);

    memset(msg_imu.orientation_covariance, 0, sizeof(msg_imu.orientation_covariance));
    memset(msg_imu.angular_velocity_covariance, 0, sizeof(msg_imu.angular_velocity_covariance));
    memset(msg_imu.linear_acceleration_covariance, 0, sizeof(msg_imu.linear_acceleration_covariance));
    msg_imu.orientation_covariance[0] = OR_COV;
    msg_imu.orientation_covariance[4] = OR_COV;
    msg_imu.orientation_covariance[8] = OR_COV;
    msg_imu.angular_velocity_covariance[0] = GY_COV;
    msg_imu.angular_velocity_covariance[4] = GY_COV;
    msg_imu.angular_velocity_covariance[8] = GY_COV;
    msg_imu.linear_acceleration_covariance[0] = AC_COV;
    msg_imu.linear_acceleration_covariance[4] = AC_COV;
    msg_imu.linear_acceleration_covariance[8] = AC_COV;

    g_debug_stage = 220u;
    g_ros_ok = 1u;
}
#else
static void uros_setup(void)
{
}
#endif

#ifdef USE_MICROROS
static void publish_imu(void)
{
    uint64_t ns = (uint64_t)HAL_GetTick() * 1000000ULL;

    if (!g_ros_ok || !g_imu_ok) {
        return;
    }

    msg_imu.header.stamp.sec = (int32_t)(ns / 1000000000ULL);
    msg_imu.header.stamp.nanosec = (uint32_t)(ns % 1000000000ULL);
    msg_imu.orientation.w = ahrs.q0;
    msg_imu.orientation.x = ahrs.q1;
    msg_imu.orientation.y = ahrs.q2;
    msg_imu.orientation.z = ahrs.q3;
    msg_imu.angular_velocity.x = imu.gx - calib.gx_bias;
    msg_imu.angular_velocity.y = imu.gy - calib.gy_bias;
    msg_imu.angular_velocity.z = imu.gz - calib.gz_bias;
    msg_imu.linear_acceleration.x = imu.ax;
    msg_imu.linear_acceleration.y = imu.ay;
    msg_imu.linear_acceleration.z = imu.az;
    if (rcl_publish(&pub_imu, &msg_imu, NULL) != RCL_RET_OK) {
        g_ros_ok = 0u;
        g_ros_error = -2;
    }
}
#else
static void publish_imu(void)
{
}
#endif

#ifdef USE_MICROROS
static void publish_encoders(void)
{
    if (!g_ros_ok || !g_esp32_ok || !g_wheels_ok) {
        return;
    }

    for (uint8_t i = 0; i < MOTOR_NUM; i++) {
        tick_buf[i] = Encoder_Ticks(i);
        vel_buf[i] = Encoder_VelMmps(i);
    }

    if (rcl_publish(&pub_ticks, &msg_ticks, NULL) != RCL_RET_OK ||
        rcl_publish(&pub_vel, &msg_vel, NULL) != RCL_RET_OK) {
        g_ros_ok = 0u;
        g_ros_error = -3;
    }
}
#else
static void publish_encoders(void)
{
}
#endif

#ifdef USE_MICROROS
static void publish_status(void)
{
    if (!g_ros_ok) {
        return;
    }

    status_buf[0] = (int32_t)g_esp32_ok;
    status_buf[1] = (int32_t)g_wheels_ok;
    status_buf[2] = (int32_t)g_imu_ok;
    status_buf[3] = (int32_t)g_host_ok;
    status_buf[4] = (int32_t)g_cmd_count;

    if (rcl_publish(&pub_status, &msg_status, NULL) != RCL_RET_OK) {
        g_ros_ok = 0u;
        g_ros_error = -4;
    }
}
#else
static void publish_status(void)
{
}
#endif

static void seed_filter_from_accel(void)
{
    float ax = imu.ax;
    float ay = imu.ay;
    float az = imu.az;
    float recip = 1.0f / sqrtf(ax * ax + ay * ay + az * az);
    float roll;
    float pitch;

    ax *= recip;
    ay *= recip;
    az *= recip;

    roll = atan2f(ay, az);
    pitch = atan2f(-ax, sqrtf(ay * ay + az * az));
    imu.roll_rad = roll;
    imu.pitch_rad = pitch;
    ahrs.q0 = cosf(roll * 0.5f) * cosf(pitch * 0.5f);
    ahrs.q1 = sinf(roll * 0.5f) * cosf(pitch * 0.5f);
    ahrs.q2 = cosf(roll * 0.5f) * sinf(pitch * 0.5f);
    ahrs.q3 = -sinf(roll * 0.5f) * sinf(pitch * 0.5f);
    ahrs.roll = roll;
    ahrs.pitch = pitch;
    ahrs.yaw = 0.0f;
    ahrs.initialised = true;
}

static void sync_attitude_from_ros_filter(void)
{
    float roll = imu.roll_rad;
    float pitch = imu.pitch_rad;

    ahrs.q0 = cosf(roll * 0.5f) * cosf(pitch * 0.5f);
    ahrs.q1 = sinf(roll * 0.5f) * cosf(pitch * 0.5f);
    ahrs.q2 = cosf(roll * 0.5f) * sinf(pitch * 0.5f);
    ahrs.q3 = -sinf(roll * 0.5f) * sinf(pitch * 0.5f);
    ahrs.roll = roll;
    ahrs.pitch = pitch;
    ahrs.yaw = 0.0f;
    ahrs.initialised = true;
}

void App_MotorEsp32Diag_Run(void)
{
    uint32_t t_enc;
    uint32_t t_step;
    uint8_t forward = 1u;
    uint8_t motor_running = 0u;

    memset(&imu, 0, sizeof(imu));
    memset(&ahrs, 0, sizeof(ahrs));

    g_debug_stage = 10u;
    LCD_Display_Init();
    g_debug_stage = 20u;
    LCD_Display_BootStatus(0u, 0u, 0u);

    g_debug_stage = 30u;
    Encoder_Init(&hi2c1);
    g_imu_ok = 0u;
    g_ros_ok = 0u;
    g_host_ok = 1u;
    g_tilt_risk = 0.0f;
    g_slip_risk = 0.0f;
    g_risk = 0.0f;
    g_scale = 1.0f;
    g_esp32_ok = 0u;
    g_wheels_ok = 0u;
    g_encoder_feedback_ok = 0u;
    if (Encoder_Update() == HAL_OK) {
        g_esp32_ok = Encoder_Esp32Connected();
        g_wheels_ok = Encoder_WheelsConnected();
        g_encoder_feedback_ok = (g_esp32_ok && g_wheels_ok) ? 1u : 0u;
    }

    g_debug_stage = 40u;
    Motor_Init(&htim10, &htim11, &htim12, &htim13, &htim14, &htim2, &htim3);
    Motor_StopAll();
    LCD_Display_BootMotorTest("DIAG ALL FWD");

    t_enc = HAL_GetTick();
    t_step = HAL_GetTick();

    while (1) {
        uint32_t now = HAL_GetTick();
        char label[24];

        g_debug_heartbeat++;

        if (now - t_enc >= DIAG_ENC_MS) {
            t_enc = now;
            if (Encoder_Update() == HAL_OK) {
                g_esp32_ok = Encoder_Esp32Connected();
                g_wheels_ok = Encoder_WheelsConnected();
                g_encoder_feedback_ok = (g_esp32_ok && g_wheels_ok) ? 1u : 0u;
            } else {
                g_esp32_ok = 0u;
                g_wheels_ok = 0u;
                g_encoder_feedback_ok = 0u;
            }
        }

        if (motor_running) {
            if (now - t_step >= DIAG_MOTOR_RUN_MS) {
                Motor_StopAll();
                motor_running = 0u;
                t_step = now;
                forward ^= 1u;
                snprintf(label, sizeof(label), "DIAG ALL %s",
                         forward ? "FWD" : "REV");
                LCD_Display_BootMotorTest(label);
            }
        } else if (now - t_step >= DIAG_MOTOR_STOP_MS) {
            for (uint8_t motor = 0u; motor < MOTOR_NUM; motor++) {
                Motor_SetTestDuty((Motor_ID)motor, DIAG_MOTOR_DUTY_PERCENT, forward);
            }
            motor_running = 1u;
            t_step = now;
            snprintf(label, sizeof(label), "RUN ALL %s",
                     forward ? "FWD" : "REV");
            LCD_Display_BootMotorTest(label);
        }
    }
}

void App_Run(void)
{
    uint32_t t_imu;
    uint32_t t_enc;
    uint32_t t_pid;
    uint32_t t_pimu;
    uint32_t t_penc;
    uint32_t t_lcd;
    uint32_t t_i2c_recover;
    uint32_t t_imu_retry;
    uint32_t t_ros_retry;
    uint8_t enc_fail_count;
    uint8_t filter_seeded;

    g_debug_stage = 10u;
    LCD_Display_Init();
    g_debug_stage = 20u;
    LCD_Display_BootStatus(0u, 0u, 0u);

    g_debug_stage = 30u;
    Encoder_Init(&hi2c1);
    g_esp32_ok = 0u;
    g_wheels_ok = 0u;
    g_encoder_feedback_ok = 0u;
    g_tilt_risk = 0.0f;
    g_slip_risk = 0.0f;
    g_risk = 0.0f;
    g_scale = 1.0f;
    g_slip_alert = 0u;
    memset(g_measured_filt, 0, sizeof(g_measured_filt));
    memset(g_wheel_slip, 0, sizeof(g_wheel_slip));
    refresh_encoder_status();
    g_imu_ok = 0u;
    LCD_Display_BootStatus(g_imu_ok, g_esp32_ok, 0u);

    g_debug_stage = 40u;
    Motor_Init(&htim10, &htim11, &htim12, &htim13, &htim14, &htim2, &htim3);
    startup_motor_check();

    g_debug_stage = 50u;
    g_imu_ok = ICM20948_Init(&hi2c1) == HAL_OK ? 1u : 0u;
    LCD_Display_BootStatus(g_imu_ok, g_esp32_ok, 0u);

    Madgwick_Init(&ahrs, MADGWICK_BETA);
    g_debug_stage = 60u;
    g_ros_ok = 0u;
    LCD_Display_BootStatus(g_imu_ok, g_esp32_ok, 0u);
    HAL_Delay(1500);

    g_debug_stage = 70u;
    memset(&calib, 0, sizeof(calib));
    while (g_imu_ok && !calib.done) {
        if (ICM20948_Read(&hi2c1, &imu) == HAL_OK) {
            ICM20948_Calibrate(&imu, &calib, CALIB_N);
        } else {
            g_imu_ok = 0u;
            break;
        }
        LCD_Display_Update(&ahrs, &imu, 0u, 0.0f, 0u, g_imu_ok, g_esp32_ok, g_wheels_ok, g_ros_ok, 0u);
        HAL_Delay(IMU_MS);
    }

    if (g_imu_ok && calib.done) {
        seed_filter_from_accel();
    }
    g_debug_stage = 80u;
    g_cmd_ms = HAL_GetTick();

    t_imu = HAL_GetTick();
    t_enc = HAL_GetTick();
    t_pid = HAL_GetTick();
    t_pimu = HAL_GetTick();
    t_penc = HAL_GetTick();
    t_lcd = HAL_GetTick();
    t_i2c_recover = HAL_GetTick();
    t_imu_retry = HAL_GetTick();
    t_ros_retry = HAL_GetTick();
    enc_fail_count = 0u;
    filter_seeded = (g_imu_ok && calib.done) ? 1u : 0u;

    while (1) {
        uint32_t now = HAL_GetTick();
        g_debug_heartbeat++;

        g_host_ok = g_ros_ok ? (((now - g_cmd_ms) <= WDG_MS) ? 1u : 0u) : 1u;
        update_normal_mode_targets(now);

        if (!g_ros_ok && now - t_ros_retry >= ROS_RETRY_MS) {
            t_ros_retry = now;
            uros_setup();
        }

        if (!g_imu_ok && now - t_imu_retry >= IMU_RETRY_MS) {
            t_imu_retry = now;
            I2C1_Recover();
            if (ICM20948_Init(&hi2c1) == HAL_OK) {
                memset(&calib, 0, sizeof(calib));
                memset(&imu, 0, sizeof(imu));
                filter_seeded = 0u;
                g_tilt_risk = 0.0f;
                g_slip_risk = 0.0f;
                g_risk = 0.0f;
                g_emergency_stop = 0u;
                g_estop_count = 0u;
                g_imu_ok = 1u;
            }
        }

        if (now - t_imu >= IMU_MS) {
            float dt = (float)(now - t_imu) * 0.001f;
            t_imu = now;
            if (g_imu_ok && ICM20948_Read(&hi2c1, &imu) == HAL_OK) {
                float gx = imu.gx - calib.gx_bias;
                float gy = imu.gy - calib.gy_bias;
                float gz = imu.gz - calib.gz_bias;

                if (!calib.done) {
                    ICM20948_Calibrate(&imu, &calib, CALIB_N);
                    if (calib.done && !filter_seeded) {
                        seed_filter_from_accel();
                        filter_seeded = 1u;
                    }
                } else {
                    (void)gx;
                    (void)gy;
                    (void)gz;
                    ICM20948_Filter(&imu, &calib, dt, CF_ALPHA);
                    sync_attitude_from_ros_filter();
                    update_safety();
                }
            } else if (g_imu_ok) {
                g_imu_ok = 0u;
                g_tilt_risk = 0.0f;
                g_risk = 0.0f;
                g_scale = 0.0f;
                g_emergency_stop = 0u;
                if (now - t_i2c_recover >= I2C_RECOVER_MS) {
                    t_i2c_recover = now;
                    I2C1_Recover();
                }
            }
        }

        if (now - t_enc >= (g_encoder_feedback_ok ? ENC_MS : ENC_RETRY_MS)) {
            t_enc = now;
            if (Encoder_Update() == HAL_OK) {
                g_esp32_ok = Encoder_Esp32Connected();
                g_wheels_ok = Encoder_WheelsConnected();
                g_encoder_feedback_ok = (g_esp32_ok && g_wheels_ok) ? 1u : 0u;
                if (g_encoder_feedback_ok) {
                    enc_fail_count = 0u;
                }
            } else {
                g_esp32_ok = 0u;
                g_wheels_ok = 0u;
                g_encoder_feedback_ok = 0u;
                if (enc_fail_count < 0xFFu) {
                    enc_fail_count++;
                }
                if (enc_fail_count < ENC_FAIL_DISABLE_COUNT &&
                    now - t_i2c_recover >= I2C_RECOVER_MS) {
                    t_i2c_recover = now;
                    I2C1_Recover();
                }
            }
        }

        if (now - t_pid >= PID_MS) {
            float dt = (float)(now - t_pid) * 0.001f;
            t_pid = now;
            if (!g_host_ok) {
                memset(g_target, 0, sizeof(g_target));
                memset(g_integral, 0, sizeof(g_integral));
                Motor_StopAll();
            } else {
                motor_pid(dt);
            }
        }

        if (now - t_pimu >= PUB_IMU_MS) {
            t_pimu = now;
            publish_imu();
        }

        if (now - t_penc >= PUB_ENC_MS) {
            t_penc = now;
            publish_status();
            publish_encoders();
        }

        if (now - t_lcd >= LCD_MS) {
            t_lcd = now;
            LCD_Display_Update(&ahrs, &imu, 1u, g_risk,
                               g_emergency_stop,
                               g_imu_ok, g_esp32_ok, g_wheels_ok, g_ros_ok, g_host_ok);
        }

#ifdef USE_MICROROS
        if (g_ros_ok) {
            rclc_executor_spin_some(&exec, RCL_MS_TO_NS(ROS_SPIN_MS));
        }
#endif
    }
}
