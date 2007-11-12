/* $Id$ */

#include "sacarg.h"
#include "sac.h"
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

/**
 * Functions for handling references to SACargs
 */
void
SACARGfree (SACarg *arg)
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
        SAC_FREE (SACARG_DATA (arg));
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

SACarg *
SACARGcopy (SACarg *arg)
{
    /*
     * actually this does exactly the same as SACARGnewReference,
     * however we need a different name here which is _only_ to
     * be used by the code generated by the sac compiler!
     */
    return (SACARGnewReference (arg));
}

void *
SACARGextractData (SACarg *arg)
{
    void *result;

    if (SACARG_RC (arg) == 1) {
        result = SACARG_DATA (arg);
        SAC_FREE (SACARG_DESC (arg));
        SAC_FREE (arg);
    } else {
        if ((SACARG_BTYPE (arg) != T_int) && (SACARG_BTYPE (arg) != T_float)
            && (SACARG_BTYPE (arg) != T_char) && (SACARG_BTYPE (arg) != T_bool)) {
            /*
             * we cannot yet copy external types, so we emit an error here
             */
            SAC_RuntimeError ("External arguments can only be extracted if "
                              "the contained data is not shared, i.e., "
                              "its reference counter is 0!");
        }

        result = SAC_MALLOC (basetype_to_size[SACARG_BTYPE (arg)] * SACARG_SIZE (arg));
        result = memcpy (result, SACARG_DATA (arg),
                         basetype_to_size[SACARG_BTYPE (arg)] * SACARG_SIZE (arg));
        SACARGfree (arg);
    }

    return (result);
}

/**
 * Accessor functions
 */
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

/**
 * Functions used to unwrap/wrap SACargs within SAC
 */

#define UNWRAP(name, ctype, btype)                                                       \
    void SACARGunwrap##name (void **data, SAC_array_descriptor_t *desc, SACarg *arg,     \
                             SAC_array_descriptor_t arg_desc)                            \
    {                                                                                    \
        /*                                                                               \
         * we create a new reference to the data here, so we need to                     \
         * increment its reference counter!                                              \
         */                                                                              \
        SACARG_RC (arg)++;                                                               \
                                                                                         \
        *data = SACARG_DATA (arg);                                                       \
        *desc = SACARG_DESC (arg);                                                       \
                                                                                         \
        /*                                                                               \
         * we consume one reference to the outer shell. If that counter drops            \
         * to 0, we can free it! (ONLY the outer shell)                                  \
         * Note that this removes one reference to the inner data!                       \
         */                                                                              \
        DESC_RC (arg_desc)--;                                                            \
        if (DESC_RC (arg_desc) == 0) {                                                   \
            SACARG_RC (arg)--;                                                           \
            SAC_FREE (arg_desc);                                                         \
            SAC_FREE (arg);                                                              \
        }                                                                                \
    }

UNWRAP (Int, int, T_int)
UNWRAP (Bool, bool, T_bool)
UNWRAP (Float, float, T_float)
UNWRAP (Double, double, T_double)
UNWRAP (Char, char, T_char)

void
SACARGunwrapUdt (void **data, SAC_array_descriptor_t *desc, SACarg *arg,
                 SAC_array_descriptor_t arg_desc)
{
    /*
     * we create a new reference to the data here, so we need to
     * increment its reference counter!
     */
    SACARG_RC (arg)++;

    *data = SACARG_DATA (arg);
    *desc = SACARG_DESC (arg);

    /*
     * we consume one reference to the outer shell. If that counter drops
     * to 0, we can free it! (ONLY the outer shell)
     * Note that this removes one reference to the inner data!
     */
    DESC_RC (arg_desc)--;
    if (DESC_RC (arg_desc) == 0) {
        SACARG_RC (arg)--;
        SAC_FREE (arg_desc);
        SAC_FREE (arg);
    }
}

#define WRAP(name, ctype, btype)                                                         \
    void SACARGwrap##name (SACarg **arg, SAC_array_descriptor_t *desc, void *data,       \
                           SAC_array_descriptor_t data_desc)                             \
    {                                                                                    \
        /*                                                                               \
         * we simply wrap it. As we consume one reference, we have                       \
         * to decrement the rc. However, SACARGmakeSacArg holds a                        \
         * new reference, so the RC cannot be 0.                                         \
         */                                                                              \
        *arg = SACARGmakeSacArg (btype, data_desc, data);                                \
        DESC_RC (data_desc)--;                                                           \
        /*                                                                               \
         * now we need a descriptor! As SACargs use standard SAC descriptors,            \
         * we can use those functions to build one. However, we have to                  \
         * manually increment the RC by 1!                                               \
         */                                                                              \
        *desc = SACARGmakeDescriptorVect (0, NULL);                                      \
        DESC_RC ((*desc))++;                                                             \
    }

WRAP (Int, int, T_int)
WRAP (Bool, bool, T_bool)
WRAP (Float, float, T_float)
WRAP (Double, double, T_double)
WRAP (Char, char, T_char)

void
SACARGwrapUdt (SACarg **arg, SAC_array_descriptor_t *desc, basetype btype, void *data,
               SAC_array_descriptor_t data_desc)
{
    /*
     * we simply wrap it. As we consume one reference, we have
     * to decrement the rc. However, SACARGmakeSacArg holds a
     * new reference, so the RC cannot be 0.
     */
    *arg = SACARGmakeSacArg (btype, data_desc, data);
    DESC_RC (data_desc)--;
    /*
     * now we need a descriptor! As SACargs use standard SAC descriptors,
     * we can use those functions to build one. However, we have to
     * manually increment the RC by 1!
     */
    *desc = SACARGmakeDescriptorVect (0, NULL);
    DESC_RC ((*desc))++;
}

#define HASTYPE(name, ctype, btype)                                                      \
    bool SACARGis##name (SACarg *arg)                                                    \
    {                                                                                    \
        return (SACARG_BTYPE (arg) == btype);                                            \
    }

HASTYPE (Int, int, T_int)
HASTYPE (Bool, bool, T_bool)
HASTYPE (Float, float, T_float)
HASTYPE (Double, double, T_double)
HASTYPE (Char, char, T_char)

bool
SACARGisUdt (basetype btype, SACarg *arg)
{
    return (SACARG_BTYPE (arg) == btype);
}
