# 給下一個AI/工程師的接手指令

目的：在公司內網重現此repo的GoogleTest/FFF/RV32/FreeRTOS/MicroPython實驗，再由使用者明確授權接入真實產品。不要猜測或替換使用者的測試框架。

## 第一輪先做

1. 讀README、AGENTS.md、PLAN、CHECKLIST、OFFLINE_HANDOFF與FAQ。列出verified/failed/not-run，不能把舊Debian結果稱Ubuntu通過。
2. 記錄實際commit與工作樹變更；檢查Ubuntu24.04 x86-64與依賴bytes/hashes。
3. 若在內網，使用offline模式，不連外；缺檔列出精確檔名/version/hash給操作者補齊。
4. 跑host→RV32bare→FreeRTOS→MicroPython的既定commands，保留exit/count/negative結果。
5. 真正成功後才接公司source，不要求把公司code送回此公開repo。

## 現有證據的範圍

純C合成模組5case；真FreeRTOS加1個OS behavior case；MicroPython11case，真parser/VM/objects/GC並非mock。FFF只作用在產品driver seam或MicroPython stdout seam。MicroPython是reduced embed profile，不是完整上游suite或OS port。

入口與memory細節在FAQ。GoogleTest沒有pthread/death tests/RTTI/C++ exceptions/filesystem功能；Python exceptions仍以C NLR處理。GC/setjmp/current single-task use有明確限制。不要讓C longjmp跨過C++ RAII frames。

## 每次改動的輸出

- Why：要解決的觀察/失敗/需求。
- What：source/config/build/test/document實際改了什麼。
- Test：命令、版本、ISA/ABI、case count、exit、negative oracle；未跑原因。
- 追加踩坑/FAQ穩定編號，更新plan/checklist，不覆蓋舊證據卻不留說明。

## 性能與安全界線

stock QEMU不是產品cache/ILM/DLM timing simulator；GoogleTest ms不是產品latency。已知真板log921600 baud，但framing/driver待確認。不要承諾150ms或從本例大heap配置推論最小RAM。

不要讀取/公開credentials、私人筆記、無關repo或不屬本實驗的材料；不要force-push或改repo visibility。後續GitHub發佈需使用者授權。此repo使用者授權不自動涵蓋其他repository。

## 進入公司環境後先切斷public push路徑

本repo是public研究實驗；加入公司真實code前，先檢查remote並改成授權內部remote或停用外網push。Source ZIP本身沒有Git history/remote，可在內網建立新repo。不能因本次user允許push公開實驗，就自動把公司source、真實log/trace或credentials送回這裡。除非新的明確授權與公司政策允許，預設所有公司資料留內網。
