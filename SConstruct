from glob import glob
import os
import os.path

ccflags=["-Wall", "-Werror", "-DNDEBUG", "-O3"]

# Check if OsX (i386)
uname = os.uname()
if uname[0] == 'Darwin' and uname[4] == 'i386':
    ccflags.append('-mdynamic-no-pic')

# Check if PARI available
hasPari = os.path.exists('/usr/include/pari/pari.h') or \
          os.path.exists('/usr/local/include/pari/pari.h')

if not hasPari:
    ccflags += ['-DNO_PARI']

env = Environment(CCFLAGS=ccflags,
                  CPPPATH='..')

cfiles  = glob('*.c') + glob('gcd/*.c')
ccfiles = glob('*.cc') + glob('gcd/*.cc')

if not hasPari:
    cfiles.remove('gcd/gcd_pari.c')

env.StaticLibrary(target='optarith',
                  source=cfiles)
    
env.StaticLibrary(target='optarithxx',
                  source=cfiles + ccfiles)

SConscript(dirs=['tests', 'timing'])

