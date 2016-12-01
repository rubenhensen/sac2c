# This file contains definitions for dismemr-related macros
# and functions that we use during configure stage.

# Check for support of MPI
MACRO(CHECK_DISTMEM_MPI)
  MESSAGE(STATUS "Checking for MPI support")
  SET(ENABLE_DISTMEM_BACKEND_MPI OFF)
  FIND_PACKAGE(MPI)
  #FIXME(hans): we should also handle the case when we
  # are compiling with CXX.
  IF(MPI_C_FOUND)
    MESSAGE(STATUS "MPI Compiler: ${MPI_C_COMPILER}")
    #XXX(hans): Normally we would use CHECK_C_SOURCE_COMPILES
    # or similar to do this, but CMAKE is extremely set on using
    # the project default compiler and does not allow at runtime
    # changes to this setting. As such we verbosely do in essence
    # what CHECK_C_SOURCE_COMPILES does...
    #XXX(hans): perhaps turn this into a macro? Something to push
    # upstream to CMAKE?
    FILE(WRITE "${PROJECT_BINARY_DIR}/mpi-test.c"
      "#include <mpi.h>

      /* This program uses MPI 3 one-sided communication to test
      whether the MPI installation does support these operations. */
      int main(int argc, char *argv[]) {
      static MPI_Win win = NULL;
      size_t SAC_DISTMEM_pagesz = 0;

      void *local_page_ptr = NULL;
      size_t owner_rank = 0;
      size_t remote_page_index;

      MPI_Get( local_page_ptr, SAC_DISTMEM_pagesz,
        MPI_BYTE, owner_rank,
        remote_page_index * SAC_DISTMEM_pagesz,
        SAC_DISTMEM_pagesz, MPI_BYTE, win);
        }")
    EXECUTE_PROCESS (COMMAND ${MPI_C_COMPILER} "${PROJECT_BINARY_DIR}/mpi-test.c"
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
      RESULT_VARIABLE MPI_COMPILES
      ERROR_FILE "mpi-error.log")
    #FILE(REMOVE "${PROJECT_BINARY_DIR}/mpi-test.c" "${PROJECT_BINARY_DIR}/a.out")
    IF(${MPI_COMPILES} EQUAL 0)
      SET(ENABLE_DISTMEM_BACKEND_MPI ON)
    ENDIF ()
  ENDIF ()
ENDMACRO ()

# Check for ARMCI for Distmem
MACRO (CHECK_DISTMEM_ARMCI)
  MESSAGE (STATUS "Checking for ARMCI")
  SET (ENABLE_DISTMEM_BACKEND_ARMCI OFF)
  # Check if ARMCI_HOME is set
  IF ($ENV{ARMCI_HOME})
    MESSAGE(STATUS "Using ARMCI_HOME: $ENV{ARMCI_HOME}")
    IF (EXISTS "$ENV{ARMCI_HOME}")
      MESSAGE(STATUS "ARMCI_HOME exists...")
      SET (ENABLE_DISTMEM_BACKEND_ARMCI ON)
    ELSE ()
      MESSAGE(STATUS "ARMCI_HOME does not exist.")
    ENDIF ()
  ENDIF ()
ENDMACRO ()

# Check for GPI for Distmem
MACRO (CHECK_DISTMEM_GPI)
  MESSAGE(STATUS "Checking for GPI")
  SET(ENABLE_DISTMEM_BACKEND_GPI OFF)
  # Check if GPI_HOME is set
  IF ($ENV{GPI_HOME})
    MESSAGE(STATUS "Using GPI_HOME: $ENV{GPI_HOME}")
    IF (EXISTS "$ENV{GPI_HOME}")
      MESSAGE(STATUS "GPI_HOME exists...")
      SET (ENABLE_DISTMEM_BACKEND_GPI ON)
    ELSE ()
      MESSAGE(STATUS "GPI_HOME does not exist.")
    ENDIF ()
  ENDIF ()
ENDMACRO ()

# Check for GasNet for Distmem
MACRO (CHECK_DISTMEM_GASNET)
  MESSAGE(STATUS "Checking for GASNET")
  SET(ENABLE_DISTMEM_BACKEND_GASNET OFF)
  # Check if GASNET_HOME is set
  #FIXME(hans): Should this maybe be $GASNETINCLUDE instead
  # of GASNET_HOME?
  IF ($ENV{GASNET_HOME})
    MESSAGE(STATUS "Using GASNET_HOME: $ENV{GASNET_HOME}")
    IF (EXISTS "$ENV{GASNET_HOME}")
      MESSAGE(STATUS "GASNET_HOME exists...")
      IF(CMAKE_HOST_UNIX)
        #FIXME(hans): We should try to use CMAKE command mode...
        EXECUTE_PROCESS (COMMAND ls -d "$ENV{GASNET_HOME}/*-conduit"
          COMMAND rev
          COMMAND cut -d/ -f1
          COMMAND rev
          COMMAND cut -d- -f1
          OUTPUT_VARIABLE CONDUITS)
        IF (${CONDUITS})
          MESSAGE (STATUS "The following conduits are supported: ${CONDUITS}")
          SET (ENABLE_DISTMEM_BACKEND_GPI ON)
        ENDIF ()
      ENDIF ()
    ELSE ()
      MESSAGE(STATUS "GASNET_HOME does not exist.")
    ENDIF ()
  ENDIF ()
ENDMACRO ()

