/*
 *
 * $Log$
 * Revision 1.2  1998/04/13 18:11:13  dkr
 * rcs-header added
 *
 *
 *
 */

#ifndef _sac_wlpragma_funs_h

#define _sac_wlpragma_funs_h

extern node *All (node *segs, node *parms, node *cubes, int dims);
extern node *Cubes (node *segs, node *parms, node *cubes, int dims);
extern node *NoBlocking (node *segs, node *parms, node *cubes, int dims);
extern node *Bv (node *segs, node *parms, node *cubes, int dims);
extern node *BvL0 (node *segs, node *parms, node *cubes, int dims);
extern node *BvL1 (node *segs, node *parms, node *cubes, int dims);
extern node *BvL2 (node *segs, node *parms, node *cubes, int dims);
extern node *Ubv (node *segs, node *parms, node *cubes, int dims);

#endif /* _sac_wlpragma_funs_h */
