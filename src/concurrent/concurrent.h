/*
 *
 * $Log$
 * Revision 3.2  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.1  2000/11/20 18:02:25  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:44:05  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   concurrent.h
 *
 * prefix: CONC
 *
 * description:
 *
 *   header file for concurrent.c
 *
 *
 *
 *
 *****************************************************************************/

#ifndef CONCURRENT_H

#define CONCURRENT_H

#include "types.h"

extern node *BuildSpmdRegions (node *syntax_tree);

extern node *CONCmodul (node *arg_node, info *arg_info);
extern node *CONCfundef (node *arg_node, info *arg_info);

#endif /* CONCURRENT_H */
