/*
 * $Log$
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#include "tree_basic.h"
#include "dbug.h"

#include "free.h"
#include "cv2cv.h"

#define TYP_IFcv2cv(fun) fun
cv2cvfunptr cv2cv[] = {
#include "type_info.mac"
};

/*
 * Functions for copying a tile from a Constant-Vector (void *)
 *
 */

#define COCv2CvTEMPLATE(type, ext)                                                       \
    void *COCv2Cv##ext (void *elems, int offset, int length)                             \
    {                                                                                    \
        type *res;                                                                       \
        int i;                                                                           \
                                                                                         \
        DBUG_ENTER ("COCv2Cv##ext");                                                     \
        res = (type *)MALLOC (sizeof (type) * length);                                   \
        for (i = 0; i < length; i++) {                                                   \
            res[i] = ((type *)elems)[i + offset];                                        \
        }                                                                                \
        DBUG_RETURN (res);                                                               \
    }

COCv2CvTEMPLATE (int, Int) COCv2CvTEMPLATE (double, Double) COCv2CvTEMPLATE (float, Float)

  void *COCv2CvDummy (void *elems, int offset, int length)
{
    DBUG_ASSERT ((1 == 0), "COCv2CvDummy called!");
    return (NULL);
}
