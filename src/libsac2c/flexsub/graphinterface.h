/*****************************************************************************
 *
 * file:   graphinterface.h
 *
 * description:
 *   header file for graphinterface.c
 *
 *****************************************************************************/

#ifndef _GRAPHINTERFACE_H_
#define _GRAPHINTERFACE_H_

#include "types.h"

node *GINlcaFromNodes (node *n1, node *n2, compinfo *ci);
int GINisReachable (node *n1, node *n2, compinfo *ci);

#endif /* _GRAPHINTERFACE_H_ */
