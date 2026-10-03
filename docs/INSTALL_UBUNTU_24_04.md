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
(cd rv32_gtest_poc && RV32_QEMU_CLOCK=icount python3 scripts/verify_all.py)
(cd micropython_gtest_poc && RV32_QEMU_CLOCK=raw python3 scripts/verify_all.py)
```

Ubuntu profile確認/etc/os-release，不注入專案Debian LD_LIBRARY_PATH。不要從先前的Debian shell繼承該路徑。

## 狀態說明

- 官方121個Ubuntu binary依賴與62套/191檔對應source已實際下載驗SHA；Release發佈另列狀態。
- 完整純TCG Ubuntu VM已離線安裝並實跑23/23步：15/icount與8/raw。原始log見evidence/ubuntu-24.04。
- raw RTOS首wait曾timeout，保留失敗；正式功能命令必須明示icount，不將它當硬體cycle模型。
- 慢巢狀TCG可明示export VERIFY_TIMEOUT_SECONDS=1800；這是外層harness等待，不改內層QEMU30秒或RTOS100tick。
- PRoot因sandbox拒絕ptrace/execve而停止；最後成功使用另一允許的純TCG VM路線。

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

## Repo腳本與Release bundle的位置契約

Repo的`scripts/ubuntu/`不是工具bundle本身。三個腳本都要明確指定已解開的Release bundle root；該root須有`ubuntu-packages.lock.json`與`apt/archives/`。

```sh
B=/path/to/unpacked/ubuntu-offline-bundle
python3 scripts/ubuntu/verify-bundle.py --bundle-root "$B"
bash scripts/ubuntu/install-ubuntu-offline.sh --bundle-root "$B" --simulate
# 只有當地授權管理員才執行 --install；脚本不自動sudo。
```

Installer僅安裝lock列出的、已驗SHA的121個required packages，不掃描目錄順便裝多餘deb；PRoot probe不是required dependency。`fetch-locked.py --bundle-root "$B"`是外部允許聯網預備機才用的下載器。其`--include-all-sources`/`--include-tcg`/`--include-base`需bundle內有對應source/image/TCG manifests，不能只拿repo單一script便假定這些optional bytes存在。

## 不碰OS的helper自我測試

```sh
python3 -m unittest discover -s tests -v
```

五個測試只在暫存目錄放合成bytes，檢查explicit root、缺root、hash不符、path traversal與多餘未列檔案；不呼叫apt、不安裝、不連網、不需root。
