#ifndef _SAC_WLPRAGMA_FUNS_H_
#define _SAC_WLPRAGMA_FUNS_H_

#include "types.h"

extern bool ExtractNaiveCompPragma (node *pragma, int line);

extern node *ExtractAplPragma (node *pragma, int line);

#define WLP(fun, str, ieee)                                                              \
    extern node *fun (node *segs, node *parms, node *cubes, int dims, int line);
#include "wlpragma_funs.mac"
#undef WLP

#endif /* _SAC_WLPRAGMA_FUNS_H_ */
