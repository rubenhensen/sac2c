dnl CHECK_CPP_FLAG(FLAG, VARIABLE)
dnl This macro checks whether a preprocessor have predefined FLAG
dnl and if it does, the VARIABLE is being set to yes
dnl in other case the VARIABLE is set to no
AC_DEFUN([CHECK_CPP_FLAG],dnl
[
  AC_REQUIRE([AC_PROG_CC])
  AS_VAR_PUSHDEF([ac_cpp_flag], [ac_cv_$1_cpp_flag])
  AC_CACHE_CHECK([whether $CC supports $1 flag],
                 [ac_cpp_flag],
                 [AC_TRY_COMPILE([],
                                 [
#ifndef $1
    choke me
#endif
                                 ],
                                 [AS_VAR_SET([ac_cpp_flag], [yes])],
                                 [AS_VAR_SET([ac_cpp_flag], [no])])
                  $2=AS_VAR_GET(ac_cpp_flag)])
  AS_VAR_POPDEF([ac_cpp_flag])
])
