/*
 * $Log$
 * Revision 1.5  2004/11/26 23:58:18  khf
 * names adjusted
 *
 * Revision 1.4  2004/11/22 18:55:29  cg
 * Moved all definitions/declarations of global variables to globals.mac
 *
 * Revision 1.3  2001/04/30 12:30:20  nmw
 * COCv2CvHidden() added
 *
 * Revision 1.2  2001/04/04 09:59:47  nmw
 *  missing convert functions for basetype char added
 *
 * Revision 1.1  2001/03/02 14:32:59  sbs
 * Initial revision
 *
 * Revision 3.1  2000/11/20 18:00:03  sacbase
 * new release made
 *
 * Revision 1.2  1999/10/22 14:15:19  sbs
 * commented and added versions for almost all simpletypes...
 *
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

/*
 * This module provides a set of functions for copying elements of constant vectors
 * into other constant vectors. The problem with this task is that such a function
 * has to be overloaded on all potential types. Since C does not support such
 * specifications, a function table is created which is parameterized over simpletype.
 * This is done by using the    .mac-mechanism     on type_info.mac in order to
 * make sure that all simpletypes are dealt with.
 */

#include "dbug.h"

#include "cv2cv.h"

/******************************************************************************
 *
 * function:
 *    void COCv2CvXXX( void *src, int off, int len, void *res, int res_off)
 *
 * description:
 *    Functions for copying elements from one cv to another one!
 *    Since all these functions are identical apart from the type casts,
 *    they are implemented using the C macro mechanism.
 *
 ******************************************************************************/

#define COcv2CvTEMPLATE(type, ext)                                                       \
    void COcv2Cv##ext (void *src, int off, int len, void *res, int res_off)              \
    {                                                                                    \
        int i;                                                                           \
                                                                                         \
        DBUG_ENTER ("COcv2Cv##ext");                                                     \
        for (i = 0; i < len; i++) {                                                      \
            ((type *)res)[i + res_off] = ((type *)src)[i + off];                         \
        }                                                                                \
        DBUG_VOID_RETURN;                                                                \
    }

/*
 * The actual function definitions are defined by the following macro usages:
 */

COcv2CvTEMPLATE (unsigned short, UShort) COcv2CvTEMPLATE (unsigned int, UInt)
  COcv2CvTEMPLATE (unsigned long, ULong) COcv2CvTEMPLATE (short, Short)
    COcv2CvTEMPLATE (int, Int) COcv2CvTEMPLATE (long, Long)

      COcv2CvTEMPLATE (float, Float) COcv2CvTEMPLATE (double, Double)
        COcv2CvTEMPLATE (long double, LongDouble)

          COcv2CvTEMPLATE (char, Char)

            COcv2CvTEMPLATE (void *, Hidden)

  /*
   * Finally, we provide a dummy function which should never be called!
   * It is defined for being able to make entries for all simpletypes in
   * type_info.mac!
   */

  void COcv2CvDummy (void *src, int off, int len, void *res, int res_off)
{
    DBUG_ENTER ("COCv2CvDummy");
    DBUG_ASSERT ((1 == 0), "COCv2CvDummy called!");
    DBUG_VOID_RETURN;
}
