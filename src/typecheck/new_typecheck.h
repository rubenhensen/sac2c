/*
 *
 * $Log$
 * Revision 3.9  2002/10/30 12:12:51  sbs
 * NTCvardec added for converting old vardecs in ntype ones.
 *
 * Revision 3.8  2002/10/28 14:04:37  sbs
 * NTCcast added.
 *
 * Revision 3.7  2002/10/18 14:36:03  sbs
 * NTCobjdef added.
 *
 * Revision 3.6  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 3.5  2002/08/30 16:36:45  dkr
 * NewTypeCheck_Expr() added
 *
 * Revision 3.4  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker !!
 *
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
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *NewTypeCheck (node *arg_node);
extern ntype *NewTypeCheck_Expr (node *arg_node);

extern node *NTCtypedef (node *arg_node, node *arg_info);
extern node *NTCobjdef (node *arg_node, node *arg_info);
extern node *NTCmodul (node *arg_node, node *arg_info);
extern node *NTCfundef (node *arg_node, node *arg_info);
extern node *NTCarg (node *arg_node, node *arg_info);
extern node *NTCblock (node *arg_node, node *arg_info);
extern node *NTCvardec (node *arg_node, node *arg_info);
extern node *NTCassign (node *arg_node, node *arg_info);
extern node *NTCcond (node *arg_node, node *arg_info);
extern node *NTClet (node *arg_node, node *arg_info);
extern node *NTCreturn (node *arg_node, node *arg_info);
extern node *NTCap (node *arg_node, node *arg_info);
extern node *NTCprf (node *arg_node, node *arg_info);
extern node *NTCarray (node *arg_node, node *arg_info);
extern node *NTCcast (node *arg_node, node *arg_info);
extern node *NTCexprs (node *arg_node, node *arg_info);
extern node *NTCid (node *arg_node, node *arg_info);
extern node *NTCnum (node *arg_node, node *arg_info);
extern node *NTCbool (node *arg_node, node *arg_info);
extern node *NTCchar (node *arg_node, node *arg_info);
extern node *NTCdouble (node *arg_node, node *arg_info);
extern node *NTCfloat (node *arg_node, node *arg_info);

extern node *NTCNwith (node *arg_node, node *arg_info);
extern node *NTCNpart (node *arg_node, node *arg_info);
extern node *NTCNgenerator (node *arg_node, node *arg_info);
extern node *NTCNwithid (node *arg_node, node *arg_info);
extern node *NTCNcode (node *arg_node, node *arg_info);
extern node *NTCNwithop (node *arg_node, node *arg_info);

extern node *NTCTriggerTypeCheck (node *fundef);

#endif /* _new_typecheck_h */
