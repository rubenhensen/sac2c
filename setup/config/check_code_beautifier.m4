dnl find a code beautifier (cb).
dnl sets and substitutes CB to whichever is found.
AC_DEFUN([CHECK_CODE_BEAUTIFIER],dnl
[
   AC_ARG_VAR([CB], [Code beautifier (defaults to indent)])
   AC_CHECK_PROGS([CB], [gindent indent], [no])
   if test "x$CB" = xno; then
     # backward compatibility
     CB="\$(PROJECT_ROOT)/src/bin/cb"
   fi
])
