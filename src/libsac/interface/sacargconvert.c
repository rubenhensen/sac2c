/*
 * This library is only used from C programs.
 * It solely provides conversion functions between void * pointers
 * and SACarg objects. The interface only exists as part of 
 * sacinterface.h which lives in sac2c/include !
 */

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#include "sacinterface.h" // serves as superset of the own interface
#include "runtime/essentials_h/types.h"   // byte,...
#include "libsac/essentials/message.h"    // SAC_RuntimeError,...

#define INCLUDED_FROM_LIBSAC
#include "sacarg.h"                       // SACARGextractData, ...
#undef INCLUDED_FROM_LIBSAC

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
        data = (ctype *)SAC_MALLOC (sizeof (ctype));                                     \
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

#ifndef SAC_BACKEND_MUTC
CONVERTER (Byte, T_byte, byte)
CONVERTER (Short, T_short, short)
CONVERTER (Int, T_int, int)
CONVERTER (Long, T_long, long)
CONVERTER (Longlong, T_longlong, longlong)
CONVERTER (Ubyte, T_ubyte, ubyte)
CONVERTER (Ushort, T_ushort, ushort)
CONVERTER (Uint, T_uint, uint)
CONVERTER (Ulong, T_ulong, ulong)
CONVERTER (Ulonglong, T_ulonglong, ulonglong)
CONVERTER (Double, T_double, double)
CONVERTER (Float, T_float, float)
CONVERTER (Bool, T_bool, int)
CONVERTER (Char, T_char, char)
#endif /* SAC_BACKEND_MUTC */

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

    result
      = SACARGmakeSacArg ((basetype)btype, SACARGmakeDescriptorVect (0, (int *)0), data);

    return (result);
}
