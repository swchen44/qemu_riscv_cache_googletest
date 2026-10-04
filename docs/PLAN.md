# 計畫書：從功能等價，到可信的RV32效能研究

## 目標與目前已知

把現有host C++ GoogleTest + FFF方法移到真正RV32執行環境，同一份C核心先確認功能，再加入真FreeRTOS行為，最後接回公司內網的真實產品。另一個較大型驗證對象是官方MicroPython interpreter。

已知產品背景：單RV32 core、約1MB code/2MB data、三個RTOS應用task、hot path約200ms，目標150ms；L1約64KB、L2约256KB；ILM為instruction、DLM為data，所述1-cycle且繞cache。這些是使用者提供的背景，尚未由本專案量測驗證。確切core、ISA extensions、cache geometry/latency、BSP、DMA、IRQ、clock與量測邊界仍需確認。真板log為921600 baud，serial framing與driver策略待確認。

目標內網主機：Ubuntu24.04 x86-64。歷史baseline：Debian13 x86-64；兩者不可混稱。

## 階段與驗收門檻

### A. 功能基線（已完成歷史實跑）

- 合成純C product module，不含公司code。
- Host GTest/FFF五個正例、故意錯誤oracle必須失敗。
- 同一test source交叉編譯成ELF32 RV32，裸機QEMU五個正例。
- 真FreeRTOS三應用task，加入queue timeout/wakeup、delay、priority preemption、notifications；GTest六個正例。
- 更大專案MicroPython固定release/commit，真parser/compiler/VM/objects/MPZ/GC/exception十一個同組host/target測試。

驗收以exit code、預期case count、negative oracle、source hash為準，不以「有畫面」或「link成功」為準。

### B. 可搬運/可重現交付（目前進行）

- 實際Ubuntu24.04 userspace驗證與官方對應依賴。
- README/FAQ/Checklist/安裝/踩坑/下一個AI指引齊備。
- source與工具固定版本/官方URL/SHA256/license。
- 區分普通Git source、Release assets及內網必帶清單；下載source ZIP本身不含Release assets。
- 從GitHub實際commit重新下載至新目錄，再跑完整matrix。

### C. 接回公司code（尚未做，需要內網操作）

1. 保留公司原host executable baseline與fixtures。
2. 固定實際compiler、ISA/ABI、產品RTOS版本、BSP與linker map。
3. 列出純邏輯、硬體mock seam、host-only/POSIX依賴與真OS行為。
4. 將相同source/tests接到本例target，逐個處理未解symbols、startup/static constructors、heap/stack與stdio。
5. 再換成公司BSP和實際板子。每次變更都保留相同輸入/輸出oracle、故意失敗與回歸紀錄。
6. 公司code、trace與內部log不預設回傳外網。

### D. 真機profile與可校準成本模型（尚未做）

- 定義200ms的開始/結束點、輸入、build、task priorities與中斷/負載條件。
- 關閉或獨立計入921600-baud log成本。若8N1，每byte至少10bits；1KiB線上時間理想值約11.1ms，尚不含formatting/driver/blocking，不能直接當產品實測值。
- 真機量cycle counter、task執行/等待、cache miss（若有PMU）、memory access、stack/heap peak；取得ILM/DLM與cache配置資料。
- 功能QEMU可幫忙replay、ISA/BSP/RTOS驗證及trace，但stock TCG不提供此core的cycle-accurate cache/ILM/DLM延遲。
- 若要QEMU custom timing model，先寫model contract與校準測試；或評估適合core的timing simulator/RTL，不能從任意模擬器預設推論產品150ms。

### E. 優化與回歸（尚未做）

以真機證據選hot functions/data；評估algorithm、allocation、task blocking、ILM/DLM placement与cache locality。每個patch同時報功能tests、code/RAM變化、真機latency分佈與測量條件。單次最佳值不等於穩定達標。

## 不在本次承諾範圍

完整MicroPython上游suite、任意host OS、公司產品已移植、任意多task共享GTest/FFF、新libc thread-safety、真core cache模型、150ms保證、bit-identical binary。

## 2026-10-03移植後發現的測試同步/clock問題

Ubuntu巢狀TCG揭露了兩個先前Debian快速執行未暴露的條件：先是supervisor用固定sleep當producer完成證據（改為真semaphore barrier）；再是raw virtual clock的100tick期限早於producer開始。診斷固定記錄return/tick/task event，保留原嚴格assert，不以加sleep或拉長deadline混成成功。功能性replay另評估明示icount配置；raw-clock觀察與真機性能研究分開保存。正式完整matrix與最終flags以新log/commit為準。

## 2026-10-04：改為無root工程機的輕量交付

內網APT可用但無root，多數host工具已安裝。優先probe並重用工具，缺項交IT，固定xPack私有解壓；不要求重編Ubuntu/QEMU/GCC。四個大型Release包先保留為optional legacy，不刪除、不移tag。對應source與完整.deb閉包不再列為每位使用者必帶項。公司機實跑與private-QEMU仍待實際驗收；之前VM/遠端clone結果保持其原scope。
