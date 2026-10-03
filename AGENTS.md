# Reproducing and extending this repository

This is a functional RV32 GoogleTest/FFF/FreeRTOS/MicroPython laboratory. Read README.md, docs/AI_HANDOFF.md, docs/CHECKLIST.md and the stable-numbered FAQ before changes.

- Keep the same C product/interpreter sources and C++ test source across host and RV32 unless a documented port boundary requires a change. Do not replace GoogleTest with another framework or mock the interpreter under test.
- Never call a build-only result a pass. Record exact command, compiler/QEMU versions, ELF32 ISA/ABI, exit code, expected case count and negative-oracle outcome.
- A zero-test GoogleTest executable may exit 0. Require 5/6/11 expected cases for the current suites. The intentional zero-case size control is diagnostic only.
- The current target is Ubuntu 24.04 x86-64. Historical Debian 13 runs are separate evidence. Run preflight and the documented target-specific dependency preparation; do not silently use Debian binary packages on Ubuntu.
- Dependency versions, official source URLs, SHA256 and licenses are required. Offline mode must not fetch the network or silently skip missing dependencies.
- Keep company/proprietary code and credentials inside the company environment. Do not upload new private code, logs, traces or identifiers by default.
- Do not infer timing accuracy from QEMU wall time, virtual ticks or GoogleTest milliseconds. Product ISA, BSP and timing model must be confirmed before performance claims.
- Preserve existing history. No force-push, unrelated-repository changes, publishing or permissions changes without explicit user authorization.
- Every commit message must include Why, What and Test sections. Explain failures and unrun stages honestly.
- Append new FAQ entries with stable IDs. Update README, plan, checklist, pitfalls and handoff documents when assumptions or reproduction steps change.
