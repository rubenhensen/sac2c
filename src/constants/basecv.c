/*
 *
 * $Log$
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
#include "dbug.h"
#include "types.h"
#include "shape.h"
#include "constants.h"
#include "basecv.h"
#include "internal_lib.h"

#define TYP_IFbasecv(fun) fun##Zero
basecvfunptr basecv_zero[] = {
#include "type_info.mac"
};

#define TYP_IFbasecv(fun) fun##One
basecvfunptr basecv_one[] = {
#include "type_info.mac"
};

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

#define COBaseCvTEMPLATE(fun_ext, s_type, arg_t, arg_ext, value)                         \
    constant *COBaseCv##arg_ext##fun_ext (shape *shp)                                    \
    {                                                                                    \
        int i;                                                                           \
        int unrlen;                                                                      \
        arg_t *data_vec;                                                                 \
        DBUG_ENTER (str (COBaseCv##arg_ext##fun_ext));                                   \
        unrlen = SHGetUnrLen (shp);                                                      \
        data_vec = (arg_t *)Malloc (unrlen * sizeof (arg_t));                            \
        for (i = 0; i < unrlen; i++)                                                     \
            data_vec[i] = value;                                                         \
        DBUG_RETURN (COMakeConstant (s_type, shp, (void *)data_vec));                    \
    }

#define COBaseCvDUMMYTEMP(arg_ext, fun_ext)                                              \
    constant *COBaseCv##arg_ext##fun_ext (shape *shp)                                    \
    {                                                                                    \
        DBUG_ENTER (str (COBaseCv##arg_ext##fun_ext));                                   \
        DBUG_RETURN ((constant *)NULL);                                                  \
    }

#define MAP(fname, const)                                                                \
    COBaseCvTEMPLATE (fname, T_ushort, unsigned short, UShort, const)                    \
      COBaseCvTEMPLATE (fname, T_uint, unsigned int, UInt, const)                        \
        COBaseCvTEMPLATE (fname, T_ulong, unsigned long, ULong, const)                   \
          COBaseCvTEMPLATE (fname, T_short, short, Short, const)                         \
            COBaseCvTEMPLATE (fname, T_int, int, Int, const)                             \
              COBaseCvTEMPLATE (fname, T_long, long, Long, const)                        \
                COBaseCvTEMPLATE (fname, T_float, float, Float, const)                   \
                  COBaseCvTEMPLATE (fname, T_double, double, Double, const)              \
                    COBaseCvTEMPLATE (fname, T_longdbl, long double, LongDouble, const)  \
                      COBaseCvDUMMYTEMP (Dummy, fname)

MAP (Zero, 0)
MAP (One, 1)

/* special versions for Boolean (Zero == False, One == True) */
COBaseCvTEMPLATE (Zero, T_bool, bool, Bool, FALSE)
  COBaseCvTEMPLATE (One, T_bool, bool, Bool, TRUE)
