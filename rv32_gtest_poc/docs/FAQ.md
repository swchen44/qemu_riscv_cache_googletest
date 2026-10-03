# GoogleTest / FFF / FreeRTOS / MicroPython：RV32 移植 FAQ

更新：2026-10-03。這是可持續追加的工程FAQ；保留 `FAQ-001` 等既有編號，不重新編號。所有「本次實测」均指本專案固定版本與設定，不代表任意產品的需求或完整上游功能。

## FAQ-001　GoogleTest 需要重新編譯嗎？

**需要，除非已有完全相容的 RV32 版 library。** 不能把 x86_64 host 的 GoogleTest executable 或 `.o` 直接交給 RV32 QEMU 跑。

本次分工：

| 元件 | Host | RV32 |
|---|---|---|
| 產品純 C / MicroPython C | host gcc | riscv-none-elf-gcc |
| GoogleTest `gtest-all.cc`、測試 `.cc`、自訂main | host g++ | riscv-none-elf-g++ |
| FFF | header展開到test translation unit | 隨RV32 C++測試一起重編 |
| 最後link | host g++ | riscv-none-elf-g++，連入相容的C++/C runtime |

可參閱 [build_bare_gtest.sh](../scripts/build_bare_gtest.sh)、[build_host.sh](../scripts/build_host.sh)、[MicroPython build.py](../../micropython_gtest_poc/scripts/build.py)。相同test source不等於相同machine-code binary。

### 必須一致的條件

- `-march=rv32imac_zicsr -mabi=ilp32`：ISA、32-bit ABI與runtime multilib要對得上。
- 本例 `-mcmodel=medany -O2 -g -ffunction-sections -fdata-sections`，link用`--gc-sections`。
- GoogleTest/測試同用C++14與相容的libstdc++ ABI；本次為xPack GCC15.2.0-1 / newlib。
- 本例C++ exceptions與RTTI停用，GoogleTest同時定義对应feature宏，不能只改其中一邊。

本次預處理器實測值見 [gtest-effective-feature-macros.txt](../evidence/gtest-effective-feature-macros.txt)：

```text
GTEST_HAS_PTHREAD=0
GTEST_HAS_EXCEPTIONS=0
GTEST_HAS_RTTI=0
GTEST_HAS_POSIX_RE=0
GTEST_HAS_STREAM_REDIRECTION=0
GTEST_HAS_FILE_SYSTEM=0
```

`GTEST_HAS_DEATH_TEST`與`GTEST_IS_THREADSAFE`在此裸機target沒有被定義，相關分支未啟用；不是宣稱支援death tests或多task同時使用GoogleTest/FFF。`_POSIX_C_SOURCE=200809L`只是打開newlib宣告，並不提供POSIX OS。

### 純C与C++ mock邊界

C header以`extern "C"`保護；測試內FFF fake定義也包`extern "C"`，以匹配由C compiler產生的symbol。原產品C source仍是C，沒有為了GoogleTest偷偷改用C++編譯。

### C++ exceptions與Python exceptions是兩回事

`-fno-exceptions`停用的是**C++ exception機制**。MicroPython本身的Python `SyntaxError`、`TypeError`、`try/except/finally`由其C runtime / NLR（本例setjmp）處理，已實測通過。C bridge把NLR捕捉留在C函式內，再用error code返回，不讓longjmp跨越GoogleTest的C++ frame/destructor。相關程式：[interpreter_bridge.c](../../micropython_gtest_poc/src/interpreter_bridge.c)。

## FAQ-002　linker script 對 C++ / GoogleTest 要特別安排什麼？

**不需要憑空新增「C++ RAM區」，但必須正確保留、配置並執行C++ runtime所需的段與初始化。** 本次完整檔案：[link.ld](../freertos/link.ld)、[start.S](../freertos/start.S)、[syscalls.c](../freertos/syscalls.c)。

### 本例做了什麼

1. `ENTRY(_start)`，保留`.text.init`。
2. 配置`.text/.rodata/.data/.sdata/.bss/.sbss`，設定`__global_pointer$`、BSS邊界。
3. 保留`.preinit_array/.init_array/.fini_array`，提供libc需要的start/end符號。
4. `.init_array`使用`KEEP`，避免在`--gc-sections`下移除；保留排序後的`.init_array.*`。
5. startup設定`gp/sp`、清BSS，呼叫`__libc_init_array()`，才進入`demo_main()`。
6. 配置`__heap_start/__heap_end/__stack_top`，提供 `_sbrk` 給newlib/C++ allocation。

關鍵片段（不是完整linker script）：

```ld
.init_array : {
  __init_array_start = .;
  KEEP(*(SORT(.init_array.*)))
  KEEP(*(.init_array))
  __init_array_end = .;
} > RAM
```

GoogleTest的`TEST(...)`註冊會用到C++ static initialization。漏掉constructor table或初始化呼叫，可能得到**0 tests、exit 0**。那不是「所有測試通過」。

本專案正例runner另外檢查預期case count：
- [rv32 verify_all.py](../scripts/verify_all.py) 的第12行同時檢查exit與log marker，第18、20–23行要求5/6個PASS。
- [MicroPython verify_all.py](../../micropython_gtest_poc/scripts/verify_all.py) 要求host/RV32各11個PASS。

本FAQ量測用的`cost-gtest_empty`刻意沒有註冊任何test，只用來計算framework成本；它的0-test輸出不能拿來證明產品功能。

### 為什麼提供 __dso_handle？

本例使用`-nostartfiles`及自訂startup，沒有直接用完整GCC CRT startup序列，實際link曾缺`__dso_handle`。在syscalls glue提供該符號，配合newlib/C++ runtime完成連結。不是說所有BSP都應重定義它；已有CRT提供時要避免duplicate definition。

### 真板需要另外處理什麼？

本例QEMU把ELF的initialized segments直接載入同一RAM，linker起點0x80000000，**沒有做Flash LMA→RAM VMA的`.data`搬移**。若真板從Flash/ROM啟動，BSP通常還需：
- 正確的`.data/.sdata` copy、`.bss/.sbss` zero與section alignment
- reset/vector/trap、UART/clock及memory map初始化
- 適用的PMP/MPU/cache/ILM/DLM配置
- 若啟用不同ABI、exceptions、TLS等功能，補對應runtime/sections；本例沒有驗證這些配置

`.fini_array`雖保留，本例回傳後直接`_exit`結束QEMU，**不宣稱執行libc exit的完整全域destructor/finalizer流程**。GoogleTest正常fixture TearDown仍會在RUN_ALL_TESTS內執行。單RAM linker有已知RWX LOAD警告，不是量產保護配置。

## FAQ-003　RV32 GoogleTest 編譯後多大？需要多少記憶體？

**必須分開看：完整test image、framework增量、ELF磁碟檔、靜態RAM，以及動態heap/stack peak。** 以下單位均為bytes；本次檔案可直接用`riscv-none-elf-size`重測。

### A. 目前完整image實測

| Image | text（含唯讀資料） | data | bss | data+bss | 含debug的ELF磁碟bytes |
|---|---:|---:|---:|---:|---:|
| 純C + GTest + FFF，裸機5 tests | 535138 | 4272 | 8720 | 12992 | 4402908 |
| FreeRTOS C harness，無GTest | 53588 | 1760 | 266960 | 268720 | 182652 |
| FreeRTOS + GTest + FFF，6 tests | 549934 | 4292 | 275296 | 279588 | 4533160 |
| MicroPython + GTest + FFF，11 tests | 670978 | 4272 | 537168 | 541440 | 5996988 |

不是「GoogleTest單體=535138 bytes」：這些包含產品/測試、FFF、GoogleTest、被linker保留的libstdc++/newlib、BSP glue等。ELF帶`-g` debug/symbol資料，因此磁碟4–6MB不代表需要4–6MB Flash或RAM。link map與實際LOAD segments才是判讀載入範圍的依據。

本例 `.text`也載入RAM；`data+bss`僅是可寫靜態部分，不能當此QEMU image總RAM用量。若真板可從Flash直接執行code，則code與RAM預算分法又不同；還有padding/alignment不能忽略。

### B. 新增受控實驗：只加framework，成本增加多少？

用同一RV32 toolchain/flags/linker/startup/syscalls做兩個可實跑控制組。兩者都沒有產品code、FFF或任何TEST case：

| 控制組 | text | data | bss |
|---|---:|---:|---:|
| C++ runtime-only：stdio banner後return0 | 8902 | 1380 | 388 |
| 同控制組＋InitGoogleTest/RUN_ALL_TESTS，0 tests | 514638 | 4268 | 7600 |
| 差額 | **505736** | **2888** | **7212** |

所以此build下「空GoogleTest及其額外拉入的C++/C runtime」增加約493.9KiB code/rodata與約9.9KiB可寫靜態data+bss。**這不是純GoogleTest source單獨的大小，也不是所有build的常數。** 添加更多測試、types、strings、matcher或功能會改變結果；換libc、compiler、optimization/LTO、路徑字串也會改變。

重跑：

```sh
# 先完成兩個專案的正常verify_all，產生上述images。
cd rv32_gtest_poc
scripts/measure_framework_cost.sh
cat evidence/framework-cost.json
```

[量測腳本](../scripts/measure_framework_cost.sh)、[JSON原始結果](../evidence/framework-cost.json)、[完整build log](../evidence/framework-cost-build.log)。控制組採same O2/GC-sections，沒有用空測試假冒功能測試；兩個控制組都在QEMU執行exit0。

### C. RAM「配置量」不是「最低需求」

已確認的本例配置：

- QEMU啟動`-m 32M`；linker使用起點0x80000000的16MiB區間。
- `__stack_top=0x81000000`，`__heap_end=0x80fe0000`，為startup stack保留128KiB。
- `_sbrk`可用區是static image後至`__heap_end`。裸機5-test build為16,097,936 bytes容量，**不代表實際消耗16MB，也不代表一定需要16MB**。
- FreeRTOS `configTOTAL_HEAP_SIZE=256*1024`，這個array已計入RTOS image的BSS；不要再加一次。
- 三個應用task各`2048`個`StackType_t`，RV32下每個8KiB，從FreeRTOS heap配置；idle task最小stack512 words=2KiB。ISR stack1024 words=4KiB為靜態配置。不要把task stack與已含它們的kernel heap重複相加。
- MicroPython GC heap固定512KiB，已計入其BSS。stdout捕捉buffer另4KiB。C++/newlib heap是另一個來源。

**尚未量測**：C++/newlib heap peak、startup/task stack實際high-water、各測試最大live objects、可可靠運行的最小RAM。因此目前不能承諾「只要X KiB RAM就足夠」。下一步若需縮到产品预算，应记录_sbrk高水位、RTOS task stack high-water/heap peak、MicroPython GC used/free，再以最坏测试与余量驗證。

來源：[FreeRTOSConfig.h](../freertos/FreeRTOSConfig.h)、[demo.c](../freertos/demo.c)、[syscalls.c](../freertos/syscalls.c)、[link.ld](../freertos/link.ld)、[MicroPython bridge](../../micropython_gtest_poc/src/interpreter_bridge.c)。

### D. 可重現不是bit-identical

獨立解包到另一個絕對路徑時，GoogleTest的`__FILE__`字串可改變text大小；例如MicroPython image的獨立重建text為671106而本目錄為670978，data/bss相同。功能/案例數/exit結果一致，但本次沒有使用prefix-map建立bit-identical build承諾。

## FAQ-004　自訂 entry point 怎麼改？FreeRTOS下順序有差嗎？

**GoogleTest本身不必擔任CPU reset entry。** 應由你的BSP完成CPU/C/C++ runtime初始化，再呼叫test runner。

### Host

```text
OS CRT → main(argc, argv)
       → testing::InitGoogleTest(&argc, argv)
       → RUN_ALL_TESTS()
       → return result
```

本例使用自己的[test_main.cc](../tests/test_main.cc)，沒有link `gtest_main`，所以不會產生兩個main。

### RV32裸機

```text
ELF ENTRY(_start)
 → 設gp、sp；清BSS
 → __libc_init_array()（static constructors/TEST註冊）
 → extern "C" demo_main()
 → InitGoogleTest / RUN_ALL_TESTS
 → return result → _exit(result)
 → QEMU test finisher：host process exit0/1
```

對應[start.S](../freertos/start.S)與[bare_main.cc](../tests/bare_main.cc)。`demo_main`用C linkage，讓assembly能以未mangle的symbol呼叫。裸機沒有OS傳argv，因此自行建立`argc=1`、app name及NULL終止argv；需要filter等flag時，可在runner配置。

### 真正FreeRTOS

```text
_start / C++ constructors / trap vector
 → C demo_main：建立queue與producer/worker/supervisor三task
 → vTaskStartScheduler()
 → supervisor task內 rv32_run_gtests()
 → RUN_ALL_TESTS()
 → OS行為GoogleTest阻塞時，producer/worker實際由scheduler執行
 → 結果回到supervisor → _exit(result)
```

constructors在scheduler前執行；真正測試則在supervisor task中執行。GoogleTest/FFF只由該task使用，不宣稱其無pthread版本支援多task同時呼叫。另有FreeRTOS idle task，所以「三task」指三個應用task。

### MicroPython

同樣由`_start → demo_main`啟動；[main.cc](../../micropython_gtest_poc/tests/main.cc)先保存有效main-stack-top供GC使用，再跑GoogleTest。每個fixture各自init/deinit interpreter。MicroPython第二階段是裸機，**沒有再放入FreeRTOS多task**。

### 搬回真板

沿用板子的reset handler/linker/CRT；在runtime與必要UART初始化完成後，呼叫自己的test runner。FreeRTOS則在指定task內呼叫。不要重複清BSS、重複跑constructors或同時link兩份CRT/main。QEMU finisher不是產品裝置：真板應改成UART結果回報、停機loop或測試台可辨識的完成訊號。

真晶片log已知921600 baud；本例虛擬UART不等於其線速或阻塞時間。GoogleTest列印的ms/QEMU tick不證明真晶片速度，更沒有模擬64KiB L1、256KiB L2或1-cycle ILM/DLM。

## 維護方式

後續新問題從FAQ-006往後追加，寫明「短答、實作/實測證據、適用範圍、未驗證項」。修正舊題保留編號並補更新日期；不要把未測估計改寫成pass。README連到本檔，避免每次另建互相矛盾的FAQ副本。

## FAQ-005　這次有在QEMU加入快取機制或timing model嗎？

**沒有。** 目前使用官方QEMU執行RV32指令、UART與FreeRTOS所需timer/排程驗證。沒有新增或校準產品的64KiB L1、256KiB L2、ILM/DLM 1-cycle bypass-cache、memory latency或cycle-accurate core model。

`qemu_riscv_cache_googletest`是研究方向的repo名稱，不能視為cache model已完成。TCG自身的translation/code cache也不是被模擬產品的L1/L2 cache。

已完成的是功能測試移植、框架/RTOS/interpreter實跑與可重現交付。Cache/timing工作列在[PLAN階段D](../../docs/PLAN.md)，仍需真core規格、trace與真機校準；GoogleTest ms、QEMU wall time、virtual ticks均不能證明200→150ms。
