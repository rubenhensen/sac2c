dnl set the preferred file extension for shared libraries.
AC_DEFUN([CHECK_DLL_EXT],dnl
[
  AC_REQUIRE([AC_CANONICAL_TARGET])
  AC_MSG_CHECKING([for file name extension for dynamic libraries])
  case "$target_os" in
     *cygwin* | *windows* | *mingw*) dll_ext=.dll ;;
     *darwin* ) dll_ext=.dylib ;;
     * ) dll_ext=.so ;;
  esac
  AC_SUBST([SHARED_LIB_EXT], [$dll_ext])
  AC_DEFINE_UNQUOTED([SHARED_LIB_EXT],
                     ["$dll_ext"],
                     [File extension used for shared library files.])
  AC_MSG_RESULT([$dll_ext])
])
