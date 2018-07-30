#ifndef _SAC_WLPRAGMA_FUNS_H_
#define _SAC_WLPRAGMA_FUNS_H_

#include "types.h"

extern bool ExtractNaiveCompPragma (node *pragma, size_t line);

extern node *ExtractAplPragma (node *pragma, size_t line);

#define WLP(fun, str, ieee)                                                              \
    extern node *fun (node *segs, node *parms, node *cubes, int dims, size_t line);
#include "wlpragma_funs.mac"
#undef WLP

#endif /* _SAC_WLPRAGMA_FUNS_H_ */
