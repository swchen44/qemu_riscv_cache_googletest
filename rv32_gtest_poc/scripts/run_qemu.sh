#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/env.sh"
cd "$ROOT"
timeout 30 "$QEMU" -machine virt -cpu rv32 -smp 1 -m 32M -bios none -nographic -monitor none -no-reboot -kernel "$1"
