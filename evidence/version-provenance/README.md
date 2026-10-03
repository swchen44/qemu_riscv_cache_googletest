# Generated MicroPython version provenance

The fresh GitHub clone generated `MICROPY_GIT_TAG "9f9e8f4-dirty"` and `MICROPY_GIT_HASH "9f9e8f4"` by finding the enclosing handoff repository. The source tar/commit itself was still correctly pinned.

The corrected preparation uses the upstream-supported MICROPY_GIT_TAG/HASH environment variables from the hash-verified lock, and checks the generated header. It now records v1.26.1 / 647c8b96cae7e202c7a020395b7cfe65e5b8ce04.

The tag participates in MicroPython sys.version via the banner macro, so correcting a contaminated tag can change loadable bytes. Host and RV32 images were rebuilt with the corrected preparation and each reran the unchanged 11 GoogleTests successfully on Debian13. Logs are attached. These are not a claim that old Ubuntu inputs were silently changed; the Ubuntu generated header/input hashes must be checked separately against its actual run.

A separate controlled build kept TAG=v1.26.1 and changed only HASH=<no hash> versus the full pinned commit. Both complete RV32 ELFs and extracted loadable bytes were identical; see hash-field-control.json and the two saved headers. This qualifies the impact of the unused HASH macro in this exact embed profile; the Ubuntu original header still needs actual extraction before deciding whether its loaded version content changed.

Actual Ubuntu headers have now been recovered read-only: TAG=v1.26.1, HASH=<no hash>, no git executable. The three built binary hashes were unchanged before/after extraction. See ../ubuntu-24.04/final-ubuntu-validation/micropython-provenance/provenance. Thus the contaminated TAG was specific to the Git checkout workflow; the Ubuntu loaded TAG was already correct. The new pinned preparation itself was exercised on Debian, not retroactively attributed to the historical Ubuntu run.
