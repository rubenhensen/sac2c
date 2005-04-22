/*
 *
 * $Log$
 * Revision 1.7  2005/04/22 08:04:19  sah
 * escape sequences have to be octal values
 *
 * Revision 1.6  2005/04/18 08:56:10  sah
 * fixed a format qualifier so that unsigned values
 * are printed as such
 *
 * Revision 1.5  2005/02/15 21:07:40  sah
 * module system fixes
 *
 * Revision 1.4  2004/11/26 16:23:52  jhb
 * compile
 *
 * Revision 1.3  2004/11/26 14:47:47  sbs
 * *** empty log message ***
 *
 * Revision 1.2  2004/11/26 14:38:55  sbs
 * compiles
 *
 * Revision 1.1  2004/09/27 13:03:37  sah
 * Initial revision
 *
 *
 *
 */

#include "constants.h"
#include "constants_internal.h"
#include <stdio.h>
#include "dbug.h"

#include "shape.h"
#include "globals.h"

void
COserializeConstant (FILE *file, constant *cnst)
{
    DBUG_ENTER ("COserializeConstant");

    DBUG_PRINT ("SET", ("Processing constant structure"));

    if (cnst == NULL) {
        fprintf (file, "NULL");
    } else {
        int cnt;
        char *data;

        fprintf (file, "COdeserializeConstant( %d, ", CONSTANT_TYPE (cnst));

        SHserializeShape (file, CONSTANT_SHAPE (cnst));

        /* the data vector of a constant is serialized as a
           char array */

        data = (char *)CONSTANT_ELEMS (cnst);

        fprintf (file, ", %d, \"", CONSTANT_VLEN (cnst));

        for (cnt = 0;
             cnt < global.basetype_size[CONSTANT_TYPE (cnst)] * CONSTANT_VLEN (cnst);
             cnt++) {
            fprintf (file, "\\%o", data[cnt]);
        }

        fprintf (file, "\")");
    }

    DBUG_VOID_RETURN;
}

constant *
COdeserializeConstant (simpletype type, shape *shp, int vlen, char *vec)
{
    constant *result;
    void *data;

    DBUG_ENTER ("COdeserializeConstant");

    /* we have to copy the data vector as it is static */
    data = COINTallocCV (type, vlen);
    COINTcopyElemsFromCVToCV (type, (void *)vec, 0, vlen, data, 0);

    result = COINTmakeConstant (type, shp, data, vlen);

    DBUG_RETURN (result);
}
