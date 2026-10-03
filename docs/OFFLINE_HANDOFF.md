# 內網搬運：不是只按一次Source ZIP

目標Ubuntu24.04 x86-64。普通Git放source/docs/測試與小型上游source archives；大型工具與額外source包放同repo Release（完成發佈後，精確清單/大小/hash會寫入manifests）。不依賴ChatGPT Library或私人下載連結。

## GitHub ZIP的缺口

GitHub的Source code(zip/tar.gz)是指定commit的Git內容快照，不包含Release assets，也不保證包含LFS objects或submodule完整內容。本專案避免以LFS/submodule充當離線依賴。

- 要完整history：clone指定repo/commit。
- 只要內容：下載指定commit的source archive。
- 兩者都要另外下載列為required的Release assets，依SHA256驗證。
- 不要只下載latest/main而不記錄commit；branch/tag可移動，外層ZIP壓縮bytes亦可能改變。

官方說明：[source archives](https://docs.github.com/en/repositories/working-with-files/using-files/downloading-source-code-archives)、[大檔/Release](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github)。

## 必帶資料類型

1. Git source snapshot：本repo兩個PoC、README/FAQ/計畫/Checklist/AI handoff、scripts、測試證據與lock manifests。
2. xPack RISC-V GCC15.2.0-1 Linux x64官方binary archive，SHA在第一階段lock。
3. Ubuntu24.04官方host build/QEMU及依賴.deb bundle（版本與hash由Ubuntu manifest固定）。不是Debian13 QEMU bundle。
4. 可选歷史Debian13 reproduction bundle，與Ubuntu清楚分開。
5. QEMU對應source、xPack recipe/helper與compiler component source包；供內網研究/修改與license對應。準備source不代表已從source重建compiler/QEMU。
6. 所有license notices與SHA256 manifests。

## 內網檢查

- 驗證commit、檔案hash/大小與company supply-chain規範。
- 執行preflight確認Ubuntu24.04/x86-64，安裝公司准許的對應Ubuntu packages。
- 用offline source preparation，不得用缺檔時自動curl或默默skip的替代流程。
- 依CHECKLIST重跑host→RV32bare→FreeRTOS→MicroPython，保留expected case count與negative結果。
- 接公司source前確認ISA/BSP/RTOS與量測邊界，機密不預設回傳外網。

## 交付狀態

目前檔案清單會隨實際上傳/驗證更新。Release asset尚未驗證存在前，不把此交付稱為「已完整可離線下載」。Ubuntu實跑與GitHub遠端新下載重跑的狀態以CHECKLIST與具體logs為準。

## 內網權限分支

沒有假設sudo/網路。已有prerequisites則直接驗收；xPack可私有解壓免root；Ubuntu.deb的system安裝交授權IT；QEMU私有解壓需完整依賴與另外驗證。具體分支見Ubuntu安裝指南。當地權限不足是明確blocker，不以安全設定變更或外網下載繞過。

## Public與公司內網邊界

此GitHub repo目前為public，只收合成範例、公開上游source與本次研究/測試文件。將其帶進公司並接入真實source後，不要沿用此public remote作寫入目的地。

- 若用Source ZIP帶入，建立公司內部repo/remote，不額外添加外網push目的地。
- 若用git clone帶入，先由當地授權操作者檢查`git remote -v`，改成公司內部remote或停用外網push，再加入公司code/log。
- 公司source、真實payload/trace/log、內部地址、credentials、tokens均不可自動推回此public repo。不要把本次上傳公開實驗的授權延伸成公司code發佈許可。
- 此處是操作邊界指引；本次沒有修改GitHub可見性、公司remote、帳號權限或安全設定。

## 已封裝的四個required assets

精確檔名/大小/SHA與放置方法見[release-assets.json](../manifests/release-assets.json)。四個檔案均已在工作環境封裝驗證；download_url為null時代表尚未發布，不可把候選清單當成遠端可下載證據。完成Release後應固定tag、source commit並逐個下載重驗。原生Ubuntu執行不需要下載外層TCG OS image；該image僅為可選驗證環境。
