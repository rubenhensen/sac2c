/*
 *
 * $Log$
 * Revision 3.2  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.1  2000/11/20 18:02:35  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:44:20  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sync_init.h
 *
 * prefix: SPMDL
 *
 * description:
 *
 *   header file for sync_init.c
 *
 *****************************************************************************/

#ifndef SYNC_INIT_H

#define SYNC_INIT_H

#include "types.h"

extern node *SYNCIassign (node *arg_node, info *arg_info);

#endif /* SYNC_INIT_H */
