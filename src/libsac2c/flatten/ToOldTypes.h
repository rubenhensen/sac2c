/*
 * $Log$
 * Revision 1.6  2004/12/19 15:33:32  sbs
 * phase brushed
 *
 * Revision 1.5  2004/12/08 17:59:15  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.4  2004/11/24 17:18:19  mwe
 * interface changes
 *
 * Revision 1.3  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 1.2  2004/11/21 20:10:20  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.1  2004/11/19 10:50:57  mwe
 * Initial revision
 *
 */

#ifndef _SAC_TOOLDTYPES_H_
#define _SAC_TOOLDTYPES_H_

#include "types.h"

/*****************************************************************************
 *
 * ToOldTypes traversal ( tot_tab)
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
extern node *TOTdoToOldTypes (node *syntax_tree);

extern node *TOTfundef (node *arg_node, info *arg_info);
extern node *TOTarg (node *arg_node, info *arg_info);
extern node *TOTblock (node *arg_node, info *arg_info);
extern node *TOTvardec (node *arg_node, info *arg_info);
extern node *TOTret (node *arg_node, info *arg_info);

#endif /* _SAC_TOOLDTYPES_H_ */
