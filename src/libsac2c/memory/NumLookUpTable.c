#include "NumLookUpTable.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

/******************************************************************************
 *
 * Type definitions
 *
 *****************************************************************************/
struct NLUT_T {
    size_t size;
    int *nums;
    node **avis;
};

#define NLUT_SIZE(n) (n->size)
#define NLUT_NUMS(n) (n->nums)
#define NLUT_AVIS(n) (n->avis)

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
    node *tmp;
    int c;

    DBUG_ENTER ();

    DBUG_ASSERT ((args == NULL) || (NODE_TYPE (args) == N_arg),
                 "First argument of NLUTgenerateNlut must be NULL or N_arg");
    DBUG_ASSERT ((vardecs == NULL) || (NODE_TYPE (vardecs) == N_vardec),
                 "Second argument of NLUTgenerateNlut must be NULL or N_vardec");

    nlut = (nlut_t *)MEMmalloc (sizeof (nlut_t));

    NLUT_SIZE (nlut) = TCcountArgs (args) + TCcountVardecs (vardecs);
    NLUT_NUMS (nlut) = (int *)MEMmalloc (NLUT_SIZE (nlut) * sizeof (int));
    NLUT_AVIS (nlut) = (node **)MEMmalloc (NLUT_SIZE (nlut) * sizeof (node *));

    c = 0;
    tmp = args;
    while (tmp != NULL) {
        NLUT_NUMS (nlut)[c] = 0;
        NLUT_AVIS (nlut)[c] = ARG_AVIS (tmp);
        AVIS_VARNO (ARG_AVIS (tmp)) = c;
        c += 1;
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;
    while (tmp != NULL) {
        NLUT_NUMS (nlut)[c] = 0;
        NLUT_AVIS (nlut)[c] = VARDEC_AVIS (tmp);
        AVIS_VARNO (VARDEC_AVIS (tmp)) = c;
        c += 1;
        tmp = VARDEC_NEXT (tmp);
    }

    DBUG_RETURN (nlut);
}

/******************************************************************************
 *
 * function:
 *   nlut_t *NLUTgenerateNlutFromNlut( nlut_t *nlut)
 *
 * description:
 *   Duplicates an Nlut
 *
 *****************************************************************************/
nlut_t *
NLUTgenerateNlutFromNlut (nlut_t *nlut)
{
    size_t i;
    nlut_t *newnlut;

    DBUG_ENTER ();

    newnlut = (nlut_t *)MEMmalloc (sizeof (nlut_t));

    NLUT_SIZE (newnlut) = NLUT_SIZE (nlut);
    NLUT_NUMS (newnlut) = (int *)MEMmalloc (NLUT_SIZE (nlut) * sizeof (int));
    NLUT_AVIS (newnlut) = (node **)MEMmalloc (NLUT_SIZE (nlut) * sizeof (node *));

    for (i = 0; i < NLUT_SIZE (nlut); i++) {
        NLUT_NUMS (newnlut)[i] = 0;
        NLUT_AVIS (newnlut)[i] = NLUT_AVIS (nlut)[i];
    }

    DBUG_RETURN (newnlut);
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
    size_t i;
    nlut_t *newnlut;

    DBUG_ENTER ();

    newnlut = (nlut_t *)MEMmalloc (sizeof (nlut_t));

    NLUT_SIZE (newnlut) = NLUT_SIZE (nlut);
    NLUT_NUMS (newnlut) = (int *)MEMmalloc (NLUT_SIZE (nlut) * sizeof (int));
    NLUT_AVIS (newnlut) = (node **)MEMmalloc (NLUT_SIZE (nlut) * sizeof (node *));

    for (i = 0; i < NLUT_SIZE (nlut); i++) {
        NLUT_NUMS (newnlut)[i] = NLUT_NUMS (nlut)[i];
        NLUT_AVIS (newnlut)[i] = NLUT_AVIS (nlut)[i];
    }

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
    DBUG_ENTER ();

    NLUT_NUMS (nlut) = MEMfree (NLUT_NUMS (nlut));
    NLUT_AVIS (nlut) = MEMfree (NLUT_AVIS (nlut));

    nlut = MEMfree (nlut);

    DBUG_RETURN (nlut);
}

/******************************************************************************
 *
 * function:
 *   nlut_t *NLUTaddNluts( nlut_t *nlut1, nlut_t *nlut2)
 *
 * description:
 *   Adds the two given nluts
 *
 *****************************************************************************/
nlut_t *
NLUTaddNluts (nlut_t *nlut1, nlut_t *nlut2)
{
    size_t i;
    nlut_t *res;

    DBUG_ENTER ();

    res = NLUTduplicateNlut (nlut1);
    for (i = 0; i < NLUT_SIZE (nlut2); i++) {
        NLUT_NUMS (res)[i] += NLUT_NUMS (nlut2)[i];
    }

    DBUG_RETURN (res);
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
    int i;

    DBUG_ENTER ();

    i = NLUT_NUMS (nlut)[AVIS_VARNO (avis)];

    DBUG_RETURN (i);
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
    DBUG_ENTER ();

    NLUT_NUMS (nlut)[AVIS_VARNO (avis)] = num;

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    /**
     * If this AVIS is marked as a dummy, that means that it only exists
     * temporarily, and will be removed in a later phase. So we should skip
     * dummies.
     */
    if (!AVIS_ISDUMMY (avis)) {
        NLUT_NUMS (nlut)[AVIS_VARNO (avis)] += num;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   node *NLUTgetNonZeroAvis( nlut_t *nlut)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
NLUTgetNonZeroAvis (nlut_t *nlut)
{
    node *res;

    static nlut_t *store;
    static size_t i;

    DBUG_ENTER ();

    if (nlut != NULL) {
        store = nlut;
        i = 0;
    }

    while ((i < NLUT_SIZE (store)) && (NLUT_NUMS (store)[i] == 0)) {
        i++;
    }

    res = (i < NLUT_SIZE (store)) ? NLUT_AVIS (store)[i++] : NULL;

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
