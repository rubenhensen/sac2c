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
])
