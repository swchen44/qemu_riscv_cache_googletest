#include "product.h"
int product_poll(void) {
    int16_t samples[16];
    struct product_result result;
    int count = sensor_read(samples, 16);
    if (count < 0 || count > 16) return -1;
    int rc = product_process(samples, (size_t)count, 10, &result);
    if (rc == 0) sink_write(&result);
    return rc;
}
