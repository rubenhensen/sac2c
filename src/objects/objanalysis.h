/*
 *
 * $Log$
 * Revision 1.1  2004/11/20 17:19:42  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _OBJANALYSIS_H
#define _OBJANALYSIS_H

#include "types.h"

extern void ObjectAnalysis (node *syntax_tree);

extern node *OANModul (node *arg_node, info *arg_info);
extern node *OANId (node *arg_node, info *arg_info);
extern node *OANAp (node *arg_node, info *arg_info);
extern node *OANFundef (node *arg_node, info *arg_info);

#endif
