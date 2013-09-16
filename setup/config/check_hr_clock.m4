dnl check for support for a high-resolution clock
AC_DEFUN([CHECK_HR_CLOCK], dnl
[
  AC_SEARCH_LIBS([clock_gettime], [rt])
  if test x"$ac_cv_search_clock_gettime" != xno; then
     AC_CACHE_CHECK([whether CLOCK_PROCESS_CPUTIME_ID is supported], [my_cv_clock_pcpuid],
                    [save_LIBS=$LIBS
                     if test x"$ac_cv_search_clock_gettime" != x"none required"; then
                       LIBS="$LIBS $ac_cv_search_clock_gettime"
                     fi
                     AC_LINK_IFELSE([AC_LANG_PROGRAM([
@%:@include <time.h>
@%:@include <sys/times.h>
], [
 struct timespec ts;
 clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
 return 0;
])], [my_cv_clock_pcpuid=yes], [my_cv_clock_pcpuid=no])
                     LIBS=$save_LIBS])

     if test x"$my_cv_clock_pcpuid" = xyes; then
       AC_DEFINE([HAVE_GETTIME], 1,
                 [Define is clock_gettime and CLOCK_PROCESS_CPUTIME_ID from POSIX RT are available.])
     fi
  fi
])
