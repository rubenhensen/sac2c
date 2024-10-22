#include "create_wrapper_code.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "CWC"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "ct_fun.h"
#include "namespaces.h"
#include "globals.h"

/**
 **
 ** Function:
 **   node *CWCdoCreateWrapperCode( node *ast)
 **
 ** Description:
 *      - Generates the bodies of the wrapper functions.
 **
 **/

/** <!--********************************************************************-->
 *
 * @fn  bool WrapperCodeIsNeeded( node *fundef)
 *
 * @brief
 *
 * @param
 * @return
 *
 ******************************************************************************/

static bool
WrapperCodeIsNeeded (node *fundef)
{
    bool result;

    DBUG_ENTER ();

    result = FUNDEF_ISLOCAL (fundef);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn  bool WrapperCodeIsPossible( node *fundef)
 *
 * @brief
 *
 * @param
 * @return
 *
 ******************************************************************************/

static bool
WrapperCodeIsPossible (node *fundef)
{
    bool result;

    DBUG_ENTER ();

    /*
     * SAC functions with var-args are not allowed
     *   -> no wrapper functions for C functions with var-args!!
     */
    result = (!(FUNDEF_HASDOTARGS (fundef) || FUNDEF_HASDOTRETS (fundef)));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertWrapperCode( node *fundef)
 *
 * Description:
 *   Creates dispatch code for the given wrapper function and stores it in
 *   FUNDEF_BODY.
 *
 ******************************************************************************/

static node *
InsertWrapperCode (node *fundef)
{
    node *ret;
    node *assigns;
    node *vardec;
    node *vardecs1, *vardecs2;

    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (fundef) == N_fundef) && FUNDEF_ISWRAPPERFUN (fundef)
                  && (FUNDEF_BODY (fundef) == NULL)),
                 "inconsistant wrapper function found!");

    if (WrapperCodeIsNeeded (fundef) && WrapperCodeIsPossible (fundef)) {   // if a function is local it needs a wrapper
                                                                            // and if it has fixed arity, then we make a wrapper function

        DBUG_PRINT ("creating wrapper body for %s", CTIitemName (fundef));

        /*
         * generate wrapper code together with the needed vardecs
         */
        vardecs1 = TUcreateTmpVardecsFromRets (FUNDEF_RETS (fundef));
        vardecs2 = NULL;
        assigns = TYcreateWrapperCode (fundef, vardecs1, &vardecs2);

        /*
         * vardecs -> return exprs
         */
        ret = NULL;
        vardec = vardecs1;
        while (vardec != NULL) {
            ret = TBmakeExprs (TBmakeId (VARDEC_AVIS (vardec)), ret);
            vardec = VARDEC_NEXT (vardec);
        }
        FUNDEF_RETURN (fundef) = ret = TBmakeReturn (ret);

        /*
         * append return statement to assignments
         */
        assigns = TCappendAssign (assigns, TBmakeAssign (ret, NULL));

        /*
         * insert function body
         */
        FUNDEF_BODY (fundef) = TBmakeBlock (assigns, TCappendVardec (vardecs1, vardecs2));
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCfundef( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISWRAPPERFUN (arg_node)) {
        DBUG_ASSERT (FUNDEF_BODY (arg_node) == NULL,
                     "wrapper function has already a body!");

        arg_node = InsertWrapperCode (arg_node);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief returns true iff create_wrapper_code would
 *        create a wrapper body for the given fundef
 *        and thus whether the wrapper in fact can
 *        be called (non-static dispatch).
 *
 * @param fundef the fundef to inspect
 *
 * @return true iff the wrapper has a body
 */
bool
CWChasWrapperCode (node *fundef)
{
    bool result;

    DBUG_ENTER ();

    DBUG_ASSERT (FUNDEF_ISWRAPPERFUN (fundef),
                 "called CWChasWrapperCode with a non-wrapper fun");

    result = WrapperCodeIsPossible (fundef);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCdoCreateWrapperCode( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCdoCreateWrapperCode (node *ast)
{
    info *info_node;

    DBUG_ENTER ();

    global.valid_ssaform = FALSE;
    /*
     * Wrapper code is created in non-SSA form and later on transformed into
     * SSA form using the standard transformation modules lac2fun and
     * ssa_transform. Therefore, we adjust the global control flag.
     */

    TRAVpush (TR_cwc);

    info_node = NULL;
    ast = TRAVdo (ast, info_node);

    TRAVpop ();

    DBUG_RETURN (ast);
}

#undef DBUG_PREFIX
