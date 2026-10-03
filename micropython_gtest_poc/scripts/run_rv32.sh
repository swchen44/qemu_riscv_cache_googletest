#!/usr/bin/env bash
set -euo pipefail
MP_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BASE="$(cd "$MP_ROOT/../rv32_gtest_poc" && pwd)"
source "$BASE/scripts/env.sh"
timeout 30 "$QEMU" -machine virt -cpu rv32 -smp 1 -m 32M -bios none -nographic -monitor none -no-reboot -kernel "$MP_ROOT/build/${1:-rv32}/firmware.elf"
