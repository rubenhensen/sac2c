### Configuration

dnl Old versions of Autoconf appear to fail
AC_PREREQ([2.63])

dnl tell configure who we are
dnl parameters: package name, version, where to send bugreports
AC_INIT(sac2c, git, info@sac-home.org)

dnl set source directory and scripting directory
AC_CONFIG_SRCDIR(.)
AC_CONFIG_AUX_DIR(./config)

dnl we use C as our compiler language
AC_LANG(C)

dnl and which files to create
AC_CONFIG_FILES(../src/makefiles/config.mkf sac2crc)
AC_CONFIG_HEADER(../src/include/config.h)

dnl check for host type
AC_CANONICAL_BUILD()
AC_CANONICAL_HOST()
AC_CANONICAL_TARGET()

dnl if cygwin set IS_CYGWIN
is_cygwin=0
case "$target_os" in
  *cygwin*) is_cygwin=1 ;;
esac
AC_DEFINE_UNQUOTED([IS_CYGWIN], [$is_cygwin],
                   [Set to 1 if target is Cygwin, otherwise 0.])

dnl set OS and ARCH flags
AC_DEFINE_UNQUOTED([OS], ["$target_os"], [OS string])
AC_DEFINE_UNQUOTED([ARCH], ["$target_cpu"], [CPU architecture string])

# Enable sac2c compilation with C++
AC_ARG_ENABLE(cplusplus,
[AS_HELP_STRING([[--enable-cplusplus[=ARG]]],
                [Compile sources with C++ compiler @<:@ARG={yes,no}@:>@])],
ENABLE_CPLUSPLUS=$enableval,
ENABLE_CPLUSPLUS=no)
enable_cplusplus=
if test "${ENABLE_CPLUSPLUS}" = "yes" ; then
  enable_cplusplus=yes
fi

# Enable sac2c compilation with LTO -- (link time optimisation),
# GCC specific, triggers -flto flag for compilation and linking
AC_ARG_ENABLE(lto,
[AS_HELP_STRING([[--enable-lto[=ARG]]],
                [Perform link time optimisation (LTO) by passing -flto flag @<:@ARG={yes,no}@:>@])],
ENABLE_LTO=$enableval,
ENABLE_LTO=no)
enable_lto=
if test "${ENABLE_LTO}" = "yes" ; then
  enable_lto=yes
fi

# Check if C preprocessor is available
AC_PROG_CPP()

# Enable system extensions when they are disabled by default
AC_USE_SYSTEM_EXTENSIONS

# Find out if we are using GCC/SunC/DECC/"Mac C"
CHECK_COMPILER_VENDOR

# Check if C compiler supports SIMD syntax
CHECK_SIMD_SYNTAX

dnl check for ranlib
AC_PROG_RANLIB()

dnl check, whether we need -traditinal flag
AC_PROG_GCC_TRADITIONAL

# Check for SAC back-ends (CHECK_LPEL must appear after CHECK_MT)
CHECK_MT
CHECK_LPEL
CHECK_CUDA
CHECK_SL

# find a code beautifier.
CHECK_CODE_BEAUTIFIER

# search for lex/flex/yacc/bison
CHECK_LEX_YACC

# search/check various utilities
CHECK_REQUIRED_TOOL([SORT], [sort])
CHECK_REQUIRED_TOOL([UNIQ], [uniq])
CHECK_REQUIRED_TOOL([M4], [gm4 m4])
CHECK_REQUIRED_TOOL([XSLT], [xsltproc sabcmd])

# search for dot
CHECK_GRAPHVIZ

dnl search for malloc.h
AC_CHECK_HEADERS(malloc.h)

dnl check for mkdtemp
AC_CHECK_FUNCS(mkdtemp)

dnl Set the file extension for shared libs to either .dll for windows or .so
AC_MSG_CHECKING([for shared lib file extension])
case "$target_os" in
  *cygwin* | *windows* | *mingw*)
    AC_SUBST(SHARED_LIB_EXT, [".dll"])
    AC_DEFINE(SHARED_LIB_EXT, ".dll", [File extension used for shared library files.])
    AC_MSG_RESULT([.dll])
    ;;
  *darwin*)
    AC_SUBST(SHARED_LIB_EXT, [".dylib"])
    AC_DEFINE(SHARED_LIB_EXT, ".dylib", [File extension used for shared library files.])
    AC_MSG_RESULT([.dylib])
    ;;
  *)
    AC_SUBST(SHARED_LIB_EXT, [".so"])  
    AC_DEFINE(SHARED_LIB_EXT, ".so", [File extension used for shared library files.])
    AC_MSG_RESULT([.so])
    ;;
esac

dnl search for omp.h
AC_CHECK_HEADER(omp.h,[

dnl if found, enable OpenMP backend
AC_DEFINE(HAVE_OMP_H, 1, set to 1 because you have the <omp.h> header file)
AC_DEFINE(ENABLE_OMP, 1, set to 1 to enable OpenMP backend)
AC_SUBST(ENABLE_OMP, yes)

],[

dnl otherwise disable OpenMP backend
AC_DEFINE(HAVE_OMP_H, 0, set to 0 because you do not have the <omp.h> header file)
AC_DEFINE(ENABLE_OMP, 0, set to 0 to disable OpenMP backend)
AC_SUBST(ENABLE_OMP, no)

])

dnl search for dlfcn.h
AC_CHECK_HEADER(dlfcn.h,[

dnl if found, enable dynamic linking
AC_DEFINE(HAVE_DLFCN_H, 1, set to 1 because you have the <dlfcn.h> header file)
AC_DEFINE(ENABLE_DL, 1, set to 1 to enable dynamic linking)
AC_SUBST(ENABLE_DL, yes)
],[

dnl otherwise disable dynamic linking
AC_DEFINE(HAVE_DLFCN_H, 0, set to 0 because you do not have the <dlfcn.h> header file)
AC_DEFINE(ENABLE_DL, 0, set to 0 to disable dynamic linking)
AC_SUBST(ENABLE_DL, no)

])

dnl set rtspec flag
if [[ "$ENABLE_DL"="yes" -a "$ENABLE_MT"="yes" ]]; then
  AC_DEFINE(ENABLE_RTSPEC, 1, runtime specialization enabled)
  AC_SUBST(ENABLE_RTSPEC, yes)
else
  AC_DEFINE(ENABLE_RTSPEC, 0, runtime specialization disabled)
  AC_SUBST(ENABLE_RTSPEC, no)
fi

dnl check whether to link math library
AC_SEARCH_LIBS(sqrt, m)

dnl check whether to link dlopen library
AC_SEARCH_LIBS(dlopen, dl)

dnl check for timing support (could also be included in libc)
AC_SEARCH_LIBS(clock_gettime, rt, [
AC_DEFINE(HAVE_GETTIME, 1, set to 1 because you have clock_gettime) 
])

dnl checking for presence of strtok and strrchr
AC_CHECK_FUNCS([strtok strrchr])

dnl check for sbrk function
AC_CHECK_FUNC(sbrk, [

dnl if found, check type check definition of sbrk
AC_MSG_CHECKING(for sbrk argument type)
include_file_list=`ls -1 /usr/include/*.h`
sbrk_types=`for name in $include_file_list; do grep sbrk $name 2>/dev/null; done`
[sbrk_types=`echo "$sbrk_types" | sed -e 's/[A-Za-z0-9 _*]*([ ]*//g' | sed -e 's/)[A-Za-z0-9_* ;]*//g' | sed -e 's/ [A-Za-z0-9_* ]*//g'` ]
[ for name in $sbrk_types; do ]
 AC_TRY_COMPILE([#include <unistd.h>],[sbrk(($name) 0)],break,name=none)
[ done ]

[ if test $name = none ; then ]
AC_DEFINE(ENABLE_PHM, 0)
AC_SUBST(ENABLE_PHM, no)
AC_MSG_RESULT(not found)
[ else ]
AC_DEFINE_UNQUOTED(SBRK_T,$name,set to the type of sbrk argument)
AC_MSG_RESULT($name)
AC_SUBST(ENABLE_PHM, yes)
AC_DEFINE(ENABLE_PHM, 1)
[ fi ]
],[

dnl otherwise disable pm
AC_DEFINE(ENABLE_PHM, 1, set to disable private heapmanagement)
AC_SUBST(ENABLE_PHM, yes)
])


dnl check if printf generates leading 0x for pointers
AC_MSG_CHECKING(for 0x prefix when printing pointers)
AC_EGREP_CPP(0x0*,
[#include <stdio.h>
int main()
{
printf("%p", (void*) 0);
}
], has_ptr_prefix=yes, has_ptr_prefix=no)
if [ test $has_ptr_prefix = yes ]; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_DEFINE(NEED_PTR_PREFIX, 1, set to 1 if printf does not print a leading
0x in front of pointers)
fi


dnl check if stderr is a constant (needed for dbug.c
AC_MSG_CHECKING(whether stderr is a constant)
AC_TRY_COMPILE([
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

FILE *_db_fp_ = stderr;
],[],stderr_is_constant=yes,stderr_is_constant=no)
if [ test $stderr_is_constant = yes ]; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(STDERR_IS_CONSTANT, 1, set to 1 if stderr is a constant)
else
  AC_MSG_RESULT(no)
fi


# Check which GCC warning options are available and 
# add them later to MKCCFLAGS and PDCCFLAGS
gcc_options=
if test "$GCC" = yes ; then
  CHECK_CC_OPTION([-Wall], [gcc_options])
  CHECK_CC_OPTION([-Wextra], [gcc_options])
  CHECK_CC_OPTION([-Wstrict-prototypes], [gcc_options])
  CHECK_CC_OPTION([-Wno-unused-parameter], [gcc_options])
  CHECK_CC_OPTION([-Wno-unused-but-set-variable], [gcc_options])
fi

# Check if GCC compiler supports lto, and if LTO is enabled
# add -flto to the compilation flags.
flags_lto=
if [[ "$GCC" = yes -a "$enable_lto" = yes ]]; then
  CHECK_CC_OPTION([-flto], [flags_lto])
  if test "x$flags_lto" = "x"; then
    AC_MSG_NOTICE([Diabling compilation with LTO, as chosen compiler does not support -flto option.])
  fi
fi


dnl test for compiler flags for sac2crc
AC_MSG_CHECKING([sac2crc compiler flags])
if [ test "$GCC" = yes ]; then
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-O1])
  AC_SUBST(OPT_O2, [-O2])
  AC_SUBST(OPT_O3, [-O3])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, ["-Wall -Wno-unused -fno-builtin -std=c99 $flags_lto"])  
  AC_SUBST(MKCCFLAGS, ["$gcc_options -pedantic -g -std=c99 $flags_lto"])
  AC_SUBST(PDCCFLAGS, ["$gcc_options -pedantic -O3 -std=c99 $flags_lto"])
  if [ test "$IS_CYGWIN" = yes ]; then
    AC_SUBST(GENPIC, ["-DPIC"])
  else
    AC_SUBST(GENPIC, ["-fPIC -DPIC"])
  fi  
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["gcc -E -C -x c"])
  AC_SUBST(CPP, ["gcc -E -P -x c"])
  AC_SUBST(CCDLLINK, ["-ldl $flags_lto"])
  AC_MSG_RESULT([gcc settings])
elif [ test "$SUNC" = yes ]; then
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-xO2])
  AC_SUBST(OPT_O2, [-xO4])
  AC_SUBST(OPT_O3, [-xO5])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, ["-dalign -fsimple -xsafe=mem -xc99=all"])
  AC_SUBST(MKCCFLAGS, ["-erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xc99=all"])
  AC_SUBST(PDCCFLAGS, ["-erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xO4 -xc99=all -KPIC"])
  AC_SUBST(GENPIC, ["-KPIC"])
  AC_SUBST(DEPSFLAG, ["-xM"])
  AC_SUBST(CPPFILE, ["cpp -E -C -x c"])
  AC_SUBST(CPP, ["cpp -E -P -x c"])
  AC_SUBST(CCDLLINK, ["-ldl"])
  AC_MSG_RESULT([Sun cc settings])
elif [ test "$DECC" = yes ]; then
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-O1])
  AC_SUBST(OPT_O2, [-O2])
  AC_SUBST(OPT_O3, [-O3])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, [""])
  AC_SUBST(MKCCFLAGS, ["-g"])
  AC_SUBST(PDCCFLAGS, ["-g3"])
  AC_SUBST(GENPIC, [""])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["cc -E -C -x c"])
  AC_SUBST(CPP, ["cc -E -P -x c"])
  AC_SUBST(CCDLLINK, ["-ldl"])
  AC_MSG_RESULT([Compaq/DEC cc settings])
elif [ test "$MACC" = yes ]; then
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-O1])
  AC_SUBST(OPT_O2, [-O2])
  AC_SUBST(OPT_O3, [-O3])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, ["-Wall -Wno-unused -fno-builtin -std=c99"])
  AC_SUBST(MKCCFLAGS, ["-Wall -std=c99 -g"])
  AC_SUBST(PDCCFLAGS, ["-std=c99"])
  AC_SUBST(GENPIC, [""])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["gcc -E -C -x c"])
  AC_SUBST(CPP, ["gcc -E -P -x c"])
  AC_SUBST(CCDLLINK, ["-ldl"])
  AC_MSG_RESULT([Apple cc settings])
else
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [""])
  AC_SUBST(OPT_O2, [""])
  AC_SUBST(OPT_O3, [""])
  AC_SUBST(OPT_g, [""])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, [""])
  AC_SUBST(MKCCFLAGS, [""])
  AC_SUBST(PDCCFLAGS, [""])
  AC_SUBST(GENPIC, [""])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["cpp -C"])
  AC_SUBST(CPP, ["cpp -P"])
  AC_SUBST(CCDLLINK, [""])
  AC_MSG_RESULT([none found])
fi   
AC_SUBST(CCMTLINK, ["$PTHREAD_LIBS"])

# This is a special case for building sac2c with c++ compiler.
if [[ "$GXX" = yes -a "$enable_cplusplus" = yes ]]; then
  AC_SUBST(SAC2C_CC, ["g++"])
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-O1])
  AC_SUBST(OPT_O2, [-O2])
  AC_SUBST(OPT_O3, [-O3])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, ["-Wall -Wno-unused -fno-builtin -std=c99"])
  AC_SUBST(SAC2C_MKCCFLAGS, ["-Wno-write-strings -g -std=c++98"])
  AC_SUBST(SAC2C_PDCCFLAGS, ["-Wno-write-strings -O3 -std=c++98"])
  AC_SUBST(GENPIC, ["-fPIC -DPIC"])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["gcc -E -C -x c"])
  AC_SUBST(CPP, ["gcc -E -P -x c"])
  AC_SUBST(CCDLLINK, ["-ldl"])
  AC_MSG_RESULT([gcc settings])
else
  AC_SUBST(SAC2C_CC, ["$CC"])
  AC_SUBST(SAC2C_MKCCFLAGS, ["$MKCCFLAGS"])
  AC_SUBST(SAC2C_PDCCFLAGS, ["$PDCCFLAGS"])
fi
 
# append platform specific files to sac2crc
AC_SUBST_FILE(RCSUN)
AC_SUBST_FILE(RCX86)
AC_SUBST_FILE(RCALPHA)
AC_SUBST_FILE(RCMAC)
RCSUN=./sac2crc.SUN
RCX86=./sac2crc.X86
RCALPHA=./sac2crc.ALPHA
RCMAC=./sac2crc.MAC


# generate specific flags for known os
AC_MSG_CHECKING([sac2crc system flags])

OSFLAGS=""
case "$target_os" in
  solaris*)     
    OSFLAGS=["-D__EXTENSIONS__ -D_XPG6 -DMUST_INIT_YY -DPIC"]
    LD_DYNAMIC=["-G -dy"]
    LD_PATH=["-L%path% -R%path%"]
    LD_FLAGS="-Wl,-z,nodefs,-z,lazyload"
    AC_MSG_RESULT([solaris settings])
    ;;
  *linux*)      
    OSFLAGS=["-fPIC -DPIC -D_POSIX_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE"]
    LD_DYNAMIC=["-shared -Wl,-allow-shlib-undefined -O3 $flags_lto"]
    LD_PATH=["-L%path% -Wl,-rpath,%path%"]
    LD_FLAGS="-Wl,-allow-shlib-undefined $flags_lto"
    AC_MSG_RESULT([linux settings])
    ;;
 *cygwin* | *mingw*)      
    OSFLAGS=["-D_POSIX_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE"]
    LD_DYNAMIC=["-dy -shared"]
    LD_PATH=["-L%path% -Wl,-rpath,%path%"]
    LD_FLAGS=""
    AC_MSG_RESULT([cygwin settings])
    ;;
  *osf*)	
    OSFLAGS=["-D_OSF_SOURCE"]
    LD_DYNAMIC=[""]
    LD_PATH=["-L%path%"]
    LD_FLAGS=""
    AC_MSG_RESULT([OSF settings])
    ;;
  *darwin*)      
    dnl darwin needs some extra flags
    AC_DEFINE(ENABLE_PHM, 0)
    AC_SUBST(ENABLE_PHM, no)
    RANLIB="$RANLIB -c"
    OSFLAGS=["-D_DARWIN_C_SOURCE"]
    LD_DYNAMIC=["-undefined suppress -flat_namespace -dynamiclib -install_name '@rpath/%libname%' "]
    LD_PATH=["-L%path% -Wl,-rpath,%path%"]
    LD_FLAGS=""
    AC_MSG_RESULT([darwin settings])
    ;;
  *bsd*)
    OSFLAGS=["-fpic"]
    LD_DYNAMIC=["ld -dy -shared -symbolic"]
    LD_PATH=["-L -Wl,-rpath,%path%"]
    LD_FLAGS=""
    AC_MSG_RESULT([BSD settings])
    ;;
  *)
    OSFLAGS=["-fPIC -DPIC -D_POSIX_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE -Dlint"]
    LD_DYNAMIC=["-dy -shared -Wl,-allow-shlib-undefined"]
    LD_PATH=["-L%path% -Wl,-rpath,%path%"]
    LD_FLAGS="-Wl,-allow-shlib-undefined"
    AC_MSG_RESULT([unknown])
    ;;
esac


AC_SUBST(OS,$target_os)
AC_SUBST(ARCH,$target_cpu)
AC_SUBST(OSFLAGS)
AC_SUBST(LD_DYNAMIC)
AC_SUBST(LD_PATH)
AC_SUBST(LD_FLAGS)
 

# static sac2c settings for config.h
AC_DEFINE( PF_MAXFUN, 100, [set to maximum number of sac functions for profiling])
AC_DEFINE( PF_MAXFUNAP, 100, 
           [set to maximum number of sac function applications for profiling])
AC_DEFINE( PF_MAXFUNNAMELEN, 100, 
           [set to maximum length of sac function names for profiling])

AC_DEFINE( SAC_PRELUDE_NAME, "sacprelude", [name of sac prelude])
AC_DEFINE( UNUSED, __attribute__((unused)), define macro for unused variables)


# create configure-generated file
AC_OUTPUT()
