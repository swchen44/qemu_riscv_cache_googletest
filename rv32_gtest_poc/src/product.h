#ifndef PRODUCT_H
#define PRODUCT_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
struct product_result { int32_t sum; uint32_t checksum; uint16_t peak; uint16_t clipped; };
/* Pure C product core: no RTOS, allocator, floating point or global state. */
int product_process(const int16_t *samples, size_t count, int16_t offset, struct product_result *result);
/* Adapter dependencies are mocked with FFF in host/RV32 GoogleTest. */
int sensor_read(int16_t *samples, size_t capacity);
void sink_write(const struct product_result *result);
int product_poll(void);
#ifdef __cplusplus
}
#endif
#endif
