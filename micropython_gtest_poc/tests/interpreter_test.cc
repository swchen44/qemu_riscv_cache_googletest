#include <gtest/gtest.h>
#include <cstring>
#include "fff.h"
#include "interpreter_bridge.h"
DEFINE_FFF_GLOBALS;
extern "C" { FAKE_VOID_FUNC(platform_log_write,const char *,size_t); }
extern void *test_stack_top;
static char output[4096];
static size_t output_size;
static bool output_overflow;
static void capture(const char *s,size_t n) {
 if(n >= sizeof(output)-output_size) {output_overflow=true;return;}
 std::memcpy(output+output_size,s,n);output_size+=n;output[output_size]=0;
}
class MicroPythonTest:public testing::Test {
 protected:
 void SetUp() override {RESET_FAKE(platform_log_write);FFF_RESET_HISTORY();output_size=0;output[0]=0;output_overflow=false;platform_log_write_fake.custom_fake=capture;interpreter_init(test_stack_top);}
 void TearDown() override {interpreter_deinit();EXPECT_FALSE(output_overflow); }
 void Eval(const char *src,int64_t expected) {int64_t value=0;ASSERT_EQ(0,interpreter_eval_int(src,&value))<<interpreter_last_error();EXPECT_EQ(expected,value);}
 void Exec(const char *src) {ASSERT_EQ(0,interpreter_exec(src))<<interpreter_last_error();}
};
TEST_F(MicroPythonTest, ParserArithmeticPrecedence) {Eval("2 + 3 * 7 - (8 // 2)",19);}
TEST_F(MicroPythonTest, FunctionsRecursionAndBytecode) {Exec("def fact(n):\n return 1 if n < 2 else n * fact(n - 1)\n");Eval("fact(8)",40320);}
TEST_F(MicroPythonTest, ListDictComprehensionAndMutation) {Exec("xs = [i*i for i in range(10)]\nd = {'left': xs[3], 'right': xs[-1]}\nxs.append(100)\n");Eval("sum(xs) + d['left'] + d['right']",475);}
TEST_F(MicroPythonTest, MpzBigIntegerBeyondRv32Word) {Exec("big = (1 << 100) + 1234567\nassert (big - 1234567) == (1 << 100)\n");Eval("big % 1000003",487673);}
TEST_F(MicroPythonTest, PythonExceptionCatchAndFinally) {Exec("state = []\ntry:\n 1 // 0\nexcept ZeroDivisionError:\n state.append(7)\nfinally:\n state.append(9)\n");Eval("state[0] * 10 + state[1]",79);}
TEST_F(MicroPythonTest, SyntaxErrorThenInterpreterRecovers) {EXPECT_EQ(-1,interpreter_exec("def broken(:\n pass\n"));EXPECT_STREQ("SyntaxError",interpreter_last_error());Eval("6 * 7",42);}
TEST_F(MicroPythonTest, RuntimeTypeErrorThenInterpreterRecovers) {EXPECT_EQ(-1,interpreter_exec("result = 1 + 'x'\n"));EXPECT_STREQ("TypeError",interpreter_last_error());Eval("100 // 4",25);}
TEST_F(MicroPythonTest, GlobalsObjectsSurviveExplicitGc) {Exec("keeper = {'values': list(range(100))}\n");interpreter_collect();Eval("sum(keeper['values'])",4950);EXPECT_GT(interpreter_heap_free(),size_t(1024));}
TEST_F(MicroPythonTest, AllocationChurnCollectsAndKeepsRoots) {Exec("import gc\nkeep = [i for i in range(64)]\nfor j in range(1000):\n temporary = [j, bytearray(1024), str(j)]\n if j % 10 == 0:\n  gc.collect()\ngc.collect()\nassert keep[63] == 63\n");Eval("sum(keep)",2016);}
TEST_F(MicroPythonTest, RealPrintUsesFffPlatformBoundary) {Exec("print('rv32', 6 * 7)\n");EXPECT_STREQ("rv32 42\n",output);EXPECT_GT(platform_log_write_fake.call_count,0u);}
TEST_F(MicroPythonTest, IndependentFixtureStartsWithoutPreviousGlobals) {EXPECT_EQ(-1,interpreter_exec("print(keeper)\n"));EXPECT_STREQ("NameError",interpreter_last_error());}
TEST_F(MicroPythonTest, DISABLED_NegativeOracleMustFail) {Eval("6 * 7",43);}
