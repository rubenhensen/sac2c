/*
 *
 * $Log$
 * Revision 1.9  2002/02/22 14:28:54  dkr
 * CSAddResult: workaround for FUNDEF_NAME as a part of TYPES is no
 * longer needed :-)
 *
 * Revision 1.8  2002/02/20 14:36:55  dkr
 * function DupTypes() renamed into DupAllTypes()
 *
 * Revision 1.7  2001/04/17 15:48:35  nmw
 * AddResult implemented
 *
 * Revision 1.6  2001/04/10 15:20:09  nmw
 * wrong macro access eliminated
 *
 * Revision 1.5  2001/04/09 15:54:19  nmw
 * CSAddArg implemented
 *
 * Revision 1.4  2001/03/29 09:19:09  nmw
 * tabs2spaces done
 *
 * Revision 1.3  2001/03/22 20:00:46  dkr
 * include of tree.h eliminated
 *
 * Revision 1.2  2001/03/02 15:50:21  nmw
 * missing includes added
 *
 * Revision 1.1  2001/03/02 15:46:04  nmw
 * Initial revision
 *
 */

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
#include "internal_lib.h"
#include "dbug.h"
#include "globals.h"
#include "free.h"
#include "DupTree.h"
#include "optimize.h"

/* internal helper functions */
static node *CSFreeApNarg (node *exprs, int actpos, int freepos);
static node *CSFreeFundefNarg (node *args, int actpos, int freepos);
static ids *CSFreeApNres (ids *idslist, int actpos, int freepos);
static types *CSFreeFundefNtype (types *typelist, int actpos, int freepos);

/******************************************************************************
 *
 * function:
 *   node *CSRemoveArg(node *fundef,
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
CSRemoveArg (node *fundef, node *arg, nodelist *letlist, bool freearg)
{
    node *funap;
    node *tmp;
    int position;

    DBUG_ENTER ("CSRemoveArg");

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

    DBUG_ASSERT ((position > 0), "given argument not found in fundef");

    if (letlist != NULL) {
        DBUG_PRINT ("CS",
                    ("remove parameter %s in position %d", ARG_NAME (arg), position));

        /* adjust the first given function application */
        DBUG_ASSERT ((NODELIST_NODE (letlist) != NULL), "no node in nodlist");
        DBUG_ASSERT ((NODE_TYPE (NODELIST_NODE (letlist)) == N_let),
                     "non let node in nodelist");

        funap = LET_EXPR (NODELIST_NODE (letlist));
        DBUG_ASSERT ((funap != NULL), "missing expr in let");
        DBUG_ASSERT ((NODE_TYPE (funap) == N_ap), "no function application in let");
        DBUG_ASSERT ((AP_FUNDEF (funap) == fundef), "application of different fundef");

        AP_ARGS (funap) = CSFreeApNarg (AP_ARGS (funap), 1, position);

        /* traverse to next function application */
        fundef = CSRemoveArg (fundef, arg, NODELIST_NEXT (letlist), freearg);

    } else {
        /* no more adjustments - remove arg from fundef if flag is set */
        DBUG_PRINT ("CS", ("remove arg %s in position %d", ARG_NAME (arg), position));
        if (freearg) {
            FUNDEF_ARGS (fundef) = CSFreeFundefNarg (FUNDEF_ARGS (fundef), 1, position);
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *CSRemoveResult(node *fundef,
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
CSRemoveResult (node *fundef, int position, nodelist *letlist)
{
    char *keep_name, *keep_mod, *keep_cmod;
    statustype keep_status, keep_attrib;

    DBUG_ENTER ("CSRemoveResult");

    if (letlist != NULL) {
        /* adjust the first given function application */
        DBUG_ASSERT ((NODELIST_NODE (letlist) != NULL), "no node in nodlist");
        DBUG_ASSERT ((NODE_TYPE (NODELIST_NODE (letlist)) == N_let),
                     "non let node in nodelist");
        DBUG_ASSERT ((LET_EXPR (NODELIST_NODE (letlist)) != NULL), "missing expr in let");
        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (NODELIST_NODE (letlist))) == N_ap),
                     "no function application in let");
        DBUG_ASSERT ((AP_FUNDEF (LET_EXPR (NODELIST_NODE (letlist))) == fundef),
                     "application of different fundef");

        LET_IDS (NODELIST_NODE (letlist))
          = CSFreeApNres (LET_IDS (NODELIST_NODE (letlist)), 1, position);

        /* traverse to next function application */
        fundef = CSRemoveResult (fundef, position, NODELIST_NEXT (letlist));
    } else {
        /* no more adjustments - remove result from return statement */
        DBUG_ASSERT ((FUNDEF_RETURN (fundef) != NULL), "no return statement in fundef");
        RETURN_EXPRS (FUNDEF_RETURN (fundef))
          = CSFreeApNarg (RETURN_EXPRS (FUNDEF_RETURN (fundef)), 1, position);

        /* remove corresponding types entry - first save fundef information */
        keep_name = FUNDEF_NAME (fundef);
        keep_mod = FUNDEF_MOD (fundef);
        keep_cmod = FUNDEF_LINKMOD (fundef);
        keep_status = FUNDEF_STATUS (fundef);
        keep_attrib = FUNDEF_ATTRIB (fundef);

        FUNDEF_TYPES (fundef) = CSFreeFundefNtype (FUNDEF_TYPES (fundef), 1, position);

        if (FUNDEF_TYPES (fundef) == NULL) {
            FUNDEF_TYPES (fundef) = MakeTypes1 (T_void);
        }

        /* restore fundef information */
        FUNDEF_NAME (fundef) = keep_name;
        FUNDEF_MOD (fundef) = keep_mod;
        FUNDEF_LINKMOD (fundef) = keep_cmod;
        FUNDEF_STATUS (fundef) = keep_status;
        FUNDEF_ATTRIB (fundef) = keep_attrib;
    }
    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   static node *CSFreeApNarg(node *exprs, int actpos, int freepos)
 *
 * description:
 *   recurive traversal of exprs list of
 *   args to free the arg in position freepos.
 *****************************************************************************/
static node *
CSFreeApNarg (node *exprs, int actpos, int freepos)
{
    node *tmp;

    DBUG_ENTER ("CSFreeApNarg");

    DBUG_ASSERT ((exprs != NULL), "unexpected end of exprs-list");

    if (actpos == freepos) {
        tmp = exprs;
        exprs = EXPRS_NEXT (exprs);

        /* free exprs-node and expression */
        FreeNode (tmp);
    } else {
        EXPRS_NEXT (exprs) = CSFreeApNarg (EXPRS_NEXT (exprs), actpos + 1, freepos);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   static node *CSFreeFundefNarg(node *args, int actpos, int freepos)
 *
 * description:
 *   recursive traversal of args list of
 *   fundef to free the arg at freepos..
 *****************************************************************************/
static node *
CSFreeFundefNarg (node *args, int actpos, int freepos)
{
    node *tmp;

    DBUG_ENTER ("CSFreeFundefNarg");

    DBUG_ASSERT ((args != NULL), "unexpected end of args-list");

    if (actpos == freepos) {
        tmp = args;
        args = ARG_NEXT (args);

        /* free arg-node */
        FreeNode (tmp);
    } else {
        ARG_NEXT (args) = CSFreeFundefNarg (ARG_NEXT (args), actpos + 1, freepos);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   static node *CSFreeApNres(node *exprs, int actpos, int freepos)
 *
 * description:
 *   recurive traversal of ids list of
 *   results to free the result in position freepos.
 *****************************************************************************/
static ids *
CSFreeApNres (ids *idslist, int actpos, int freepos)
{
    ids *tmp;

    DBUG_ENTER ("CSFreeApNres");

    DBUG_ASSERT ((idslist != NULL), "unexpected end of ids-list");

    if (actpos == freepos) {
        tmp = idslist;
        idslist = IDS_NEXT (idslist);

        /* free ids */
        FreeOneIds (tmp);
    } else {
        IDS_NEXT (idslist) = CSFreeApNres (IDS_NEXT (idslist), actpos + 1, freepos);
    }

    DBUG_RETURN (idslist);
}

/******************************************************************************
 *
 * function:
 *   static node *CSFreeFundefNtype(types *typelist, int actpos, int freepos)
 *
 * description:
 *   recurive traversal of type list of
 *   results to free the type in position freepos.
 *****************************************************************************/
static types *
CSFreeFundefNtype (types *typelist, int actpos, int freepos)
{
    types *tmp;

    DBUG_ENTER ("CSFreeFundefNtype");

    DBUG_ASSERT ((typelist != NULL), "unexpected end of type-list");

    if (actpos == freepos) {
        tmp = typelist;
        typelist = TYPES_NEXT (typelist);

        /* free type */
        FreeOneTypes (tmp);
    } else {
        TYPES_NEXT (typelist)
          = CSFreeFundefNtype (TYPES_NEXT (typelist), actpos + 1, freepos);
    }

    DBUG_RETURN (typelist);
}

/******************************************************************************
 *
 * function:
 *   node *CSAddArg(node *fundef,
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
CSAddArg (node *fundef, node *arg, nodelist *letlist)
{
    node *new_exprs;

    DBUG_ENTER ("CSAddArg");

    if (letlist != NULL) {
        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (NODELIST_NODE (letlist))) == N_ap),
                     "no function application");
        DBUG_ASSERT ((AP_FUNDEF (LET_EXPR (NODELIST_NODE (letlist))) == fundef),
                     "call to different fundef");

        /* add given node as additional argument of function application */
        new_exprs = MakeExprs (NODELIST_ATTRIB2 (letlist),
                               AP_ARGS (LET_EXPR (NODELIST_NODE (letlist))));

        AP_ARGS (LET_EXPR (NODELIST_NODE (letlist))) = new_exprs;

        /* traverse to next application */
        fundef = CSAddArg (fundef, arg, NODELIST_NEXT (letlist));
    } else {
        /* all applictions adjusted, now adjust prototype */
        ARG_NEXT (arg) = FUNDEF_ARGS (fundef);
        FUNDEF_ARGS (fundef) = arg;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
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
CSAddResult (node *fundef, node *vardec, nodelist *letlist)
{
    ids *new_ids;
    node *new_id;

    DBUG_ENTER ("CSAddResult");

    if (letlist != NULL) {
        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (NODELIST_NODE (letlist))) == N_ap),
                     "no function application");
        DBUG_ASSERT ((AP_FUNDEF (LET_EXPR (NODELIST_NODE (letlist))) == fundef),
                     "call to different fundef");
        DBUG_ASSERT ((NODE_TYPE ((node *)(NODELIST_ATTRIB2 (letlist))) == N_vardec),
                     "no vardec for new result identifier");

        /*
         * create new ids for given vardec and add it as additional argument
         * of the function application.
         */

        new_ids = MakeIds_Copy (
          StringCopy (VARDEC_NAME (((node *)(NODELIST_ATTRIB2 (letlist))))));
        IDS_VARDEC (new_ids) = NODELIST_ATTRIB2 (letlist);
        IDS_AVIS (new_ids) = VARDEC_AVIS (((node *)(NODELIST_ATTRIB2 (letlist))));

        LET_IDS (NODELIST_NODE (letlist))
          = AppendIds (new_ids, LET_IDS (NODELIST_NODE (letlist)));

        /* traverse to next application */
        fundef = CSAddResult (fundef, vardec, NODELIST_NEXT (letlist));

    } else {
        /*
         * all applictions adjusted, now adjust fundef:
         * 1. add additional result (given id in exprs chain)
         * 2. add additional type to types chain
         */
        DBUG_ASSERT ((FUNDEF_RETURN (fundef) != NULL),
                     "missing link to return statement");

        new_id = MakeId_Copy (StringCopy (VARDEC_OR_ARG_NAME (vardec)));
        ID_VARDEC (new_id) = vardec;
        ID_AVIS (new_id) = VARDEC_OR_ARG_AVIS (vardec);

        RETURN_EXPRS (FUNDEF_RETURN (fundef))
          = MakeExprs (new_id, RETURN_EXPRS (FUNDEF_RETURN (fundef)));

        if (TYPES_BASETYPE (FUNDEF_TYPES (fundef)) == T_void) {
            FUNDEF_TYPES (fundef) = FreeAllTypes (FUNDEF_TYPES (fundef));
        }

        /* create new type */
        FUNDEF_TYPES (fundef)
          = AppendTypes (DupAllTypes (VARDEC_TYPE (vardec)), FUNDEF_TYPES (fundef));
    }

    DBUG_RETURN (fundef);
}
