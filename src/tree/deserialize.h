/*
 *
 * $Log$
 * Revision 1.2  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.1  2004/09/23 20:44:53  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _DESERIALIZE_H
#define _DESERIALIZE_H

#include "types.h"

extern node *AddFunctionBodyToHead (node *fundef, node *module);

extern node *DSReturn (node *arg_node, info *arg_info);
extern node *DSBlock (node *arg_node, info *arg_info);
extern node *DSArg (node *arg_node, info *arg_info);
extern node *DSVardec (node *arg_node, info *arg_info);

#endif /* _DESERIALIZE_H */
