/* $Id$ */

#include "sacarg.h"
#include "string.h"

/**
 * SACarg structure
 */
struct SAC_SACARG {
    SAC_array_descriptor_t desc;
    void *data;
    basetype basetype;
};

/**
 * access macros
 */
#define SACARG_DESC(n) ((n)->desc)
#define SACARG_DIM(n) DESC_DIM (SACARG_DESC (n))
#define SACARG_RC(n) DESC_RC (SACARG_DESC (n))
#define SACARG_SIZE(n) DESC_SIZE (SACARG_DESC (n))
#define SACARG_SHAPE(n, p) DESC_SHAPE (SACARG_DESC (n), p)
#define SACARG_DATA(n) ((n)->data)
#define SACARG_BTYPE(n) ((n)->basetype)

/*
 * basetype to size conversion
 */
static const int basetype_to_size[] = {
#define TYP_IFsize(sz) sz
#include "type_info.mac"
#undef TYP_IFsize
};

/**
 * Functions for creating SACargs
 */

SAC_array_descriptor_t
SACARGmakeDescriptor (int dim, va_list args)
{
    int pos;
    int size;
    va_list argp;
    SAC_array_descriptor_t result;

    result = (SAC_array_descriptor_t)SAC_MALLOC (BYTE_SIZE_OF_DESC (dim));

    size = 1;
    va_copy (argp, args);
    for (pos = 0; pos < dim; pos++) {
        DESC_SHAPE (result, pos) = va_arg (argp, int);
        size *= DESC_SHAPE (result, pos);
    }
    va_end (argp);

    DESC_DIM (result) = dim;
    DESC_SIZE (result) = size;
    DESC_RC (result) = 0;

    return (result);
}

SAC_array_descriptor_t
SACARGmakeDescriptorVect (int dim, int *shape)
{
    int pos;
    int size;
    SAC_array_descriptor_t result;

    result = (SAC_array_descriptor_t)SAC_MALLOC (BYTE_SIZE_OF_DESC (dim));

    size = 1;
    for (pos = 0; pos < dim; pos++) {
        DESC_SHAPE (result, pos) = shape[pos];
        size *= DESC_SHAPE (result, pos);
    }

    DESC_DIM (result) = dim;
    DESC_SIZE (result) = size;
    DESC_RC (result) = 0;

    return (result);
}

SACarg *
SACARGmakeSacArg (basetype btype, SAC_array_descriptor_t desc, void *data)
{
    SACarg *result;

    result = (SACarg *)SAC_MALLOC (sizeof (SACarg));

    SACARG_DESC (result) = desc;
    SACARG_DATA (result) = data;
    SACARG_BTYPE (result) = btype;

    /*
     * we reference this data once now, so we increase the RC
     */
    SACARG_RC (result)++;

    return (result);
}

void
SACARGfreeSacArg (SACarg *arg, bool freedata)
{
    /*
     * we will free one reference to the array, so decrement its RC
     */
    SACARG_RC (arg)--;

    if (SACARG_RC (arg) == 0) {
        /*
         * last reference freed, so free the contained data
         */
        SAC_FREE (SACARG_DESC (arg));
        if (freedata) {
            SAC_FREE (SACARG_DATA (arg));
        }
    }

    /*
     * free the outer wrapper structure
     */
    SAC_FREE (arg);
}

SACarg *
SACARGnewReference (SACarg *arg)
{
    return (SACARGmakeSacArg (SACARG_BTYPE (arg), SACARG_DESC (arg), SACARG_DATA (arg)));
}

void *
SACARGextractData (SACarg *arg)
{
    void *result;

    if (SACARG_RC (arg) == 1) {
        result = SACARG_DATA (arg);
        SACARGfreeSacArg (arg, false);
    } else {
        result = SAC_MALLOC (basetype_to_size[SACARG_BTYPE (arg)] * SACARG_SIZE (arg));
        result = memcpy (result, SACARG_DATA (arg),
                         basetype_to_size[SACARG_BTYPE (arg)] * SACARG_SIZE (arg));
        SACARGfreeSacArg (arg, true);
    }

    return (result);
}

int
SACARGgetDim (SACarg *arg)
{
    return (SACARG_DIM (arg));
}

int
SACARGgetShape (SACarg *arg, int pos)
{
    return (SACARG_SHAPE (arg, pos));
}

int
SACARGgetBasetype (SACarg *arg)
{
    return (SACARG_BTYPE (arg));
}
