/*
 *
 * $Log$
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

/*****************************************************************************
 *
 * file:   sync_opt.h
 *
 * prefix: SYNCO
 *
 * description:
 *
 *   header file for sync_opt.c
 *
 *****************************************************************************/

#ifndef SYNC_OPT_H

#define SYNC_OPT_H

#include "types.h"

extern node *SYNCOsync (node *arg_node, node *arg_info);
extern node *SYNCOassign (node *arg_node, node *arg_info);

#endif /* SYNC_OPT_H */
