/* $Id$ */

#ifndef _SAC_SACARG_H_
#define _SAC_SACARG_H_

#include "sacinterface.h"
#include "sac.h"
#include <stdarg.h>

/*
 * names for types
 */

typedef enum {
#define TYP_IFname(name) name
#include "type_info.mac"
#undef TYP_IFname
} basetype;

/**
 * low-level SACarg functions
 */

extern SAC_array_descriptor_t SACARGmakeDescriptor (int dim, va_list args);
extern SAC_array_descriptor_t SACARGmakeDescriptorVect (int dim, int *shape);
extern SACarg *SACARGmakeSacArg (basetype btype, SAC_array_descriptor_t desc, void *data);

extern SACarg *SACARGnewReference (SACarg *arg);
extern SACarg *SACARGcopy (SACarg *arg);
extern void SACARGfree (SACarg *arg);

extern void *SACARGextractData (SACarg *arg);
extern int SACARGgetDim (SACarg *arg);
extern int SACARGgetShape (SACarg *arg, int pos);
extern int SACARGgetBasetype (SACarg *arg);

#endif /* _SAC_SACARG_H_ */
