#define INCLUDED_FROM_LIBSAC
#include "sacarg.h"
#undef INCLUDED_FROM_LIBSAC

#include <stdio.h>
#include <assert.h>
#include "config.h"
#include "string.h"

#if ENABLE_CUDA
#include <cuda_runtime.h>
#endif

#include "runtime/essentials_h/types.h" // floatvec (through type_info.mac)
#include "runtime/essentials_h/bool.h" // bool
#include "runtime/essentials_h/cuda_transfer_methods.h" // SAC_DO_CUDA_ALLOC
#include "runtime/essentials_h/std_gen.h" // SAC_ND_DECL__DESC
#include "runtime/essentials_h/std.h" // SAC_array_descriptor_t


#ifdef __cplusplus
#define va_copy(a, b) __builtin_va_copy (a, b)
#endif

#ifndef SAC_RC_METHOD
#error SAC_RC_METHOD is not defined!
#endif

#if SAC_RC_METHOD == -1
#error SAC_RC_METHOD is defined to an invalid value!
#endif

#ifndef SAC_BACKEND_MUTC
#include "fun-attrs.h"
#else
#define NO_FUN_ATTRS
#include "fun-attrs.h"
#undef NO_FUN_ATTRS
#endif

/*#undef SAC_FREE
#define SAC_FREE(x)               SAC_FREE_dbg(x, sizeof(*(x)))(*/
#define SAC_FREE_nrm(x) free ((x));
#define SAC_FREE_dbg(x, siz)                                                             \
    {                                                                                    \
        memset ((x), 0, (siz));                                                          \
        free ((x));                                                                      \
    }

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
#define SACARG_DATA(n) ((n)->data)
#define SACARG_BTYPE(n) ((n)->mbasetype)
#define SACARG_DIM(n) DESC_DIM (SACARG_DESC (n))
#define SACARG_SIZE(n) DESC_SIZE (SACARG_DESC (n))
#define SACARG_SHAPE(n, p) DESC_SHAPE (SACARG_DESC (n), p)

/**
 *
 * Hidden debug stuff
 */

void
SACARGprint (SACarg *arg)
{
    printf ("descriptor at 0x%08lx:\ndata at 0x%08lx\n", (long)SACARG_DESC (arg),
            (long)SACARG_DATA (arg));
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

#define result_nt                                                                        \
    (result, (AUD, (NHD, (NUQ, (INT, (GLO, (FAG, (NOT, (NDI, (OTH, ))))))))))
#define param_nt (param, (AUD, (NHD, (NUQ, (INT, (GLO, (FAG, (NOT, (NDI, (OTH, ))))))))))
#define param_hid_nt                                                                     \
    (param, (SCL, (HID, (NUQ, (INT, (GLO, (FAG, (NOT, (NDI, (OTH, ))))))))))
#define out_nt (out, (AUD, (NHD, (NUQ, (INT, (GLO, (FAG, (NOT, (NDI, (OTH, ))))))))))
#define data_nt (data, (AUD, (NHD, (NUQ, (INT, (GLO, (FAG, (NOT, (NDI, (OTH, ))))))))))
#define data_hid_nt                                                                      \
    (data, (SCL, (HID, (NUQ, (INT, (GLO, (FAG, (NOT, (NDI, (OTH, ))))))))))

/**
 * Functions for creating SACargs
 */

/** Create a pristine descriptor for SACarg itself. */
SAC_array_descriptor_t
SACARGmakeDescriptor (int dim, va_list args)
{
    int pos;
    va_list argp;

    SAC_ND_DECL__DESC (result_nt, );
    int SAC_ND_A_MIRROR_SIZE (result_nt) = 1;
    int SAC_ND_A_MIRROR_DIM (result_nt) = dim;

    SAC_ND_ALLOC__DESC (result_nt, dim);
    SAC_ND_INIT__RC (result_nt, 0);

    va_copy (argp, args);
    for (pos = 0; pos < dim; pos++) {
        SAC_ND_A_MIRROR_SIZE (result_nt) *= SAC_ND_A_DESC_SHAPE (result_nt, pos)
          = va_arg (argp, int);
    }
    va_end (argp);
    SAC_ND_A_DESC_SIZE (result_nt) = SAC_ND_A_MIRROR_SIZE (result_nt);

    return (SAC_ND_A_DESC (result_nt));
}

/** Create a pristine descriptor for SACarg itself. */
SAC_array_descriptor_t
SACARGmakeDescriptorVect (int dim, int *shape)
{
    int pos;

    SAC_ND_DECL__DESC (result_nt, );
    int SAC_ND_A_MIRROR_SIZE (result_nt) = 1;
    int SAC_ND_A_MIRROR_DIM (result_nt) = dim;
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    int SAC_ND_A_MIRROR_CUDA_PINNED (result_nt) = 0;
#endif

    SAC_ND_ALLOC__DESC (result_nt, dim);
    SAC_ND_INIT__RC (result_nt, 0);

    for (pos = 0; pos < dim; pos++) {
        SAC_ND_A_MIRROR_SIZE (result_nt) *= SAC_ND_A_DESC_SHAPE (result_nt, pos)
          = shape[pos];
    }
    SAC_ND_A_DESC_SIZE (result_nt) = SAC_ND_A_MIRROR_SIZE (result_nt);
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    SAC_ND_A_DESC_CUDA_PINNED (result_nt) = SAC_ND_A_MIRROR_CUDA_PINNED (result_nt);
#endif

    return (SAC_ND_A_DESC (result_nt));
}

/* Unfortunately param is the wrong way round, else we could just use one macro */
SACarg *
SACARGmakeSacArg (basetype btype, SAC_ND_PARAM_in_justdesc (param_nt, void),
                  SAC_ND_PARAM_in_nodesc (param_nt, void))
{
    SACarg *result;

    result = (SACarg *)SAC_MALLOC (sizeof (SACarg));

    SACARG_DATA (result) = SAC_ND_A_FIELD (param_nt);
    SACARG_BTYPE (result) = btype;

    /* Create a new reference (inc_ref), then convert the reference
     * into an asynchronous mode. */
    SAC_ND_INC_RC (param_nt, 1);

#ifdef SAC_DESC_RC_GIVE_ASYNC
    SAC_DESC_RC_GIVE_ASYNC (SACARG_DESC (result), SAC_ND_A_DESC (param_nt));
#else
    SACARG_DESC (result) = SAC_ND_A_DESC (param_nt);
#endif

    return (result);
}

/*
 * this function is generated by sac4c
 */

#if IS_CYGWIN
void *
SACARGfreeDataUdt (basetype btype, void *data)
{
}
#else
extern void *SACARGfreeDataUdt (basetype btype, void *data);
#endif

#define TYP_SEPARATOR
#define TYP_IFname(name)                                                                 \
    static inline void SACARGfreeData##name (void *data)                                 \
    {                                                                                    \
        if (data != NULL) {                                                              \
            SAC_FREE (data);                                                             \
        }                                                                                \
    }
#include "type_info.mac"
#undef TYP_IFname
#undef TYP_SEPARATOR

/* Check data is set as it may have been extracted */
static inline void
SACARGfreeDataInternal (void *data)
{
    if (data != NULL) {
        SAC_FREE (data);
    }
}

void
SACARGfree (SACarg *arg)
{
    /* Release/free the SACarg structure.
     * We will free one reference to the array, so decrement its RC.
     */

    SAC_ND_DECL__DATA (data_nt, void, );
    SAC_ND_DECL__DESC (data_nt, );
    int SAC_ND_A_MIRROR_SIZE (data_nt) = 0;
    int SAC_ND_A_MIRROR_DIM (data_nt) = 0;
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    int SAC_ND_A_MIRROR_CUDA_PINNED (data_nt) = 0;
#endif

    SAC_ND_A_FIELD (data_nt) = SACARG_DATA (arg);
    SAC_ND_A_DESC (data_nt) = SACARG_DESC (arg);
    SAC_ND_A_MIRROR_SIZE (data_nt) = SAC_ND_A_DESC_SIZE (data_nt);
    SAC_ND_A_MIRROR_DIM (data_nt) = SAC_ND_A_DESC_DIM (data_nt);
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    SAC_ND_A_MIRROR_CUDA_PINNED (data_nt) = SAC_ND_A_DESC_CUDA_PINNED (data_nt);
#endif

    /*
     * Make name tuple hidden so we can control how freeing is done
     */
    if (BTYPE_ISINTERNAL (SACARG_BTYPE (arg))) {
        switch (SACARG_BTYPE (arg)) {
#define TYP_SEPARATOR
#define TYP_IFname(name)                                                                 \
    case name:                                                                           \
        SAC_ND_DEC_RC_FREE (data_hid_nt, 1, SACARGfreeData##name);                       \
        break;
#include "type_info.mac"
#undef TYP_IFname
#undef TYP_SEPARATOR
        default:
            assert (0 == 1);
        }
    } else {
        SAC_ND_DEC_RC_FREE (data_hid_nt, 1, SACARGfreeDataInternal);
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

/*
 * this function is generated by sac4c
 */

#if IS_CYGWIN
void *
SACARGcopyDataUdt (basetype btype, int size, void *data)
{
}
#else
extern void *SACARGcopyDataUdt (basetype btype, int size, void *data);
#endif

void *
SACARGextractData (SACarg *arg)
{
    /* Release/free the SACarg structure.
     * We will free one reference to the array, so decrement its RC.
     */
    void *result = NULL;
    SAC_ND_DECL__DATA (data_nt, void, );
    SAC_ND_DECL__DESC (data_nt, );
    int SAC_ND_A_MIRROR_SIZE (data_nt) = 0;
    int SAC_ND_A_MIRROR_DIM (data_nt) = 0;
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    int SAC_ND_A_MIRROR_CUDA_PINNED (data_nt) = 0;
#endif
    int rc = 0;

    SAC_ND_A_FIELD (data_nt) = SACARG_DATA (arg);
    SAC_ND_A_DESC (data_nt) = SACARG_DESC (arg);

    SAC_ND_A_MIRROR_SIZE (data_nt) = SAC_ND_A_DESC_SIZE (data_nt);
    SAC_ND_A_MIRROR_DIM (data_nt) = SAC_ND_A_DESC_DIM (data_nt);
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    SAC_ND_A_MIRROR_CUDA_PINNED (data_nt) = SAC_ND_A_DESC_CUDA_PINNED (data_nt);
#endif

    /** SAC_ND_A_RC will always return 1 iff it is safe to reuse the
        data associated with this descriptor */
    rc = SAC_ND_A_RC (data_nt);
    if (rc == 1) {
        /* Can reuse */
        result = SAC_ND_A_FIELD (data_nt);
        /* Set to null so not freed with the rest */
        SACARG_DATA (arg) = NULL;
    } else {
        /* Can NOT reuse */
        if (!BTYPE_ISINTERNAL (SACARG_BTYPE (arg))) {
            result = SACARGcopyDataUdt (SACARG_BTYPE (arg), SAC_ND_A_SIZE (data_nt),
                                        SAC_ND_A_FIELD (data_nt));
        } else {
            result = SAC_MALLOC (basetype_to_size[SACARG_BTYPE (arg)]
                                 * SAC_ND_A_SIZE (data_nt));
            result
              = memcpy (result, SAC_ND_A_FIELD (data_nt),
                        basetype_to_size[SACARG_BTYPE (arg)] * SAC_ND_A_SIZE (data_nt));
        }
    }

    SACARGfree (arg);

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

static inline void SACARG_common_unwrap (SAC_ND_PARAM_out (out_nt, void),
                                         SAC_ND_PARAM_in (param_nt, SACarg))
{
    int UNUSED SAC_ND_A_MIRROR_SIZE (param_nt) = SAC_ND_A_DESC_SIZE (param_nt);
    int UNUSED SAC_ND_A_MIRROR_DIM (param_nt) = SAC_ND_A_DESC_DIM (param_nt);

    SAC_ND_DECL__DATA (data_nt, void, );
    SAC_ND_DECL__DESC (data_nt, );
    int SAC_ND_A_MIRROR_SIZE (data_nt) = 0;
    int SAC_ND_A_MIRROR_DIM (data_nt) = 0;
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    int SAC_ND_A_MIRROR_CUDA_PINNED (data_nt) = 0;
#endif

    SAC_ND_A_FIELD (data_nt) = SACARG_DATA (SAC_ND_A_FIELD (param_nt));
    SAC_ND_A_DESC (data_nt) = SACARG_DESC (SAC_ND_A_FIELD (param_nt));
    SAC_ND_A_MIRROR_SIZE (data_nt) = SAC_ND_A_DESC_SIZE (data_nt);
    SAC_ND_A_MIRROR_DIM (data_nt) = SAC_ND_A_DESC_DIM (data_nt);
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    SAC_ND_A_MIRROR_CUDA_PINNED (data_nt) = SAC_ND_A_DESC_CUDA_PINNED (data_nt);
#endif
    /*
     * we create a new reference to the data here, so we need to
     * increment its reference counter!
     */
    SAC_ND_INC_RC (data_nt, 1);

    /*
     * we consume one reference to the outer shell. If that counter drops
     * to 0, we can free it! (ONLY the outer shell)
     * Note that this removes one reference to the inner data!
     */

    SAC_ND_DEC_RC_FREE (param_hid_nt, 1, SACARGfree);

    SAC_ND_RET_out (out_nt, data_nt);
}

#define UNWRAP(name, ctype, btype)                                                       \
    void SACARGunwrap##name (SAC_ND_PARAM_out (out_nt, ctype),                           \
                             SAC_ND_PARAM_in (param_nt, SACarg))                         \
    {                                                                                    \
        SAC_ND_DECL_PARAM_inout (out_nt, void);                                          \
                                                                                         \
        SACARG_common_unwrap (SAC_ND_ARG_inout (out_nt, void),                           \
                              SAC_ND_ARG_in (param_nt, SACarg));                         \
        SAC_ND_RET_inout (out_nt, out_nt);                                               \
    }

UNWRAP (Int, int, T_int)          /* SACARGwrapInt */
UNWRAP (Bool, bool, T_bool)       /* SACARGwrapBool */
UNWRAP (Float, float, T_float)    /* SACARGwrapFloat */
UNWRAP (Double, double, T_double) /* SACARGwrapDouble */
UNWRAP (Char, char, T_char)       /* SACARGwrapChar */

void SACARGunwrapUdt (SAC_ND_PARAM_inout (out_nt, void),
                      SAC_ND_PARAM_in (param_nt, SACarg))
{
    SAC_ND_DECL_PARAM_inout (out_nt, void);

    SACARG_common_unwrap (SAC_ND_ARG_inout (out_nt, void),
                          SAC_ND_ARG_in (param_nt, SACarg));
    SAC_ND_RET_inout (out_nt, out_nt);
}

#define UNWRAPWRAPPER(name, ctype)                                                       \
    void SACARGunwrapUdt##name (SAC_ND_PARAM_inout (out_nt, ctype),                      \
                                SAC_ND_PARAM_in (param_nt, SACarg))                      \
    {                                                                                    \
        SAC_ND_DECL_PARAM_inout (out_nt, void);                                          \
        int UNUSED SAC_ND_A_MIRROR_SIZE (out_nt) = SAC_ND_A_DESC_SIZE (out_nt);          \
        int UNUSED SAC_ND_A_MIRROR_DIM (out_nt) = SAC_ND_A_DESC_DIM (out_nt);            \
                                                                                         \
        SACARGunwrapUdt (SAC_ND_ARG_inout (out_nt, void),                                \
                         SAC_ND_ARG_in (param_nt, SACarg));                              \
        SAC_ND_RET_inout (out_nt, out_nt);                                               \
    }

UNWRAPWRAPPER (Int, int)
UNWRAPWRAPPER (Bool, bool)
UNWRAPWRAPPER (Float, float)
UNWRAPWRAPPER (Double, double)
UNWRAPWRAPPER (Char, char)

static inline void
SACARG_common_wrap (SAC_ND_PARAM_out (out_nt, SACarg), basetype btype,
                    SAC_ND_PARAM_in (param_nt, void))
{
    SAC_ND_DECL__DATA (data_nt, void, );
    SAC_ND_DECL__DESC (data_nt, );
    int UNUSED SAC_ND_A_MIRROR_SIZE (data_nt) = 0;
    int UNUSED SAC_ND_A_MIRROR_DIM (data_nt) = 0;
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
    int UNUSED SAC_ND_A_MIRROR_CUDA_PINNED (data_nt) = 0;
    int param__cuda_pinned = 0; /* FIXME something strange is happening here */
#endif

    /*
     * we simply wrap it. As we consume one reference, we have
     * to decrement the rc.
     * SACARGmakeSacArg holds a new reference, but it is potentially
     * an asynchronous copy, hence we need to use the full dec.ref. operation.
     */
    SAC_ND_A_FIELD (data_nt)
      = SACARGmakeSacArg (btype, SAC_ND_A_DESC (param_nt), SAC_ND_A_FIELD (param_nt));
    SAC_ND_DEC_RC_FREE (param_nt, 1, );
    /*
     * now we need a descriptor! As SACargs use standard SAC descriptors,
     * we can use those functions to build one. However, we have to
     * manually increment the RC by 1!
     */
    SAC_ND_A_DESC (data_nt) = SACARGmakeDescriptorVect (0, NULL);
    SAC_ND_INC_RC (data_nt, 1);
    SAC_ND_RET_inout (out_nt, data_nt);
}

#define WRAP(name, ctype, btype)                                                         \
    void SACARGwrap##name (SAC_ND_PARAM_inout (out_nt, SACarg),                          \
                           SAC_ND_PARAM_in (param_nt, ctype))                            \
    {                                                                                    \
        SAC_ND_DECL_PARAM_inout (out_nt, SACarg);                                        \
        SACARG_common_wrap (SAC_ND_ARG_inout (out_nt, SACarg), btype,                    \
                            SAC_ND_ARG_in (param_nt, void));                             \
        SAC_ND_RET_inout (out_nt, out_nt);                                               \
    }

WRAP (Int, int, T_int)
WRAP (Bool, bool, T_bool)
WRAP (Float, float, T_float)
WRAP (Double, double, T_double)
WRAP (Char, char, T_char)

void
SACARGwrapUdt (SAC_ND_PARAM_out (out_nt, SACarg), basetype btype,
               SAC_ND_PARAM_in (param_nt, void))
{
    SAC_ND_DECL__DATA (data_nt, SACarg, );
    SAC_ND_DECL__DESC (data_nt, );
    SACARG_common_wrap (SAC_ND_ARG_out (data_nt, SACarg), btype,
                        SAC_ND_ARG_in (param_nt, void));
    SAC_ND_RET_inout (out_nt, data_nt);
}

#define WRAPWRAPPER(name, ctype)                                                         \
    void SACARGwrapUdt##name (SAC_ND_PARAM_inout (out_nt, SACarg), basetype btype,       \
                              SAC_ND_PARAM_in (param_nt, ctype))                         \
    {                                                                                    \
        SAC_ND_DECL_PARAM_inout (out_nt, SACarg);                                        \
        SACARGwrapUdt (SAC_ND_ARG_inout (out_nt, SACarg), btype,                         \
                       SAC_ND_ARG_in (param_nt, ctype));                                 \
        SAC_ND_RET_inout (out_nt, out_nt);                                               \
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
