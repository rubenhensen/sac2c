/*
 *
 * $Log$
 * Revision 1.6  2000/03/23 17:37:06  dkr
 * DFM2Types() renamed to DFM2ReturnTypes()
 *
 * Revision 1.5  2000/03/17 20:57:25  dkr
 * IsEmptyDFMstack() added
 *
 * Revision 1.4  2000/03/17 20:44:11  dkr
 * stuff for DFMstack added
 *
 * Revision 1.3  2000/03/09 18:35:59  jhs
 *  new copy routine
 *
 * Revision 1.2  2000/02/03 17:30:06  dkr
 * LUTs added
 *
 * Revision 1.1  2000/01/21 16:52:09  dkr
 * Initial revision
 *
 */

#ifndef _sac_DataFlowMaskUtils_h
#define _sac_DataFlowMaskUtils_h

#include "LookUpTable.h"

typedef void *DFMstack_t;
typedef int (*fun_t) (DFMmask_t mask, char *id, node *decl);

extern DFMstack_t GenerateDFMstack (void);
extern int IsEmptyDFMstack (DFMstack_t stack);
extern void RemoveDFMstack (DFMstack_t *stack);

extern void PushDFMstack (DFMstack_t *stack, DFMmask_t mask);
extern DFMmask_t PopDFMstack (DFMstack_t *stack);

extern void ForeachDFMstack (DFMstack_t stack, fun_t fun, char *id, node *decl);
extern void WhileDFMstack (DFMstack_t stack, fun_t fun, char *id, node *decl);

extern types *DFM2ReturnTypes (DFMmask_t mask);
extern node *DFM2Vardecs (DFMmask_t mask, LUT_t lut);
extern node *DFM2Args (DFMmask_t mask, LUT_t lut);
extern node *DFM2Exprs (DFMmask_t mask, LUT_t lut);
extern ids *DFM2Ids (DFMmask_t mask, LUT_t lut);

extern DFMmask_t DFMDuplicateMask (DFMmask_t mask, DFMmask_base_t base);

#endif /* _sac_DataFlowMaskUtils_h */
