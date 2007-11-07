/*
 * $Id$
 */

#ifndef _SAC_OBJECT_INIT_H_
#define _SAC_OBJECT_INIT_H_

#include "types.h"

/******************************************************************************
 *
 * Object initialization traversal ( objinit_tab)
 *
 * Prefix: OI
 *
 *****************************************************************************/
extern node *OIobjdef (node *arg_node, info *arg_info);
extern node *OImodule (node *arg_node, info *arg_info);

extern node *OIdoObjectInit (node *syntax_tree);

#endif /* _SAC_OBJECT_INIT_H_  */
