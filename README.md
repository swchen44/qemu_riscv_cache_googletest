# RV32 GoogleTest / FreeRTOS / MicroPython 重現實驗

> 發佈中：本批先提供可讀的計畫與FAQ。source/evidence及required Release資產正在補齊；目前不可只下載這個中間commit便宣稱完整離線重現。

**目前是功能測試移植。尚未在QEMU實作自訂L1/L2、ILM/DLM或cycle/latency model。** Repo名稱中的cache是後續研究方向，不是已完成項；QEMU/GoogleTest的ms不能證明產品200→150ms。

目標：讓下一位工程師/AI把同一套C++ GoogleTest＋FFF從host帶到真正RV32，並把完整source與離線依賴移入公司Ubuntu24.04 x86-64環境。未包含公司私有code。

## 已實跑與尚未完成

| 路徑 | 證據狀態 |
|---|---|
| 純C host GTest/FFF | 5 tests PASS |
| 同份test source、RV32裸機 | 5 tests PASS |
| 真FreeRTOS三個應用task＋GTest | 6 tests PASS，含queue/tick/priority/notification |
| 真MicroPython host→RV32裸機 | 同一份11 tests，兩端PASS |
| 各路徑刻意錯誤oracle | 預期exit1，均被抓到 |
| 上述Debian13 baseline乾淨重建 | 已驗證，包括此次新cloud還原重跑 |
| Ubuntu24.04完整TCG VM | boot及host測試已PASS；FreeRTOS驗收發現completion同步race，已加入真barrier，正重跑 |
| 自訂cache/timing模型、產品150ms | 尚未實作/驗證 |
| 遠端commit新下載後重跑 | 首次推送後執行，未用本地結果冒充 |

詳細狀態以[Checklist](docs/CHECKLIST.md)、[證據說明](docs/EVIDENCE.md)與各evidence/results.json為準。

## 文件入口

- [計畫書與功能→性能階段](docs/PLAN.md)
- [Checklist](docs/CHECKLIST.md)
- [FAQ-001～005](rv32_gtest_poc/docs/FAQ.md)：重編、C++ linker、實測大小/記憶體、entry、尚未加入cache模型
- [實際踩坑紀錄](docs/PITFALLS.md)
- [Ubuntu24.04安裝](docs/INSTALL_UBUNTU_24_04.md) / [Debian13歷史baseline](docs/INSTALL_DEBIAN_BASELINE.md)
- [內網必帶清單與GitHub ZIP缺口](docs/OFFLINE_HANDOFF.md)
- [下一個AI接手指引](docs/AI_HANDOFF.md) / [Commit Why/What/Test](docs/COMMIT_GUIDE.md)
- [第一階段細節](rv32_gtest_poc/README_繁體中文.md) / [MicroPython coverage與限制](micropython_gtest_poc/README_繁體中文.md)

## Ubuntu重現流程

```sh
python3 scripts/preflight.py --profile ubuntu-24.04
# 先按Ubuntu指南準備官方.deb與xPack離線工具，並驗SHA256。
python3 scripts/prepare-ubuntu-sources.py .
export RV32_HOST_PROFILE=ubuntu-24.04
export QEMU=/usr/bin/qemu-system-riscv32
(cd rv32_gtest_poc && python3 scripts/verify_all.py)
(cd micropython_gtest_poc && python3 scripts/verify_all.py)
```

不假設公司帳號有sudo或網路：preflight後按安裝指南選已安裝、私有解壓或交IT離線安裝分支。

這不是拿Debian13 QEMU deb在Ubuntu上直接執行。Ubuntu profile檢查實際userspace，不注入專案Debian shared-library路徑。缺依賴就停止，內網不得暗中下載。

## 下載整包到內網：務必讀

普通Git包含本專案source/docs/tests以及GoogleTest/FFF/FreeRTOS/MicroPython固定release source tar。大型xPack工具、Ubuntu官方套件閉包與額外QEMU/compiler source將以同repo Release asset提供。

**GitHub「Source code(zip)」不包含Release assets。** 只按Download ZIP不能聲稱已帶齊離線工具；必須另外取得當前manifest列出的required assets、核對size/SHA。未完成Release上傳驗證前，離線交付仍標未完成。不依賴Library或私有下載URL；不使用LFS/submodule隱藏依賴缺口。

## 實驗版本與界線

GoogleTest1.15.2、FFF1.1、FreeRTOS11.1.0、MicroPython1.26.1（commit647c8b96cae7e202c7a020395b7cfe65e5b8ce04）、xPack GCC15.2.0-1；demo RV32IMAC+Zicsr/ILP32/virt，並非產品core規格。Ubuntu與Debian QEMU版本分開記錄於lock。

MicroPython是reduced embed profile與11個新增測試，不是完整upstream suite。真機UART已知921600 baud，但framing/driver/log overhead未實測。所有功能測試與真機性能評估分開；heap/stack peak與最小RAM仍未量測。

保留原MIT LICENSE與history。新code亦MIT；第三方license與原始source notices保留。每個commit寫Why/What/Test，不force-push、不碰無關repo。
