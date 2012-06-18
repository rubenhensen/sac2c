/*****************************************************************************
 *
 * file:   query.h
 *
 * description:
 *   header file for query.c
 *
 *****************************************************************************/

#ifndef _QUERY_H_
#define _QUERY_H_

#include "types.h"

node *GINlcaFromNodes (node *n1, node *n2, compinfo *ci);
int GINisReachable (node *n1, node *n2, compinfo *ci);

#endif /* _QUERY_H_ */
