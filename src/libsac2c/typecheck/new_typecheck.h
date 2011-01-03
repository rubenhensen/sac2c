/*
 * $Id$
 */

#ifndef _SAC_NEW_TYPECHECK_H_
#define _SAC_NEW_TYPECHECK_H_

#include "types.h"

extern node *NTCdoNewTypeCheck (node *arg_node);
extern node *NTCdoNewReTypeCheck (node *arg_node);
extern node *NTCdoNewReTypeCheckFromScratch (node *arg_node);
extern ntype *NTCnewTypeCheck_Expr (node *arg_node);
extern simpletype NTCnodeToType (node *arg_node);

extern node *NTCmodule (node *arg_node, info *arg_info);
extern node *NTCfundef (node *arg_node, info *arg_info);
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
extern node *NTCtype (node *arg_node, info *arg_info);
extern node *NTCnum (node *arg_node, info *arg_info);
extern node *NTCnumbyte (node *arg_node, info *arg_info);
extern node *NTCnumshort (node *arg_node, info *arg_info);
extern node *NTCnumint (node *arg_node, info *arg_info);
extern node *NTCnumlong (node *arg_node, info *arg_info);
extern node *NTCnumlonglong (node *arg_node, info *arg_info);
extern node *NTCnumubyte (node *arg_node, info *arg_info);
extern node *NTCnumushort (node *arg_node, info *arg_info);
extern node *NTCnumuint (node *arg_node, info *arg_info);
extern node *NTCnumulong (node *arg_node, info *arg_info);
extern node *NTCnumulonglong (node *arg_node, info *arg_info);
extern node *NTCbool (node *arg_node, info *arg_info);
extern node *NTCchar (node *arg_node, info *arg_info);
extern node *NTCdouble (node *arg_node, info *arg_info);
extern node *NTCfloat (node *arg_node, info *arg_info);
extern node *NTCstr (node *arg_node, info *arg_info);

extern node *NTCwith (node *arg_node, info *arg_info);
extern node *NTCpart (node *arg_node, info *arg_info);
extern node *NTCgenerator (node *arg_node, info *arg_info);
extern node *NTCwithid (node *arg_node, info *arg_info);
extern node *NTCcode (node *arg_node, info *arg_info);
extern node *NTCgenarray (node *arg_node, info *arg_info);
extern node *NTCmodarray (node *arg_node, info *arg_info);
extern node *NTCfold (node *arg_node, info *arg_info);
extern node *NTCbreak (node *arg_node, info *arg_info);
extern node *NTCpropagate (node *arg_node, info *arg_info);

extern node *NTCtriggerTypeCheck (node *fundef);

#endif /* _SAC_NEW_TYPECHECK_H_ */
