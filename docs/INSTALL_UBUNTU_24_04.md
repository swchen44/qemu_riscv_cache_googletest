# Ubuntu24.04 x86-64：先用既有工具，無root安裝路線

2026-10-04更新：目標工程機有內網APT，但使用者沒有root；多數編譯工具已安裝。預設先probe已有工具。Ubuntu/QEMU/GCC本身使用prebuilt，不必自行重編；本專案、GoogleTest與MicroPython的host/RV32測試映像仍須build。

## 1. 只讀檢查

在repo root執行；不安裝或變更系統：

```sh
python3 scripts/preflight.py --profile ubuntu-24.04
command -v gcc g++ make python3 qemu-system-riscv32 bash tar gzip timeout sha256sum
gcc --version
g++ --version
make --version
python3 --version
qemu-system-riscv32 --version
dpkg-query -W build-essential gcc g++ make python3 qemu-system-misc
apt-cache policy build-essential python3 qemu-system-misc
```

`preflight.py`目前只查host基本工具，不含QEMU或cross compiler，所以後續檢查仍必要。command/dpkg-query遇到缺項可能exit1，記錄missing即可；已具gcc/g++/headers/make時，不需要只為了build-essential這個meta-package重裝。版本報告留在內網，勿直接把公司環境資訊推回public repo。

## 2. 直接套件需求與APT診斷

| 頂層套件 | 實際用途 |
|---|---|
| build-essential | 便利meta-package，帶gcc/g++、make、C headers與相關建置工具 |
| python3 | build/test harness及MicroPython產生器；Ubuntu24.04預設Python3.12符合脚本 |
| qemu-system-misc | 本次Ubuntu8.2.2實測套件包含qemu-system-riscv32，供裸機/FreeRTOS system模擬 |

正常Ubuntu已有bash、tar、gzip與coreutils（timeout/sha256sum）。git只在clone時需要；curl/ca-certificates只在使用HTTPS下載時需要，可以由公司其他合規搬運方式替代。CMake、Node/npm/xpm、Docker、PRoot及外層Ubuntu VM image不是本PoC原生Ubuntu路線的必備。

先前121個.deb是空白離線VM的完整build/runtime依賴閉包，含許多傳遞libraries；不是要工程機逐一手動安裝121包。

無root可先做simulation，列出缺項交IT：

```sh
apt-get --simulate --no-install-recommends install build-essential python3 qemu-system-misc
```

這不會安裝，也不授予安裝權限。若APT索引缺少/過舊，交公司管理員處理；本指南不執行sudo、不改APT sources、不自動update或降級系統libraries。已有工具直接用；不同安全更新版本須記錄並重跑既定測試，不能把舊版本pass直接套用。

## 3. 特殊工具：xPack私有解壓

已測工具鏈是xPack RISC-V GCC15.2.0-1（Linux x64），包含裸機newlib/libstdc++與RV32 multilib。Host gcc或gcc-riscv64-linux-gnu不能直接等同這個RV32裸機C++配置。公司既有裸機工具鏈可另評估，但須確認ISA/ABI、C++ runtime及multilib後重驗。

官方[安裝指南](https://xpack-dev-tools.github.io/riscv-none-elf-gcc-xpack/docs/install/)支援手動下載解壓。固定官方archive：

https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v15.2.0-1/xpack-riscv-none-elf-gcc-15.2.0-1-linux-x64.tar.gz

大小433494794 bytes；SHA256 `aaaa8060c914851a3e5ee1ba82cc3d6f80972f90638a05c6e823a37557a33758`。官方URL與SHA亦在rv32_gtest_poc/dependencies.lock.json。

把repo放在自己有寫權限的HOME目錄。以下下載只在公司允許連到官方來源的機器執行；內網APT可用不代表GitHub可用。若GitHub不可達，於允許的預備機下載核對後搬入相同位置。

```sh
mkdir -p rv32_gtest_poc/tools
curl --fail --location --output rv32_gtest_poc/tools/xpack-riscv-none-elf-gcc-15.2.0-1-linux-x64.tar.gz https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v15.2.0-1/xpack-riscv-none-elf-gcc-15.2.0-1-linux-x64.tar.gz
printf '%s  %s\n' aaaa8060c914851a3e5ee1ba82cc3d6f80972f90638a05c6e823a37557a33758 rv32_gtest_poc/tools/xpack-riscv-none-elf-gcc-15.2.0-1-linux-x64.tar.gz | sha256sum -c -
python3 scripts/prepare-ubuntu-sources.py .
rv32_gtest_poc/tools/xpack-riscv-none-elf-gcc-15.2.0-1/bin/riscv-none-elf-g++ --version
```

prepare只驗鎖定SHA並解壓到專案vendor/tools，沒有網路或系統安裝，不需要root。現有MicroPython build及共用env固定上述project tools路徑；把archive解到任意HOME共用prefix後只改PATH，**目前不保證可用**。將整個repo放HOME即可使用現有已測配置，毋須改永久PATH或/etc。

## 4. 執行相同測試

確認已有QEMU與host工具後：

```sh
export RV32_HOST_PROFILE=ubuntu-24.04
export QEMU=/usr/bin/qemu-system-riscv32
(cd rv32_gtest_poc && RV32_QEMU_CLOCK=icount python3 scripts/verify_all.py)
(cd micropython_gtest_poc && RV32_QEMU_CLOCK=raw python3 scripts/verify_all.py)
```

Ubuntu profile查/etc/os-release，不注入Debian shared-library路徑。第一階段15步、MicroPython8步，正例5/6/11 tests與六個刻意exit1負例都需符合預期；0 tests/exit0不是成功。原Ubuntu完整VM已按15/icount＋8/raw通過，但**公司這台無root工程機尚未實測**。icount不是cycle/cache模型；真機150ms目標未驗證。

## 5. 如果缺QEMU而不能系統安裝

優先將qemu-system-misc缺項與版本資訊交IT。`apt download`可以取.deb，`dpkg-deb -x`可以私有解壓；但它們不保證shared libraries、loader、data/modules路徑或maintainer scripts已處理完。此rootless private-QEMU路線尚未完成本專案驗收，不能承諾搬單一binary即可執行。若IT無法提供，需要另作有界依賴/ABI實驗並重跑矩陣，不變更host安全設定。

## 6. 歷史離線bundle屬可選備援

[v0.1.0-poc-20261003](https://github.com/swchen44/qemu_riscv_cache_googletest/releases/tag/v0.1.0-poc-20261003)四個大型附件保留為optional legacy；此次文件更新沒有刪除或移tag。一般工程機不需要下載Ubuntu/toolchain的對應source archives，更不需要從它們重編工具。Ubuntu binary closure也不是已裝工具的工程機必帶項。xPack binary可從官方來源取得，無需使用此repo鏡像。

若刻意重現空白離線VM的舊bundle方案，才使用scripts/ubuntu/*的明確`--bundle-root`契約；system installer的`--install`仍需要當地管理員。現有無root使用者不要執行該安裝分支。保留第三方license與source鎖檔作歷史研究；一般執行需求與再散布義務是不同問題。
