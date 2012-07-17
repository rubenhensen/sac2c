/* $Id$ */

#include "sacarg.h"
#include "sac.h"
#include <stdio.h>

#include "string.h"

#ifdef __cplusplus
#define va_copy(a, b) __builtin_va_copy (a, b)
#endif

/**
 * SACarg structure
 */
struct SAC_SACARG {
    SAC_array_descriptor_t desc;
    void *data;
    basetype mbasetype;
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
#define SACARG_BTYPE(n) ((n)->mbasetype)

/**
 *
 * Hidden debug stuff
 */

void
SACARGprint (SACarg *arg)
{
    printf ("descriptor at 0x%08lx: RC: %d\ndata at 0x%08lx\n", (long)SACARG_DESC (arg),
            SACARG_RC (arg), (long)SACARG_DATA (arg));
}

/*
 * basetype to size conversion
 */
static const int basetype_to_size[] = {
#define TYP_IFsize(sz) sz
#include "type_info.mac"
#undef TYP_IFsize
};

#define BTYPE_ISINTERNAL(btype)                                                          \
    ((btype == T_int) || (btype == T_float) || (btype == T_byte) || (btype == T_short)   \
     || (btype == T_long) || (btype == T_longlong) || (btype == T_ubyte)                 \
     || (btype == T_ushort) || (btype == T_uint) || (btype == T_ulong)                   \
     || (btype == T_ulonglong) || (btype == T_double) || (btype == T_char)               \
     || (btype == T_bool))

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
SACARGfreeDataInternal (basetype btype, void *data)
{
    SAC_FREE (data);
}

/*
 * this function is generated by sac4c
 */
extern void SACARGfreeDataUdt (basetype btype, void *data);

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
        if (!BTYPE_ISINTERNAL (SACARG_BTYPE (arg))) {
            SACARGfreeDataUdt (SACARG_BTYPE (arg), SACARG_DATA (arg));
        } else {
            SACARGfreeDataInternal (SACARG_BTYPE (arg), SACARG_DATA (arg));
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
    SACarg *res = NULL;
    if (arg != NULL) {
        res = SACARGmakeSacArg (SACARG_BTYPE (arg), SACARG_DESC (arg), SACARG_DATA (arg));
    }
    return (res);
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
SACARGcopyDataInternal (basetype btype, int size, void *data)
{
    void *result;

    result = SAC_MALLOC (basetype_to_size[btype] * size);
    result = memcpy (result, data, basetype_to_size[btype] * size);

    return (result);
}

/*
 * this function is generated by sac4c
 */
extern void *SACARGcopyDataUdt (basetype btype, int size, void *data);

void *
SACARGextractData (SACarg *arg)
{
    void *result;

    if (SACARG_RC (arg) == 1) {
        result = SACARG_DATA (arg);
        SAC_FREE (SACARG_DESC (arg));
        SAC_FREE (arg);
    } else {
        if (!BTYPE_ISINTERNAL (SACARG_BTYPE (arg))) {
            result = SACARGcopyDataUdt (SACARG_BTYPE (arg), SACARG_SIZE (arg),
                                        SACARG_DATA (arg));
        } else {
            result = SACARGcopyDataInternal (SACARG_BTYPE (arg), SACARG_SIZE (arg),
                                             SACARG_DATA (arg));
        }

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

#define UNWRAPWRAPPER(name, ctype)                                                       \
    void SACARGunwrapUdt##name (ctype **data, SAC_array_descriptor_t *desc, SACarg *arg, \
                                SAC_array_descriptor_t arg_desc)                         \
    {                                                                                    \
        SACARGunwrapUdt ((void **)data, desc, arg, arg_desc);                            \
    }

UNWRAPWRAPPER (Int, int)
UNWRAPWRAPPER (Bool, bool)
UNWRAPWRAPPER (Float, float)
UNWRAPWRAPPER (Double, double)
UNWRAPWRAPPER (Char, char)

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

#define WRAPWRAPPER(name, ctype)                                                         \
    void SACARGwrapUdt##name (SACarg **arg, SAC_array_descriptor_t *desc,                \
                              basetype btype, ctype *data,                               \
                              SAC_array_descriptor_t data_desc)                          \
    {                                                                                    \
        SACARGwrapUdt (arg, desc, btype, (void *)data, data_desc);                       \
    }

WRAPWRAPPER (Int, int)
WRAPWRAPPER (Bool, bool)
WRAPWRAPPER (Float, float)
WRAPWRAPPER (Double, double)
WRAPWRAPPER (Char, char)

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

UNWRAP (Byte, byte, T_byte)
UNWRAP (Short, short, T_short)
UNWRAP (Long, long, T_long)
UNWRAP (Longlong, longlong, T_longlong)
UNWRAP (Ubyte, ubyte, T_ubyte)
UNWRAP (Ushort, ushort, T_ushort)
UNWRAP (Uint, uint, T_uint)
UNWRAP (Ulong, ulong, T_ulong)
UNWRAP (Ulonglong, ulonglong, T_ulonglong)

WRAP (Byte, byte, T_byte)
WRAP (Short, short, T_short)
WRAP (Long, long, T_long)
WRAP (Longlong, longlong, T_longlong)
WRAP (Ubyte, ubyte, T_ubyte)
WRAP (Ushort, ushort, T_ushort)
WRAP (Uint, uint, T_uint)
WRAP (Ulong, ulong, T_ulong)
WRAP (Ulonglong, ulonglong, T_ulonglong)

#ifndef SAC_BACKEND_MUTC
WRAPWRAPPER (Byte, byte)
WRAPWRAPPER (Short, short)
WRAPWRAPPER (Long, long)
WRAPWRAPPER (Longlong, longlong)
WRAPWRAPPER (Ubyte, ubyte)
WRAPWRAPPER (Ushort, ushort)
WRAPWRAPPER (Uint, uint)
WRAPWRAPPER (Ulong, ulong)
WRAPWRAPPER (Ulonglong, ulonglong)

UNWRAPWRAPPER (Byte, byte)
UNWRAPWRAPPER (Short, short)
UNWRAPWRAPPER (Long, long)
UNWRAPWRAPPER (Longlong, longlong)
UNWRAPWRAPPER (Ubyte, ubyte)
UNWRAPWRAPPER (Ushort, ushort)
UNWRAPWRAPPER (Uint, uint)
UNWRAPWRAPPER (Ulong, ulong)
UNWRAPWRAPPER (Ulonglong, ulonglong)
#endif /* ! SAC_BACKEND_MUTC */

HASTYPE (Byte, byte, T_byte)
HASTYPE (Short, short, T_short)
HASTYPE (Long, long, T_long)
HASTYPE (Longlong, longlong, T_longlong)
HASTYPE (Ubyte, ubyte, T_ubyte)
HASTYPE (Ushort, ushort, T_ushort)
HASTYPE (Uint, uint, T_uint)
HASTYPE (Ulong, ulong, T_ulong)
HASTYPE (Ulonglong, ulonglong, T_ulonglong)

bool
SACARGisUdt (basetype btype, SACarg *arg)
{
    return (SACARG_BTYPE (arg) == btype);
}
