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
  CUDA_ROOT=no
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
             if $NVCC conftest.cc -o conftest$EXEEXT; then
                AC_MSG_RESULT([$enable_cuda])
                NVCC_PATH=$NVCC
                # get cuda base directory from NVCC location
                # this is generally safe as CUDA has the same directory
                # structure across systems
                CUDA_ROOT=$(dirname $(dirname $NVCC))
                AC_MSG_NOTICE([using '$CUDA_ROOT' as [CUDA_ROOT]])
                # extract the architecture flags from the test program
                AC_MSG_CHECKING([what CUDA compute level is supported])
                if ./conftest$EXEEXT 2>&1 >/dev/null; then
                  CUDA_ARCH=$(./conftest$EXEEXT)
                else
                  CUDA_ARCH="\"please set this manually\""
                fi
                AC_MSG_RESULT([$CUDA_ARCH])
            else
                enable_cuda=no
                AC_MSG_RESULT([$enable_cuda])
            fi
        fi
      fi
  fi
  AC_SUBST([CUDA_ARCH])
  AC_SUBST([NVCC_PATH])
  AC_SUBST([CUDA_ROOT])
])
