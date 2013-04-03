/*
 *
 * $Log$
 * Revision 1.4  2004/11/26 16:09:53  jhb
 * compile
 *
 * Revision 1.3  2004/11/22 18:55:29  cg
 * Moved all definitions/declarations of global variables to globals.mac
 *
 * Revision 1.2  2001/05/17 14:16:21  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 1.1  2001/05/02 08:00:21  nmw
 * Initial revision
 *
 *
 */

/*
 * This module provides a set of functions for generating base constantants like
 * Zero/One/True/False.
 * The problem with this task is that each such function has to be overloaded on
 * all potential types.
 * Since C does not support such specifications, a function table is created
 * which is parameterized over simpletype.
 * This is done by using the    .mac-mechanism     on type_info.mac in order
 * to make sure that all simpletypes are dealt with.
 */

#include <strings.h>
#include <stdlib.h>

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "types.h"
#include "shape.h"
#include "constants.h"
#include "basecv.h"
#include "str.h"
#include "memory.h"

/******************************************************************************
 *
 * function:
 *    void COBaseCvXXXXYYY( shape shp*)
 *
 * description:
 *    Functions to create BaseConstants filled with Zero/False or One/True.
 *
 *
 * function:
 *    void COBaseCvDummyYYY( shape *shp)
 *
 * description:
 *    Finally, we provide a dummy function for types where we cannot construct
 *    fixed values.
 *
 ******************************************************************************/

#define str(s) #s

#define CObaseCvTEMPLATE(fun_ext, s_type, arg_t, arg_ext, value)                         \
    constant *CObaseCv##arg_ext##fun_ext (shape *shp)                                    \
    {                                                                                    \
        int i;                                                                           \
        int unrlen;                                                                      \
        arg_t *data_vec;                                                                 \
        DBUG_ENTER ();                                                                   \
        unrlen = SHgetUnrLen (shp);                                                      \
        data_vec = (arg_t *)MEMmalloc (unrlen * sizeof (arg_t));                         \
        for (i = 0; i < unrlen; i++)                                                     \
            data_vec[i] = value;                                                         \
        DBUG_RETURN (COmakeConstant (s_type, shp, (void *)data_vec));                    \
    }

#define CObaseCvDUMMYTEMP(arg_ext, fun_ext)                                              \
    constant *CObaseCv##arg_ext##fun_ext (shape *shp)                                    \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        DBUG_RETURN ((constant *)NULL);                                                  \
    }

#define MAP(fname, const)                                                                \
    CObaseCvTEMPLATE (fname, T_ubyte, unsigned char, UByte, const)                       \
      CObaseCvTEMPLATE (fname, T_ushort, unsigned short, UShort, const)                  \
        CObaseCvTEMPLATE (fname, T_uint, unsigned int, UInt, const)                      \
          CObaseCvTEMPLATE (fname, T_ulong, unsigned long, ULong, const)                 \
            CObaseCvTEMPLATE (fname, T_ulonglong, unsigned long long, ULongLong, const)  \
              CObaseCvTEMPLATE (fname, T_byte, char, Byte, const)                        \
                CObaseCvTEMPLATE (fname, T_char, unsigned char, Char, const)             \
                  CObaseCvTEMPLATE (fname, T_short, short, Short, const)                 \
                    CObaseCvTEMPLATE (fname, T_int, int, Int, const)                     \
                      CObaseCvTEMPLATE (fname, T_long, long, Long, const)                \
                        CObaseCvTEMPLATE (fname, T_longlong, long long, LongLong, const) \
                          CObaseCvTEMPLATE (fname, T_float, float, Float, const)         \
                            CObaseCvTEMPLATE (fname, T_double, double, Double, const)    \
                              CObaseCvTEMPLATE (fname, T_longdbl, long double,           \
                                                LongDouble, const)                       \
                                CObaseCvDUMMYTEMP (Dummy, fname)

MAP (NegativeOne, -1)
MAP (Zero, 0)
MAP (One, 1)

/* special versions for Boolean (Zero == False, One == True) */
CObaseCvTEMPLATE (NegativeOne, T_bool, bool, Bool, FALSE) /* This is a cheat */
  CObaseCvTEMPLATE (Zero, T_bool, bool, Bool, FALSE)
    CObaseCvTEMPLATE (One, T_bool, bool, Bool, TRUE)

#undef DBUG_PREFIX
