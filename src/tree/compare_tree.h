/*
 * $Log$
 * Revision 1.8  2004/11/22 21:29:55  ktr
 * Big Switch Header! SacDevCamp 04
 *
 * Revision 1.7  2004/08/01 18:44:21  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.6  2003/01/28 18:16:22  ktr
 * CompareTreeLUT added to compare_tree
 *
 * Revision 1.5  2001/04/30 12:24:58  nmw
 * comments corrected
 *
 * Revision 1.4  2001/03/20 14:22:50  nmw
 * CMPTarray added, checks for equal types, too
 *
 * Revision 1.3  2001/03/09 11:48:54  sbs
 * types.h now explicitly included.
 *
 * Revision 1.2  2001/03/07 10:03:40  nmw
 * first implementation
 *
 * Revision 1.1  2001/03/06 13:16:50  nmw
 * Initial revision
 *
 *
 *
 */

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
extern cmptree_t CMPTdoCompareTreeLut (node *tree1, node *tree2, lut_t *lut);
extern cmptree_t CMPTdoCompareTree (node *tree1, node *tree2);

extern node *CMPTnum (node *arg_node, info *arg_info);
extern node *CMPTchar (node *arg_node, info *arg_info);
extern node *CMPTbool (node *arg_node, info *arg_info);
extern node *CMPTstr (node *arg_node, info *arg_info);
extern node *CMPTid (node *arg_node, info *arg_info);
extern node *CMPTfloat (node *arg_node, info *arg_info);
extern node *CMPTdouble (node *arg_node, info *arg_info);
extern node *CMPTarray (node *arg_node, info *arg_info);
extern node *CMPTlet (node *arg_node, info *arg_info);
extern node *CMPTprf (node *arg_node, info *arg_info);
extern node *CMPTap (node *arg_node, info *arg_info);
extern node *CMPTwithid (node *arg_node, info *arg_info);
extern node *CMPTgenerator (node *arg_node, info *arg_info);
extern node *CMPTfold (node *arg_node, info *arg_info);
extern node *CMPTcode (node *arg_node, info *arg_info);
extern node *CMPTunknown (node *arg_node, info *arg_info);
extern node *CMPTtravSons (node *arg_node, info *arg_info);
extern node *CMPTnodeType (node *arg_node, info *arg_info);

#endif /* SAC_CHECKAVIS_H */
