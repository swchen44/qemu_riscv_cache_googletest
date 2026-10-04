# 內網搬運：已有工具的輕量路線

目標Ubuntu24.04 x86-64。內網可用APT但使用者沒有root，工程機多數工具已裝。普通Git保留source/docs/tests與小型上游source tar；大型Release鏡像不是一般重現必帶項。

## 要带什麼

1. 指定commit的Git source snapshot與lock/evidence/docs；需完整history才clone，否則Source ZIP可用。
2. Probe工程機已有工具，詳見[Ubuntu指南](INSTALL_UBUNTU_24_04.md)。缺系統套件先APT simulation、交IT，不自行sudo。
3. 若未有相容cross compiler，從固定官方來源取xPack15.2.0-1 archive、驗SHA，放自己的repo/tools；內網GitHub不可達則由允許的預備機搬入。
4. 保留第三方license與hash。不要下載/重編Ubuntu與compiler source只為執行PoC。

## GitHub ZIP與歷史bundle

Source ZIP不包含Release assets；本專案無LFS/submodule隱藏依賴。不要只記main而不記commit。

四個大型附件在[v0.1.0-poc-20261003](https://github.com/swchen44/qemu_riscv_cache_googletest/releases/tag/v0.1.0-poc-20261003)保留為optional legacy。Ubuntu binary閉包適合刻意還原空白離線VM；對應source包供工具研究/再散布來源材料，沒有被一般build脚本使用。xPack binary鏡像可由官方URL取代。它们本次沒有被刪除，tag亦不移動；是否精簡附件是另一項待確認操作。

[Release manifest](../manifests/release-assets.json)保留舊資產資訊及optional分類。原tag的歷史README可能仍寫required/pending，以此後續無root指南解釋現行使用路線，不改寫歷史snapshot。

## 內網驗收界線

- 用offline source preparation驗鎖定bytes；缺檔即停止，不暗中連外或skip。
- 依CHECKLIST跑15/icount＋MicroPython8/raw，檢查case count及負例exit1。
- 原Ubuntu VM與Debian遠端clone成功不等於公司這台工程機已通過；工具版本不同需記錄並重驗。
- QEMU私有解壓的shared-library/loader/data整合尚未驗證；不能因apt可下載就承諾無root一定能跑。

## Public與公司內網邊界

此GitHub repo目前為public，只收合成範例、公開上游source與本次研究/測試文件。將其帶進公司並接入真實source後，不要沿用此public remote作寫入目的地。

- 若用Source ZIP帶入，建立公司內部repo/remote，不額外添加外網push目的地。
- 若用git clone帶入，先由當地授權操作者檢查`git remote -v`，改成公司內部remote或停用外網push，再加入公司code/log。
- 公司source、真實payload/trace/log、內部地址、credentials、tokens均不可自動推回此public repo。不要把本次上傳公開實驗的授權延伸成公司code發佈許可。
- 此處是操作邊界指引；本次沒有修改GitHub可見性、公司remote、帳號權限或安全設定。
