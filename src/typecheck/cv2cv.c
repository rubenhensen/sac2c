/*
 * $Log$
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

#define TYP_IFcv2cv(fun) fun
cv2cvfunptr cv2cv[] = {
#include "type_info.mac"
};

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

#define COCv2CvTEMPLATE(type, ext)                                                       \
    void COCv2Cv##ext (void *src, int off, int len, void *res, int res_off)              \
    {                                                                                    \
        int i;                                                                           \
                                                                                         \
        DBUG_ENTER ("COCv2Cv##ext");                                                     \
        for (i = 0; i < len; i++) {                                                      \
            ((type *)res)[i + res_off] = ((type *)src)[i + off];                         \
        }                                                                                \
        DBUG_VOID_RETURN;                                                                \
    }

/*
 * The actual function definitions are defined by the following macro usages:
 */

COCv2CvTEMPLATE (unsigned short, UShort) COCv2CvTEMPLATE (unsigned int, UInt)
  COCv2CvTEMPLATE (unsigned long, ULong) COCv2CvTEMPLATE (short, Short)
    COCv2CvTEMPLATE (int, Int) COCv2CvTEMPLATE (long, Long)

      COCv2CvTEMPLATE (float, Float) COCv2CvTEMPLATE (double, Double)
        COCv2CvTEMPLATE (long double, LongDouble)

  /*
   * Finally, we provide a dummy function which should never be called!
   * It is defined for being able to make entries for all simpletypes in
   * type_info.mac!
   */

  void COCv2CvDummy (void *src, int off, int len, void *res, int res_off)
{
    DBUG_ENTER ("COCv2CvDummy");
    DBUG_ASSERT ((1 == 0), "COCv2CvDummy called!");
    DBUG_VOID_RETURN;
}
