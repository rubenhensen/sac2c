/*
 *
 * $Log$
 * Revision 1.2  2000/02/03 17:30:06  dkr
 * LUTs added
 *
 * Revision 1.1  2000/01/21 16:52:09  dkr
 * Initial revision
 *
 */

#ifndef _sac_DataFlowMaskUtils_h
#define _sac_DataFlowMaskUtils_h

extern types *DFM2Types (DFMmask_t mask);
extern node *DFM2Vardecs (DFMmask_t mask, lut_t *lut);
extern node *DFM2Args (DFMmask_t mask, lut_t *lut);
extern node *DFM2Exprs (DFMmask_t mask, lut_t *lut);
extern ids *DFM2Ids (DFMmask_t mask, lut_t *lut);

#endif /* _sac_DataFlowMaskUtils_h */
