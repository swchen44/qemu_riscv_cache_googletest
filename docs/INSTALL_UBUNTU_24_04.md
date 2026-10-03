# Ubuntu24.04 x86-64 安裝與驗收

此為使用者確認的內網target。不能把歷史Debian13 QEMU deb直接當Ubuntu工具。

1. 先執行 `python3 scripts/preflight.py --profile ubuntu-24.04`。不符distro/version/architecture時停止。
2. 取得本repo指定的Ubuntu官方.deb離線bundle與manifest，按其固定版本安装指南處理host gcc/g++/make/Python/QEMU及依賴；涉及內網管理員權限時由當地授權管理員執行，不繞過公司政策。
3. 確認xPack官方archive置於rv32_gtest_poc/tools，GoogleTest/FFF/FreeRTOS source tar在vendor。
4. 只解共用sources/cross compiler，**不解Debian QEMU包**：

```sh
python3 scripts/prepare-ubuntu-sources.py .
export RV32_HOST_PROFILE=ubuntu-24.04
export QEMU=/usr/bin/qemu-system-riscv32
(cd rv32_gtest_poc && python3 scripts/verify_all.py)
(cd micropython_gtest_poc && python3 scripts/verify_all.py)
```

Ubuntu profile確認/etc/os-release，不注入專案Debian LD_LIBRARY_PATH。不要從先前的Debian shell繼承該路徑。

## 狀態說明

- 官方Ubuntu source/package metadata、版本、SHA與離線閉包正在準備。
- PRoot userspace試驗因sandbox拒絕ptrace/execve而停止，這不是Ubuntu test pass。
- 另以無KVM/無ptrace的純TCG完整Ubuntu VM嘗試實跑；成功/失敗以後續log更新，不預先標完成。
- 若只能做ABI/static檢查，必須仍由內網真Ubuntu執行上述matrix才算Ubuntu驗收。

## 不假設公司帳號有root/sudo或網路

先跑preflight，只讀環境，沒有安裝或安全設定變更。依結果選分支：

| 狀況 | 路線 | host需要root？ |
|---|---|---|
| gcc/g++/make/Python與Ubuntu QEMU已由IT裝好 | 直接用現有工具，但核對版本/ABI並記錄差異 | 不需要 |
| 只缺RISC-V cross compiler | 在專案tools或$HOME解壓固定xPack archive，局部PATH | 不需要 |
| 缺QEMU但host build工具已齊 | 可用`dpkg-deb -x`將Ubuntu QEMU及完整shared-lib依賴私有解壓，使用僅對該process生效的wrapper；必須先檢查ldd/loader及跑matrix | 解壓不需要；此私有QEMU路線未完成獨立驗證前不可標PASS |
| 缺host compiler/headers/make/Python等 | 將鎖定的Ubuntu.deb閉包交IT，依公司流程離線安裝；不要擅自sudo | 系統安裝需要當地授權管理員 |
| 沒管理權限且私有依賴不完整/ABI不合 | 停止，列出缺檔/版本/符號給IT或操作者；不要偷偷連外或改security設定 | 不嘗試繞過 |

`dpkg-deb -x`只提取檔案，不執行package maintainer scripts，不等於完成正常system package安裝。不要把未測私有prefix當成適用所有Ubuntu機器的承諾。尤其host compiler的sysroot/headers/cc1路徑與QEMU外掛/data路径要一起處理，不能只搬一個binary。

任何export只放在本次shell或wrapper中，不修改全域/etc、使用者永久啟動檔或company policy。沒有網路是正常offline流程，所有required bytes應事先帶入。
