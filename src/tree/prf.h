/*
 *
 * $Log$
 * Revision 3.2  2002/09/09 17:40:59  dkr
 * categories removed (too error prone!)
 *
 * Revision 3.1  2000/11/20 18:03:27  sacbase
 * new release made
 *
 * Revision 1.3  2000/11/02 13:51:55  dkr
 * dito
 *
 * Revision 1.2  2000/10/16 13:55:41  dkr
 * order of array-prfs changed
 *
 * Revision 1.1  2000/10/12 15:46:27  dkr
 * Initial revision
 *
 */

#ifndef _sac_prf_h_
#define _sac_prf_h_

/* the bounds of the prf list */

#define FIRST_LEGAL_PRF F_toi

#define LAST_LEGAL_PRF F_rotate

/* all legal prfs */

#define LEGAL_PRF(prf) ((prf >= FIRST_LEGAL_PRF) && (prf <= LAST_LEGAL_PRF))

#endif
