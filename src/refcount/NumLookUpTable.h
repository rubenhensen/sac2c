/*
 *
 * $Log$
 * Revision 1.2  2005/07/16 09:57:55  ktr
 * enhanced functionality
 *
 * Revision 1.1  2005/07/03 16:57:49  ktr
 * Initial revision
 *
 */
#ifndef _SAC_NUMLOOKUPTABLE_H_
#define _SAC_NUMLOOKUPTABLE_H_

#include "types.h"

/******************************************************************************
 *
 * Look up table which associates integers to variables
 *
 * Prefix: NLUT
 *
 *****************************************************************************/
extern nlut_t *NLUTgenerateNlut (node *args, node *vardecs);
extern nlut_t *NLUTduplicateNlut (nlut_t *nlut);
extern nlut_t *NLUTremoveNlut (nlut_t *nlut);

extern int NLUTgetNum (nlut_t *nlut, node *avis);
extern void NLUTsetNum (nlut_t *nlut, node *avis, int num);
extern void NLUTincNum (nlut_t *nlut, node *avis, int num);

extern dfmask_t *NLUTgetNonZeroMask (nlut_t *nlut);

#endif /* _SAC_NUMLOOKUPTABLE_H_ */
