/*
 * $Id$
 */

#ifndef _SAC_BIT_H_
#define _SAC_BIT_H_

#include "types.h"

typedef int BITint;
typedef shape *BITshp;
typedef const *BITval;

extern Bool BITeqIntInt (BITint a, BITint b);
extern Bool BITleIntInt (BITint a, BITint b);
extern Bool BITgeIntInt (BITint a, BITint b);
extern Bool BITeqShpShp (BITshp a, BITshp b);
extern Bool BITeqValVal (BITval a, BITval b);

extern Bool BITakvaks (BITint dim, BITshape shp, BITconst val, BITint dim, BITshape shp);
extern Bool BITaksakd (BITint dim, BITshape shp, BITint dim);
extern Bool BITakdaudge (BITint dim, BITint dim);

#endif /* _SAC_BIT_H_ */
