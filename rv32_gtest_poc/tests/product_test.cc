#include <gtest/gtest.h>
#include <cstring>
#include "fff.h"
#include "product.h"
#include "replay.h"
DEFINE_FFF_GLOBALS;
extern "C" {
FAKE_VALUE_FUNC(int, sensor_read, int16_t *, size_t);
FAKE_VOID_FUNC(sink_write, const struct product_result *);
}
class ProductTest : public testing::Test {
 void SetUp() override { RESET_FAKE(sensor_read); RESET_FAKE(sink_write); FFF_RESET_HISTORY(); }
};
TEST_F(ProductTest, DeterministicReplay) {
 product_result r{};
 ASSERT_EQ(0, product_process(replay_samples, REPLAY_COUNT, REPLAY_OFFSET, &r));
 EXPECT_EQ(REPLAY_SUM, r.sum); EXPECT_EQ(REPLAY_PEAK, r.peak);
 EXPECT_EQ(REPLAY_CLIPPED, r.clipped); EXPECT_EQ(REPLAY_CHECKSUM, r.checksum);
}
TEST_F(ProductTest, RejectsInvalidInputs) {
 product_result r{};
 EXPECT_EQ(-1, product_process(nullptr, 1, 0, &r));
 EXPECT_EQ(-1, product_process(replay_samples, 1025, 0, &r));
 EXPECT_EQ(-1, product_process(replay_samples, 1, 0, nullptr));
 EXPECT_EQ(0, product_process(nullptr, 0, 0, &r)); EXPECT_EQ(0, r.sum);
}
TEST_F(ProductTest, FffReadFailureDoesNotWrite) {
 sensor_read_fake.return_val = -1;
 EXPECT_EQ(-1, product_poll()); EXPECT_EQ(1u, sensor_read_fake.call_count);
 EXPECT_EQ(0u, sink_write_fake.call_count);
}
static product_result captured;
static int replay_read(int16_t *out, size_t capacity) {
 if (capacity < REPLAY_COUNT) return -1;
 std::memcpy(out, replay_samples, sizeof replay_samples); return REPLAY_COUNT;
}
static void capture_write(const product_result *r) { captured = *r; }
TEST_F(ProductTest, FffAdapterCapturesOutputAndOrder) {
 sensor_read_fake.custom_fake = replay_read; sink_write_fake.custom_fake = capture_write;
 ASSERT_EQ(0, product_poll()); EXPECT_EQ(1u, sensor_read_fake.call_count);
 EXPECT_EQ(1u, sink_write_fake.call_count); EXPECT_EQ(REPLAY_SUM, captured.sum);
 EXPECT_EQ(REPLAY_CHECKSUM, captured.checksum);
 EXPECT_EQ((fff_function_t)sensor_read, fff.call_history[0]);
 EXPECT_EQ((fff_function_t)sink_write, fff.call_history[1]);
}
TEST_F(ProductTest, FffRejectsOversizedDriverReply) {
 sensor_read_fake.return_val = 17; EXPECT_EQ(-1, product_poll());
 EXPECT_EQ(0u, sink_write_fake.call_count);
}
TEST_F(ProductTest, DISABLED_NegativeOracleMustFail) {
 product_result r{}; ASSERT_EQ(0, product_process(replay_samples, REPLAY_COUNT, REPLAY_OFFSET, &r));
 EXPECT_EQ(REPLAY_SUM + 1, r.sum) << "Intentional wrong expected value: runner must detect this failure";
}

#ifdef USE_FREERTOS
extern "C" int rv32_check_os_behavior(void);
TEST(FreeRtosBehavior, QueueTimeoutWakeupDelayPriorityAndNotifications) {
 EXPECT_EQ(0, rv32_check_os_behavior());
}
#endif
