/*
 *
 * $Log$
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

extern node *INSVDfundef (node *arg_node, info *arg_info);
extern node *INSVDid (node *arg_node, info *arg_info);
extern node *INSVDwithid (node *arg_node, info *arg_info);
extern node *INSVDlet (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_VARDEC_H_ */
