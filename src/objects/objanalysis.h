/*
 *
 * $Log$
 * Revision 1.3  2004/11/26 21:06:48  jhb
 * compile
 *
 * Revision 1.2  2004/11/22 17:53:27  ktr
 * SacDevCamp 2004 Get Ready for Rumble!
 *
 * Revision 1.1  2004/11/20 17:19:42  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_OBJANALYSIS_H_
#define _SAC_OBJANALYSIS_H_

#include "types.h"

/******************************************************************************
 *
 * Object Analysis traversal ( oan_tab)
 *
 * Prefix: OAN
 *
 *****************************************************************************/
extern node *OANdoObjectAnalysis (node *syntax_tree);

extern node *OANmodule (node *arg_node, info *arg_info);
extern node *OANglobobj (node *arg_node, info *arg_info);
extern node *OANap (node *arg_node, info *arg_info);
extern node *OANfundef (node *arg_node, info *arg_info);

#endif /* _SAC_OBJANALYSIS_H_ */
