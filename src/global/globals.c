/*
 *
 * $Log$
 * Revision 2.12  1999/07/09 07:31:24  cg
 * SAC heap manager integrated into sac2c.
 *
 * Revision 2.11  1999/06/25 14:50:36  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 2.10  1999/06/11 12:54:04  cg
 * Added global variables csfile and csdir to implement the corresponding
 * sac2c command line arguments.
 *
 * Revision 2.9  1999/06/09 08:41:02  rob
 * Introduce support for dynamic shape arrays option "ds".
 *
 * Revision 2.8  1999/06/04 14:32:48  cg
 * added global variable cachesim_host
 *
 * Revision 2.7  1999/05/27 08:50:04  cg
 * global variable show_icm made obsolete and removed.
 *
 * Revision 2.6  1999/05/26 14:32:23  jhs
 * Added options MTO and SBE for multi-thread optimsation and
 * synchronisation barrier elimination, both options are by
 * default disabled.
 *
 * Revision 2.5  1999/05/20 14:05:14  cg
 * bug fixed in intialization of variable cachesim.
 *
 * Revision 2.4  1999/05/12 14:31:16  cg
 * some global variables associated with options renamed.
 * Optimizations are now triggered by bit field optimize instead
 * of single individual int variables.
 *
 * Revision 2.3  1999/04/14 09:20:40  cg
 * Settings for cache simulation improved.
 *
 * Revision 2.2  1999/03/31 11:30:27  cg
 * added global variable cachesim.
 *
 * Revision 2.1  1999/02/23 12:39:20  sacbase
 * new release made
 *
 * Revision 1.30  1999/02/19 18:08:04  dkr
 * added use_efence
 *
 * Revision 1.29  1999/02/15 13:34:09  sbs
 * added -noDLAW opt_dlaw;
 *
 * Revision 1.28  1999/01/14 14:25:54  cg
 * added variable opt_tile to enable/disable tiling,
 * set current sac2c version to v0.8.
 *
 * Revision 1.27  1999/01/07 14:01:01  sbs
 * more sophisticated breaking facilities inserted;
 * Now, a break in a specific cycle can be triggered!
 *
 * Revision 1.26  1998/12/10 12:38:02  cg
 * Loop invariant removal is disabled by default for production
 * versions of sac2c.
 *
 * Revision 1.25  1998/12/07 17:29:55  cg
 * added variables version_id and target_platform to keep track
 * of this information used in usage.c and gen_startup_code.c
 *
 * Revision 1.24  1998/10/26 12:34:14  cg
 * new compiler option:
 * use intrinsic array operations instead of with-loop based implementations
 * in the stdlib. The corresponding information is stored by the new
 * global variable intrinsics.
 *
 * Revision 1.23  1998/10/23 14:29:46  cg
 * added the new command line option -inparsize <no> which allows to
 * specify a minimum generator size for with-loops to be executed in
 * parallel if such execution is enabled.
 * The information stored by the global variable min_parallel_size.
 *
 * Revision 1.22  1998/08/07 18:11:29  sbs
 * inserted gen_mt_code; it prevents spmd regions from being created per default
 * only if one of the following options is set:
 * -mtstatic <no> / -mtdynamic <no> / -mtall <no>
 * spmd regions will be introduced!
 *
 * Revision 1.21  1998/07/07 13:40:08  cg
 * added global variable all_threads implementing the command line option -mt-all
 *
 * Revision 1.20  1998/06/23 15:04:05  cg
 * added global variables show_syscall and gen_cccall
 *
 * Revision 1.19  1998/06/19 16:34:11  dkr
 * added opt_uip
 *
 * Revision 1.18  1998/05/27 11:17:57  cg
 * added global variable 'puresacfilename' which provides the file to be
 * compiled without leading path information as contained in sacfilename.
 *
 * Revision 1.17  1998/05/13 13:52:57  srs
 * added wlunrnum
 *
 * Revision 1.16  1998/05/13 13:38:57  srs
 * added opt_wlunr and renamed opt_unr to opt_lunr
 *
 * Revision 1.15  1998/05/11 08:31:05  srs
 * activated LIR again
 *
 * Revision 1.14  1998/05/06 11:40:45  cg
 * added globals max_sync_fold and max_threads
 *
 * Revision 1.13  1998/05/05 12:33:03  srs
 * inserted opt_wlt
 *
 * Revision 1.12  1998/04/29 17:11:22  dkr
 * added new compiler phases
 *
 * Revision 1.11  1998/04/25 11:53:06  sbs
 * indent inserted.
 *
 * Revision 1.10  1998/04/17 17:28:18  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.9  1998/04/02 16:09:41  dkr
 * added new compiler phase name:
 *   generating concurrent regions
 *
 * Revision 1.8  1998/03/24 11:48:31  cg
 * added global variables used to gather compile time information
 * later used for profiling.
 *
 * Revision 1.7  1998/03/13 13:35:00  dkr
 * fixed corrupted header ?!?
 *
 * Revision 1.6  1998/03/13 13:15:02  dkr
 * removed a bug with flag _DBUG:
 *   new var 'my?dbug'
 *   vars 'dbug_...' renamed in 'my_dbug...'
 *
 * Revision 1.5  1998/03/04 16:20:08  cg
 * added  cc_debug,  cc_optimize, tmp_dirname.
 * removed ccflagsstr, useranlib.
 *
 * Revision 1.4  1998/03/02 13:56:02  cg
 * global variables psi_optimize and backend_optimize removed.
 * Loop Invariant Removal disabled by default.
 *
 * Revision 1.3  1998/02/27 13:37:25  srs
 * activated opt_wlf again
 *
 * Revision 1.2  1998/02/26 15:22:58  cg
 * target_name now initialized as 'default'
 *
 * Revision 1.1  1998/02/25 09:10:20  cg
 * Initial revision
 *
 *
 */

/*
 * File : globals.c
 *
 * This file should contain the definitions of all global variables
 * used in the implementation of the sac2c compiler which are not restricted
 * to the use within one particular file or within the files of one
 * particular subdirectory. While the former ones have to be declared static,
 * the latter ones should be defined in a file called globals_xyz.c specific
 * to the respective subdirectory.
 *
 * However, the usage of global variables should be as extensive as possible
 * since a functional programming style is preferred in the SAC project. The
 * major application of global variables therefore is the storage of such
 * global information as determined by the command line arguments of a sac2c
 * compiler call.
 *
 */

#include "filemgr.h"
#include "types.h"
#include "globals.h"
#include "Error.h"

/*
 *  Version control
 */

char version_id[] = "v0.9";
/* version identifier of sac2c */

#if defined(SOLARIS_SPARC)

char target_platform[] = "SOLARIS_SPARC";

#elif defined(LINUX_X86)

char target_platform[] = "LINUX_X86";

#else
/*
 * This case should never happen since the Makefile guarantees that any one
 * of the supported platforms is selected.
 */

char target_platform[] = "unknown";

#endif

/*
 *  File handling
 */

FILE *outfile;
/* stream to write result to */

char sacfilename[MAX_FILE_NAME] = "";
/* name of file to be compiled */

char *puresacfilename;
/* sacfilename without path specification */

char outfilename[MAX_FILE_NAME] = "";
/* name of executable    */

char modulename[MAX_FILE_NAME] = "";
/* name of module/class which is compiled    */

char cfilename[MAX_FILE_NAME];
/* name of C source code file       */

char targetdir[MAX_FILE_NAME];
/* name of C source code file       */

char commandline[MAX_PATH_LEN] = "";
/* command line, used for diagnostic output (status report)  */

file_type filetype;
/* kind of file: F_PROG, F_MODIMP or F_CLASSIMP */

char *tmp_dirname = NULL;
/* directory for storing files generated by sac2c until      */
/* the final compilation steps are completed.                */

/*
 * Target architecture description
 */

char *target_name = "default";
/* name of target architecture, information taken from sac2crc file  */

/*
 * Dynamic-sized arrays option
 */

int dynamic_shapes = 0; /* Dynamic shapes are disabled by default */

/*
 * Multi-thread options
 */

int gen_mt_code = 0;
/*
 * will be set to 1 iff one of the following options is set:
 * -mtstatic <no> / -mtdynamic <no> / -mtall <no>
 */

int num_threads = 1;
/*
 * number of threads to be generated.
 *  0  : dynamic number of threads, specified as first command line argument
 *       on application program startup.
 *  1  : sequential program
 *  >1 : exact number of threads to be started
 */

int max_sync_fold = 3;
/*
 * maximum number of fold operations in a single synchronisation block,
 * should always be 2**n-1
 */

int max_threads = 32;
/*
 * maximum number of threads if exact number is determined dynamically.
 */

int min_parallel_size = 250;
/*
 * minimum generator size for parallel execution of with-loops.
 */

/*
 * Preprocessor options
 *
 *  refer to -D command line option
 */

char *cppvars[MAX_CPP_VARS];
/* pointer to respective argv field */

int num_cpp_vars = 0;
/* number of preprocessor options */

/*
 * C compiler options
 */

int cc_debug = 0;
/* Enable/disable inclusion of debug code into object files. */

#ifdef PRODUCTION
int cc_optimize = 3;
#else  /* PRODUCTION */
int cc_optimize = 0;
#endif /* PRODUCTION */
/* C compiler level of optimization */

/*
 * Options concerning the new with-loop format
 */

int Make_Old2NewWith = 0;

/*
 * Command line options for triggering optimizations
 */

#ifdef PRODUCTION
unsigned int optimize = OPT_ALL & (~OPT_LIR) & (~OPT_MTO) & (~OPT_SBE);
#else  /* PRODUCTION */
unsigned int optimize = OPT_ALL & (~OPT_MTO) & (~OPT_SBE);
#endif /* PRODUCTION */

/*
 * Command line options for specifying particular side conditions
 * for some optimizations.
 */

int optvar = 50;
int inlnum = 1;
int unrnum = 2;
int wlunrnum = 9;
int minarray = 4;
int max_overload = 10;
int max_optcycles = 4;
int initial_heapsize = 2;

/*
 * Display options
 *
 * These options modify the way print functions behave.
 */

int show_refcnt = 0;
int show_idx = 0;

/*
 * Runtime options
 *
 * These options insert special code sections into the target C code
 * in order to do additional checks, profiling, etc., or to use intrinsic
 * implementations of some array operations.
 */

unsigned int traceflag = TRACE_NONE;
unsigned int profileflag = PROFILE_NONE;
unsigned int runtimecheck = RUNTIMECHECK_NONE;
unsigned int intrinsics = INTRINSIC_NONE;
unsigned int cachesim = CACHESIM_NO | CACHESIM_PIPE;
char cachesim_host[MAX_FILE_NAME] = "";
char cachesim_file[MAX_FILE_NAME] = "";
char cachesim_dir[MAX_FILE_NAME] = "";

/*
 * Profiling information storage facilities
 */

int PFfuncntr = 1;
char *PFfunnme[PF_MAXFUN] = {"main"};
int PFfunapcntr[PF_MAXFUN];
int PFfunapline[PF_MAXFUN][PF_MAXFUNAP];
int PFfunapmax = 1;

/*
 * Compile time options
 *
 * These options specify the way sac2c behaves.
 */

int use_efence = 0;
/* link executable with ElectricFence (Malloc Debugger) */

int cleanup = 1;
/* Don't remove temporary files and directory when compiling
   module/class implementations. */

int linkstyle = 2;
/* Specify linkage style for module/class implementations */

int libstat = 0;
/* Don't actually compile, but display library information. */

int makedeps = 0;
/* Don't actually compile, but infer module dependencies. */

int gen_cccall = 0;
/* Generate shell script '.sac2c' in current directory
   that contains the C compiler call produced by sac2c. */

int show_syscall = 0;
/* Show system calls during compilation. */

compiler_phase_t break_after = PH_final;
/* Stop compilation process after given phase. */

int break_cycle_specifier = -1;
/* Additional break specifier that allows a designated break within
   a particular (optimization) loop */

char break_specifier[MAX_BREAK_SPECIFIER] = "";
/* Additional break specifier to allow breaking within a particular
   compiler phase at any position. */

/*
 *  Definitions of some global variables necessary for the
 *  glorious SAC2C compile time information system
 */

int errors = 0;
/* counter for number of errors   */

int warnings = 0;
/* counter for number of warnings */

#ifdef PRODUCTION
int verbose_level = 1;
#else  /* PRODUCTION */
int verbose_level = 3;
#endif /* PRODUCTION */
       /* controls compile time output   */

compiler_phase_t compiler_phase = PH_setup;
/* counter for compilation phases */

int message_indent = 0;  /* used for formatting compile time output */
int last_indent = 0;     /* used for formatting compile time output */
int current_line_length; /* used for formatting compile time output */

char error_message_buffer[MAX_ERROR_MESSAGE_LENGTH];
/* buffer for generating formatted message */

char *filename;
/* current file name */

char *compiler_phase_name[] = {"",
                               "Setting up sac2c compiler environment",
                               "Loading SAC program",
                               "Resolving imports from modules and classes",
                               "Checking required libraries",
                               "Generating generic types and functions",
                               "Simplifying source code",
                               "Running type inference system",
                               "Checking module/class declaration file",
                               "Resolving implicit types",
                               "Analysing functions and global objects",
                               "Generating SAC-Information-Block",
                               "Resolving global objects and reference parameters",
                               "Checking uniqueness property of objects",
                               "Generating purely functional code",
                               "Running SAC optimizations",
                               "Running PSI optimizations",
                               "Running reference count inference system",
                               "Transforming with-loop representation",
                               "Generating SPMD- and SYNC-regions",
                               "Preparing C code generation",
                               "Generating C code",
                               "Creating C file",
                               "Invoking C compiler",
                               "Creating SAC library",
                               "Writing dependencies to stdout",
                               "Unknown compiler phase"};

/*
 * DBug options
 */

compiler_phase_t my_dbug_from = PH_initial;
compiler_phase_t my_dbug_to = PH_final;
int my_dbug = 0;
int my_dbug_active = 0;
char *my_dbug_str = NULL;

/*
 * Memory counters
 */

unsigned int total_allocated_mem = 0;
unsigned int current_allocated_mem = 0;
unsigned int max_allocated_mem = 0;

/*
 * Special purpose global variables
 */

int print_objdef_for_header_file = 0;
/*
 *  This global variable serves a very special purpose.
 *  When generating separate C-files for functions and global variables,
 *  a header file is required which contains declarations of them all.
 *  In this case the ICM ND_KS_DECL_GLOBAL_ARRAY must be written
 *  differently. This global variable triggers the respective print
 *  function defined in icm2c.c. It is set by PrintModul.
 */

int function_counter = 1;
/*
 *  This global variable is used whenever the functions of a module or
 *  class are written to separate files.
 */

deps *dependencies = NULL;
/*
 *  This global variable is used to store dependencies during the
 *  whole compilation process.
 *
 *  The dependency table is built by import.c and readsib.c and used
 *  for several purposes, such as generating makefiles or link lists.
 */

int indent = 0;
/*
 *  This global variable is used for indenting while printing SAC-code.
 *  It has to be made global since printing is not only done in the printing-
 *  directory but from within icm2c_xxx.c as well!
 */

/*
 * These character arrays are the macro-name-parts used to select
 * array class and array uniqueness properties.
 * See NT_NAME_CLASS_INDEX and NT_UNI_CLASS_INDEX in types.h
 */
#define NTIF(type, str) str
#define ATTRIB 1

char *nt_class_str[] = {
#include "nt_info.mac"
};
#undef ATTRIB 1

#define ATTRIB 2

char *nt_uni_str[] = {
#include "nt_info.mac"
};

#undef ATTRIB
#undef NTIF
