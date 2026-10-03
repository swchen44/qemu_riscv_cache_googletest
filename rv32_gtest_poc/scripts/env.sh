#!/usr/bin/env bash
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export PATH="$ROOT/tools/xpack-riscv-none-elf-gcc-15.2.0-1/bin:$PATH"
RV32_HOST_PROFILE="${RV32_HOST_PROFILE:-debian-13}"
case "$RV32_HOST_PROFILE" in
  debian-13)
    if [[ -z "${QEMU:-}" ]]; then
      export LD_LIBRARY_PATH="$ROOT/tools/qemu-root/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
      QEMU="$ROOT/tools/qemu-root/usr/bin/qemu-system-riscv32"
    fi
    ;;
  ubuntu-24.04)
    if ! (source /etc/os-release; [[ "$ID" == ubuntu && "$VERSION_ID" == 24.04 ]]); then
      echo 'ubuntu-24.04 profile requires actual Ubuntu 24.04 userspace' >&2
      return 2 2>/dev/null || exit 2
    fi
    QEMU="${QEMU:-/usr/bin/qemu-system-riscv32}"
    ;;
  *) echo "Unknown RV32_HOST_PROFILE: $RV32_HOST_PROFILE" >&2; return 2 2>/dev/null || exit 2 ;;
esac
RV32_QEMU_CLOCK="${RV32_QEMU_CLOCK:-raw}"
QEMU_CLOCK_ARGS=()
case "$RV32_QEMU_CLOCK" in
 raw) ;;
 icount) QEMU_CLOCK_ARGS=(-icount shift=0,align=off,sleep=on) ;;
 *) echo "Unknown RV32_QEMU_CLOCK: $RV32_QEMU_CLOCK" >&2; return 2 2>/dev/null || exit 2 ;;
esac
export QEMU RV32_HOST_PROFILE RV32_QEMU_CLOCK
