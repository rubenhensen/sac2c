dnl check for pthreads and MT targets
AC_DEFUN([CHECK_MT], dnl
[
  AC_ARG_ENABLE([mt],
                [AS_HELP_STRING([--disable-mt],
                [Disable checking for MT/pthreads])],
                [enable_mt=$enableval],
                [enable_mt=yes])
  if test x"$enable_mt" = xyes; then
     AX_PTHREAD
     if test x"$ax_pthread_ok" != xyes; then
        enable_mt=no
     fi
  fi
  have_pthread=`if test x"$enable_mt" != xno; then echo 1; else echo 0; fi`
  AC_DEFINE_UNQUOTED([ENABLE_MT],
                     [$have_pthread],
                     [Define to 1 if pthreads are available and MT is enabled, otherwise 0.])
  AC_SUBST([ENABLE_MT], [$have_pthread])
])
