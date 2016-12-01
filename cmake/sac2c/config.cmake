## FIXME(artem)
##
## I am collecting here a list of *.m4 files in setup/config that
## were not propertly ported into CMake.
##
##    * check_isl.m4, we don't check for libisl at all!  Figire out whether
##                    this library is still relevant (ask Bob), and port this
##                    file if needed.
##    * check_lex_yacc.m4 -- deprecated.
##    * check_sl.m4 -- I don't know if it is still useful.
##    * check_stdlib.m4 -- Is it useful at all?
##    * check_brackets.m4 -- The check is not properly implemented, port it!
## 

## This files is analogous to configure.ac (*.m4 files) used in the 
## autotools build system.
##
## The configuration is organised as follows:
##TODO


## CMAKE INCLUDES
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckCSourceCompiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckLibraryExists)
INCLUDE (CheckCSourceRuns)
INCLUDE (CheckCCompilerFlag)

## Include other config files
INCLUDE ("cmake/sac2c-version-related.cmake")

# Macros with distmem checks
INCLUDE ("cmake/sac2c/distmem-related.cmake")

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

# Check functions
CHECK_FUNCTION_EXISTS (strtok HAVE_STRTOK)
CHECK_FUNCTION_EXISTS (strrchr HAVE_STRRCHR)
CHECK_FUNCTION_EXISTS (mkdtemp HAVE_MKDTEMP)

# Check if we have clock_gettime and whether we need -lrt
SET (ENABLE_GETTIME OFF)
SET (LIB_RT "")
FIND_LIBRARY (RT_LIB      "rt")
IF (RT_LIB)
    GET_FILENAME_COMPONENT (RT_PATH  ${RT_LIB} PATH)
    CHECK_LIBRARY_EXISTS (${RT_LIB} "clock_gettime" ${RT_PATH} RT_FOUND)
ENDIF ()
IF (RT_LIB AND RT_FOUND)
   CHECK_C_SOURCE_COMPILES ("
       #include <time.h>
       #include <sys/times.h>
       int main (void) {
         struct timespec ts;
         clock_gettime(CLOCK_MONOTONIC, &ts);
         return 0;
       }
   "
   ENABLE_GETTIME
   )
   IF (ENABLE_GETTIME)
      SET (LIB_RT  -Xl -lrt)
   ENDIF ()
ENDIF ()


# Check functions in libs
ASSERT_LIB (M     "m"     "sqrt")

ASSERT_LIB (DL    "dl"    "dlopen")
CHECK_INCLUDE_FILES ("dlfcn.h" HAVE_DLFCN_H)
ENABLE_VAR_IF (ENABLE_DL  HAVE_DLFCN_H AND DL_LIB)


# Decide whether to use uuid or not inside the RTspec
FIND_LIBRARY (UUID_LIB      "uuid")
IF (UUID_LIB)
    GET_FILENAME_COMPONENT (UUID_PATH  ${UUID_LIB} PATH)
    CHECK_LIBRARY_EXISTS (${UUID_LIB} "uuid_generate" ${UUID_PATH} UUID_FOUND)
ENDIF ()
ENABLE_VAR_IF (ENABLE_UUID UUID_LIB AND UUID_FOUND)

CHECK_INCLUDE_FILES ("uuid/uuid.h"  HAVE_UUID_H)
IF (NOT ENABLE_UUID OR NOT HAVE_UUID_H OR NOT UUID_LIB)
    SET (ENABLE_UUID OFF)
ENDIF ()

FIND_LIBRARY (CRYPT_LIB NAMES "crypt")
IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  CHECK_INCLUDE_FILES ("unistd.h" HAVE_CRYPT_H)
ELSE ()
  CHECK_INCLUDE_FILES ("crypt.h" HAVE_CRYPT_H)
ENDIF ()
ENABLE_VAR_IF (ENABLE_HASH  HAVE_CRYPT_H AND CRYPT_LIB)

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
SET (CCMT_PTH_CFLAGS "")
SET (CCMT_PTH_LINK "")
IF (MT)
    # Prefer -pthread over -lpthread and stuff.
    SET (THREADS_PREFER_PTHREAD_FLAG ON)
    FIND_PACKAGE (Threads)
    IF (THREADS_FOUND AND NOT WIN32)
        MESSAGE (STATUS "Enabling MT")
        SET (ENABLE_MT  ON)
        IF (THREADS_HAVE_PTHREAD_ARG)
          # XXX(artem) dobule check that it substitutes adeauately with
          #            non GCC compilers.
          SET (CCMT_PTH_CFLAGS " -pthread")
          SET (CCMT_PTH_LINK "${CMAKE_THREAD_LIBS_INIT}")
        ENDIF ()
    ENDIF ()
ENDIF ()

# If option OMP is set.
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

# Check and see if we can enable RTSPEC
SET (ENABLE_RTSPEC OFF)
IF (ENABLE_DL AND ENABLE_MT AND ENABLE_UUID AND ENABLE_HASH)
  SET (ENABLE_RTSPEC ON)
ELSE ()
  # We do this to prevent the compilation of rtspec components
  UNSET(ENABLE_HASH)
  UNSET(ENABLE_UUID)
ENDIF ()

# Check if sbrk exists which yields ENABLE_PHM
SET (CAN_USE_PHM "0")
SET (SBRK_T)
IF (PHM)
  CHECK_FUNCTION_EXISTS ("sbrk" HAVE_SBRK)
  IF (HAVE_SBRK)
    SET (CAN_USE_PHM "1")
    # FIXME Select the right type from "intptr_t ptrdiff_t ssize_t int"
    # currently this is just a hack.
    SET (SBRK_T      "intptr_t")
  ELSE ()
    SET (PHM OFF)
  ENDIF ()
ENDIF ()

# Check if "%p" format of printf routines prints "0x"
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


# Check compiler identities variables.
# FIXME  CMake has builtin flag CMAKE_C_COMPILER_ID
#        so may be it would be more elegant solution.
CHECK_COMPILER (__SUNPRO_C  SUNC)
CHECK_COMPILER (__DECC  DECC)
CHECK_COMPILER (__APPLE__  MACC)
CHECK_COMPILER (__clang__ CLANG)

# Check tools which required for compilation.
# Find BASH interpreter.
FIND_PROGRAM (BASH_EXEC
  NAME    bash
  HINTS   ENV PATH
  DOC     "Check for bash interpreter")
IF (NOT BASH_EXEC)
  MESSAGE (FATAL_ERROR "Cannot find a bash interpreter")
ENDIF ()
SET (BASH "${BASH_EXEC}")

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
FIND_PACKAGE (CUDA)
SET (ENABLE_CUDA OFF)
SET (CUDA_ARCH)
IF (CUDA_FOUND)
  SET (ENABLE_CUDA ON)
  SET (CUDA_ARCH  "-arch=sm_20") # TODO: update sac2c to make better use of newer architectures...
  SET (CUDA_ROOT  "${CUDA_TOOLKIT_ROOT_DIR}") # FIXME: switch to package var?
  SET (NVCC_PATH  "${CUDA_TOOLKIT_ROOT_DIR}/bin/nvcc")
ENDIF ()

# Indenting the code
SET (CB "${PROJECT_BINARY_DIR}/cb")
FIND_PROGRAM (INDENT_EXEC
  NAME        indent  gindent
  HINTS       ENV PATH
  DOC         "Indent the C code")
IF (INDENT_EXEC)
  EXECUTE_PROCESS (COMMAND ${INDENT_EXEC} --version OUTPUT_VARIABLE indent_v ERROR_QUIET)
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

# Check if we should add distmem support...
MESSAGE(STATUS "Checking for Distmem backends (MPI, ARMCI, GASNET, GPI)")
CHECK_DISTMEM_MPI()
CHECK_DISTMEM_ARMCI()
CHECK_DISTMEM_GASNET()
CHECK_DISTMEM_GPI()

SET(ENABLE_DISTMEM OFF)
IF((ENABLE_DISTMEM_BACKEND_MPI OR ENABLE_DISTMEM_BACKEND_ARMCI
    OR ENABLE_DISTMEM_BACKEND_GASNET OR ENABLE_DISTMEM_BACKEND_GPI)
    AND DISTMEM)
  SET(ENABLE_DISTMEM ON)
ENDIF ()

# Misc variables
# FIXME  Try to get the file-path lengths from the correct include-files.
SET (MAX_PATH_LEN       10000)   # This is bullshit by the way, limits.h have PATH_MAX
SET (MAX_FILE_NAME      256)
SET (PF_MAXFUN          100)
SET (PF_MAXFUNAP        100)
SET (PF_MAXFUNNAMELEN   100)

# Get current date and time.
STRING (TIMESTAMP DATE)

# Get the name of the machine we are compiling sac2c on.
SITE_NAME (HOST_NAME)

# Get current user.
# FIXME why the hell this is useful?
GET_USERNAME (USER_NAME)

# Get the sac2c repository version
GET_SAC2C_VERSION (SAC2C_VERSION SAC2C_VERSION_MAJOR SAC2C_VERSION_MINOR SAC2C_VERSION_PATCH)

# Get an md5 hash of the `ast.xml'.
EXECUTE_PROCESS (
    COMMAND
        ${XSLT_EXEC}
        "${PROJECT_SOURCE_DIR}/src/libsac2c/xml/ast2fingerprint.xsl"
        "${PROJECT_SOURCE_DIR}/src/libsac2c/xml/ast.xml"
    OUTPUT_FILE "${CMAKE_BINARY_DIR}/__ast-xml-fingerprint")
FILE (MD5 "${CMAKE_BINARY_DIR}/__ast-xml-fingerprint" AST_MD5)

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

IF ((CMAKE_COMPILER_IS_GNUCC OR CLANG) AND (NOT MACC))
  SET (GCC_FLAGS   "")
  SET (GCC_ARCH_FLAGS  "")
  CHECK_CC_FLAG ("-Wall" GCC_FLAGS)
  CHECK_CC_FLAG ("-Wextra" GCC_FLAGS)
  CHECK_CC_FLAG ("-Wstrict-prototypes" GCC_FLAGS)
  CHECK_CC_FLAG ("-Wno-unused-parameter" GCC_FLAGS)
  CHECK_CC_FLAG ("-march=native" GCC_ARCH_FLAGS)
  CHECK_CC_FLAG ("-mtune=native" GCC_ARCH_FLAGS)
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
  # FIXME (hans): we currently are using these flags for building the compiler as well as
  #               the SAC sources - which it not optimal for packaging
  SET (RCCCFLAGS    "${GCC_FLAGS} ${GCC_ARCH_FLAGS} -std=gnu99 -pedantic -Wno-unused -fno-builtin")
  SET (MKCCFLAGS    "${GCC_FLAGS} ${GCC_ARCH_FLAGS} -std=gnu99 -pedantic -g ${FLAGS_LTO}")
  SET (DEV_FLAGS    "${GCC_FLAGS} -mtune=generic -std=gnu99 -pedantic -g ${FLAGS_LTO}")
  SET (PDCCFLAGS    "${GCC_FLAGS} ${GCC_ARCH_FLAGS} -std=gnu99 -pedantic -g -O3 -std=c99 ${FLAGS_LTO}")
  SET (PROD_FLAGS   "${GCC_FLAGS} -mtune=generic -std=gnu99 -pedantic -g -O3 -std=c99 ${FLAGS_LTO}")
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
  SET (DEV_FLAGS     "${MKCCFLAGS}")
  SET (PROD_FLAGS    "${PDCCFLAGS}")
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
  SET (DEV_FLAGS     "${MKCCFLAGS}")
  SET (PROD_FLAGS    "${PDCCFLAGS}")
  SET (GENPIC        "")
  SET (DEPSFLAG      "-M")
  SET (CPPFILE       "${CPP_CMD} -C -x c")
  SET (CPP           "${CPP_CMD} -P -x c")
  SET (CCMTLINK      "-pthread")
  SET (CCDLLINK      "-ldl")
ELSEIF (MACC)
  SET (CC_FLAGS   "")
  # TODO(artem) Check whether this helps to handle the bracket error!
  IF ("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
     CHECK_CC_FLAG ("-fbracket-depth=2048" CC_FLAGS)
  ENDIF ()
  SET (OPT_O0        "")
  SET (OPT_O1        "-O1")
  SET (OPT_O2        "-O2")
  SET (OPT_O3        "-O3")
  SET (OPT_g         "-g")
  SET (RCLDFLAGS     "")
  SET (RCCCFLAGS     "${CC_FLAGS} -Wall -no-cpp-precomp -Wno-unused -fno-builtin -std=c99")
  SET (MKCCFLAGS     "${CC_FLAGS} -Wall -std=c99 -g")
  SET (PDCCFLAGS     "${CC_FLAGS} -std=c99")
  SET (DEV_FLAGS     "${MKCCFLAGS}")
  SET (PROD_FLAGS    "${PDCCFLAGS}")
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


# A list of supported build types.  Every supported build type must
# come with a postfix and a set of C flags that must be defined in
# variables CMAKE_<build-type>_POSTFIX and CMAKE_C_FLAGS_<build-type>
SET (KNOWN_BUILD_TYPES "DEBUG;RELEASE")
IF (NOT CMAKE_BUILD_TYPE)
  SET (CMAKE_BUILD_TYPE "DEBUG")
ENDIF ()

# Prepare C flags for the debug version of the compiler
SET (CMAKE_C_FLAGS_DEBUG 
     "${CMAKE_C_FLAGS_DEBUG} -DSANITYCHECKS -DWLAA_DEACTIVATED \
      -DAE_DEACTIVATED -DTSI_DEACTIVATED -DPADT_DEACTIVATED \
      -DCHECK_NODE_ACCESS -DINLINE_MACRO_CHECKS ${DEV_FLAGS}")

# Prepare C flags for the product version of the compiler
SET (CMAKE_C_FLAGS_RELEASE
     "${CMAKE_C_FLAGS_RELEASE} -DDBUG_OFF -DPRODUCTION \
      -DWLAA_DEACTIVATED -DAE_DEACTIVATED -DTSI_DEACTIVATED \
      -DPADT_DEACTIVATED ${PROD_FLAGS}")

SET (CMAKE_RELEASE_POSTFIX "_p")
SET (CMAKE_DEBUG_POSTFIX "_d")

INCLUDE ("cmake/sac2c/buildtype-related.cmake")

# Setting build-type-related variables
SET (BUILD_TYPE_POSTFIX "${CMAKE_${CMAKE_BUILD_TYPE}_POSTFIX}")
SET (BUILD_TYPE_C_FLAGS "${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}")
STRING (TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_NAME)

# Configure sac2c-version-manager script
INCLUDE ("cmake/sac2c/version-manager-related.cmake")

# These CC flags that are always present
ADD_DEFINITIONS (
    ${OSFLAGS}
    -DSHARED_LIB_EXT="${SHARED_LIB_EXT}"
    -DBUILD_TYPE_POSTFIX="${BUILD_TYPE_POSTFIX}"
    -Wfatal-errors # in GCC 4.4.7 this leads to a false error
    -D_POSIX_C_SOURCE=200809L
)

# These flags will be used by the linker when creating a shared library
# FIXME(artem) Revisit this later.  Maybe we don't need -shared and other
#              flags when defining LD_DYNAMIC as CMake should be smart enough
#              to figure out basic call for building shared library on all
#              the systems.
SET (CMAKE_SHARED_LINKER_FLAGS ${LD_DYNAMIC})

# these are the install paths, they are *intentionally* not absolute! This
# is to make package relocatable - be aware that CMAKE_INSTALL_PREFIX is used
# for `make install' whereas CPACK_PACKAGE_INSTALL_PREFIX is used by CPack. Both
# are automatically added to the install paths below - they *do not* need to be
# explicitly added!
SET (RTPATH_INSTALL "${PACKAGE_PREFIX}lib/sac2c/${SAC2C_VERSION}/rt")
SET (MODPATH_INSTALL "${PACKAGE_PREFIX}lib/sac2c/${SAC2C_VERSION}/modlibs")
SET (INCPATH_INSTALL "${PACKAGE_PREFIX}include/sac2c/${SAC2C_VERSION}/${BUILD_TYPE_NAME}")
SET (TREEPATH_INSTALL "${PACKAGE_PREFIX}libexec/sac2c/${SAC2C_VERSION}")
SET (SAC2CRC_INSTALL  "${PACKAGE_PREFIX}share/sac2c/${SAC2C_VERSION}")
SET (SOURCE_INSTALL  "${PACKAGE_PREFIX}src")
SET (INSTALLER_INSTALL  "${PACKAGE_PREFIX}installers")
SET (TOP_INSTALL     "${PACKAGE_PREFIX}.")

# FIXME (hans): clean this up!?
SET (RTPATH_CONF "${CMAKE_INSTALL_PREFIX}/lib/sac2c/${SAC2C_VERSION}/rt")
SET (MODPATH_CONF "${CMAKE_INSTALL_PREFIX}/lib/sac2c/${SAC2C_VERSION}/modlibs")
SET (INCPATH_CONF "${CMAKE_INSTALL_PREFIX}/include/sac2c/${SAC2C_VERSION}/${BUILD_TYPE_NAME}")
SET (TREEPATH_CONF "${CMAKE_INSTALL_PREFIX}/libexec/sac2c/${SAC2C_VERSION}")
SET (DLL_DIR "${TREEPATH_CONF}")
SET (DLL_BUILD_DIR "${PROJECT_BINARY_DIR}/lib")
SET (SAC2CRC_DIR  "${CMAKE_INSTALL_PREFIX}/share/sac2c/${SAC2C_VERSION}")
SET (SAC2CRC_CONF  "${CMAKE_INSTALL_PREFIX}/share/sac2c/${SAC2C_VERSION}/sac2crc${BUILD_TYPE_POSTFIX}")
SET (SAC2CRC_BUILD_CONF "${PROJECT_BINARY_DIR}/sac2crc${BUILD_TYPE_POSTFIX}")
SET (SAC_PRELUDE_NAME   "sacprelude${BUILD_TYPE_POSTFIX}")

# Make sure that all the libraries are found here.
SET (LIBRARY_OUTPUT_PATH "${DLL_BUILD_DIR}")
# Make sure that this directory exists.
FILE (MAKE_DIRECTORY "${DLL_BUILD_DIR}")

# Print out configuration information
#FIXME(hans): we are missing a few details here.
MESSAGE ("
* Configuration done.
*
* Detected OS string:      ${CMAKE_SYSTEM}
* Detected CPU string:     ${CMAKE_SYSTEM_PROCESSOR}
* 
* CMake Generator:         ${CMAKE_GENERATOR}
* CMake Varient:           ${CMAKE_BUILD_TYPE}
*
* Run-time specialization: ${ENABLE_RTSPEC}
* Private heap manager:    ${PHM}
* Back-ends:
* - MT/pthread:            ${ENABLE_MT}
* - MT/LPEL:               ${ENABLE_MT_LPEL}
* - CUDA:                  ${ENABLE_CUDA}
* - OpenMP:                ${ENABLE_OMP}
* - SL:                    ${ENABLE_SL}
* - Distributed memory:    ${ENABLE_DISTMEM}$distmem_details_print
* => distmen is still non functional <=
* - CC:                    ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID})
* - CCFLAGS:               ${BUILD_TYPE_C_FLAGS}
* - SaC compiler CFLAGS:   ${MKCCFLAGS}
* - SaC programs CFLAGS:   ${RCCCFLAGS}
*
* Optional packages:
* - Integer Set Library (ISL): ${ENABLE_ISL}
*
")

# vim: ts=2 sw=2
