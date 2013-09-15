dnl check for sbrk and try to enable PHM, let
dnl user disable if desired
AC_DEFUN([CHECK_PHM], dnl
[
   AC_ARG_ENABLE([phm],
                 [AS_HELP_STRING([--disable-phm],
                                 [Disable checking for the private heap manager])],
                 [enable_phm=$enableval],
                 [enable_phm=yes])
   sbrk_arg_type=none
   if test x"$enable_phm" = xyes; then
      AC_CHECK_FUNC([sbrk])
      if test x$ac_cv_func_sbrk = xyes; then
         AC_MSG_CHECKING([for argument type of sbrk()])
         for sbrk_arg_type in intptr_t ptrdiff_t ssize_t int; do
            AC_TRY_COMPILE([@%:@include <unistd.h>], [sbrk(($sbrk_arg_type) 0)],
                           [break], [sbrk_arg_type=none])
         done
         AC_MSG_RESULT([$sbrk_arg_type])
         if test $sbrk_arg_type = none; then
            enable_phm=no
         fi
      fi
   fi
   AC_DEFINE_UNQUOTED([SBRK_T], [$sbrk_arg_type],
                      [Type of the argument of sbrk().])
   have_phm=`if test x"$enable_phm" = xyes; then echo 1; else echo 0; fi`
   AC_SUBST([ENABLE_PHM], [$enable_phm])
   AC_DEFINE_UNQUOTED([ENABLE_PHM], [$have_phm],
                      [Define to 1 if the private heap manager is enabled, otherwise 0.])
])
