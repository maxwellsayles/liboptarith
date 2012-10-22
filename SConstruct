from glob import glob

ccflags=["-Wall", "-Werror", "-DNDEBUG", "-O3"]

import os
uname = os.uname()
if uname[0] == 'Darwin' and uname[4] == 'i386':
    ccflags.append('-mdynamic-no-pic')

env = Environment(CCFLAGS=ccflags,
                  CPPPATH='..')

env.StaticLibrary(target='optarith',
                  source=glob('*.c'))
    
env.StaticLibrary(target='optarithxx',
                  source=glob('*.c') + glob('*.cc'))

SConscript('tests/SConstruct')