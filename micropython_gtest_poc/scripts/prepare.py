#!/usr/bin/env python3
import hashlib,json,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parent.parent
lock=json.loads((root/'dependencies.lock.json').read_text())
base=root.parent/'rv32_gtest_poc'
assert (base/'dependencies.lock.json').is_file(),'Place the first-stage rv32_gtest_poc archive beside this directory'
assert hashlib.sha256((base/'dependencies.lock.json').read_bytes()).hexdigest()==lock['reuse_lock_sha256'],'First-stage dependency lock mismatch'
p=root/lock['micropython']['archive']
assert hashlib.file_digest(open(p,'rb'),'sha256').hexdigest()==lock['micropython']['sha256'],'MicroPython source SHA256 mismatch'
subprocess.run(['tar','-xzf',str(p),'-C',str(root/'vendor')],check=True)
# Config-keyed directory prevents stale qstr/module registration fragments after profile changes.
config_hash=hashlib.sha256((root/'src/mpconfigport.h').read_bytes()).hexdigest()[:16]
subprocess.run(['make','-B','-f','micropython_embed.mk','BUILD=../buildgen/'+config_hash],cwd=root/'src',check=True)
