#ifndef _SAC_MEM_EMR_UTIL_H_
#define _SAC_MEM_EMR_UTIL_H_

#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "free.h"

/**
 * @brief check that two ntypes have the same shape
 *
 * @param t1 NewType
 * @param t2 NewType
 * @return boolean
 */
static inline bool
ShapeMatch (ntype *t1, ntype *t2)
{
    ntype *aks1, *aks2;
    bool res;

    aks1 = TYeliminateAKV (t1);
    aks2 = TYeliminateAKV (t2);

    res = TYisAKS (aks1) && TYeqTypes (aks1, aks2);

    aks1 = TYfreeType (aks1);
    aks2 = TYfreeType (aks2);

    return res;
}

/**
 * @brief find matching N_avis nodes within an N_exrps of N_id
 *
 * @param exprs N_exprs chain
 * @param id N_id node
 * @return boolean
 */
static inline bool
doAvisMatch (node *exprs, node *id)
{
    while (exprs) {
        if (ID_AVIS (id) == ID_AVIS (EXPRS_EXPR (exprs)))
            return true;
        else
            exprs = EXPRS_NEXT (exprs);
    }
    return false;
}

/**
 * @brief filter out N_id in N_exprs chain based on another N_exprs chain
 *
 * @param fexprs N_exprs chain of N_id that are to be found
 * @param exprs N_exprs chain of N_id that is to be filtered
 * @return N_exprs chain after filtering
 */
static inline node *
filterDuplicateId (node *fexprs, node **exprs)
{
    node * filtered;

    filtered = TCfilterExprsArg (doAvisMatch, fexprs, exprs);

    /* we delete all duplicate N_id in exprs */
    if (filtered != NULL) {
        filtered = FREEdoFreeTree (filtered);
    }

    return *exprs;
}

#endif /* _SAC_MEM_EMR_UTIL_H_ */
