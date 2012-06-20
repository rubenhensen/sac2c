/*
 * $Id$
 */

#include "builtin_types.h"
#include "shape.h"
#include "constants.h"

typedef int BITint;
typedef shape *BITshp;
typedef const *BITval;

bool
BITeqIntInt (BITint a, BITint b)
{
    DBUG_ENTER ();
    DBUG_RETURN (a == b);
}

bool
BITleIntInt (BITint a, BITint b)
{
    DBUG_ENTER ();
    DBUG_RETURN (a <= b);
}

bool
BITgeIntInt (BITint a, BITint b)
{
    DBUG_ENTER ();
    DBUG_RETURN (a >= b);
}

bool
BITeqShpShp (BITshp a, BITshp b)
{
    DBUG_ENTER ();
    DBUG_RETURN (SHcompareShapes (a, b));
}

bool
BITeqValVal (BITval a, BITval b)
{
    DBUG_ENTER ();
    DBUG_RETURN (COcompareConstants (a, b));
}

bool
BITakvaks (BITint dim, BITshape shp, BITconst val, BITint dim2, BITshape shp2)
{
    DBUG_ENTER ();
    DBUG_RETURN (BITeqIntInt (dim, dim2) && BITeqShpShp (shp, shp2));
}

bool
BITaksakd (BITint dim, BITshape shp, BITint dim2)
{
    DBUG_ENTER ();
    DBUG_RETURN (BITeqIntInt (dim, dim2));
}

bool
BITakdaudge (BITint dim, BITint dim2)
{
    DBUG_ENTER ();
    DBUG_RETURN (BITleIntInt (dim, dim2));
}

bool
BITaudgeaudge (BITint dim, BITint dim2)
{
    DBUG_ENTER ();
    DBUG_RETURN (BITgeIntInt (dim, dim2));
}
