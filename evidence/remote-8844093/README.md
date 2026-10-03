# Fresh remote commit verification

Fetched by a new git clone from the actual public GitHub repository, exact commit88440935b08205b11c9b2cdfa9196ca5ad1986d0. All Git content hashes passed before building.

On Debian13: first-stage15 steps with RV32_QEMU_CLOCK=icount; MicroPython8 with raw; all six intentional negatives exited1. Five isolated offline bundle helper tests also passed. Same normal C/test sources and pinned MicroPython generated version preparation as this commit.

Dependencies were restored from the existing SHA-locked local cache and verified by --offline; this does not claim Release download validation. The first preparation correctly stopped on three missing historical Debian QEMU source cache files; its failure log is preserved, then exact locked files were restored and preparation passed. Ubuntu uses the separate Ubuntu source preparation and its own verified packages.

This evidence-only follow-up commit does not change runtime source, tests, runner flags or dependency locks. Original Ubuntu23-step VM evidence is separate and explicitly uses15/icount +8/raw. No physical timing/cache-model claim.
