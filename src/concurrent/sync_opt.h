/*
 *
 * $Log$
 * Revision 3.4  2004/11/21 23:01:01  ktr
 * ISMOP 2004!!!!!!!
 *
 * Revision 3.3  2004/11/21 17:54:54  skt
 * moved functions from concurrent_lib into sync_opt to remove concurrent_lib
 *
 * Revision 3.2  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.1  2000/11/20 18:02:36  sacbase
 * new release made
 *
 * Revision 2.2  1999/07/07 15:54:54  jhs
 * Added SYNC[sync|assign].
 *
 * Revision 2.1  1999/02/23 12:44:22  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */
#ifndef _SAC_SYNC_OPT_H_
#define _SAC_SYNC_OPT_H_

#include "types.h"

/******************************************************************************
 *
 * SYNC optimization traversal ( synco_tab)
 *
 * Prefix: SYNCO
 *
 *****************************************************************************/
extern node *SYNCOassign (node *arg_node, info *arg_info);
extern node *SYNCOsync (node *arg_node, info *arg_info);

#endif /* _SAC_SYNC_OPT_H_ */
