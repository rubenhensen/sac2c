INCLUDE (CheckIncludeFiles)
INCLUDE (CheckCSourceCompiles)
INCLUDE (CheckFunctionExists) 
INCLUDE (CheckLibraryExists) 
INCLUDE (CheckCSourceRuns)



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
MACRO (ENABLE_IF  COND  FLAG)
  SET (${FLAG} OFF)
  IF (${COND})
    SET (${FLAG} ON)
  ENDIF ()
ENDMACRO ()


# System-dependent variables.
SET (OS       "${CMAKE_SYSTEM}")
SET (ARCH     "${CMAKE_SYSTEM_NAME}")



# Check headers.
CHECK_INCLUDE_FILES (dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILES (memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILES (pthread.h HAVE_PTHREAD_H)
CHECK_INCLUDE_FILES (omp.h HAVE_OMP_H)



# Check functions
CHECK_FUNCTION_EXISTS (strtok HAVE_STRTOK)
CHECK_FUNCTION_EXISTS (strrchr HAVE_STRRCHR)
CHECK_FUNCTION_EXISTS (mkdtemp HAVE_MKDTEMP)



# Check functions in libs
# FIXME What happens if there is not `m', or `dl'?
# Do we stop the compilation, or we try to recover somehow?
SET (LIBS)
FIND_LIBRARY (M_PATH "m")
CHECK_LIBRARY_EXISTS ("m" "sqrt" ${M_PATH} m_sqrt_found)
IF (${m_sqrt_found})
  SET (LIBS "${LIBS} -lm")
ENDIF ()

FIND_LIBRARY (DL_PATH "dl")
CHECK_LIBRARY_EXISTS ("dl" "dlopen" ${DL_PATH} dl_dlopen_found)
IF (${dl_dlopen_found})
  SET (LIBS "${LIBS} -ldl")
ENDIF ()

FIND_LIBRARY (UUID_PATH "uuid")
CHECK_LIBRARY_EXISTS ("uuid" "uuid_generate" ${UUID_PATH} uuid_generate_found)
IF (${uuid_generate_found})
  SET (LIBS "${LIBS} -luuid")
ENDIF ()




# Options which depend on availability of header-files and functions
ENABLE_IF (HAVE_DLFCN_H ENABLE_DL)
ENABLE_IF (HAVE_PTHREAD_H  ENABLE_MT)
ENABLE_IF (HAVE_OMP_H  ENABLE_OMP)
SET (ENABLE_RTSPEC OFF)
IF (ENABLE_DL AND ENABLE_MT)
  SET (ENABLE_RTSPEC ON)
ENDIF ()

# Check if sbrk exists which yields ENABLE_PHM
SET (ENABLE_PHM  OFF)
SET (SBRK_T)
CHECK_FUNCTION_EXISTS ("sbrk" HAVE_SBRK)
IF (HAVE_SBRK)
  SET (ENABLE_PHM  ON)
  SET (SBRK_T      "intptr_t")
ENDIF ()

# Check if "%p" fomat of printf routines prints "0x"
# in the beginning of the number.  NEED_PTR_PREFIX will
# be set to either ON or OFF.
CHECK_C_SOURCE_RUNS ("
#include <stdio.h>
#include <string.h>
int main ()
{
  char buf[128];
  sprintf (buf, \"%p\", (void*)0);
  if (!strncmp (buf, \"0x\", 2))
    return 1;
  else
    return 0;
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



# Check tools which required for compilation.
# Find xslt processor.
FIND_PROGRAM (XSLT_EXEC
  NAME	  xsltproc
  HINTS   ENV PATH
  DOC	  "Chek for xslt processor")
IF (NOT XSLT_EXEC)
  MESSAGE (FATAL_ERROR "Cannot find a suitable xslt processor")
ENDIF ()

# Find m4 preprocessor.
FIND_PROGRAM (M4_EXEC
  NAME    m4 gm4
  HINTS   ENV PATH
  DOC	  "Chek for m4 processor")
IF (NOT M4_EXEC)
  MESSAGE (FATAL_ERROR "Cannot find a suitable m4 processor")
ENDIF ()

# Check for mutc
FIND_PROGRAM (MUTC_EXEC
  NAME    slc
  HINTS   ENV PATH
  DOC	  "Chek for Mutc slc.")

# Check for options meeting requirements.
# Dot program
FIND_PROGRAM (DOT_FLAG
  NAME    dot
  HINTS   ENV PATH
  DOC	  "Dot graph visualizer")

# Cuda 
# FIXME  Make sure that it is enough of a check.
FIND_PACKAGE (CUDA)
SET (ENABLE_CUDA OFF)
SET (CUDA_ARCH)
IF (CUDA_FOUND)
  SET (ENABLE_CUDA ON)
  MATH (EXPR version_number "${CUDA_VERSION_MAJOR} * 10 + ${CUDA_VERSION_MINOR}")
  SET (CUDA_ARCH  "-arch=${version_number}")
ENDIF ()

# Indenting the code
SET (CB "${PROJECT_BINARY_DIR}/cb")
FIND_PROGRAM (INDENT_EXEC  
  NAME	  indent  gindent
  HINTS	  ENV PATH
  DOC	  "Indent the C code")
IF (INDENT_EXEC)
  EXECUTE_PROCESS (COMMAND ${INDENT_EXEC} --version OUTPUT_VARIABLE indent_v)
  IF (${indent_v} MATCHES "GNU indent")
    SET (CB ${INDENT_EXEC})
  ENDIF ()
ENDIF ()



# Set variables depending on the tools.
SET (ENABLE_MUTC OFF)
IF (MUTC_EXEC)
  SET (ENABLE_MUTC ON)
ENDIF ()

SET (XSLT   "${XSLT_EXEC}")
SET (M4     "${M4_EXEC}")
SET (DOT    "${DOT_FLAG}")




# Misc variables
# FIXME  Try to get the file-path lengths from the correct include-files.
SET (MAX_PATH_LEN	10000)   # This is bullshit by the way, limits.h have PATH_MAX
SET (MAX_FILE_NAME      256)
SET (PF_MAXFUN          100)
SET (PF_MAXFUNAP        100)
SET (PF_MAXFUNNAMELEN   100)
SET (SAC_PRELUDE_NAME   "sacprelude")



# Configuration variables for build.c.in
# NOTE:  Macros which return user, host and date are
#        properly implemented for unix systems only.
#        It would not fall with an error for other 
#        systems, but the data returned would be fake.

# Get current date and time.
MACRO (GET_DATE RES)
  IF (UNIX)
    EXECUTE_PROCESS(COMMAND "date" OUTPUT_VARIABLE ${RES})
  ELSE ()
    MESSAGE(SEND_ERROR "Date not implemented on the given platform")
    SET(${RES} "<date-unknown>")
  ENDIF ()
  STRING (REGEX REPLACE "\n" "" ${RES} ${${RES}})
ENDMACRO ()

# Get the name of the machine.
MACRO (GET_HOSTNAME RES)
  IF (UNIX)
    EXECUTE_PROCESS(COMMAND "hostname" OUTPUT_VARIABLE ${RES})
  ELSE ()
    MESSAGE(SEND_ERROR "`hostname' not implemented on the given platform")
    SET(${RES} "<host-unknown>")
  ENDIF ()
  STRING (REGEX REPLACE "\n" "" ${RES} ${${RES}})
ENDMACRO ()

# Get current user.
MACRO (GET_USERNAME RES)
  IF (UNIX)
    EXECUTE_PROCESS(COMMAND "whoami" OUTPUT_VARIABLE ${RES})
  ELSE ()
    MESSAGE(SEND_ERROR "`hostname' not implemented on the given platform")
    SET(${RES} "<user-unknown>")
  ENDIF ()
  STRING (REGEX REPLACE "\n" "" ${RES} ${${RES}})
ENDMACRO ()

# Get svn version.
#INCLUDE (FindSubversion)
#Subversion_WC_INFO (${PROJECT_SOURCE_DIR} SVN)

# FIXME get this from git!
SET (SVN_WC_REVISION "test-version")

# FIXME build-style should come from options.
SET (BUILD_STYLE  "developer")
GET_DATE (DATE)
GET_HOSTNAME (HOST_NAME)
GET_USERNAME (USER_NAME)
SET (REVISION ${SVN_WC_REVISION})
SET (REVISION_NUMBER ${SVN_WC_REVISION})

# Get an md5 hash of the `ast.xml'.
SET (XMLDIR  "${PROJECT_SOURCE_DIR}/src/libsac2c/xml")
SET (MD5CALC )
EXECUTE_PROCESS (
  COMMAND   ${XSLT_EXEC}  "${XMLDIR}/ast2fingerprint.xsl" "${XMLDIR}/ast.xml" 
  COMMAND   "${PROJECT_BINARY_DIR}/md5"
  OUTPUT_VARIABLE AST_MD5
)



# sac2crc and makefile-related variables
FILE (READ "${PROJECT_SOURCE_DIR}/setup/sac2crc.SUN" RCSUN)
FILE (READ "${PROJECT_SOURCE_DIR}/setup/sac2crc.X86" RCX86)
FILE (READ "${PROJECT_SOURCE_DIR}/setup/sac2crc.ALPHA" RCALPHA)
FILE (READ "${PROJECT_SOURCE_DIR}/setup/sac2crc.MAC" RCMAC)
SET (CC  ${CMAKE_C_COMPILER})

SET (OPT_O0)
SET (OPT_O1)
SET (OPT_O2)
SET (OPT_O3)
SET (OPT_g)
SET (RCLDFLAGS)
SET (RCCCFLAGS)
SET (MKCCFLAGS)
SET (PDCCFLAGS)
SET (GENPIC)
SET (DEPSFLAG)
SET (CPPFILE)
SET (CCMTLINK)
SET (CCDLLINK)
IF (CMAKE_COMPILER_IS_GNUCC AND NOT MACC)
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
  SET (RCLDFLAGS    "")
  SET (RCCCFLAGS    "-Wall -Wno-unused -fno-builtin -std=c99")
  SET (MKCCFLAGS    "-Wall -g -std=c99")
  SET (PDCCFLAGS    "-Wall -g -O3 -std=c99")
  SET (GENPIC	    "-fPIC -DPIC")
  SET (DEPSFLAG     "-M")
  SET (CPPFILE	    "gcc -E -C -x c")
  SET (CCMTLINK     "${GCC_PTHREADS}")
  SET (CCDLLINK     "-ldl")
ELSEIF (SUNC)
  SET (OPT_O0	      "")
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
  SET (CPPFILE       "cpp -E -C -x c")
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
  SET (CPPFILE       "cc -E -C -x c")
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
  SET (CPPFILE       "gcc -E -C -x c")
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
  SET (OSFLAGS      "-fPIC -DPIC -D_POSIX_SOURCE -D_SVID_SOURCE -D_BSD_SOURCE -Dlint")
  SET (LD_DYNAMIC   "-dy -shared -Wl,-allow-shlib-undefined")
  SET (LD_PATH      "-L%path% -Wl,-rpath,%path%")
  SET (LD_FLAGS     "-Wl,-allow-shlib-undefined")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES ".*osf.*") 
  SET (OSFLAGS      "-D_OSF_SOURCE")
  SET (LD_DYNAMIC   "")
  SET (LD_PATH      "-L%path%")
  SET (LD_FLAGS     "")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET (ENABLE_PHM   OFF)
  SET (RANLIB       "$RANLIB -c")
  IF (${CMAKE_SYSTEM_VERSION} MATCHES "10.")
    SET (OSFLAGS    "-no-cpp-precomp -D_DARWIN_C_SOURCE")
    SET (LD_DYNAMIC "-undefined suppress -flat_namespace -dynamiclib -install_name '@rpath/%libname%' ")
    SET (LD_PATH    "-L%path% -Wl,-rpath,%path%")
    SET (LD_FLAGS   "")
  ELSEIF (${CMAKE_SYSTEM_VERSION} MATCHES "9.")
    SET (OSFLAGS    "-no-cpp-precomp -Wno-long-double -D_DARWIN_C_SOURCE")
    SET (LD_DYNAMIC "-undefined suppress -flat_namespace -dynamiclib -install_name '@rpath/%libname%' ")
    SET (LD_PATH    "-L%path% -Wl,-rpath,%path%")
    SET (LD_FLAGS   "")
  ELSEIF (${CMAKE_SYSTEM_VERSION} MATCHES "[6-8].")
    SET (OSFLAGS    "-no-cpp-precomp -Wno-long-double -D_DARWIN_C_SOURCE")
    SET (LD_DYNAMIC "-undefined suppress -flat_namespace -bundle")
    SET (LD_PATH    "-L%path%")
    SET (LD_FLAGS   "")
  ENDIF ()
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES ".*bsd")
  SET (OSFLAGS      "-fpic")
  SET (LD_DYNAMIC   "ld -dy -shared -symbolic")
  SET (LD_PATH      "-L -Wl,-rpath,%path%")
  SET (LD_FLAGS     "")
ENDIF ()



# Create files depending on the options.
CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/src/include/xconfig.h.in"
  "${PROJECT_BINARY_DIR}/include/config.h"
)

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/src/libsac2c/global/build.c.in"
  "${PROJECT_BINARY_DIR}/src/build.c"
)

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/setup/sac2crc.in"
  "${PROJECT_SOURCE_DIR}/sac2crc"
)

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/src/makefiles/config.mkf.in"
  "${PROJECT_SOURCE_DIR}/src/makefiles/config.mkf"
)
