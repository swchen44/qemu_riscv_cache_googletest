#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/env.sh"
cd "$ROOT"
name=${1:-rtos}; extra=${2:-}; mkdir -p build/"$name"
k=vendor/FreeRTOS-Kernel-11.1.0
flags="-march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medany -O2 -g -ffunction-sections -fdata-sections -Ifreertos -Isrc -Ifixtures -I$k/include -I$k/portable/GCC/RISC-V -I$k/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions"
objects=()
for f in src/product.c freertos/demo.c freertos/syscalls.c freertos/start.S "$k/tasks.c" "$k/queue.c" "$k/list.c" "$k/portable/MemMang/heap_4.c" "$k/portable/GCC/RISC-V/port.c" "$k/portable/GCC/RISC-V/portASM.S"; do
 obj="build/$name/$(basename "$f").o"
 riscv-none-elf-gcc $flags $extra -DUSE_FREERTOS -c "$f" -o "$obj"
 objects+=("$obj")
done
riscv-none-elf-gcc $flags -nostartfiles -Tfreertos/link.ld -Wl,--gc-sections,-Map,build/"$name"/firmware.map "${objects[@]}" -o build/"$name"/firmware.elf
riscv-none-elf-size build/"$name"/firmware.elf
