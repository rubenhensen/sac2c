/*
 * $Log$
 * Revision 1.1  1999/10/20 12:51:25  sbs
 * Initial revision
 *
 *
 */

#ifndef _new_typecheck_h
#define _new_typecheck_h

#include "tree.h"

extern node *NewTypeCheck (node *arg_node);
extern node *NTCmodul (node *arg_node, node *arg_info);
extern node *NTCtypedef (node *arg_node, node *arg_info);

#endif /* _new_typecheck_h */
