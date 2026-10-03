# QEMU raw-clock與固定icount：功能驗收，不是硬體效能模型

## 為什麼分profile

Ubuntu24.04巢狀TCG實驗保留了原RTOS100tick/通知返回==1的嚴格檢查。加入producer completion barrier後，raw-clock仍觀察到首wait tick37→148回0，而首send為272、首notify為310。這支持「未校準raw virtual clock期限與巢狀模擬執行耗時不匹配」；沒有通知累積>1的證據，也不能由這個trace判定host排程/JIT各占比。

不能把sleep加長、deadline放寬、移除assert或只保留pass log當修好。raw失敗與trace必須保存。

## 明示的兩種設定

| 設定 | 意義 | 使用目的 |
|---|---|---|
| `RV32_QEMU_CLOCK=raw` | 不加icount；保留既有QEMU預設clock行為 | 對照觀察，不能当產品100ms期限保證 |
| `RV32_QEMU_CLOCK=icount` | `-icount shift=0,align=off,sleep=on` | 功能性replay的明確虛擬時間設定 |

以實驗所用QEMU8.2.2原始`qemu-options.hx`第4654–4686行為依據：fixed shift使virtual time與instruction count相關；sleep=on使虛擬CPU睡眠時推進到下一個timer deadline。這不等於真core CPI、pipeline/cache/ILM/DLM模型。`shift=0`更不表示已證明產品CPU為1GHz。

本例FreeRTOS `configCPU_CLOCK_HZ=10000000`實際用來計算QEMU virt的mtime tick間距；不要從這個macro名稱推論產品CPU frequency。真BSP需確認timer timebase。

## 驗證方式

- 同一診斷ELF分別跑raw與icount，只變clock args；已保存raw failure與icount三次正例、負例證據。
- 診斷trace由`RTOS_DIAGNOSTICS`控制，有限陣列、無交換過程stdio；正式normal build不啟用trace。
- 內層QEMU30秒deadline、RTOS100tick與==1未放寬。
- 全套normal-build已實跑15/icount + 8/raw，23步全部符合預期，不能拿診斷ELF代替normal suite。
- Outer Python build/run harness在慢TCG可明示`VERIFY_TIMEOUT_SECONDS=1800`；這只影響外部執行等待，不改RTOS期限。原生host保留120/180秒default。

## 正式命令必須匹配證據

README/結果manifest逐項標記第一階段與MicroPython實際測過的profile。不要用未測的profile組合代換，再把另一組pass套用。低階runner的raw預設可以保留，但正式功能驗收命令應明示其已測profile。

結果JSON應記host profile、clock profile及outer harness timeout；原本已產生的歷史results不改寫，另用validation manifest綁定原始log/hash/明確環境設定。
