/*
 *
 * $Log$
 * Revision 1.1  2001/03/30 14:07:30  dkr
 * Initial revision
 *
 */

#ifndef _sac_wl_bounds_h_
#define _sac_wl_bounds_h_

/******************************************************************************
 *
 *  Modul: wl_bounds
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

#define IDX_IS_NUM(idx) ((idx) >= 0)

#define GET_SHAPE_IDX(shape, dim)                                                        \
    (((shape) != NULL) ? SHPSEG_SHAPE ((shape), (dim)) : IDX_SHAPE)

extern bool NameOrVal_CheckConsistency (char *name, int val);

extern bool NameOrVal_IsInt (char *name, int val);
extern bool NodeOrInt_IsInt (nodetype nt, void *node_or_int);

extern void NodeOrInt_GetNameOrVal (char **ret_name, int *ret_val, nodetype nt,
                                    void *node_or_int);

extern void NameOrVal_SetNodeOrInt (nodetype ret_nt, void *ret_node_or_int, char *name,
                                    int val);
extern void NodeOrInt_SetNodeOrInt (nodetype ret_nt, void *ret_node_or_int, nodetype nt,
                                    void *node_or_int);

extern node *NameOrVal_MakeNode (char *name, int val);
extern node *NodeOrInt_MakeNode (nodetype nt, void *node_or_int);

extern node *NameOrVal_MakeIndex (char *name, int val, int dim, char *wl_name,
                                  bool no_num, bool no_icm);
extern node *NodeOrInt_MakeIndex (nodetype nt, void *node_or_int, int dim, char *wl_name,
                                  bool no_num, bool no_icm);

extern bool NameOrVal_Eq (char *name1, int val1, char *name2, int val2, int shape);
extern bool NodeOrInt_Eq (nodetype nt1, void *node_or_int1, nodetype nt2,
                          void *node_or_int2, int shape);
extern bool NodeOrInt_IntEq (nodetype nt1, void *node_or_int1, int val2, int shape);
extern bool NodeOrInt_StrEq (nodetype nt1, void *node_or_int1, char *name2, int shape);

extern bool NameOrVal_Le (char *name1, int val1, char *name2, int val2, int shape);
extern bool NodeOrInt_Le (nodetype nt1, void *node_or_int1, nodetype nt2,
                          void *node_or_int2, int shape);

#endif _sac_wl_bounds_h_
