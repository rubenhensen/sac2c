/*
 *
 * $Log$
 * Revision 1.8  2004/12/07 17:24:30  sah
 * added INSVDdo to fix dependency on ast
 * defined traversal order
 *
 * Revision 1.7  2004/12/05 20:12:26  sah
 * fixes
 *
 * Revision 1.6  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.5  2004/11/25 15:23:40  khf
 * SacDevCamp04
 *
 * Revision 1.4  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 1.3  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.2  2002/02/22 09:26:19  sbs
 * INSVDwithid added .
 *
 * Revision 1.1  2002/02/21 15:12:39  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_INSERT_VARDEC_H_
#define _SAC_INSERT_VARDEC_H_

#include "types.h"

/******************************************************************************
 *
 * Insert Vardec traversal ( insvd_tab)
 *
 * Prefix: INSVD
 *
 *****************************************************************************/
extern node *INSVDdoInsertVardec (node *syntaxtree);

extern node *INSVDmodule (node *arg_node, info *arg_info);
extern node *INSVDfundef (node *arg_node, info *arg_info);
extern node *INSVDspap (node *arg_node, info *arg_info);
extern node *INSVDspid (node *arg_node, info *arg_info);
extern node *INSVDspids (node *arg_node, info *arg_info);
extern node *INSVDwith (node *arg_node, info *arg_info);
extern node *INSVDpart (node *arg_node, info *arg_info);
extern node *INSVDcode (node *arg_node, info *arg_info);
extern node *INSVDlet (node *arg_node, info *arg_info);
extern node *INSVDdo (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_VARDEC_H_ */
