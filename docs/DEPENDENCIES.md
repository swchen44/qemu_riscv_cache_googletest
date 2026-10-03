# 固定依賴、source與離線資產

所有實際bytes以lock的SHA256為準；版本號/URL不是完整驗證的替代品。普通Git已放小型runtime source archives，大型toolchain/OS dependency包是另外的Release assets。尚未上傳並核對前不能標成已提供。

## Runtime/test sources（普通Git有bytes）

| 元件 | 固定版本 | 用途 | 授權 |
|---|---|---|---|
| GoogleTest | 1.15.2 | C++ test framework | BSD-3-Clause |
| FFF | 1.1 | C linkage seam fake | MIT |
| FreeRTOS Kernel | 11.1.0 | 真RV32 RTOS scheduler/queue/notification | MIT |
| MicroPython | 1.26.1 / 647c8b96cae7e202c7a020395b7cfe65e5b8ce04 | 真parser/compiler/VM/objects/GC | MIT及保留的component notices |

官方URL/SHA/bytes見兩專案dependencies.lock.json。source tar本身保留上游license。新專案MIT不覆蓋任何第三方條款。

## Toolchain與Ubuntu Release候選

- xPack RISC-V GCC15.2.0-1官方Linux x64 archive：433494794 bytes；SHA256 `aaaa8060c914851a3e5ee1ba82cc3d6f80972f90638a05c6e823a37557a33758`。內含newlib/libstdc++與23項distro notice。
- xPack component source包：235303198 bytes；SHA256 `f1c5309148669ac3a90ad68f6cbbba7b6b93abdc01b4a9b0a5166b6d85c9a662`。25個現成archive：23個component sources，加固定build recipe/helper；官方URL與逐檔SHA見manifests/toolchain-sources.lock.json。
- Ubuntu24.04 binary addon：165859735 bytes，SHA256 `57ff71f308c6415ea353af9e1dc047399f7e20fcaaada0630feddc80b1c95694`；121個required binary build/runtime packages：官方版號/URL/SHA/license路徑見manifests/ubuntu-packages.lock.json。PRoot probe另列optional，不是安裝需求。
- Ubuntu對應source包：62套source、191檔，source archives/metadata原始bytes總733960033；壓縮Release候選731976764 bytes，SHA256 `40ca91cd27453ab77831eafe13917fb4e075d36b1f1a49c477e8912037e67239`。exact .dsc/orig/debian來源，不是只有下載連結；逐檔lock為manifests/ubuntu-corresponding-sources.lock.json。

此處是準備好的候選檔案資訊，不表示Release已完成發佈；實際asset URL/發佈狀態以最終manifest與README為準。

## Source完整性與重建界線

已取回runtime、QEMU、cross compiler相關source inputs及原build recipe。這不等於已從零bootstrap GCC/QEMU，也不承諾bit-identical compiler。xPack recipe的host bootstrap工具、build-time npm/xpm環境另有要求；已測workflow使用固定官方prebuilt compiler。

Ubuntu package sources與licenses是所提供Ubuntu binaries的對應材料。保留完整第三方條款，任何對外再散布/修改binary前仍應按公司開源審核處理。此文件不是法律意見或合規認證。

## 安裝與選擇

Ubuntu24.04公司target讀INSTALL_UBUNTU_24_04.md。Debian13僅歷史baseline；不要把舊517MB Debian工具包當Ubuntu必備。xPack可私有解壓免root；系統Ubuntu packages由當地授權IT安裝，或在符合限制且另有驗證的私有prefix使用。沒有sudo/網路是正常情境，不用繞過政策。
