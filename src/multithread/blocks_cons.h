/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:03:02  sacbase
 * new release made
 *
 * Revision 1.1  2000/03/02 12:54:49  jhs
 * Initial revision
 *
 *
 * constructed by jhs@dArtagnan at home
 */

/*****************************************************************************
 *
 * file:   blocks_cons.h
 *
 * description:
 *   header file for blocks_cons.c
 *
 *****************************************************************************/

#ifndef BLOCKS_CONS_H

#define BLOCKS_CONS_H

extern node *BlocksCons (node *arg_node, node *arg_info);

extern node *BLKCOxt (node *arg_node, node *arg_info);
extern node *BLKCOfundef (node *arg_node, node *arg_info);
extern node *BLKCOassign (node *arg_node, node *arg_info);

#endif /* BLOCKS_CONS_H */
