#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/env.sh"
cd "$ROOT"
g=vendor/googletest-1.15.2/googletest
flags="-march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medany -O2 -g -ffunction-sections -fdata-sections -I$g/include -I$g"
cxxflags="-std=c++14 -D_POSIX_C_SOURCE=200809L -fno-exceptions -fno-rtti -DGTEST_HAS_PTHREAD=0 -DGTEST_HAS_EXCEPTIONS=0 -DGTEST_HAS_RTTI=0 -DGTEST_HAS_POSIX_RE=0 -DGTEST_HAS_STREAM_REDIRECTION=0 -DGTEST_HAS_FILE_SYSTEM=0"
for mode in runtime_only gtest_empty; do
 b="build/cost-$mode"; mkdir -p "$b"
 objs=()
 for f in freertos/start.S freertos/syscalls.c; do
  o="$b/$(basename "$f").o"; riscv-none-elf-gcc $flags -c "$f" -o "$o";objs+=("$o")
 done
 o="$b/control.o";riscv-none-elf-g++ $flags $cxxflags -c "diagnostics/$mode.cc" -o "$o";objs+=("$o")
 if [ "$mode" = gtest_empty ]; then
  o="$b/gtest-all.o";riscv-none-elf-g++ $flags $cxxflags -c "$g/src/gtest-all.cc" -o "$o";objs+=("$o")
 fi
 riscv-none-elf-g++ $flags $cxxflags -nostartfiles -Tfreertos/link.ld -Wl,--gc-sections,-Map,"$b/firmware.map" "${objs[@]}" -o "$b/firmware.elf"
 riscv-none-elf-size "$b/firmware.elf"
 scripts/run_qemu.sh "$b/firmware.elf" > "evidence/cost-$mode-run.log" 2>&1
 riscv-none-elf-objcopy -O binary "$b/firmware.elf" "$b/firmware.bin"
 riscv-none-elf-strip -o "$b/firmware.stripped.elf" "$b/firmware.elf"
done
printf '#include <gtest/gtest.h>\n' | riscv-none-elf-g++ $flags $cxxflags -dM -E -x c++ - | grep -E 'GTEST_HAS_(PTHREAD|EXCEPTIONS|RTTI|POSIX_RE|STREAM_REDIRECTION|FILE_SYSTEM|DEATH_TEST)|GTEST_IS_THREADSAFE' > evidence/gtest-effective-feature-macros.txt
python3 - <<'PY'
import json,pathlib,subprocess
paths=['build/cost-runtime_only/firmware.elf','build/cost-gtest_empty/firmware.elf','build/bare-gtest/firmware.elf','build/rtos/firmware.elf','build/rtos-gtest/firmware.elf','../micropython_gtest_poc/build/rv32/firmware.elf']
rows=[]
for p in paths:
 vals=subprocess.check_output(['riscv-none-elf-size',p],text=True).splitlines()[1].split()
 syms={}
 for l in subprocess.check_output(['riscv-none-elf-nm',p],text=True).splitlines():
  v=l.split()
  if len(v)==3 and v[2] in ['__heap_start','__heap_end','__stack_top']:syms[v[2]]=int(v[0],16)
 rows.append(dict(path=p,text=int(vals[0]),data=int(vals[1]),bss=int(vals[2]),elf_disk_bytes=pathlib.Path(p).stat().st_size,configured_sbrk_capacity=syms['__heap_end']-syms['__heap_start'],configured_startup_stack_reserve=syms['__stack_top']-syms['__heap_end']))
delta={k:rows[1][k]-rows[0][k] for k in ['text','data','bss']}
json.dump(dict(note='Measured retained image delta: empty GoogleTest plus additionally required C++/C runtime, NOT GoogleTest source alone; no tests/FFF in control.',rows=rows,empty_gtest_minus_runtime=delta),open('evidence/framework-cost.json','w'),indent=2)
print(json.dumps(rows,indent=2));print('EMPTY_GTEST_INCREMENT',delta)
PY
