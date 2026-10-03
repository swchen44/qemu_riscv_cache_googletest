#!/usr/bin/env python3
import json,subprocess,pathlib,datetime,sys,os
root=pathlib.Path(__file__).resolve().parent.parent
STEP_TIMEOUT=int(os.environ.get('VERIFY_TIMEOUT_SECONDS','120'))
if STEP_TIMEOUT < 1: raise ValueError('VERIFY_TIMEOUT_SECONDS must be positive')
(root/'evidence').mkdir(exist_ok=True)
results=[]
def run(name,cmd,want=0,contains=()):
 with open(root/'evidence'/f'{name}.log','w') as f:
  try: r=subprocess.run(cmd,cwd=root,stdout=f,stderr=subprocess.STDOUT,timeout=STEP_TIMEOUT);rc=r.returncode
  except subprocess.TimeoutExpired:rc=124
  f.write(f'\nHARNESS_EXIT={rc}; EXPECTED_EXIT={want}\n')
 text=(root/'evidence'/f'{name}.log').read_text()
 ok=rc==want and all(x in text for x in contains)
 results.append(dict(name=name,command=cmd,exit=rc,expected_exit=want,status='PASS' if ok else 'FAIL'))
 print(name,results[-1]['status'],flush=True)
 if not ok:print(text[-3000:]);raise RuntimeError(name)
try:
 run('host-build',['scripts/build_host.sh'])
 run('host-pass',['build/host/product_tests'],contains=['[  PASSED  ] 5 tests.'])
 run('host-negative',['build/host/product_tests','--gtest_also_run_disabled_tests','--gtest_filter=*Negative*'],1,['[  FAILED  ]'])
 for label,builder,count in [('rtos','scripts/build_rtos.sh',None),('bare-gtest','scripts/build_bare_gtest.sh',5),('rtos-gtest','scripts/build_rtos_gtest.sh',6)]:
  run(label+'-build',[builder,label])
  markers=['PASS: replay'] if count is None else [f'[  PASSED  ] {count} tests.']
  run(label+'-pass',['scripts/run_qemu.sh',f'build/{label}/firmware.elf'],contains=markers)
  run(label+'-negative-build',[builder,label+'-negative','-DINJECT_FAILURE'])
  run(label+'-negative',['scripts/run_qemu.sh',f'build/{label}-negative/firmware.elf'],1,['FAIL'])
finally:
 (root/'evidence'/'results.json').write_text(json.dumps({'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'host_profile':os.environ.get('RV32_HOST_PROFILE','debian-13'),'qemu_clock_profile':os.environ.get('RV32_QEMU_CLOCK','raw'),'harness_step_timeout_seconds':STEP_TIMEOUT,'results':results},indent=2)+'\n')
print('ALL EXPECTED POSITIVE AND NEGATIVE OUTCOMES VERIFIED')
