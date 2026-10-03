#!/usr/bin/env python3
import pathlib,json,hashlib,argparse
B=pathlib.Path(__file__).resolve().parent.parent
p=argparse.ArgumentParser();p.add_argument('--include-probes',action='store_true');a=p.parse_args()
lock=json.loads((B/'ubuntu-packages.lock.json').read_text());items=lock['packages']
if a.include_probes:items+=lock.get('probe_packages',[])
for item in items:
 p=B/item['path'];assert p.is_file(),f'Missing {p}'
 assert hashlib.file_digest(p.open('rb'),'sha256').hexdigest()==item['sha256'],f'Hash mismatch: {p}'
print(f'OK: {len(items)} Ubuntu package files match the locked SHA256 values')
