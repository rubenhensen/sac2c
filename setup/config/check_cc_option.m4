# CHECK_CC_OPTION(OPTION, VAR_TO_APPEND)
# The macro is checking whether chosen compiler supports an OPTION
# in case it does, the VAR_TO_APPEND is being appended by the OPTION
AC_DEFUN([CHECK_CC_OPTION],[
  AC_REQUIRE([AC_PROG_CC])
  AS_VAR_PUSHDEF([ac_cc_option], [ac_cv_$1_option])
  AC_CACHE_CHECK([whether $CC supports $1], ac_cc_option,
                 [save_CFLAGS="$CFLAGS";
                  CFLAGS="$1"
                  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
                                    [AS_VAR_SET(ac_cc_option, yes)],
                                    [AS_VAR_SET(ac_cc_option, no)])
                  AS_IF([test AS_VAR_GET(ac_cc_option) = yes],
                        [$2="$$2 $1"])
                  CFLAGS="$save_CFLAGS"
                  ])
  AS_VAR_POPDEF([ac_cc_option])
])
