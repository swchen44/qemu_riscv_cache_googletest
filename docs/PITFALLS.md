# 實際踩坑紀錄

每條列出現象、原因、處理與證據。這裡記錄已發生的問題，不把可能性寫成已驗證。

1. **FFF include遮蔽GoogleTest**：FFF1.1附舊gtest目錄；先將1.15.2 include路徑放前面。否則編譯出大量互斥定義。
2. **newlib fileno未宣告**：嚴格C++ mode的feature visibility；設`_POSIX_C_SOURCE=200809L`，不是建立POSIX OS。初次compile failure log保留。
3. **自訂startup缺__dso_handle**：`-nostartfiles`未使用完整CRT；提供受控符號。已有CRT的真板不要重複定義。
4. **constructors/0tests風險**：TEST registration需.init_array及__libc_init_array；正常runner檢查5/6/11 case count。0-case控制組只量size。
5. **QEMU官方deb缺共享庫**：初次Debian cloud缺libndctl/libdaxctl；下載官方對應包並驗hash。此修補不是Ubuntu相容性證明。
6. **MicroPython basic profile缺platform與open**：補明確platform字串；本例無filesystem所以關閉IO，不提供虛假的成功open。
7. **MicroPython generated moduledefs殘留**：改config後舊registration仍在；用config-hash獨立buildgen目錄重新生成。
8. **C++/Python exceptions混淆**：前者編譯停用，後者C NLR仍真實執行並測過。NLR不跨C++ frame。
9. **多task runtime風險**：本例GoogleTest由一個task使用；newlib/FFF global state不自動變thread-safe。
10. **ELF磁碟4–6MB不是RAM需求**：含debug；size/text/data/bss、配置heap與實際peak分开看。絕對__FILE__路徑會影響retained bytes。
11. **workspace可消失**：原實驗以有hash的source/tool artifacts保存；還原後先驗hash，再從實際遠端commit重建。
12. **GitHub帳號push=true≠integration可寫**：本次get_repo可讀但create_blob實際403；停止上傳並請修正正式授權，不讀token/繞過限制。
13. **GitHub source ZIP不是Release bundle**：LFS/submodule/release assets不一定包含；內網必帶清單與SHA必須明列。
14. **Ubuntu與Debian混用**：Debian13 QEMU及libraries不能直接當Ubuntu24.04答案。各自記錄version/ABI/run evidence。

詳細C++ linker、entry、memory與命令請見[FAQ](../rv32_gtest_poc/docs/FAQ.md)。

15. **Ubuntu offline apt cache pathname**：保留--no-download時local.deb安裝遇Pathname to install is not absolute；明確指定`Dir::Cache::archives`到預載apt/archives後已跨過安裝階段。首輪失敗不刪除。
16. **FreeRTOS supervisor過早檢查producer計數**：worker八次notification不代表低priority producer已從最後xQueueSend返回；固定delay2在巢狀TCG暴露produced/preemptions=7而processed=8。改用producer完成後give的binary semaphore，supervisor真take；原8/8/8/1嚴格assert全保留，不靠加sleep或放寬oracle。先前交付的source archive是修復前Debian歷史snapshot；內網請用最終repo/Release commit。
17. **巢狀TCG raw-clock 100tick timeout**：加completion barrier後，診斷顯示首wait tick37→148回0，當時processed/produced為0；首send tick272、首notify tick310，後7次return=1。這不是通知合併>1；期限在工作開始前已過。相同diagnostic ELF採固定icount後3次正例成功；完整normal-build驗收另列。Trace不能分辨host scheduling/JIT各占比，也不代表raw-clock修好了，更不是硬體cycle/cache模型。
18. **把public研究remote帶進公司後仍繼續push**：此repo只授權公開實驗/source/研究。接公司code前要改內部remote或停用外網push；不要把真實公司source/log/trace/credentials沿用原origin推回來。本次未改任何repo visibility或公司設定。

19. **MicroPython tarball拾到外層handoff repo的Git版本**：新clone曾在generated mpversion.h看到9f9e8f4-dirty，雖vendor bytes/commit正確。官方makeversionhdr支援MICROPY_GIT_TAG/HASH；prepare從已驗SHA的lock顯式傳v1.26.1/647c8b...，不改upstream interpreter或test assertions，讓runtime metadata不混入handoff repo commit。原輸入log保留，後續版本記錄此metadata-only差異。
20. **把手動修改只留在vendor/generated**：prepare會從鎖定archive重新生成這些目錄；未保存的修改可能被覆蓋。公司code/patch要在自己的受控source tree保存，先commit或另存patch，再明確套用；不要把私有改動塞回公開上游archive或上傳public repo。
