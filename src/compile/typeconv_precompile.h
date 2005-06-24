/*
 *
 * $Log$
 * Revision 1.2  2005/06/24 16:09:23  sah
 * added insertion of type conversion assignments
 * for conditionals
 *
 * Revision 1.1  2004/11/27 02:35:57  ktr
 * Initial revision
 *
 */

#ifndef _SAC_TYPE_CONVERSIONS_H_
#define _SAC_TYPE_CONVERSIONS_H_

#include "types.h"

/******************************************************************************
 *
 * TYPE CONVERSIONS PRECOMPILATIONS
 *
 * Prefix: TCP
 *
 *****************************************************************************/
extern node *TCPdoTypeConversions (node *syntax_tree);

extern node *TCPmodule (node *arg_node, info *arg_info);
extern node *TCPfundef (node *arg_node, info *arg_info);
extern node *TCPassign (node *arg_node, info *arg_info);
extern node *TCPap (node *arg_node, info *arg_info);
extern node *TCPcond (node *arg_node, info *arg_info);
extern node *TCPdo (node *arg_node, info *arg_info);

#endif /* _SAC_TYPE_CONVERSIONS_H_ */
