#!/usr/bin/env python3
"""Online preload on an approved machine. Exact URLs and SHA256; no installation."""
import argparse,pathlib,json,hashlib,subprocess
p=argparse.ArgumentParser();p.add_argument('--bundle-root',type=pathlib.Path,required=True);p.add_argument('--include-tcg',action='store_true',help='Also fetch the optional ~254 MiB Ubuntu VM image and host VM tools');p.add_argument('--include-all-sources',action='store_true',help='Also fetch every exact Ubuntu dependency source package');p.add_argument('--include-base',action='store_true',help='Fetch the optional original Ubuntu Base archive for static inspection');a=p.parse_args()
B=a.bundle_root.resolve();B.mkdir(parents=True,exist_ok=True)
items=json.loads((B/'ubuntu-packages.lock.json').read_text())['packages']
if a.include_base:items += [x for x in json.loads((B/'ubuntu-source-image.lock.json').read_text()) if 'ubuntu-base-' in x['path']]
if a.include_tcg:items+=json.loads((B/'tcg-downloads.lock.json').read_text())
if a.include_all_sources:
 for source in json.loads((B/'ubuntu-corresponding-sources.lock.json').read_text())['sources']:items+=source['files']
for d in items:
 dest=B/d['path'];assert dest.resolve().is_relative_to(B)
 dest.parent.mkdir(parents=True,exist_ok=True)
 if not dest.is_file():
  part=dest.with_name(dest.name+'.part')
  subprocess.run(['curl','-fL','--retry','2','--max-time','1800',d['url'],'-o',str(part)],check=True)
  assert hashlib.file_digest(part.open('rb'),'sha256').hexdigest()==d['sha256'],f'Hash mismatch: {part}'
  part.replace(dest)
 assert hashlib.file_digest(dest.open('rb'),'sha256').hexdigest()==d['sha256'],f'Hash mismatch: {dest}'
 print('VERIFIED',d['path'],flush=True)
