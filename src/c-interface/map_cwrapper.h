/*
 * $Log$
 * Revision 1.2  2000/07/12 10:06:40  nmw
 * RCS-Header added
 *
 *
 *
 *
 */

#ifndef _sac_map_cwrapper_h
#define _sac_map_cwrapper_h

#include "tree.h"
#include "globals.h"

extern node *MCWcwrapper (node *arg_node, node *arg_info);
extern node *MCWmodul (node *arg_node, node *arg_info);
extern node *MCWfundef (node *arg_node, node *arg_info);
extern node *MCWarg (node *arg_node, node *arg_info);
extern node *MapCWrapper (node *syntax_tree);

#endif /* _sac_map_cwrapper_h */
