/*
 *
 * $Log$
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

/* build status_info.mac #### instead of this*/
/* StatusTypeAsString */
static char *
STAS (statustype status)
{
    char *result;
    DBUG_ENTER ("STAS");
    switch (status) {
    case ST_call_any:
        result = "ST_call_any";
        break;
    case ST_call_st:
        result = "ST_call_st";
        break;
    case ST_call_mt:
        result = "ST_call_mt";
        break;
    case ST_call_rep:
        result = "ST_call_rep";
        break;
    default:
        result = malloc (11);
        sprintf (result, "(%i)", (int)status);
    }
    DBUG_RETURN (result);
}

node *
MtfunsInit (node *arg_node, node *arg_info)
{
    funtab *old_tab;
    statustype old_attrib;

    DBUG_ENTER ("MtfunsInit");

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

/* MTFINst and MTFINmt */
node *
MTFINxt (node *arg_node, node *arg_info)
{
    nodetype old_attrib;

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

    DBUG_PRINT ("MTFIN",
                ("traverse into region %s", STAS (INFO_MTFIN_CURRENTATTRIB (arg_info))));
    MT_OR_ST_REGION (arg_node) = Trav (MT_OR_ST_REGION (arg_node), arg_info);
    DBUG_PRINT ("MTFIN",
                ("traverse from region %s", STAS (INFO_MTFIN_CURRENTATTRIB (arg_info))));

    INFO_MTFIN_CURRENTATTRIB (arg_info) = old_attrib;

    DBUG_RETURN (arg_node);
}

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

        if (FUNDEF_ATTRIB (old_fundef) == ST_call_any) {
            DBUG_PRINT ("MTFIN",
                        ("call_any %s %s", FUNDEF_NAME (old_fundef), STAS (ST_call_any)));

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
                                  STAS (FUNDEF_ATTRIB (old_fundef))));
            /* nothing happens */
        } else if (FUNDEF_ATTRIB (old_fundef) != INFO_MTFIN_CURRENTATTRIB (arg_info)) {
            DBUG_PRINT ("MTFIN", ("!= current %s %s", FUNDEF_NAME (old_fundef),
                                  STAS (FUNDEF_ATTRIB (old_fundef))));
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
        /* ignores repfuns #### */
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MTFINfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MTFINfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
