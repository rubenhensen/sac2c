/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:54  sacbase
 * new release made
 *
 * Revision 1.67  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.66  1998/12/10 17:28:27  sbs
 * MRD_GETSUBST changed.
 *
 * Revision 1.65  1998/08/06 15:06:39  dkr
 * OPTicm inserted
 *
 * Revision 1.64  1998/05/15 15:46:14  srs
 * added comment
 *
 * Revision 1.63  1998/05/13 13:44:32  srs
 * renamed unr_expr to lunr_expr and inserted wlunr_expr
 *
 * Revision 1.62  1998/05/12 15:11:41  srs
 * added ae_tab to MRD_TAB
 *
 * Revision 1.61  1998/04/29 17:15:58  dkr
 * removed OPTSpmd
 *
 * Revision 1.60  1998/04/26 21:53:14  dkr
 * removed OPTSync
 *
 * Revision 1.59  1998/04/23 17:33:50  dkr
 * added OptSync
 *
 * Revision 1.58  1998/04/17 17:26:43  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.57  1998/04/09 12:01:40  dkr
 * ReGenMask() is now external
 *
 * Revision 1.56  1998/04/03 21:07:14  dkr
 * added OPTconc
 *
 * Revision 1.55  1998/04/03 11:58:12  srs
 * exported old_wlf_expr and old_wlt_expr to be visible in WithloopFolding.c
 *
 * Revision 1.54  1998/04/02 13:47:23  srs
 * NOTE_OPTIMIZER_PHASE() deleted
 *
 * Revision 1.53  1998/03/22 18:11:56  srs
 * added wlt_expr and NOTE_OPTIMIZER_PHASE()
 *
 * Revision 1.52  1998/03/21 22:46:54  srs
 * added wlt_tab to MRD_TAB
 *
 * Revision 1.51  1998/03/04 14:39:16  srs
 * changed wlf_tab to wli_tab in MRD_TAB
 *
 * Revision 1.50  1998/02/25 09:16:03  cg
 * Definition of global variable mrdl_stack moved to optimize.c
 *
 * Revision 1.49  1998/02/16 16:53:30  srs
 * added wlf_tab to MRD_TAB
 *
 * Revision 1.48  1998/02/13 13:40:15  srs
 * removed macro TOS (same as MRD_TOS)
 *
 * Revision 1.47  1998/02/12 11:10:26  srs
 * added OPTNwith
 *
 * Revision 1.46  1998/02/09 16:10:12  srs
 * introduced WLF
 *
 * Revision 1.45  1997/10/28 13:14:11  srs
 * removed dead code
 *
 * Revision 1.44  1997/04/25 12:13:00  sbs
 * MAlloc replaced by Malloc from internal.lib
 *
 * Revision 1.43  1997/04/24  15:03:43  sbs
 * HAVE_MALLOC_O inserted
 *
 * Revision 1.42  1996/08/09  16:42:52  asi
 * dead function removal added
 *
 * Revision 1.41  1996/02/13  13:58:14  asi
 * MRD_GETCSE changed
 *
 * Revision 1.40  1996/01/17  16:49:21  asi
 * added common subexpression elimination
 *
 * Revision 1.38  1996/01/17  14:48:39  asi
 * global variables moved to globals.h
 * added typedefs stelm and stack
 * added defines MIN_STACK_SIZE, MAKE_MRDL_STACK, FREE_MRDL_STACK, TOS, MRD_TOS, MRD_TAB
 * MRD_LIST, MRD, MRD_GETSUBST, MRD_GETCS, MRD_GETLAST, MRD_GETDATA
 * added external definitions for functions MrdGet, GetExpr, PushMRDL, PushDupMRDL
 * PopMRDL, PopMRDL2
 *
 * ... [eliminated] ...
 *
 * Revision 1.1  1994/12/09  10:48:23  sbs
 * Initial revision
 *
 *
 */

#ifndef _sac_optimize_h

#define _sac_optimize_h

#include "free.h"

#define VAR_LENGTH 10

extern int dead_expr;
extern int dead_var;
extern int dead_fun;
extern int cf_expr;
extern int lir_expr;
extern int inl_fun;
extern int lunr_expr;
extern int wlunr_expr;
extern int uns_expr;
extern int optvar_counter;
extern int elim_arrays;
extern int cse_expr;
extern int wlf_expr;
extern int wlt_expr;
extern int old_wlf_expr, old_wlt_expr;

extern node *GetExpr (node *arg_node);

extern node *Optimize (node *arg_node);
extern node *OPTmodul (node *arg_node, node *arg_info);
extern node *OPTfundef (node *arg_node, node *arg_info);

#endif /* _sac_optimize_h */
