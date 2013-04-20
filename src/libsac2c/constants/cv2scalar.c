#include "tree_basic.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "cv2scalar.h"

/*
 * Functions for converting a single element from a Constant-Vector (void *)
 * into a scalar node from the AST!
 */

node *
COcv2Num (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNum (((int *)elems)[offset]));
}

node *
COcv2Numbyte (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumbyte (((char *)elems)[offset]));
}

node *
COcv2Numshort (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumshort (((short *)elems)[offset]));
}

node *
COcv2Numint (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumint (((int *)elems)[offset]));
}

node *
COcv2Numlong (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumlong (((long *)elems)[offset]));
}

node *
COcv2Numlonglong (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumlonglong (((long long *)elems)[offset]));
}

node *
COcv2Numubyte (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumubyte (((unsigned char *)elems)[offset]));
}

node *
COcv2Numushort (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumushort (((unsigned short *)elems)[offset]));
}

node *
COcv2Numuint (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumuint (((unsigned int *)elems)[offset]));
}

node *
COcv2Numulong (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumulong (((unsigned long *)elems)[offset]));
}

node *
COcv2Numulonglong (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeNumulonglong (((unsigned long long *)elems)[offset]));
}

node *
COcv2Double (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeDouble (((double *)elems)[offset]));
}

node *
COcv2Bool (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeBool (((bool *)elems)[offset]));
}

node *
COcv2Float (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeFloat (((float *)elems)[offset]));
}

node *
COcv2Char (void *elems, int offset)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeChar (((char *)elems)[offset]));
}

node *
COcv2ScalarDummy (void *elems, int offset)
{
    DBUG_ASSERT (1 == 0, "COCv2SCalarDummy called!");
    return (NULL);
}

#undef DBUG_PREFIX
