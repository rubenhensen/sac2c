/*
 *
 * $Log$
 * Revision 1.8  2004/11/25 11:38:37  ktr
 * COMPILES!
 *
 * Revision 1.7  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.6  2004/09/18 16:07:23  ktr
 * SPMD blocks and functions are treated as well
 *
 * Revision 1.5  2004/08/01 13:18:36  ktr
 * added MMVwlsegx
 *
 * Revision 1.4  2004/07/28 12:27:58  khf
 * MMVassign no longer needed
 *
 * Revision 1.3  2004/07/28 09:08:19  khf
 * support for ExplicitAccumulate added
 *
 * Revision 1.2  2004/07/22 14:13:50  ktr
 * a = fill( b, c) is now converted into c = b
 * which appears to be easier to compile.
 *
 * Revision 1.1  2004/07/21 16:52:46  ktr
 * Initial revision
 *
 *
 */

#ifndef _SAC_MARKMEMVALS_H_
#define _SAC_MARKMEMVALS_H_

#include "types.h"

/******************************************************************************
 *
 * MarkMemVals traversal ( mmv_tab)
 *
 * Prefix: MMV
 *
 *****************************************************************************/
extern node *MMVdoMarkMemVals (node *syntax_tree);

extern node *MMVcode (node *arg_node, info *arg_info);
extern node *MMVfold (node *arg_node, info *arg_info);
extern node *MMVfundef (node *arg_node, info *arg_info);
extern node *MMVgenarray (node *arg_node, info *arg_info);
extern node *MMVid (node *arg_node, info *arg_info);
extern node *MMVlet (node *arg_node, info *arg_info);
extern node *MMVmodarray (node *arg_node, info *arg_info);
extern node *MMVprf (node *arg_node, info *arg_info);
extern node *MMVspmd (node *arg_node, info *arg_info);
extern node *MMVvardec (node *arg_node, info *arg_info);
extern node *MMVwith (node *arg_node, info *arg_info);
extern node *MMVwith2 (node *arg_node, info *arg_info);
extern node *MMVwlseg (node *arg_node, info *arg_info);
extern node *MMVwlsegvar (node *arg_node, info *arg_info);

#endif /* _SAC_MARKMEMVALS_H_ */
