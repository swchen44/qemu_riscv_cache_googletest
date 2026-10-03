# 歷史研究報告可讀轉錄

> 研究日期：正文2026-10-03。這是當時的研究/方案評估，不是目前實作狀態。原始DOCX保持bytes不變；本轉錄保留段落、4張表與37個參考來源，不重寫原研究結論。最新狀態以本repo主README、PLAN、Checklist與實測log為準。

重要後續差異：原報告提出薄C/Unity作為候選，後續使用者要求及本repo實作保留GoogleTest+FFF，已直接在RV32裸機/FreeRTOS通過。原文的一般最新版GoogleTest C++17敘述不替代本repo固定1.15.2/C++14實測設定。原文cache/timing方案仍未在本repo實作；不把研究建議當完成項。

## RV32 RTOS 效能模擬與最佳化研究規劃

從 200 ms 到 150 ms 的可行方案比較　｜　研究日期 2026 年 10 月 3 日

## 一 結論與建議決策

這個方向可行，但應拆成兩個問題：重播真實工作負載，以及估計改動後的硬體效能。 建議先保留 PC GoogleTest，抽出共用 C 測例與輸入，建立輕量 RV32 target harness；用 QEMU 執行目標程式、統計實際執行路徑與 cache 行為，再用少量真機量測校準。當候選改動明顯依賴 pipeline、bus contention 或時序回饋時，再升級 gem5 或 GVSoC。

第一個值得動手的交付物是「同一組輸入能在 host、RV32 模擬器與單板真機執行的 hot-path benchmark」，而不是先把九台裝置、整個 SoC 與 GoogleTest runtime 全部搬進模擬器。九設備情境仍是最後端到端驗收，日常最佳化不必每次依賴它。

200 → 150 ms 是延遲減少 25%，等價 speedup 為 1.333 倍。必須先確認 200 ms 是運算時間、task response time，還是含外部等待的端到端時間。

目前只有 RV32 與記憶體容量，不足以保證模擬誤差；「只看相對改善」也不能免除校準。可先取得可比較、可解釋的 evidence，再逐步提高準確度。

這份文件是調查與規劃，尚未移植測試、修改韌體、建立模擬平台或執行實機效能實驗。下列工期與驗收門檻均為工程估計。

| 方案 | 判定 | 主要價值與限制 |
| --- | --- | --- |
| 共享 C workload＋真機單板 benchmark | 最優先 可行 | 直接測 RV32 真實指令與 memory；需可隔離輸入，但不用每次九台連線 |
| QEMU＋客製 cache 與成本分析 | 第一階段 可行 | 快速執行／定位／參數掃描；cycles 是外加模型估計，不是 QEMU 原生量測 |
| gem5 timing model | 條件式可行 | cache／bus／pipeline 結合較完整；平台移植與校準成本較高 |
| GVSoC event driven model | 條件式可行 | 可調 core 與 interconnect timing；需確認核心、cache 和周邊模型適配 |
| Renode＋scenario peers | 功能整合可行 | RTOS／周邊／多節點測試有利；預設時間模型與 L1 分析不滿足完整 L1/L2 timing |
| QEMU icount 直接換算真機 ms | 不適合作效能結論 | 指令數不等於 cycles；固定 ns/instruction 只是虛擬時間設定 |
| 舊文字 log 直接變完整 replay | 資訊不足時不可行 | 可先還原部分 behavior；缺 inputs、初始狀態、事件順序就無法唯一重建 |

## 二 已知條件與要保留的假設

已知：純 C 產品程式、RV32、單核心 RTOS，約 1000 kB code 與 2000 KB data；系統提及三個 task，本規劃暫將 hot path 視為其中一個 task 或其 handler，另兩個 task 可能造成搶占與 cache 干擾。ILM 放 instruction、DLM 放 data，依你的要求都視為 1-cycle memory，完全 bypass cache；另有經 cache 存取的 SRAM。L1 64 KB、L2 256 KB。

未知資料不阻擋方案調查；先做參數化假設，實作前再從 datasheet、linker map 或短 microbenchmark 補齊。尤其不要把「RV32」當作某一種 pipeline，也不要把「ILM/DLM 一 cycle」推成所有指令一 cycle。

核心：RV32I/M/C/F 等 ISA extension、ABI、時脈、issue width、load-use、branch、mul/div、CSR 與 trap 成本。

cache：64 KB 是 L1I/L1D 各自容量、兩者總和，還是 unified L1？line size、associativity、replacement、write-through/write-back、write-allocate、prefetch、MSHR、L2 inclusion 等都會改變結果。

記憶體與 bus：ILM/DLM 容量、位址、port 與 placement；SRAM hit/service latency、bus width、burst、bandwidth、arbitration、DMA 是否共用及 coherence。容量中的 KB/kB 也應在設定時明確換成 bytes。

軟體：compiler 版本、-march/-mabi、optimization/LTO、production linker script、RTOS task priority/tick、IRQ、HAL 與周邊暫存器語意。

重要區別：1-cycle 是記憶體服務假設，CPU-visible latency 還可能被路由或模型預設延遲拉長。gem5 的 SimpleMemory 有 latency 與 bandwidth；若放在預設 SystemXBar 後方，還會加上 interconnect 延遲。ILM/DLM 應用獨立路徑明確旁路，並用 fetch/load microbenchmark 檢查，而非只填一個 latency=1。[8][9]

## 三 模擬器能做什麼

### QEMU 適合執行與觀察 需要補上成本模型

QEMU 的官方 cache plugin 可配置 split L1I/L1D 與可選 unified L2，支援容量、line size、associativity，以及 LRU/FIFO/random replacement；可列出造成 miss 的指令。因此不是完全沒有 cache 分析，但它提供的是模型統計。[1]

目前 cache.c 原始碼以 tag/valid 為主，略過 MMIO，將 L1 miss 送到 L2；讀寫沒有完整 dirty/writeback 行為，沒有 bus queue、latency 或 ILM/DLM region bypass 選項，也沒有依 access size 主動展開所有跨 cache-line 存取。正式使用要補齊必要行為並加單元測試，不能照原樣視為本 SoC。[2]

instruction-execution callback 應用於動態指令統計，不能計算翻譯 TB 時看見的指令數來代替執行次數。instruction callback 在執行前觸發；遇 exception 時不一定等同 retired instruction。data memory callback 與 instruction fetch 也要分開；需要記錄 PC／instruction size 與 load/store address／size／type，system mode 再核對 physical address。plugins 為觀察工具，不能僅靠 cache plugin 把 miss latency 回饋進 guest 執行時序。[3]

QEMU icount 官方明言不模擬每條指令在實際硬體需要幾 cycle。host wall time、guest virtual time、instruction count 和硬體 cycles 是四種不同數字。QEMU 適合先跑 extracted RV32 workload；完整 RTOS binary 仍可能需要 BSP/MMIO/interrupt/timer 移植。[4]

### gem5 適合需要時序回饋的第二階段

gem5 有 RV32 ISA 設定與 RiscvBareMetal workload，但預設設定與現成平台不一定吻合你的 SoC。SE mode 可研究抽出的應用計算，卻不能代表原 RTOS、MMIO、中斷或 scheduler；要保留那些行為，應選 bare-metal/full-system 平台路線。[5][36]

TimingSimpleCPU 可作明確宣告的簡化序列模型；MinorCPU 適合需要 in-order pipeline 與相依性建模的情況。Classic caches 具 MSHR、write buffer 等機制。這些能力讓模型更完整，但非自動代表未知核心；若實機是小型 blocking core，套用高併發預設會過度樂觀。[6][7][35]

### GVSoC 是值得試配的替代方案

GVSoC 官方模型文件示範 RV32 ISA、instruction latency、dependency/resource stall；tutorial 也示範為 interconnect mapping 加延遲與 bandwidth。適合想建立簡化但有 timing feedback 的客製 MCU 模型，特別是目標與已有 PULP 類核心相近時。[10][11]

它不是任意商用 RV32 SoC 的即用替身。需要實際核對 core extension、cache topology、MMIO/IRQ 與 RTOS port；PULP 文件中的 L1/L2 有時指 scratchpad 層級，不能與本案 L1/L2 cache 名稱直接對上。若沒有接近的核心模型，試配成本可能高於 QEMU 的局部分析。

### Renode Spike 與 host 工具的定位

Renode 支援 RISC-V guest profiling、instruction／exception／memory/peripheral 統計，利於 RTOS 與周邊整合。其文件中的 guest cache analyzer 是 post-mortem L1I/L1D 分析；virtual time 以設定的 PerformanceInMips 驅動，因此原生這組工具不能直接當本案的 L1/L2 timing model。[12][13][14]

Spike 是 RV32 功能模型，已有 I/D/L2 cache 模擬入口與統計，原始碼亦處理 dirty writeback；但沒有本案所需的完整 latency/bus timing。可作 ISA correctness 或輕量交叉檢查，優先度低於已有成熟執行環境的選项。[15]

DynamoRIO/drmemtrace 或 host Cachegrind 一類工具可以研究演算法與 locality。可是 x86/host binary 的 instruction stream、pointer width、layout 與 target RV32 不同；不能將 host cache 結果直接當 target 結論。若採外部 trace analyzer，優先輸入 RV32 目標執行所得的 trace，並核對格式、physical address、access size 與 write policy；工具支援 RISC-V 也不代表可直接執行 bare-metal RTOS。[16]

## 四 建立可解釋的記憶體與成本模型

### 先分位址區域 再計算 cache

instruction fetch 落在 ILM：走理想 1-cycle 路徑，不查詢也不填入 L1/L2。data access 落在 DLM 同理。其他允許的交叉存取依實際 memory map 定義。

cache-backed SRAM：依 L1I/L1D → L2 → SRAM 的路徑處理 miss、refill、eviction/writeback；MMIO 單獨記錄，不與一般 RAM cache 混算。

ILM/DLM placement 不只是省當次 miss，也會減少其他資料的 cache pollution。若 code/data 搬移，應重編並重跑，不能只從舊報告扣幾個 miss。

address 與 linker map 必須以 runtime address 為準，區分載入位址與執行位址；初始化搬移成本若不是 hot path，也應另外列出。

### 第一版允許簡化 但不能重複計價

建議把估計 cycles 分成 CPU base cost、未被重疊掉的 memory stall、branch/dependency 額外成本、以及 bus/ISR/RTOS 影響。若 base cost 已含 1-cycle load，就只加額外等待。L1 miss 且 L2 miss 的一路存取，應累計各層增量延遲，不能同時加兩個都含完整 SRAM round trip 的 miss penalty。

第一版可用 weighted instruction count 加增量 miss cost 來排序，但它只是 screening approximation，既不保證上界也不保證下界。prefetch、store buffer、memory-level parallelism、blocking/nonblocking cache、pipeline overlap 與 bus contention 都可能打破簡單加總。若這些正是候選改動的主要效果，就進 timing simulator 或直接真機量測。

bus latency 也要拆成固定延遲與傳輸成本；refill 通常搬一條 cache line，不能把一次 4-byte load 的成本套到整條 line。queueing、dirty eviction、DMA traffic 必須依需求逐項加入，並記清楚是否已包含在較高層 penalty 裡。

### 參數不確定時做敏感度分析

用已知設定＋合理上下界作小範圍 sweep：line size、ways、L2/SRAM latency、bus bandwidth、branch cost、cold/warm cache。若新方案在範圍內都改善，再排入真機驗證；若排序翻轉，先查明最敏感參數。不要把任意調出的單一 baseline 200 ms 當校準完成，很多互相矛盾的參數組都能湊出這個數字。

同一 binary、同一控制流與存取序列可重用 trace 做 cache 參數探索。變更演算法、compiler flags、memory layout、輸入或搶占行為後要重跑。尤其 timing 改變會改變 task interleaving／timeout／peer 反應時，固定 trace 的 post-processing 不能預測新的 closed-loop execution。

## 五 GoogleTest 到 RV32 的務實路線

保留既有 host GoogleTest，把可重用部分抽成 C fixture／scenario data；target 端用薄的 C runner 或 Unity。 產品是純 C 與 host 測試是 C++ 並不矛盾，也不要求產品改成 C++。GoogleTest 官方目前以至少 C++17 為基礎；裸機可移植性取決於 C++ runtime、libc、平台適配及測試用到的功能，不能視為「一定不行」，但不值得先把大量 STL／filesystem／thread fixture 全搬過去。[17]

Unity 的核心為一個 C 檔與兩個 header，設計面向 embedded C，可自訂輸出。少量 HAL/time/transport seam 先手寫 fake 最省事；CMock 可從 C header 產生 mock，Ruby 是 host 產生器的依賴，不是 target 必須跑 Ruby。測試框架不應成為整個計畫的主成本。[18][19]

共享層：canonical input bytes、設定／state 初始化、被測 C entry point、結果語意檢查；host 與 target 分別接 assertion 與 runner。不要跨平台直接序列化含 pointer、padding 或 size\_t 的原始 struct。

target image：最小 startup/linker/trap、確定的 ILM/DLM/SRAM 配置、ROI markers、結果緩衝與測後輸出。第二步才將完整 RTOS task/IRQ 加回。

量測區間內不做 printf、UART、semihosting 或測試失敗格式化。即使不在 ROI，harness 仍會改變 code layout 與初始 cache 狀態；盡量保留 production placement，並定義一致 warm-up。

host tests 保留大量快速功能回歸；target tests 聚焦 RV32 ABI、32-bit 寬度、alignment、codegen、target libc/libgcc、memory map 與真實性能。

## 六 從文字 log 到可重播情境

文字 log 可以是起點，不能保證含有重建 execution 所需的資訊。先判斷它是否記到完整輸入、初始狀態與事件次序；只有「進入函式／完成步驟」的字串通常只能用來建立覆蓋圖與定位 stage。缺失 payload 或不同 task 的順序，不能靠推測補成可信 testcase。

| 重播目的 | 需要什麼 | 能驗證什麼 |
| --- | --- | --- |
| 行為 regression | 初始 state＋有序 inputs＋output oracle | 某條流程的功能／錯誤處理 |
| 同一 binary deterministic replay | 所有 relevant nondeterminism，必要時含搶占位置 | 重現既有 execution／race |
| 最佳化版本效能比較 | 相同外部情境、可控 clock、允許新 execution 自然改變 | 候選效能與新時序風險 |

QEMU record/replay 記錄非決定性事件，以事件間 instruction count 決定注入點；不是把任意真機文字 log 直接載入。改 binary 後指令位置變了，也不應期待原 replay 仍有同樣語意。工業 RTOS replay 論文同樣強調 IPC／I/O／task state 與 preemption 資訊完整性。[20][21]

### 建議最小紀錄格式

schema version、scenario ID、firmware hash、compiler/RTOS config、裝置角色；可重建的設定、persistent state、queue 初始內容與 PRNG seed。

事件 kind、來源／目的地、sequence/correlation ID、完整 payload bytes 與 length；monotonic/logical timestamp、clock domain／單位及相同時間的 tie-break order。

相關 HAL 返回值、DMA 完成／資料、error、timeout、random/time 查詢；若做 concurrency replay，再記 task switch、wake/block、IRQ、priority 與必要 preemption point。

expected output、state invariant、允許差異與 trace dropped/overflow 計數。資料丟失應明確失敗；不要只把舊版輸出當唯一 oracle，否則舊 bug 也會被固定。

### 九台設備先縮成一台 DUT 與外部事件

先將其他八台的輸入放在 DUT 邊界重播。如果三個 task 透過 queue 串接，可先切出 dequeue → handler → emitted messages 的可決定性 step，再逐步補 RTOS。這能開始優化核心工作，但不能宣稱已驗證真實搶占或整個九台系統。

若 peer 下一個 response 取決於 DUT 當下 request 或時間，就需要 scripted/stateful peer，以 correlation ID、protocol state 與 logical time 回應。照固定時間播放舊封包只能驗證那條 recorded trajectory，可能在最佳化後失真。加入 late/duplicate/out-of-order input、queue full、timeout 邊界與重啟等錄製時沒碰到的情境。

## 七 如何建立相對效能可信度

### 先拆解 200 ms

把端到端延遲拆為 CPU service、被其他 task/ISR 搶占、lock/queue wait、周邊／peer wait。對單一請求，CPU 最佳化能省下的時間不能超過可改善部分。例：若只有 80 ms 可改，其他 120 ms 固定，要到 150 ms 就必須把這 80 ms 壓到 30 ms，局部 speedup 要 2.67 倍；若固定等待已超過 150 ms，單靠 C 運算最佳化不可能達標。

真機 cycle/time/instret 應分開看。函式入口與出口的 cycle 差通常包含期間 ISR 與其他 task，不能直接稱 task active cycles。RV32 讀 64-bit counter 要用 high-low-high 重讀避免 rollover；counter 是否實作／可存取、睡眠與時脈行為要核對。標準 counters 不等於各廠 cache miss PMU 都標準化。[22]

### 校準與驗證使用不同資料

microbenchmarks：dependent/independent ALU、branch、load-use、call/return、mul/div、CSR/trap、IRQ/RTOS switch，以及 ILM/DLM、L1/L2 hit/miss、stride/conflict、streaming refill／writeback。保留結果 checksum，避免 compiler 刪掉工作。

校準一組參數後，用未參與 fitting 的真實 kernel／scenario 作 holdout。至少含 compute-bound、cache-resident、超出 L1/L2、ILM/DLM placement 與 MMIO-heavy 類型。

A/B 同輸入、同 clock、同 memory/cache 初始政策、同 compiler policy；交錯 baseline/candidate，排除溫度、頻率或量測漂移。報每個 case，不只平均。

報 T\_new/T\_old、cycles/instret、miss/MPKI、bus bytes、region usage、stack、功能結果；硬體有波動時再報足夠樣本的 median/tail 與 deadline misses。

本案可採一個實用 gate：先做 4–6 個具代表性的已知改動，比較模擬與真機改善比例；若方向一致且 ratio error 明顯小於預期 25% 降幅，再把模型用於篩選。譬如把改善比例誤差控制到約 5 個百分點可作專案目標，但這不是通用達標保證；若分辨不了 5% 小收益，就不該據此宣告小幅最佳化有效。

### 文獻支持校準必要性 不提供本案準確率

GVSoC 論文作者摘要在其研究平台上報告相對 cycle-accurate simulation 約 2500 倍速度與通常低於 10% 的效能誤差，支持中間抽象層可行；這是作者報告而非本次重現，數字不能移植到未建模的商用核心。[23]

2025 年 RISC-V full-system component-level calibration 研究，作者摘要在兩種 silicon 與 SPEC CPU2017 情境報 mean error 約 19–23%。較詳細的 simulator 與調參並不自動消除誤差；本次未重現其實驗。[24]

核讀 CARRV 2021 論文全文結果與結論，gem5 對 RSD 的部分 benchmark cycles 差異可達 36%，整體平均約 18%；作者討論 branch prediction 與 memory system 抽象差異。這支持以 microbenchmarks、真實 kernel 與 holdout 驗證，而不是只相信 ISA 一致。[26]

早期 ISCA 2001 的 Desikan 論文適合了解 microbenchmark calibration，但其後有作者 errata 修正一個跨研究比較案例；本報告不使用該案例證明候選排名會反轉。相對收益是否可信仍須由本案敏感度與真機 A/B 證據判定。[25][37]

## 八 C 最佳化心法與容易反效果的做法

心法是少做工作、少搬資料、縮小當下工作集、減少序列相依與昂貴同步，再讓 compiler 看懂。 「少 loop、禁 pointer、禁 recursion、拆 function」不是可靠規則。應先找占時間最多、真的可能省 50 ms 的機制；若熱 code/data 已在 ILM/DLM，cache 改動的直接收益通常會減少。

### Loop 看總工作與 reuse distance

優先做演算法與資料流改良：減少重複搜尋、incremental update、只處理 dirty entries、移除多餘 pass/copy/format/log。兩個線性 pass 可能比一個二次方 loop 快；不要數 for 關鍵字。

Fusion 可讓兩個 pass 共用剛算出的值，省再讀／中間 buffer；但 loop body、register pressure 與 spill 可能增加。Fission 可縮小熱指令／live values，代價是多次讀寫。兩者都要檢查跨 iteration 相依與對外可觀測順序。[27]

C 連續二維 array 通常讓最內層沿相鄰 column 走；有重用的大 working set 可試 tiling。tile bytes 必須加總 input/output/scratch，並留 stack、ISR 與其他 task 餘量；不能因 L1=64 KB 就選 64 KB tile。

只試小倍率 unroll，如 1/2/4，觀察 branch、text、spill 與 I-cache；全域 unroll 可能讓 code 更慢。loop invariant hoisting 只適用於真正不變的值；由 ISR/DMA 更新的資料需先定義 snapshot 與 ownership 語意。

### Pointer 看連續性與 alias 不必禁止

a[i] 與 \*(a+i) 常會形成相同生成碼；真正危險的是 scattered pointer chasing，下一次地址依賴上一次 load，難以重疊且 locality 差。可試 contiguous array、固定容量 pool、compact index，以及 hot/cold fields 分離；不要為了避 pointer 改成反覆 by-value 複製大 struct。

AoS 適合逐物件使用多數欄位；SoA 適合跨大量物件只掃少數欄位，AoSoA 則分塊折衷。restrict 是必須符合的 alias 契約，不是免費加速標籤；有重疊讀寫的 API 不可亂加，const 也不是 no-alias。自然對齊優先；全面 packed 可能讓 RV32 存取變慢或 trap，全面 cache-line padding 又會膨脹 working set。[28][29]

### Function 按熱路徑與資料生命週期拆

把罕見 error handling、formatting、diagnostic log 或 fallback 移到 cold function，常走的 fast path 保持緊密；小而頻繁的 helper 可選擇性 inline。過度 inline 與拆太細都會有代價。GCC cold/section 是提示與配置工具，必須檢查 ELF/map/assembly，不能只看 C 函式長短。[30]

拆 function 不會自動改善 D-cache；需要同時改變資料布局、存取順序、batch size 或 live data。把一個 function 拆成三個各掃一次大 array 的函式，反而可能更慢。

ILM/DLM 應按「熱度 × 每次能省的成本 ÷ 佔用 bytes」優先配置。ILM 放真正熱 code；DLM 放重複使用的 hot data/scratch，必要時評估 stack。連 callee、constants、copy-in/out、DMA 可達性、容量與 ownership 一起檢查；沒有容量資料不能承諾全部 1 MB code／2 MB data 搬入。section attribute 不是已放對位置的證據。

### Recursion 要可界定的 stack 與最壞時間

hot path 中深度無上限的 recursion 不利於 stack 安全與 timing，可改成有界 iteration／explicit stack。但 explicit stack 仍有 memory cost，tail recursion 可能早已被 compiler 轉成 loop；先看 assembly、最大深度、每層 frame 與各 task stack high-water。-fstack-usage 也不能單獨證明整個 call chain 的上界。[31]

### MMIO 只省掉語意允許省的交易

先核對 register 的 width、alignment、read-clear、FIFO pop、W1C、command trigger、ordering 與 ownership。可合法做的包括獨占且穩定 configuration shadow、單次 status snapshot 供多個判斷、SET/CLEAR alias、FIFO/burst/batch，以及用事件代替長時間 polling；每一項都須有 datasheet 或 driver contract 支持。

不能把 MMIO 一律 cache 到 local variable，或移除 volatile。例：W1C status 用「讀回值 OR mask 再寫回」可能誤清其他 pending events；FIFO register 少讀一次就可能少取一筆資料。volatile 不是 cache attribute、atomic、task synchronization 或 hardware fence；用平台驗證的 exact-width accessor，保留必要 ordering 與 DMA cache maintenance。[32][29]

polling 改 interrupt 可能省 CPU 但增加 wake/ISR latency；batch 可省交易卻拉長首筆 response time。用原本的 end-to-end deadline、queue depth、ISR rate 與 CPU time 判定，不能只看 MMIO 次數變少。

### RV32 型別與 compiler 找出隱藏成本

核對 -march/-mabi/-mtune。RV32 不保證有 M、F、D、V、B 或 C；64-bit arithmetic、double 與變數除法可能生成多條指令或 libgcc helper。頻繁的 \_\_udivdi3／\_\_divdi3 等才是明確線索，不能一概禁止 64-bit。縮小型別需證明範圍與精度，不能引入 signed overflow／截斷／錯誤 rounding。[33][34]

8/16-bit storage 可縮小 data footprint，運算卻可能多 promotion/mask/sign extension；分開設計 storage width 與 arithmetic width。先查 compiler 是否已優化 constant division，不要把負數 signed division 直接改成 shift，或隨意用浮點 reciprocal 替代整數除法。

建立小型 build matrix：固定 toolchain/ISA/ABI/linker/input，比較 -O2/-Os/-O3 與 LTO off/on，並記錄 latency、text/rodata、RAM、stack 與正確性。-Os 有時因較小 text 而更快；-O3 不是保證。PGO 需要代表性訓練資料。不要先用 -Ofast/-ffast-math 改變產品數值語意。[31]

使用 disassembly、map、optimization remarks 解釋結果：helper/copy 是否消失、load/store 次數、spill、cold outline、ILM/DLM placement。若有 C extension 可量測壓縮指令對 code/fetch footprint 的影響，但不能借用規範的平均收益當本產品保證。少 memcpy/memset、formatting、buffer copy 常比語法微調有效；zero-copy 仍須維持 lifetime、DMA 與同步正確性。

## 九 第一輪最值得比較的實驗

以下每一列是一個可否證的假說。先做 functionality check，再觀察對應的低層指標，最後由同一口徑的產品 latency 決定。一次只改一種機制，最後再測候選組合與交互作用。

| 實驗 | 希望看到的機制 | 可能失敗或反效果 |
| --- | --- | --- |
| 兩 pass／fusion／tiling | 較少 intermediate load/store；較短 reuse distance | register spill、text 膨脹、破壞相依 |
| AoS／SoA／hot-cold | 有效 hot bytes 降低；cache miss 或 SRAM bytes 下降 | 全欄位 workload 變多串流 |
| 順序 array／pointer chain | serial load dependency 與 locality 的差異 | 資料規模過小，全在 DLM/L1 看不到差異 |
| cold outline／selective inline | 熱 instruction footprint 下降或 call cost 下降 | compiler 重組、更多 call/spill、錯誤路徑退化 |
| ILM/DLM selective placement | 目標區域 bypass，並降低其他 cache pollution | 容量、callee/constants 未搬、copy 成本、DMA 不可達 |
| 32/64-bit 與除法 variants | 減少 helper 或 expensive instruction | 數值語意／range 不等價 |
| MMIO snapshot／batch／事件 | 合法 transaction 減少或 busy cycles 減少 | read-clear/W1C 錯誤、response/tail 變慢 |
| -O2/-Os/-O3 與 LTO | 更好的 target codegen／size tradeoff | 指令數下降卻 I miss 或 stack 上升 |

先建小型 known-pattern microbenchmark 確認「量得到該機制」，再套到真實 extracted kernel。結果必須可觀測且可驗證，防止 compiler 消除；不要為了防止消除把所有 buffer 都標 volatile，那會改掉本來要測的優化。

測 warm/cold、單 task isolation 與三 task／ISR 混合負載。每案記錄 output、elapsed latency、cycles/instret、cache/region/bus/MMIO 指標（有可用來源才報）、code/data/stack。miss rate 下降不代表總 miss 數下降；instruction count 下降也不必然更快。

最終候選除了原本 200 ms 指標降至 ≤150 ms，還要確認其他 task deadlines、queue high-water、error paths、stack 與 ILM/DLM 容量沒有回退。若需求是硬即時，有限次實驗中的最大值不是 formal WCET，仍需另做上界分析。

## 十 建議 PoC 與選擇門檻

以下為熟悉現有 C code／RTOS 的一位工程師可用來估量的範圍，工作可重疊；若 BSP、custom ISA 或周邊模型缺失，成本會顯著增加。估計不是承諾交期，也不是現在開始實作。

| 階段與估計 | 產出 | 通過／停止條件 |
| --- | --- | --- |
| 0 基準定義 1–2 人日 | 200 ms breakdown；hot-path 邊界；ELF/map/build manifest | 找出可改善 50 ms 的來源；若 dominated by wait，改研究協定／排程 |
| 1 共用 workload 3–5 人日 | host gtest＋C fixture＋RV32 runner；最小 golden case | 相同輸入結果一致；可重複執行；不要求完整 SoC |
| 2 trace 與 cache PoC 4–8 人日 | ROI/region 指標；L1/L2 模型；cold/warm；首批候選 | microtrace 驗證 line/eviction/bypass；找到可解釋 hot spot |
| 3 校準與 A/B 3–5 人日 | 真機 microbench＋holdout；敏感度與候選 ranking | 效能方向有證據；不夠準則退回真機 benchmark 作裁判 |
| 4 timing 升級 視需要 | gem5 或 GVSoC 的 minimal SoC/core/memory model | 只有重疊／bus／schedule 影響排名才做；通常另需 2–6+ 週 |
| 5 九設備驗收 視場地 | 候選版本 E2E／tail／error／deadline report | 達到定義的 150 ms 目標並維持功能與穩定性 |

第一輪建議只選 QEMU 與單板 benchmark，投入約 1–2 週取得能重播、可量測的小範例，再判斷是否加深 cache model；基本比較與校準整體可按約 2–4 週規劃。GVSoC 若有相近 core 可先做短試配；若不相近，優先考慮 gem5 的可組態元件，避免同時維護兩套平台。

### 每一輪必須留下可重現的證據

case/scenario、binary hash、compiler/ISA/ABI、map、simulator release/commit、所有 cache/bus/ILM/DLM 參數與 random seed。

baseline/candidate 功能結果、ROI instruction/region/cache 指標、估計 cycles 與明確模型假設、真機量測／未量測標記。

改善原因、壞情境、code/data/stack 成本與 correctness guard；每次先改一種因素，最後才測組合交互作用。

停止標準：若平台移植持續擴張卻無法改變決策，保留功能模擬，性能回到 shared workload 的單板 A/B；不為了模擬完整而模擬完整。

## 參考來源與閱讀順序

以下為官方文件、原始碼與原作者論文。工具能力依 2026 年 10 月 3 日可查得的文件與分支核對；正式實作時應固定 release 或 commit，不能假設本機舊版本具有相同選項。本文的模型選擇、門檻與工期是工程建議，不是文獻保證。

[1] [QEMU Emulation cache modelling](https://www.qemu.org/docs/master/about/emulation.html#cache-modelling)

[2] [QEMU cache plugin source](https://github.com/qemu/qemu/blob/master/contrib/plugins/cache.c)

[3] [QEMU TCG Plugins API](https://www.qemu.org/docs/master/devel/tcg-plugins.html)

[4] [QEMU TCG Instruction Counting](https://www.qemu.org/docs/master/devel/tcg-icount.html)

[5] [gem5 RISC V ISA configuration](https://github.com/gem5/gem5/blob/stable/src/arch/riscv/RiscvISA.py)。另見 RiscvBareMetal workload 文件與目前版本平台設定

[6] [gem5 SimpleCPU and TimingSimpleCPU](https://www.gem5.org/documentation/general_docs/cpu_models/SimpleCPU)

[7] [gem5 Classic caches](https://www.gem5.org/documentation/general_docs/memory_system/classic_caches/)

[8] [gem5 SimpleMemory parameters](https://github.com/gem5/gem5/blob/stable/src/mem/SimpleMemory.py)

[9] [gem5 Crossbar parameters](https://github.com/gem5/gem5/blob/stable/src/mem/XBar.py)

[10] [GVSoC Models and ISS timing](https://gvsoc-developer.readthedocs.io/en/latest/models.html)

[11] [GVSoC Tutorials interconnect timing](https://gvsoc-developer.readthedocs.io/en/latest/tutorials.html#how-to-customize-an-interconnect-timing)

[12] [Renode Metrics and profiling](https://renode.readthedocs.io/en/latest/execution-tracing/metrics-and-profiling.html)

[13] [Renode Guest cache modelling](https://renode.readthedocs.io/en/latest/execution-tracing/guest-cache-modelling.html)

[14] [Renode Time framework](https://renode.readthedocs.io/en/latest/advanced/time_framework.html)

[15] [Spike cache simulator source](https://github.com/riscv-software-src/riscv-isa-sim/blob/master/riscv/cachesim.cc)

[16] [DynamoRIO drmemtrace overview](https://dynamorio.org/sec_drcachesim.html)

[17] [GoogleTest CMake quickstart](https://github.com/google/googletest/blob/main/docs/quickstart-cmake.md)

[18] [Unity embedded C test framework](https://github.com/ThrowTheSwitch/Unity)

[19] [CMock mock generation for C](https://github.com/ThrowTheSwitch/CMock)

[20] [QEMU execution replay](https://www.qemu.org/docs/master/devel/replay.html)

[21] [Sundmark et al industrial RTOS replay AADEBUG 2003](https://arxiv.org/abs/cs/0311019)

[22] [RISC V counters specification](https://docs.riscv.org/reference/isa/v20260120/unpriv/counters.html)

[23] [Bruschi et al GVSoC ICCD 2021](https://arxiv.org/abs/2201.08166)。優先閱讀：中間抽象層與 calibration 的可行案例

[24] [Pathak et al Component Level Calibration TECS 2025](https://doi.org/10.1145/3737876)。優先閱讀：full-system 模型也必須校準

[25] [Desikan et al Measuring Experimental Error ISCA 2001](https://doi.org/10.1145/379240.565338)。歷史 calibration 方法；須一併看作者 errata，不採其被修正的比較案例

[26] [Chatzopoulos et al RISC V performance modeling CARRV 2021](https://arxiv.org/abs/2106.09991)

[27] [LLVM Loop Fusion legality and dependencies](https://llvm.org/docs/LoopFusion.html)

[28] [LLVM vectorization alias checks and optimization remarks](https://llvm.org/docs/Vectorizers.html)。引用 alias/分析概念，不假設本 RV32 有 vector extension

[29] [RISC V RV32I load store alignment and FENCE](https://docs.riscv.org/reference/isa/v20260120/unpriv/rv32.html)

[30] [GCC common function attributes](https://gcc.gnu.org/onlinedocs/gcc-13.4.0/gcc/Common-Function-Attributes.html)

[31] [GCC Optimization Options](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)

[32] [GCC Volatiles](https://gcc.gnu.org/onlinedocs/gcc/Volatiles.html)

[33] [GCC RISC V target options](https://gcc.gnu.org/onlinedocs/gcc/RISC-V-Options.html)

[34] [GCC Integer library routines](https://gcc.gnu.org/onlinedocs/gccint/Integer-library-routines.html)

[35] [gem5 MinorCPU in order pipeline model](https://www.gem5.org/documentation/general_docs/cpu_models/minor_cpu)

[36] [gem5 RiscvBareMetal workload](https://doxygen.gem5.org/develop/classgem5_1_1RiscvISA_1_1BareMetal.html)

[37] [Desikan et al Errata on Measuring Experimental Error](https://doi.org/10.1145/511120.511122)
