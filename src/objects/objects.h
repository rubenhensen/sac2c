/*
 *
 * $Log$
 * Revision 3.3  2004/11/22 17:53:27  ktr
 * SacDevCamp 2004 Get Ready for Rumble!
 *
 * Revision 3.2  2004/07/17 14:30:09  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 3.1  2000/11/20 18:02:00  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:43:22  sacbase
 * new release made
 *
 * Revision 1.2  1995/11/01 16:35:12  cg
 * added external declarations of functions OBJap, OBJid and OBJlet.
 *
 * Revision 1.1  1995/10/31  17:22:05  cg
 * Initial revision
 *
 * Revision 1.1  1995/10/31  17:22:05  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_OBJECTS_H_
#define _SAC_OBJECTS_H_

#include "types.h"

/******************************************************************************
 *
 * Objects traversal ( obj_tab)
 *
 * Prefix: OBJ
 *
 *****************************************************************************/
extern node *OBJdoHandleObjects (node *syntax_tree);

extern node *OBJmodule (node *arg_node, info *arg_info);
extern node *OBJfundef (node *arg_node, info *arg_info);
extern node *OBJobjdef (node *arg_node, info *arg_info);
extern node *OBJarg (node *arg_node, info *arg_info);
extern node *OBJap (node *arg_node, info *arg_info);
extern node *OBJid (node *arg_node, info *arg_info);
extern node *OBJlet (node *arg_node, info *arg_info);

#endif /* _SAC_OBJECTS_H_ */
