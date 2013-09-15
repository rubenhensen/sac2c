dnl CHECK_LTO
dnl This macro checks whether LTO  (link time optimizations)
dnl is requested as configure flag, and
dnl whether supported by compiler.
dnl this also set $flags_lto to -flto if supported/activated
AC_DEFUN([CHECK_LTO],dnl
[
  AC_REQUIRE([AC_PROG_CC])
  AC_ARG_ENABLE([lto],
                [AS_HELP_STRING([--enable-lto],
                                [Enable link time optimisations (LTO)])],
                [enable_lto=$enableval], [enable_lto=no])
  flags_lto=""
  if test "x$enable_lto" != xno; then
      CHECK_CC_OPTION([-flto], [flags_lto])
      if test "x$flags_lto" = x; then
          AC_MSG_ERROR([C compiler does not support -flto])
      fi
  fi
])
