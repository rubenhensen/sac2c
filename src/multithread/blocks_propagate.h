/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:03:07  sacbase
 * new release made
 *
 * Revision 1.2  2000/03/21 13:07:26  jhs
 * Added BLKPPassign, BLKPPfundef.
 *
 * Revision 1.1  2000/03/09 19:50:36  jhs
 * Initial revision
 *
 *
 *
 * constructed by jhs@dArtagnan at home
 */

/*****************************************************************************
 *
 * file:   blocks_propagate.h
 *
 * description:
 *   header file for blocks_propagate.c
 *
 *****************************************************************************/

#ifndef BLOCKS_PROPAGATE_H

#define BLOCKS_PROPAGATE_H

extern node *BlocksPropagate (node *arg_node, node *arg_info);

extern node *BLKPPassign (node *arg_node, node *arg_info);
extern node *BLKPPfundef (node *arg_node, node *arg_info);

#endif /* BLOCKS_PROPAGATE_H */
