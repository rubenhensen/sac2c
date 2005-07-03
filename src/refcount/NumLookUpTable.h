/*
 *
 * $Log$
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
extern lut_t *NLUTgenerateNlut (node *args, node *vardecs);
extern lut_t *NLUTduplicateNlut (lut_t *nlut);
extern lut_t *NLUTremoveNlut (lut_t *nlut);

extern void NLUTsetNum (lut_t *nlut, node *avis, int num);
extern int NLUTgetNum (lut_t *nlut, node *avis);

#endif /* _SAC_NUMLOOKUPTABLE_H_ */
