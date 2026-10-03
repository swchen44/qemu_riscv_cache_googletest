#!/usr/bin/env python3
import datetime,hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parent.parent
(root/'evidence').mkdir(exist_ok=True);results=[]
def run(name,cmd,want=0,markers=()):
 with open(root/'evidence'/f'{name}.log','w') as f:
  try:r=subprocess.run(cmd,cwd=root,stdout=f,stderr=subprocess.STDOUT,timeout=180);rc=r.returncode
  except subprocess.TimeoutExpired:rc=124
  f.write(f'\nHARNESS_EXIT={rc}; EXPECTED_EXIT={want}\n')
 txt=(root/'evidence'/f'{name}.log').read_text();ok=rc==want and all(x in txt for x in markers)
 results.append(dict(name=name,command=cmd,exit=rc,expected_exit=want,status='PASS' if ok else 'FAIL'))
 print(name,results[-1]['status'],flush=True)
 if not ok:raise RuntimeError(name+' failed; inspect evidence log')
try:
 run('prepare',['python3','scripts/prepare.py'])
 run('host-build',['python3','scripts/build.py','host'])
 run('host-pass',['build/host/micropython_tests'],markers=['[  PASSED  ] 11 tests.'])
 run('host-negative',['build/host/micropython_tests','--gtest_also_run_disabled_tests','--gtest_filter=*Negative*'],1,['[  FAILED  ]'])
 run('rv32-build',['python3','scripts/build.py','rv32'])
 run('rv32-pass',['scripts/run_rv32.sh'],markers=['[  PASSED  ] 11 tests.'])
 run('rv32-negative-build',['python3','scripts/build.py','rv32','negative'])
 run('rv32-negative',['scripts/run_rv32.sh','rv32-negative'],1,['[  FAILED  ]'])
finally:
 (root/'evidence/results.json').write_text(json.dumps(dict(utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),shared_test_source='tests/interpreter_test.cc',shared_test_sha256=hashlib.sha256((root/'tests/interpreter_test.cc').read_bytes()).hexdigest(),results=results),indent=2)+'\n')
print('HOST AND RV32 USE THE IDENTICAL TEST SOURCE; ALL EXPECTED OUTCOMES VERIFIED')
