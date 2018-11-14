/*
 * This module provides a set of functions for applying arithmetic functions
 * to two elements of constant vectors.
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

#include "zipcv.h"

/******************************************************************************
 *
 * function:
 *    void COzipCvXXXYYY( void *arg1, size_t pos1, void *arg2, size_t pos2,
 *                        void *res, size_t res_pos)
 *
 * description:
 *    Functions for applying function YYY to two elements of cv's arg1 and arg2
 *    each of which are of type XXX.
 *    Since all these functions are identical apart from the type casts,
 *    they are implemented using the C macro mechanism.
 *
 *
 * function:
 *    void COzipCvDummyYYY( void *arg1, size_t pos1, void *arg2, size_t pos2,
 *                        void *res, size_t res_pos)
 *
 * description:
 *    Finally, we provide a dummy function for each operation YYY which should
 *    never be called! It is defined for being able to make entries for all
 *     simpletypes in type_info.mac!
 *
 ******************************************************************************/

#define str(s) #s

#define COzipCvTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                             \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        ((res_t *)res)[res_pos] = (res_t) (((arg_t *)arg1)[pos1] fun                     \
                                           ((arg_t *)arg2)[pos2]);                       \
        DBUG_RETURN ();                                                                  \
    }

#define COzipCvDUMMYTEMP(arg_ext, fun_ext)                                               \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        DBUG_UNREACHABLE (str (COzipCv##arg_ext##fun_ext called !));                     \
        DBUG_RETURN ();                                                                  \
    }

#define COzipCvMINMAXTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                       \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        ((res_t *)res)[res_pos]                                                          \
          = (res_t)((((arg_t *)arg1)[pos1])fun (((arg_t *)arg2)[pos2])                   \
            ? (((arg_t *)arg1)[pos1])                                                    \
            : (((arg_t *)arg2)[pos2]));                                                  \
        DBUG_RETURN ();                                                                  \
    }

#define COzipCvUNARYTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                        \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        ((res_t *)res)[res_pos] = (res_t) fun (((arg_t *)arg1)[pos1]);                   \
        DBUG_RETURN ();                                                                  \
    }

#define SIGNUM(x) ((0 == (x)) ? 0 : (0 < (x)) ? 1 : -1)

/* Ensure that the code below and the code in runtime/essentials/prf.h match! */

#define COzipCvAPLMODTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                       \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        arg_t x;                                                                         \
        arg_t y;                                                                         \
        arg_t z;                                                                         \
        DBUG_ENTER ();                                                                   \
        x = ((arg_t *)arg1)[pos1];                                                       \
        y = ((arg_t *)arg2)[pos2];                                                       \
        z = (arg_t)((0 == y) ? x : x - (y * (x / y)));                                   \
        if ((0 != z) && (SIGNUM (x) != SIGNUM (y))) {                                    \
            z = (arg_t)(z + y);                                                          \
        }                                                                                \
        ((res_t *)res)[res_pos] = (arg_t)z;                                              \
        DBUG_RETURN ();                                                                  \
    }

#define COzipCvABSTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                          \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        ((res_t *)res)[res_pos] = (res_t)(((arg_t *)arg1)[pos1] < 0                      \
                                          ? (-((arg_t *)arg1)[pos1])                     \
                                          : ((arg_t *)arg1)[pos1]);                      \
        DBUG_RETURN ();                                                                  \
    }

#define COzipCvTOBOOLTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                       \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        if (0 == (((arg_t *)arg1)[pos1])) {                                              \
            ((res_t *)res)[res_pos] = 0;                                                 \
        } else {                                                                         \
            if (1 == (((arg_t *)arg1)[pos1])) {                                          \
                ((res_t *)res)[res_pos] = 1;                                             \
            } else {                                                                     \
                /* FIXME. What is wrong here?  DBUG_ASSERT( 1==0, "tobool() given        \
                 * non-Boolean-valued argument")  */                                     \
                ((res_t *)res)[res_pos] = 0;                                             \
            }                                                                            \
        }                                                                                \
        DBUG_RETURN ();                                                                  \
    }

#if 0

/*
 * The following macro is not yet used but might prove useful in the future:
 */
#define COzipCvTOCTEMPLATE(fun, fun_ext, arg_t, arg_ext, res_t)                          \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
                                                                                         \
        DBUG_ASSERT (0 <= (((arg_t *)arg1)[pos1]) "toc() given negative argument")       \
        DBUG_ASSERT (256 > (((arg_t *)arg1)[pos1]) "toc() given argument > 255")         \
        ((res_t *)res)[res_pos] = (res_t) ((arg_t *)arg1[pos1]);                         \
        DBUG_RETURN ();                                                                  \
    }
#endif

/* macro expansion for all basetypes - or dummy if not useful */
#define MAP_NUMxNUM_NUM(fun, fname)                                                      \
    COzipCvTEMPLATE (fun, fname, unsigned char, UByte, unsigned char)                    \
      COzipCvTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)               \
        COzipCvTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)                   \
          COzipCvTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)              \
            COzipCvTEMPLATE (fun, fname, unsigned long long, ULongLong,                  \
                             unsigned long long)                                         \
              COzipCvTEMPLATE (fun, fname, signed char, Byte, signed char)               \
                COzipCvTEMPLATE (fun, fname, short, Short, short)                        \
                  COzipCvTEMPLATE (fun, fname, int, Int, int)                            \
                    COzipCvTEMPLATE (fun, fname, long, Long, long)                       \
                      COzipCvTEMPLATE (fun, fname, long long, LongLong, long long)       \
                        COzipCvTEMPLATE (fun, fname, float, Float, float)                \
                          COzipCvTEMPLATE (fun, fname, double, Double, double)           \
                            COzipCvTEMPLATE (fun, fname, long double, LongDouble,        \
                                             long double) COzipCvDUMMYTEMP (Bool, fname) \
                              COzipCvDUMMYTEMP (Char, fname)                             \
                                COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_MINMAX_NUMxNUM_NUM(fun, fname)                                               \
    COzipCvMINMAXTEMPLATE (fun, fname, unsigned char, UByte, unsigned char)              \
      COzipCvMINMAXTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)         \
        COzipCvMINMAXTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)             \
          COzipCvMINMAXTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)        \
            COzipCvMINMAXTEMPLATE (fun, fname, unsigned long long, ULongLong,            \
                                   unsigned long long)                                   \
              COzipCvMINMAXTEMPLATE (fun, fname, signed char, Byte, signed char)         \
                COzipCvMINMAXTEMPLATE (fun, fname, short, Short, short)                  \
                  COzipCvMINMAXTEMPLATE (fun, fname, int, Int, int)                      \
                    COzipCvMINMAXTEMPLATE (fun, fname, long, Long, long)                 \
                      COzipCvMINMAXTEMPLATE (fun, fname, long long, LongLong, long long) \
                        COzipCvMINMAXTEMPLATE (fun, fname, float, Float, float)          \
                          COzipCvMINMAXTEMPLATE (fun, fname, double, Double, double)     \
                            COzipCvMINMAXTEMPLATE (fun, fname, long double, LongDouble,  \
                                                   long double)                          \
                              COzipCvDUMMYTEMP (Bool, fname)                             \
                                COzipCvDUMMYTEMP (Char, fname)                           \
                                  COzipCvDUMMYTEMP (Dummy, fname)

/* This is used only by _aplmod_(), and is likely of no use to others. */
#define MAP_INTxINT_INT(fun, fname)                                                      \
    COzipCvAPLMODTEMPLATE (fun, fname, unsigned char, UByte, unsigned char)              \
      COzipCvAPLMODTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)         \
        COzipCvAPLMODTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)             \
          COzipCvAPLMODTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)        \
            COzipCvAPLMODTEMPLATE (fun, fname, unsigned long long, ULongLong,            \
                                   unsigned long long)                                   \
              COzipCvAPLMODTEMPLATE (fun, fname, signed char, Byte, signed char)         \
                COzipCvAPLMODTEMPLATE (fun, fname, short, Short, short)                  \
                  COzipCvAPLMODTEMPLATE (fun, fname, int, Int, int)                      \
                    COzipCvAPLMODTEMPLATE (fun, fname, long, Long, long)                 \
                      COzipCvAPLMODTEMPLATE (fun, fname, unsigned long long, LongLong,   \
                                             unsigned long long)                         \
                        COzipCvDUMMYTEMP (Float, fname) COzipCvDUMMYTEMP (Double, fname) \
                          COzipCvDUMMYTEMP (LongDouble, fname)                           \
                            COzipCvDUMMYTEMP (Bool, fname)                               \
                              COzipCvDUMMYTEMP (Char, fname)                             \
                                COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_BOOLxBOOL_BOOL(fun, fname)                                                   \
    COzipCvDUMMYTEMP (UByte, fname) COzipCvDUMMYTEMP (UShort, fname)                     \
      COzipCvDUMMYTEMP (UInt, fname) COzipCvDUMMYTEMP (ULong, fname)                     \
        COzipCvDUMMYTEMP (ULongLong, fname) COzipCvDUMMYTEMP (Byte, fname)               \
          COzipCvDUMMYTEMP (Short, fname) COzipCvDUMMYTEMP (Int, fname)                  \
            COzipCvDUMMYTEMP (Char, fname) COzipCvDUMMYTEMP (Long, fname)                \
              COzipCvDUMMYTEMP (LongLong, fname) COzipCvDUMMYTEMP (Float, fname)         \
                COzipCvDUMMYTEMP (Double, fname) COzipCvDUMMYTEMP (LongDouble, fname)    \
                  COzipCvTEMPLATE (fun, fname, bool, Bool, bool)                         \
                    COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_ANYxANY_BOOL(fun, fname)                                                     \
    COzipCvTEMPLATE (fun, fname, unsigned char, UByte, bool)                             \
      COzipCvTEMPLATE (fun, fname, unsigned short, UShort, bool)                         \
        COzipCvTEMPLATE (fun, fname, unsigned int, UInt, bool)                           \
          COzipCvTEMPLATE (fun, fname, unsigned long, ULong, bool)                       \
            COzipCvTEMPLATE (fun, fname, unsigned long long, ULongLong, bool)            \
              COzipCvTEMPLATE (fun, fname, signed char, Byte, bool)                      \
                COzipCvTEMPLATE (fun, fname, short, Short, bool)                         \
                  COzipCvTEMPLATE (fun, fname, int, Int, bool)                           \
                    COzipCvTEMPLATE (fun, fname, long, Long, bool)                       \
                      COzipCvTEMPLATE (fun, fname, long long, LongLong, bool)            \
                        COzipCvTEMPLATE (fun, fname, float, Float, bool)                 \
                          COzipCvTEMPLATE (fun, fname, double, Double, bool)             \
                            COzipCvTEMPLATE (fun, fname, long double, LongDouble, bool)  \
                              COzipCvTEMPLATE (fun, fname, bool, Bool, bool)             \
                                COzipCvTEMPLATE (fun, fname, unsigned char, Char, bool)  \
                                  COzipCvDUMMYTEMP (Dummy, fname)

#if 0
/*
 * Currently not used but potentially in the future.
 */
#define MAP_NUMxNUM_BOOL(fun, fname)                                                     \
    COzipCvTEMPLATE (fun, fname, unsigned char, UByte, bool)                             \
      COzipCvTEMPLATE (fun, fname, unsigned short, UShort, bool)                         \
        COzipCvTEMPLATE (fun, fname, unsigned int, UInt, bool)                           \
          COzipCvTEMPLATE (fun, fname, unsigned long, ULong, bool)                       \
            COzipCvTEMPLATE (fun, fname, unsigned long long, ULongLong, bool)            \
              COzipCvTEMPLATE (fun, fname, signed char, Byte, bool)                      \
                COzipCvTEMPLATE (fun, fname, short, Short, bool)                         \
                  COzipCvTEMPLATE (fun, fname, int, Int, bool)                           \
                    COzipCvTEMPLATE (fun, fname, long, Long, bool)                       \
                      COzipCvTEMPLATE (fun, fname, long long, LongLong, bool)            \
                        COzipCvTEMPLATE (fun, fname, float, Float, bool)                 \
                          COzipCvTEMPLATE (fun, fname, double, Double, bool)             \
                            COzipCvTEMPLATE (fun, fname, long double, LongDouble, bool)  \
                              COzipCvDUMMYTEMP (Bool, fname)                             \
                                COzipCvDUMMYTEMP (Char, fname)                           \
                                  COzipCvDUMMYTEMP (Dummy, fname)
#endif

#define MAP_BOOL_BOOL(fun, fname)                                                        \
    COzipCvDUMMYTEMP (UByte, fname) COzipCvDUMMYTEMP (UShort, fname)                     \
      COzipCvDUMMYTEMP (UInt, fname) COzipCvDUMMYTEMP (ULong, fname)                     \
        COzipCvDUMMYTEMP (ULongLong, fname) COzipCvDUMMYTEMP (Byte, fname)               \
          COzipCvDUMMYTEMP (Short, fname) COzipCvDUMMYTEMP (Int, fname)                  \
            COzipCvDUMMYTEMP (Char, fname) COzipCvDUMMYTEMP (Long, fname)                \
              COzipCvDUMMYTEMP (LongLong, fname) COzipCvDUMMYTEMP (Float, fname)         \
                COzipCvDUMMYTEMP (Double, fname) COzipCvDUMMYTEMP (LongDouble, fname)    \
                  COzipCvUNARYTEMPLATE (fun, fname, bool, Bool, bool)                    \
                    COzipCvDUMMYTEMP (Dummy, fname)

#if 0
/*
 * Currently not used but potentially in the future.
 */
#define MAP_NUM_TYPE(fun, fname, target_t)                                               \
    COzipCvUNARYTEMPLATE (fun, fname, unsigned char, UByte, target_t)                    \
      COzipCvUNARYTEMPLATE (fun, fname, unsigned short, UShort, target_t)                \
        COzipCvUNARYTEMPLATE (fun, fname, unsigned int, UInt, target_t)                  \
          COzipCvUNARYTEMPLATE (fun, fname, unsigned long, ULong, target_t)              \
            COzipCvUNARYTEMPLATE (fun, fname, unsigned long long, ULongLong, target_t)   \
              COzipCvUNARYTEMPLATE (fun, fname, signed char, Byte, target_t)             \
                COzipCvUNARYTEMPLATE (fun, fname, short, Short, target_t)                \
                  COzipCvUNARYTEMPLATE (fun, fname, int, Int, target_t)                  \
                    COzipCvUNARYTEMPLATE (fun, fname, long, Long, target_t)              \
                      COzipCvUNARYTEMPLATE (fun, fname, long long, LongLong, target_t)   \
                        COzipCvUNARYTEMPLATE (fun, fname, float, Float, target_t)        \
                          COzipCvUNARYTEMPLATE (fun, fname, double, Double, target_t)    \
                            COzipCvUNARYTEMPLATE (fun, fname, long double, LongDouble,   \
                                                  target_t)                              \
                              COzipCvDUMMYTEMP (Char, fname)                             \
                                COzipCvDUMMYTEMP (Bool, fname)                           \
                                  COzipCvDUMMYTEMP (Dummy, fname)
#endif

#define MAP_ANY_TYPE(fun, fname, target_t)                                               \
    COzipCvUNARYTEMPLATE (fun, fname, unsigned char, UByte, target_t)                    \
      COzipCvUNARYTEMPLATE (fun, fname, unsigned short, UShort, target_t)                \
        COzipCvUNARYTEMPLATE (fun, fname, unsigned int, UInt, target_t)                  \
          COzipCvUNARYTEMPLATE (fun, fname, unsigned long, ULong, target_t)              \
            COzipCvUNARYTEMPLATE (fun, fname, unsigned long long, ULongLong, target_t)   \
              COzipCvUNARYTEMPLATE (fun, fname, signed char, Byte, target_t)             \
                COzipCvUNARYTEMPLATE (fun, fname, short, Short, target_t)                \
                  COzipCvUNARYTEMPLATE (fun, fname, int, Int, target_t)                  \
                    COzipCvUNARYTEMPLATE (fun, fname, long, Long, target_t)              \
                      COzipCvUNARYTEMPLATE (fun, fname, long long, LongLong, target_t)   \
                        COzipCvUNARYTEMPLATE (fun, fname, float, Float, target_t)        \
                          COzipCvUNARYTEMPLATE (fun, fname, double, Double, target_t)    \
                            COzipCvUNARYTEMPLATE (fun, fname, long double, LongDouble,   \
                                                  target_t)                              \
                              COzipCvUNARYTEMPLATE (fun, fname, bool, Bool, target_t)    \
                                COzipCvUNARYTEMPLATE (fun, fname, unsigned char, Char,   \
                                                      target_t)                          \
                                  COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_NUM_NUM(fun, fname)                                                          \
    COzipCvUNARYTEMPLATE (fun, fname, unsigned char, UByte, unsigned char)               \
      COzipCvUNARYTEMPLATE (fun, fname, unsigned short, UShort, unsigned short)          \
        COzipCvUNARYTEMPLATE (fun, fname, unsigned int, UInt, unsigned int)              \
          COzipCvUNARYTEMPLATE (fun, fname, unsigned long, ULong, unsigned long)         \
            COzipCvUNARYTEMPLATE (fun, fname, unsigned long long, ULongLong,             \
                                  unsigned long long)                                    \
              COzipCvUNARYTEMPLATE (fun, fname, signed char, Byte, signed char)          \
                COzipCvUNARYTEMPLATE (fun, fname, short, Short, short)                   \
                  COzipCvUNARYTEMPLATE (fun, fname, int, Int, int)                       \
                    COzipCvUNARYTEMPLATE (fun, fname, long, Long, long)                  \
                      COzipCvUNARYTEMPLATE (fun, fname, long long, LongLong, long long)  \
                        COzipCvUNARYTEMPLATE (fun, fname, float, Float, float)           \
                          COzipCvUNARYTEMPLATE (fun, fname, double, Double, double)      \
                            COzipCvUNARYTEMPLATE (fun, fname, long double, LongDouble,   \
                                                  long double)                           \
                              COzipCvDUMMYTEMP (Char, fname)                             \
                                COzipCvDUMMYTEMP (Bool, fname)                           \
                                  COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_ABS_NUM_NUM(fun, fname)                                                      \
    COzipCvUNARYTEMPLATE (, fname, unsigned char, UByte, unsigned char)                  \
      COzipCvUNARYTEMPLATE (, fname, unsigned short, UShort, unsigned short)             \
        COzipCvUNARYTEMPLATE (, fname, unsigned int, UInt, unsigned int)                 \
          COzipCvUNARYTEMPLATE (, fname, unsigned long, ULong, unsigned long)            \
            COzipCvUNARYTEMPLATE (, fname, unsigned long long, ULongLong,                \
                                  unsigned long long)                                    \
              COzipCvABSTEMPLATE (fun, fname, signed char, Byte, signed char)            \
                COzipCvABSTEMPLATE (fun, fname, short, Short, short)                     \
                  COzipCvABSTEMPLATE (fun, fname, int, Int, int)                         \
                    COzipCvABSTEMPLATE (fun, fname, long, Long, long)                    \
                      COzipCvABSTEMPLATE (fun, fname, long long, LongLong, long long)    \
                        COzipCvABSTEMPLATE (fun, fname, float, Float, float)             \
                          COzipCvABSTEMPLATE (fun, fname, double, Double, double)        \
                            COzipCvABSTEMPLATE (fun, fname, long double, LongDouble,     \
                                                long double)                             \
                              COzipCvDUMMYTEMP (Bool, fname)                             \
                                COzipCvDUMMYTEMP (Char, fname)                           \
                                  COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_TOC_NUM_NUM(fun, fname)                                                      \
    COzipCvUNARYTEMPLATE (, fname, unsigned char, UByte, unsigned char)                  \
      COzipCvUNARYTEMPLATE (, fname, unsigned short, UShort, unsigned char)              \
        COzipCvUNARYTEMPLATE (, fname, unsigned int, UInt, unsigned char)                \
          COzipCvUNARYTEMPLATE (, fname, unsigned long, ULong, unsigned char)            \
            COzipCvUNARYTEMPLATE (, fname, unsigned long long, ULongLong, unsigned char) \
              COzipCvUNARYTEMPLATE (, fname, signed char, Byte, unsigned char)           \
                COzipCvUNARYTEMPLATE (, fname, short, Short, unsigned char)              \
                  COzipCvUNARYTEMPLATE (, fname, int, Int, unsigned char)                \
                    COzipCvUNARYTEMPLATE (, fname, long, Long, unsigned char)            \
                      COzipCvUNARYTEMPLATE (, fname, long long, LongLong, unsigned char) \
                        COzipCvDUMMYTEMP (Float, fname) COzipCvDUMMYTEMP (Double, fname) \
                          COzipCvDUMMYTEMP (LongDouble, fname)                           \
                            COzipCvDUMMYTEMP (Bool, fname)                               \
                              COzipCvDUMMYTEMP (Char, fname)                             \
                                COzipCvDUMMYTEMP (Dummy, fname)

#define MAP_TOBOOL_NUM_NUM(fun, fname)                                                   \
    COzipCvTOBOOLTEMPLATE (, fname, unsigned char, UByte, bool)                          \
      COzipCvTOBOOLTEMPLATE (, fname, unsigned short, UShort, bool)                      \
        COzipCvTOBOOLTEMPLATE (, fname, unsigned int, UInt, bool)                        \
          COzipCvTOBOOLTEMPLATE (, fname, unsigned long, ULong, bool)                    \
            COzipCvTOBOOLTEMPLATE (, fname, unsigned long long, ULongLong, bool)         \
              COzipCvTOBOOLTEMPLATE (, fname, signed char, Byte, bool)                   \
                COzipCvTOBOOLTEMPLATE (, fname, short, Short, bool)                      \
                  COzipCvTOBOOLTEMPLATE (, fname, int, Int, bool)                        \
                    COzipCvTOBOOLTEMPLATE (, fname, long, Long, bool)                    \
                      COzipCvTOBOOLTEMPLATE (, fname, long long, LongLong, bool)         \
                        COzipCvTOBOOLTEMPLATE (, fname, float, Float, bool)              \
                          COzipCvTOBOOLTEMPLATE (, fname, double, Double, bool)          \
                            COzipCvTOBOOLTEMPLATE (, fname, long long, LongDouble, bool) \
                              COzipCvTOBOOLTEMPLATE (, fname, bool, Bool, bool)          \
                                COzipCvDUMMYTEMP (Char, fname)                           \
                                  COzipCvDUMMYTEMP (Dummy, fname)
/*
 * The actual function definitions are defined by the following macro usages:
 */

MAP_NUMxNUM_NUM (+, Plus)

  MAP_NUMxNUM_NUM (-, Minus)

    MAP_NUMxNUM_NUM (*, Mul)

      MAP_NUMxNUM_NUM (/, Div)

        MAP_INTxINT_INT (%, Mod) MAP_INTxINT_INT (aplmod %, Aplmod)

          MAP_MINMAX_NUMxNUM_NUM (<, Min)

            MAP_MINMAX_NUMxNUM_NUM (>, Max)

              MAP_BOOLxBOOL_BOOL (&&, And)

                MAP_BOOLxBOOL_BOOL (||, Or)

                  MAP_ANYxANY_BOOL (==, Eq)

                    MAP_ANYxANY_BOOL (!=, Neq)

                      MAP_ANYxANY_BOOL (<=, Le)

                        MAP_ANYxANY_BOOL (<, Lt)

                          MAP_ANYxANY_BOOL (>, Gt)

                            MAP_ANYxANY_BOOL (>=, Ge)

                              MAP_BOOL_BOOL (!, Not)

                                MAP_ANY_TYPE ((signed char), Tob, signed char)

                                  MAP_ANY_TYPE ((short), Tos, short)

                                    MAP_ANY_TYPE ((int), Toi, int)

                                      MAP_ANY_TYPE ((long), Tol, long)

                                        MAP_ANY_TYPE ((long long), Toll, long long)

                                          MAP_ANY_TYPE ((unsigned char), Toub,
                                                        unsigned char)

                                            MAP_ANY_TYPE ((unsigned short), Tous,
                                                          unsigned short)

                                              MAP_ANY_TYPE ((unsigned int), Toui,
                                                            unsigned int)

                                                MAP_ANY_TYPE ((unsigned long), Toul,
                                                              unsigned long)

                                                  MAP_ANY_TYPE ((unsigned long long),
                                                                Toull, unsigned long long)

                                                    MAP_ANY_TYPE ((float), Tof, float)

                                                      MAP_ANY_TYPE ((double), Tod, double)

                                                        MAP_ABS_NUM_NUM (NOP, Abs)

                                                          MAP_TOC_NUM_NUM ((signed char),
                                                                           Toc)

                                                            MAP_TOBOOL_NUM_NUM ((boolean),
                                                                                Tobool)

                                                              MAP_NUM_NUM (-, Neg)

#undef DBUG_PREFIX
