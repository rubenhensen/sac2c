/*
 *
 * $Log$
 * Revision 3.13  2004/12/09 10:59:30  mwe
 * support for type_upgrade added
 *
 * Revision 3.12  2004/11/26 17:54:18  skt
 * renamed OPTmodul into OPTmodule
 *
 * Revision 3.11  2004/11/22 18:10:19  sbs
 * SacDevCamp04
 *
 * Revision 3.10  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 3.9  2004/06/30 12:13:39  khf
 * wlpg_expr removed
 *
 * Revision 3.8  2004/04/08 08:09:55  khf
 * support for wlfs and wlpg added but are currently
 * deactivated in global.c
 *
 * Revision 3.7  2004/03/02 16:49:49  mwe
 * support for CVP added
 *
 * Revision 3.6  2003/08/16 08:44:25  ktr
 * SelectionPropagation added. Must currently be activated with -dosp.
 *
 * Revision 3.5  2003/02/08 16:00:01  mwe
 * dl_expr added
 *
 * Revision 3.4  2002/06/07 17:16:08  mwe
 * al_expr added
 *
 * Revision 3.3  2002/03/13 16:03:20  ktr
 * wls_expr declared
 *
 * Revision 3.2  2001/04/19 16:34:14  nmw
 * statistics for wlir added
 *
 * Revision 3.1  2000/11/20 18:00:45  sacbase
 * new release made
 *
 *
 * ... [eliminated] ...
 *
 * Revision 1.1  1994/12/09  10:48:23  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_OPTIMIZE_H_
#define _SAC_OPTIMIZE_H_

#include "types.h"

#define VAR_LENGTH 10

extern int dead_expr;
extern int dead_var;
extern int dead_fun;
extern int cf_expr;
extern int lir_expr;
extern int wlir_expr;
extern int inl_fun;
extern int lunr_expr;
extern int wlunr_expr;
extern int uns_expr;
extern int optvar_counter;
extern int elim_arrays;
extern int cse_expr;
extern int wlf_expr;
extern int wlt_expr;
extern int wls_expr;
extern int al_expr;
extern int old_wlf_expr, old_wlt_expr;
extern int ap_padded;
extern int ap_unsupported;
extern int dl_expr;
extern int sp_expr;
extern int cvp_expr;
extern int wlfs_expr;
extern int tup_expr;

extern node *OPTdoOptimize (node *arg_node);
extern node *OPTmodule (node *arg_node, info *arg_info);
extern node *OPTfundef (node *arg_node, info *arg_info);

#endif /* _SAC_OPTIMIZE_H_ */
