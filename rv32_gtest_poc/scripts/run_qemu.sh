#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/env.sh"
cd "$ROOT"
printf 'QEMU_CLOCK_PROFILE=%s\n' "$RV32_QEMU_CLOCK"
timeout 30 "$QEMU" "${QEMU_CLOCK_ARGS[@]}" -machine virt -cpu rv32 -smp 1 -m 32M -bios none -nographic -monitor none -no-reboot -kernel "$1"
