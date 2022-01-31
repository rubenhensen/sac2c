# These options are both used to turn on and off compilation options for the compiler
# as well as part of the CMake configuration stage as status flags. Please don't be
# surprised if an option flips once configuration has completed - it is likely that
# one or more dependencies could not be satisifed.
OPTION (DOT       "Build sac with dot-file support for visualising AST"               OFF)
OPTION (CPLUSPLUS "Use C++ compiler to build sac2c"                                   OFF)
OPTION (LTO       "Use Link Time Optimisations if supported by the C compiler"        OFF)
OPTION (MT        "Build sac2c with thread-based backend support"                     ON)
OPTION (OMP       "Build sac2c with openmp-based backend support"                     OFF)
OPTION (LPEL      "Build sac2c with LPEL support"                                     OFF)
OPTION (PHM       "Build sac2c with Private Heap Manager"                             ON)
OPTION (DISTMEM   "Build sac2c with distributed memory support"                       OFF)
OPTION (CUDA      "Build sac2c with CUDA backend support"                             ON)
OPTION (HWLOC     "Build sac2c with hwloc support"                                    ON)
OPTION (ISL       "Build sac2c with Integer Set Library"                              ON)
OPTION (BARVINOK  "Build sac2c with Barvinok Library"                                 ON)
OPTION (FUNCTESTS "Enable functional tests (requires GTest library)"                  OFF)
OPTION (BUILDGENERIC "Build the compiler with -march=generic -mtune=generic"          OFF)
