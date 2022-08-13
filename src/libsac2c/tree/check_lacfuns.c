/**
 *
 * @file: check_lacfuns.c
 *
 * prefix: CHKLACF
 *
 * This file implements a traversal of the ast that checks whether LaC functions
 * are only called once. This is an internal ast consistency check.
 *
 */

#include "check_lacfuns.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "ctinfo.h"
#include "globals.h"
#include "memory.h"
#include "DupTree.h"

/**
 * INFO structure
 */

struct INFO {
    bool spine;
    node *fundef;
    node *regularfundef;
};

/**
 * INFO macros
 */

#define INFO_SPINE(n) ((n)->spine)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_REGULARFUNDEF(n) ((n)->regularfundef)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SPINE (result) = TRUE;
    INFO_FUNDEF (result) = NULL;
    INFO_REGULARFUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Anonymous traversal to clear FUNDEF_CALLFUN links
 */

static node *
ATravCHKLACFCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ATravCHKLACFCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_CALLFUN (arg_node) = NULL;

    DBUG_ASSERT (((NULL == FUNDEF_LOCALFUNS (arg_node))
                  || (N_fundef == NODE_TYPE (FUNDEF_LOCALFUNS (arg_node)))),
                 "Non-N_fundef on FUNDEF_LOCALFUNS chain");
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    DBUG_ASSERT (((NULL == FUNDEF_NEXT (arg_node))
                  || (N_fundef == NODE_TYPE (FUNDEF_NEXT (arg_node)))),
                 "Non-N_fundef on FUNDEF_NEXT chain");
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ClearCallSiteLinks (node *arg_node)
{
    anontrav_t dfrc_trav[4] = {{N_module, &ATravCHKLACFCmodule},
                               {N_fundef, &ATravCHKLACFCfundef},
                               {(nodetype)0, NULL}};

    DBUG_ENTER ();

    TRAVpushAnonymous (dfrc_trav, &TRAVsons);

    arg_node = TRAVopt (arg_node, NULL);
    TRAVopt (DUPgetCopiedSpecialFundefsHook (), NULL);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKLACFmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
CHKLACFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKLACFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
CHKLACFfundef (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ();

    if (INFO_SPINE (arg_info)) {
        if (!FUNDEF_ISLOOPFUN (arg_node) && !FUNDEF_ISCONDFUN (arg_node)) {
            INFO_FUNDEF (arg_info) = arg_node;
            INFO_REGULARFUNDEF (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_REGULARFUNDEF (arg_info) = NULL;
            INFO_FUNDEF (arg_info) = NULL;
        }

        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (global.local_funs_grouped) {
            fundef = FUNDEF_LOCALFUNS (INFO_REGULARFUNDEF (arg_info));
            while ((fundef != NULL) && (fundef != arg_node)) {
                fundef = FUNDEF_NEXT (fundef);
            }

            if (fundef == NULL) {
                fundef = DUPgetCopiedSpecialFundefsHook ();
                while ((fundef != NULL) && (fundef != arg_node)) {
                    fundef = FUNDEF_NEXT (fundef);
                }
            }

            if (fundef == NULL) {
                CTIerror (EMPTY_LOC, "LaC function %s called in regular function %s, "
                          "but not a member of regular function's local "
                          "function set or on CopiedSpecialFundefsHook",
                          FUNDEF_NAME (arg_node),
                          FUNDEF_NAME (INFO_REGULARFUNDEF (arg_info)));
            }
        }

        if (FUNDEF_CALLFUN (arg_node) == NULL) {
            FUNDEF_CALLFUN (arg_node) = INFO_FUNDEF (arg_info);
        } else {
            // FIXME: it would be nice to provide more context here
            CTIerror (EMPTY_LOC, 
                      "LaC function %s called again in %s.\n"
                      "Previous call site in ...",
                      FUNDEF_NAME (arg_node), FUNDEF_NAME (FUNDEF_CALLFUN (arg_node)));
        }

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKLACFblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
CHKLACFblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKLACFap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
CHKLACFap (node *arg_node, info *arg_info)
{
    bool spine;

    DBUG_ENTER ();

    if (!AP_ISRECURSIVEDOFUNCALL (arg_node)
        && (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))
            || FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node)))) {
        spine = INFO_SPINE (arg_info);
        INFO_SPINE (arg_info) = FALSE;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_SPINE (arg_info) = spine;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn CHKLACFdoCheckLacFuns( node *arg_node)
 *
 *  @brief initiates traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
CHKLACFdoCheckLacFuns (node *arg_node)
{
    info *info;
    node *keep_next = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module) || (NODE_TYPE (arg_node) == N_fundef),
                 "Illegal argument node!");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module) || global.local_funs_grouped,
                 "If run fun-based, special funs must be grouped.");

    if (global.valid_ssaform) {
        if (NODE_TYPE (arg_node) == N_fundef) {
            /*
             * If this check is called function-based, we do not want to traverse
             * into the next fundef, but restrict ourselves to this function and
             * its subordinate special functions.
             */
            keep_next = FUNDEF_NEXT (arg_node);
            FUNDEF_NEXT (arg_node) = NULL;
        }

        CTItell (4, "         Running LaC fun check");

        info = MakeInfo ();

        TRAVpush (TR_chklacf);
        arg_node = TRAVdo (arg_node, info);
        TRAVpop ();

        info = FreeInfo (info);

        arg_node = ClearCallSiteLinks (arg_node);

        if (NODE_TYPE (arg_node) == N_fundef) {
            /*
             * If this check is called function-based, we must restore the original
             * fundef chain here.
             */
            FUNDEF_NEXT (arg_node) = keep_next;
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
