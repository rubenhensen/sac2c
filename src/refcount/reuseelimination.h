/*
 *
 * $Log$
 * Revision 1.6  2004/12/01 16:35:23  ktr
 * Rule for Block added.
 *
 * Revision 1.5  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.4  2004/11/07 14:25:24  ktr
 * ongoing implementation.
 *
 * Revision 1.3  2004/11/02 14:33:25  ktr
 * Reuseelimination is now performed seperately for each branch of a cond.
 *
 * Revision 1.2  2004/10/22 15:38:19  ktr
 * Ongoing implementation.
 *
 * Revision 1.1  2004/10/22 14:13:56  ktr
 * Initial revision
 *
 */
#ifndef _SAC_REUSEELIMINATION_H_
#define _SAC_REUSEELIMINATION_H_

#include "types.h"

/******************************************************************************
 *
 * Reuse elimination traversal ( emre_tab)
 *
 * Prefix: EMRE
 *
 *****************************************************************************/
extern node *EMREdoReuseElimination (node *syntax_tree);

extern node *EMREassign (node *arg_node, info *arg_info);
extern node *EMREblock (node *arg_node, info *arg_info);
extern node *EMREcond (node *arg_node, info *arg_info);
extern node *EMREfundef (node *arg_node, info *arg_info);
extern node *EMREgenarray (node *arg_node, info *arg_info);
extern node *EMRElet (node *arg_node, info *arg_info);
extern node *EMREmodarray (node *arg_node, info *arg_info);
extern node *EMREprf (node *arg_node, info *arg_info);
extern node *EMREvardec (node *arg_node, info *arg_info);

#endif /* _SAC_REUSEELIMINATION_H_ */
