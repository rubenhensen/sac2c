/*
 * $Id$
 */

#ifndef _SAC_BIT_H_
#define _SAC_BIT_H_

#include "types.h"

typedef int BITint;
typedef shape *BITshp;
typedef const *BITval;

extern bool BITeqIntInt (BITint a, BITint b);
extern bool BITleIntInt (BITint a, BITint b);
extern bool BITgeIntInt (BITint a, BITint b);
extern bool BITeqShpShp (BITshp a, BITshp b);
extern bool BITeqValVal (BITval a, BITval b);

extern bool BITakvaks (BITint dim, BITshape shp, BITconst val, BITint dim, BITshape shp);
extern bool BITaksakd (BITint dim, BITshape shp, BITint dim);
extern bool BITakdaudge (BITint dim, BITint dim);
extern bool BITaudgeaudge (BITint dim, BITint dim);

#endif /* _SAC_BIT_H_ */
