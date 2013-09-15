dnl Check if a tool is available and report an error if
dnl not found.
AC_DEFUN([CHECK_REQUIRED_TOOL],dnl
[
  AC_ARG_VAR([$1], [Location of '$2' (should be autodetected)])
  AC_CHECK_PROGS([$1], [$2], [no])
  if test x"$$1" = xno; then
     AC_MSG_ERROR([Cannot find utility for $1.])
  fi
])
