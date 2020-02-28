#ifndef _SAC_MEM_EMR_UTIL_H_
#define _SAC_MEM_EMR_UTIL_H_

#include "types.h"

extern bool ShapeMatch (ntype *t1, ntype *t2);
extern bool doAvisMatch (node *exprs, node *id);
extern node *filterDuplicateId (node *fexprs, node **exprs);
extern node *ElimDupes (node *exprs);
extern node *ElimDupesOfAvis (node *avis, node *exprs);
extern node *isSameShapeAvis (node *avis, node *exprs);

#endif /* _SAC_MEM_EMR_UTIL_H_ */
