/*
 * $Log$
 * Revision 1.11  2004/11/25 15:15:00  sbs
 * SSATdoTransformAllowGos renamed into SSATdoTransformAllowGOs
 *
 * Revision 1.10  2004/11/25 13:53:47  mwe
 * SSATids added
 *
 * Revision 1.9  2004/11/24 19:16:24  mwe
 * changing function names according to naming conventions
 *
 * Revision 1.8  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 1.7  2004/09/18 15:57:51  ktr
 * added header for SSATransformExplicitAllocs
 *
 * Revision 1.6  2004/08/07 16:01:29  sbs
 * SSAwith2 added for N_Nwith2 support
 *
 * Revision 1.5  2004/08/07 10:10:14  sbs
 * SSANwithXXX renamed into SSAwithXXXX
 *
 * Revision 1.4  2004/08/06 21:05:24  sbs
 * SSAfuncond added SSAexprs removed
 *
 * Revision 1.3  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.2  2004/02/25 08:22:32  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 *
 * Revision 1.1  2004/01/28 16:53:29  skt
 * Initial revision
 *
 *
 *
 *
 *
 ************ Attention! ************
 * File was moved from ../tree
 * following older Revisions can be found there
 *
 *
 * Revision 1.8  2003/03/12 18:20:58  dkr
 * SSAicm added
 *
 * Revision 1.7  2002/10/18 13:49:54  sbs
 * SSATransformAllowGOs added.
 *
 * Revision 1.6  2001/04/24 16:09:07  nmw
 * SSATransformSingleFundef renamed to SSATransformOneFunction
 *
 * Revision 1.5  2001/04/18 12:58:47  nmw
 * additional traversal setup function for single fundef traversal added
 *
 * Revision 1.4  2001/03/26 13:26:14  nmw
 * SSANewVardec for general usage added
 *
 * Revision 1.3  2001/03/23 09:31:19  nmw
 * SSAwhile/do removed SSADummy added
 *
 * Revision 1.2  2001/02/14 14:40:36  nmw
 * function bodies and traversal order implemented
 *
 * Revision 1.1  2001/02/13 15:16:19  nmw
 * Initial revision
 *
 *
 */

/* this module traverses the AST and transformes the code in SSA form */

#ifndef _SAC_SSATRANSFORM_H_
#define _SAC_SSATRANSFORM_H_

#include "types.h"

/******************************************************************************
 *
 * SSATransform traversal ( ssafrm_tab)
 *
 * Prefix: SSAT
 *
 *****************************************************************************/
extern node *SSATdoTransform (node *ast);
extern node *SSATdoTransformAllowGOs (node *ast);
extern node *SSATdoTransformExplicitAllocs (node *ast);
extern node *SSATdoTransformOneFunction (node *fundef);
extern node *SSATdoTransformOneFundef (node *fundef);

extern node *SSATap (node *arg_node, info *arg_info);
extern node *SSATassign (node *arg_node, info *arg_info);
extern node *SSATblock (node *arg_node, info *arg_info);
extern node *SSATcode (node *arg_node, info *arg_info);
extern node *SSATcond (node *arg_node, info *arg_info);
extern node *SSATfundef (node *arg_node, info *arg_info);
extern node *SSATlet (node *arg_node, info *arg_info);
extern node *SSATicm (node *arg_node, info *arg_info);
extern node *SSATarg (node *arg_node, info *arg_info);
extern node *SSATvardec (node *arg_node, info *arg_info);
extern node *SSATid (node *arg_node, info *arg_info);
extern node *SSATwith (node *arg_node, info *arg_info);
extern node *SSATwith2 (node *arg_node, info *arg_info);
extern node *SSATpart (node *arg_node, info *arg_info);
extern node *SSATwithid (node *arg_node, info *arg_info);
extern node *SSATfuncond (node *arg_node, info *arg_info);
extern node *SSATreturn (node *arg_node, info *arg_info);
extern node *SSATids (node *arg_node, info *arg_info);

extern node *SSATnewVardec (node *old_vardec_or_arg); /* TODO use AVIS instead */

#endif /* _SAC_SSATRANSFORM_H_ */
