/*
 *
 * $Log$
 * Revision 3.3  2004/11/22 11:00:05  ktr
 * Ismop 2004 SacDevCamp 04
 *
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

#ifndef _SAC_MAP_CWRAPPER_H_
#define _SAC_MAP_CWRAPPER_H_

#include "types.h"

/******************************************************************************
 *
 * Map CWrappers traversal ( mapcw_tab)
 *
 * Prefix: MCW
 *
 *****************************************************************************/
extern node *MCWdoMapCWrapper (node *syntax_tree);

extern node *MCWarg (node *arg_node, info *arg_info);
extern node *MCWcwrapper (node *arg_node, info *arg_info);
extern node *MCWfundef (node *arg_node, info *arg_info);
extern node *MCWmodule (node *arg_node, info *arg_info);

#endif /* _SAC_MAP_CWRAPPER_H_ */
