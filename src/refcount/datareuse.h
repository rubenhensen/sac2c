/*
 *
 * $Log$
 * Revision 1.4  2004/12/01 16:36:22  ktr
 * post DK bugfix
 *
 * ,
 *
 * Revision 1.3  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.2  2004/11/09 19:39:37  ktr
 * ongoing implementation
 *
 * Revision 1.1  2004/11/02 14:27:11  ktr
 * Initial revision
 *
 */
#ifndef _SAC_DATAREUSE_H_
#define _SAC_DATAREUSE_H_

#include "types.h"

/******************************************************************************
 *
 * Data reuse traversal
 *
 * Prefix: EMDR
 *
 *****************************************************************************/
extern node *EMDRdoDataReuse (node *syntax_tree);

extern node *EMDRap (node *arg_node, info *arg_info);
extern node *EMDRassign (node *arg_node, info *arg_info);
extern node *EMDRcode (node *arg_node, info *arg_info);
extern node *EMDRcond (node *arg_node, info *arg_info);
extern node *EMDRfundef (node *arg_node, info *arg_info);
extern node *EMDRlet (node *arg_node, info *arg_info);
extern node *EMDRprf (node *arg_node, info *arg_info);

#endif /* _SAC_DATAREUSE_H_ */
