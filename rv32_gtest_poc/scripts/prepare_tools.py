#!/usr/bin/env python3
"""Download only immutable locked dependencies; --offline never uses network."""
import argparse,hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parent.parent
p=argparse.ArgumentParser();p.add_argument('--offline',action='store_true');a=p.parse_args()
for d in json.loads((root/'dependencies.lock.json').read_text())['downloads']:
 dest=root/d['path'];dest.parent.mkdir(parents=True,exist_ok=True)
 if not dest.exists():
  if a.offline:sys.exit(f'MISSING offline dependency: {d["path"]}')
  subprocess.run(['curl','-fL','--retry','2','--max-time','900',d['url'],'-o',str(dest)],check=True)
 actual=hashlib.file_digest(open(dest,'rb'),'sha256').hexdigest()
 if actual!=d['sha256']:sys.exit('SHA256 MISMATCH: '+d['path'])
 print('VERIFIED',d['path'],flush=True)
# Tarballs are official locked trusted sources. Do not run this script against an unreviewed lock file.
for pat,directory in [('vendor/googletest-v1.15.2.tar.gz','vendor'),('vendor/fff-v1.1.tar.gz','vendor'),('vendor/FreeRTOS-Kernel-V11.1.0.tar.gz','vendor'),('tools/xpack-riscv-none-elf-gcc-15.2.0-1-linux-x64.tar.gz','tools')]:
 subprocess.run(['tar','-xzf',str(root/pat),'-C',str(root/directory)],check=True)
for d in json.loads((root/'dependencies.lock.json').read_text())['downloads']:
 if d['path'].startswith('tools/debs/') and d['path'].endswith('.deb'):
  subprocess.run(['dpkg-deb','-x',str(root/d['path']),str(root/'tools/qemu-root')],check=True)
print('Prepared project-private tools. No system installation was performed.')
