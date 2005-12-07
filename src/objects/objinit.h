/*
 * $Id$
 */

#ifndef _SAC_OBJINIT_H_
#define _SAC_OBJINIT_H_

#include "types.h"

/******************************************************************************
 *
 * Object initialization traversal ( objinit_tab)
 *
 * Prefix: OI
 *
 *****************************************************************************/
extern node *OIdoObjectInit (node *syntax_tree);

extern node *OImodule (node *arg_node, info *arg_info);
extern node *OIobjdef (node *arg_node, info *arg_info);

#endif /* _SAC_OBJINIT_H_  */
