AC_DEFUN([CHECK_CC_BRACKETS], dnl
[
  AC_REQUIRE([AC_PROG_CC])
  brackets_ok=no
  save_CFLAGS=$CFLAGS
  for cf in "" "-fbracket-depth=2048"; do
     CFLAGS="$save_CFLAGS $cf"
     AC_MSG_CHECKING([whether $CC $cf supports 300 bracket nesting levels])
AC_TRY_COMPILE([],[
int a =
((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((0))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

int main(void) { return a; }
],[AC_MSG_RESULT(yes); brackets_ok=yes; break])
  done
  if test $brackets_ok = no; then
     AC_MSG_ERROR([$CC does not support large bracketing depths])
  fi
])
  
