dnl
dnl $Id$
dnl

dnl configure script for building sac2c compiler 
dnl on different hosts and operating systems
dnl the general layout is to first to the platform
dnl independent checks for programs and then have
dnl one big check for different operating systems

dnl tell configure who we are
dnl parameters: package name, version, where to send bugreports
AC_INIT(sac2c, svn, info@sac-home.org)

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

dnl set OS and ARCH flags
AC_DEFINE_UNQUOTED(OS,"$target_os",set to the os type)
AC_SUBST(OS,$target_os)
AC_DEFINE_UNQUOTED(ARCH,"$target_cpu",set to the architecture)
AC_SUBST(ARCH,$target_cpu)

dnl search for a C compiler
AC_PROG_CC(gcc cc)

dnl
dnl check whether its suns cc
dnl

AC_MSG_CHECKING(whether cc is a Sun Pro C compiler)
AC_TRY_COMPILE(,
[#ifndef __SUNPRO_C
  choke me
#endif],
AC_MSG_RESULT(yes)
SUNC=yes,
AC_MSG_RESULT(no)
SUNC=no
)


dnl
dnl check whether its compaqs (or dec) cc
dnl

AC_MSG_CHECKING(whether cc is a Compaq/DEC C compiler)
AC_TRY_COMPILE(,
[#ifndef __DECC
  choke me
#endif],
AC_MSG_RESULT(yes)
DECC=yes,
AC_MSG_RESULT(no)
DECC=no
)


dnl
dnl check whether its apples cc
dnl

AC_MSG_CHECKING(whether cc is a Apple compiler)
AC_TRY_COMPILE(,
[#ifndef __APPLE__
  choke me
  #endif],
  AC_MSG_RESULT(yes)
  MACC=yes
  GCC=no,
  AC_MSG_RESULT(no)
  MACC=no
)
  


dnl
dnl Check For Darwin 9+ for linker rpath support
dnl

dnl
dnl check whether popen is an extension
dnl

AC_MSG_CHECKING([need of _EXTENSIONS_ flag])
AC_TRY_COMPILE([#include <stdio.h>],[popen("rw","testfile")],
AC_MSG_RESULT(no)
need_ext_flags=no,
AC_MSG_RESULT(yes)
need_ext_flags=yes)

dnl check for C preprocessor
AC_PROG_CPP()

dnl check for ranlib
AC_PROG_RANLIB()

dnl check, whether we need -traditinal flag
AC_PROG_GCC_TRADITIONAL

dnl
dnl check whether its gnu indent
dnl

AC_MSG_CHECKING([whether GNU indent works])
GNUINDENT=$(indent --version 2>/dev/null)
case "$GNUINDENT" in
  *GNU*) 
    CB="indent"
    AC_MSG_RESULT(yes)
    ;;
  *) 
    GNUINDENT=$(gindent --version 2>/dev/null)
    case "$GNUINDENT" in
    *GNU*)
      CB="gindent"
      AC_MSG_RESULT(yes)
      ;;
    *)
      CB="\$(PROJECT_ROOT)/src/bin/cb"
      AC_MSG_RESULT(no)
      ;;
    esac
esac
AC_SUBST(CB, $CB)

dnl search for bison, byacc or yacc
AC_PROG_YACC()


dnl
dnl check whether a non yacc works
dnl

if [ test "$YACC" != "yacc" ]; then
  AC_MSG_CHECKING([whether $YACC works])
  mkdir .ac_yacc_test
  cd .ac_yacc_test
  cp ../../src/libsac2c/scanparse/sac.y .
  if [ $YACC sac.y >/dev/null 2>/dev/null ]; then
    AC_MSG_RESULT(ok)
  else
    AC_MSG_RESULT(failed)
    AC_CHECK_PROG(YACC, [yacc], [yacc], [!fail])
    if [ test "$YACC" != "yacc" ]; then
      AC_MSG_ERROR([cannot find working bison/yacc], 10)
    else
      AC_MSG_NOTICE([using yacc instead])
    fi
  fi
  cd ..
  rm -rf .ac_yacc_test
fi

dnl search for flex or lex
AC_PROG_LEX()

dnl search for xlst engine
AC_CHECK_PROGS(XSLT, [xsltproc sabcmd], [!fail])

if [ test "$XSLT" = "!fail" ]; then
  AC_MSG_ERROR( [Cannot find a usable xslt processor.])
fi

dnl search for m4 macro processor
AC_CHECK_PROGS(M4, [gm4 m4], [!fail])

if [ test "$M4" = "!fail" ]; then
  AC_MSG_ERROR( [Cannot find a usable m4 processor.])
fi

dnl search for malloc.h
AC_CHECK_HEADERS(malloc.h)

dnl check for mkdtemp
AC_CHECK_FUNCS(mkdtemp)

dnl search for pthread.h
AC_CHECK_HEADER(pthread.h,[

dnl if found, enable multi threading
AC_DEFINE(HAVE_PTHREAD_H, 1, set to 1 if you have the <pthread.h> header file)
AC_SUBST(DISABLE_MT, no)

],[

dnl otherwise disable mt
AC_DEFINE(DISABLE_MT, 1, set to 1 to disable multi threading)
AC_SUBST(DISABLE_MT, yes)

])

dnl check whether to link math library
AC_SEARCH_LIBS(sqrt, m)

dnl check whether to link dlopen library
AC_SEARCH_LIBS(dlopen, dl)

dnl checking for presence of strtok and strrchr
AC_CHECK_FUNCS([strtok strrchr])

dnl check for sbrk function
AC_CHECK_FUNC(sbrk, [

dnl if found, check type
dnl  check definition of sbrk
AC_MSG_CHECKING(for sbrk argument type)
include_file_list=`ls -1 /usr/include/*.h`
sbrk_types=`for name in $include_file_list; do grep sbrk $name 2>/dev/null; done`
[sbrk_types=`echo "$sbrk_types" | sed -e 's/[A-Za-z0-9 _*]*([ ]*//g' | sed -e 's/)[A-Za-z0-9_* ;]*//g' | sed -e 's/ [A-Za-z0-9_* ]*//g'` ]
[ for name in $sbrk_types; do ]
 AC_TRY_COMPILE([#include <unistd.h>],[sbrk(($name) 0)],break,name=none)
[ done ]

[ if test $name = none ; then ]
AC_DEFINE(DISABLE_PHM, 1)
AC_SUBST(DISABLE_PHM, yes)
AC_MSG_RESULT(not found)
[ else ]
AC_DEFINE_UNQUOTED(SBRK_T,$name,set to the type of sbrk argument)
AC_MSG_RESULT($name)
AC_SUBST(DISABLE_PHM, no)
[ fi ]
],[

dnl otherwise disable pm
AC_DEFINE(DISABLE_PHM, 1, set to disable private heapmanagement)
AC_SUBST(DISABLE_PHM, yes)
])

dnl
dnl check if printf generates leading 0x for pointers
dnl

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


dnl
dnl check if stderr is a constant (needed for dbug.c
dnl

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


dnl
dnl when not using bison, we need to reference YYLABELS
dnl

if test "$YACC" = "obsolete_yacc"; then
  AC_DEFINE(MUST_REFERENCE_YYLABELS, 1, set to 1 if gcc complains about unused labels)
fi

dnl
dnl check for pthread flag on gcc
dnl

if test "$GCC" = yes; then
  AC_MSG_CHECKING([existence of pthread flag])
  echo "int main(){return 0;}" > .sac_configure_temp.c
  if test "`gcc -o .sac_configure_temp -pthread .sac_configure_temp.c 2>&1 | grep "unrecognized option"`" = ""; then
    AC_MSG_RESULT([supported])
    GCC_PTHREADS="-pthread"
  else
    AC_MSG_RESULT([unsupported])
    GCC_PTHREADS="-lpthread"
  fi
  rm -f .sac_configure_temp.c .sac_configure_temp
fi


dnl
dnl test for compiler flags for sac2crc
dnl

AC_MSG_CHECKING([sac2crc compiler flags])
if [ test "$GCC" = yes ]; then
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-O1])
  AC_SUBST(OPT_O2, [-O2])
  AC_SUBST(OPT_O3, [-O3])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, ["-Wall -Wno-unused -fno-builtin -std=c99"])
  AC_SUBST(MKCCFLAGS, ["-Wall -g -std=c99"])
  AC_SUBST(PDCCFLAGS, ["-Wall -g -O3 -std=c99"])
  AC_SUBST(GENPIC, ["-fPIC -DPIC"])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["gcc -E -C -x c"])
  AC_SUBST(CCMTLINK, [$GCC_PTHREADS])
  AC_MSG_RESULT([gcc settings])
elif [ test "$SUNC" = yes ]; then
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-xO2])
  AC_SUBST(OPT_O2, [-xO4])
  AC_SUBST(OPT_O3, [-xO5])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, ["-dalign -fsimple -xsafe=mem"])
  AC_SUBST(MKCCFLAGS, ["-erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xc99=all"])
  AC_SUBST(PDCCFLAGS, ["-erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xO4 -xc99=all -KPIC"])
  AC_SUBST(GENPIC, ["-KPIC"])
  AC_SUBST(DEPSFLAG, ["-xM"])
  AC_SUBST(CPPFILE, ["cc -E -C -x c"])
  AC_SUBST(CCMTLINK, ["-lpthread"])
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
  AC_SUBST(CCMTLINK, ["-pthread"])
  AC_MSG_RESULT([Compaq/DEC cc settings])
elif [ test "$MACC" = yes ]; then
  AC_SUBST(OPT_O0, [""])
  AC_SUBST(OPT_O1, [-O1])
  AC_SUBST(OPT_O2, [-O2])
  AC_SUBST(OPT_O3, [-O3])
  AC_SUBST(OPT_g, [-g])
  AC_SUBST(RCLDFLAGS, [""])
  AC_SUBST(RCCCFLAGS, ["-Wall -no-cpp-precomp -Wno-unused -fno-builtin"])
  AC_SUBST(MKCCFLAGS, ["-Wall -std=c99 -g"])
  AC_SUBST(PDCCFLAGS, ["-std=c99"])
  AC_SUBST(GENPIC, [""])
  AC_SUBST(DEPSFLAG, ["-M"])
  AC_SUBST(CPPFILE, ["gcc -E -C -x c"])
  AC_SUBST(CCMTLINK, [""])
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
  AC_SUBST(CCMTLINK, [""])
  AC_MSG_RESULT([none found])
fi   


dnl 
dnl append platform specific files to sac2crc
dnl 

AC_SUBST_FILE(RCSUN)
AC_SUBST_FILE(RCX86)
AC_SUBST_FILE(RCALPHA)
AC_SUBST_FILE(RCMAC)
RCSUN=./sac2crc.SUN
RCX86=./sac2crc.X86
RCALPHA=./sac2crc.ALPHA
RCMAC=./sac2crc.MAC


dnl
dnl generate specific flags for known os
dnl

AC_MSG_CHECKING([sac2crc system flags])

OSFLAGS=""
case "$target_os" in
  solaris*)     
    OSFLAGS=["-D__EXTENSIONS__ -DMUST_INIT_YY -DPIC"]
    LD_DYNAMIC=["-G -dy"]
    LD_PATH=["-R"]
    LD_FLAGS=""
    AC_MSG_RESULT([solaris settings])
    ;;
  *linux*)      
    OSFLAGS=["-fPIC -DPIC -D_POSIX_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE -Dlint"]
    LD_DYNAMIC=["-dy -shared -allow-shlib-undefined"]
    LD_PATH=["-Wl,-rpath,"]
    LD_FLAGS="-Wl,-allow-shlib-undefined"
    AC_MSG_RESULT([linux settings])
    ;;
  *osf*)	
    OSFLAGS=["-D_OSF_SOURCE"]
    LD_DYNAMIC=[""]
    LD_PATH=[""]
    LD_FLAGS=""
    AC_MSG_RESULT([OSF settings])
    ;;
  *darwin*)      
    dnl darwin needs some extra flags
    AC_DEFINE(DISABLE_PHM, 1)
    AC_SUBST(DISABLE_PHM, yes)
    RANLIB="$RANLIB -c"
    OSFLAGS=["-no-cpp-precomp -Wno-long-double"]
    case "$target_os" in
      *darwin9\.*)
        LD_DYNAMIC=["-undefined suppress -flat_namespace -dynamiclib -install_name '@rpath/%libname%' "]
        LD_PATH=["-Wl,-rpath,"]
        LD_FLAGS=""
        AC_MSG_RESULT([darwin 9.x settings])
        ;;
      [*darwin[6-8]\.*)]
        LD_DYNAMIC=["-undefined suppress -flat_namespace -bundle"]
        LD_PATH=["-L"]
        LD_FLAGS=""
        AC_MSG_RESULT([darwin 7.x/8.x settings])
      ;;
      *)
        AC_MSG_RESULT([unknown darwin system])
    esac
    ;;
  *bsd*)
    OSFLAGS=["-fpic"]
    LD_DYNAMIC=["ld -dy -shared -symbolic"]
    LD_PATH=["-Wl,-rpath,"]
    LD_FLAGS=""
    AC_MSG_RESULT([BSD settings])
    ;;
  *)
    AC_MSG_RESULT([unknown])
    ;;
esac
AC_SUBST(OSFLAGS)
AC_SUBST(LD_DYNAMIC)
AC_SUBST(LD_PATH)
AC_SUBST(LD_FLAGS)


dnl 
dnl static sac2c settings for config.h
dnl

AC_DEFINE( MAX_PATH_LEN, 1024, [set to maximum path length])
AC_DEFINE( MAX_FILE_NAME, 256, [set to maximum file name length])
AC_DEFINE( PF_MAXFUN, 100, [set to maximum number of sac functions for profiling])
AC_DEFINE( PF_MAXFUNAP, 100, 
           [set to maximum number of sac function applications for profiling])
AC_DEFINE( PF_MAXFUNNAMELEN, 100, 
           [set to maximum length of sac function names for profiling])

AC_DEFINE( SAC_PRELUDE_NAME, "sacprelude", [name of sac prelude])


dnl
dnl create configure-generated file
dnl

AC_OUTPUT()
