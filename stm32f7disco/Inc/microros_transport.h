#ifndef MICROROS_TRANSPORT_H
#define MICROROS_TRANSPORT_H

#include "stm32f7xx_hal.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct uxrCustomTransport;

bool transport_open(struct uxrCustomTransport *transport);
bool transport_close(struct uxrCustomTransport *transport);
size_t transport_write(struct uxrCustomTransport *transport,
                       const uint8_t *buf,
                       size_t len,
                       uint8_t *err);
size_t transport_read(struct uxrCustomTransport *transport,
                      uint8_t *buf,
                      size_t len,
                      int timeout_ms,
                      uint8_t *err);
void transport_reset_debug(void);
void transport_drain_rx(uint32_t timeout_ms);

#endif /* MICROROS_TRANSPORT_H */
