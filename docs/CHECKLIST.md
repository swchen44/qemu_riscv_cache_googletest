# 重現與內網交接Checklist

請填commit、日期、操作者/AI與具體log位置；未執行不能勾選。歷史完成與本次遠端重現分開記錄。

## 已保存的歷史baseline

- [x] 純C host GTest/FFF 5 tests + negative exit1
- [x] 同組RV32裸機5 tests + negative exit1
- [x] 真FreeRTOS三應用task，GTest6 tests + negative exit1
- [x] 真MicroPython host/RV32同組11 tests + negative exit1
- [x] 獨立解包乾淨重建通過
- [x] framework cost控制組實跑；明示runtime依賴與未測heap/stack peak

## 本次Ubuntu/GitHub交付

- [x] 目標repo/既有history核對，只操作qemu_riscv_cache_googletest
- [x] 使用者確認Ubuntu24.04 x86-64
- [x] Ubuntu24.04完整VM normal matrix 23/23 PASS（15/icount、8/raw）；原始證據與raw失敗保留
- [x] Ubuntu121 binary依賴、62套對應source的版本/URL/hash/license完整鎖定並下載
- [ ] Release工具包完成上傳，hash與size確認
- [x] main source commit實際遠端可見；每commit含Why/What/Test
- [x] 從遠端8844093新clone，source/dependency hashes通過（dependencies為固定local cache）
- [x] 遠端source於Debian13重跑15/icount＋8/raw及helper5全PASS；Ubuntu23步另有明示profile證據
- [ ] 離線必帶清單完整且不依賴Library/private links

## 帶入內網前

- [ ] 閱讀docs/OFFLINE_HANDOFF.md，理解GitHub source ZIP不含Release assets
- [ ] 取得指定source commit及所有required Release assets
- [ ] 驗證完整SHA256清單、scan與公司開源/資安規範
- [ ] 備份Ubuntu版本/architecture/libc與必要host工具清單
- [ ] 斷網模式缺檔即fail，不能偷偷download或skip
- [ ] 保存原始測試log與預期case count/negative結果

## 公司code與真機

- [ ] 確認產品ISA/ABI/RTOS/BSP/map，不照抄demo virt地址
- [ ] 相同輸入fixture/oracle與host baseline
- [ ] 檢查C++ constructors、.data copy、stack/heap、newlib/RTOS allocation
- [ ] 921600 UART framing/clock/divisor/driver/log策略確認
- [ ] 真機性能量測邊界、cache/ILM/DLM、IRQ/task負載確認
- [ ] 200→150ms只用可信真機或校準timing結果判定
- [ ] 公司機密不預設上傳外網
