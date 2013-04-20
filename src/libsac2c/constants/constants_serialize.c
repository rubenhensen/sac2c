#include "constants.h"
#include "constants_internal.h"
#include "str.h"
#include "memory.h"
#include <stdio.h>

#define DBUG_PREFIX "SET"
#include "debug.h"

#include "shape.h"
#include "globals.h"

void
COserializeConstant (FILE *file, constant *cnst)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Processing constant structure");

    if (cnst == NULL) {
        fprintf (file, "NULL");
    } else {
        int max;
        char *data;

        fprintf (file, "COdeserializeConstant( %d, ", CONSTANT_TYPE (cnst));

        SHserializeShape (file, CONSTANT_SHAPE (cnst));

        max = global.basetype_size[CONSTANT_TYPE (cnst)] * CONSTANT_VLEN (cnst);
        data = STRbytes2Hex (max, (unsigned char *)CONSTANT_ELEMS (cnst));

        fprintf (file, ", %d, \"%s\")", CONSTANT_VLEN (cnst), data);

        data = MEMfree (data);
    }

    DBUG_RETURN ();
}

constant *
COdeserializeConstant (simpletype type, shape *shp, int vlen, char *vec)
{
    constant *result;
    void *data;

    DBUG_ENTER ();

    data = COINTallocCV (type, vlen);
    data = STRhex2Bytes ((unsigned char *)data, vec);

    result = COINTmakeConstant (type, shp, data, vlen);

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
