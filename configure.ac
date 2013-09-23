### Configuration

dnl Old versions of Autoconf appear to fail
AC_PREREQ([2.63])

dnl tell configure who we are
dnl parameters: package name, version, where to send bugreports
AC_INIT(sac2c, git, info@sac-home.org)

dnl set source directory and scripting directory
AC_CONFIG_SRCDIR([setup])
AC_CONFIG_AUX_DIR([setup/config])

dnl we use C as our compiler language
AC_LANG(C)

dnl and which files to create
AC_CONFIG_FILES([src/makefiles/config.mkf setup/sac2crc])
AC_CONFIG_HEADER([src/include/config.h])

dnl check for host type
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

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

dnl get prefix and save it in the config.h file
AH_VERBATIM([PREFIX],                  [#undef PREFIX])
if test "x$prefix" = "xNONE" ; then
    prefix="$ac_default_prefix"
fi
AC_DEFINE_UNQUOTED(PREFIX, "${prefix}")
AC_SUBST([PREFIX], ["${prefix}"])

dnl Enable sac2c compilation with C++
AC_ARG_ENABLE([cplusplus],
              [AS_HELP_STRING([[--enable-cplusplus]],
                               [Use the C++ compiler to compile files produced by sac2c])],
              [enable_cplusplus=$enableval],
              [enable_cplusplus=no])

dnl Check if C compiler and preprocessor are available
dnl we force CFLAGS to be empty, so that it does not default to -g -O2
CFLAGS=" "
AC_PROG_CC_C99
AC_PROG_CPP

dnl Enable system extensions when they are disabled by default
AC_USE_SYSTEM_EXTENSIONS

dnl Find out if we are using GCC/SunC/DECC/"Mac C"
CHECK_COMPILER_VENDOR

dnl Try to enable link time optimizations (LTO)
CHECK_LTO

dnl Check if C compiler supports SIMD syntax
CHECK_SIMD_SYNTAX

dnl Check for SAC back-ends (CHECK_LPEL must appear after CHECK_MT)
CHECK_MT
CHECK_LPEL
CHECK_CUDA
CHECK_SL
CHECK_OMP
dnl enable private heap manager?
CHECK_PHM

dnl check for ranlib
AC_PROG_RANLIB

dnl find a code beautifier.
CHECK_CODE_BEAUTIFIER

dnl search/check various utilities
CHECK_REQUIRED_TOOL([SORT], [sort])
CHECK_REQUIRED_TOOL([UNIQ], [uniq])
CHECK_REQUIRED_TOOL([M4], [gm4 m4])
CHECK_REQUIRED_TOOL([XSLT], [xsltproc sabcmd])
CHECK_REQUIRED_TOOL([BASH], [bash])

dnl search for dot
CHECK_GRAPHVIZ

dnl check for various environment features
CHECK_PTR_PREFIX
CHECK_CONST_STDERR
AC_CHECK_HEADERS([malloc.h])
AC_CHECK_FUNCS([mkdtemp])
AC_SEARCH_LIBS([sqrt], [m])
AC_CHECK_FUNCS([strtok strrchr])
CHECK_HR_CLOCK

dnl check support for dynamic libraries
AC_CHECK_HEADER([dlfcn.h], [], [AC_MSG_ERROR([dlfcn.h is required.])])
AC_SEARCH_LIBS([dlopen], [dl])
CHECK_DLL_EXT

dnl set rtspec flag
enable_rtspec=no
if test x"$ac_cv_search_dlopen" != xno -a x"$enable_mt" = xyes; then
  enable_rtspec=yes
fi
case $ac_cv_search_dlopen in
  no | none*) dlopen_libs= ;;
  * ) dlopen_libs=$ac_cv_search_dlopen ;;
esac
have_rtspec=`if test $enable_rtspec = yes; then echo 1; else echo 0; fi`
AC_DEFINE_UNQUOTED([ENABLE_RTSPEC], [$have_rtspec],
                   [Define to 1 if runtime specialization is enabled, otherwise 0.])
AC_SUBST([ENABLE_RTSPEC], [$enable_rtspec])

dnl Check which GCC warning options are available and
dnl add them later to MKCCFLAGS and PDCCFLAGS
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
  AC_SUBST(RCCCFLAGS, ["-Wall -Wno-unused -fno-builtin $flags_lto"])
  AC_SUBST(MKCCFLAGS, ["$gcc_options -pedantic -g $flags_lto"])
  AC_SUBST(PDCCFLAGS, ["$gcc_options -pedantic -g -O3 $flags_lto"])
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
  AC_SUBST(RCCCFLAGS, ["-dalign -fsimple -xsafe=mem"])
  AC_SUBST(MKCCFLAGS, ["-erroff=E_CAST_DOESNT_YIELD_LVALUE -g"])
  AC_SUBST(PDCCFLAGS, ["-erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xO4 -KPIC"])
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
  AC_SUBST(RCCCFLAGS, ["-Wall -Wno-unused -fno-builtin"])
  AC_SUBST(MKCCFLAGS, ["-Wall -g"])
  AC_SUBST(PDCCFLAGS, [""])
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

dnl This is a special case for building sac2c with c++ compiler.
if [[ "$GXX" = yes -a "$enable_cplusplus" = yes ]]; then
  AC_SUBST(SAC2C_CC, ["g++"])
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-O1])
  AC_SUBST(OPT_O2, [-O2])
  AC_SUBST(OPT_O3, [-O3])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCCCFLAGS, ["-Wall -Wno-unused -fno-builtin"])
  AC_SUBST(SAC2C_MKCCFLAGS, ["-Wno-write-strings -g"])
  AC_SUBST(SAC2C_PDCCFLAGS, ["-Wno-write-strings -g -O3"])
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

dnl
dnl append platform specific files to sac2crc
dnl

AC_SUBST_FILE([RCSUN])
AC_SUBST_FILE([RCX86])
AC_SUBST_FILE([RCALPHA])
AC_SUBST_FILE([RCMAC])
RCSUN=setup/sac2crc.SUN
RCX86=setup/sac2crc.X86
RCALPHA=setup/sac2crc.ALPHA
RCMAC=setup/sac2crc.MAC


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
    RANLIB="$RANLIB -c"
    OSFLAGS=["-D_DARWIN_C_SOURCE"]
    LD_DYNAMIC=["-undefined suppress -flat_namespace -dynamiclib -install_name '@rpath/%libname%' "]
    LD_PATH=["-L%path% -Wl,-rpath,%path%"]
    LD_FLAGS=""
    AC_MSG_RESULT([darwin settings])
    ;;
  *bsd*)
    OSFLAGS=["-fPIC -DPIC"]
    LD_DYNAMIC=["-shared -symbolic -Wl,-allow-shlib-undefined $flags_lto"]
    LD_PATH=["-L%path% -Wl,-rpath,%path%"]
    LD_FLAGS=["$flags_lto"]
    AC_MSG_RESULT([BSD settings])
    ;;
  *)
    OSFLAGS=["-fPIC -DPIC -D_POSIX_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE"]
    LD_DYNAMIC=["-dy -shared -Wl,-allow-shlib-undefined"]
    LD_PATH=["-L%path% -Wl,-rpath,%path%"]
    LD_FLAGS="-Wl,-allow-shlib-undefined"
    AC_MSG_RESULT([unknown])
    ;;
esac

AC_SUBST([OS], [$target_os])
AC_SUBST([ARCH], [$target_cpu])
AC_SUBST([OSFLAGS])
AC_SUBST([CFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([LDFLAGS])
AC_SUBST([LD_DYNAMIC])
AC_SUBST([LD_PATH])
AC_SUBST([LD_FLAGS])

dnl
dnl static sac2c settings for config.h
dnl

AC_DEFINE([PF_MAXFUN], [100], [set to maximum number of sac functions for profiling])
AC_DEFINE([PF_MAXFUNAP], [100],
           [set to maximum number of sac function applications for profiling])
AC_DEFINE([PF_MAXFUNNAMELEN], [100],
           [set to maximum length of sac function names for profiling])

AC_DEFINE([SAC_PRELUDE_NAME], ["sacprelude"], [Name of the sac prelude module.])
AC_DEFINE( UNUSED, __attribute__((unused)), define macro for unused variables)


AC_OUTPUT

cat <<EOF
*
* Configuration done.
*
* Detected OS string:      $target_os
* Detected CPU string:     $target_cpu
*
* Run-time specialization: $enable_rtspec
* Private heap manager:    $enable_phm
* Back-ends:
* - MT/pthread:            $enable_mt
* - MT/LPEL:               $enable_mt_lpel
* - CUDA:                  $enable_cuda
* - OpenMP:                $enable_omp
* - SL:                    $enable_sl
*
EOF
