/* $Id$ */

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
    COcv2CvTEMPLATE (int, Int) COcv2CvTEMPLATE (bool, Bool) COcv2CvTEMPLATE (long, Long)

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
