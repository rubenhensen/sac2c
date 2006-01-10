/*
 *
 * $Id$
 *
 */

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
#include "dbug.h"
#include "internal_lib.h"

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
    char *COcv2Str##ext (void *src, int off, int len, int max_char)                      \
    {                                                                                    \
        int i;                                                                           \
        char format[10];                                                                 \
        char *buffer;                                                                    \
        char *buffer_act;                                                                \
                                                                                         \
        DBUG_ENTER ("COcv2Str##ext");                                                    \
        sprintf (format, ",%s", form);                                                   \
        buffer = (char *)ILIBmalloc ((100 + max_char) * sizeof (char));                  \
        buffer_act = buffer;                                                             \
                                                                                         \
        if (len > 0) {                                                                   \
            buffer_act += sprintf (buffer_act, form, ((type *)src)[off]);                \
            for (i = 1; (i < len) && (buffer_act - buffer < max_char); i++) {            \
                buffer_act += sprintf (buffer_act, format, ((type *)src)[i + off]);      \
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

COcv2StrTEMPLATE (unsigned short, UShort, "%hud")
  COcv2StrTEMPLATE (unsigned int, UInt, "%ud")
    COcv2StrTEMPLATE (unsigned long, ULong, "%lud") COcv2StrTEMPLATE (short, Short, "%hd")
      COcv2StrTEMPLATE (int, Int, "%d") COcv2StrTEMPLATE (long, Long, "%ld")

        COcv2StrTEMPLATE (float, Float, "%f") COcv2StrTEMPLATE (double, Double, "%f")
          COcv2StrTEMPLATE (long double, LongDouble, "%Lf")

            COcv2StrTEMPLATE (char, Char, "%c")

  /*
   * Finally, we provide a dummy function which should never be called!
   * It is defined for being able to make entries for all simpletypes in
   * type_info.mac!
   */

  char *COcv2StrDummy (void *src, int off, int len, int max_char)
{
    DBUG_ENTER ("COcv2StrDummy");
    DBUG_ASSERT ((1 == 0), "COcv2StrDummy called!");
    DBUG_RETURN ((char *)NULL);
}
