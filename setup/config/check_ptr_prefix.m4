dnl Check whether printf outputs 0x for %p formats.
dnl defines NEED_PTR_PREFIX if necessary.
AC_DEFUN([CHECK_PTR_PREFIX], dnl
[
  AC_MSG_CHECKING([whether printf("%d") issues a leading "0x"])
  AC_RUN_IFELSE([AC_LANG_PROGRAM([@%:@include <stdio.h>],
                                 [char buf@<:@64@:>@;
                                  snprintf(buf, 64, "%p", buf);
                                  if (buf@<:@0@:>@ == '0' && buf@<:@1@:>@ == 'x')
                                     return 0;
                                  return 1;
                                  ])],
                [AC_MSG_RESULT([yes])],
                [AC_MSG_RESULT([no])
                 AC_DEFINE([NEED_PTR_PREFIX], 1,
                           [Define if printf does not print a leading 0x in %p formats.])
                ])
])
