#include <cstdio>
extern "C" int demo_main(void) {
 setvbuf(stdout, nullptr, _IONBF, 0);
 puts("RV32 framework cost control");
 return 0;
}
