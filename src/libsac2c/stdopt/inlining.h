/*
 *
 * $Log$
 * Revision 1.4  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.3  2005/05/13 16:46:54  ktr
 * removed lacinlining functionality
 *
 * Revision 1.2  2005/04/12 15:50:16  ktr
 * Travsersal invocation function INLdoLACInlining added. Only former loop
 * and conditional functions will be inlined.
 *
 * Revision 1.1  2005/02/14 11:17:37  cg
 * Initial revision
 *
 *
 */

#ifndef _SAC_INLINING_H_
#define _SAC_INLINING_H_

#include "types.h"

extern node *INLdoInlining (node *arg_node);
extern node *INLdoInliningAnon (node *arg_node, info *arg_info);

extern node *INLmodule (node *arg_node, info *arg_info);
extern node *INLfundef (node *arg_node, info *arg_info);
extern node *INLassign (node *arg_node, info *arg_info);
extern node *INLlet (node *arg_node, info *arg_info);
extern node *INLap (node *arg_node, info *arg_info);

#endif /* _SAC_INLINING_H_ */
