/*
 *
 * $Log$
 * Revision 1.1  2005/02/14 11:15:46  cg
 * Initial revision
 *
 *
 */

#ifndef _SAC_PREPARE_INLINING_H_
#define _SAC_PREPARE_INLINING_H_

#include "types.h"

extern node *PINLdoPrepareInlining (node **vardecs, node *fundef, node *letids,
                                    node *apargs);

extern node *PINLfundef (node *arg_node, info *arg_info);
extern node *PINLvardec (node *arg_node, info *arg_info);
extern node *PINLavis (node *arg_node, info *arg_info);
extern node *PINLblock (node *arg_node, info *arg_info);
extern node *PINLassign (node *arg_node, info *arg_info);
extern node *PINLid (node *arg_node, info *arg_info);
extern node *PINLids (node *arg_node, info *arg_info);
extern node *PINLarg (node *arg_node, info *arg_info);

#endif
