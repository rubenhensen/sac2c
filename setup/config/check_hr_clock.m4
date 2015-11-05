dnl check for support for a high-resolution clock
AC_DEFUN([CHECK_HR_CLOCK], dnl
[
  AC_SEARCH_LIBS([clock_gettime], [rt])
  if test x"$ac_cv_search_clock_gettime" != xno; then
     AC_CACHE_CHECK([whether CLOCK_MONOTONIC is supported], [my_cv_clock_pcpuid],
                    [AC_LINK_IFELSE([AC_LANG_PROGRAM([
@%:@include <time.h>
@%:@include <sys/times.h>
], [
 struct timespec ts;
 clock_gettime(CLOCK_MONOTONIC, &ts);
 return 0;
])], [my_cv_clock_pcpuid=yes], [my_cv_clock_pcpuid=no])])

     if test x"$my_cv_clock_pcpuid" = xyes; then
       AC_DEFINE([HAVE_GETTIME], 1,
                 [Define if clock_gettime and CLOCK_MONOTONIC from POSIX RT are available.])
     fi
  fi
])
