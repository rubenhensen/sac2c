/*
 *
 * $Log$
 * Revision 1.4  2000/03/09 18:33:21  jhs
 * Brushing ...
 *
 * Revision 1.3  2000/02/02 17:22:47  jhs
 * BlocksInit improved.
 *
 * Revision 1.2  2000/02/02 12:27:13  jhs
 * Added INFO_MUTH_FUNDEF, improved BLKIN.
 *
 * Revision 1.1  2000/01/28 13:21:35  jhs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   blocks_init.h
 *
 * description:
 *   header file for blocks_init.c
 *
 *****************************************************************************/

#ifndef BLOCKS_INIT_H

#define BLOCKS_INIT_H

extern node *BlocksInit (node *arg_node, node *arg_info);

extern node *BLKINassign (node *arg_node, node *arg_info);

/* this should be moved to multithread_lib.[ch] #### */
extern node *InsertST (node *assign, node *arg_info);
extern node *InsertMT (node *assign, node *arg_info);

#endif /* BLOCKS_INIT_H */
