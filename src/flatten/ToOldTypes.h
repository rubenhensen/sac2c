/*
 * $Log$
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

#ifndef SAC_TONEWTYPES_H
#define SAC_TONEWTYPES_H

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
node *ToOldTypes (node *syntax_tree);

#endif
