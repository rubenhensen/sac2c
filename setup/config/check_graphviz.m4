dnl Searches for graphviz.
dnl defines DOT_FLAG to 0/1 and DOT_CMD to the program name
dnl may be overriden by user with DOT argument/envvar
AC_DEFUN([CHECK_GRAPHVIZ],dnl
[
  AC_CHECK_PROGS([DOT], [dot], [no])
  have_dot=`if test x"$DOT" = xno; then echo 0; else echo 1; fi`
  AC_DEFINE_UNQUOTED([DOT_FLAG],
                     [$have_dot],
                     [Define to 1 to enable graphviz visualization, otherwise 0.])
  AC_DEFINE_UNQUOTED([DOT_CMD],
                     ["$DOT "], dnl SPACE IS IMPORTANT!
                     [Define to the command to use to invoke graphviz/dot, followed by a space.])
])
