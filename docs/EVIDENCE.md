# 證據與路徑正規化

兩個專案evidence保留真實命令/exit/count/size及失敗紀錄。公開版將不影響判讀的雲端絕對workspace路徑改成`${REPO}`/`${WORKSPACE}`、host識別名改cloud-host；數值與測試結果不改。這是公開文件處理，不是重新產生測試通過結果。

歷史Debian13完整矩陣已通過；此次從保存archive還原到新cloud再跑15步及8步也通過。這仍不是從GitHub新commit下載後的證明；後者待首次推送後獨立執行並新增紀錄。Ubuntu24.04另以完整TCG VM測試中，boot成功不能替代完整matrix。

原始source archives保持其先前SHA；repo內SOURCE_SHA256SUMS會依本次受控runner/docs修改重新建立。正常GTest counts必須5/6/11，負例必須exit1，timeout不是pass。

## 已完成遠端source重現

GitHub commit9f9e8f410ac23b7586ee1ed58543d48456d02f21已新clone，Git內容hash全過，Debian13的15+8步全PASS（含barrier）。具體logs在evidence/remote-debian；dependencies來自固定hash的local cache，未把這件事冒稱Release download驗收。
