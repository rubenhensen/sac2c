/*
 *
 * $Log$
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

extern node *CONCmodul (node *arg_node, node *arg_info);
extern node *CONCfundef (node *arg_node, node *arg_info);

#endif /* CONCURRENT_H */
