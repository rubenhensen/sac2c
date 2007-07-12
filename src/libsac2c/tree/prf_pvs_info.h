#ifndef _PRF_PVS_INFO_H_
#define _PRF_PVS_INFO_H_

#include "types.h"

extern void PPIinitializePVs ();
extern constant *PPIgetPVIdxId (int n);
extern constant *PPIgetPVReshape (int n);
extern constant *PPIgetPVModarray (int n);
extern constant *PPIgetPVTakeAndDrop (int n);

#endif /* _PRF_PVS_INFO_H_*/
