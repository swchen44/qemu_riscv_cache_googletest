# Fresh remote-source verification

Source was freshly cloned from GitHub commit `9f9e8f410ac23b7586ee1ed58543d48456d02f21` into a new directory. Every Git content hash passed. The fixed producer completion barrier is included.

On the available Debian13 x86-64 cloud host, the 15-step RV32/FreeRTOS matrix and 8-step MicroPython matrix passed, including the expected failing negative oracles. Logs and machine-readable results are included here.

Dependencies came from the existing local cache and were verified against the exact locked SHA256 before use. This is evidence for remote-source reproduction with pinned dependencies, not a claim that pending GitHub Release assets were already uploaded/downloaded. Ubuntu VM validation is separate and still pending.

The Ubuntu bundle helper was independently run read-only against the complete 121-package bundle on disk; it verifies hashes but does not claim to install Ubuntu packages on Debian.
