#ifndef _LUBCROSS_H_
#define _LUBCROSS_H_

matrix *LUBcreateReachMat (compinfo *ci);

matrix *LUBcreatePCPTMat (matrix *reachmat, compinfo *ci);

dynarray *LUBsortInPostorder (compinfo *ci);

void LUBorColumnsAndUpdate (matrix *m1, int colidx1, matrix *m2, int colidx2,
                            matrix *result, int rescolidx);

int LUBisNodeCsrc (node *n, dynarray *csrc);

dynarray *LUBrearrangeCsrcOnTopo (dynarray *csrc, dynarray *prearr);

dynarray *LUBrearrangeNoncsrcOnTopo (dynarray *noncsrc);

matrix *LUBrearrangeMatOnTopo (dynarray *topoarr, matrix *mat);

matrix *LUBcreatePCPCMat (matrix *reachmat, dynarray *postarr, compinfo *ci);

void LUBincorporateCrossEdges (compinfo *ci);

#endif
