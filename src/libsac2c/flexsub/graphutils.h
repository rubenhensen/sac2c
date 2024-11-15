/*****************************************************************************
 *
 * file:   graphutils.h
 *
 * description:
 *   header file for graphutils.c
 *
 *****************************************************************************/

#ifndef _GRAPHUTILS_H_
#define _GRAPHUTILS_H_

#include <string.h>
#include "types.h"

extern bool GUvertInList (node *n, nodelist *nl);
extern nodelist *GUmergeLists (nodelist *nla, nodelist *nlb);
extern void GUremoveEdge (node *src, node *tar);

#endif /* _GRAPHUTILS_H_ */
