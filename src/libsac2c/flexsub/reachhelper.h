#ifndef _REACHHELPER_H_
#define _REACHHELPER_H_

extern void buildTransitiveLinkTable (dynarray *arrayd);

extern void setSrcTarArrays (dynarray *arrayd, dynarray **arrX, dynarray **arrY);

extern matrix *computeTLCMatrix (dynarray *arrayd, dynarray *arrX, dynarray *arrY);

#endif
