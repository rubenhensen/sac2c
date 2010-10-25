/*****************************************************************************
 *
 * file:   lubtree.h
 *
 * description:
 *   header file for lubtree.c
 *
 *****************************************************************************/

#ifndef _LUBTREE_H_
#define _LUBTREE_H_

#include "types.h"

extern lubinfo *LUBcreatePartitions (dynarray *eulertour);
extern int LUBgetLowestFromCandidates (dynarray *d, int indices[4]);
extern node *LUBcomputeLCAinSPTree (node *n1, node *n2, compinfo *ci);

#endif /* _LUBTREE_H_ */
