/*
 *
 * $Log$
 * Revision 1.5  2003/04/07 14:24:41  sbs
 * new printing mechanism built and correct format strings inserted
 * for longs etcpp.
 *
 * Revision 1.4  2001/05/17 10:04:56  nmw
 * missing include of internal_lib.h added
 *
 * Revision 1.3  2001/05/16 13:43:58  nmw
 * MALLOC/FREE changed to Malloc/Free
 *
 * Revision 1.2  2001/04/04 10:00:02  nmw
 *  missing convert functions for basetype char added
 *
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
#include "internal_lib.h"

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
    char *COCv2Str##ext (void *src, int off, int len, int max_char)                      \
    {                                                                                    \
        int i;                                                                           \
        char format[10];                                                                 \
        char *buffer;                                                                    \
        char *buffer_act;                                                                \
                                                                                         \
        DBUG_ENTER ("COCv2Str##ext");                                                    \
        sprintf (format, ",%s", form);                                                   \
        buffer = (char *)Malloc ((100 + max_char) * sizeof (char));                      \
        buffer_act = buffer;                                                             \
                                                                                         \
        if (len > 0) {                                                                   \
            buffer_act += sprintf (buffer_act, form, ((type *)src)[off]);                \
        }                                                                                \
        for (i = 1; (i < len) && (buffer_act - buffer < max_char); i++) {                \
            buffer_act += sprintf (buffer_act, format, ((type *)src)[i + off]);          \
        }                                                                                \
        if ((i < len) || (buffer_act > buffer + max_char)) {                             \
            sprintf (buffer + max_char, "...");                                          \
        }                                                                                \
        DBUG_RETURN (buffer);                                                            \
    }

/*
 * The actual function definitions are defined by the following macro usages:
 */

COCv2StrTEMPLATE (unsigned short, UShort, "%hud")
  COCv2StrTEMPLATE (unsigned int, UInt, "%ud")
    COCv2StrTEMPLATE (unsigned long, ULong, "%lud") COCv2StrTEMPLATE (short, Short, "%hd")
      COCv2StrTEMPLATE (int, Int, "%d") COCv2StrTEMPLATE (long, Long, "%ld")

        COCv2StrTEMPLATE (float, Float, "%f") COCv2StrTEMPLATE (double, Double, "%f")
          COCv2StrTEMPLATE (long double, LongDouble, "%Lf")

            COCv2StrTEMPLATE (char, Char, "%c")

  /*
   * Finally, we provide a dummy function which should never be called!
   * It is defined for being able to make entries for all simpletypes in
   * type_info.mac!
   */

  char *COCv2StrDummy (void *src, int off, int len, int max_char)
{
    DBUG_ENTER ("COCv2StrDummy");
    DBUG_ASSERT ((1 == 0), "COCv2StrDummy called!");
    DBUG_RETURN ((char *)NULL);
}
