/*
 *
 * $Log$
 * Revision 1.2  2005/09/01 12:20:15  sah
 * added AFBap: when adding a functions body, the bodies of all LAC funs
 *              are added now as well
 *
 * Revision 1.1  2005/07/22 15:09:51  sah
 * Initial revision
 *
 *
 */

#ifndef _SAC_ADD_FUNCTION_BODY_H_
#define _SAC_ADD_FUNCTION_BODY_H_

#include "types.h"

extern node *AFBdoAddFunctionBody (node *fundef);

extern node *AFBfundef (node *arg_node, info *arg_info);
extern node *AFBreturn (node *arg_node, info *arg_info);
extern node *AFBap (node *arg_node, info *arg_info);
extern node *AFBblock (node *arg_node, info *arg_info);
extern node *AFBarg (node *arg_node, info *arg_info);

#endif
