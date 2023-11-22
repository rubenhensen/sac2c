/**
 * @file
 * @brief utilities used by traversals dealing with Extended Memory Reuse (EMR).
 *
 */
#include "emr_utils.h"

#define DBUG_PREFIX "EMRUTIL"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "type_utils.h"
#include "free.h"

/**
 * @brief check that two ntypes have the same shape
 *
 * @param t1 NewType
 * @param t2 NewType
 * @return boolean
 */
bool
ShapeMatch (ntype *t1, ntype *t2)
{
    ntype *aks1, *aks2;
    bool res;

    DBUG_ENTER ();

    aks1 = TYeliminateAKV (t1);
    aks2 = TYeliminateAKV (t2);

    res = TYisAKS (aks1) && TYeqTypes (aks1, aks2);

    aks1 = TYfreeType (aks1);
    aks2 = TYfreeType (aks2);

    DBUG_RETURN (res);
}

/**
 * @brief find matching N_avis nodes within an N_exrps of N_id
 *
 * @param exprs N_exprs chain
 * @param id N_id node
 * @return boolean
 */
bool
doAvisMatch (node *exprs, node *id)
{
    bool res = false;

    DBUG_ENTER ();

    while (exprs) {
        if (ID_AVIS (id) == ID_AVIS (EXPRS_EXPR (exprs))) {
            res = true;
            break;
        } else
            exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (res);
}

/**
 * @brief For a given N_avis, remove destructively all occurences of it in
 *        a N_exprs chain.
 *
 * @param avis the N_avis to search for
 * @param exprs the N_exprs chain to search in
 * @return updated N_exprs chain
 */
node *
ElimDupesOfAvis (node *avis, node *exprs)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (avis) == N_avis, "First arg is not N_avis!");

    if (exprs != NULL) {
        if (EXPRS_NEXT (exprs) != NULL) {
            EXPRS_NEXT (exprs) = ElimDupesOfAvis (avis, EXPRS_NEXT (exprs));
        }

        if (ID_AVIS (EXPRS_EXPR (exprs)) == avis) {
            DBUG_PRINT ("elimating duplicate of %s", AVIS_NAME (avis));
            exprs = FREEdoFreeNode (exprs);
        }
    }

    DBUG_RETURN (exprs);
}

/**
 * @brief Remove destructively all duplicate N_avis in a N_exprs chain.
 *
 * @param exprs the N_exprs chain to search in
 * @return updated N_exprs chain
 */
node *
ElimDupes (node *exprs)
{
    DBUG_ENTER ();

    if (exprs != NULL) {
        EXPRS_NEXT (exprs)
          = ElimDupesOfAvis (ID_AVIS (EXPRS_EXPR (exprs)), EXPRS_NEXT (exprs));

        EXPRS_NEXT (exprs) = ElimDupes (EXPRS_NEXT (exprs));
    }

    DBUG_RETURN (exprs);
}

/**
 * @brief filter out N_id in N_exprs chain based on another N_exprs chain
 *
 * @param fexprs N_exprs chain of N_id that are to be found
 * @param exprs N_exprs chain of N_id that is to be filtered
 * @return N_exprs chain after filtering
 */
node *
filterDuplicateId (node *fexprs, node **exprs)
{
    node *filtered;

    DBUG_ENTER ();

    filtered = TCfilterExprsArg (doAvisMatch, fexprs, exprs);

    /* we delete all duplicate N_id in exprs */
    filtered = FREEoptFreeTree(filtered);

    DBUG_RETURN (*exprs);
}

/**
 * @brief Find a N_id in N_exprs chain that matches shape of an N_avis
 *
 * @param avis N_avis node to compare against
 * @param exprs N_expres chain with N_id
 * @return the N_id node that matches, or NULL
 */
node *
isSameShapeAvis (node *avis, node *exprs)
{
    node * ret = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (avis) == N_avis, "First arg is not N_avis!");

    if (exprs != NULL) {
        if ((ShapeMatch (AVIS_TYPE (avis), ID_NTYPE (EXPRS_EXPR (exprs)))
             || TCshapeVarsMatch (avis, ID_AVIS (EXPRS_EXPR (exprs))))
            && TUeqElementSize (AVIS_TYPE (avis), ID_NTYPE (EXPRS_EXPR (exprs)))) {
            ret = EXPRS_EXPR (exprs);
        } else {
            ret = isSameShapeAvis (avis, EXPRS_NEXT (exprs));
        }
    }

    DBUG_RETURN (ret);
}

#undef DBUG_PREFIX
