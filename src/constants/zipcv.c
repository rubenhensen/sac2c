/*
 *
 * $Log$
 * Revision 1.5  2004/11/26 16:09:53  jhb
 * compile
 *
 * Revision 1.4  2004/11/22 18:55:29  cg
 * Moved all definitions/declarations of global variables to globals.mac
 *
 * Revision 1.3  2003/04/09 15:37:16  sbs
 * zipcv_neg added.
 *
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

/******************************************************************************
 *
 * function:
 *    void COzipCvXXXYYY( void *arg1, int pos1, void *arg2, int pos2,
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
 *    void COzipCvDummyYYY( void *arg1, int pos1, void *arg2, int pos2,
 *                        void *res, int res_pos)
 *
 * description:
 *    Finally, we provide a dummy function for each operation YYY which should
 *    never be called! It is defined for being able to make entries for all
 *     simpletypes in type_info.mac!
 *
 ******************************************************************************/

#define str(s) #s

#define COzipCvTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                             \
    void COzipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COzipCv##arg_ext##fun_ext));                                    \
        ((res_t *)res)[res_pos] = ((arg_t *)arg1)[pos1] fun ((arg_t *)arg2)[pos2];       \
        DBUG_VOID_RETURN;                                                                \
    }

#define COzipCvDUMMYTEMP(arg_ext, fun_ext)                                               \
    void COzipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COzipCv##arg_ext##fun_ext));                                    \
        DBUG_ASSERT ((1 == 0), str (COzipCv##arg_ext##fun_ext called !));                \
        DBUG_VOID_RETURN;                                                                \
    }

#define COzipCvMINMAXTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                       \
    void COzipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COzipCv##arg_ext##fun_ext));                                    \
        ((res_t *)res)[res_pos] = (((arg_t *)arg1)[pos1])fun (((arg_t *)arg2)[pos2])     \
                                    ? (((arg_t *)arg1)[pos1])                            \
                                    : (((arg_t *)arg2)[pos2]);                           \
        DBUG_VOID_RETURN;                                                                \
    }

#define COzipCvUNARYTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                        \
    void COzipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COzipCv##arg_ext##fun_ext));                                    \
        ((res_t *)res)[res_pos] = fun (((arg_t *)arg1)[pos1]);                           \
        DBUG_VOID_RETURN;                                                                \
    }

#define COzipCvABSTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                          \
    void COzipCv##arg_ext##fun_ext (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos)                              \
    {                                                                                    \
        DBUG_ENTER (str (COzipCv##arg_ext##fun_ext));                                    \
        ((res_t *)res)[res_pos] = (((arg_t *)arg1)[pos1]) < 0 ? (-((arg_t *)arg1)[pos1]) \
                                                              : (((arg_t *)arg1)[pos1]); \
        DBUG_VOID_RETURN;                                                                \
    }

/* macro expansion for all basetypes - or dummy if not useful */
#define MAP_NUMxNUM_NUM(fun, fname)                                                      \
    COzipCvTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)                 \
      COzipCvTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)                     \
        COzipCvTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)                \
          COzipCvTEMPLATE (fun, fname, short, Short, short)                              \
            COzipCvTEMPLATE (fun, fname, int, Int, int)                                  \
              COzipCvTEMPLATE (fun, fname, long, Long, long)                             \
                COzipCvTEMPLATE (fun, fname, float, Float, float)                        \
                  COzipCvTEMPLATE (fun, fname, double, Double, double)                   \
                    COzipCvTEMPLATE (fun, fname, long double, LongDouble, long double)   \
                      COzipCvDUMMYTEMP (Bool, fname) COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_MINMAX_NUMxNUM_NUM(fun, fname)                                               \
    COzipCvMINMAXTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)           \
      COzipCvMINMAXTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)               \
        COzipCvMINMAXTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)          \
          COzipCvMINMAXTEMPLATE (fun, fname, short, Short, short)                        \
            COzipCvMINMAXTEMPLATE (fun, fname, int, Int, int)                            \
              COzipCvMINMAXTEMPLATE (fun, fname, long, Long, long)                       \
                COzipCvMINMAXTEMPLATE (fun, fname, float, Float, float)                  \
                  COzipCvMINMAXTEMPLATE (fun, fname, double, Double, double)             \
                    COzipCvMINMAXTEMPLATE (fun, fname, long double, LongDouble,          \
                                           long double) COzipCvDUMMYTEMP (Bool, fname)   \
                      COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_INTxINT_INT(fun, fname)                                                      \
    COzipCvTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)                 \
      COzipCvTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)                     \
        COzipCvTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)                \
          COzipCvTEMPLATE (fun, fname, short, Short, short)                              \
            COzipCvTEMPLATE (fun, fname, int, Int, int)                                  \
              COzipCvTEMPLATE (fun, fname, long, Long, long)                             \
                COzipCvDUMMYTEMP (Float, fname) COzipCvDUMMYTEMP (Double, fname)         \
                  COzipCvDUMMYTEMP (LongDouble, fname) COzipCvDUMMYTEMP (Bool, fname)    \
                    COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_BOOLxBOOL_BOOL(fun, fname)                                                   \
    COzipCvDUMMYTEMP (UShort, fname) COzipCvDUMMYTEMP (UInt, fname)                      \
      COzipCvDUMMYTEMP (ULong, fname) COzipCvDUMMYTEMP (Short, fname)                    \
        COzipCvDUMMYTEMP (Int, fname) COzipCvDUMMYTEMP (Long, fname)                     \
          COzipCvDUMMYTEMP (Float, fname) COzipCvDUMMYTEMP (Double, fname)               \
            COzipCvDUMMYTEMP (LongDouble, fname)                                         \
              COzipCvTEMPLATE (fun, fname, bool, Bool, bool)                             \
                COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_ANYxANY_BOOL(fun, fname)                                                     \
    COzipCvTEMPLATE (fun, fname, unsigned short, UShort, bool)                           \
      COzipCvTEMPLATE (fun, fname, unsigned int, UInt, bool)                             \
        COzipCvTEMPLATE (fun, fname, unsigned long, ULong, bool)                         \
          COzipCvTEMPLATE (fun, fname, short, Short, bool)                               \
            COzipCvTEMPLATE (fun, fname, int, Int, bool)                                 \
              COzipCvTEMPLATE (fun, fname, long, Long, bool)                             \
                COzipCvTEMPLATE (fun, fname, float, Float, bool)                         \
                  COzipCvTEMPLATE (fun, fname, double, Double, bool)                     \
                    COzipCvTEMPLATE (fun, fname, long double, LongDouble, bool)          \
                      COzipCvTEMPLATE (fun, fname, bool, Bool, bool)                     \
                        COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_NUMxNUM_BOOL(fun, fname)                                                     \
    COzipCvTEMPLATE (fun, fname, unsigned short, UShort, bool)                           \
      COzipCvTEMPLATE (fun, fname, unsigned int, UInt, bool)                             \
        COzipCvTEMPLATE (fun, fname, unsigned long, ULong, bool)                         \
          COzipCvTEMPLATE (fun, fname, short, Short, bool)                               \
            COzipCvTEMPLATE (fun, fname, int, Int, bool)                                 \
              COzipCvTEMPLATE (fun, fname, long, Long, bool)                             \
                COzipCvTEMPLATE (fun, fname, float, Float, bool)                         \
                  COzipCvTEMPLATE (fun, fname, double, Double, bool)                     \
                    COzipCvTEMPLATE (fun, fname, long double, LongDouble, bool)          \
                      COzipCvDUMMYTEMP (Bool, fname) COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_BOOL_BOOL(fun, fname)                                                        \
    COzipCvDUMMYTEMP (UShort, fname) COzipCvDUMMYTEMP (UInt, fname)                      \
      COzipCvDUMMYTEMP (ULong, fname) COzipCvDUMMYTEMP (Short, fname)                    \
        COzipCvDUMMYTEMP (Int, fname) COzipCvDUMMYTEMP (Long, fname)                     \
          COzipCvDUMMYTEMP (Float, fname) COzipCvDUMMYTEMP (Double, fname)               \
            COzipCvDUMMYTEMP (LongDouble, fname)                                         \
              COzipCvUNARYTEMPLATE (fun, fname, bool, Bool, bool)                        \
                COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_NUM_TYPE(fun, fname, target_t)                                               \
    COzipCvUNARYTEMPLATE (fun, fname, unsigned short, UShort, target_t)                  \
      COzipCvUNARYTEMPLATE (fun, fname, unsigned int, UInt, target_t)                    \
        COzipCvUNARYTEMPLATE (fun, fname, unsigned long, ULong, target_t)                \
          COzipCvUNARYTEMPLATE (fun, fname, short, Short, target_t)                      \
            COzipCvUNARYTEMPLATE (fun, fname, int, Int, target_t)                        \
              COzipCvUNARYTEMPLATE (fun, fname, long, Long, target_t)                    \
                COzipCvUNARYTEMPLATE (fun, fname, float, Float, target_t)                \
                  COzipCvUNARYTEMPLATE (fun, fname, double, Double, target_t)            \
                    COzipCvUNARYTEMPLATE (fun, fname, long double, LongDouble, target_t) \
                      COzipCvDUMMYTEMP (Bool, fname) COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_NUM_NUM(fun, fname)                                                          \
    COzipCvUNARYTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)            \
      COzipCvUNARYTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)                \
        COzipCvUNARYTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)           \
          COzipCvUNARYTEMPLATE (fun, fname, short, Short, short)                         \
            COzipCvUNARYTEMPLATE (fun, fname, int, Int, int)                             \
              COzipCvUNARYTEMPLATE (fun, fname, long, Long, long)                        \
                COzipCvUNARYTEMPLATE (fun, fname, float, Float, float)                   \
                  COzipCvUNARYTEMPLATE (fun, fname, double, Double, double)              \
                    COzipCvUNARYTEMPLATE (fun, fname, long double, LongDouble,           \
                                          long double) COzipCvDUMMYTEMP (Bool, fname)    \
                      COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_ABS_NUM_NUM(fun, fname)                                                      \
    COzipCvUNARYTEMPLATE (, fname, unsigned short, UShort, unsigned short)               \
      COzipCvUNARYTEMPLATE (, fname, unsigned int, UInt, unsigned int)                   \
        COzipCvUNARYTEMPLATE (, fname, unsigned long, ULong, unsigned long)              \
          COzipCvABSTEMPLATE (fun, fname, short, Short,                                  \
                              short) COzipCvABSTEMPLATE (fun, fname, int, Int, int)      \
            COzipCvABSTEMPLATE (fun, fname, long, Long, long)                            \
              COzipCvABSTEMPLATE (fun, fname, float, Float, float)                       \
                COzipCvABSTEMPLATE (fun, fname, double, Double, double)                  \
                  COzipCvABSTEMPLATE (fun, fname, long double, LongDouble, long double)  \
                    COzipCvDUMMYTEMP (Bool, fname) COzipCvDUMMYTEMP (Dummy, fname)
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

                                        MAP_NUM_NUM (-, Neg)
