/*
 *
 * $Log$
 * Revision 1.2  2000/03/29 16:08:51  jhs
 * Duplication of lifted function added.
 *
 * Revision 1.1  2000/03/29 09:46:37  jhs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   blocks_lift.h
 *
 * description:
 *   header file for blocks_lift.c
 *
 *****************************************************************************/

#ifndef BLOCKS_LIFT_H

#define BLOCKS_LIFT_H

extern node *BlocksLift (node *arg_node, node *arg_info);

extern node *BLKLIfundef (node *arg_node, node *arg_info);
extern node *BLKLImt (node *arg_node, node *arg_info);

#endif /* BLOCKS_CONS_H */
