/*
 *
 * $Log$
 * Revision 1.1  2001/03/02 14:33:05  sbs
 * Initial revision
 *
 * Revision 1.2  2001/02/27 11:27:07  dkr
 * no changes done
 *
 * Revision 1.1  2001/02/23 18:07:49  sbs
 * Initial revision
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
#include "dbug.h"

#include "cv2str.h"

#define TYP_IFcv2str(fun) fun
cv2strfunptr cv2str[] = {
#include "type_info.mac"
};

/******************************************************************************
 *
 * function:
 *    char *COCv2StrXXX( void *src, int off, int len)
 *
 * description:
 *    Functions for printing elements from cv's into a given string buffer.
 *    Since all these functions are identical apart from the type casts,
 *    they are implemented using the C macro mechanism.
 *
 ******************************************************************************/

#define COCv2StrTEMPLATE(type, ext, form)                                                \
    char *COCv2Str##ext (void *src, int off, int len)                                    \
    {                                                                                    \
        int i;                                                                           \
        char format[10];                                                                 \
        char *buffer;                                                                    \
        char *buffer_act;                                                                \
                                                                                         \
        DBUG_ENTER ("COCv2Str##ext");                                                    \
        sprintf (format, " %s", form);                                                   \
        buffer = (char *)malloc (100 * sizeof (char));                                   \
        buffer_act = buffer;                                                             \
                                                                                         \
        if (len > 0) {                                                                   \
            buffer_act += sprintf (buffer_act, format, ((type *)src)[off]);              \
        }                                                                                \
        for (i = 1; i < len; i++) {                                                      \
            buffer_act += sprintf (buffer_act, format, ((type *)src)[i + off]);          \
            if (buffer_act > buffer + 50) {                                              \
                sprintf (buffer_act, "...(truncated) ");                                 \
            }                                                                            \
        }                                                                                \
        DBUG_RETURN (buffer);                                                            \
    }

/*
 * The actual function definitions are defined by the following macro usages:
 */

COCv2StrTEMPLATE (unsigned short, UShort, "%d")
  COCv2StrTEMPLATE (unsigned int, UInt, "%d")
    COCv2StrTEMPLATE (unsigned long, ULong, "%d") COCv2StrTEMPLATE (short, Short, "%d")
      COCv2StrTEMPLATE (int, Int, "%d") COCv2StrTEMPLATE (long, Long, "%d")

        COCv2StrTEMPLATE (float, Float, "%f") COCv2StrTEMPLATE (double, Double, "%f")
          COCv2StrTEMPLATE (long double, LongDouble, "%f")

  /*
   * Finally, we provide a dummy function which should never be called!
   * It is defined for being able to make entries for all simpletypes in
   * type_info.mac!
   */

  char *COCv2StrDummy (void *src, int off, int len)
{
    DBUG_ENTER ("COCv2StrDummy");
    DBUG_ASSERT ((1 == 0), "COCv2StrDummy called!");
    DBUG_RETURN ((char *)NULL);
}
