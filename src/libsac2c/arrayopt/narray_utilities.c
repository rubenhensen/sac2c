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
#include "copywlelim.h"

#include "globals.h"

#define DBUG_PREFIX "NAUT"
#include "debug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "pattern_match.h"
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

    if ((PMmatchFlat (patcon1, arg_node))
        && (PMmatchFlat (patcon2, EXPRS_EXPR (ARRAY_AELEMS (arg_node))))) {
        coz = COeq (con1, con2, NULL);
        z = COisTrue (coz, TRUE);
        con1 = (NULL != con1) ? COfreeConstant (con1) : NULL;
        con2 = (NULL != con2) ? COfreeConstant (con2) : NULL;
        coz = (NULL != coz) ? COfreeConstant (coz) : NULL;
    } else {
        z = TRUE;
        aelems = ARRAY_AELEMS (arg_node);
        PMmatchFlat (pat1, EXPRS_EXPR (aelems));
        while (z && (NULL != elem) && (NULL != aelems)) {
            z = PMmatchFlat (pat2, EXPRS_EXPR (aelems));
            aelems = EXPRS_NEXT (aelems);
        }
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    patcon1 = PMfree (patcon1);
    patcon2 = PMfree (patcon2);

    DBUG_RETURN (z);
}
#undef DBUG_PREFIX
