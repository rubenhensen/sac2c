#ifndef _SAC_SACARG_H_
#define _SAC_SACARG_H_

/*
 * sbs: not sure who needs this ..... SACARGcopy and SACARGfree conflict with
 * compiler generated ones in the object code...... so made this only available
 * upon inclusion from sacarg.c and sacargconvert.c
 * FIXME: requires future investigation....
 */

#ifdef INCLUDED_FROM_LIBSAC

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#include "config.h"
#include "sacinterface.h"
#include "runtime/essentials_h/std.h"
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

extern SACarg *SACARGcopy (SACarg *arg);
extern void SACARGfree (SACarg *arg);

extern void *SACARGextractData (SACarg *arg);

#if IS_CYGWIN == 0
extern void *SACARGcopyDataUdt (basetype btype, int size, void *data);
extern void *SACARGfreeDataUdt (basetype btype, void *data);
#endif


/*
 * The following functions are for export to the sacinterface only.
 * sacinterface.h contains a duplicate of these external function
 * declarations:
 */
extern int SACARGgetDim (SACarg *arg);
extern int SACARGgetShape (SACarg *arg, int pos);
extern int SACARGgetBasetype (SACarg *arg);
extern SACarg *SACARGnewReference (SACarg *arg);

#endif

#endif /* _SAC_SACARG_H_ */
