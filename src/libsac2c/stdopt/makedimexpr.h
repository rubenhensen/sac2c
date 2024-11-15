#ifndef _SAC_MAKEDIMEXPR_H_
#define _SAC_MAKEDIMEXPR_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Make Dim Expression traversal ( mde_tab)
 *
 * Prefix: MDE
 *
 *****************************************************************************/
extern node *MDEdoMakeDimExpression (node *expr, node *avis, node *allids, node *fundef);

extern node *MDEfundef (node *arg_node, info *arg_info);
extern node *MDEassign (node *arg_node, info *arg_info);
extern node *MDEid (node *arg_node, info *arg_info);
extern node *MDEfuncond (node *arg_node, info *arg_info);
extern node *MDEap (node *arg_node, info *arg_info);
extern node *MDEarray (node *arg_node, info *arg_info);
extern node *MDEprf (node *arg_node, info *arg_info);
extern node *MDEwith (node *arg_node, info *arg_info);
extern node *MDEbool (node *arg_node, info *arg_info);
extern node *MDEchar (node *arg_node, info *arg_info);
extern node *MDEnum (node *arg_node, info *arg_info);
extern node *MDEnumbyte (node *arg_node, info *arg_info);
extern node *MDEnumshort (node *arg_node, info *arg_info);
extern node *MDEnumint (node *arg_node, info *arg_info);
extern node *MDEnumlong (node *arg_node, info *arg_info);
extern node *MDEnumlonglong (node *arg_node, info *arg_info);
extern node *MDEnumubyte (node *arg_node, info *arg_info);
extern node *MDEnumushort (node *arg_node, info *arg_info);
extern node *MDEnumuint (node *arg_node, info *arg_info);
extern node *MDEnumulong (node *arg_node, info *arg_info);
extern node *MDEnumulonglong (node *arg_node, info *arg_info);
extern node *MDEfloat (node *arg_node, info *arg_info);
extern node *MDEfloatvec (node *arg_node, info *arg_info);
extern node *MDEdouble (node *arg_node, info *arg_info);
extern node *MakeScalarAvis (char *name);

#endif /* _SAC_MAKEDIMEXPR_H_ */
