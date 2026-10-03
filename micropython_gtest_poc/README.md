# MicroPython：同一組 GoogleTest 從 host 到 RV32

官方MicroPython v1.26.1（commit `647c8b96cae7e202c7a020395b7cfe65e5b8ce04`），使用真正parser/compiler/VM/objects/MPZ/GC；新加的11個GoogleTest在host與RV32裸機全部實跑通過，兩端故意錯誤oracle均exit1。FFF只fake平台stdout。

- [完整實測/coverage/限制](README_繁體中文.md)
- [共用FAQ：重編、linker、大小、entry point](../rv32_gtest_poc/docs/FAQ.md)
- [固定版本與source hash](dependencies.lock.json)
- [驗證命令、預期/實際exit、相同test source SHA](evidence/results.json)

## 重現

baseline為有必要system libraries的Debian13 x86_64；明確需要GNU make、gcc/g++、Python3、bash、tar、timeout。先把第一階段`rv32_gtest_poc`工具準備好，兩目錄並排。

```sh
# 第一階段目錄，已備齊離線依賴時
cd rv32_gtest_poc
python3 scripts/prepare_tools.py --offline
# 第二階段
cd ../micropython_gtest_poc
python3 scripts/verify_all.py
```

單步：

```sh
python3 scripts/prepare.py
python3 scripts/build.py host
build/host/micropython_tests
python3 scripts/build.py rv32
scripts/run_rv32.sh
```

MicroPython tar已包含，不連網；prepare檢查自身source與第一階段lock hash。config-hash獨立生成目錄避免殘留qstr/moduledefs。C interpreter用gcc，test/framework用g++；host與RV32編同一`tests/interpreter_test.cc`。

## 不能誤讀的界線

- 11個新增功能測試，不是完整upstream regression suite。
- basic profile，關閉IO/external file import/float；不是所有標準庫/ports都測過。
- RV32 `virt`裸機，沒有Linux中介，第二階段沒有再跑MicroPython FreeRTOS多task。
- C++ exceptions/RTTI停用；Python exceptions仍由C NLR處理，已測。NLR不跨C++ frames。
- `interpreter_eval_int`內部是machine-word conversion；100-bit MPZ在Python內部驗證，並非任意int64/MPZ export API。
- GC案例不是所有register-root/OOM/stack-overflow coverage；`MICROPY_STACK_CHECK=0`，只執行有界測試script。
- stdout固定4KiB buffer，overflow使測試失敗；不在C callback裡丟出C++ exception。
- text/data/BSS與heap/stack的區別見共用FAQ；不能用ELF磁碟size當RAM需求。
- 真晶片log為921600 baud；QEMU virtual UART並非其時序，未驗證200→150ms或cache/ILM/DLM效能。

未修改upstream interpreter source，未push/publish。新bridge/tests/scripts MIT，上游notice保留。原交付tar不變，README/FAQ另版更新。
