/*
 *
 * $Log$
 * Revision 1.2  2001/03/22 14:28:07  nmw
 * macros and function tables for primitive ari functions added
 *
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
#include "types.h"

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

#define TYP_IFzipcv(fun) fun##Mod
zipcvfunptr zipcv_mod[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Min
zipcvfunptr zipcv_min[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Max
zipcvfunptr zipcv_max[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##And
zipcvfunptr zipcv_and[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Or
zipcvfunptr zipcv_or[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Eq
zipcvfunptr zipcv_eq[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Neq
zipcvfunptr zipcv_neq[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Le
zipcvfunptr zipcv_le[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Lt
zipcvfunptr zipcv_lt[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Gt
zipcvfunptr zipcv_gt[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Ge
zipcvfunptr zipcv_ge[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Not
zipcvfunptr zipcv_not[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Toi
zipcvfunptr zipcv_toi[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Tof
zipcvfunptr zipcv_tof[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Tod
zipcvfunptr zipcv_tod[] = {
#include "type_info.mac"
};

#define TYP_IFzipcv(fun) fun##Abs
zipcvfunptr zipcv_abs[] = {
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

#define COZipCvDUMMYTEMP(arg_ext, fun_ext)                                               \
    void COZipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COZipCv##arg_ext##fun_ext));                                    \
        DBUG_ASSERT ((1 == 0), str (COZipCv##arg_ext##fun_ext called !));                \
        DBUG_VOID_RETURN;                                                                \
    }

#define COZipCvMINMAXTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                       \
    void COZipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COZipCv##arg_ext##fun_ext));                                    \
        ((res_t *)res)[res_pos] = (((arg_t *)arg1)[pos1])fun (((arg_t *)arg2)[pos2])     \
                                    ? (((arg_t *)arg1)[pos1])                            \
                                    : (((arg_t *)arg2)[pos2]);                           \
        DBUG_VOID_RETURN;                                                                \
    }

#define COZipCvUNARYTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                        \
    void COZipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COZipCv##arg_ext##fun_ext));                                    \
        ((res_t *)res)[res_pos] = fun (((arg_t *)arg1)[pos1]);                           \
        DBUG_VOID_RETURN;                                                                \
    }

#define COZipCvABSTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                          \
    void COZipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COZipCv##arg_ext##fun_ext));                                    \
        ((res_t *)res)[res_pos] = (((arg_t *)arg1)[pos1]) < 0 ? (-((arg_t *)arg1)[pos1]) \
                                                              : (((arg_t *)arg1)[pos1]); \
        DBUG_VOID_RETURN;                                                                \
    }

/* macro expansion for all basetypes - or dummy if not useful */
#define MAP_NUMxNUM_NUM(fun, fname)                                                      \
    COZipCvTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)                 \
      COZipCvTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)                     \
        COZipCvTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)                \
          COZipCvTEMPLATE (fun, fname, short, Short, short)                              \
            COZipCvTEMPLATE (fun, fname, int, Int, int)                                  \
              COZipCvTEMPLATE (fun, fname, long, Long, long)                             \
                COZipCvTEMPLATE (fun, fname, float, Float, float)                        \
                  COZipCvTEMPLATE (fun, fname, double, Double, double)                   \
                    COZipCvTEMPLATE (fun, fname, long double, LongDouble, long double)   \
                      COZipCvDUMMYTEMP (Bool, fname) COZipCvDUMMYTEMP (Dummy, fname)

#define MAP_MINMAX_NUMxNUM_NUM(fun, fname)                                               \
    COZipCvMINMAXTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)           \
      COZipCvMINMAXTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)               \
        COZipCvMINMAXTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)          \
          COZipCvMINMAXTEMPLATE (fun, fname, short, Short, short)                        \
            COZipCvMINMAXTEMPLATE (fun, fname, int, Int, int)                            \
              COZipCvMINMAXTEMPLATE (fun, fname, long, Long, long)                       \
                COZipCvMINMAXTEMPLATE (fun, fname, float, Float, float)                  \
                  COZipCvMINMAXTEMPLATE (fun, fname, double, Double, double)             \
                    COZipCvMINMAXTEMPLATE (fun, fname, long double, LongDouble,          \
                                           long double) COZipCvDUMMYTEMP (Bool, fname)   \
                      COZipCvDUMMYTEMP (Dummy, fname)

#define MAP_INTxINT_INT(fun, fname)                                                      \
    COZipCvTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)                 \
      COZipCvTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)                     \
        COZipCvTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)                \
          COZipCvTEMPLATE (fun, fname, short, Short, short)                              \
            COZipCvTEMPLATE (fun, fname, int, Int, int)                                  \
              COZipCvTEMPLATE (fun, fname, long, Long, long)                             \
                COZipCvDUMMYTEMP (Float, fname) COZipCvDUMMYTEMP (Double, fname)         \
                  COZipCvDUMMYTEMP (LongDouble, fname) COZipCvDUMMYTEMP (Bool, fname)    \
                    COZipCvDUMMYTEMP (Dummy, fname)

#define MAP_BOOLxBOOL_BOOL(fun, fname)                                                   \
    COZipCvDUMMYTEMP (UShort, fname) COZipCvDUMMYTEMP (UInt, fname)                      \
      COZipCvDUMMYTEMP (ULong, fname) COZipCvDUMMYTEMP (Short, fname)                    \
        COZipCvDUMMYTEMP (Int, fname) COZipCvDUMMYTEMP (Long, fname)                     \
          COZipCvDUMMYTEMP (Float, fname) COZipCvDUMMYTEMP (Double, fname)               \
            COZipCvDUMMYTEMP (LongDouble, fname)                                         \
              COZipCvTEMPLATE (fun, fname, bool, Bool, bool)                             \
                COZipCvDUMMYTEMP (Dummy, fname)

#define MAP_ANYxANY_BOOL(fun, fname)                                                     \
    COZipCvTEMPLATE (fun, fname, unsigned short, UShort, bool)                           \
      COZipCvTEMPLATE (fun, fname, unsigned int, UInt, bool)                             \
        COZipCvTEMPLATE (fun, fname, unsigned long, ULong, bool)                         \
          COZipCvTEMPLATE (fun, fname, short, Short, bool)                               \
            COZipCvTEMPLATE (fun, fname, int, Int, bool)                                 \
              COZipCvTEMPLATE (fun, fname, long, Long, bool)                             \
                COZipCvTEMPLATE (fun, fname, float, Float, bool)                         \
                  COZipCvTEMPLATE (fun, fname, double, Double, bool)                     \
                    COZipCvTEMPLATE (fun, fname, long double, LongDouble, bool)          \
                      COZipCvTEMPLATE (fun, fname, bool, Bool, bool)                     \
                        COZipCvDUMMYTEMP (Dummy, fname)

#define MAP_NUMxNUM_BOOL(fun, fname)                                                     \
    COZipCvTEMPLATE (fun, fname, unsigned short, UShort, bool)                           \
      COZipCvTEMPLATE (fun, fname, unsigned int, UInt, bool)                             \
        COZipCvTEMPLATE (fun, fname, unsigned long, ULong, bool)                         \
          COZipCvTEMPLATE (fun, fname, short, Short, bool)                               \
            COZipCvTEMPLATE (fun, fname, int, Int, bool)                                 \
              COZipCvTEMPLATE (fun, fname, long, Long, bool)                             \
                COZipCvTEMPLATE (fun, fname, float, Float, bool)                         \
                  COZipCvTEMPLATE (fun, fname, double, Double, bool)                     \
                    COZipCvTEMPLATE (fun, fname, long double, LongDouble, bool)          \
                      COZipCvDUMMYTEMP (Bool, fname) COZipCvDUMMYTEMP (Dummy, fname)

#define MAP_BOOL_BOOL(fun, fname)                                                        \
    COZipCvDUMMYTEMP (UShort, fname) COZipCvDUMMYTEMP (UInt, fname)                      \
      COZipCvDUMMYTEMP (ULong, fname) COZipCvDUMMYTEMP (Short, fname)                    \
        COZipCvDUMMYTEMP (Int, fname) COZipCvDUMMYTEMP (Long, fname)                     \
          COZipCvDUMMYTEMP (Float, fname) COZipCvDUMMYTEMP (Double, fname)               \
            COZipCvDUMMYTEMP (LongDouble, fname)                                         \
              COZipCvUNARYTEMPLATE (fun, fname, bool, Bool, bool)                        \
                COZipCvDUMMYTEMP (Dummy, fname)

#define MAP_NUM_TYPE(fun, fname, target_t)                                               \
    COZipCvUNARYTEMPLATE (fun, fname, unsigned short, UShort, target_t)                  \
      COZipCvUNARYTEMPLATE (fun, fname, unsigned int, UInt, target_t)                    \
        COZipCvUNARYTEMPLATE (fun, fname, unsigned long, ULong, target_t)                \
          COZipCvUNARYTEMPLATE (fun, fname, short, Short, target_t)                      \
            COZipCvUNARYTEMPLATE (fun, fname, int, Int, target_t)                        \
              COZipCvUNARYTEMPLATE (fun, fname, long, Long, target_t)                    \
                COZipCvUNARYTEMPLATE (fun, fname, float, Float, target_t)                \
                  COZipCvUNARYTEMPLATE (fun, fname, double, Double, target_t)            \
                    COZipCvUNARYTEMPLATE (fun, fname, long double, LongDouble, target_t) \
                      COZipCvDUMMYTEMP (Bool, fname) COZipCvDUMMYTEMP (Dummy, fname)

#define MAP_ABS_NUM_NUM(fun, fname)                                                      \
    COZipCvUNARYTEMPLATE (, fname, unsigned short, UShort, unsigned short)               \
      COZipCvUNARYTEMPLATE (, fname, unsigned int, UInt, unsigned int)                   \
        COZipCvUNARYTEMPLATE (, fname, unsigned long, ULong, unsigned long)              \
          COZipCvABSTEMPLATE (fun, fname, short, Short,                                  \
                              short) COZipCvABSTEMPLATE (fun, fname, int, Int, int)      \
            COZipCvABSTEMPLATE (fun, fname, long, Long, long)                            \
              COZipCvABSTEMPLATE (fun, fname, float, Float, float)                       \
                COZipCvABSTEMPLATE (fun, fname, double, Double, double)                  \
                  COZipCvABSTEMPLATE (fun, fname, long double, LongDouble, long double)  \
                    COZipCvDUMMYTEMP (Bool, fname) COZipCvDUMMYTEMP (Dummy, fname)
/*
 * The actual function definitions are defined by the following macro usages:
 */

MAP_NUMxNUM_NUM (+, Plus)

  MAP_NUMxNUM_NUM (-, Minus)

    MAP_NUMxNUM_NUM (*, Mul)

      MAP_NUMxNUM_NUM (/, Div)

        MAP_INTxINT_INT (%, Mod)

          MAP_MINMAX_NUMxNUM_NUM (<, Min)

            MAP_MINMAX_NUMxNUM_NUM (>, Max)

              MAP_BOOLxBOOL_BOOL (&&, And)

                MAP_BOOLxBOOL_BOOL (||, Or)

                  MAP_ANYxANY_BOOL (==, Eq)

                    MAP_ANYxANY_BOOL (!=, Neq)

                      MAP_NUMxNUM_BOOL (<=, Le)

                        MAP_NUMxNUM_BOOL (<, Lt)

                          MAP_NUMxNUM_BOOL (>, Gt)

                            MAP_NUMxNUM_BOOL (>=, Ge)

                              MAP_BOOL_BOOL (!, Not)

                                MAP_NUM_TYPE ((int), Toi, int)

                                  MAP_NUM_TYPE ((float), Tof, float)

                                    MAP_NUM_TYPE ((double), Tod, double)

                                      MAP_ABS_NUM_NUM (NOP, Abs)
