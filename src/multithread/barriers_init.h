/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:03:01  sacbase
 * new release made
 *
 * Revision 1.2  2000/04/10 15:45:35  jhs
 * Added BARINmt and BARINassign to traversal.
 *
 * Revision 1.1  2000/03/22 17:29:47  jhs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   barriers_init.h
 *
 * description:
 *   header file for barriers_init.c
 *
 *****************************************************************************/

#ifndef BARRIERS_INIT_H

#define BARRIERS_INIT_H

extern node *BarriersInit (node *arg_node, node *arg_info);

extern node *BARINassign (node *arg_node, node *arg_info);
extern node *BARINfundef (node *arg_node, node *arg_info);
extern node *BARINmt (node *arg_node, node *arg_info);

#endif /* BARRIERS_INIT_H */
