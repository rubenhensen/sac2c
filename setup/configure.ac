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

# Check if C preprocessor is available
AC_PROG_CPP()

# Enable system extensions when they are disabled by default
AC_USE_SYSTEM_EXTENSIONS

# Find out if we are using GCC/SunC/DECC/"Mac C"
CHECK_COMPILER_VENDOR

# Try to enable link time optimizations (LTO)
CHECK_LTO

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
CHECK_OMP
# enable private heap manager?
CHECK_PHM

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

# check for various environment features
CHECK_PTR_PREFIX
CHECK_CONST_STDERR
AC_CHECK_HEADERS([malloc.h])
AC_CHECK_FUNCS([mkdtemp])
AC_SEARCH_LIBS([sqrt], [m])
AC_CHECK_FUNCS([strtok strrchr])

AC_SEARCH_LIBS([clock_gettime], [rt], [dnl
  AC_DEFINE([HAVE_GETTIME], 1,
            [Define is clock_gettime from POSIX RT is available.])
])

# check support for dynamic libraries
AC_CHECK_HEADER([dlfcn.h], [], [AC_MSG_ERROR([dlfcn.h is required.])])
AC_SEARCH_LIBS([dlopen], [dl])
CHECK_DLL_EXT

dnl set rtspec flag
enable_rtspec=no
if test x"$ac_cv_search_dlopen" != xno -a x"$enable_mt" = xyes; then
  enable_rtspec=yes
fi
case $ac_cv_search_dlopen in
  no | none required) dlopen_libs= ;;
  * ) dlopen_libs=$ac_cv_search_dlopen ;;
esac
have_rtspec=`if test $enable_rtspec = yes; then echo 1; else echo 0; fi`
AC_DEFINE_UNQUOTED([ENABLE_RTSPEC], [$have_rtspec],
                   [Define to 1 if runtime specialization is enabled, otherwise 0.])
AC_SUBST([ENABLE_RTSPEC], [$enable_rtspec])


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

dnl
dnl test for compiler flags for sac2crc
dnl

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
  AC_SUBST(CPPFILE, ["$CPP -C -x c"])
  AC_SUBST(CPP, ["$CPP -P -x c"])
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
  AC_SUBST(CPPFILE, ["$CPP -C -x c"])
  AC_SUBST(CPP, ["$CPP -P -x c"])
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
  AC_SUBST(CPPFILE, ["$CPP -C -x c"])
  AC_SUBST(CPP, ["$CPP -P -x c"])
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
  AC_SUBST(CPPFILE, ["$CPP -C -x c"])
  AC_SUBST(CPP, ["$CPP -P -x c"])
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
  AC_SUBST(CPPFILE, ["$CPP -C"])
  AC_SUBST(CPP, ["$CPP -P"])
  AC_MSG_RESULT([none found])
fi   
AC_SUBST(CCMTLINK, ["$PTHREAD_LIBS"])
AC_SUBST(CCDLLINK, ["$dlopen_libs $flags_lto"])

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
  AC_SUBST(CPPFILE, ["$CPP -C -x c"])
  AC_SUBST(CPP, ["$CPP -P -x c"])
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
