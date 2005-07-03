/*
 *
 * $Log$
 * Revision 1.1  2005/07/03 16:57:39  ktr
 * Initial revision
 *
 */
#include "NumLookUpTable.h"

#include "tree_basic.h"
#include "internal_lib.h"
#include "dbug.h"
#include "LookUpTable.h"

/******************************************************************************
 *
 * Static functions
 *
 *****************************************************************************/
static int *
FreeInt (int *i)
{
    DBUG_ENTER ("FreeInt");

    i = ILIBfree (i);

    DBUG_RETURN (i);
}

static int *
DupInt (int *i)
{
    int *j;

    DBUG_ENTER ("DupInt");

    j = ILIBmalloc (sizeof (int));
    *j = *i;

    DBUG_RETURN (j);
}

/******************************************************************************
 *
 * function:
 *   lut_t *NLUTgenerateNlut( node *args, node *vardecs)
 *
 * description:
 *   Generates a new NLUT which associates zeros with all vardecs and args
 *
 *****************************************************************************/
lut_t *
NLUTgenerateNlut (node *args, node *vardecs)
{
    lut_t *lut;
    node *n, *avis;
    int *z;

    DBUG_ENTER ("NLUTgenerateNlut");

    DBUG_ASSERT ((args == NULL) || (NODE_TYPE (args) == N_arg),
                 "First argument of NLUTgenerateNlut must be NULL or N_arg");
    DBUG_ASSERT ((vardecs == NULL) || (NODE_TYPE (vardecs) == N_vardec),
                 "Second argument of NLUTgenerateNlut must be NULL or N_vardec");

    lut = LUTgenerateLut ();

    n = args;
    while (n != NULL) {
        avis = ARG_AVIS (n);

        DBUG_ASSERT (LUTsearchInLutPp (lut, avis) == avis,
                     "AVIS already exists in NLUT!");

        z = ILIBmalloc (sizeof (int));
        *z = 0;

        lut = LUTinsertIntoLutP (lut, avis, z);
        n = ARG_NEXT (n);
    }

    n = vardecs;
    while (n != NULL) {
        avis = VARDEC_AVIS (n);

        DBUG_ASSERT (LUTsearchInLutPp (lut, avis) == avis,
                     "AVIS already exists in NLUT!");

        z = ILIBmalloc (sizeof (int));
        *z = 0;

        lut = LUTinsertIntoLutP (lut, avis, z);
        n = VARDEC_NEXT (n);
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *NLUTduplicateNlut( lut_t *nlut)
 *
 * description:
 *   Duplicates an Nlut
 *
 *****************************************************************************/
lut_t *
NLUTduplicateNlut (lut_t *nlut)
{
    lut_t *lut;

    DBUG_ENTER ("NLUTduplicateNlut");

    lut = LUTduplicateLut (nlut);

    lut = LUTmapLutP (lut, (void *(*)(void *))DupInt);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *NLUTremoveNlut( lut_t *nlut)
 *
 * description:
 *   Removes an Nlut an all its content
 *
 *****************************************************************************/
lut_t *
NLUTremoveNlut (lut_t *nlut)
{
    DBUG_ENTER ("NLUTremoveNlut");

    nlut = LUTmapLutP (nlut, (void *(*)(void *))FreeInt);

    nlut = LUTremoveLut (nlut);

    DBUG_RETURN (nlut);
}

/******************************************************************************
 *
 * function:
 *   void NLUTsetNum( lut_t *nlut, node *avis, int num)
 *
 * description:
 *   Associates the given avis with the given integer value
 *
 *****************************************************************************/
void
NLUTsetNum (lut_t *nlut, node *avis, int num)
{
    int *i;

    DBUG_ENTER ("NLUTsetNum");

    i = LUTsearchInLutPp (nlut, avis);

    DBUG_ASSERT ((void *)i != (void *)avis, "AVIS does not exist in NLUT!");

    *i = num;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   int NLUTgetNum( lut_t *nlut, node *avis)
 *
 * description:
 *   Associates the given avis with the given integer value
 *
 *****************************************************************************/
int
NLUTgetNum (lut_t *nlut, node *avis)
{
    int *i;

    DBUG_ENTER ("NLUTgetNum");

    i = LUTsearchInLutPp (nlut, avis);

    DBUG_ASSERT (((void *)i) != ((void *)avis), "AVIS does not exist in NLUT!");

    DBUG_RETURN (*i);
}
