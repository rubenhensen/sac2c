/*
 *
 * $Log$
 * Revision 1.24  1998/04/25 12:36:44  sbs
 * sorry, I wrote level instead of indent 8-(
 *
 * Revision 1.23  1998/04/25 12:02:17  sbs
 * indent inserted
 *
 * Revision 1.22  1998/03/24 11:48:31  cg
 * added global variables used to gather compile time information
 * later used for profiling.
 *
 * Revision 1.21  1998/03/17 14:21:58  cg
 * file src/compile/trace.h removed.
 * definition of symbolic values of global variable traceflag moved to globals.h
 *
 * Revision 1.20  1998/03/13 13:15:57  dkr
 * removed a bug with flag _DBUG:
 *   new var 'my?dbug'
 *   vars 'dbug_...' renamed in 'my_dbug...'
 *
 * Revision 1.19  1998/03/04 16:20:08  cg
 * added  cc_debug,  cc_optimize, tmp_dirname.
 * removed ccflagsstr, useranlib.
 *
 * Revision 1.18  1998/03/02 13:57:07  cg
 *  global variables psi_optimize and backend_optimize removed.
 *
 * Revision 1.17  1998/02/26 15:22:58  cg
 * added declaration of target_name
 *
 * Revision 1.16  1998/02/25 09:21:08  cg
 * Unnecessary global variable removed.
 *
 * Revision 1.15  1998/02/06 13:33:19  srs
 * extern int opt_wlf;
 *
 * Revision 1.14  1997/10/29 14:34:53  srs
 * added more counters for malloc statistics
 *
 * Revision 1.13  1997/10/09 13:54:12  srs
 * counter for memory allocation
 *
 * Revision 1.12  1997/06/03 10:14:46  sbs
 * -D option integrated
 *
 * Revision 1.11  1997/05/27  09:18:25  sbs
 * analyseflag renamed to profileflag
 *
 * Revision 1.10  1997/05/14  08:17:34  sbs
 * analyseflag activated
 *
 * Revision 1.9  1997/03/19  13:35:34  cg
 * added new global variables deps *dependencies and file_type filetype
 *
 * Revision 1.8  1996/09/11  06:29:43  cg
 * Added some new global variables for new command line options.
 *
 * Revision 1.7  1996/08/09  16:44:12  asi
 * dead function removal added
 *
 * Revision 1.6  1996/05/29  14:18:57  sbs
 * inserted noRCO opt_rco!
 *
 * Revision 1.5  1996/01/17  16:49:21  asi
 * added common subexpression elimination
 *
 * Revision 1.4  1996/01/16  16:42:22  cg
 * added global variable int check_malloc
 *
 * Revision 1.3  1996/01/05  12:25:18  cg
 * added cleanup and targetdir[]
 *
 * Revision 1.2  1996/01/02  15:45:44  cg
 * include of filemgr.h no longer necessary
 *
 * Revision 1.1  1995/12/30  14:47:06  cg
 * Initial revision
 *
 *
 *
 *
 */

/*
 * File : globals.c
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

extern FILE *outfile;

extern char sacfilename[];
extern char outfilename[];
extern char modulename[];
extern char cfilename[];
extern char targetdir[];
extern char commandline[];
extern file_type filetype;
extern char *tmp_dirname;

extern char target_name[];

extern int num_threads;

extern char *cppvars[];
extern int num_cpp_vars;

extern int cc_debug;
extern int cc_optimize;

extern int optimize;
extern int opt_dcr;
extern int opt_dfr;
extern int opt_cf;
extern int opt_lir;
extern int opt_inl;
extern int opt_unr;
extern int opt_uns;
extern int opt_cse;
extern int opt_wlf;
extern int opt_ae;
extern int opt_ive;
extern int opt_rco;

extern int optvar;
extern int inlnum;
extern int unrnum;
extern int minarray;
extern int max_overload;
extern int max_optcycles;

extern int show_refcnt;
extern int show_idx;
extern int show_icm;

extern int traceflag;

#define NO_TRACE 0x0000
#define TRACE_UDF 0x0001
#define TRACE_PRF 0x0002
#define TRACE_WST 0x0004
#define TRACE_REF 0x0008
#define TRACE_MEM 0x0010
#define TRACE_ALL 0xffff
/*
 * Allowed values of traceflag
 */

extern int profileflag;

#define NO_PROFILE 0x0000
#define PROFILE_FUN 0x0001
#define PROFILE_INL 0x0002
#define PROFILE_LIB 0x0004
#define PROFILE_WITH 0x0008
#define PROFILE_ALL 0xffff
/*
 * Allowed values of profileflag
 */

extern int PFfuncntr;
extern char *PFfunnme[PF_MAXFUN];
extern int PFfunapcntr[PF_MAXFUN];
extern int PFfunapline[PF_MAXFUN][PF_MAXFUNAP];
extern int PFfunapmax;

extern int check_boundary;
extern int check_malloc;

extern int libstat;
extern int linkstyle;
extern int cleanup;
extern int makedeps;
extern compiler_phase_t break_after;
extern char break_specifier[];

extern int print_objdef_for_header_file;
extern int function_counter;
extern deps *dependencies;

extern unsigned int total_allocated_mem;
extern unsigned int current_allocated_mem;
extern unsigned int max_allocated_mem;

extern int errors;
extern int warnings;
extern int last_indent;
extern int current_line_length;
extern int message_indent;
extern int verbose_level;
extern compiler_phase_t compiler_phase;

extern char *filename;
extern char *compiler_phase_name[];
extern char error_message_buffer[];

extern compiler_phase_t my_dbug_from;
extern compiler_phase_t my_dbug_to;
extern int my_dbug;
extern int my_dbug_active;
extern char *my_dbug_str;

extern int Make_Old2NewWith;
extern int indent;

#endif /* _sac_globals_h */
