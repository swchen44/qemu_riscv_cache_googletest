# 證據與路徑正規化

兩個專案evidence保留真實命令/exit/count/size及失敗紀錄。公開版將不影響判讀的雲端絕對workspace路徑改成`${REPO}`/`${WORKSPACE}`、host識別名改cloud-host；數值與測試結果不改。這是公開文件處理，不是重新產生測試通過結果。

歷史Debian13完整矩陣已通過；此次從保存archive還原到新cloud再跑15步及8步也通過。GitHub新clone驗證另見下節。Ubuntu24.04完整TCG VM已正常build實跑23/23步，詳見evidence/ubuntu-24.04：15/icount、8/raw，並保存修復前與raw-clock失敗。

原始source archives保持其先前SHA；repo內SOURCE_SHA256SUMS會依本次受控runner/docs修改重新建立。正常GTest counts必須5/6/11，負例必須exit1，timeout不是pass。

## 已完成遠端source重現

GitHub commit9f9e8f410ac23b7586ee1ed58543d48456d02f21已新clone，Git內容hash全過，Debian13的15+8步全PASS（含barrier）。具體logs在evidence/remote-debian；dependencies來自固定hash的local cache，未把這件事冒稱Release download驗收。

## 最終功能source遠端重驗

[88440935b08205b11c9b2cdfa9196ca5ad1986d0](https://github.com/swchen44/qemu_riscv_cache_googletest/commit/88440935b08205b11c9b2cdfa9196ca5ad1986d0)由新目錄實際clone，全部Git SHA通過，Debian13明示15/icount＋8/raw與helper5全部PASS。見evidence/remote-8844093。其後此紀錄commit只有文件/evidence，沒有更動測試/runtime來源。Release遠端下載驗證仍另外待完成。
