dnl
dnl check CUDA tools to enable the CUDA  back-end.
dnl
dnl This substitutes NVCC_PATH to the absolute path to the "nvcc" command
dnl and also defines the preprocessor macro ENABLE_CUDA to 0 or 1 depending
dnl on whether support is detected and working.
AC_DEFUN([CHECK_CUDA],dnl
[
  AC_ARG_ENABLE([cuda],
                [AS_HELP_STRING([--disable-cuda],
                                [Disable checking for CUDA])],
                [enable_cuda=$enableval],
                [enable_cuda=yes])

  CUDA_ARCH=no
  NVCC_PATH=no
  if test x"$enable_cuda" != xno; then
      # search for the NVidia compiler (nvcc)
      AC_ARG_VAR([NVCC], [NVidia CUDA compiler (defaults to nvcc)])
      AC_PATH_PROGS([NVCC], [nvcc], [no])
      if test x"$NVCC" = xno; then
         enable_cuda=no
      else
         # try to compile a test CUDA program
         if cp -f "$srcdir"/src/tools/cuda/deviceQuery.cpp conftest.cc 2>/dev/null; then
             AC_MSG_CHECKING([whether nvcc works])
             if ! $NVCC conftest.cc -o conftest$EXEEXT; then
                enable_cuda=no
                AC_MSG_RESULT([$enable_cuda])
             else
                AC_MSG_RESULT([$enable_cuda])
                # extract the architecture flags from the test program
                AC_MSG_CHECKING([what CUDA compute level is supported])
                CUDA_ARCH=`./conftest$EXEEXT 2>/dev/null`
                NVCC_PATH=$NVCC
                AC_MSG_RESULT([$CUDA_ARCH])
            fi
        fi
      fi
  fi
  AC_SUBST([CUDA_ARCH])
  AC_SUBST([NVCC_PATH])
])
