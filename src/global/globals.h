/*
 *
 * $Log$
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
 * cachesim bit mask macros reorganized.
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
 * Revision 2.1  1999/02/23 12:39:21  sacbase
 * new release made
 *
 * Revision 1.44  1999/02/19 17:21:41  dkr
 * *** empty log message ***
 *
 * Revision 1.43  1999/02/19 17:15:26  dkr
 * use_efence added
 *
 * Revision 1.42  1999/02/15 13:34:09  sbs
 * added -noDLAW opt_dlaw;
 *
 * Revision 1.41  1999/01/26 14:24:46  cg
 * Added INTRINSIC_PSI
 *
 * Revision 1.40  1999/01/15 15:14:32  cg
 * added opt_tile, modified value constants for intrinsics.
 *
 * Revision 1.39  1999/01/14 14:25:54  cg
 * added variable opt_tile to enable/disable tiling.
 *
 * Revision 1.38  1999/01/07 14:01:01  sbs
 * more sophisticated breaking facilities inserted;
 * Now, a break in a specific cycle can be triggered!
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

extern char *target_name;

extern int gen_mt_code;
extern int num_threads;
extern int max_sync_fold;
extern int max_threads;
extern int all_threads;
extern int min_parallel_size;

#define MAX_CPP_VARS 32

extern char *cppvars[MAX_CPP_VARS];
extern int num_cpp_vars;

extern int cc_debug;
extern int cc_optimize;

extern int Make_Old2NewWith;

extern unsigned int optimize;

#define OPT_NONE 0x00000000 /* all optimizations disabled          */
#define OPT_ALL 0xFFFFFFFF  /* all optimizations enabled           */

#define OPT_DCR 0x00000001  /* dead code removal                   */
#define OPT_CF 0x00000002   /* constant folding                    */
#define OPT_LIR 0x00000004  /* loop invariant removal              */
#define OPT_INL 0x00000008  /* function inlining                   */
#define OPT_LUR 0x00000010  /* loop unrolling                      */
#define OPT_WLUR 0x00000020 /* with-loop unrolling                 */
#define OPT_LUS 0x00000040  /* loop unswitching                    */
#define OPT_CSE 0x00000080  /* common subexpression elimination    */
#define OPT_DFR 0x00000100  /* dead function removal               */
#define OPT_WLT 0x00000200  /* with-loop transformation            */
#define OPT_WLF 0x00000400  /* with-loop folding                   */
#define OPT_IVE 0x00000800  /* index vector elimination            */
#define OPT_AE 0x00001000   /* array elimination                   */
#define OPT_DLAW 0x00002000 /* distributive law                    */
#define OPT_RCO 0x00004000  /* reference count optimization        */
#define OPT_UIP 0x00008000  /* update-in-place analysis            */
#define OPT_TSI 0x00010000  /* with-loop tile size inference       */
#define OPT_TSP 0x00020000  /* with-loop tile size pragmas         */
#define OPT_MTO 0x00040000  /* multi-thread optimization           */
#define OPT_SBE 0x00080000  /* synchronisation barrier elimination */

extern int optvar;
extern int inlnum;
extern int unrnum;
extern int wlunrnum;
extern int minarray;
extern int max_overload;
extern int max_optcycles;

extern int show_refcnt;
extern int show_idx;

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

extern unsigned int runtimecheck;

#define RUNTIMECHECK_NONE 0x0000
#define RUNTIMECHECK_ALL 0xffff

#define RUNTIMECHECK_MALLOC 0x0001
#define RUNTIMECHECK_BOUNDARY 0x0002
#define RUNTIMECHECK_ERRNO 0x0004

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
extern char *filename;
extern char *compiler_phase_name[];

extern compiler_phase_t my_dbug_from;
extern compiler_phase_t my_dbug_to;
extern int my_dbug;
extern int my_dbug_active;
extern char *my_dbug_str;

extern unsigned int total_allocated_mem;
extern unsigned int current_allocated_mem;
extern unsigned int max_allocated_mem;

extern int print_objdef_for_header_file;
extern int function_counter;
extern deps *dependencies;

extern int indent;

#endif /* _sac_globals_h */
