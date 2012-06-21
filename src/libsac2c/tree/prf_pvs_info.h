#ifndef _PRF_PVS_INFO_H_
#define _PRF_PVS_INFO_H_

#include "types.h"

extern void PPIinitializePVs (void);
extern constant *PPIgetPVId (int n);
extern constant *PPIgetPVIdxId (int n);
extern constant *PPIgetPVS (int n);
extern constant *PPIgetPVV (int n);
extern constant *PPIgetPVSxS (int n);
extern constant *PPIgetPVSxV (int n);
extern constant *PPIgetPVVxS (int n);
extern constant *PPIgetPVVxV (int n);
extern constant *PPIgetPVDim (int n);
extern constant *PPIgetPVReshape (int n);
extern constant *PPIgetPVSel (int n);
extern constant *PPIgetPVShape (int n);
extern constant *PPIgetPVModarray (int n);
extern constant *PPIgetPVTakeAndDrop (int n);

#endif /* _PRF_PVS_INFO_H_*/
