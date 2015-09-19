dnl CHECK_COMPILER_VENDOR
dnl this macro checks what type of C compiler
dnl is being used.
AC_DEFUN([CHECK_COMPILER_VENDOR],dnl
[
  # When C compiler is not GCC, check if one of
  # Sun, Dec or Mac compiler is available on the system.
  if test "$GCC" != yes ; then
    CHECK_CPP_FLAG([__SUNPRO_C], [SUNC])
    CHECK_CPP_FLAG([__DECC], [DECC])
    CHECK_CPP_FLAG([__APPLE_CC__], [MACC])
  fi

  # For some reason, ICC is recognized as GCC.
  CHECK_CPP_FLAG([__INTEL_COMPILER], [ICC])
  if test "$ICC" != yes ; then
      CHECK_CPP_FLAG([__ICC], [ICC])
  fi
  if test "$ICC" = yes ; then
    [GCC=no]
  fi
])