/*
 *
 * $Log$
 * Revision 3.9  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.8  2001/05/11 15:13:43  cg
 * Added new variable max_schedulers to keep track of the maximum
 * number of schedulers within a single SPMD function.
 *
 * Revision 3.7  2001/05/08 13:11:56  nmw
 * flag valid_ssaform added
 *
 * Revision 3.6  2001/05/07 15:02:46  dkr
 * type of flags is defined as 'bool' instead of 'int' now
 *
 * Revision 3.5  2001/02/09 14:39:29  nmw
 * global ssa flag added
 *
 * Revision 3.4  2000/12/12 12:15:22  dkr
 * internal flag 'dkr' added
 *
 * Revision 3.3  2000/11/27 21:04:38  cg
 * Added general support for new optimization APL,
 * "array placement"
 *
 * Revision 3.2  2000/11/24 16:30:07  nmw
 * TRACE_CENV added
 *
 * Revision 3.1  2000/11/20 17:59:29  sacbase
 * new release made
 *
 * Revision 2.38  2000/10/27 13:23:13  cg
 * Added new command line options -aplimit and -apdiaglimit.
 *
 * Revision 2.37  2000/10/24 10:02:51  dkr
 * simpletype_size renamed into basetype_size
 *
 * Revision 2.36  2000/10/17 16:51:16  dkr
 * macro MAIN_MOD_NAME added
 *
 * Revision 2.35  2000/08/17 10:06:09  dkr
 * all the NT stuff is now in a separate modul (NameTuples.[ch])
 *
 * Revision 2.34  2000/08/02 14:22:23  mab
 * added flag "-apdiag"
 *
 * Revision 2.33  2000/07/24 14:50:19  nmw
 * added declaration of object_counter
 *
 * Revision 2.32  2000/06/13 13:39:39  dkr
 * Make_Old2NewWith renamed into make_patchwith
 *
 * Revision 2.31  2000/06/07 11:41:26  nmw
 * additional define for GENERATELIBRARY added
 *
 * Revision 2.30  2000/06/07 11:08:26  nmw
 * added global variable generatelibrary and defines for the types
 *
 * Revision 2.29  2000/05/26 14:21:45  sbs
 * OPT_AP added.
 *
 * Revision 2.28  2000/03/16 14:27:41  dkr
 * do_lac_fun_transformation replaced by do_lac2fun, do_fun2lac
 *
 * Revision 2.27  2000/03/02 18:50:04  cg
 * Added new option -lac2fun that activates lac2fun conversion and
 * vice versa between psi optimizations and precompiling.
 *
 * Revision 2.26  2000/02/04 14:45:50  jhs
 * Added -maxrepsize.
 *
 * Revision 2.25  2000/02/03 16:45:11  cg
 * Added new optimization option MSCA.
 *
 * Revision 2.24  2000/01/24 12:23:08  jhs
 * Added options to activate/dactivate printing after a break
 * (-noPAB, -doPAB).
 *
 * Revision 2.23  2000/01/21 13:19:53  jhs
 * Added new mt ... infrastructure expanded ...
 *
 * Revision 2.22  2000/01/17 17:58:45  cg
 * Added new heap manager optimization options
 * APS (arena preselection) and
 * RCAO (reference counter allocation optimization).
 *
 * Revision 2.21  2000/01/17 16:25:58  cg
 * Added new global variable to control initial heap sizes separately
 * for master's arena of arenas, workers' arena of arenas and the
 * top arena.
 *
 * Revision 2.20  1999/10/22 14:16:41  sbs
 * made simpletype_size global, since it is needed in compile,
 * tile_size_inference AND constants already!
 *
 * [...]
 *
 */

/*
 * File : globals.h
 *
 * Declaration of global variables
 * which are all defined and initialized in globals.c
 */

#ifndef _sac_globals_h_
#define _sac_globals_h_

#include <stdio.h>
#include "types.h"

#define MAIN_MOD_NAME "_MAIN"

extern bool sbs;
extern bool dkr;

extern char version_id[];
extern char target_platform[];

extern FILE *outfile;

extern char sacfilename[];
extern char *puresacfilename;
extern char outfilename[];
extern char modulename[];
extern char cfilename[];
extern char targetdir[];
extern char commandline[];
extern file_type filetype;
extern char *tmp_dirname;

extern bool dynamic_shapes;

extern char *target_name;

extern int gen_mt_code;

#define GEN_MT_NONE 0
#define GEN_MT_OLD 1
#define GEN_MT_NEW 2

extern int num_threads;
extern int max_sync_fold;
extern int needed_sync_fold;
extern int max_threads;
extern int min_parallel_size;
extern int max_replication_size;
extern int max_schedulers;

#define MAX_CPP_VARS 32

extern char *cppvars[MAX_CPP_VARS];
extern int num_cpp_vars;

extern bool cc_debug;
extern int cc_optimize;

extern int patch_with;

extern unsigned int optimize;

#define OPT_NONE 0x00000000 /* all optimizations disabled                  */
#define OPT_ALL 0xFFFFFFFF  /* all optimizations enabled                   */

#define OPT_DCR 0x00000001  /* dead code removal                           */
#define OPT_CF 0x00000002   /* constant folding                            */
#define OPT_LIR 0x00000004  /* loop invariant removal                      */
#define OPT_INL 0x00000008  /* function inlining                           */
#define OPT_LUR 0x00000010  /* loop unrolling                              */
#define OPT_WLUR 0x00000020 /* with-loop unrolling                         */
#define OPT_LUS 0x00000040  /* loop unswitching                            */
#define OPT_CSE 0x00000080  /* common subexpression elimination            */
#define OPT_DFR 0x00000100  /* dead function removal                       */
#define OPT_WLT 0x00000200  /* with-loop transformation                    */
#define OPT_WLF 0x00000400  /* with-loop folding                           */
#define OPT_IVE 0x00000800  /* index vector elimination                    */
#define OPT_AE 0x00001000   /* array elimination                           */
#define OPT_DLAW 0x00002000 /* distributive law                            */
#define OPT_RCO 0x00004000  /* reference count optimization                */
#define OPT_UIP 0x00008000  /* update-in-place analysis                    */
#define OPT_TSI 0x00010000  /* with-loop tile size inference               */
#define OPT_TSP 0x00020000  /* with-loop tile size pragmas                 */
#define OPT_MTO 0x00040000  /* multi-thread optimization                   */
#define OPT_SBE 0x00080000  /* synchronisation barrier elimination         */
#define OPT_MTI 0x00100000  /* ??                                          */
#define OPT_PHM 0x00200000  /* private heap management                     */
#define OPT_APS 0x00400000  /* arena preselection (for PHM)                */
#define OPT_RCAO 0x00800000 /* ref count allocation optimization (for PHM) */
#define OPT_MSCA 0x01000000 /* memory size cache adjustment (for PHM)      */
#define OPT_AP 0x02000000   /* array padding                               */
#define OPT_APL 0x04000000  /* array placement                             */

/* use ssa-form based optimizations instead of old opts */
extern bool use_ssaform;

/* ast is in ssa form */
extern bool valid_ssaform;

extern int optvar;
extern int inlnum;
extern int unrnum;
extern int wlunrnum;
extern int minarray;
extern int max_overload;
extern int max_optcycles;

extern int initial_master_heapsize;
extern int initial_worker_heapsize;
extern int initial_unified_heapsize;

extern bool show_refcnt;
extern bool show_idx;

extern bool print_after_break;

extern bool apdiag;
extern int padding_overhead_limit;
extern int apdiag_limit;

extern unsigned int traceflag;

#define TRACE_NONE 0x0000 /* don't trace at all */
#define TRACE_ALL 0xffff  /* enable all implemented trace options */

#define TRACE_FUN 0x0001  /* trace user-defined fun apps */
#define TRACE_PRF 0x0002  /* trace prim fun apps */
#define TRACE_OWL 0x0004  /* trace old with-loop execution */
#define TRACE_REF 0x0008  /* trace reference counting operations */
#define TRACE_MEM 0x0010  /* trace malloc/free operations */
#define TRACE_WL 0x0020   /* trace new with-loop execution */
#define TRACE_MT 0x0040   /* trace multi-threading specific operations */
#define TRACE_CENV 0x0080 /* trace c runtime enviroment init/exit */

extern unsigned int profileflag;

#define PROFILE_NONE 0x0000
#define PROFILE_ALL 0xffff

#define PROFILE_FUN 0x0001
#define PROFILE_INL 0x0002
#define PROFILE_LIB 0x0004
#define PROFILE_WITH 0x0008

extern unsigned int cachesim;

#define CACHESIM_NO 0x0000

#define CACHESIM_YES 0x0001
#define CACHESIM_ADVANCED 0x0002
#define CACHESIM_FILE 0x0004
#define CACHESIM_PIPE 0x0008
#define CACHESIM_IMMEDIATE 0x0010
#define CACHESIM_BLOCK 0x0020

extern char cachesim_host[];
extern char cachesim_file[];
extern char cachesim_dir[];

extern unsigned int runtimecheck;

#define RUNTIMECHECK_NONE 0x0000
#define RUNTIMECHECK_ALL 0xffff

#define RUNTIMECHECK_MALLOC 0x0001
#define RUNTIMECHECK_BOUNDARY 0x0002
#define RUNTIMECHECK_ERRNO 0x0004
#define RUNTIMECHECK_HEAP 0x0008

extern unsigned int intrinsics;

#define INTRINSIC_NONE 0x0000
#define INTRINSIC_ALL 0xffff

#define INTRINSIC_ADD 0x0001
#define INTRINSIC_SUB 0x0002
#define INTRINSIC_MUL 0x0004
#define INTRINSIC_DIV 0x0008
#define INTRINSIC_TO 0x0010
#define INTRINSIC_TAKE 0x0020
#define INTRINSIC_DROP 0x0040
#define INTRINSIC_ROT 0x0080
#define INTRINSIC_CAT 0x0100
#define INTRINSIC_SEL 0x0200
#define INTRINSIC_MODA 0x0400

extern unsigned int generatelibrary;

/* generate no library - dummy value for init */
#define GENERATELIBRARY_NOTHING 0x0000
/* generate SAC library from module */
#define GENERATELIBRARY_SAC 0x0001
/* generate C library and headerfile from module */
#define GENERATELIBRARY_C 0x0002

#define PF_MAXFUN 100
#define PF_MAXFUNAP 100
#define PF_MAXFUNNAMELEN 100

extern int PFfuncntr;
extern char *PFfunnme[PF_MAXFUN];
extern int PFfunapcntr[PF_MAXFUN];
extern int PFfunapline[PF_MAXFUN][PF_MAXFUNAP];
extern int PFfunapmax;

#define MAX_BREAK_SPECIFIER 32

extern compiler_phase_t break_after;
extern int break_cycle_specifier;
extern char break_specifier[MAX_BREAK_SPECIFIER];

extern bool use_efence;
extern bool cleanup;
extern int linkstyle;
extern bool libstat;
extern int makedeps;
extern bool gen_cccall;
extern bool show_syscall;

extern int errors;
extern int warnings;

extern int verbose_level;
extern compiler_phase_t compiler_phase;

extern int message_indent;
extern int last_indent;
extern int current_line_length;

extern char error_message_buffer[];
extern int linenum;
extern char *filename;
extern char *compiler_phase_name[];

extern compiler_phase_t my_dbug_from;
extern compiler_phase_t my_dbug_to;
extern int my_dbug;
extern int my_dbug_active;
extern char *my_dbug_str;
#ifdef SHOW_MALLOC
extern int malloc_align_step;
#endif

extern int do_lac2fun[], do_fun2lac[];

extern unsigned int total_allocated_mem;
extern unsigned int current_allocated_mem;
extern unsigned int max_allocated_mem;

extern int print_objdef_for_header_file;
extern int function_counter;
extern int object_counter;
extern deps *dependencies;

extern int indent;

extern int basetype_size[];

#endif /* _sac_globals_h_ */
