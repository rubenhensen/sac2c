dnl
dnl $Log$
dnl Revision 1.5  2003/03/21 00:43:23  sah
dnl fixed bug in stderr_is_constant
dnl detection
dnl
dnl Revision 1.4  2003/03/21 00:34:39  sah
dnl added support for linux
dnl (tested on gentoo linux)
dnl
dnl Revision 1.3  2003/03/20 23:12:59  sah
dnl cpu sparc now matches sac SPARC setting
dnl
dnl Revision 1.2  2003/03/20 22:28:07  sah
dnl os solaris* os now matched to SOLARIS
dnl
dnl Revision 1.1  2003/03/20 22:19:39  sah
dnl Initial revision
dnl
dnl

dnl configure script for building sac2c compiler 
dnl on different hosts and os

dnl tell configure who we are
dnl parameters: package name, version, where to send bugreports
AC_INIT(sac2c, rcs, info@sac-home.org)

dnl set source directory and scripting directory
AC_CONFIG_SRCDIR(Makefile.Config.in)
AC_CONFIG_AUX_DIR(./config)

dnl we use C as our compiler language
AC_LANG(C)

dnl and which files to create
AC_CONFIG_FILES(Makefile.Config)
AC_CONFIG_HEADER(./src/global/config.h)

dnl check for host type
AC_CANONICAL_BUILD()
AC_CANONICAL_HOST()
AC_CANONICAL_TARGET()

dnl set OS and ARCH flags

dnl match autoconf os names to 
dnl the ususal sac ones
case $target_os in
 *solaris*) target_os=SOLARIS;;
 *linux*)   target_os=LINUX;;
esac

dnl match autoconf arch names to
dnl the usual sac ones
case $target_cpu in
 sparc) target_cpu=SPARC;;
 i?86)  target_cpu=X86;;
esac

AC_DEFINE_UNQUOTED(OS,"$target_os",set to the os type)
AC_SUBST(OS,$target_os)
AC_DEFINE_UNQUOTED(ARCH,"$target_cpu",set to the architecture)
AC_SUBST(ARCH,$target_cpu)

dnl search for a C compiler
AC_PROG_CC(gcc,cc)

dnl check, whether we need -traditinal flag
AC_PROG_GCC_TRADITIONAL

dnl search for bison, byacc or yacc
AC_PROG_YACC()

dnl search for flex or lex
AC_PROG_LEX()

dnl search for malloc.h
AC_CHECK_HEADERS(malloc.h)

dnl search for pthread.h
AC_CHECK_HEADER(pthread.h,[

dnl check whether to link math library
AC_SEARCH_LIBS(fabs, m)

dnl if found, enable multi threading
AC_DEFINE(HAVE_PTHREAD_H, 1, set to 1 if you have the <pthread.h> header file)
AC_SUBST(DISABLE_MT, no)

],[

dnl otherwise disable mt
AC_DEFINE(DISABLE_MT, 1, set to 1 to disable multi threading)
AC_SUBST(DISABLE_MT, yes)

])

dnl checking for presence of strtok and strrchr
AC_CHECK_FUNCS([strtok strrchr])

dnl check for sbrk function
AC_CHECK_FUNC(sbrk, [

dnl if found, check type
dnl  check definition of sbrk
AC_MSG_CHECKING(for sbrk argument type)
include_file_list=`ls -1 /usr/include/*.h`
sbrk_types=`for name in $include_file_list; do grep sbrk $name 2>/dev/null; done`
[sbrk_types=`echo "$sbrk_types" | sed 's/[A-Za-z0-9 _*]*([ ]*//g' | sed 's/\( [A-Za-z0-9*_ ]*\)*)[A-Za-z0-9_ ;]*//g'`]
AC_DEFINE_UNQUOTED(SBRK_T,$sbrk_types,set to the type of sbrk argument)
AC_MSG_RESULT($sbrk_types)
AC_SUBST(DISABLE_PHM, no)

],[

dnl otherwise disable pm
AC_DEFINE(DISABLE_PHM, 1, set to disable private heapmanagement)
AC_SUBST(DISABLE_PHM, yes)
])

dnl check if we need to define _REENTRANT
dnl or if its defined in pthreads.d
AC_MSG_CHECKING(for need of _REENTRANT)
AC_EGREP_CPP(yes, 
[#include <pthread.h>
#ifdef _REENTRANT
no
#else
yes
#endif
], need_reentrant_definition=yes, need_reentrant_definition=no)
if [ test $need_reentrant_definition = yes ]; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(NEED_REENTRANT, 1, set to 1 if pthread.h does not define _REENTRANT)
else
  AC_MSG_RESULT(no)
fi

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
FILE *_db_fp_ = stderr;
],[],stderr_is_constant=yes,stderr_is_constant=no)
if [ test $stderr_is_constant = yes ]; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(STDERR_IS_CONSTANT, 1, set to 1 if stderr is a constant)
else
  AC_MSG_RESULT(no)
fi

dnl check whether to remove ar magic file
AC_MSG_CHECKING(for ar magic file)
mkdir .ar_magic_file_test
cd .ar_magic_file_test
touch testfile
ar -rc test.ar testfile
rm testfile
ar -x test.ar
[if [ -f ________64ELEL_ ]; then]
  AC_DEFINE(MUST_KILL_AR_MAGIC_FILE,1,set to 1 if ar generates a magic file)
  rm ________64ELEL_
  AC_MSG_RESULT(found);
else
  AC_MSG_RESULT(not found);
fi
rm testfile test.ar
cd ..
rmdir .ar_magic_file_test

dnl when not using bison, we need to
dnl reference YYLABELS
if test "$YACC" = "yacc"; then
  AC_DEFINE(MUST_REFERENCE_YYLABELS, 1, set to 1 if gcc complains about unused labels)
fi

dnl generate the Makefile.Config out of Makefile.Config.in
AC_OUTPUT()
