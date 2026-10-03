# 歷史研究依據

- [可讀Markdown轉錄](research_plan_2026-10-03.md)
- [原始Word報告](RV32_RTOS_效能模擬與最佳化研究規劃.docx)
- [來源/完整性與37個參考URL](source_provenance.json)

正文研究日期為2026-10-03；DOCX保留原bytes，其模板內建2013 core timestamps不是研究日期。本文比較QEMU/cache plugin、gem5、GVSoC、Renode、Spike、replay/calibration與C最佳化，保留原作者當时的限制與工程估計。

這份報告是歷史研究，不是已實作清單。後續明確保留GoogleTest/FFF，直接RV32/FreeRTOS與MicroPython功能測試已有結果；原文薄C/Unity只是當時候選，不是目前採用路線。GoogleTest實驗固定1.15.2/C++14；不要拿原文一般最新版C++17敘述改掉已驗證設定。Cache/ILM/DLM/timing model仍未實作，icount也不是cycle-accurate性能證据。

報告內來源連結與研究日期按原檔保留，未把今天的repo實測回填為當年的研究結果。最新狀態請看主README、PLAN、FAQ與evidence。
