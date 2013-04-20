#ifndef _SAC_BIT_H_
#define _SAC_BIT_H_

#include "types.h"

typedef int BITint;
typedef shape *BITshp;
typedef constant *BITval;

extern bool BITeqIntInt (BITint a, BITint b);
extern bool BITleIntInt (BITint a, BITint b);
extern bool BITgeIntInt (BITint a, BITint b);
extern bool BITeqShpShp (BITshp a, BITshp b);
extern bool BITeqValVal (BITval a, BITval b);

extern bool BITakvaks (BITint dim, BITshp shp, BITval val, BITint dim2, BITshp shp2);
extern bool BITaksakd (BITint dim, BITshp shp, BITint dim2);
extern bool BITakdaudge (BITint dim, BITint dim2);
extern bool BITaudgeaudge (BITint dim, BITint dim2);

#endif /* _SAC_BIT_H_ */
