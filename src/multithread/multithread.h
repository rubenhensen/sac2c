/*
 *
 * $Log$
 * Revision 3.2  2004/06/08 14:40:56  skt
 * definition of execution modes added
 *
 * Revision 3.1  2000/11/20 18:03:11  sacbase
 * new release made
 *
 * Revision 1.2  2000/01/21 14:28:09  jhs
 * Added MUTHmodul and MUTHfundef.
 *
 * Revision 1.1  2000/01/21 13:11:16  jhs
 * Initial revision
 *
 *
 */

/** <!--********************************************************************-->
 *
 * @file multithread.h
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

/* definition of the execution modes */
#define MUTH_ANY 0
#define MUTH_SINGLE 1
#define MUTH_ONCE 2
#define MUTH_MULTI 3

extern node *BuildMultiThread (node *syntax_tree);

extern node *MUTHmodul (node *arg_node, node *arg_info);
extern node *MUTHfundef (node *arg_node, node *arg_info);

#endif /* MULTITHREAD_H */
