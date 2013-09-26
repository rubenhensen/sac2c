dnl check for the availability of the SAC standard library sources.
dnl example use: CHECK_STDLIB([modules/structures/ArrayBasics.sac])
dnl The argument indicates a file whose presence confirms the path is correct.
AC_DEFUN([CHECK_STDLIB],dnl
[
  AC_ARG_WITH([stdlib-sources],
              [AS_HELP_STRING([--with-stdlib-sources],
                              [Where to find the SAC standard library sources.])],
              [with_stdlib_sources=$withval],
              [with_stdlib_sources=])
  if test x"$with_stdlib_sources" = x; then
     AC_MSG_WARN([--with-stdlib-sources is not set; do not forget to edit sac2crc with the final location.])
  else
     if ! test -r "$with_stdlib_sources/$1"; then
        AC_MSG_ERROR([Cannot find $with_stdlib_sources/$1. Is --with-stdlib-sources correct?])
     fi
     with_stdlib_sources=`(cd "$with_stdlib_sources" && pwd)`
  fi
  STDLIB_SOURCES=$with_stdlib_sources
  AC_SUBST([STDLIB_SOURCES])
])
