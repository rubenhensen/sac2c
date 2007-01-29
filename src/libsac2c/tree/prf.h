/*
 *
 * $Log$
 * Revision 3.4  2004/11/22 21:29:55  ktr
 * Big Switch Header! SacDevCamp 04
 *
 * Revision 3.3  2002/09/11 23:23:35  dkr
 * ..._LEGAL_PRF corrected
 *
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

#ifndef _SAC_PRF_H_
#define _SAC_PRF_H_

/* the bounds of the prf list */

#define FIRST_LEGAL_PRF F_toi_S

#define LAST_LEGAL_PRF F_type_error

/* all legal prfs */

#define LEGAL_PRF(prf) ((prf >= FIRST_LEGAL_PRF) && (prf <= LAST_LEGAL_PRF))

#endif /* _SAC_PRF_H_ */
