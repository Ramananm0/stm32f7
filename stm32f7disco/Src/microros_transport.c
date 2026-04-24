#include "microros_transport.h"
#include "main.h"

#ifdef USE_MICROROS
#include <time.h>
#include <uxr/client/transport.h>

#define UART_IT_BUFFER_SIZE 2048u

extern UART_HandleTypeDef huart6;
extern volatile uint32_t g_ros_rx_bytes;
extern volatile uint32_t g_ros_tx_bytes;
extern volatile uint8_t g_ros_rx_sample[4];
extern volatile uint8_t g_ros_rx_sample_len;

static uint8_t g_uart_it_byte;
static uint8_t g_uart_it_buffer[UART_IT_BUFFER_SIZE];
static volatile size_t g_uart_it_head;
static volatile size_t g_uart_it_tail;

void transport_reset_debug(void)
{
    g_ros_rx_bytes = 0u;
    g_ros_tx_bytes = 0u;
    g_ros_rx_sample_len = 0u;
    g_ros_rx_sample[0] = 0u;
    g_ros_rx_sample[1] = 0u;
    g_ros_rx_sample[2] = 0u;
    g_ros_rx_sample[3] = 0u;
}

void transport_drain_rx(uint32_t timeout_ms)
{
    (void)timeout_ms;
    __disable_irq();
    g_uart_it_head = 0u;
    g_uart_it_tail = 0u;
    __enable_irq();
}

bool transport_open(struct uxrCustomTransport *transport)
{
    UART_HandleTypeDef *uart = (UART_HandleTypeDef *)transport->args;
    g_uart_it_head = 0u;
    g_uart_it_tail = 0u;
    HAL_UART_AbortReceive_IT(uart);
    HAL_UART_Receive_IT(uart, &g_uart_it_byte, 1u);
    return true;
}

bool transport_close(struct uxrCustomTransport *transport)
{
    UART_HandleTypeDef *uart = (UART_HandleTypeDef *)transport->args;
    HAL_UART_AbortReceive_IT(uart);
    return true;
}

size_t transport_write(struct uxrCustomTransport *transport,
                       const uint8_t *buf,
                       size_t len,
                       uint8_t *err)
{
    HAL_StatusTypeDef status;
    UART_HandleTypeDef *uart = (UART_HandleTypeDef *)transport->args;

    status = HAL_UART_Transmit(uart, (uint8_t *)buf, (uint16_t)len, 200);
    if (status == HAL_OK) {
        g_ros_tx_bytes += (uint32_t)len;
    }
    *err = status == HAL_OK ? 0u : 1u;
    return status == HAL_OK ? len : 0u;
}

size_t transport_read(struct uxrCustomTransport *transport,
                      uint8_t *buf,
                      size_t len,
                      int timeout_ms,
                      uint8_t *err)
{
    UART_HandleTypeDef *uart = (UART_HandleTypeDef *)transport->args;
    uint32_t start_ms;
    uint32_t last_data_ms = 0u;
    size_t received = 0u;
    uint32_t first_timeout_ms;
    uint32_t inter_byte_timeout_ms = 10u;

    start_ms = HAL_GetTick();
    *err = 0u;
    first_timeout_ms = (timeout_ms <= 0) ? 1u : (uint32_t)timeout_ms;

    while (received < len) {
        while ((g_uart_it_head != g_uart_it_tail) && (received < len)) {
            buf[received] = g_uart_it_buffer[g_uart_it_head];
            g_uart_it_head = (g_uart_it_head + 1u) % UART_IT_BUFFER_SIZE;
            if (g_ros_rx_sample_len < 4u) {
                g_ros_rx_sample[g_ros_rx_sample_len] = buf[received];
                g_ros_rx_sample_len++;
            }
            g_ros_rx_bytes++;
            received++;
            last_data_ms = HAL_GetTick();
        }

        if (received > 0u) {
            if ((HAL_GetTick() - last_data_ms) >= inter_byte_timeout_ms) {
                break;
            }
        } else {
            if ((HAL_GetTick() - start_ms) >= first_timeout_ms) {
                break;
            }
        }

        (void)uart;
        HAL_Delay(1);
    }

    return received;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    size_t next_tail;

    if (huart != &huart6) {
        return;
    }

    next_tail = (g_uart_it_tail + 1u) % UART_IT_BUFFER_SIZE;
    if (next_tail != g_uart_it_head) {
        g_uart_it_buffer[g_uart_it_tail] = g_uart_it_byte;
        g_uart_it_tail = next_tail;
    }

    HAL_UART_Receive_IT(&huart6, &g_uart_it_byte, 1u);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart6) {
        return;
    }

    __HAL_UART_CLEAR_OREFLAG(&huart6);
    __HAL_UART_CLEAR_NEFLAG(&huart6);
    __HAL_UART_CLEAR_FEFLAG(&huart6);
    __HAL_UART_CLEAR_PEFLAG(&huart6);
    HAL_UART_Receive_IT(&huart6, &g_uart_it_byte, 1u);
}

int clock_gettime(int clock_id, struct timespec *tp)
{
    uint64_t now_ms;

    (void)clock_id;

    if (tp == NULL) {
        return -1;
    }

    now_ms = HAL_GetTick();
    tp->tv_sec = (time_t)(now_ms / 1000u);
    tp->tv_nsec = (long)((now_ms % 1000u) * 1000000u);

    return 0;
}
#endif
