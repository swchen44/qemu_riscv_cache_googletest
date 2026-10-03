# Debian13歷史baseline（不是Ubuntu安裝指南）

本repo最初在Debian13 x86-64完成15步RV32/FreeRTOS與8步MicroPython正負案例，保存實際log。新cloud還原後也重新實跑此baseline。

具備gcc/g++、GNU make、Python3.11+、bash、tar、dpkg-deb、timeout、curl及evidence/environment.txt所列host libraries後：

```sh
cd rv32_gtest_poc
python3 scripts/prepare_tools.py --offline
export RV32_HOST_PROFILE=debian-13
python3 scripts/verify_all.py
cd ../micropython_gtest_poc
python3 scripts/verify_all.py
```

離線需備齊dependencies.lock.json的全部bytes，包括原Debian QEMU/deb及xPack archive。此路徑不代表Ubuntu24.04相容性；Ubuntu請讀另一份指南。
