/*
 *
 * $Log$
 * Revision 1.1  2004/09/27 13:03:37  sah
 * Initial revision
 *
 *
 *
 */

#include "constants.h"
#include "constants_internal.h"
#include <stdio.h>
#include "shape.h"
#include "globals.h"

void
COSerializeConstant (FILE *file, constant *cnst)
{
    DBUG_ENTER ("COSerializeConstant");

    DBUG_PRINT ("SET", ("Processing constant structure"));

    if (cnst == NULL) {
        fprintf (file, "NULL");
    } else {
        int cnt;
        char *data;

        fprintf (file, "CODeserializeConstant( %d, ", CONSTANT_TYPE (cnst));

        SHSerializeShape (file, CONSTANT_SHAPE (cnst));

        /* the data vector of a constant is serialized as a
           char array */

        data = (char *)CONSTANT_ELEMS (cnst);

        fprintf (file, ", %d, \"", CONSTANT_VLEN (cnst));

        for (cnt = 0; cnt < basetype_size[CONSTANT_TYPE (cnst)] * CONSTANT_VLEN (cnst);
             cnt++) {
            fprintf (file, "\\%d", data[cnt]);
        }

        fprintf (file, "\")");
    }

    DBUG_VOID_RETURN;
}

constant *
CODeserializeConstant (simpletype type, shape *shp, int vlen, char *vec)
{
    constant *result;
    void *data;

    DBUG_ENTER ("CODeserializeConstant");

    /* we have to copy the data vector as it is static */
    data = AllocCV (type, vlen);
    CopyElemsFromCVToCV (type, (void *)vec, 0, vlen, data, 0);

    result = MakeConstant (type, shp, data, vlen);

    DBUG_RETURN (result);
}
