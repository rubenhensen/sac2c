/*
 *
 * $Log$
 * Revision 3.4  2000/12/12 12:15:40  dkr
 * internal flag 'dkr' added
 *
 * Revision 3.3  2000/12/06 10:06:45  dkr
 * version incremented to 0.91
 *
 * Revision 3.2  2000/11/27 21:04:38  cg
 * Added general support for new optimization APL,
 * "array placement"
 *
 * Revision 3.1  2000/11/20 17:59:28  sacbase
 * new release made
 *
 * Revision 2.36  2000/10/27 13:23:13  cg
 * Added new command line options -aplimit and -apdiaglimit.
 *
 * Revision 2.35  2000/10/24 10:03:04  dkr
 * simpletype_size renamed into basetype_size
 *
 * Revision 2.34  2000/08/17 10:06:18  dkr
 * all the NT stuff is now in a separate modul (NameTuples.[ch])
 *
 * Revision 2.33  2000/08/02 14:22:23  mab
 * added flag "-apdiag"
 *
 * Revision 2.32  2000/07/24 14:49:58  nmw
 * added global var object_counter
 *
 * Revision 2.31  2000/06/13 13:39:31  dkr
 * Make_Old2NewWith renamed into make_patchwith
 *
 * Revision 2.30  2000/06/08 12:12:48  jhs
 * Fixed comment.
 *
 * Revision 2.29  2000/06/07 11:42:27  nmw
 * init of generatelibrary changed
 *
 * Revision 2.28  2000/06/07 11:06:04  nmw
 * added global variable generatelibrary with default GENERATELIBRARY_SAC
 * used for genlib commandline switch
 *
 * Revision 2.27  2000/03/23 14:00:51  jhs
 * Brushing around includes of nt_info.mac.
 *
 * Revision 2.26  2000/03/16 14:28:25  dkr
 * do_lac_fun_transformation replaced by do_lac2fun, do_fun2lac
 * phase_info.mac used
 *
 * Revision 2.25  2000/03/02 18:50:04  cg
 * Added new option -lac2fun that activates lac2fun conversion and
 * vice versa between psi optimizations and precompiling.
 *
 * Revision 2.24  2000/02/04 14:45:50  jhs
 * Added -maxrepsize.
 *
 * Revision 2.23  2000/01/24 12:23:08  jhs
 * Added options to activate/dactivate printing after a break
 * (-noPAB, -doPAB).
 *
 * Revision 2.22  2000/01/21 13:19:53  jhs
 * Added new mt ... infrastructure expanded ...
 *
 * Revision 2.21  2000/01/17 19:45:25  cg
 * Default value for top arena initialization set to 0.
 * See comment in file why.
 *
 * Revision 2.20  2000/01/17 16:25:58  cg
 * Added new global variable to control initial heap sizes separately
 * for master's arena of arenas, workers' arena of arenas and the
 * top arena.
 *
 * Revision 2.19  1999/10/22 14:16:41  sbs
 * made simpletype_size global, since it is needed in compile, tile_size_inference AND
 * constants already!
 *
 * Revision 2.18  1999/10/04 11:58:34  sbs
 * secret option "sbs" added!
 *
 * Revision 2.17  1999/10/04 09:24:53  sbs
 * linenum moved from scanparse to globals!
 *
 * Revision 2.16  1999/08/09 15:54:32  dkr
 * #undef statement corrected
 *
 * Revision 2.15  1999/08/05 13:30:40  jhs
 * Added OPT_MTI (default now: off), to steer mto-part during spmdinit.
 *
 * Revision 2.14  1999/07/21 16:28:19  jhs
 * needed_sync_fold introduced, max_sync_fold adjusted, command-line and usage
 * updated.
 *
 * Revision 2.13  1999/07/20 07:56:22  cg
 * Added global variable malloc_align_step.
 *
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
 * ... [eliminated]
 *
 * Revision 1.1  1998/02/25 09:10:20  cg
 * Initial revision
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
#include "internal_lib.h"

/*
 *  Version control
 */

char version_id[] = "v0.91";
/* version identifier of sac2c */

#if defined(SAC_FOR_SOLARIS_SPARC)

char target_platform[] = "SOLARIS_SPARC";

#elif defined(SAC_FOR_LINUX_X86)

char target_platform[] = "LINUX_X86";

#elif defined(SAC_FOR_OSF_ALPHA)

char target_platform[] = "OSF_ALPHA";

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

int gen_mt_code = GEN_MT_NONE;
/*
 *  will be set to
 *  GEN_MT_OLD   iff one of the following options is set: -mt
 *  GEN_MT_NEW   iff one of the following options is set: -mtn
 *  GEN_MT_NONE  otherwise
 */

int num_threads = 1;
/*
 * number of threads to be generated.
 *  0  : dynamic number of threads, specified as first command line argument
 *       on application program startup.
 *  1  : sequential program
 *  >1 : exact number of threads to be started
 */

int max_sync_fold = -1;
/*
 * maximum number of fold operations in a single synchronisation block.
 *   -1 => value of infered needed_sync_fold will be used
 *    0 => no fold-with-loops, will be executed concurrently,
 *         because no fold are allowed to be contained in a spmd/sync-block.
 *   >0 => as much folds will be contained in one sync-block as maximum.
 */

int needed_sync_fold = 0;
/*
 * maximum number of fold operations in a single synchronisation block,
 * will be infered mechanically.
 */

int max_threads = 32;
/*
 * maximum number of threads if exact number is determined dynamically.
 */

int min_parallel_size = 250;
/*
 * minimum generator size for parallel execution of with-loops.
 */

int max_replication_size = 250;
/*
 *  maximum size of array for replication, otherwise the calculation will be
 *  executed single-threaded.
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

int make_patchwith = 0;

/*
 * Command line options for triggering optimizations
 */

#ifdef PRODUCTION
unsigned int optimize
  = OPT_ALL & (~OPT_LIR) & (~OPT_MTO) & (~OPT_SBE) & (~OPT_MTI) & (~OPT_APL);
#else  /* PRODUCTION */
unsigned int optimize = OPT_ALL & (~OPT_MTO) & (~OPT_SBE) & (~OPT_MTI) & (~OPT_APL);
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

int initial_master_heapsize = 1024;
int initial_worker_heapsize = 64;
int initial_unified_heapsize = 0;
/*
 * Why is the default initial top arena size 0?
 *
 * Unfortunately, it turned out that pthread_key_create() allocates
 * some amount of memory, actually one page, without using malloc()
 * but by direct manipulation of the process' break value via sbrk().
 * Since in multi-threaded execution malloc() is always called upon
 * program startup, early intitialization of the heap manager's internal
 * data structures is enforced.
 * Subsequent manipulation of the break value, however, leads to memory
 * fragmentation as the initial top arena cannot be extended smoothly
 * due to the missing page on top of the initially requested heap memory.
 *
 * As long as there is no elegant solution to this problem, there should
 * be no initialization of the top arena, except when an upper boundary
 * for the total memory consumption is known and used for initialization.
 */

/*
 * special hidden options!
 */
int dkr = 0;
int sbs = 0;

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

int print_after_break = PAB_YES;
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

unsigned int generatelibrary = GENERATELIBRARY_NOTHING;
/* Specify interfaces to generate from SAC modules
   init: nothing, but changed to default standard SAC library
   if commandline switch is not used */

int padding_overhead_limit = 10;
/*
 * Limit for additional resource allocation due to array padding in percentage.
 * Can be modified via -aplimit option.
 */

bool apdiag = FALSE;
/* Diagnostics of array padding may be written into a file.
   Per default no information is written. Use -apdiag to enable
   output to "modulename.ap". */

int apdiag_limit = 20000;
/*
 * Limit for size of diagnostic output given in approximate number of lines.
 * This avoids the creation of extremely huge diagnostic output files.
 */

/*
 *  Definitions of some global variables necessary for the
 *  glorious SAC2C compile time information system
 */

int errors = 0; /* counter for number of errors   */

int warnings = 0; /* counter for number of warnings */

#ifdef PRODUCTION
int verbose_level = 1; /* controls compile time output   */
#else                  /* PRODUCTION */
int verbose_level = 3;
#endif                 /* PRODUCTION */

compiler_phase_t compiler_phase = PH_setup; /* counter for compilation phases */

int message_indent = 0;  /* used for formatting compile time output */
int last_indent = 0;     /* used for formatting compile time output */
int current_line_length; /* used for formatting compile time output */

/* buffer for generating formatted message */
char error_message_buffer[MAX_ERROR_MESSAGE_LENGTH];

int linenum = 1; /* current line number */

char *filename; /* current file name */

#define PH_SELtext(it_text) it_text
char *compiler_phase_name[PH_final + 1] = {
#include "phase_info.mac"
};

/*
 * DBug options
 */

compiler_phase_t my_dbug_from = PH_initial;
compiler_phase_t my_dbug_to = PH_final;
int my_dbug = 0;
int my_dbug_active = 0;
char *my_dbug_str = NULL;
#ifdef SHOW_MALLOC
int malloc_align_step;
#endif

/*
 * lac2fun, fun2lac conversion
 */

/* (do_lac2fun[i] > 0): do lac2fun conversion before phase i */
#define PH_SELlac2fun(it_lac2fun) it_lac2fun
int do_lac2fun[PH_final + 1] = {
#include "phase_info.mac"
};
/* (do_fun2lac[i] > 0): do fun2lac conversion after phase i */
#define PH_SELfun2lac(it_fun2lac) it_fun2lac
int do_fun2lac[PH_final + 1] = {
#include "phase_info.mac"
};

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

int object_counter = 0;
/*
 *  This global variable is used whenever the objectinitflags of a module
 *  are written to separate files.
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
 * The following array of integers keeps the lengths of all simpletypes.
 * It at least is used in compile.c, tile_size_inference.c, and constants.c!
 */
#define TYP_IFsize(sz) sz
int basetype_size[] = {
#include "type_info.mac"
};
