/*
 *
 * $Log$
 * Revision 1.13  2004/11/25 16:55:42  skt
 * big compiler switch during SACDevCampDK 2k4
 *
 * Revision 1.12  2004/11/25 16:23:43  skt
 * sone debugging ...
 *
 * Revision 1.11  2004/11/25 15:25:56  skt
 * made the function names consistent
 *
 * Revision 1.10  2004/11/22 21:29:55  ktr
 * Big Switch Header! SacDevCamp 04
 *
 * Revision 1.9  2004/07/04 00:54:47  dkrHH
 * GET_SHAPE_IDX modified: shape is a SHPSEG again...
 *
 * Revision 1.8  2004/03/09 23:57:59  dkrHH
 * old backend removed
 *
 * Revision 1.7  2002/08/08 13:02:40  dkr
 * GET_SHAPE_IDX modified: shape is no SHPSEG but a int*
 *
 * Revision 1.6  2002/07/15 14:45:55  dkr
 * signature of NodeOrInt_MakeIndex() modified
 *
 * Revision 1.5  2002/07/15 14:28:12  dkr
 * modifications for new backend done
 *
 * Revision 1.4  2001/06/28 08:50:03  sbs
 * _sac_wl_bounds_h_ after endif preproc directive put into
 * comment. gcc 3.0 was complaining.
 *
 * Revision 1.3  2001/04/02 16:02:16  dkr
 * NodeOrInt_MakeIndex modified
 * NameOrVal_MakeIndex removed
 * NodeOrInt_Print added
 *
 * Revision 1.2  2001/04/02 11:41:07  dkr
 * includes added
 *
 * Revision 1.1  2001/03/30 14:07:30  dkr
 * Initial revision
 *
 */

#ifndef _SAC_WL_BOUNDS_H_
#define _SAC_WL_BOUNDS_H_

#include "types.h"

/******************************************************************************
 *
 *  Modul: wl_bounds
 *
 *  Prefix: WLB
 *
 *  Description:
 *
 *    The index bounds for a with-loop in internal representation (N_Nwith2)
 *    are stored in the AST as values of type 'int' (N_WLseg, N_WLblock,
 *    N_WLublock, N_WLstride, N_WLgrid) or as N_id/N_num nodes (N_WLsegVar,
 *    N_WLstrideVar, N_WLgridVar).
 *
 *    This module provides functions for handling all the different
 *    respresentations of index bounds uniformly.
 *
 ******************************************************************************/
/*
 * symbolic bounds for strides/grids and IDX_MIN, IDX_MAX
 */
#define IDX_SHAPE (-1) /* equals the shape */
#define IDX_OTHER (-2) /* other */

extern bool WLBnameOrValCheckConsistency (const char *name, int val);

extern bool WLBnameOrValIsInt (const char *name, int val);
extern bool WLBnodeOrIntIsInt (nodetype nt, void *node_or_int);

extern void WLBnodeOrIntGetNameOrVal (char **ret_name, int *ret_val, nodetype nt,
                                      void *node_or_int);

extern void WLBnodeOrIntSetNodeOrInt (nodetype ret_nt, void *ret_node_or_int, nodetype nt,
                                      void *node_or_int);

extern node *WLBnodeOrIntMakeNode (nodetype nt, void *node_or_int);

extern node *WLBnodeOrIntMakeIndex (nodetype nt, void *node_or_int, int dim,
                                    node *wl_ids);

extern bool WLBnameOrValEq (const char *name1, int val1, const char *name2, int val2,
                            int shape);
extern bool WLBnodeOrIntEq (nodetype nt1, void *node_or_int1, nodetype nt2,
                            void *node_or_int2, int shape);
extern bool WLBnodeOrIntIntEq (nodetype nt1, void *node_or_int1, int val2, int shape);
extern bool WLBnodeOrIntStrEq (nodetype nt1, void *node_or_int1, const char *name2,
                               int shape);

extern bool WLBnameOrValLe (const char *name1, int val1, const char *name2, int val2,
                            int shape);
extern bool WLBnodeOrIntLe (nodetype nt1, void *node_or_int1, nodetype nt2,
                            void *node_or_int2, int shape);

extern void WLBnodeOrIntPrint (FILE *handle, nodetype nt, void *node_or_int, int dim);

#endif /* _SAC_WL_BOUNDS_H_ */
