dnl check for lex/yacc.
dnl this substitutes and documents LEX and YACC.
AC_DEFUN([CHECK_LEX_YACC],dnl
[
  AC_ARG_VAR([LEX], [location of flex/lex (should be autodetected)])
  AC_PROG_LEX
  if test $LEX = : ; then
    AC_MSG_ERROR([flex/lex not found.])
  fi

  AC_PROG_YACC
  AC_MSG_CHECKING([whether $YACC works])
  cp -f "$srcdir"/../src/libsac2c/scanparse/sac.y conftest.y
  if $YACC conftest.y >/dev/null 2>/dev/null; then
     AC_MSG_RESULT([yes])
  else
     if ! yacc conftest.y >/dev/null 2>/dev/null; then
        AC_MSG_RESULT([no])
        YACC=no
     else
        AC_MSG_RESULT([no, but yacc does])
        YACC=yacc
     fi
  fi
  if test x"$YACC" = xno; then
    AC_MSG_ERROR([usable bison/yacc not found.])
  fi
  if test "$YACC" = "yacc"; then
    AC_DEFINE([MUST_REFERENCE_YYLABELS], [1],
              [Set to 1 if compiler complains about unused yylabels.])
  fi
])
