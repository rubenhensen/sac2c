/*
 *
 * $Log$
 * Revision 3.2  2001/03/22 18:05:24  dkr
 * include of tree.h eliminated
 *
 * Revision 3.1  2000/11/20 18:03:41  sacbase
 * new release made
 *
 * Revision 1.2  2000/07/12 10:06:40  nmw
 * RCS-Header added
 *
 */

#ifndef _sac_map_cwrapper_h
#define _sac_map_cwrapper_h

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *MCWcwrapper (node *arg_node, node *arg_info);
extern node *MCWmodul (node *arg_node, node *arg_info);
extern node *MCWfundef (node *arg_node, node *arg_info);
extern node *MCWarg (node *arg_node, node *arg_info);
extern node *MapCWrapper (node *syntax_tree);

#endif /* _sac_map_cwrapper_h */
