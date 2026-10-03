#ifndef REPLAY_H
#define REPLAY_H
#include <stdint.h>
static const int16_t replay_samples[] = {-20, 0, 10, 100, 4090, 4095, 5000, 42};
#define REPLAY_COUNT 8
#define REPLAY_OFFSET 10
#define REPLAY_SUM 12477
#define REPLAY_PEAK 4095
#define REPLAY_CLIPPED 4
#define REPLAY_CHECKSUM 0xdf8ba548u
#endif
