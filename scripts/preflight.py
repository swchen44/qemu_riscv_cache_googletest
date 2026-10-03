#!/usr/bin/env python3
"""Read-only host identification; never installs or changes system settings."""
import argparse,json,pathlib,platform,shutil,subprocess,sys
p=argparse.ArgumentParser();p.add_argument('--profile',choices=['ubuntu-24.04','debian-13'],default='ubuntu-24.04');a=p.parse_args()
osr={}
for line in pathlib.Path('/etc/os-release').read_text().splitlines():
 if '=' in line:k,v=line.split('=',1);osr[k]=v.strip('"')
want=('ubuntu','24.04') if a.profile=='ubuntu-24.04' else ('debian','13')
errors=[]
if (osr.get('ID'),osr.get('VERSION_ID'))!=want:errors.append('Host distribution/version does not match selected profile')
if platform.machine() not in ('x86_64','amd64'):errors.append('Only x86-64 host tooling is packaged')
commands={c:shutil.which(c) for c in ['gcc','g++','make','python3','bash','tar','timeout','sha256sum']}
if any(v is None for v in commands.values()):errors.append('Missing build prerequisites; use target-specific installation guide')
r={'requested_profile':a.profile,'id':osr.get('ID'),'version_id':osr.get('VERSION_ID'),'architecture':platform.machine(),'libc':platform.libc_ver(),'commands':commands,'errors':errors,'status':'PASS' if not errors else 'BLOCKED'}
print(json.dumps(r,indent=2));sys.exit(0 if not errors else 2)
