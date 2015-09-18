dnl check for Integer Set Library
AC_DEFUN([CHECK_ISL], dnl
[
  AC_ARG_ENABLE([isl],
                [AS_HELP_STRING([--disable-isl],
                [Disable checking for ISL])],
                [enable_isl=$enableval],
                [enable_isl=yes])

  dnl Check for presence of ISL link library
  AC_CHECK_LIB(isl,isl_ctx_alloc)
  if test "$ac_cv_lib_isl" = "no"
  then
    enable_isl=no
    AC_MSG_NOTICE([ISL link lib not found])
  fi
  AC_MSG_NOTICE([ISL LIBS is "$LIBS"])

  dnl Check for presence of a typical ISL include file
  AC_CHECK_HEADERS([isl/ctx.h])
  if test "$ac_cv_header_isl_ctx_h" = "yes"
  then
    if test "$enable_isl" = "yes"
    then
      AC_MSG_NOTICE([ ISL is enabled])
      enable_isl=yes
    fi
  else
    AC_MSG_NOTICE([ ISL is disabled])
    enable_isl=no
  fi
  AC_SUBST([HAVE_LIBISL],[$enable_isl])

])
