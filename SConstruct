from glob import glob

ccflags=["-Wall", "-Werror", "-DNDEBUG", "-O3"]
#ccflags=["-Wall", "-Werror", "-O3"]

import os
uname = os.uname()
if uname[0] == 'Darwin' and uname[4] == 'i386':
    ccflags.append('-mdynamic-no-pic')

env = Environment(CCFLAGS=ccflags,
                  CPPPATH='..')

cfiles  = glob('*.c') + glob('gcd/*.c')
ccfiles = glob('*.cc') + glob('gcd/*.cc')

env.StaticLibrary(target='optarith',
                  source=cfiles)
    
env.StaticLibrary(target='optarithxx',
                  source=cfiles + ccfiles)

SConscript(dirs=['tests', 'timing'])

