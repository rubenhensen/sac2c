/*
 * $Log$
 * Revision 1.2  2004/11/21 20:10:20  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.1  2004/11/19 10:50:57  mwe
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   ToOldTypes.h
 *
 * prefix: TOT
 *
 * description:
 *
 *   This module restores all types-structures from ntype-structures.
 *   All ntype-structures will be removed.
 *
 *
 *****************************************************************************/

#include "types.h"

#ifndef _SAC_TONEWTYPES_H_
#define _SAC_TONEWTYPES_H_

node *TOTcast (node *arg_node, node *arg_info);
node *TOTarray (node *arg_node, node *arg_info);
node *TOTlet (node *arg_node, node *arg_info);
node *TOTassign (node *arg_node, node *arg_info);
node *TOTvardec (node *arg_node, node *arg_info);
node *TOTarg (node *arg_node, node *arg_info);
node *TOTblock (node *arg_node, node *arg_info);
node *TOTfundef (node *arg_node, node *arg_info);
node *TOTobjdef (node *arg_node, node *arg_info);
node *TOTtypedef (node *arg_node, node *arg_info);

node *TOTdoToOldTypes (node *syntax_tree);

#endif /* _SAC_TONEWTYPES_H_ */
