/*****************************************************************************
 *
 * file:   change_signature.h
 *
 * prefix: CS
 *
 * description:
 *  thismodule implements some functions to change the functions signature
 *  (add/remove args or results) of a given funtion for a given list of
 *  functions applications.
 *
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "CS"
#include "debug.h"

#include "free.h"
#include "DupTree.h"
#include "new_types.h"

/******************************************************************************
 *
 * function:
 *   static node *FreeApNarg(node *exprs, int actpos, int freepos)
 *
 * description:
 *   recursive traversal of exprs list of
 *   args to free the arg in position freepos.
 *****************************************************************************/

static node *
FreeApNarg (node *exprs, int actpos, int freepos)
{
    node *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (exprs != NULL, "unexpected end of exprs-list");

    if (actpos == freepos) {
        tmp = exprs;
        exprs = EXPRS_NEXT (exprs);

        /* free exprs-node and expression */
        FREEdoFreeNode (tmp);
    } else {
        EXPRS_NEXT (exprs) = FreeApNarg (EXPRS_NEXT (exprs), actpos + 1, freepos);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   static node *FreeFundefNarg(node *args, int actpos, int freepos)
 *
 * description:
 *   recursive traversal of args list of
 *   fundef to free the arg at freepos..
 *****************************************************************************/

static node *
FreeFundefNarg (node *args, int actpos, int freepos)
{
    node *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (args != NULL, "unexpected end of args-list");

    if (actpos == freepos) {
        tmp = args;
        args = ARG_NEXT (args);

        /* free arg-node */
        FREEdoFreeNode (tmp);
    } else {
        ARG_NEXT (args) = FreeFundefNarg (ARG_NEXT (args), actpos + 1, freepos);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   static node *FreeApNres(node *exprs, int actpos, int freepos)
 *
 * description:
 *   recurive traversal of ids list of
 *   results to free the result in position freepos.
 *****************************************************************************/

static node *
FreeApNres (node *ids, int actpos, int freepos)
{
    node *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (ids != NULL, "unexpected end of ids-list");

    if (actpos == freepos) {
        tmp = ids;
        ids = IDS_NEXT (ids);

        /* free arg-node */
        FREEdoFreeNode (tmp);
    } else {
        IDS_NEXT (ids) = FreeApNres (IDS_NEXT (ids), actpos + 1, freepos);
    }

    DBUG_RETURN (ids);
}

/******************************************************************************
 *
 * function:
 *   static node *FreeFundefNret(node *ret, int actpos, int freepos)
 *
 * description:
 *   recurive traversal of ret list of
 *   results to free the result in position freepos.
 *****************************************************************************/

static node *
FreeFundefNret (node *ret, int actpos, int freepos)
{
    node *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (ret != NULL, "unexpected end of ret-list");

    if (actpos == freepos) {
        tmp = ret;
        ret = RET_NEXT (ret);

        /* free arg-node */
        FREEdoFreeNode (tmp);
    } else {
        RET_NEXT (ret) = FreeFundefNret (RET_NEXT (ret), actpos + 1, freepos);
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   static node *FreeFundefNretExpr(node *ret, int actpos, int freepos)
 *
 * description:
 *   recurive traversal of return expressions list of
 *   results to free the result in position freepos.
 *****************************************************************************/

static node *
FreeFundefNretExpr (node *exprs, int actpos, int freepos)
{
    node *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (exprs != NULL, "unexpected end of ret-list");

    if (actpos == freepos) {
        tmp = exprs;
        exprs = RET_NEXT (exprs);

        /* free arg-node */
        FREEdoFreeNode (tmp);
    } else {
        EXPRS_NEXT (exprs) = FreeFundefNretExpr (EXPRS_NEXT (exprs), actpos + 1, freepos);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   node *CSremoveArg(node *fundef,
 *                     node *arg,
 *                     nodelist *letlist,
 *                     bool freearg)
 *
 * description:
 *   remove given arg from fundef and adjust all given applications.
 *   this function does NOT check if there are still any references to this
 *   arg.
 *   be carefull when removing args from traversing the arg chain: set
 *   freearg to FALSE and remove the arg on your own.
 *
 ******************************************************************************/

node *
CSremoveArg (node *fundef, node *arg, nodelist *letlist, bool freearg)
{
    node *funap;
    node *tmp;
    int position;

    DBUG_ENTER ();

    /* get position in arg list */
    position = 0;
    tmp = FUNDEF_ARGS (fundef);
    while (tmp != NULL) {
        position++;
        if (tmp == arg) {
            tmp = NULL; /* terminate search */
        } else {
            tmp = ARG_NEXT (tmp);
        }
    }

    DBUG_ASSERT (position > 0, "given argument not found in fundef");

    while (letlist != NULL) {
        DBUG_PRINT ("remove parameter %s in position %d", ARG_NAME (arg), position);

        /* adjust the first given function application */
        DBUG_ASSERT (NODELIST_NODE (letlist) != NULL, "no node in nodelist");
        DBUG_ASSERT (NODE_TYPE (NODELIST_NODE (letlist)) == N_let,
                     "non let node in nodelist");

        funap = LET_EXPR (NODELIST_NODE (letlist));
        DBUG_ASSERT (funap != NULL, "missing expr in let");
        DBUG_ASSERT (NODE_TYPE (funap) == N_ap, "no function application in let");
        DBUG_ASSERT (AP_FUNDEF (funap) == fundef, "application of different fundef");

        AP_ARGS (funap) = FreeApNarg (AP_ARGS (funap), 1, position);

        /* continue with next function application */
        letlist = NODELIST_NEXT (letlist);
    }

    /* no more adjustments - remove arg from fundef if flag is set */
    DBUG_PRINT ("remove arg %s in position %d", ARG_NAME (arg), position);
    if (freearg) {
        FUNDEF_ARGS (fundef) = FreeFundefNarg (FUNDEF_ARGS (fundef), 1, position);
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *CSremoveResult(node *fundef,
 *                        int position,
 *                        nodelist *letlist)
 *
 * description:
 *   removes result on given position from fundefs return and adjust all let
 *   exprs in list.
 *   also removes corresponding type from fundefs types list.
 *   be carefull removing the concering result from traversing a resultlist
 *   of this function. Do not add this let to the letlist and remove this one
 *   ids on your own.
 *
 *****************************************************************************/

node *
CSremoveResult (node *fundef, int position, nodelist *letlist)
{
    DBUG_ENTER ();

    while (letlist != NULL) {
        /*
         * adjust the first given function application
         */
        DBUG_ASSERT (NODELIST_NODE (letlist) != NULL, "no node in nodlist");
        DBUG_ASSERT (NODE_TYPE (NODELIST_NODE (letlist)) == N_let,
                     "non let node in nodelist");
        DBUG_ASSERT (LET_EXPR (NODELIST_NODE (letlist)) != NULL, "missing expr in let");
        DBUG_ASSERT (NODE_TYPE (LET_EXPR (NODELIST_NODE (letlist))) == N_ap,
                     "no function application in let");
        DBUG_ASSERT (AP_FUNDEF (LET_EXPR (NODELIST_NODE (letlist))) == fundef,
                     "application of different fundef");

        LET_IDS (NODELIST_NODE (letlist))
          = FreeApNres (LET_IDS (NODELIST_NODE (letlist)), 1, position);

        letlist = NODELIST_NEXT (letlist);
    }

    /*
     * no more adjustments - remove result from return statement
     */
    DBUG_ASSERT (FUNDEF_RETURN (fundef) != NULL, "no return statement in fundef");

    RETURN_EXPRS (FUNDEF_RETURN (fundef))
      = FreeFundefNretExpr (RETURN_EXPRS (FUNDEF_RETURN (fundef)), 1, position);

    /*
     * Remove return list entry.
     */
    FUNDEF_RETS (fundef) = FreeFundefNret (FUNDEF_RETS (fundef), 1, position);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *CSaddArg(node *fundef,
 *                  node *arg,
 *                  nodelist *letlist)
 *
 * description:
 *   adds the given new arg to the fundefs argument chain and adjusts all
 *   let-ap nodes of this fundef (given in nodelist). as you need a new
 *   parameter in all calling contexts the NODELIST_ATTRIB2 parameter that
 *   will be set by NodeListAppend(nl, node, attrib2) points to the node
 *   that should be inserted in the function application.
 *   the result is the modified fundef
 *
 *****************************************************************************/

node *
CSaddArg (node *fundef, node *arg, nodelist *letlist)
{
    node *new_exprs;

    DBUG_ENTER ();

    while (letlist != NULL) {
        DBUG_ASSERT (NODE_TYPE (LET_EXPR (NODELIST_NODE (letlist))) == N_ap,
                     "no function application");
        DBUG_ASSERT (AP_FUNDEF (LET_EXPR (NODELIST_NODE (letlist))) == fundef,
                     "call to different fundef");

        /*
         * add given node as additional argument of function application
         */
        new_exprs = TBmakeExprs (NODELIST_ATTRIB2 (letlist),
                                 AP_ARGS (LET_EXPR (NODELIST_NODE (letlist))));

        AP_ARGS (LET_EXPR (NODELIST_NODE (letlist))) = new_exprs;

        /* traverse to next application */
        letlist = NODELIST_NEXT (letlist);
    }

    /*
     * Now adjust prototype
     */
    ARG_NEXT (arg) = FUNDEF_ARGS (fundef);
    FUNDEF_ARGS (fundef) = arg;

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *CSaddResult(node *fundef, node *vardec, nodelist *letlist)
 *
 * description:
 *   adds the given vardec as additional result to the given fundef. all let-ap
 *   nodes of this fundef are adjusted to deal with the additional result.
 *   the NODELIST_ATTRIB2 parameter gives the necessary vardecs for each
 *   additional result identifier.
 * remark: the AVIS_SSAASSIGN attribute is not set by this function.
 *   if you are in ssaform, you have to correct the attribute on your own for
 *   all new identifiers.
 *
 *****************************************************************************/

node *
CSaddResult (node *fundef, node *vardec, nodelist *letlist)
{
    node *new_ids;
    node *new_id;

    DBUG_ENTER ();

    while (letlist != NULL) {
        DBUG_ASSERT (NODE_TYPE (LET_EXPR (NODELIST_NODE (letlist))) == N_ap,
                     "no function application");
        DBUG_ASSERT (AP_FUNDEF (LET_EXPR (NODELIST_NODE (letlist))) == fundef,
                     "call to different fundef");
        DBUG_ASSERT (NODE_TYPE (NODELIST_ATTRIB2 (letlist)) == N_vardec,
                     "no vardec for new result identifier");

        /*
         * create new ids for given vardec and add it as additional argument
         * of the function application.
         */

        new_ids = TBmakeIds (VARDEC_AVIS (NODELIST_ATTRIB2 (letlist)), NULL);

        LET_IDS (NODELIST_NODE (letlist))
          = TCappendIds (new_ids, LET_IDS (NODELIST_NODE (letlist)));

        /*
         * continue with next application
         */
        letlist = NODELIST_NEXT (letlist);
    }

    /*
     * all applictions adjusted, now adjust fundef:
     * 1. add additional result (given id in exprs chain)
     */
    DBUG_ASSERT (FUNDEF_RETURN (fundef) != NULL, "missing link to return statement");

    new_id = TBmakeId (VARDEC_OR_ARG_AVIS (vardec));

    RETURN_EXPRS (FUNDEF_RETURN (fundef))
      = TBmakeExprs (new_id, RETURN_EXPRS (FUNDEF_RETURN (fundef)));

    FUNDEF_RETS (fundef)
      = TCappendRet (TBmakeRet (TYcopyType (AVIS_TYPE (VARDEC_OR_ARG_AVIS (vardec))),
                                NULL),
                     FUNDEF_RETS (fundef));

    DBUG_RETURN (fundef);
}

#undef DBUG_PREFIX
