#ifndef _SAC_MAKESHAPEEXPR_H_
#define _SAC_MAKESHAPEEXPR_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Make Shape Expression traversal ( mse_tab)
 *
 * Prefix: MSE
 *
 *****************************************************************************/
extern node *MSEdoMakeShapeExpression (node *expr, node *avis, node *allids,
                                       node *fundef);

extern node *MSEid (node *arg_node, info *arg_info);
extern node *MSEfuncond (node *arg_node, info *arg_info);
extern node *MSEap (node *arg_node, info *arg_info);
extern node *MSEarray (node *arg_node, info *arg_info);
extern node *MSEprf (node *arg_node, info *arg_info);
extern node *MSEwith (node *arg_node, info *arg_info);
extern node *MSEbool (node *arg_node, info *arg_info);
extern node *MSEchar (node *arg_node, info *arg_info);
extern node *MSEnum (node *arg_node, info *arg_info);
extern node *MSEnumbyte (node *arg_node, info *arg_info);
extern node *MSEnumshort (node *arg_node, info *arg_info);
extern node *MSEnumint (node *arg_node, info *arg_info);
extern node *MSEnumlong (node *arg_node, info *arg_info);
extern node *MSEnumlonglong (node *arg_node, info *arg_info);
extern node *MSEnumubyte (node *arg_node, info *arg_info);
extern node *MSEnumushort (node *arg_node, info *arg_info);
extern node *MSEnumuint (node *arg_node, info *arg_info);
extern node *MSEnumulong (node *arg_node, info *arg_info);
extern node *MSEnumulonglong (node *arg_node, info *arg_info);
extern node *MSEfloat (node *arg_node, info *arg_info);
extern node *MSEfloatvec (node *arg_node, info *arg_info);
extern node *MSEdouble (node *arg_node, info *arg_info);
extern node *MakeVectAvis (char *name, node *dim);

#endif /* _SAC_MAKESHAPEEXPR_H_ */
