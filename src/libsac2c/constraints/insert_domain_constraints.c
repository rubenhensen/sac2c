/*
 *
 * $Id$
 */

/**
 *
 * @file insert_domain_constraints.c
 *
 */

#include "dbug.h"

#include "types.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "constants.h"
#include "globals.h"
#include "memory.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "ctinfo.h"

#include "insert_domain_constraints.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    int level;
    node *reccall;
    node *extcall;
    node *assigns;
    node *vardecs;
    node *lastassign;
    node *precond;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_EXTCALL(n) ((n)->extcall)
#define INFO_EXTASSIGNS(n) ((n)->assigns)
#define INFO_EXTVARDECS(n) ((n)->vardecs)
#define INFO_RECCALL(n) ((n)->reccall)
#define INFO_LASTASSIGN(n) ((n)->lastassign)
#define INFO_PRECONDASSIGN(n) ((n)->precond)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_EXTCALL (result) = NULL;
    INFO_EXTASSIGNS (result) = NULL;
    INFO_EXTVARDECS (result) = NULL;
    INFO_RECCALL (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_PRECONDASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

node *
FindAvisOfLastDefinition (node *exprs)
{
    node *avis, *last_avis = NULL;
    node *expr;

    DBUG_ENTER ("FindAvisOfLastDefinition");

    while (exprs != NULL) {
        expr = EXPRS_EXPR (exprs);
        if (NODE_TYPE (expr) == N_id) {
            avis = ID_AVIS (expr);
            DBUG_ASSERT (ASSIGN_POS (AVIS_SSAASSIGN (avis)) > 0,
                         "IDCaddConstraint used before IDCinit()!");
            if ((last_avis == NULL)
                || (ASSIGN_POS (AVIS_SSAASSIGN (last_avis))
                    < ASSIGN_POS (AVIS_SSAASSIGN (avis)))) {
                last_avis = avis;
            }
        }

        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (last_avis);
}

/** <!--******************************************************************-->
 *
 * @fn  node *IDCinitialize( node *fundef, bool all)
 *
 *  @brief initializes the constraint system for the given fundef.
 *         Iff all is true apply initialization to the entire fundef chain
 *
 *  @param fundef: fundef to be initialized
 *         all:    indicator whether to be used on the entire chain
 *                 or on one fundef only
 *  @return the initialized fundef (chain)
 *
 ***************************************************************************/

node *
IDCinitialize (node *fundef, bool all)
{
    DBUG_ENTER ("IDCinitialize");
    DBUG_RETURN (fundef);
}

/** <!--******************************************************************-->
 *
 * @fn node *IDCaddTypeConstraint( ntype *type, node *avis);
 *
 *  @brief add a constraint that by means of a call to IDCinsertConstraints()
 *     will lead to the following code:
 *         id`, p = _type_constraint_( type, id)
 *     where id is an N_id that points to the given N_avis avis and p is the
 *     returned predicate.
 *     Issues a runtime error  in case IDCinit() has not been called yet.
 *     Note here, that subsequent calls may return the same predicate and
 *     it may sharpen the constraining type.
 *
 *  @param type: type constraint
 *         avis: variable that is to be constrained
 *
 *  @return N_avis node of the predicate
 *
 ***************************************************************************/

node *
IDCaddTypeConstraint (ntype *type, node *avis)
{
    node *res;
    ntype *act_type;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("IDCaddTypeConstraint");

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (type, FALSE, 0););
    DBUG_PRINT ("IDC",
                ("type constraint requested: %s for %s", tmp_str, AVIS_NAME (avis)));
    DBUG_EXECUTE ("IDC", tmp_str = MEMfree (tmp_str););

    act_type = AVIS_TYPE (avis);
    if (TYleTypes (act_type, type)) {
        DBUG_PRINT ("IDC", ("inferred type is precise enough"));
    } else {
        if (AVIS_CONSTRTYPE (avis) != NULL) {
            if (TYleTypes (AVIS_CONSTRTYPE (avis), type)) {
                DBUG_PRINT ("IDC", ("strong enough constraint exists already"));
            } else {
                AVIS_CONSTRTYPE (avis) = TYfreeType (AVIS_CONSTRTYPE (avis));
                AVIS_CONSTRTYPE (avis) = type;
                /**
                 * for getting half-decent error-msgs, we copy the pos info
                 * into the  avis from where we will spread it upon code
                 * generation
                 */
                res = AVIS_CONSTRVAR (avis);
                NODE_LINE (res) = NODE_LINE (avis);
                NODE_FILE (res) = NODE_FILE (avis);
                DBUG_PRINT ("IDC", ("replacing existing constraint"));
            }
        } else {
            AVIS_CONSTRTYPE (avis) = type;
            res = TBmakeAvis (TRAVtmpVar (),
                              TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
            AVIS_CONSTRVAR (avis) = res;
            /**
             * for getting half-decent error-msgs, we copy the pos info
             * into the  avis from where we will spread it upon code
             * generation
             */
            NODE_LINE (res) = NODE_LINE (avis);
            NODE_FILE (res) = NODE_FILE (avis);
            DBUG_PRINT ("IDC", ("constraint added"));
        }
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn node *IDCaddFunConstraint( node *expr);
 *
 *  @brief add a function constraint expr to the free variables contained in
 *     it. expr either is a (flattened) N_prf or an N_ap! By means of a call
 *     to IDCinsertConstraints() this will either lead to:
 *         a1`, ..., am`, p = prf( a1, ..., an);
 *     or it will lead to:
 *         p = udf( a1, ..., an);
 *         a1`, ..., an` = _guard_( p, a1, ..., an);
 *     depending on whether expr is  prf( a1, ..., an) or udf( a1, ..., an).
 *     Requires IDCinit() to have been called on the function that the
 *     expression is being located in.
 *
 *  @param expr: prf-expression or ap-expression that introduces a constraint
 *               to its arguments.
 *
 *  @return N_avis node of the predicate p
 *
 ***************************************************************************/

node *
IDCaddFunConstraint (node *expr)
{
    node *args, *avis, *res;

    DBUG_ENTER ("IDCaddPrfConstraint");

    DBUG_ASSERT ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_ap),
                 "illegal expr in IDCaddFunConstraint");

    DBUG_PRINT ("IDC", ("constraint requested: %s",
                        (NODE_TYPE (expr) == N_prf ? "prf" : "udf")));

    args = AP_OR_PRF_ARGS (expr);
    avis = FindAvisOfLastDefinition (args);

    if (avis == NULL) {
        CTIwarnLine (NODE_LINE (expr), "illegal requirement ignored!");
        res = NULL;
    } else {
        res = TBmakeAvis (TRAVtmpVar (),
                          TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
        AVIS_CONSTRSET (avis) = TBmakeConstraint (res, expr, AVIS_CONSTRSET (avis));
        /**
         * for getting half-decent error-msgs, we copy the pos info
         * into the  avis from where we will spread it upon code
         * generation
         */
        NODE_LINE (res) = NODE_LINE (expr);
        NODE_FILE (res) = NODE_FILE (expr);
        DBUG_PRINT ("IDC", ("constraint added to %s", AVIS_NAME (avis)));
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn node *IDCinsertConstraints( node *fundef, bool all)
 *
 *  @brief inserts all constraints that have been added since IDCinit().
 *     This includes appropriate vardec insertions and appropriate
 *     renamings of bound identifiers.
 *     The new code should be inserted as early as possible in order
 *     to maximise the "back-propagation-effect" of the constraints.
 *     Issues a runtime error  in case IDCinit() has not been called yet.
 *
 *  @param fundef: the function to be modified
 *         all:    indicator whether to be used on the entire chain
 *                 or on one fundef only
 *
 *  @return the modified function
 *
 ***************************************************************************/

node *
IDCinsertConstraints (node *fundef, bool all)
{
    node *res;

    DBUG_ENTER ("IDCinsertConstraints");
    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn node *IDCfinalize( node *fundef, bool all)
 *
 *  @brief finalizes the constraint system for the given fundef.
 *         Iff all is true apply finalization to the entire fundef chain
 *
 *  @param fundef: fundef to be ifinalized
 *         all:    indicator whether to be used on the entire chain
 *                 or on one fundef only
 *  @return the finalized fundef (chain)

 ***************************************************************************/

node *
IDCfinalize (node *fundef, bool all)
{
    DBUG_ENTER ("IDCfinalize");
    DBUG_RETURN (fundef);
}
