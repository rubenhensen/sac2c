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

SACARG2C (Int, T_int, int)
SACARG2C (Double, T_double, double)
SACARG2C (Float, T_float, float)
SACARG2C (Bool, T_bool, int)
SACARG2C (Char, T_char, char)

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

SACARGFROMC (Int, T_int, int)
SACARGFROMC (Double, T_double, double)
SACARGFROMC (Float, T_float, float)
SACARGFROMC (Bool, T_bool, int)
SACARGFROMC (Char, T_char, char)

SACARGFROMCVECT (Int, T_int, int)
SACARGFROMCVECT (Double, T_double, double)
SACARGFROMCVECT (Float, T_float, float)
SACARGFROMCVECT (Bool, T_bool, int)
SACARGFROMCVECT (Char, T_char, char)
