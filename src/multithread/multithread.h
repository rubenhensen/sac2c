/*
 *
 * $Log$
 * Revision 1.2  2000/01/21 14:28:09  jhs
 * Added MUTHmodul and MUTHfundef.
 *
 * Revision 1.1  2000/01/21 13:11:16  jhs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:        multithread.h
 *
 * prefix:      MUTH
 *
 * description:
 *
 *   header file for multithread.c
 *
 *****************************************************************************/

#ifndef MULTITHREAD_H

#define MULTITHREAD_H

extern node *BuildMultiThread (node *syntax_tree);

extern node *MUTHmodul (node *arg_node, node *arg_info);
extern node *MUTHfundef (node *arg_node, node *arg_info);

#endif /* MULTITHREAD_H */
