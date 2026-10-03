#include <gtest/gtest.h>
#include <cstdio>
#ifdef USE_FREERTOS
extern "C" int rv32_run_gtests(void) {
#else
extern "C" int demo_main(void) {
#endif
 setvbuf(stdout, nullptr, _IONBF, 0);
 #ifdef USE_FREERTOS
 puts("RV32 GoogleTest + FFF inside FreeRTOS supervisor task; no Linux");
#else
 puts("RV32 bare metal GoogleTest + FFF, no Linux, no RTOS");
#endif
 int argc=1; char name[]="rv32-tests"; char *argv[]={name,nullptr};
 testing::InitGoogleTest(&argc,argv);
#ifdef INJECT_FAILURE
 GTEST_FLAG_SET(also_run_disabled_tests,true);
 GTEST_FLAG_SET(filter,"*Negative*");
#endif
 return RUN_ALL_TESTS();
}
