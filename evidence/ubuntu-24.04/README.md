# Ubuntu24.04 x86-64 實際驗收

純TCG完整Ubuntu VM、無NIC，官方Ubuntu套件離線安裝。final-ubuntu-validation/evidence保存實際23/23步結果：第一階段15步明示icount；MicroPython8步明示raw。正例GTest數5/6/11；六個刻意負例均exit1；套件外層exit0。不是只有boot或build成功。

RTOS source SHA888b43e2e99c501b10d60e415e8c9946b831c9b6cb3d3835ebbe36f6084c0279；正常build未定義RTOS_DIAGNOSTICS。clock-diagnostic是另一次同ELF raw/icount對照，保留raw return0失敗及icount三正一負。attempt2/3保存race與raw-clock失敗，不把未校準raw宣稱修好。

本次VM原harness僅outer timeout改1800秒；內層QEMU30秒、RTOS100tick與==1不變。交付script改用VERIFY_TIMEOUT_SECONDS環境變數，是等價配置介面；新增results metadata不回寫這批原始JSON。

final-ubuntu-validation/micropython-provenance/provenance為測後只讀取得的原header：TAG=v1.26.1、HASH=<no hash>，guest沒有git；讀取前後binary hashes相同。後續交付prepare明確pin完整hash，並在Debian重建/實跑同11例。../version-provenance/hash-field-control.json證明固定相同TAG時只改HASH在本embed profile的完整RV32 ELF及load bytes相同；不假稱Ubuntu曾用新helper重編。

這批是實際測試來源/runner payload驗證，並非最終Git commit已下載重驗的替代品；遠端驗證另列。虛擬time不能保證真機性能，沒有cache或cycle模型。

機器可讀摘要：UBUNTU_VALIDATION_RESULTS.json；完整報告：UBUNTU_VALIDATION_REPORT_繁體中文.md。報告中的attempts與logs完整集合隨Ubuntu Release addon提供；此Git目錄另保留最關鍵raw/race診斷，並未聲稱把所有VM輸入搬進Git。
