/*
 *
 * $Log$
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

#endif /*_SAC_TYPEUPGRADE_H_ */
