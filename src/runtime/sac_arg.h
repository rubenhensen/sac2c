/*
 * $Log$
 * Revision 1.3  2000/07/06 09:23:32  nmw
 * reference to type_info.mac removed
 *
 * Revision 1.2  2000/07/06 08:18:57  dkr
 * RCS header added
 * absolute path of type_info.mac-include removed
 * -> but I think type_info.mac should NOT be included here anyway!
 *
 */

#ifndef _sac_arg_h

#define _sac_arg_h

/* abstract datatype for SAC <-> c arguments and return types */
#include "sac.h"
#include "sac_cinterface.h"

/* basetypes (same as in sac2c internally used) */
typedef int SAC_ARG_simpletype;

/* SAC integer array */
typedef struct SAC_ARG_STRUCT {
    int type;    /* type of argument */
    int *shpvec; /* shapevector of argument */
    int dim;     /* dim of shape */
    void *elems; /* pointer to data elements */
    int *rc;     /* pointer to revcounter */
    int lrc;     /* local refcounter copy */
} SAC_ARG_STRUCT;

#define SAC_ARG_LRC(a) (a->lrc)
#define SAC_ARG_RC(a) (a->rc)
#define SAC_ARG_ELEMS(a) (a->elems)
#define SAC_ARG_TYPE(a) (a->type)
#define SAC_ARG_SHPVEC(a) (a->shpvec)
#define SAC_ARG_DIM(a) (a->dim)

extern SAC_arg SAC_CI_NewSACArg (SAC_ARG_simpletype basetype, int dim, int *shpvec);
extern SAC_arg SAC_CI_CreateSACArg (SAC_ARG_simpletype basetype, int dim, ...);
extern bool SAC_CI_CmpSACArgType (SAC_arg sa, SAC_ARG_simpletype basetype, int dim, ...);
extern void SAC_CI_FreeSACArg (SAC_arg sa);

#endif
