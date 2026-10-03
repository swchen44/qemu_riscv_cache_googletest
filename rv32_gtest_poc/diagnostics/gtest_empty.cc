#include <cstdio>
#include <gtest/gtest.h>
extern "C" int demo_main(void) {
 setvbuf(stdout, nullptr, _IONBF, 0);
 puts("RV32 framework cost control");
 int argc=1; char app[]="control"; char *argv[]={app,nullptr};
 testing::InitGoogleTest(&argc,argv);
 return RUN_ALL_TESTS();
}
