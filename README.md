# RV32 GoogleTest / FreeRTOS / MicroPython 重現實驗

> 目前推薦輕量路線：先用Ubuntu工程機已裝工具；無root時只probe/診斷，特殊xPack工具私有解壓到自己的專案目錄。四個大型Release附件保留為optional legacy，不是每位使用者的必帶項。

**目前是功能測試移植。尚未在QEMU實作自訂L1/L2、ILM/DLM或cycle/latency model。** Repo名稱中的cache是後續研究方向，不是已完成項；QEMU/GoogleTest的ms不能證明產品200→150ms。

目標：讓下一位工程師/AI把同一套C++ GoogleTest＋FFF從host帶到真正RV32，並把完整source與離線依賴移入公司Ubuntu24.04 x86-64環境。未包含公司私有code。此repo是public；帶入公司並加入真實code前，應先改用內部remote或停用外網push，不能把公司source/log/credentials推回這裡。

## 已實跑與尚未完成

| 路徑 | 證據狀態 |
|---|---|
| 純C host GTest/FFF | 5 tests PASS |
| 同份test source、RV32裸機 | 5 tests PASS |
| 真FreeRTOS三個應用task＋GTest | 6 tests PASS，含queue/tick/priority/notification |
| 真MicroPython host→RV32裸機 | 同一份11 tests，兩端PASS |
| 各路徑刻意錯誤oracle | 預期exit1，均被抓到 |
| 上述Debian13 baseline乾淨重建 | 已驗證，包括此次新cloud還原重跑 |
| Ubuntu24.04完整TCG VM | 23/23步PASS：第一階段15/icount、MicroPython8/raw；保留raw timeout對照失敗 |
| 自訂cache/timing模型、產品150ms | 尚未實作/驗證 |
| 遠端commit新下載後重跑 | 已從8844093新clone驗全Git SHA，15/icount＋8/raw與helper5 PASS（Debian；固定local dependency cache） |

詳細狀態以[Checklist](docs/CHECKLIST.md)、[證據說明](docs/EVIDENCE.md)與各evidence/results.json為準。

正常Ubuntu驗收證據：[23步原始log與profile](evidence/ubuntu-24.04/README.md)。低階runner預設raw；正式功能重現請明示以下已測profile。icount不是硬體效能模型。

## 文件入口

- [計畫書與功能→性能階段](docs/PLAN.md)
- [原始研究報告、方案比較與37個來源](docs/research/README.md)
- [Checklist](docs/CHECKLIST.md)
- [FAQ-001～007](rv32_gtest_poc/docs/FAQ.md)：重編、C++ linker、實測大小/記憶體、entry、尚未加入cache模型、工具source用途、無root安裝
- [Clock profiles與raw失敗](docs/CLOCK_PROFILES.md) / [固定依賴與source完整性](docs/DEPENDENCIES.md)
- [實際踩坑紀錄](docs/PITFALLS.md)
- [Ubuntu24.04安裝](docs/INSTALL_UBUNTU_24_04.md) / [Debian13歷史baseline](docs/INSTALL_DEBIAN_BASELINE.md)
- [內網必帶清單與GitHub ZIP缺口](docs/OFFLINE_HANDOFF.md)
- [下一個AI接手指引](docs/AI_HANDOFF.md) / [Commit Why/What/Test](docs/COMMIT_GUIDE.md)
- [第一階段細節](rv32_gtest_poc/README_繁體中文.md) / [MicroPython coverage與限制](micropython_gtest_poc/README_繁體中文.md)

## Ubuntu重現流程

```sh
python3 scripts/preflight.py --profile ubuntu-24.04
# 按Ubuntu指南額外查QEMU/現有套件；只補缺項。
# xPack官方archive放專案tools並驗SHA，完全不需root。
python3 scripts/prepare-ubuntu-sources.py .
export RV32_HOST_PROFILE=ubuntu-24.04
export QEMU=/usr/bin/qemu-system-riscv32
(cd rv32_gtest_poc && RV32_QEMU_CLOCK=icount python3 scripts/verify_all.py)
(cd micropython_gtest_poc && RV32_QEMU_CLOCK=raw python3 scripts/verify_all.py)
```

內網APT可用但沒有root：先probe已有build-essential所提供工具、python3、qemu-system-misc。apt --simulate只做診斷；缺系統工具交IT。xPack可放HOME內repo/tools。Ubuntu/QEMU/GCC不用自己重編，專案/GTest/MicroPython測試仍要build。完整命令與限制見[Ubuntu指南](docs/INSTALL_UBUNTU_24_04.md)。

這不是拿Debian13 QEMU deb在Ubuntu上直接執行。Ubuntu profile檢查實際userspace，不注入專案Debian shared-library路徑。缺依賴就停止，內網不得暗中下載。

## 下載整包到內網：務必讀

普通Git包含本專案source/docs/tests，以及GoogleTest/FFF/FreeRTOS/MicroPython固定source tar。Ubuntu工具優先使用工程機已安裝套件；只缺特殊工具時從固定官方URL取得xPack，核對SHA後私有解壓。

**GitHub Source ZIP不含Release assets。** 這不表示輕量路線必須下載四個大包：它们是歷史空白離線VM的可選備援，tool/OS的對應source不參與一般build/run。[Release manifest](manifests/release-assets.json)保留檔名/hash與optional狀態；本次不刪附件、不移tag。[內網指南](docs/OFFLINE_HANDOFF.md)區分「工具已裝」與「完全離線空白環境」。

## 實驗版本與界線

GoogleTest1.15.2、FFF1.1、FreeRTOS11.1.0、MicroPython1.26.1（commit647c8b96cae7e202c7a020395b7cfe65e5b8ce04）、xPack GCC15.2.0-1；demo RV32IMAC+Zicsr/ILP32/virt，並非產品core規格。Ubuntu與Debian QEMU版本分開記錄於lock。

MicroPython是reduced embed profile與11個新增測試，不是完整upstream suite。真機UART已知921600 baud，但framing/driver/log overhead未實測。所有功能測試與真機性能評估分開；heap/stack peak與最小RAM仍未量測。

保留原MIT LICENSE與history。新code亦MIT；第三方license與原始source notices保留。每個commit寫Why/What/Test，不force-push、不碰無關repo。
