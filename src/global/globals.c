/*
 *
 * $Log$
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
#include "scnprs.h"
#include "Error.h"

/*
 *  File handling
 */

FILE *outfile;
/* stream to write result to */

char sacfilename[MAX_FILE_NAME] = "";
/* name of file to be compiled */

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

char target_name[MAX_FILE_NAME] = "default";
/* name of target architecture, information taken from sac2crc file  */

/*
 * Multi-thread options
 */

int num_threads = 1;
/*
 * number of threads to be generated.
 *  0  : dynamic number of threads, specified as first command line argument
 *       on application program startup.
 *  1  : sequential program
 *  >1 : exact number of threads to be started
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
 *
 */

int cc_debug = 0;
/* Enable/disable inclusion of debug code into object files. */

int cc_optimize = 0;
/* C compiler level of optimization */

/*
 * Options concerning the new with-loop format
 */

int Make_Old2NewWith = 0;

/*
 * Command line options for triggering optimizations
 */

int optimize = 1;
/* enable/disable optimizations in general */

int opt_dcr = 1;
/* enable/disable dead code removal */

int opt_cf = 1;
/* enable/disable constant folding */

int opt_lir = 0;
/* enable/disable loop invariant removal */

int opt_inl = 1;
/* enable/disable function inlining */

int opt_unr = 1;
/* enable/disable loop unrolling */

int opt_uns = 1;
/* enable/disable loop unswitching */

int opt_cse = 1;
/* enable/disable common subexpression elimination */

int opt_dfr = 1;
/* enable/disable dead function removal */

int opt_wlf = 1;
/* enable/disable with loop folding */

int opt_ive = 1;
/* enable/disable index vector elimination  */

int opt_ae = 1;
/* enable/disable array elimination */

int opt_rco = 1;
/* enable/disable refcount optimization */

/*
 * Command line options for specifying particular side conditions
 * for some optimizations.
 */

int optvar = 50;
int inlnum = 1;
int unrnum = 2;
int minarray = 4;
int max_overload = 10;
int max_optcycles = 4;

/*
 * Display options
 *
 * These options modify the way print functions behave.
 */

int show_refcnt = 0;
int show_idx = 0;
int show_icm = 0;

/*
 * Runtime options
 *
 * These options insert special code sections into the target C code
 * in order to do additional checks, profiling, etc.
 */

int traceflag = 0;
int profileflag = 0;
int check_malloc = 0;
int check_boundary = 0;

/*
 * Compile time options
 *
 * These options specify the way sac2c behaves.
 */

int cleanup = 1;
/* Don't remove temporary files and directory when compiling
   module/class implementations. */

int linkstyle = 2;
/* Specify linkage style for module/class implementations */

int libstat = 0;
/* Don't actually compile, but display library information. */

int makedeps = 0;
/* Don't actually compile, but infer module dependencies. */

compiler_phase_t break_after = PH_final;
/* Stop compilation process after given phase. */

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

int verbose_level = 3;
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
