/*
 * $Id$
 */

#ifndef _SAC_DAG_H_
#define _SAC_DAG_H_

#include "types.h"

extern dag *DAGgenGraph ();

extern vertex *DAGaddVertex (dag *g, void *annotation);
extern void *DAGgetVertexAnnotation (dag *g, vertex *from);

extern edge *DAGaddEdge (dag *g, vertex *from, vertex *to);
extern void *DAGgetEdgeAnnotation (edge *e);

extern vertex *DAGgetLub (dag *g, vertex *from, vertex *to);

#endif /* _SAC_DAG_H_ */
