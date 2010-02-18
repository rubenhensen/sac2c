/* $Id$ */

#ifndef _SAC_TYPECONV_PRECOMPILE_H_
#define _SAC_TYPECONV_PRECOMPILE_H_

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
extern node *TCPrange (node *arg_node, info *arg_info);

#endif /* _SAC_TYPECONV_PRECOMPILE_H_ */
