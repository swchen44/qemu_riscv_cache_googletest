#!/usr/bin/env python3
import concurrent.futures,os,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parent.parent;os.chdir(root)
profile=sys.argv[1] if len(sys.argv)>1 else 'host';negative='negative' in sys.argv[2:]
base=root.parent/'rv32_gtest_poc';target=profile!='host';name=profile+('-negative' if negative else '')
build=root/'build'/name;build.mkdir(parents=True,exist_ok=True)
tool=base/'tools/xpack-riscv-none-elf-gcc-15.2.0-1/bin';cc=str(tool/'riscv-none-elf-gcc') if target else 'gcc';cxx=str(tool/'riscv-none-elf-g++') if target else 'g++'
g=base/'vendor/googletest-1.15.2/googletest';embed=root/'generated/micropython_embed'
flags=['-O2','-g','-DNDEBUG','-ffunction-sections','-fdata-sections','-Isrc','-I'+str(embed),'-I'+str(embed/'port'),'-I'+str(g/'include'),'-I'+str(g),'-I'+str(base/'vendor/fff-1.1')]
if target: flags+=['-march=rv32imac_zicsr','-mabi=ilp32','-mcmodel=medany','-DRV32_BARE_METAL']
cxxflags=['-std=c++14']
if target:cxxflags+=['-D_POSIX_C_SOURCE=200809L','-fno-exceptions','-fno-rtti','-DGTEST_HAS_PTHREAD=0','-DGTEST_HAS_EXCEPTIONS=0','-DGTEST_HAS_RTTI=0','-DGTEST_HAS_POSIX_RE=0','-DGTEST_HAS_STREAM_REDIRECTION=0','-DGTEST_HAS_FILE_SYSTEM=0']
else:cxxflags+=['-pthread']
if negative:cxxflags+=['-DINJECT_FAILURE']
cfiles=sorted(embed.glob('*/*.c'))+sorted(embed.glob('*/*/*.c'))
cfiles=[p for p in cfiles if p.name!='mphalport.c']+[root/'src/interpreter_bridge.c']
if target:cfiles+=[root/'freertos/syscalls.c',root/'freertos/start.S']
cppfiles=[root/'tests/interpreter_test.cc',root/'tests/main.cc',g/'src/gtest-all.cc']
objs=[];jobs=[]
for idx,f in enumerate(cfiles+cppfiles):
 obj=build/(str(idx)+'_'+f.name+'.o');objs.append(str(obj))
 cmd=[cxx if f.suffix=='.cc' else cc]+flags+(cxxflags if f.suffix=='.cc' else ['-std=gnu99'])+['-c',str(f),'-o',str(obj)]
 jobs.append(cmd)
def run(cmd):subprocess.run(cmd,check=True)
with concurrent.futures.ThreadPoolExecutor(4) as pool:list(pool.map(run,jobs))
out=build/('firmware.elf' if target else 'micropython_tests')
link=[cxx]+flags+cxxflags+objs+['-Wl,--gc-sections','-o',str(out)]
if target:link+=['-nostartfiles','-Tfreertos/link.ld','-Wl,-Map,'+str(build/'firmware.map')]
run(link)
print('COMPILED_C_SOURCES='+str(len(cfiles)));print('OUTPUT='+str(out))
if target:run([str(tool/'riscv-none-elf-size'),str(out)])
