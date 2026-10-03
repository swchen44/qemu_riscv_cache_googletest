#include <gtest/gtest.h>
#include <cstdio>
void *test_stack_top;
#ifdef RV32_BARE_METAL
extern "C" int demo_main(void) {
 int argc=1;char app[]="micropython-tests";char *argv[]={app,nullptr};
#else
int main(int argc,char **argv) {
#endif
 int stack_top;test_stack_top=&stack_top;setvbuf(stdout,nullptr,_IONBF,0);
 puts("Real MicroPython 1.26.1 interpreter + GoogleTest + FFF platform stdout seam");
 testing::InitGoogleTest(&argc,argv);
#ifdef INJECT_FAILURE
 GTEST_FLAG_SET(also_run_disabled_tests,true);GTEST_FLAG_SET(filter,"*Negative*");
#endif
 return RUN_ALL_TESTS();
}
