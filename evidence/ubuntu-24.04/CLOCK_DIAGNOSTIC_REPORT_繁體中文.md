# Ubuntu QEMU8 / FreeRTOS時鐘診斷

## 已確認的兩個不同問題

1. 完成同步：原supervisor用vTaskDelay(2)推測producer完成；Ubuntu巢狀TCG跑出processed=8但produced/preemptions=7。用producer_done binary semaphore建立真正完成關係後，8/8/8和timeout1均成立。這是獨立的應用同步修正。
2. 未校準raw clock期限：barrier版仍出現通知等待失敗。加入編譯期RTOS_DIAGNOSTICS後，實際return值確認為0，不是大於1的累積通知。

## 同一診斷ELF比較

Source demo.c SHA256：888b43e2e99c501b10d60e415e8c9946b831c9b6cb3d3835ebbe36f6084c0279。

Raw clock第一個正例：

- supervisor第一次wait tick37開始，tick148返回0，已經過111tick
- 此時produced=0、processed=0，還沒有工作完成
- producer第一次send在tick272
- worker第一次notify在tick310
- 接下來七個notification return均為1；最終produced=8、processed=8、preemptions=8、queue_timeouts=1
- 整個正例仍exit1，結果保留為FAIL，未改寫為通過

Fixed icount：-icount shift=0,align=off,sleep=on。

- 相同ELF三次正例全部exit0
- 第一次wait tick0→5，返回1；之後每1tick接收一次，八次皆1
- producer/supervisor完成在tick13；58個trace events，dropped=0
- 三份stdout SHA256一致：84537e64aa540859ef86e047ede14e6f9f9515e8b177340d9a4b337653e63ae6
- INJECT_FAILURE負例仍exit1，且正常排程計數保持8/8/8/1
- raw負例也exit1；其結果不單獨證明沒有其他raw時鐘錯誤，判讀需看完整log

全程保留原ulTaskNotifyTake(...,100)、==1與所有數值斷言。沒有增加sleep或放寬期限。Trace以有限96格陣列先記錄task/event/tick，最後輸出，不在正常資料交换期間輸出trace；失敗診斷仍會立即列出actual return。

## 結論與限制

證據顯示這次失敗是notification尚未送出前，100tick期限已到；沒有觀察到大於1的累積通知。相同ELF切換明確icount後三次trace一致，支持raw虛擬時鐘受巢狀模擬執行耗時影響。

這不是host排程/JIT開銷的分項量測；trace本身可能擾動raw時序，不能從這些資料唯一判定兩者各占比例，也不能宣稱修好了default raw clock。

因此交付提供兩個清楚模式：RV32_QEMU_CLOCK=raw保留原未校準觀察；RV32_QEMU_CLOCK=icount是明確的決定性功能重播。官方QEMU8.2.2說明icount不是cycle-accurate模擬，不能用這裡的虛擬毫秒或指令數宣称真晶片cycle、cache命中率或效能。

完整Ubuntu矩陣將第一階段15步標為icount功能模式，MicroPython8步標為raw模式；不得合併描述成「default clock全部通過」。原raw失敗、barrier階段結果、trace正負例及最後矩陣均保留。

## 證據

- attempts/attempt2/evidence/rv32_gtest_poc/rtos-pass.log：原完成同步失敗
- attempts/attempt3/evidence/freertos-preflight-positive.log：barrier後計數正確，但通知期限失敗
- attempts/clock-diagnostic/evidence/clock-raw-positive-1.log：actual return/tick/order
- attempts/clock-diagnostic/evidence/clock-icount-positive-{1,2,3}.log：同ELF三次功能重播
- attempts/clock-diagnostic/evidence/clock-icount-negative.log：故意失敗仍傳遞exit1
- logs/notification-trace-analysis.json：逐次wait解析
- logs/qemu8-icount-official-documentation.txt：官方qemu-options.hx相關說明
