/*
 *
 * $Log$
 * Revision 1.1  2004/09/23 20:44:53  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _DESERIALIZE_H
#define _DESERIALIZE_H

#include "types.h"

extern node *CombineFunctionHeadAndBody (node *fundef, node *body);

extern node *DSReturn (node *arg_node, info *arg_info);
extern node *DSBlock (node *arg_node, info *arg_info);
extern node *DSArg (node *arg_node, info *arg_info);
extern node *DSVardec (node *arg_node, info *arg_info);

#endif /* _DESERIALIZE_H */
