/*
 *
 * $Log$
 * Revision 1.10  2005/09/09 17:52:00  sah
 * floats and doubles are serialized properly now.
 *
 * Revision 1.9  2005/06/02 20:35:27  sah
 * found a ANSI C compliant portable way to
 * print constant vectors.
 *
 * Revision 1.8  2005/05/18 13:55:23  sah
 * using %hho instead of %o as the char is transformed to
 * int when passed to printf and thus we need to tell printf
 * that it was a char indeed.
 *
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
#include "internal_lib.h"
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
        int max;
        char *data;

        fprintf (file, "COdeserializeConstant( %d, ", CONSTANT_TYPE (cnst));

        SHserializeShape (file, CONSTANT_SHAPE (cnst));

        max = global.basetype_size[CONSTANT_TYPE (cnst)] * CONSTANT_VLEN (cnst);
        data = ILIBbyteArrayToHexString (max, CONSTANT_ELEMS (cnst));

        fprintf (file, ", %d, \"%s\")", CONSTANT_VLEN (cnst), data);

        data = ILIBfree (data);
    }

    DBUG_VOID_RETURN;
}

constant *
COdeserializeConstant (simpletype type, shape *shp, int vlen, char *vec)
{
    constant *result;
    void *data;

    DBUG_ENTER ("COdeserializeConstant");

    data = COINTallocCV (type, vlen);
    data = ILIBhexStringToByteArray ((unsigned char *)data, vec);

    result = COINTmakeConstant (type, shp, data, vlen);

    DBUG_RETURN (result);
}
