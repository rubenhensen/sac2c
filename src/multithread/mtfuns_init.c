/*
 *
 * $Log$
 * Revision 1.5  2000/03/21 13:09:23  jhs
 * Comments.
 *
 * Revision 1.4  2000/03/15 15:49:09  dkr
 * fixed some bugs:
 *   old_attrib has type statustype now
 *   MT_OR_ST_REGION no left hand side is replaced by L_MT_OR_ST_REGION
 *
 * Revision 1.3  2000/03/09 18:33:39  jhs
 * Brushing
 *
 * Revision 1.2  2000/03/02 14:13:58  jhs
 * Using mdb_statustype now.
 *
 * Revision 1.1  2000/03/02 12:54:00  jhs
 * Initial revision
 *
 *
 * constructed by jhs@dArtagnan at home
 */

/******************************************************************************
 *
 * file:   mtfuns_init.c
 *
 * prefix: MTFIN
 *
 * description:
 *   For each function called within a N_mt a copy of this function is created.
 *   This new version of the function will be called if the original function
 *   is called within any N_mt.
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"
#include "free.h"

#include "internal_lib.h"
#include "multithread_lib.h"

#include "mtfuns_init.h"

/******************************************************************************
 *
 * function:
 *   node *MtfunsInit( node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses *only* the function handed over via arg_node with dfa_tab,
 *   will not traverse FUNDEF_NEXT( arg_node).
 *
 *   This routine ignores (returns without changes):
 *   - all functions that are not the main function!!!
 *     Then all functions could be modified.
 *
 ******************************************************************************/
node *
MtfunsInit (node *arg_node, node *arg_info)
{
    funtab *old_tab;
    statustype old_attrib;

    DBUG_ENTER ("MtfunsInit");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "MtfunsInit expects a N_fundef as arg_node");

    /*
     *  This traversal starts from function main only!!!
     *  Because the scheme with the miniphases should not be disturb
     *  all other functions are ignored within here
     */
    if (strcmp (FUNDEF_NAME (arg_node), "main") == 0) {
        old_tab = act_tab;
        act_tab = mtfin_tab;

        /* push info */
        old_attrib = INFO_MTFIN_CURRENTATTRIB (arg_info);
        INFO_MTFIN_CURRENTATTRIB (arg_info) = ST_call_st;

        FUNDEF_ATTRIB (arg_node) = ST_call_st;
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        /* pop info */
        INFO_MTFIN_CURRENTATTRIB (arg_info) = old_attrib;

        act_tab = old_tab;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTFINxt(node *arg_node, node *arg_info)
 *
 * description:
 *   This is the traversal function for N_st and N_mt!!!
 *   One does not need explicit versions MTFINst or MTFINmt here.
 *
 *   This routine set the INFO_MTFIN_CURRENTATTRIB by the actual
 *   kind of node we found.
 *   - N_mt => ST_call_mt
 *   - N_st => ST_call_st
 *   - else => fail
 *   Then it traverses the inner region, restores INFO_MTFIN_CURRENTASSIGN
 *   afterwards.
 *
 ******************************************************************************/
node *
MTFINxt (node *arg_node, node *arg_info)
{
    statustype old_attrib;

    DBUG_ENTER ("MTFINxt");

    DBUG_ASSERT (((NODE_TYPE (arg_node) == N_mt) || (NODE_TYPE (arg_node) == N_st)),
                 "wrong node type");

    old_attrib = INFO_MTFIN_CURRENTATTRIB (arg_info);

    if (NODE_TYPE (arg_node) == N_mt) {
        INFO_MTFIN_CURRENTATTRIB (arg_info) = ST_call_mt;
    } else if (NODE_TYPE (arg_node) == N_st) {
        INFO_MTFIN_CURRENTATTRIB (arg_info) = ST_call_st;
    } else {
        DBUG_ASSERT (0, ("not handled"));
    }

    DBUG_PRINT ("MTFIN", ("traverse into region with %s",
                          mdb_statustype[INFO_MTFIN_CURRENTATTRIB (arg_info)]));
    L_MT_OR_ST_REGION (arg_node, Trav (MT_OR_ST_REGION (arg_node), arg_info));
    DBUG_PRINT ("MTFIN", ("traverse from region with %s",
                          mdb_statustype[INFO_MTFIN_CURRENTATTRIB (arg_info)]));

    INFO_MTFIN_CURRENTATTRIB (arg_info) = old_attrib;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTFINlet(node *arg_node, node *arg_info)
 *
 * description:
 *   The action of the function depends on the the righ-hand-side of the
 *   let.
 *   - If we find an N_ap (ap) calling a function f (= AP_FUNDEF(ap)),
 *     we check the a = FUNDEF_ATTRIB( f) of the called function
 *     and if necessary compare it to i = INFO_MTFIN_CURRENTATTRIB( arg_info).
 *     - a = ST_call_any
 *       There has been no duplication of this function. Set the attrib
 *       of the function to i
 *     - a == i
 *       The call leads to the right function, nothing is done.
 *     - a <> i
 *       If there is no duplicate d of the function with the other kind of
 *       attrib we create d.
 *       In both cases we change the call to call d.
 *   - If we find a with-loop we do nothing (see comment below)
 *   - Anything else will be traversed.
 *
 ******************************************************************************/
node *
MTFINlet (node *arg_node, node *arg_info)
{
    node *ap;
    node *old_fundef;
    node *new_fundef;

    DBUG_ENTER ("MTFINlet");

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        ap = LET_EXPR (arg_node);

        old_fundef = AP_FUNDEF (ap);
        DBUG_ASSERT ((FUNDEF_ATTRIB (old_fundef) != ST_call_rep), ("hit call_rep"));

        if (FUNDEF_ATTRIB (old_fundef) == ST_call_any) {
            DBUG_PRINT ("MTFIN", ("call_any %s %s", FUNDEF_NAME (old_fundef),
                                  mdb_statustype[ST_call_any]));

            DBUG_ASSERT (((FUNDEF_BODY (old_fundef) != NULL)
                          || (INFO_MTFIN_CURRENTATTRIB (arg_info) = ST_call_st)),
                         ("function without body, cannot be call_mt"));

            FUNDEF_ATTRIB (old_fundef) = INFO_MTFIN_CURRENTATTRIB (arg_info);

            /* return value not needed */
            DBUG_PRINT ("MTFIN", ("traverse info fundef"));
            Trav (old_fundef, arg_info);
            DBUG_PRINT ("MTFIN", ("traverse from fundef"));
        } else if (FUNDEF_ATTRIB (old_fundef) == INFO_MTFIN_CURRENTATTRIB (arg_info)) {
            DBUG_PRINT ("MTFIN", ("== current %s %s", FUNDEF_NAME (old_fundef),
                                  mdb_statustype[FUNDEF_ATTRIB (old_fundef)]));
            /* nothing happens */
        } else if (FUNDEF_ATTRIB (old_fundef) != INFO_MTFIN_CURRENTATTRIB (arg_info)) {

            DBUG_PRINT ("MTFIN", ("!= current %s %s", FUNDEF_NAME (old_fundef),
                                  mdb_statustype[FUNDEF_ATTRIB (old_fundef)]));
            new_fundef = FUNDEF_COMPANION (old_fundef);
            if (new_fundef != NULL) {
                DBUG_PRINT ("MTFIN", ("exists"));
                ap = MUTHExchangeApplication (ap, new_fundef);
            } else {
                DBUG_PRINT ("MTFIN", ("not exists"));
                new_fundef = DupNode (old_fundef);
                FUNDEF_ATTRIB (new_fundef) = INFO_MTFIN_CURRENTATTRIB (arg_info);
                FUNDEF_COMPANION (old_fundef) = new_fundef;
                FUNDEF_COMPANION (new_fundef) = old_fundef;

                DBUG_PRINT ("MTFIN", ("hit"));
                /*
                 *  Change names.
                 */
                if (FUNDEF_ATTRIB (old_fundef) == ST_call_mt) {
                    old_fundef = MUTHExpandFundefName (old_fundef, "__CALL_MT__");
                    new_fundef = MUTHExpandFundefName (new_fundef, "__CALL_ST__");
                } else {
                    old_fundef = MUTHExpandFundefName (old_fundef, "__CALL_ST__");
                    new_fundef = MUTHExpandFundefName (new_fundef, "__CALL_MT__");
                }
                DBUG_PRINT ("MTFIN", ("hit2"));

                /*
                 *  Add new replicated function into funs
                 */
                FUNDEF_NEXT (new_fundef) = FUNDEF_NEXT (old_fundef);
                FUNDEF_NEXT (old_fundef) = new_fundef;

                DBUG_PRINT ("MTFIN", ("hit3"));
                ap = MUTHExchangeApplication (ap, new_fundef);

                DBUG_PRINT ("MTFIN", ("traverse into fundef"));
                /* return value not needed */
                Trav (new_fundef, arg_info);
                DBUG_PRINT ("MTFIN", ("traverse from fundef"));
            }
        } else {
            DBUG_ASSERT (0, ("this cannot be reached!!!"));
        }
    } else if (NODE_TYPE (LET_EXPR (arg_node)) != N_Nwith2) {
        /*
         *  We traverse all the rst, but *not* with-loops.
         *  Therefore we cannot reach repfuns.
         *  Every function called within a with-loop is a repfun by now,
         *  and these are not transformed in to st-funs or mt-funs, so this
         *  is exactly what we want.
         */
        DBUG_PRINT ("MTFIN", ("trav into with-loop"));
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        DBUG_PRINT ("MTFIN", ("trav from with-loop"));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTFINfundef (node *arg_node, node *arg_info)
 *
 * description:
 *   Traverse the body only.
 *   DO NOT TRAVERSE THE NEXT HERE!!!
 *
 ******************************************************************************/
node *
MTFINfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MTFINfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
