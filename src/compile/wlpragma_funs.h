/*
 *
 * $Log$
 * Revision 1.4  1998/05/25 13:15:42  dkr
 * signature of wlpragma-funs changed:
 *   lineno added for error-messages
 *
 * Revision 1.3  1998/04/13 19:59:06  dkr
 * added ConstSegs
 *
 * Revision 1.2  1998/04/13 18:11:13  dkr
 * rcs-header added
 *
 *
 *
 */

#ifndef _sac_wlpragma_funs_h

#define _sac_wlpragma_funs_h

extern node *All (node *segs, node *parms, node *cubes, int dims, int line);
extern node *Cubes (node *segs, node *parms, node *cubes, int dims, int line);
extern node *ConstSegs (node *segs, node *parms, node *cubes, int dims, int line);
extern node *NoBlocking (node *segs, node *parms, node *cubes, int dims, int line);
extern node *Bv (node *segs, node *parms, node *cubes, int dims, int line);
extern node *BvL0 (node *segs, node *parms, node *cubes, int dims, int line);
extern node *BvL1 (node *segs, node *parms, node *cubes, int dims, int line);
extern node *BvL2 (node *segs, node *parms, node *cubes, int dims, int line);
extern node *Ubv (node *segs, node *parms, node *cubes, int dims, int line);

#endif /* _sac_wlpragma_funs_h */
