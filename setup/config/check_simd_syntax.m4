dnl Check whether CC supports vector operations expressed in GCC form.
dnl Normally we would expect that we are dealing with GCC,
dnl but if another compiler can support the same syntax -- awesome!
AC_DEFUN([CHECK_SIMD_SYNTAX],dnl
[
   AC_REQUIRE([AC_PROG_CC])
   AC_CACHE_CHECK([whether $CC supports GNU vector syntax],
                  [check_cv_simd_syntax],
                  [AC_TRY_COMPILE([],
                                 [
 float __attribute__((vector_size (4*sizeof(float)))) a, b, c;
 float x;
 a = b + c;
 a = b - c;
 a = b / c;
 a = b * c;
 x = a@<:@0@:>@;
                                 ],
                                 [check_cv_simd_syntax=yes],
                                 [check_cv_simd_syntax=no])
                   ])

  have_gnu_simd_syntax=`if test $check_cv_simd_syntax = yes; then echo 1; else echo 0; fi`
  AC_DEFINE_UNQUOTED([HAVE_GCC_SIMD_OPERATIONS],
                     [$have_gnu_simd_syntax],
                     [Set to 1 if compiler supports GNU vector syntax, otherwise 0.])
])
