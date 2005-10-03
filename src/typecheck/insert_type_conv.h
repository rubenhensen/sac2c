/*
 *
 * $Log$
 * Revision 1.3  2005/10/03 14:38:33  sbs
 * added INSTCwith
 *
 * Revision 1.2  2005/08/19 17:26:54  sbs
 * added avis and block
 *
 * Revision 1.1  2005/08/18 07:01:55  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_INSERT_TYPE_CONV_H_
#define _SAC_INSERT_TYPE_CONV_H_

#include "types.h"

/******************************************************************************
 *
 * Insert Vardec traversal ( insta_tab)
 *
 * Prefix: INSTC
 *
 *****************************************************************************/

extern node *INSTCdoInsertTypeConv (node *syntaxtree);

extern node *INSTCfundef (node *arg_node, info *arg_info);
extern node *INSTCblock (node *arg_node, info *arg_info);
extern node *INSTCavis (node *arg_node, info *arg_info);
extern node *INSTCassign (node *arg_node, info *arg_info);
extern node *INSTCids (node *arg_node, info *arg_info);
extern node *INSTCid (node *arg_node, info *arg_info);
extern node *INSTCwith (node *arg_node, info *arg_info);
extern node *INSTCreturn (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_TYPE_CONV_H_ */
