#!/usr/bin/env python3
"""Extract only the locked shared source and xPack inputs; no network or Debian QEMU."""
import argparse, hashlib, json, pathlib, subprocess
p=argparse.ArgumentParser();p.add_argument('projects',type=pathlib.Path);a=p.parse_args()
r=a.projects.resolve()/'rv32_gtest_poc'
selected={'vendor/googletest-v1.15.2.tar.gz':'vendor','vendor/fff-v1.1.tar.gz':'vendor','vendor/FreeRTOS-Kernel-V11.1.0.tar.gz':'vendor','tools/xpack-riscv-none-elf-gcc-15.2.0-1-linux-x64.tar.gz':'tools'}
lock={x['path']:x for x in json.loads((r/'dependencies.lock.json').read_text())['downloads']}
for path,dest in selected.items():
 f=r/path
 assert hashlib.file_digest(f.open('rb'),'sha256').hexdigest()==lock[path]['sha256'],f'Hash mismatch: {f}'
 subprocess.run(['tar','-xzf',str(f),'-C',str(r/dest)],check=True)
 print('VERIFIED and EXTRACTED',path)
