#ifndef SAC_COMPARETREE_H
#define SAC_COMPARETREE_H

#include "types.h"
#include "LookUpTable.h"

/*****************************************************************************
 *
 * file:   compare_tree.h
 *
 * prefix: CMPT
 *
 * description:
 *   this module implements a literal tree compare for two given parts of
 *   the ast. it compares for equal structre, identifiers and values.
 *   this modules is used by SSACSE to find common subexpressions.
 *
 * returns:
 *    CMPT_EQ     tree1 == tree2
 *    CMPT_NEQ    tree1 != tree2 (or some unsupported nodes in tree)
 *
 * remarks:
 *   identifier are compared by their avis pointers.
 *****************************************************************************/
extern cmptree_t CMPTdoCompareTreeLUT (node *tree1, node *tree2, lut_t *lut);
extern cmptree_t CMPTdoCompareTree (node *tree1, node *tree2);

extern node *CMPTnum (node *arg_node, info *arg_info);
extern node *CMPTnumbyte (node *arg_node, info *arg_info);
extern node *CMPTnumshort (node *arg_node, info *arg_info);
extern node *CMPTnumint (node *arg_node, info *arg_info);
extern node *CMPTnumlong (node *arg_node, info *arg_info);
extern node *CMPTnumlonglong (node *arg_node, info *arg_info);
extern node *CMPTnumubyte (node *arg_node, info *arg_info);
extern node *CMPTnumushort (node *arg_node, info *arg_info);
extern node *CMPTnumuint (node *arg_node, info *arg_info);
extern node *CMPTnumulong (node *arg_node, info *arg_info);
extern node *CMPTnumulonglong (node *arg_node, info *arg_info);
extern node *CMPTchar (node *arg_node, info *arg_info);
extern node *CMPTbool (node *arg_node, info *arg_info);
extern node *CMPTfloat (node *arg_node, info *arg_info);
extern node *CMPTfloatvec (node *arg_node, info *arg_info);
extern node *CMPTdouble (node *arg_node, info *arg_info);
extern node *CMPTtype (node *arg_node, info *arg_info);
extern node *CMPTstr (node *arg_node, info *arg_info);
extern node *CMPTid (node *arg_node, info *arg_info);
extern node *CMPTspid (node *arg_node, info *arg_info);
extern node *CMPTids (node *arg_node, info *arg_info);
extern node *CMPTarray (node *arg_node, info *arg_info);
extern node *CMPTprf (node *arg_node, info *arg_info);
extern node *CMPTap (node *arg_node, info *arg_info);
extern node *CMPTgenerator (node *arg_node, info *arg_info);
extern node *CMPTfold (node *arg_node, info *arg_info);

extern node *CMPTunknown (node *arg_node, info *arg_info);

/* call of own traversal mechanism:*/
extern node *CMPTblock (node *arg_node, info *arg_info);
extern node *CMPTassign (node *arg_node, info *arg_info);
extern node *CMPTlet (node *arg_node, info *arg_info);
extern node *CMPTreturn (node *arg_node, info *arg_info);
extern node *CMPTcond (node *arg_node, info *arg_info);
extern node *CMPTdo (node *arg_node, info *arg_info);
extern node *CMPTfuncond (node *arg_node, info *arg_info);
extern node *CMPTexprs (node *arg_node, info *arg_info);
extern node *CMPTempty (node *arg_node, info *arg_info);
extern node *CMPTwith (node *arg_node, info *arg_info);
extern node *CMPTpart (node *arg_node, info *arg_info);
extern node *CMPTwithid (node *arg_node, info *arg_info);
extern node *CMPTcode (node *arg_node, info *arg_info);
extern node *CMPTgenarray (node *arg_node, info *arg_info);
extern node *CMPTmodarray (node *arg_node, info *arg_info);

/* pre-travesal function */
extern node *CMPTnodeType (node *arg_node, info *arg_info);

#endif /* SAC_CHECKAVIS_H */
