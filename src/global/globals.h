/*
 *
 * $Log$
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
 * made simpletype_size global, since it is needed in compile, tile_size_inference AND
 * constants already!
 *
 * [...]
 *
 * Revision 1.1  1995/12/30  14:47:06  cg
 * Initial revision
 *
 *
 */

/*
 * File : globals.h
 *
 * Declaration of global variables
 * which are all defined and initialized in globals.c
 *
 */

#ifndef _sac_globals_h

#define _sac_globals_h

#include "types.h"

#include <stdio.h>

#define MAX_BREAK_SPECIFIER 32

#define PF_MAXFUN 100
#define PF_MAXFUNAP 100
#define PF_MAXFUNNAMELEN 100

extern int sbs;

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

extern int dynamic_shapes;

extern char *target_name;

#define GEN_MT_NONE 0
#define GEN_MT_OLD 1
#define GEN_MT_NEW 2

extern int gen_mt_code;
extern int num_threads;
extern int max_sync_fold;
extern int needed_sync_fold;
extern int max_threads;
extern int min_parallel_size;

extern int max_replication_size;

#define MAX_CPP_VARS 32

extern char *cppvars[MAX_CPP_VARS];
extern int num_cpp_vars;

extern int cc_debug;
extern int cc_optimize;

extern int Make_Old2NewWith;

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

extern int show_refcnt;
extern int show_idx;

#define PAB_NO 0
#define PAB_YES 1

extern int print_after_break;

extern unsigned int traceflag;

#define TRACE_NONE 0x0000 /* don't trace at all */
#define TRACE_ALL 0xffff  /* enable all implemented trace options */

#define TRACE_FUN 0x0001 /* trace user-defined fun apps */
#define TRACE_PRF 0x0002 /* trace prim fun apps */
#define TRACE_OWL 0x0004 /* trace old with-loop execution */
#define TRACE_REF 0x0008 /* trace reference counting operations */
#define TRACE_MEM 0x0010 /* trace malloc/free operations */
#define TRACE_WL 0x0020  /* trace new with-loop execution */
#define TRACE_MT 0x0040  /* trace multi-threading specific operations */

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
#define INTRINSIC_PSI 0x0200
#define INTRINSIC_MODA 0x0400

extern int PFfuncntr;
extern char *PFfunnme[PF_MAXFUN];
extern int PFfunapcntr[PF_MAXFUN];
extern int PFfunapline[PF_MAXFUN][PF_MAXFUNAP];
extern int PFfunapmax;

extern int use_efence;
extern int cleanup;
extern int linkstyle;
extern int libstat;
extern int makedeps;
extern int gen_cccall;
extern int show_syscall;

extern compiler_phase_t break_after;
extern int break_cycle_specifier;
extern char break_specifier[MAX_BREAK_SPECIFIER];

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

extern unsigned int total_allocated_mem;
extern unsigned int current_allocated_mem;
extern unsigned int max_allocated_mem;

extern int print_objdef_for_header_file;
extern int function_counter;
extern deps *dependencies;

extern int indent;

extern int simpletype_size[];

extern char *nt_class_str[];
extern char *nt_uni_str[];

#endif /* _sac_globals_h */
