AC_DEFUN([CHECK_CONST_STDERR], dnl
[
  AC_MSG_CHECKING([whether stderr is a variable])
  AC_TRY_COMPILE([@%:@include <stdio.h>],
                 [FILE *_mine_ = stderr;],
                 [AC_MSG_RESULT([yes])],
                 [AC_MSG_RESULT([no])
                  AC_DEFINE([STDERR_IS_CONSTANT], 1,
                            [Define if stderr is a constant.])])
])
