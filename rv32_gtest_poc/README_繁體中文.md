# RV32：純 C + GoogleTest + FFF + 真正 FreeRTOS 實跑原型

日期：2026-10-03。所有工作均在 dot 雲端 Linux 完成，未使用使用者電腦、公司程式碼或產品機密。本套示範程式為新寫的合成案例。

## 結論

**可以。GoogleTest + FFF 不必被替換成另一個框架，也不必先經過 Linux。** 本原型已把同一份 `src/product.c` 以 C compiler 編譯，再與 C++ GoogleTest/FFF 連結，在以下環境實際通過：

1. Host Linux x86_64：5 個 GoogleTest 產品測試。
2. RV32 QEMU、無 OS：相同 5 個 GoogleTest 產品測試。
3. RV32 QEMU、真正 FreeRTOS kernel：相同 5 個產品測試，加 1 個 GoogleTest OS 行為測試，共 6 個。
4. 獨立 FreeRTOS C harness 也通過相同 OS 行為檢查。

各條路徑另有故意錯誤的期望值；測試器均偵測失敗並回傳 exit 1。`scripts/verify_all.py` 對 positive 預期 exit 0、negative 預期 exit 1；negative 被抓到，才算驗證器 PASS。RTOS+GoogleTest負案例用filter只跑刻意錯誤的產品oracle；OS錯誤注入則由獨立rtos-negative驗證。

這是移植可行性的實驗證據，不代表現有大型測試執行檔可以不重編譯直接使用，更不代表整套產品測試都已移植。

## 真正驗證了什麼

`product.c` 是無 OS、無動態配置、無浮點、無全域狀態的純 C 函式：對固定 ADC-like 輸入加 offset、clip 到 0..4095，產生 sum/peak/clipped/FNV-like checksum。另一份 C adapter 透過 `sensor_read` 與 `sink_write` 介面工作。

C++ GoogleTest 以 FFF fake 這兩個 C linkage 介面，驗證：
- 固定重播輸入與 checksum；輸入位於 `fixtures/replay.h`
- NULL / 超長輸入、空輸入
- driver 回傳錯誤時不可輸出
- 成功輸出、fake 呼叫數與呼叫順序
- driver 回傳超過 buffer 容量時拒絕

FreeRTOS 的三個應用 task：producer（priority 1）、worker（2）、supervisor / GoogleTest runner（3）。此外 kernel 自有 idle task；因此「三個」指應用 task，不是系統 task 總數。

OS 測試檢查：queue 空佇列 3 ticks timeout、阻塞後喚醒、producer delay、8 次資料處理、8 次高優先權 worker 在 xQueueSend 回傳前已處理資料、task notification 的交付。這些是實際 scheduler / portASM / machine timer 驅動，不是 mock RTOS，也不是用 host thread 假裝。

OS 測試被包在真正 `TEST(FreeRtosBehavior, ...)` 中；產品測試仍保留 FFF。FFF global fake state 與 GoogleTest 只由 supervisor task 使用，**沒有宣稱此組停用 pthread 的移植可供多 task 同時呼叫 GoogleTest/FFF**。

## 示範硬體設定，不是產品事實

- ISA / ABI：`rv32imac_zicsr` / `ilp32`，ELF32 RISC-V
- QEMU：`virt`、`-cpu rv32 -smp 1 -m 32M -bios none`
- M-mode firmware 起點：0x80000000；linker 使用 16 MiB RAM 區域
- UART：0x10000000；CLINT mtime：0x0200bff8；mtimecmp：0x02004000
- QEMU virt timer 使用 10 MHz，FreeRTOS tick 1 kHz
- 沒有 MMU/Linux/OpenSBI 的依賴；`-bios none` 載入自有啟動碼

產品的 exact core、ISA extensions、memory map、interrupt controller 與 FreeRTOS port 尚未知，因此全部明示為 demo profile。不要把這份 linker script 當產品 linker script。

## GoogleTest 裸機移植的實際代價

GoogleTest 原始碼未修改，使用正式 release 1.15.2；FFF 1.1；FreeRTOS Kernel 11.1.0。

本次用 xPack GCC 15.2.0-1 的 RV32 multilib C++ runtime / newlib。不是只有「能編譯 C」就足夠：GoogleTest 需要 C++ standard library、動態配置、stdio 與靜態建構子初始化。

移植層包含：
- `_start` 設 sp/gp、清 BSS、呼叫 `__libc_init_array`，讓 TEST registration 的 C++ static constructors 執行
- linker `.init_array`、`.data`、`.bss`、heap/stack 區域
- `_write`→UART、`_sbrk`→有界 heap、`_gettimeofday`→mtime
- `_exit`→QEMU test finisher，使 host 自動化能辨識 exit 0 / 1
- `_fstat` / `_isatty` / `_read` / `_close` 等最小 newlib hooks
- 自訂 startup 未引入 GCC crtbegin 時，提供 `__dso_handle`

編譯設定停用 exceptions、RTTI、pthread、POSIX regex、stream redirection、filesystem。`_POSIX_C_SOURCE=200809L` 只開啟 newlib 所需宣告；它不代表提供 POSIX OS。首次實際 compile 遇到 fileno 宣告缺失、link 遇到 __dso_handle，原始錯誤 log 均保留。

因此以下項目沒有被驗證、也不能直接套用：death tests/fork、gtest XML 檔案輸出、stdio capture、多 thread 測試、exceptions/RTTI-dependent 測試、完整檔案系統、socket、locale/時間完整語義等。本例 `_read` 回 EOF、`_kill` 不支援。最小 syscall shim 不是完整 BSP。`_sbrk`、newlib stdio/allocator 尚未加多task lock或完整reentrancy；目前只有supervisor使用GoogleTest/newlib C++配置，worker/producer在成功路徑不做配置或stdio，不可直接當成量產多執行緒runtime。

純 bare GoogleTest firmware 的 text 約 535 KB；整合 FreeRTOS 約 549 KB（exact `size` 在 build log）。RTOS heap 配置 256 KiB，C++ heap 是另一區，並非已測到真實 peak usage。這顯示完整 framework 成本不小；不要推論能直接塞進產品約 1 MB code / 2 MB data 之外的剩餘容量。ELF 的 RWX LOAD warning 是示範單 RAM linker 的已知警告；未建立產品 PMP/MPU/區段保護。

## 可重現執行

建議 baseline：Debian 13 x86_64，已有 gcc/g++、Python 3.11+、bash、curl、tar、dpkg-deb、timeout 及 `evidence/environment.txt` 所列共用函式庫。其他 host 請提供對應工具鏈/QEMU，不要假設 Linux x64 二進位可在 macOS 執行。

線上預備（只寫專案內 tools/vendor，不 sudo、不改系統）：

```sh
cd rv32_gtest_poc
python3 scripts/prepare_tools.py
python3 scripts/verify_all.py
```

單獨跑最貼近需求的 FreeRTOS + GoogleTest + FFF：

```sh
scripts/build_rtos_gtest.sh
scripts/run_qemu.sh build/rtos-gtest/firmware.elf
```

單獨裸機：

```sh
scripts/build_bare_gtest.sh
scripts/run_qemu.sh build/bare-gtest/firmware.elf
```

每次編譯都先用 gcc 編 C module，再用 g++ 編測試/框架、link。相同 source 檔並沒有偷偷改成 C++。

`evidence/results.json` 是最近完整驗證命令、預期與實際 exit code；各步 log 在 evidence。測試 firmware build 失敗時不會被報成 pass。每次 QEMU 有 30 秒 timeout；timeout 124 不是成功。

## 公司離線搬運

交付的 source archive 含新寫的程式、腳本、所有測試、GoogleTest/FFF/FreeRTOS 的原始 release tarballs、QEMU 對應 Debian source 三件組、licenses、依賴 URLs/版本/SHA256、已跑過的 logs。**source archive 本身不含 433 MB cross compiler 與所有 QEMU binary deb**，不能在一台完全空白離線主機上憑空編譯。

可在允許聯網的外部預備機先執行 `prepare_tools.py`，再將整份目錄（含 tools 與 vendor）搬進符合公司規定的內網。或者合併另附的離線工具快取包（若同時收到）。內網執行：

```sh
python3 scripts/prepare_tools.py --offline
python3 scripts/verify_all.py
```

`--offline` 不發任何網路請求；缺依賴或 SHA256 不符會停止。prepare 是重新解壓並验证，不是從 source 重建 compiler 或 QEMU。跨不同 Linux baseline 仍可能缺系統動態庫；完整共用庫 closure 或受控 VM/container image 應由公司環境工程補齊。此工具快取已包含本雲端缺少的QEMU依賴，但不冒稱適合任意空白OS。

`dependencies.lock.json` 鎖定取回的實際 bytes；下載以 HTTPS 官方來源，tarball SHA256 留存，xPack 額外與發布者 .sha 比對一致。Debian deb/source 與官方 index SHA256 比對；本次未額外做 Debian Release/PGP 信任链验证。企業環境應依內部供應鏈規範重新做來源審核、簽章與掃描。

QEMU source：`qemu_10.0.13+ds.orig.tar.xz` 加 `.debian.tar.xz` 與 `.dsc`，對應本次執行的 Debian QEMU 1:10.0.13+ds-0+deb13u1。這是 Debian `+ds` source distribution，並非宣稱含上游所有未使用 firmware/submodules。source build 尚未實跑；本次執行的是 Debian 官方 binary。可用 `dpkg-source -x ...dsc` 取得帶 Debian patches 的樹；其 Build-Depends 在 .dsc，離線 source build 必須事先鏡像這些額外依賴。若要改 QEMU 做 cache/ILM/DLM model，這就是下一階段起點，不能當本次已完成。

## Linux 路徑與效能界線

替代路徑是 RV32 Linux userspace GoogleTest + QEMU user-mode，或 RV32 Linux kernel/system emulation。它通常更容易保留 POSIX 功能，但需要正確 RV32 Linux C++ sysroot/ABI；`riscv64-linux-gnu` 的 distro toolchain 不一定有 RV32 multilib。**本次未實跑 Linux 中介，因已直接完成裸機及 FreeRTOS 實驗；不要把任一路徑的 pass 混稱另一條。**

QEMU TCG 是 functional emulation，不是你產品的 cycle-accurate microarchitecture。log 中 GoogleTest `(N ms)` 或 FreeRTOS tick 只描述這次模擬的時間來源與排程，不是產品 latency。沒有模擬 64 KB L1、256 KB L2，也沒有建模 instruction ILM / data DLM 1-cycle bypass-cache。200→150 ms 的目標仍需產品指令/記憶體 traces、真機校準或合適 timing simulator/model。

## 接回使用者大型 GoogleTest 執行檔

1. 留住 host baseline 與 fixture；把 product C source 列表、extern C mock seam、GoogleTest/FFF 版本固定。
2. 先用本例 RV32 toolchain link；列出所有未解 symbols/host OS dependencies。
3. 把 test registration、C++ startup、heap/stack、printf、exit 先打通，不改 test 意圖。
4. 將測試分類：可直搬純邏輯、需 syscall/RTOS adapter、依賴 host-only 功能。逐類移植並保留原始 assertions。
5. 在真 FreeRTOS task 中執行 framework；對 OS 行為另加明確測試；不要假設 host-only 原測試已覆蓋 task競態/priority/timeout。
6. 最後用產品 BSP、正確 ISA/ABI與 linker取代本例 virt platform。功能等價通過後才做效能模型。

## 授權與來源

本套新寫 code 採 MIT（LICENSE）。GoogleTest：BSD-3-Clause；FFF/FreeRTOS：MIT。QEMU：主要 GPL-2.0，含各元件獨立條款，見 source 與 licenses/QEMU-Debian-copyright。xPack/compiler/runtime 各元件的原始 license 已保留在 licenses/xpack-components.tar.gz（原notice完整壓縮包，可解開檢視），含 GCC/libstdc++ runtime exceptions；不是整包都 MIT。

只在公司內部使用、修改與對外散布是不同情境。這份清單不是法律意見；把QEMU/GCC修改或binary交付第三方前，依各license檢查對應source與notice義務。來源完整URL與SHA256以 dependencies.lock.json 為準。
