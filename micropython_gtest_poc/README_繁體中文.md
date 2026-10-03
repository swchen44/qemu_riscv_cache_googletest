# MicroPython 真實 interpreter：同一組 GoogleTest 從 host 跑到 RV32 QEMU

完成日期：2026-10-03。全程在 dot 雲端，未動用使用者 Mac、公司 source 或真晶片。

## 已完成的實驗

使用官方 MicroPython **v1.26.1**，commit **647c8b96cae7e202c7a020395b7cfe65e5b8ce04**，加上新寫的 C API 測試 bridge 與 C++ GoogleTest。

- Host Linux x86_64：11 / 11 個新 GoogleTest PASS，exit 0。
- QEMU `virt`、真正 RV32 ELF32 裸機：**完全同一份 `tests/interpreter_test.cc`**，11 / 11 PASS，exit 0。
- Host/RV32 各執行一個刻意錯誤 oracle（6 × 7 預期為 43），均被 GoogleTest 抓到，exit 1。
- 使用 GoogleTest 1.15.2；FFF 1.1 只 fake stdout 平台輸出函式，**MicroPython parser/compiler/VM/object/GC 沒有被 mock**。
- 沒有改用 RV64，沒有 Linux guest/userspace 中介，也沒有將測試換成 Unity。

最新自動驗證命令/exit code 在 `evidence/results.json`。shared test source 的 SHA256 也存於同檔。正負案例完整 log 都保留；QEMU 30 秒 timeout 會被判失敗，而不是 pass。

這是「新增的 11 個有意義功能測試」；**不是全套上游 MicroPython regression suite**、不是所有 ports/標準庫功能都通過，也不是移植使用者尚未提供的產品測試。

## 怎麼確定是真的 interpreter

以官方 `ports/embed/embed.mk` 生成可嵌入 C sources，保留 MicroPython upstream implementation。新寫 `src/interpreter_bridge.c` 透過真正的：

- `mp_lexer_new_from_str_len` 與 `mp_parse`：token/parse
- `mp_compile`：生成 interpreter bytecode
- `mp_call_function_0`：進入 MicroPython VM 執行
- `mp_obj_get_int`：取回 runtime object 的整數值
- `gc_collect`、`gc_info`：實際 GC

ELF 的 parser/compiler/VM/GC/MPZ symbols 可見 `evidence/interpreter-symbols.txt`。C runtime sources 使用 gcc 編譯，不當成 C++；只有測試、main、GoogleTest 用 g++。

host build 列出 132 個 C translation units，RV32多兩個startup/syscall單元。但當中有被 feature macros 關閉的 upstream 檔案，不能拿此數字聲稱 132 個功能全部啟用。

## 同組 11 個測試覆蓋

1. ParserArithmeticPrecedence：運算優先序、括號與除法。
2. FunctionsRecursionAndBytecode：編譯遞迴函式，計算 8!。
3. ListDictComprehensionAndMutation：list comprehension、dict、索引、append 與 sum。
4. MpzBigIntegerBeyondRv32Word：`1 << 100` 的多精度整數，檢查運算與模數；不靠host word size偷過。
5. PythonExceptionCatchAndFinally：真正 Python ZeroDivisionError、except、finally。
6. SyntaxErrorThenInterpreterRecovers：C 邊界收到 SyntaxError 後，下一段script仍可執行。
7. RuntimeTypeErrorThenInterpreterRecovers：TypeError 與恢復。
8. GlobalsObjectsSurviveExplicitGc：global dict/list roots 在GC後保留。
9. AllocationChurnCollectsAndKeepsRoots：1000輪 1 KiB bytearray/list/string 配置與反覆GC，存活物件仍正確。
10. RealPrintUsesFffPlatformBoundary：真正 Python print 產生 `rv32 42\n`，FFF捕捉輸出。
11. IndependentFixtureStartsWithoutPreviousGlobals：每個fixture重新初始化 interpreter，前測試globals不洩漏。

C bridge的`interpreter_eval_int`雖使用int64_t輸出容器，內部`mp_obj_get_int`只接受machine-word signed range，RV32為32-bit；不是任意64-bit/MPZ轉換API。100-bit MPZ在Python內部完成運算/斷言，返回範圍內的modulus供GTest比對。

每個 fixture setup/deinit 都使用同一 C bridge。沒有用不同的 host expected values 與 target expected values。刻意失敗案例平時 disabled，負測试明確啟用。

## Port/config 限縮與實際遇到的問題

採官方 embed port，加 `MICROPY_CONFIG_ROM_LEVEL_BASIC_FEATURES`；啟用 compiler、GC、gc module、MPZ。外部檔案 import、IO/open、float 關閉。使用 setjmp-based MicroPython NLR 及 GC register capture，維持 host/RV32相同設定；不是測原生RV32 assembly NLR或native-code emitter。Python bytecode compiler/VM 是真的。`MICROPY_STACK_CHECK=0`，只執行本包有界腳本，尚未驗證stack overflow；沒有宣稱可安全執行任意不受控script。

初次 host 編譯實際遇到 basic profile 必需的 `MICROPY_PY_SYS_PLATFORM` 缺口；加了明確字串。IO 預設需要 `mp_builtin_open_obj`，本次無檔案系統，因此**明確關閉 IO**，沒有塞會永遠回成功的假open。另辨識出舊 generated moduledefs 會殘留 io registration；現在 generation 用 config-hash 專屬 buildgen 目錄，避免換config後沿用stale fragments。初次失敗 log 保留在 evidence。

NLR/longjmp 的捕捉完全待在 C bridge 裡，返回普通 error code/type string；不讓 longjmp 跨越 GoogleTest 的 C++ frame/destructor。C++ exceptions/RTTI/pthread/file output/stream capture 的裸機限制沿用前一階段。平台輸出只捕捉 print 字串，未替代 interpreter內部行為。capture使用有界4096-byte固定buffer與memcpy，不丟出C++ exception；overflow會讓測試失敗。

此範例為單執行緒裸機 MicroPython runtime。第二階段沒有再把 MicroPython 放入 FreeRTOS三tasks；第一階段FreeRTOS+GoogleTest已獨立驗證。不宣稱GC根掃描、newlib allocator或stdio已支援任意多task使用。GC案例驗證globals roots與配置churn，未完整覆蓋所有register roots、OOM或stack-overflow情況。

## RV32與記憶體

- Demo ISA/ABI：rv32imac_zicsr / ilp32，單hart，QEMU virt，32 MiB QEMU RAM。
- 無Linux、無OpenSBI，`-bios none`，自有startup/linker與newlib syscalls。
- Firmware text **670,978 bytes**、data **4,272 bytes**、bss **537,168 bytes**（本次size log）。
- BSS包含固定 **512 KiB MicroPython GC heap**。另有 C++/newlib heap 與 stack，並未量測兩者的peak。
- 這些數字是此縮減功能profile的image，不是完整MicroPython所有功能，也不是使用者產品的容量證明。
- 單RAM linker 的 RWX LOAD warning 已知，沒有建構量產PMP/MPU保護。

## 重現與離線使用

本階段沿用已交付的第一階段工具。目錄應並排：

```text
workspace/
  rv32_gtest_poc/       # 第一階段 source + 已解壓的工具
  micropython_gtest_poc/
```

第一階段先完成 `python3 scripts/prepare_tools.py --offline`（如果工具尚未準備好，依第一階段離線重組說明）。baseline仍是具備必要system libraries的Debian13 x86_64，非任意空白OS。本階段另外明確需要 GNU make；其餘包含 gcc/g++、Python3、bash、tar、timeout。

MicroPython來源tarball已包含在本包，不需要連網。執行：

```sh
cd micropython_gtest_poc
python3 scripts/verify_all.py
```

這會先驗證source hash與第一階段dependency lock，再生成embed sources、build/run host、跑host负案例，最後build/run RV32正負案例。所有結果寫到evidence。**沒有下載網路依賴、沒有修改第一階段工具或系統設定。**

單獨跑：

```sh
python3 scripts/prepare.py
python3 scripts/build.py host
build/host/micropython_tests
python3 scripts/build.py rv32
scripts/run_rv32.sh
```

upstream MicroPython source不改動；新增的config/bridge/tests/buildscripts在本專案，不需要把GoogleTest插進每個upstream C檔案。若要搬回公司repo，可將相同embed generation與測試target納入自己的buildsystem，再接到產品BSP。

## 真晶片921600 baud與時間界線

使用者補充：**真晶片log使用921600 baud**。此資訊已記入依賴/實驗設定；尚未收到serial framing、UART clock、polling/DMA/interrupt/log buffer細節。

本次QEMU virt UART只是虛擬console通路，沒有宣稱設定或模擬真板921600 baud的serialization delay。真板移植需按實際UARTclock算divisor並設定921600；不能照抄virt地址或假定clock。

若實際使用8N1，每byte線上需10bit，理論serialization rate約92,160 bytes/s；1KiB純線上時間約11.1ms，另加格式化、阻塞、driver/RTOS等成本。這是**明示8N1假設下的估算**，不是已測你的硬體，也不能直接從hotpath扣掉固定值。

GoogleTest/QEMU log裡的ms均不可當真晶片速度。未建模64KB L1、256KB L2或1-cycle ILM/DLM；本階段沒有驗證200→150ms。真機性能量測應把log輸出關閉/緩衝或獨立計算，保留同一workload與計時邊界。

## 官方來源與授權

- Release：https://github.com/micropython/micropython/releases/tag/v1.26.1
- 固定commit：https://github.com/micropython/micropython/tree/647c8b96cae7e202c7a020395b7cfe65e5b8ce04
- 官方embed port说明：該commit的 `ports/embed/README.md`（本包upstream source內也有）
- 移植概念參考：https://docs.micropython.org/en/v1.26.0/develop/porting.html （1.26.0文件，實際code固定1.26.1）

MicroPython MIT、GoogleTest BSD-3-Clause、FFF MIT；各原始notice已保留。新bridge/tests/scripts採MIT（LICENSE）。版本、tag object、commit、archive SHA256與第一階段dependency lock SHA256在 `dependencies.lock.json`。未publish、未push、未送出upstream PR。
