#ifndef _SAC_IDAG_H_
#define _SAC_IDAG_H_

#include "types.h"
typedef node idag;
typedef node ivertex_fam;
typedef node iedge_fam;
typedef vertex ivertex;

extern idag *IDAGgenInfiniteGraph ();

extern ivertex *IDAGaddVertex (idag *g, void *annotation);

extern void *IDAGgetVertexAnnotation (idag *g, ivertex *from);

extern ivertex_fam *IDAGaddVertexFamily (idag *g, idag_fun_t cmpfun);

extern ivertex *IDAGregisterVertexFamilyMember (idag *g, ivertex_fam *vfam, void *param);

extern void IDAGaddEdge (idag *g, ivertex *from, ivertex *to);

extern void IDAGaddEdgeFamily (idag *g, ivertex_fam *from, ivertex_fam *to,
                               idag_fun_t checkfun);

extern ivertex *IDAGgetLub (idag *g, ivertex *from, ivertex *to);

#endif /* _SAC_IDAG_H_ */
