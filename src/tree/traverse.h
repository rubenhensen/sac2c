/*
 *
 * $Log$
 * Revision 3.88  2004/11/23 22:22:03  sah
 * rewrite
 *
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 */

#ifndef _SAC_TRAVERSE_H_
#define _SAC_TRAVERSE_H_

#include "types.h"

extern node *TRAVdo (node *arg_node, info *arg_info);
extern void TRAVpush (trav_t traversal);
extern trav_t TRAVpop ();

#endif /* _SAC_TRAVERSE_H_ */
