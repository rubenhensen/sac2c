/*
 *
 * $Log$
 * Revision 3.3  2002/05/31 14:51:54  sbs
 * intermediate version to ensure compilable overall state.
 *
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
extern node *NTCtypedef (node *arg_node, node *arg_info);
extern node *NTCmodul (node *arg_node, node *arg_info);
extern node *NTCfundef (node *arg_node, node *arg_info);
extern node *NTCarg (node *arg_node, node *arg_info);
extern node *NTCblock (node *arg_node, node *arg_info);
extern node *NTClet (node *arg_node, node *arg_info);
extern node *NTCreturn (node *arg_node, node *arg_info);
extern node *NTCap (node *arg_node, node *arg_info);
extern node *NTCarray (node *arg_node, node *arg_info);
extern node *NTCexprs (node *arg_node, node *arg_info);
extern node *NTCid (node *arg_node, node *arg_info);
extern node *NTCnum (node *arg_node, node *arg_info);
extern node *NTCbool (node *arg_node, node *arg_info);
extern node *NTCchar (node *arg_node, node *arg_info);
extern node *NTCdouble (node *arg_node, node *arg_info);
extern node *NTCfloat (node *arg_node, node *arg_info);

#endif /* _new_typecheck_h */
