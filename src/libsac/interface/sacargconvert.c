/* $Id$ */

#include "sacinterface.h"
#include "sac.h"
#include "sacarg.h"

/**
 * conversion functions SACarg -> C
 */

#define SACARG2C(name, btype, ctype)                                                     \
    ctype *SACARGconvertTo##name##Array (SACarg *arg)                                    \
    {                                                                                    \
        ctype *result = (ctype *)0;                                                      \
                                                                                         \
        if (SACARGgetBasetype (arg) != btype) {                                          \
            SAC_RuntimeError ("Types do not match in conversion!");                      \
        } else {                                                                         \
            result = (ctype *)SACARGextractData (arg);                                   \
        }                                                                                \
                                                                                         \
        return (result);                                                                 \
    }

#define SACARGFROMC(name, btype, ctype)                                                  \
    SACarg *SACARGconvertFrom##name##Pointer (ctype *data, int dim, ...)                 \
    {                                                                                    \
        SACarg *result;                                                                  \
        va_list argp;                                                                    \
                                                                                         \
        va_start (argp, dim);                                                            \
        result = SACARGmakeSacArg (btype, SACARGmakeDescriptor (dim, argp), data);       \
        va_end (argp);                                                                   \
                                                                                         \
        return (result);                                                                 \
    }

#define SACARGFROMCVECT(name, btype, ctype)                                              \
    SACarg *SACARGconvertFrom##name##PointerVect (ctype *data, int dim, int *shape)      \
    {                                                                                    \
        SACarg *result;                                                                  \
                                                                                         \
        result = SACARGmakeSacArg (btype, SACARGmakeDescriptorVect (dim, shape), data);  \
                                                                                         \
        return (result);                                                                 \
    }

#define SACARGFROMCSCALAR(name, btype, ctype)                                            \
    SACarg *SACARGconvertFrom##name##Scalar (ctype value)                                \
    {                                                                                    \
        ctype *data;                                                                     \
        SACarg *result;                                                                  \
                                                                                         \
        data = SAC_MALLOC (sizeof (ctype));                                              \
        *data = value;                                                                   \
                                                                                         \
        result = SACARGconvertFrom##name##Pointer (data, 0);                             \
                                                                                         \
        return (result);                                                                 \
    }

#define CONVERTER(name, btype, ctype)                                                    \
    SACARG2C (name, btype, ctype)                                                        \
    SACARGFROMC (name, btype, ctype)                                                     \
    SACARGFROMCVECT (name, btype, ctype)                                                 \
    SACARGFROMCSCALAR (name, btype, ctype)

CONVERTER (Int, T_int, int)
CONVERTER (Double, T_double, double)
CONVERTER (Float, T_float, float)
CONVERTER (Bool, T_bool, int)
CONVERTER (Char, T_char, char)

void *
SACARGconvertToVoidPointer (int btype, SACarg *arg)
{
    void *result = (void *)0;

    if (SACARGgetBasetype (arg) != btype) {
        SAC_RuntimeError ("Types do not match in conversion!");
    } else {
        result = (void *)SACARGextractData (arg);
    }

    return (result);
}

SACarg *
SACARGconvertFromVoidPointer (int btype, void *data)
{
    SACarg *result;

    result = SACARGmakeSacArg (btype, SACARGmakeDescriptorVect (0, (int *)0), data);

    return (result);
}
