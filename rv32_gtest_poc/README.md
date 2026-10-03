# RV32 GoogleTest / FFF / FreeRTOS 實跑實驗

同一份純C產品模組與C++ GoogleTest/FFF，已在host、RV32裸機、真正FreeRTOS上跑過。第二階段使用真正MicroPython interpreter，在host/RV32跑同一組新測試。

這是一個可重現的功能驗證專案；不是cycle-accurate性能模型，也不是已移植使用者的私有產品code。尚未提供GitHub repository URL，所以本地整理完成，**未push/publish**。

## 結果與入口

| 路徑 | 結果 |
|---|---|
| 純C模組＋GTest/FFF host | 5 tests PASS |
| 相同測試 RV32裸機 | 5 tests PASS |
| 真FreeRTOS＋GTest/FFF | 6 tests PASS，含queue/tick/priority/notification檢查 |
| 真MicroPython host → RV32裸機 | 同一份11 tests，兩端PASS |
| 故意錯誤oracle | 每條路徑被偵測，exit1 |

- [持續追加的FAQ](docs/FAQ.md)：重編、C++ linker、實測大小/RAM、entry point
- [第一階段完整繁中說明](README_繁體中文.md)
- [MicroPython README](../micropython_gtest_poc/README.md)
- [版本/下載/雜湊lock](dependencies.lock.json)
- [實際完整驗證記錄](evidence/results.json)

## 前置條件

已驗證Debian13 x86_64；需要gcc/g++、GNU make（MicroPython生成步驟）、Python3.11+、bash、curl、tar、dpkg-deb、timeout，以及[evidence/environment.txt](evidence/environment.txt)所列QEMU system libraries。工具只解壓至專案tools，不sudo、不改系統。不是任意空白OS的turnkey rootfs。

Demo target：`rv32imac_zicsr / ilp32`、單hart、QEMU virt、`-bios none`。產品exact core/ISA/BSP仍未知。

## 第一階段：從來源重現

聯網預備機：

```sh
cd rv32_gtest_poc
python3 scripts/prepare_tools.py
python3 scripts/verify_all.py
```

離線：先在許可的聯網環境取得lock指定的所有tar/deb，或合併既有工具快取包，將整份目錄搬入公司內網，再執行：

```sh
python3 scripts/prepare_tools.py --offline
python3 scripts/verify_all.py
```

`--offline`缺bytes或SHA不符即失敗；不嘗試網路。QEMU source三件組已附，但本次執行官方Debian binary，**未實跑QEMU source build**。

單跑路徑：

```sh
scripts/build_host.sh
build/host/product_tests
scripts/build_bare_gtest.sh
scripts/run_qemu.sh build/bare-gtest/firmware.elf
scripts/build_rtos_gtest.sh
scripts/run_qemu.sh build/rtos-gtest/firmware.elf
```

## 第二階段：MicroPython

將兩個專案並排，先完成第一階段工具準備：

```text
workspace/
  rv32_gtest_poc/
  micropython_gtest_poc/
```

```sh
cd ../micropython_gtest_poc
python3 scripts/verify_all.py
```

第二階段內含MicroPython source tar，generation/build/run不連網、不修改第一階段工具。11個新測試涵蓋parser、bytecode VM、objects、MPZ、GC與exceptions；不是全套upstream suite。詳見其README。

## 大小控制實驗

先完成兩個專案build，再執行：

```sh
cd ../rv32_gtest_poc
scripts/measure_framework_cost.sh
cat evidence/framework-cost.json
```

這兩個控制組沒有test cases，只量化空GoogleTest+額外runtime的保留image成本；不能拿0tests/exit0證明產品功能。完整解釋與RAM未測項在[FAQ-003](docs/FAQ.md)。

## 已踩過的坑

- host binary不能直接跑RV32；ISA/ABI/C++runtime必須相容。
- FFF1.1附舊gtest目錄；include path必須先放本次GoogleTest1.15.2。
- 漏`.init_array`/constructors可能0tests仍exit0；正常runner必須檢查5/6/11預期數量。
- 自訂CRT曾缺`__dso_handle`；newlib曾缺`fileno`宣告，修正與首次failure log均保留。
- 停用C++ exceptions不等於關閉MicroPython Python exceptions。
- `.data` copy、UART/clock/IRQ/ILM/DLM真板配置不能照抄QEMU RAM-only linker。
- ELF含debug磁碟大小，不等於Flash/RAM；heap/stack peak尚未量測。
- FreeRTOS heap與MicroPython GC heap已計入BSS，不能重複加總。
- MicroPython變更config後要重新生成qstr/moduledefs，避免殘留registration。
- 絕對`__FILE__`路徑可改變code/rodata bytes，本次承諾功能重現，不承諾bit-identical。
- 真板921600 baud與QEMU console不同；QEMU ms不能證明產品200→150ms。

## 授權與文件版本

新code採MIT；上游元件保留各自license，見LICENSE、licenses及dependency lock。已有交付的source/tool archives保持不變；新增README/FAQ/量測腳本可作小型documentation update套入專案。未經明確授權不會自行建立remote或推送GitHub。
