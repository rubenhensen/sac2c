/**
 *
 * $Id$
 *
 * file: check_lacfuns.c
 *
 * prefix: CHKLACF
 *
 * Description:
 *
 * This file implements a traversal of the ast that checks whether LaC functions
 * are only called once. This is an internal ast consistency check.
 *
 */

#include "check_lacfuns.h"

#include "dbug.h"
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
};

/**
 * INFO macros
 */

#define INFO_SPINE(n) ((n)->spine)
#define INFO_FUNDEF(n) ((n)->fundef)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_SPINE (result) = TRUE;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Anonymous traversal to clear FUNDEF_CALLFUN links
 */

static node *
ATravCHKLACFCmodule (node *arg_node, info *arg_info)
{
    node *foo;

    DBUG_ENTER ("ATravCHKLACFCmodule");

    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    foo = TRAVopt (DUPgetCopiedSpecialFundefsHook (), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ATravCHKLACFCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravCHKLACFCfundef");

    FUNDEF_CALLFUN (arg_node) = NULL;

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ClearCallSiteLinks (node *syntax_tree)
{
    anontrav_t dfrc_trav[4]
      = {{N_module, &ATravCHKLACFCmodule}, {N_fundef, &ATravCHKLACFCfundef}, {0, NULL}};

    DBUG_ENTER ("ClearCallSiteLinks");

    TRAVpushAnonymous (dfrc_trav, &TRAVsons);

    syntax_tree = TRAVopt (syntax_tree, NULL);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKLACFmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
CHKLACFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKLACFmodule");

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
    DBUG_ENTER ("CHKLACFfundef");

    if (INFO_SPINE (arg_info)) {
        if (!FUNDEF_ISDOFUN (arg_node) && !FUNDEF_ISCONDFUN (arg_node)) {
            INFO_FUNDEF (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_FUNDEF (arg_info) = NULL;
        }

        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (FUNDEF_CALLFUN (arg_node) == NULL) {
            FUNDEF_CALLFUN (arg_node) = INFO_FUNDEF (arg_info);
        } else {
            CTIerror ("LaC function %s called again in %s.\n"
                      "Previous call site in %s",
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
    DBUG_ENTER ("CHKLACFblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

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

    DBUG_ENTER ("CHKLACFap");

    if (!AP_ISRECURSIVEDOFUNCALL (arg_node)
        && (FUNDEF_ISDOFUN (AP_FUNDEF (arg_node))
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
 * @fn CHKLACFdoCheckLacFuns( node *syntax_tree)
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
    node *syntax_tree;

    DBUG_ENTER ("CHKLACFdoCheckLacFuns");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module) || (NODE_TYPE (arg_node) == N_fundef),
                 "Illegal argument node!");

    if (global.valid_ssaform) {
        if (NODE_TYPE (arg_node) == N_fundef) {
            syntax_tree = global.syntax_tree;
        } else {
            syntax_tree = arg_node;
        }

        CTItell (4, "         Running LaC fun check");

        info = MakeInfo ();

        TRAVpush (TR_chklacf);
        syntax_tree = TRAVdo (syntax_tree, info);
        TRAVpop ();

        info = FreeInfo (info);

        syntax_tree = ClearCallSiteLinks (syntax_tree);
    }

    DBUG_RETURN (arg_node);
}
