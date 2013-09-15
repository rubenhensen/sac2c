dnl check for LPEL and enable MT backend if found
dnl
AC_DEFUN([CHECK_LPEL],dnl
[
   AC_ARG_ENABLE([mt-lpel],
                 [AS_HELP_STRING([--disable-lpel],
                 [Disable checking for MT/LPEL])],
                 [enable_mt_lpel=$enableval],
                 [enable_mt_lpel=yes])

   AC_ARG_VAR([LPEL_CFLAGS], [compiler flags to locate lpel.h])
   AC_ARG_VAR([LPEL_LIBS], [linker flags to link with lpel])

   AC_ARG_WITH([lpel-includes],
               [AC_HELP_STRING([--with-lpel-includes=DIR],
                               [look up lpel.h in DIR])],
               [], [with_lpel_includes=])
   AC_ARG_WITH([lpel-libs],
               [AC_HELP_STRING([--with-lpel-libs=DIR],
                               [look up liblpel in DIR])],
               [], [with_lpel_libs=])

   if test x"$with_lpel_includes" != x; then
     LPEL_CFLAGS="-I$with_lpel_includes $LPEL_CFLAGS"
   fi
   if test x"$with_lpel_libs" != x; then
     LPEL_LIBS="-L$with_lpel_libs $LPEL_LIBS"
   fi

   # check for header
   save_CPPFLAGS=$CPPFLAGS
   CPPFLAGS="$CPPFLAGS $LPEL_CFLAGS"
   AC_CHECK_HEADER([lpel.h])
   CPPFLAGS=$save_CPPFLAGS

   # check for lib
   save_LDFLAGS=$LDFLAGS
   LDFLAGS="$LDFLAGS $LPEL_LIBS"
   AC_CHECK_LIB([lpel], [LpelInit], [$PTHREAD_LIBS])
   LDFLAGS=$save_LDFLAGS

   # have both?
   if test x"$ac_cv_lib_lpel_LpelInit" != xyes -o x"$ac_cv_header_lpel_h" != xyes; then
      enable_mt_lpel=no
   fi

   # inform configuration
   have_lpel=`if test $enable_mt_lpel = yes; then echo 1; else echo 0; fi`
   AC_SUBST([ENABLE_MT_LPEL], [$enable_mt_lpel])
   AC_SUBST([MT_CFLAGS_LPEL], [$LPEL_CFLAGS])
   LPEL_LIBS="$LPEL_LIBS -llpel"
   AC_SUBST([MT_LDFLAGS_LPEL], [$LPEL_LIBS])
   AC_DEFINE_UNQUOTED([ENABLE_MT_LPEL],
                      [$have_lpel],
                      [Define to 1 if the MT/LPEL backend is enabled, otherwise 0.])
])
