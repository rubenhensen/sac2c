/*
 *
 * $Log$
 * Revision 1.1  2001/03/05 16:59:14  sbs
 * Initial revision
 *
 *
 */

/*
 * This module provides a set of functions for applying arithmetic functions
 * two two elements of constant vectors.
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

#include "zipcv.h"

#define TYP_IFzipcv(fun) fun##Plus
zipcvfunptr zipcv_plus[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Minus
zipcvfunptr zipcv_minus[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Mul
zipcvfunptr zipcv_mul[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Div
zipcvfunptr zipcv_div[] = {
#include "type_info.mac"
};

/******************************************************************************
 *
 * function:
 *    void COZipCvXXXYYY( void *arg1, int pos1, void *arg2, int pos2,
 *                        void *res, int res_pos)
 *
 * description:
 *    Functions for applying function YYY to two elements of cv's arg1 and arg2
 *    each of which are of type XXX.
 *    Since all these functions are identical apart from the type casts,
 *    they are implemented using the C macro mechanism.
 *
 *
 * function:
 *    void COZipCvDummyYYY( void *arg1, int pos1, void *arg2, int pos2,
 *                        void *res, int res_pos)
 *
 * description:
 *    Finally, we provide a dummy function for each operation YYY which should
 *    never be called! It is defined for being able to make entries for all
 *     simpletypes in type_info.mac!
 *
 ******************************************************************************/

#define str(s) #s

#define COZipCvTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                             \
    void COZipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COZipCv##arg_ext##fun_ext));                                    \
        ((res_t *)res)[res_pos] = ((arg_t *)arg1)[pos1] fun ((arg_t *)arg2)[pos2];       \
        DBUG_VOID_RETURN;                                                                \
    }

#define MAP(fun, fname)                                                                  \
    COZipCvTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)                 \
      COZipCvTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)                     \
        COZipCvTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)                \
          COZipCvTEMPLATE (fun, fname, short, Short, short)                              \
            COZipCvTEMPLATE (fun, fname, int, Int, int)                                  \
              COZipCvTEMPLATE (fun, fname, long, Long, long)                             \
                COZipCvTEMPLATE (fun, fname, float, Float, float)                        \
                  COZipCvTEMPLATE (fun, fname, double, Double, double)                   \
                    COZipCvTEMPLATE (fun, fname, long double, LongDouble, long double)   \
                                                                                         \
                      void COZipCvDummy##fname (void *arg1, int pos1, void *arg2,        \
                                                int pos2, void *res, int res_pos)        \
    {                                                                                    \
        DBUG_ENTER (str (COZipCvDummy##fname));                                          \
        DBUG_ASSERT ((1 == 0), str (COZipCvDummy##fname called !));                      \
        DBUG_VOID_RETURN;                                                                \
    }

/*
 * The actual function definitions are defined by the following macro usages:
 */

MAP (+, Plus)

MAP (-, Minus)

MAP (*, Mul)

MAP (/, Div)
