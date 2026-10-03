#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/env.sh"
cd "$ROOT"; name=${1:-bare-gtest}; extra=${2:-}; mkdir -p build/"$name"
g=vendor/googletest-1.15.2/googletest
flags="-march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medany -O2 -g -ffunction-sections -fdata-sections -Isrc -Ifixtures -I$g/include -I$g -Ivendor/fff-1.1"
cxxflags="-std=c++14 -D_POSIX_C_SOURCE=200809L -fno-exceptions -fno-rtti -DGTEST_HAS_PTHREAD=0 -DGTEST_HAS_EXCEPTIONS=0 -DGTEST_HAS_RTTI=0 -DGTEST_HAS_POSIX_RE=0 -DGTEST_HAS_STREAM_REDIRECTION=0 -DGTEST_HAS_FILE_SYSTEM=0"
objs=()
for f in src/product.c src/product_adapter.c freertos/syscalls.c freertos/start.S; do
 obj="build/$name/$(basename "$f").o"; riscv-none-elf-gcc $flags -c "$f" -o "$obj"; objs+=("$obj")
done
for f in tests/product_test.cc tests/bare_main.cc "$g/src/gtest-all.cc"; do
 obj="build/$name/$(basename "$f").o"; riscv-none-elf-g++ $flags $cxxflags $extra -c "$f" -o "$obj"; objs+=("$obj")
done
riscv-none-elf-g++ $flags $cxxflags -nostartfiles -Tfreertos/link.ld -Wl,--gc-sections,-Map,build/"$name"/firmware.map "${objs[@]}" -o build/"$name"/firmware.elf
riscv-none-elf-size build/"$name"/firmware.elf
