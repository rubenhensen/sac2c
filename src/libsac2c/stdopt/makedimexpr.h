/*
 * $Id$
 */
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
extern node *MDEfloat (node *arg_node, info *arg_info);
extern node *MDEdouble (node *arg_node, info *arg_info);
extern node *MakeScalarAvis (char *name);

#endif /* _SAC_MAKEDIMEXPR_H_ */
