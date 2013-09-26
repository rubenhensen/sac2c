dnl check for "slc" and enable the MUTC
dnl back-end if found.
AC_DEFUN([CHECK_SL],dnl
[
  AC_ARG_ENABLE([sl],
                [AS_HELP_STRING([--disable-sl],
                                [Disable checking for SL])],
                [enable_sl=$enableval],
                [enable_sl=yes])
  if test x"$enable_sl" != xno; then
     AC_CHECK_PROGS([SLC], [slc], [no])
     AC_CHECK_PROGS([SLAR], [slar], [no])
     AC_CHECK_PROGS([SLRANLIB], [slranlib], [no])
     if test x"$SLC" = xno -o x"$SLAR" = xno -o x"$SLRANLIB" = xno; then
        enable_sl=no
     fi
  fi
  AC_SUBST([SLC])
  AC_SUBST([SLAR])
  AC_SUBST([SLRANLIB])
])
