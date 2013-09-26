dnl check for OpenMP
AC_DEFUN([CHECK_OMP], dnl
[
   AC_ARG_ENABLE([omp],
                 [AS_HELP_STRING([--disable-omp],
                                 [Disable checking for OpenMP])],
                 [enable_omp=$enableval],
                 [enable_omp=yes])
   if test x"$enable_omp" != xno; then
      AX_OPENMP([], [enable_omp=no])
   fi
   if test x"$enable_omp" != xno; then
      save_CFLAGS=$CFLAGS
      CFLAGS="$CFLAGS $OPENMP_CFLAGS"
      AC_CHECK_HEADER([omp.h], [], [enable_omp=no])
      CFLAGS=$save_CFLAGS
   fi
   AC_SUBST([OPENMP_CFLAGS])
])
