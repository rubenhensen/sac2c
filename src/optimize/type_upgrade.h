/*
 *
 * $Log$
 * Revision 1.3  2005/01/11 15:21:11  mwe
 * ongoing implementation
 *
 * Revision 1.2  2004/12/13 14:08:21  mwe
 * new traversals added
 *
 * Revision 1.1  2004/12/08 12:18:07  mwe
 * Initial revision
 *
 */

#ifndef _SAC_TYPEUPGRADE_H_
#define _SAC_TYPEUPGRADE_H_

#include "types.h"

extern node *TUPdoTypeUpgrade (node *arg_node);

extern node *TUPblock (node *arg_node, info *arg_info);
extern node *TUPassign (node *arg_node, info *arg_info);
extern node *TUPlet (node *arg_node, info *arg_info);
extern node *TUPreturn (node *arg_node, info *arg_info);
extern node *TUPwith (node *arg_node, info *arg_info);
extern node *TUPpart (node *arg_node, info *arg_info);
extern node *TUPgenerator (node *arg_node, info *arg_info);
extern node *TUPcode (node *arg_node, info *arg_info);
extern node *TUPap (node *arg_node, info *arg_info);
extern node *TUPids (node *arg_node, info *arg_info);
extern node *TUPfold (node *arg_node, info *arg_info);
extern node *TUPmodarray (node *arg_node, info *arg_info);
extern node *TUPgenarray (node *arg_node, info *arg_info);
extern node *TUPnum (node *arg_node, info *arg_info);
extern node *TUPdouble (node *arg_node, info *arg_info);
extern node *TUPchar (node *arg_node, info *arg_info);
extern node *TUPbool (node *arg_node, info *arg_info);
extern node *TUPprf (node *arg_node, info *arg_info);
extern node *TUParray (node *arg_node, info *arg_info);
extern node *TUPid (node *arg_node, info *arg_info);
extern node *TUPexprs (node *arg_node, info *arg_info);

#endif /*_SAC_TYPEUPGRADE_H_ */
