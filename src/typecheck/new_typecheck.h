/*
 *
 * $Log$
 * Revision 3.2  2001/03/22 21:04:15  dkr
 * no changes done
 *
 * Revision 3.1  2000/11/20 18:00:08  sacbase
 * new release made
 *
 * Revision 1.1  1999/10/20 12:51:25  sbs
 * Initial revision
 *
 */

#ifndef _new_typecheck_h
#define _new_typecheck_h

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *NewTypeCheck (node *arg_node);
extern node *NTCmodul (node *arg_node, node *arg_info);
extern node *NTCtypedef (node *arg_node, node *arg_info);

#endif /* _new_typecheck_h */
