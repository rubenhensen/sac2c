/*
 * $Log$
 * Revision 1.5  2004/11/26 14:45:22  sbs
 * compiles
 *
 * Revision 1.4  2004/11/22 18:55:29  cg
 * Moved all definitions/declarations of global variables to globals.mac
 *
 * Revision 1.3  2003/08/11 16:16:18  sbs
 * Now, COCv2Char indeed creates Chars (instead of Booleans) in COCv2Char!
 *
 * Revision 1.2  2001/03/22 14:27:31  nmw
 * functions to convert float, bool, char added
 *
 * Revision 1.1  2001/03/02 14:33:02  sbs
 * Initial revision
 *
 * Revision 3.1  2000/11/20 18:00:05  sacbase
 * new release made
 *
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 */

#include "tree_basic.h"
#include "dbug.h"

#include "cv2scalar.h"

/*
 * Functions for converting a single element from a Constant-Vector (void *)
 * into a scalar node from the AST!
 */

node *
COcv2Num (void *elems, int offset)
{
    DBUG_ENTER ("COCv2Num");

    DBUG_RETURN (TBmakeNum (((int *)elems)[offset]));
}

node *
COcv2Double (void *elems, int offset)
{
    DBUG_ENTER ("COCv2Num");

    DBUG_RETURN (TBmakeDouble (((double *)elems)[offset]));
}

node *
COcv2Bool (void *elems, int offset)
{
    DBUG_ENTER ("COCv2Bool");

    DBUG_RETURN (TBmakeBool (((bool *)elems)[offset]));
}

node *
COcv2Float (void *elems, int offset)
{
    DBUG_ENTER ("COCv2Float");

    DBUG_RETURN (TBmakeFloat (((float *)elems)[offset]));
}

node *
COcv2Char (void *elems, int offset)
{
    DBUG_ENTER ("COCv2Char");

    DBUG_RETURN (TBmakeChar (((char *)elems)[offset]));
}

node *
COcv2ScalarDummy (void *elems, int offset)
{
    DBUG_ASSERT ((1 == 0), "COCv2SCalarDummy called!");
    return (NULL);
}
