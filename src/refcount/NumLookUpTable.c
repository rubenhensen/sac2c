/*
 *
 * $Log$
 * Revision 1.2  2005/07/16 09:57:55  ktr
 * enhanced functionality
 *
 * Revision 1.1  2005/07/03 16:57:39  ktr
 * Initial revision
 *
 */
#include "NumLookUpTable.h"

#include "tree_basic.h"
#include "internal_lib.h"
#include "dbug.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"

/******************************************************************************
 *
 * Type definitions
 *
 *****************************************************************************/
struct NLUT_T {
    dfmask_base_t *maskbase;
    int *maskbase_rc;
    dfmask_t *mask;
    lut_t *lut;
};

#define NLUT_MASKBASE(n) (n->maskbase)
#define NLUT_MASKBASE_RC(n) (n->maskbase_rc)
#define NLUT_MASKBASE_RCVAL(n) (*NLUT_MASKBASE_RC (n))
#define NLUT_MASK(n) (n->mask)
#define NLUT_LUT(n) (n->lut)

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
 *   nlut_t *NLUTgenerateNlut( node *args, node *vardecs)
 *
 * description:
 *   Generates a new NLUT which associates zeros with all vardecs and args
 *
 *****************************************************************************/
nlut_t *
NLUTgenerateNlut (node *args, node *vardecs)
{
    nlut_t *nlut;
    node *avis;
    int *z;

    DBUG_ENTER ("NLUTgenerateNlut");

    DBUG_ASSERT ((args == NULL) || (NODE_TYPE (args) == N_arg),
                 "First argument of NLUTgenerateNlut must be NULL or N_arg");
    DBUG_ASSERT ((vardecs == NULL) || (NODE_TYPE (vardecs) == N_vardec),
                 "Second argument of NLUTgenerateNlut must be NULL or N_vardec");

    nlut = ILIBmalloc (sizeof (nlut_t));

    NLUT_MASKBASE (nlut) = DFMgenMaskBase (args, vardecs);
    NLUT_MASKBASE_RC (nlut) = ILIBmalloc (sizeof (int));
    NLUT_MASKBASE_RCVAL (nlut) = 1;
    NLUT_MASK (nlut) = DFMgenMaskClear (NLUT_MASKBASE (nlut));
    NLUT_LUT (nlut) = LUTgenerateLut ();

    /*
     * Insert zeroes into lut for every variable
     */
    avis = DFMgetMaskEntryAvisClear (NLUT_MASK (nlut));
    while (avis != NULL) {
        DBUG_ASSERT (LUTsearchInLutPp (NLUT_LUT (nlut), avis) == avis,
                     "AVIS already exists in NLUT!");

        z = ILIBmalloc (sizeof (int));
        *z = 0;

        NLUT_LUT (nlut) = LUTinsertIntoLutP (NLUT_LUT (nlut), avis, z);

        avis = DFMgetMaskEntryAvisClear (NULL);
    }

    DBUG_RETURN (nlut);
}

/******************************************************************************
 *
 * function:
 *   nlut_t *NLUTduplicateNlut( nlut_t *nlut)
 *
 * description:
 *   Duplicates an Nlut
 *
 *****************************************************************************/
nlut_t *
NLUTduplicateNlut (nlut_t *nlut)
{
    nlut_t *newnlut;

    DBUG_ENTER ("NLUTduplicateNlut");

    newnlut = ILIBmalloc (sizeof (nlut_t));
    NLUT_MASKBASE (newnlut) = NLUT_MASKBASE (nlut);
    NLUT_MASKBASE_RC (newnlut) = NLUT_MASKBASE_RC (nlut);
    NLUT_MASKBASE_RCVAL (newnlut) += 1;
    NLUT_MASK (newnlut) = DFMgenMaskCopy (NLUT_MASK (nlut));
    NLUT_LUT (newnlut) = LUTduplicateLut (NLUT_LUT (nlut));

    NLUT_LUT (newnlut) = LUTmapLutP (NLUT_LUT (newnlut), (void *(*)(void *))DupInt);

    DBUG_RETURN (newnlut);
}

/******************************************************************************
 *
 * function:
 *   nlut_t *NLUTremoveNlut( nlut_t *nlut)
 *
 * description:
 *   Removes an Nlut an all its content
 *
 *****************************************************************************/
nlut_t *
NLUTremoveNlut (nlut_t *nlut)
{
    DBUG_ENTER ("NLUTremoveNlut");

    NLUT_LUT (nlut) = LUTmapLutP (NLUT_LUT (nlut), (void *(*)(void *))FreeInt);

    NLUT_LUT (nlut) = LUTremoveLut (NLUT_LUT (nlut));
    NLUT_MASK (nlut) = DFMremoveMask (NLUT_MASK (nlut));
    NLUT_MASKBASE_RCVAL (nlut) -= 1;

    if (NLUT_MASKBASE_RCVAL (nlut) == 0) {
        NLUT_MASKBASE_RC (nlut) = ILIBfree (NLUT_MASKBASE_RC (nlut));
        NLUT_MASKBASE (nlut) = DFMremoveMaskBase (NLUT_MASKBASE (nlut));
    }

    nlut = ILIBfree (nlut);

    DBUG_RETURN (nlut);
}

/******************************************************************************
 *
 * function:
 *   int NLUTgetNum( nlut_t *nlut, node *avis)
 *
 * description:
 *   Yields the integer value associated with the given identifier
 *
 *****************************************************************************/
int
NLUTgetNum (nlut_t *nlut, node *avis)
{
    int *i;

    DBUG_ENTER ("NLUTgetNum");

    i = LUTsearchInLutPp (NLUT_LUT (nlut), avis);

    DBUG_ASSERT (((void *)i) != ((void *)avis), "AVIS does not exist in NLUT!");

    DBUG_RETURN (*i);
}

/******************************************************************************
 *
 * function:
 *   void NLUTsetNum( nlut_t *nlut, node *avis, int num)
 *
 * description:
 *   Associates the given avis with the given integer value
 *
 *****************************************************************************/
void
NLUTsetNum (nlut_t *nlut, node *avis, int num)
{
    int *i;

    DBUG_ENTER ("NLUTsetNum");

    i = LUTsearchInLutPp (NLUT_LUT (nlut), avis);

    DBUG_ASSERT ((void *)i != (void *)avis, "AVIS does not exist in NLUT!");

    *i = num;

    if (num != 0) {
        DFMsetMaskEntrySet (NLUT_MASK (nlut), NULL, avis);
    } else {
        DFMsetMaskEntryClear (NLUT_MASK (nlut), NULL, avis);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void NLUTincNum( nlut_t *nlut, node *avis, int num)
 *
 * description:
 *   Increases the integer value associated with the avis by num
 *
 *****************************************************************************/
void
NLUTincNum (nlut_t *nlut, node *avis, int num)
{
    DBUG_ENTER ("NLUTincNum");

    NLUTsetNum (nlut, avis, NLUTgetNum (nlut, avis) + num);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   dfmaskt_t *NLUTgetNonZeroMask( nlut_t *nlut)
 *
 * description:
 *   Returns a NLUT's non zero mask
 *
 *****************************************************************************/
dfmask_t *
NLUTgetNonZeroMask (nlut_t *nlut)
{
    DBUG_ENTER ("NLUTgetNonZeroMask");

    DBUG_RETURN (NLUT_MASK (nlut));
}
