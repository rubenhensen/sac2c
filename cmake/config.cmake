INCLUDE (CheckIncludeFiles)
INCLUDE (CheckCSourceCompiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckLibraryExists)
INCLUDE (CheckCSourceRuns)
INCLUDE (CheckCCompilerFlag)


# Checks if the C compiler defines a macro called DEF.
# In case macro is defined the ${FLAG} will be set on.
MACRO (CHECK_COMPILER DEF FLAG)
  CHECK_C_SOURCE_COMPILES ("
  #ifndef ${DEF}
    You have to die.
  #else
    int main ()
      {
        return 0;
      }
  #endif"
  ${FLAG}
  )
ENDMACRO ()



# Enables FLAG in case the COND is enabled.
MACRO (ENABLE_VAR_IF  FLAG)
  SET (${FLAG} OFF)
  # Here we use a trick that makes it possible to pass
  # a condition to the macro via last arguments.
  IF (${ARGN})
    SET (${FLAG} ON)
  ENDIF ()
ENDMACRO ()


# System-dependent variables.
SET (OS       "${CMAKE_SYSTEM}")
SET (ARCH     "${CMAKE_SYSTEM_PROCESSOR}")

# FIXME(artem) use CYGWIN value directly. [after the move to cmake]
SET (IS_CYGWIN  ${CYGWIN})

# These will be substituted in the sac2crc file.
SET (SHARED_LIB_EXT "${CMAKE_SHARED_LIBRARY_SUFFIX}")
SET (EXEEXT "${CMAKE_EXECUTABLE_SUFFIX}")

# Get sac2c version
GET_VERSION (SAC2C_VERSION)
STRING (REGEX REPLACE "^([0-9]+)\\..*" "\\1" SAC2C_VERSION_MAJOR "${SAC2C_VERSION}")
STRING (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\2" SAC2C_VERSION_MINOR "${SAC2C_VERSION}")
SET (SAC2C_VERSION_PATCH  0)
IF ("${SAC2C_VERSION}" MATCHES "-([0-9]+)(-g[a-f0-9]+)?(-dirty)?$")
    SET (SAC2C_VERSION_PATCH  "${CMAKE_MATCH_1}")
ENDIF ()


# Check for Link Time Optimisation
SET (HAVE_LTO   OFF)
SET (FLAGS_LTO  "")
IF (LTO)
    CHECK_C_COMPILER_FLAG ("-flto" c_compiler_has_lto)
    if (c_compiler_has_lto)
        SET (HAVE_LTO   ON)
        SET (FLAGS_LTO  "-flto")
    endif ()
ENDIF ()

# Check what extension to use for library files
SET (SHARED_LIB_EXT ".so") # assume UNIX
IF (WIN32 OR CYGWIN)
    SET (SHARED_LIB_EXT ".dll")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET (SHARED_LIB_EXT ".dylib")
ENDIF ()

# Check if C compiler supports explicit SIMD syntax.
CHECK_C_SOURCE_COMPILES ("
    int main (void) {
      float __attribute__((vector_size (4*sizeof(float)))) a, b, c;
      float x;
      a = b + c;
      a = b - c;
      a = b / c;
      a = b * c;
      x = a[0];
      return 0;
    }
    "
    HAVE_GCC_SIMD_OPERATIONS
)


# Check headers.
CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES (memory.h HAVE_MEMORY_H)

# TODO this should be checked by FindThreads and FindOpenmp
#CHECK_INCLUDE_FILES (pthread.h HAVE_PTHREAD_H)
#CHECK_INCLUDE_FILES (omp.h HAVE_OMP_H)



# Check functions
CHECK_FUNCTION_EXISTS (strtok HAVE_STRTOK)
CHECK_FUNCTION_EXISTS (strrchr HAVE_STRRCHR)
CHECK_FUNCTION_EXISTS (mkdtemp HAVE_MKDTEMP)



# Check functions in libs
ASSERT_LIB (M     "m"     "sqrt")

ASSERT_LIB (DL    "dl"    "dlopen")
CHECK_INCLUDE_FILES ("dlfcn.h" HAVE_DLFCN_H)
ENABLE_VAR_IF (ENABLE_DL  HAVE_DLFCN_H AND DL_LIB)


ASSERT_LIB (UUID  "uuid"  "uuid_generate")
CHECK_INCLUDE_FILES ("uuid/uuid.h"  HAVE_UUID_H)
ENABLE_VAR_IF (ENABLE_UUID  HAVE_UUID_H AND UUID_LIB)

FIND_LIBRARY (CRYPT_LIB   "crypt")
CHECK_INCLUDE_FILES ("crypt.h" HAVE_CRYPT_H)
ENABLE_VAR_IF (ENABLE_CRYPT  HAVE_CRYPT_H AND CRYPT_LIB)
# This is needed for the config.h file
# FIXME(artem) we can use the ENABLE_CRYPT directly
SET (ENABLE_HASH  ${ENABLE_CRYPT})

SET (LPEL_PATH)
IF (LPEL)
    ASSERT_LIB (LPEL  "lpel"  "LpelInit")
    FIND_PATH (LPEL_INCLUDE_DIR lpel.h)
    IF (LPEL_INCLUDE_DIR)
        # FIXME we enable this globally but maybe we want to put it only
        # in the saclib-related part.
        INCLUDE_DIRECTORIES (${LPEL_INCLUDE_DIR})
        SET (ENABLE_MT_LPEL     ON)
        SET (MT_CFLAGS_LPEL     "-I${LPEL_INCLUDE_DIR}")
        SET (MT_LDFLAGS_LPEL    "-L${LPEL_PATH} -llpel")
    ELSE ()
        MESSAGE (FATAL_ERROR "Cannot find lpel.h header")
    ENDIF ()
ENDIF ()


# If Option MT is set
IF (MT)
    # Prefer -pthread over -lpthread and stuff.
    SET (THREADS_PREFER_PTHREAD_FLAG ON)
    FIND_PACKAGE (Threads)
    IF (THREADS_FOUND AND NOT WIN32)
        MESSAGE (STATUS "Enabling MT")
        SET (ENABLE_MT  ON)
        # FIXME propagate ${CMAKE_THREAD_LIBS_INIT} into sac2crc
        #MESSAGE (STATUS "Threading library is '${CMAKE_THREAD_LIBS_INIT}'")
    ENDIF ()
ENDIF ()

# If oprion OMP is set.
IF (OMP)
    FIND_PACKAGE (OpenMP)
    IF (OPENMP_FOUND)
        # FIXME We can use only one of the variables.  I.e. of course
        # we have openmp if it is enabled.
        SET (ENABLE_OMP      ON)
        SET (HAVE_OPENMP     ON)
        SET (OPENMP_CFLAGS   ${OpenMP_C_FLAGS})
    ENDIF ()
ENDIF ()


SET (ENABLE_RTSPEC OFF)
IF (ENABLE_DL AND ENABLE_MT AND ENABLE_UUID AND ENABLE_CRYPT)
  SET (ENABLE_RTSPEC ON)
ENDIF ()

# Check if sbrk exists which yields ENABLE_PHM
SET (ENABLE_PHM  OFF)
SET (CAN_USE_PHM "0")
SET (SBRK_T)
CHECK_FUNCTION_EXISTS ("sbrk" HAVE_SBRK)
IF (HAVE_SBRK)
  SET (ENABLE_PHM  ON)
  SET (CAN_USE_PHM "1")
  # FIXME Select the right type from "intptr_t ptrdiff_t ssize_t int"
  # currently this is just a hack.
  SET (SBRK_T      "intptr_t")
ENDIF ()

# Check if "%p" fomat of printf routines prints "0x"
# in the beginning of the number.  NEED_PTR_PREFIX will
# be set to either ON or OFF.
CHECK_C_SOURCE_RUNS ("
#include <stdio.h>
#include <string.h>
int main (void)
{
  char buf[128];
  sprintf (buf, \"%p\", (void*)0);
  if (!strncmp (buf, \"0x\", 2))
    return 0;
  else
    return 1;
}
" NEED_PTR_PREFIX)


# Check if stderr is a constant.  NOTE: This is dbug.c crazyness.
CHECK_C_SOURCE_COMPILES ("
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

FILE *_db_fp_ = stderr;
" STDERR_IS_CONSTANT)


# FIXME This doesn't check if CLOCK_PROCESS_CPUTIME_ID is supported
CHECK_LIBRARY_EXISTS ("rt" "clock_gettime" "" HAVE_GETTIME)


# Check compiler identities variables.
# FIXME  CMake has builtin flag CMAKE_C_COMPILER_ID
#        so may be it would be more elegant solution.
CHECK_COMPILER (__SUNPRO_C  SUNC)
CHECK_COMPILER (__DECC  DECC)
CHECK_COMPILER (__APPLE__  MACC)



# Check tools which required for compilation.
# Find xslt processor.
FIND_PROGRAM (XSLT_EXEC
  NAME    xsltproc
  HINTS   ENV PATH
  DOC     "Chek for xslt processor")
IF (NOT XSLT_EXEC)
  MESSAGE (FATAL_ERROR "Cannot find a suitable xslt processor")
ENDIF ()

# Find m4 preprocessor.
FIND_PROGRAM (M4_EXEC
  NAME    m4 gm4
  HINTS   ENV PATH
  DOC     "Check for m4 processor")
IF (NOT M4_EXEC)
  MESSAGE (FATAL_ERROR "Cannot find a suitable m4 processor")
ENDIF ()

# Check for mutc
FIND_PROGRAM (MUTC_EXEC
  NAME    slc
  HINTS   ENV PATH
  DOC     "Check for Mutc slc.")

# Check for the dot program in case we are compiling with dot support.
IF (DOT)
    FIND_PROGRAM (DOT_CMD
        NAME    dot
        HINTS   ENV PATH
        DOC     "Dot graph visualizer")
    # FIXME the name DOT_FLAG is moronic.
    SET (DOT_FLAG   ON)
ELSE ()
    SET (DOT_FLAG   OFF)
ENDIF ()

# Cuda
# FIXME  Make sure that it is enough of a check.
FIND_PACKAGE (CUDA)
SET (ENABLE_CUDA OFF)
SET (CUDA_ARCH)
IF (CUDA_FOUND)
  SET (ENABLE_CUDA ON)
  SET (CUDA_ARCH  "-arch=sm20")
ENDIF ()

# Indenting the code
SET (CB "${PROJECT_BINARY_DIR}/cb")
FIND_PROGRAM (INDENT_EXEC
  NAME        indent  gindent
  HINTS       ENV PATH
  DOC         "Indent the C code")
IF (INDENT_EXEC)
  EXECUTE_PROCESS (COMMAND ${INDENT_EXEC} --version OUTPUT_VARIABLE indent_v)
  IF (${indent_v} MATCHES "GNU indent")
    SET (CB ${INDENT_EXEC})
  ENDIF ()
ENDIF ()
MESSAGE (STATUS "Using `${CB}' for code-beautification")



# Set variables depending on the tools.
SET (ENABLE_MUTC OFF)
IF (MUTC_EXEC)
  SET (ENABLE_MUTC ON)
ENDIF ()

SET (XSLT   "${XSLT_EXEC}")
SET (M4     "${M4_EXEC}")
SET (DOT    "${DOT_FLAG}")

# Get GIT version.
FIND_PACKAGE (Git)
IF (NOT GIT_FOUND)
    MESSAGE (FATAL_ERROR "Could not find GIT on the system!")
ENDIF ()

# Misc variables
# FIXME  Try to get the file-path lengths from the correct include-files.
SET (MAX_PATH_LEN       10000)   # This is bullshit by the way, limits.h have PATH_MAX
SET (MAX_FILE_NAME      256)
SET (PF_MAXFUN          100)
SET (PF_MAXFUNAP        100)
SET (PF_MAXFUNNAMELEN   100)
SET (SAC_PRELUDE_NAME   "sacprelude")

# Get current date and time.
CURRENT_TIME (DATE)

# Get the name of the machine we are compiling sac2c on.
SITE_NAME (HOST_NAME)

# Get current user.
# FIXME why the hell this is useful?
GET_USERNAME (USER_NAME)

# XXX(artem) This is an old way of generating MD5.  Several lines down we
#            Introduced the CMake-based MD5 sum generation, with an extra
#            dependency on the ast.xml.  This allows us to get rid of local
#            MD5 dependency.
## SET (MD5CALC)
## EXECUTE_PROCESS (
##   COMMAND   ${XSLT_EXEC}  "${XMLDIR}/ast2fingerprint.xsl" "${XMLDIR}/ast.xml"
##   COMMAND   "${PROJECT_BINARY_DIR}/md5"
##   OUTPUT_VARIABLE AST_MD5
## )


# Get an md5 hash of the `ast.xml'.
EXECUTE_PROCESS (
    COMMAND
        ${XSLT_EXEC}
        "${PROJECT_SOURCE_DIR}/src/libsac2c/xml/ast2fingerprint.xsl"
        "${PROJECT_SOURCE_DIR}/src/libsac2c/xml/ast.xml"
    OUTPUT_FILE "${CMAKE_BINARY_DIR}/__ast-xml-fingerprint")
FILE (MD5 "${CMAKE_BINARY_DIR}/__ast-xml-fingerprint" NEW_AST_MD5)
# Create a configure dependency that will trigger cmake reconfiguration
# every time when ast.xml is touched.
# XXX This might be too heavy-weight, as we might not need to rebuild
# everything, but I am not sure how to figure out which targets exactly
# to rebuild.
CONFIGURE_FILE (${PROJECT_SOURCE_DIR}/src/libsac2c/xml/ast.xml
                ${PROJECT_BINARY_DIR}/src/libsac2c/xml/ast.xml)


SET (CC  ${CMAKE_C_COMPILER})
# FIXME(artem) A better way to get preprocessor command is by replacing
#              <...> strings int he CMAKE_C_CREATE_PREPROCESSED_SOURCE
#              variable.
SET (CPP_CMD "${CMAKE_C_COMPILER} -E ")

SET (OPT_O0      "")
SET (OPT_O1      "")
SET (OPT_O2      "")
SET (OPT_O3      "")
SET (OPT_g       "")
SET (RCCCFLAGS   "")
SET (MKCCFLAGS   "")
SET (PDCCFLAGS   "")
SET (GENPIC      "")
SET (DEPSFLAG    "-M")
SET (CPPFILE     "${CPP_CMD} -C")
SET (CPP         "${CPP_CMD} -P")
# FIXME(artem) These are named differently now in the configure.ac
SET (CCMTLINK    "")
SET (CCDLLINK    "")

IF (CMAKE_COMPILER_IS_GNUCC AND NOT MACC)
  SET (GCC_FLAGS   "")
  CHECK_GCC_FLAG ("-Wall" GCC_FLAGS)
  CHECK_GCC_FLAG ("-Wextra" GCC_FLAGS)
  CHECK_GCC_FLAG ("-Wstrict-prototypes" GCC_FLAGS)
  CHECK_GCC_FLAG ("-Wno-unused-parameter" GCC_FLAGS)
  CHECK_GCC_FLAG ("-march=native" GCC_FLAGS)
  CHECK_GCC_FLAG ("-mtune=native" GCC_FLAGS)
  # FIXME(artem) Can we get these flags from the Pthread checking macro?
  EXECUTE_PROCESS (
    COMMAND ${CC} -pthread
    ERROR_VARIABLE GCC_ERR
  )
  SET (GCC_PTHREADS  "-lpthread")
  IF (NOT ${GCC_ERR} MATCHES ".*unrecognized option.*")
    SET (GCC_PTHREADS  "-pthread")
  ENDIF ()
  SET (OPT_O0       "")
  SET (OPT_O1       "-O1")
  SET (OPT_O2       "-O2")
  SET (OPT_O3       "-O3")
  SET (OPT_g        "-g")
  SET (RCCCFLAGS    "${GCC_FLAGS} -std=gnu99 -pedantic -Wno-unused -fno-builtin")
  SET (MKCCFLAGS    "${GCC_FLAGS} -std=gnu99 -pedantic -g ${FLAGS_LTO}")
  SET (PDCCFLAGS    "${GCC_FLAGS} -std=gnu99 -pedantic -g -O3 -std=c99 ${FLAGS_LTO}")
  SET (GENPIC       "-fPIC -DPIC")
  SET (DEPSFLAG     "-M")
  SET (CPPFILE      "${CPP_CMD} -C -x c")
  SET (CPP          "${CPP_CMD} -P -x c")
  SET (CCMTLINK     "${GCC_PTHREADS}")
  SET (CCDLLINK     "-ldl")
ELSEIF (SUNC)
  SET (OPT_O0        "")
  SET (OPT_O1        "-xO2")
  SET (OPT_O2        "-xO4")
  SET (OPT_O3        "-xO5")
  SET (OPT_g         "-g")
  SET (RCLDFLAGS     "")
  SET (RCCCFLAGS     "-dalign -fsimple -xsafe=mem -xc99=all")
  SET (MKCCFLAGS     "-erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xc99=all")
  SET (PDCCFLAGS     "-erroff=E_CAST_DOESNT_YIELD_LVALUE -g -xO4 -xc99=all -KPIC")
  SET (GENPIC        "-KPIC")
  SET (DEPSFLAG      "-xM")
  SET (CPPFILE       "${CPP_CMD} -C -x c")
  SET (CPP           "${CPP_CMD} -P -x c")
  SET (CCMTLINK      "-lpthread")
  SET (CCDLLINK      "-ldl")
ELSEIF (DECC)
  SET (OPT_O0        "")
  SET (OPT_O1        "-O1")
  SET (OPT_O2        "-O2")
  SET (OPT_O3        "-O3")
  SET (OPT_g         "-g")
  SET (RCLDFLAGS     "")
  SET (RCCCFLAGS     "")
  SET (MKCCFLAGS     "-g")
  SET (PDCCFLAGS     "-g3")
  SET (GENPIC        "")
  SET (DEPSFLAG      "-M")
  SET (CPPFILE       "${CPP_CMD} -C -x c")
  SET (CPP           "${CPP_CMD} -P -x c")
  SET (CCMTLINK      "-pthread")
  SET (CCDLLINK      "-ldl")
ELSEIF (MACC)
  SET (OPT_O0        "")
  SET (OPT_O1        "-O1")
  SET (OPT_O2        "-O2")
  SET (OPT_O3        "-O3")
  SET (OPT_g         "-g")
  SET (RCLDFLAGS     "")
  SET (RCCCFLAGS     "-Wall -no-cpp-precomp -Wno-unused -fno-builtin -std=c99")
  SET (MKCCFLAGS     "-Wall -std=c99 -g")
  SET (PDCCFLAGS     "-std=c99")
  SET (GENPIC        "")
  SET (DEPSFLAG      "-M")
  SET (CPPFILE       "${CPP_CMD} -C -x c")
  SET (CPP           "${CPP_CMD} -P -x c")
  SET (CCMTLINK      "")
  SET (CCDLLINK      "-ldl")
ENDIF ()


# Operating system dependent flags
SET (OSFLAGS)
SET (LD_DYNAMIC)
SET (LD_PATH)
SET (LD_FLAGS)
SET (RANLIB   "${CMAKE_RANLIB}")
IF (${CMAKE_SYSTEM_NAME} MATCHES "Solaris")
  SET (OSFLAGS      "-D__EXTENSIONS__ -D_XPG6 -DMUST_INIT_YY -DPIC")
  SET (LD_DYNAMIC   "-G -dy")
  SET (LD_PATH      "-L%path% -R%path%")
  SET (LD_FLAGS     "-Wl,-z,nodefs,-z,lazyload")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET (OSFLAGS      "-fPIC -DPIC -D_POSIX_SOURCE -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE")
  SET (LD_DYNAMIC   "-shared -Wl,-allow-shlib-undefined -O3 ${FLAGS_LTO}")
  SET (LD_PATH      "-L%path% -Wl,-rpath,%path%")
  SET (LD_FLAGS     "-Wl,-allow-shlib-undefined ${FLAGS_LTO}")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES ".*osf.*")
  SET (OSFLAGS      "-D_OSF_SOURCE")
  SET (LD_DYNAMIC   "")
  SET (LD_PATH      "-L%path%")
  SET (LD_FLAGS     "")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET (ENABLE_PHM   OFF)
  SET (RANLIB       "$RANLIB -c")
  SET (OSFLAGS      "-D_DARWIN_C_SOURCE")
  SET (LD_DYNAMIC   "-undefined suppress -flat_namespace -dynamiclib -install_name '@rpath/%libname%.dylib' ")
  SET (LD_PATH      "-L%path% -Wl,-rpath,%path%")
  SET (LD_FLAGS     "")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES ".*bsd")
  SET (OSFLAGS      "-fPIC -DPIC")
  SET (LD_DYNAMIC   "-shared -Wl,-allow-shlib-undefined ${FLAGS_LTO}")
  SET (LD_PATH      "-L%path% -Wl,-rpath,%path%")
  SET (LD_FLAGS     "${FLAGS_LTO}")
ELSE ()
  SET (OSFLAGS      "-fPIC -DPIC -D_POSIX_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE")
  SET (LD_DYNAMIC   "-dy -shared -Wl,-allow-shlib-undefined")
  SET (LD_PATH      "-L%path% -Wl,-rpath,%path%")
  SET (LD_FLAGS     "-Wl,-allow-shlib-undefined")
ENDIF ()


# Prpeare C flags for the debug version of the compiler
SET (CMAKE_C_FLAGS_DEBUG
  ${CMAKE_C_FLAGS_DEBUG}
  -DSANITYCHECKS
  -DWLAA_DEACTIVATED
  -DAE_DEACTIVATED
  -DTSI_DEACTIVATED
  -DPADT_DEACTIVATED
  -DCHECK_NODE_ACCESS
  -DINLINE_MACRO_CHECKS
  ${MKCCFLAGS}

)

# Prepare C flags for the product version of the compiler
SET (CMAKE_C_FLAGS_RELEASE
  ${CMAKE_C_FLAGS_RELEASE}
  -DDBUG_OFF
  -DPRODUCTION
  -DWLAA_DEACTIVATED
  -DAE_DEACTIVATED
  -DTSI_DEACTIVATED
  -DPADT_DEACTIVATED
  ${PDCCFLAGS}
)

IF (NOT CMAKE_BUILD_TYPE)
    ADD_DEFINITIONS (
        ${CMAKE_C_FLAGS_DEBUG}
    )
ENDIF ()



# These CC flags are always present
ADD_DEFINITIONS (
    ${OSFLAGS}
    -DSHARED_LIB_EXT="${SHARED_LIB_EXT}"
    -Wfatal-errors
    -D_POSIX_C_SOURCE=200809L
)

# These flags will be used by the linker when creating a shared library
# FIXME(artem) Revisit this later.  Mayve we don't need -shared and other
#              flags when defining LD_DYNAMIC as CMake should be smart enough
#              to figure out basic call for building shared library on all
#              the systems.
SET (CMAKE_SHARED_LINKER_FLAGS ${LD_DYNAMIC})

# sac2crc and makefile-related variables
MACRO (SUBST_SAC2CRC_FILE f var)
    CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/setup/${f}.in" "${PROJECT_BINARY_DIR}/${f}")
    FILE (READ "${PROJECT_BINARY_DIR}/${f}" ${var})
ENDMACRO ()

SET (RTPATH_CONF "${CMAKE_INSTALL_PREFIX}/lib/sac2c/${SAC2C_VERSION}/rt")
SET (MODPATH_CONF "${CMAKE_INSTALL_PREFIX}/lib/sac2c/${SAC2C_VERSION}/modlibs")
SET (INCPATH_CONF "${CMAKE_INSTALL_PREFIX}/include/sac2c/${SAC2C_VERSION}")
SET (TREEPATH_CONF "${CMAKE_INSTALL_PREFIX}/libexec/sac2c/${SAC2C_VERSION}")
SET (DLL_DIR "${TREEPATH_CONF}")
SET (DLL_BUILD_DIR "${PROJECT_BINARY_DIR}/lib")
SET (SAC2CRC_DIR  "${CMAKE_INSTALL_PREFIX}/share/sac2c/${SAC2C_VERSION}")
SET (SAC2CRC_CONF  "${CMAKE_INSTALL_PREFIX}/share/sac2c/${SAC2C_VERSION}/sac2crc")
SET (SAC2CRC_BUILD_CONF "${PROJECT_BINARY_DIR}/sac2crc")

# variables for SAC2C invocations
# FIXME We have duplication in libsac-project...
SET (LD_LIB_PATH "${DLL_BUILD_DIR}:$ENV{LD_LIBRARY_PATH}")
SET (DYLD_LIB_PATH "${DLL_BUILD_DIR}:$ENV{LD_LIBRARY_PATH}")
SET (SAC2C_EXTRA_INC
    -I${PROJECT_SOURCE_DIR}/include
    -I${PROJECT_SOURCE_DIR}/src/include
    -I${PROJECT_SOURCE_DIR}/src/libsacphm/heap
    -I${PROJECT_SOURCE_DIR}/src/libsacdistmem/commlib)

# Make sure that all the libraries are found here.
SET (LIBRARY_OUTPUT_PATH "${DLL_BUILD_DIR}")
# Make sure that this directory exists.
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}")


SUBST_SAC2CRC_FILE ("sac2crc.backend.mutc" RCMUTC)
SUBST_SAC2CRC_FILE ("sac2crc.backend.cuda" RCCUDA)
SUBST_SAC2CRC_FILE ("sac2crc.modifiers.cc" RCCC)
SUBST_SAC2CRC_FILE ("sac2crc.modifiers.malloc" RCMALLOC)
SUBST_SAC2CRC_FILE ("sac2crc.modifiers.rcm" RCRCM)
SUBST_SAC2CRC_FILE ("sac2crc.SUN" RCSUN)
SUBST_SAC2CRC_FILE ("sac2crc.X86" RCX86)
SUBST_SAC2CRC_FILE ("sac2crc.ALPHA" RCALPHA)
SUBST_SAC2CRC_FILE ("sac2crc.MAC" RCMAC)
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/setup/sac2crc.pre.in" "${PROJECT_BINARY_DIR}/sac2crc.pre" @ONLY)
CONFIGURE_FILE ("${PROJECT_BINARY_DIR}/sac2crc.pre" "${SAC2CRC_BUILD_CONF}")

# Create files depending on the options.
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xconfig.h.in" "${PROJECT_BINARY_DIR}/include/config.h")
# Create files depending on the options.
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/include/xsacdirs.h.in" "${PROJECT_BINARY_DIR}/include/sacdirs.h")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/libsac2c/global/build.c.in" "${PROJECT_BINARY_DIR}/src/build.c")
#CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/makefiles/config.mkf.in"
#                "${PROJECT_SOURCE_DIR}/src/makefiles/config.mkf")

