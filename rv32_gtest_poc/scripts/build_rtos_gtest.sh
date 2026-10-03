#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/env.sh"
cd "$ROOT"; name=${1:-rtos-gtest}; extra=${2:-}; mkdir -p build/"$name"
k=vendor/FreeRTOS-Kernel-11.1.0
g=vendor/googletest-1.15.2/googletest
flags="-march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medany -O2 -g -ffunction-sections -fdata-sections -Isrc -Ifixtures -I$g/include -I$g -Ivendor/fff-1.1 -Ifreertos -I$k/include -I$k/portable/GCC/RISC-V -I$k/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions -DUSE_FREERTOS -DRUN_GTEST_IN_RTOS"
cxxflags="-std=c++14 -D_POSIX_C_SOURCE=200809L -fno-exceptions -fno-rtti -DGTEST_HAS_PTHREAD=0 -DGTEST_HAS_EXCEPTIONS=0 -DGTEST_HAS_RTTI=0 -DGTEST_HAS_POSIX_RE=0 -DGTEST_HAS_STREAM_REDIRECTION=0 -DGTEST_HAS_FILE_SYSTEM=0"
objs=()
for f in src/product.c src/product_adapter.c freertos/syscalls.c freertos/start.S freertos/demo.c "$k/tasks.c" "$k/queue.c" "$k/list.c" "$k/portable/MemMang/heap_4.c" "$k/portable/GCC/RISC-V/port.c" "$k/portable/GCC/RISC-V/portASM.S"; do
 obj="build/$name/$(basename "$f").o"; riscv-none-elf-gcc $flags $extra -c "$f" -o "$obj"; objs+=("$obj")
done
for f in tests/product_test.cc tests/bare_main.cc "$g/src/gtest-all.cc"; do
 obj="build/$name/$(basename "$f").o"; riscv-none-elf-g++ $flags $cxxflags $extra -c "$f" -o "$obj"; objs+=("$obj")
done
riscv-none-elf-g++ $flags $cxxflags -nostartfiles -Tfreertos/link.ld -Wl,--gc-sections,-Map,build/"$name"/firmware.map "${objs[@]}" -o build/"$name"/firmware.elf
riscv-none-elf-size build/"$name"/firmware.elf
