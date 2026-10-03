# Ubuntu 24.04 amd64 驗收結果

完成：2026-10-03 14:35:26 UTC。最後矩陣實跑2794秒（46分34秒）。

## 結論

**23/23個預期步驟通過，但必須保留各自時鐘配置。**

| 套件/項目 | 實際配置 | 結果 |
|---|---|---|
| 第一階段host GoogleTest + FFF | Ubuntu GCC/G++13.3，native x86-64 | 5例通過，故意負例exit1 |
| FreeRTOS custom harness | Ubuntu QEMU8.2.2，fixed icount | produced/processed/preemptions=8、queue_timeouts=1；正例exit0、注入錯誤exit1 |
| RV32 bare GoogleTest | Ubuntu QEMU8.2.2，fixed icount | 5例通過，故意負例exit1 |
| FreeRTOS內GoogleTest | Ubuntu QEMU8.2.2，fixed icount | 6例通過，故意負例exit1 |
| MicroPython host | Ubuntu GCC/G++13.3，native x86-64 | 11例通過，故意負例exit1 |
| MicroPython RV32 | Ubuntu QEMU8.2.2，原raw clock | 同份11例通過，故意負例exit1 |

第一階段15步全PASS；MicroPython8步全PASS；另外normal-build的FreeRTOS正負preflight亦通過。六個負例均經逐份log核對：GoogleTest只出現指定NegativeOracleMustFail，custom FreeRTOS負例只因刻意錯誤sum oracle失敗，沒有通知期限失敗混入最終矩陣。

MicroPython實際編入132個host C來源／134個RV32 C來源；它是編譯後的嵌入式解譯器，不是用host Python替代目標端執行。

## 真正執行環境

- Outer host：dot雲端Debian13，Linux6.18.44，x86-64
- Outer VM：官方Ubuntu24.04 minimal cloud image，回報Ubuntu24.04.5 LTS，自己的Ubuntu Linux6.8.0-146-generic kernel
- 使用純x86 QEMU10.0.13 TCG、2vCPU、3072MiB RAM、10GiB file-backed虛擬磁碟
- Guest沒有NIC；所有依賴與source從唯讀ISO輸入；沒有host mount、KVM、ptrace、host安全設定變更或外部登入服務
- Guest工具：GCC/G++13.3、Make4.3、Python3.12.3、Ubuntu QEMU1:8.2.2+ds-0ubuntu1.18
- Target工具：原xPack riscv-none-elf GCC15.2.0-1，rv32imac_zicsr/ilp32
- 公司實體Ubuntu主機尚未實跑；本結果是完整Ubuntu kernel/userspace VM驗收，不是把Debian結果改標Ubuntu

## 配置與限制

第一階段設RV32_QEMU_CLOCK=icount，實際旗標為-icount shift=0,align=off,sleep=on。MicroPython設RV32_QEMU_CLOCK=raw，不加icount。Outer VM本身仍是原預設virtual clock。

所有RTOS數值斷言、100tick通知期限與==1均保留。Inner QEMU仍30秒timeout；僅外層建置harness因TCG較慢調到1800秒。正式矩陣未開RTOS_DIAGNOSTICS。

保留的raw失敗不能寫成通過：原兩tick等待暴露完成同步race；加completion semaphore後，raw期限仍可能早於第一個通知到期。診斷確認return0，沒有觀察到大於1累積通知。固定icount提供清楚、重複的功能重播；不是cycle/cache/真晶片效能模型，也不代表raw default clock已修好。詳細trace與因果限制見CLOCK_DIAGNOSTIC_REPORT_繁體中文.md。

## Source與版本證據

- 最終demo.c：888b43e2e99c501b10d60e415e8c9946b831c9b6cb3d3835ebbe36f6084c0279
- 共用MicroPython test source：09653e3c43399c6fe3fc78432b439abbd62e5ef759ddd6d82a4530990cc9e62c
- 共用第一階段GoogleTest source：5148c94846c6e296561ae5f0f366e61f5608bd70b5be54e33f162a3962889010

實際Ubuntu生成的mpversion.h已另以只讀取既有檔案的guest批次取出：TAG=v1.26.1、HASH=<no hash>、BUILD_DATE=2026-10-03；guest沒有Git。兩份header SHA相同：226210701adff7c9aa566dea3206904df5b8c40efde5ecb949230c450bfa6340。取出前後三個MicroPython binary hash完全不變，沒有prepare/rebuild或修改原header。後續repo prepare會從locked source明確pin tag/commit；原Ubuntuheader與其差異保留，不偽稱本次已用full-hash header重建。

## 離線交付

- 必要Ubuntu binary：121個.deb，164,806,852 bytes；每個有固定version、官方URL、SHA256與copyright
- 配對source：62套、191個官方精確對應檔，733,960,033 bytes；官方Sources SHA256及.dsc檔案完整性均通過
- source附件：ubuntu24.04-corresponding-sources-20261003.tar.gz，SHA256 40ca91cd27453ab77831eafe13917fb4e075d36b1f1a49c477e8912037e67239
- 不散布變更VM disk、整個rootfs、OS image、PRoot探測binary、host Git metadata、cookie或credential
- GitHub「Download ZIP」不會包含Release的大檔；轉入公司內網前需把README列出的Release附件一併下載並核對SHA

標準Ubuntu24.04已具Python3是安裝前提；PRoot被沙箱拒絕的Base路線不是完成的bootstrap驗收。安裝脚本只讀取鎖檔指定套件，使用--no-download、--no-remove、--no-install-recommends及明確archive cache；先--simulate，經目標機管理員審閱後才--install。

## 查核入口

- UBUNTU_VALIDATION_RESULTS.json：機器可讀聚合結果，明示profile、命令、exit、來源hash和限制
- final-ubuntu-validation/evidence/*/results.json：原始兩份結果，未加入事後欄位或改寫
- final-ubuntu-validation/evidence/*/*.log：正負例、建置與版本
- final-ubuntu-validation/evidence/built-artifact-sha256.txt、rv32-elf-headers.txt：實際產物hash/ELF32 RISC-V辨識
- final-ubuntu-validation/micropython-provenance/：原版本header、prepare.py與三個binary hash
- attempts/：最初APT路徑問題、raw RTOS失敗及同ELF clock比較
- logs/helper-preparation-summary.json：交付TCG準備helper已驗鎖檔、patch、ISO內source SHA、qcow2一致性；沒有把這項準備檢查冒充另一輪完整VM測試
