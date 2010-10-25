#ifndef _TFPRINTUTILS_H_
#define _TFPRINTUTILS_H_

extern void printMatrix (matrix *m);
extern void printMatrixInDotFormat (matrix *m);
extern void printTransitiveLinkTable (dynarray *arrayd);
extern void printDepthAndPre (dynarray *d);
extern void printLubInfo (lubinfo *linfo);

#endif
