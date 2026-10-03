# Commit格式

每個commit都要包含：

```text
Short action-oriented subject

Why:
- 觀察到的問題/需求與改動原因。

What:
- 實際修改的source/config/tests/docs。

Test:
- 已跑命令、環境、case count與結果。
- 未跑項與原因；不要用舊baseline冒充新target。
```

不force-push既有history。未授權不要改其他repo、visibility、permissions或新增會取權限的workflows。必要的大檔放Release並提供hash/完整離線清單。
