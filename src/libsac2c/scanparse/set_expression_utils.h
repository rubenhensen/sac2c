#ifndef _SET_EXPRESSSION_UTILS_H_
#define _SET_EXPRESSSION_UTILS_H_

#include "types.h"

extern idtable *SEUTbuildIdTable (node *vec, idtable* oldscope);
extern idtable *SEUTfreeIdTable (idtable *identry);
extern idtable *SEUTfreeIdTableChain (idtable *identry);

extern bool SEUTisVector (idtable *identry);
extern bool SEUTisScalar (idtable *identry);

extern idtable *SEUTsearchIdInTable (char *id, idtable *from, idtable *to);
extern bool SEUTcontainsIdFromTable (node *expr, idtable *from, idtable *to);
extern int SEUTcountIds (idtable *table);
extern bool SEUTshapeInfoComplete (idtable *table);

extern void SEUTscanSelectionForShapeInfo (node *idxvec, node *arg, idtable *scope);
extern node *SEUTgenShape (idtable *identry);

extern node *SEUTsubstituteIdxs (node *expr, node *idxs, node *subst);
extern bool SEUTcontainsIdxs (node *expr, node *idxs);

extern node *SEUTbuildSimpleWl (node *shape, node *def);

#endif /* _SET_EXPRESSSION_UTILS_H_ */
