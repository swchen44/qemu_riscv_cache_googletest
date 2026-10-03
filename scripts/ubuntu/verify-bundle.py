#!/usr/bin/env python3
"""Read-only verification of an explicitly selected unpacked Ubuntu bundle."""
import argparse,hashlib,json,pathlib
p=argparse.ArgumentParser();p.add_argument('--bundle-root',type=pathlib.Path,required=True);p.add_argument('--include-probes',action='store_true');a=p.parse_args()
B=a.bundle_root.resolve();lock=json.loads((B/'ubuntu-packages.lock.json').read_text());items=list(lock['packages'])
if a.include_probes:items+=lock.get('probe_packages',[])
for item in items:
 f=(B/item['path']).resolve();assert f.is_relative_to(B),f'Path escapes bundle: {f}'
 assert f.is_file(),f'Missing {f}'
 assert hashlib.file_digest(f.open('rb'),'sha256').hexdigest()==item['sha256'],f'Hash mismatch: {f}'
print(f'OK: {len(items)} Ubuntu package files match locked SHA256 values')
