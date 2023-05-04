/** <!--********************************************************************-->
 *
 * @defgroup naut N_array utility functions
 *
 *  Overview: These functions are intended to provide useful
 *            manipulation servieces for N_arrays.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file narray_utilities.c
 *
 * Prefix: NAUT
 *
 *****************************************************************************/

#include "globals.h"

#define DBUG_PREFIX "NAUT"
#include "debug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "pattern_match.h"
#include "print.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "new_types.h"
#include "constants.h"
#include "new_typecheck.h"
#include "narray_utilities.h"

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!-- ****************************************************************** -->
 * @fn bool NAUTisAllElemsSame( node *arg_node)
 *
 * @brief Predicate for determining that all elements of an N_array
 *        have the same value, e.g., [ true, true, true] or [ 2, 2, 2, 2]
 *
 * @param arg_node, assumed to have at least one element.
 *
 * @return TRUE if predicate holds; else FALSE.
 *
 ******************************************************************************/
bool
NAUTisAllElemsSame (node *arg_node)
{
    bool z = FALSE;
    node *elem = NULL;
    node *aelems;
    pattern *pat1;
    pattern *pat2;
    pattern *patcon1;
    pattern *patcon2;
    constant *con1 = NULL;
    constant *con2 = NULL;
    constant *coz;

    DBUG_ENTER ();

    pat1 = PMvar (1, PMAgetNode (&elem), 0);
    pat2 = PMvar (1, PMAisVar (&elem), 0);
    patcon1 = PMconst (1, PMAgetVal (&con1));
    patcon2 = PMconst (1, PMAgetVal (&con2));

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_array,
                 "NAUTisAllElemsSame called with other than N_array node");
    DBUG_PRINT ("checking:");
    DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node););
    if ((PMmatchFlat (patcon1, arg_node))
        && (PMmatchFlat (patcon2, EXPRS_EXPR (ARRAY_AELEMS (arg_node))))) {
        coz = COeq (con1, con2, NULL);
        z = COisTrue (coz, TRUE);
        coz = COfreeConstant (coz);
    } else {
        aelems = ARRAY_AELEMS (arg_node);
        PMmatchFlat (pat1, EXPRS_EXPR (aelems));
        z = (elem != NULL);
        while (z && (NULL != aelems)) {
            z = PMmatchFlat (pat2, EXPRS_EXPR (aelems));
            aelems = EXPRS_NEXT (aelems);
        }
    }
    DBUG_PRINT ("%s", z?"identical!":"possibly not identical!");

    con1 = (NULL != con1) ? COfreeConstant (con1) : NULL;
    con2 = (NULL != con2) ? COfreeConstant (con2) : NULL;
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    patcon1 = PMfree (patcon1);
    patcon2 = PMfree (patcon2);

    DBUG_RETURN (z);
}
#undef DBUG_PREFIX

/** <!--********************************************************************-->
 *
 * @fn NAUTisMemberArray( node *arg_node)
 *
 * @brief Set membership for Boolean N_array node.
 *
 * @params arg_node:
 *
 * @result: NAUTisMemberArray: TRUE if any element of the
 *          N_array matches tf. Otherwise FALSE.
 *
 *          The idea here is that we may have something like
 *
 *            narr = [ TRUE, DONTKNOW];
 *
 *          Hence, checking for a constant won't always work here.
 *
 * NB. A FALSE result does NOT mean that the set membership
 *     does not hold. It may only mean "don't know".
 *
 *****************************************************************************/
bool
NAUTisMemberArray (bool tf, node *arg_node)
{
    bool z = FALSE;
    node *aelems = NULL;
    constant *con = NULL;
    pattern *patcon;
    pattern *patarr;
    node *array = NULL;

    DBUG_ENTER ();

    patcon = PMconst (1, PMAgetVal (&con));
    patarr = PMarray (1, PMAgetNode (&array), 1, PMskip (0));

    if (PMmatchFlat (patarr, arg_node)) {
        aelems = ARRAY_AELEMS (array);
    }

    while ((!z) && (aelems != NULL)) {
        DBUG_ASSERT (NODE_TYPE (aelems) == N_exprs, "no N_exprs node found!");
        z = PMmatchFlat (patcon, EXPRS_EXPR (aelems))
            && (tf ? COisTrue (con, TRUE) : COisFalse (con, TRUE));
        aelems = EXPRS_NEXT (aelems);
        con = (NULL != con) ? COfreeConstant (con) : NULL;
    }

    patcon = PMfree (patcon);
    patarr = PMfree (patarr);

    DBUG_RETURN (z);
}
