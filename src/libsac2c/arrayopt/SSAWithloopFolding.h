#ifndef _SAC_WITHLOOPFOLDING_H_
#define _SAC_WITHLOOPFOLDING_H_

#include "types.h"

/******************************************************************************
 *
 * exported functions
 *
 ******************************************************************************/

/* general functions */
extern int WLFlocateIndexVar (node *idn, node *wln);

extern void WLFarrayST2ArrayInt (node *arrayn, int **iarray, int shape);

/* index_info related functions */
extern index_info *WLFcreateIndex (int vector);
extern index_info *WLFduplicateIndexInfo (index_info *iinfo);
extern index_info *WLFvalidLocalId (node *idn);
extern void WLFdbugIndexInfo (index_info *iinfo);

/* intern_gen related functions */
extern intern_gen *WLFtree2InternGen (node *wln, node *filter);
extern node *WLFinternGen2Tree (node *wln, intern_gen *ig);
extern int WLFnormalizeInternGen (intern_gen *ig);
extern intern_gen *WLFcreateInternGen (int shape, int stepwidth);
extern intern_gen *WLFappendInternGen (intern_gen *, int, node *, int);
extern intern_gen *WLFcopyInternGen (intern_gen *source);
extern intern_gen *WLFfreeInternGen (intern_gen *tmp);
extern intern_gen *WLFfreeInternGenChain (intern_gen *ig);
extern void WLFprintfInternGen (FILE *file, intern_gen *ig,
                                bool with_code, bool whole_chain);

/******************************************************************************
 *
 * defines
 *
 ******************************************************************************/

/* if not defined, indexes with more than one occurence of an
   index scalar are allowed to be valid transformations, e.g. [i,i,j] */
/* #define TRANSF_TRUE_PERMUTATIONS */

#endif /* _SAC_WITHLOOPFOLDING_H_ */
