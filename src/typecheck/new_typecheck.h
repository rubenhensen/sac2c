/*
 *
 * $Log$
 * Revision 3.15  2005/07/24 20:01:50  sah
 * moved all the preparations for typechecking
 * into a different phase
 *
 * Revision 3.14  2004/11/25 18:01:40  sbs
 * compiles
 *
 * Revision 3.13  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 3.12  2004/11/14 15:20:47  sah
 * made CheckUdtAndSetBaseType visible
 *
 * Revision 3.11  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.10  2004/03/05 19:32:28  sbs
 * NTCfuncond added.
 *
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

#ifndef _SAC_NEW_TYPECHECK_H_
#define _SAC_NEW_TYPECHECK_H_

#include "types.h"

extern node *NTCdoNewTypeCheck (node *arg_node);
extern ntype *NTCnewTypeCheck_Expr (node *arg_node);

extern ntype *NTCcheckUdtAndSetBaseType (usertype udt, int *visited);

extern node *NTCmodule (node *arg_node, info *arg_info);
extern node *NTCfundef (node *arg_node, info *arg_info);
extern node *NTCarg (node *arg_node, info *arg_info);
extern node *NTCblock (node *arg_node, info *arg_info);
extern node *NTCvardec (node *arg_node, info *arg_info);
extern node *NTCassign (node *arg_node, info *arg_info);
extern node *NTCcond (node *arg_node, info *arg_info);
extern node *NTCfuncond (node *arg_node, info *arg_info);
extern node *NTClet (node *arg_node, info *arg_info);
extern node *NTCreturn (node *arg_node, info *arg_info);
extern node *NTCap (node *arg_node, info *arg_info);
extern node *NTCprf (node *arg_node, info *arg_info);
extern node *NTCarray (node *arg_node, info *arg_info);
extern node *NTCcast (node *arg_node, info *arg_info);
extern node *NTCexprs (node *arg_node, info *arg_info);
extern node *NTCid (node *arg_node, info *arg_info);
extern node *NTCglobobj (node *arg_node, info *arg_info);
extern node *NTCnum (node *arg_node, info *arg_info);
extern node *NTCbool (node *arg_node, info *arg_info);
extern node *NTCchar (node *arg_node, info *arg_info);
extern node *NTCdouble (node *arg_node, info *arg_info);
extern node *NTCfloat (node *arg_node, info *arg_info);

extern node *NTCwith (node *arg_node, info *arg_info);
extern node *NTCpart (node *arg_node, info *arg_info);
extern node *NTCgenerator (node *arg_node, info *arg_info);
extern node *NTCwithid (node *arg_node, info *arg_info);
extern node *NTCcode (node *arg_node, info *arg_info);
extern node *NTCgenarray (node *arg_node, info *arg_info);
extern node *NTCmodarray (node *arg_node, info *arg_info);
extern node *NTCfold (node *arg_node, info *arg_info);

extern node *NTCtriggerTypeCheck (node *fundef);

#endif /* _SAC_NEW_TYPECHECK_H_ */
