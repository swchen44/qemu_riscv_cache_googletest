#include "product.h"
int product_process(const int16_t *samples, size_t count, int16_t offset, struct product_result *result) {
    if (!result || (!samples && count) || count > 1024) return -1;
    struct product_result r = {0, 2166136261u, 0, 0};
    for (size_t i = 0; i < count; ++i) {
        int32_t value = (int32_t)samples[i] + offset;
        if (value < 0) { value = 0; ++r.clipped; }
        if (value > 4095) { value = 4095; ++r.clipped; }
        r.sum += value;
        if (value > r.peak) r.peak = (uint16_t)value;
        r.checksum = (r.checksum ^ (uint32_t)value) * 16777619u;
    }
    *result = r;
    return 0;
}
