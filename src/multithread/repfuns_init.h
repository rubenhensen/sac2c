/*
 *
 * $Log$
 * Revision 1.2  2000/02/04 14:44:43  jhs
 * Added infrastructure.
 *
 * Revision 1.1  2000/02/04 13:48:36  jhs
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

#ifndef REPFUNS_INIT_H

#define REPFUNS_INIT_H

extern node *RepfunsInit (node *arg_node, node *arg_info);

extern node *RFINnwith2 (node *arg_node, node *arg_info);
extern node *RFINlet (node *arg_node, node *arg_info);

#endif /* REPFUNS_INIT_H */
