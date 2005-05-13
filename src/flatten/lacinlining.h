/*
 *
 * $Log$
 * Revision 1.1  2005/05/13 16:37:41  ktr
 * Initial revision
 *
 */

#ifndef _SAC_LACINLINING_H_
#define _SAC_LACINLINING_H_

#include "types.h"

extern node *LINLdoLACInlining (node *arg_node);
extern node *LINLdoLACInliningOneFundef (node *arg_node);

extern node *LINLmodule (node *arg_node, info *arg_info);
extern node *LINLfundef (node *arg_node, info *arg_info);
extern node *LINLassign (node *arg_node, info *arg_info);
extern node *LINLlet (node *arg_node, info *arg_info);
extern node *LINLap (node *arg_node, info *arg_info);

#endif /* _SAC_INLINING_H_ */
