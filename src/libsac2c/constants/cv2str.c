/*
 * This module provides a set of functions for creating strings from elements
 * of constant vectors.
 * The problem with this task is that such a function has to be overloaded on
 * all potential types.
 * Since C does not support such specifications, a function table is created
 * which is parameterized over simpletype.
 * This is done by using the    .mac-mechanism     on type_info.mac in order
 * to make sure that all simpletypes are dealt with.
 */

#include <strings.h>
#include <stdlib.h>

#include "cv2str.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"

/******************************************************************************
 *
 * function:
 *    char *COcv2StrXXX( void *src, int off, int len)
 *
 * description:
 *    Functions for printing elements from cv's into a given string buffer.
 *    Since all these functions are identical apart from the type casts,
 *    they are implemented using the C macro mechanism.
 *
 ******************************************************************************/

#define COcv2StrTEMPLATE(type, ext, form)                                                \
    char *COcv2Str##ext (void *src, size_t off, size_t len, size_t max_char)             \
    {                                                                                    \
        size_t i;                                                                        \
        char format[10];                                                                 \
        char *buffer;                                                                    \
        char *buffer_act;                                                                \
        size_t max = max_char;                                                           \
                                                                                         \
        DBUG_ENTER ();                                                                   \
        sprintf (format, ",%s", form);                                                   \
        buffer = (char *)MEMmalloc ((100 + max) * sizeof (char));                        \
        buffer_act = buffer;                                                             \
                                                                                         \
        if (len > 0) {                                                                   \
            buffer_act += snprintf (buffer_act, 100, form, ((type *)src)[off]);          \
            for (i = 1; (i < len) && ((size_t)(buffer_act - buffer) < max_char); i++) {  \
                buffer_act                                                               \
                  += snprintf (buffer_act, 100, format, ((type *)src)[i + off]);         \
            }                                                                            \
            if ((i < len) || (buffer_act > buffer + max_char)) {                         \
                sprintf (buffer + max_char, "...");                                      \
            }                                                                            \
        } else {                                                                         \
            *buffer_act = '\0';                                                          \
        }                                                                                \
        DBUG_RETURN (buffer);                                                            \
    }

/*
 * The actual function definitions are defined by the following macro usages:
 */

COcv2StrTEMPLATE (unsigned char, UByte, "%c")
  COcv2StrTEMPLATE (unsigned short, UShort, "%hu")
    COcv2StrTEMPLATE (unsigned int, UInt, "%u")
      COcv2StrTEMPLATE (unsigned long, ULong, "%lu")
        COcv2StrTEMPLATE (unsigned long long, ULongLong, "%llu")
          COcv2StrTEMPLATE (char, Byte, "%c") COcv2StrTEMPLATE (short, Short, "%hd")
            COcv2StrTEMPLATE (int, Int, "%d") COcv2StrTEMPLATE (long, Long, "%ld")
              COcv2StrTEMPLATE (long long, LongLong, "%lld")
                COcv2StrTEMPLATE (bool, Bool, "%d")

                  COcv2StrTEMPLATE (float, Float, "%f")
                    COcv2StrTEMPLATE (double, Double, "%f")
                      COcv2StrTEMPLATE (long double, LongDouble, "%Lf")

                        COcv2StrTEMPLATE (char, Char, "%c")

  /* SIMD vectors are kind of special case.
     FIXME Do the implementation.  */
  char *COcv2StrFloatvec (void *src, size_t off, size_t len, size_t max_char)
{
    char *buf = (char *)MEMmalloc (1024);
    char *p = buf;
    char *end = p + 1024;
    size_t n_left = (size_t) (end - p);

    p += snprintf (p,  n_left, "floatvec<%zu>[", len);

    for (size_t i = 0; i < len; i++){
        n_left = (size_t) (end - p);
        if (i < 3)
            p += snprintf (p, n_left, "[%f,...]", *(float *)&((floatvec *)src)[i + off]);
        else {
            p += snprintf (p, n_left, "...");
            break;
        }
    }

    n_left = (size_t) (end - p);
    p += snprintf (p, n_left, "]");

    return buf;
}

/*
 * Finally, we provide a dummy function which should never be called!
 * It is defined for being able to make entries for all simpletypes in
 * type_info.mac!
 */

char *
COcv2StrDummy (void *src, size_t off, size_t len, size_t max_char)
{
    DBUG_ENTER ();
    DBUG_UNREACHABLE ("COcv2StrDummy called!");
    DBUG_RETURN ((char *)NULL);
}

#undef DBUG_PREFIX
