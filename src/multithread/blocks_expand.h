/*
 *
 * $Log$
 * Revision 1.2  2000/02/21 17:52:07  jhs
 * Expansion on N_mt's finished.
 *
 * Revision 1.1  2000/02/21 11:02:13  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:  blocks_expand.h
 *
 * description:
 *   header file for blocks_expand.c
 *
 ******************************************************************************/

#ifndef BLOCKS_EXPAND_H

#define BLOCKS_EXPAND_H

extern node *BlocksExpand (node *arg_node, node *arg_info);

node *BLKEXassign (node *arg_node, node *arg_info);

#endif /* BLOCKS_EXPAND_H */
