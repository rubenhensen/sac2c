/*
 * $Id$
 *
 */

/*****************************************************************************
 *
 * file:   ToNewTypes.c
 *
 * prefix: TNT
 *
 * description:
 *
 *   replaces all old types by new types!
 *
 *****************************************************************************/

#include "dbug.h"
#include "ToNewTypes.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "new_types.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"

/**
 * use of INFO structure in this file:
 *
 * node*      FUNDEF        (current working fundef)
 * bool       TRAVMODE      (traversal mode: all fundefs/single fundef)
 * types      TYPES         (funtype during ret-traversal)
 */

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    int singlefundef;
    types *old_types;
};

/**
 * INFO macros
 */
#define INFO_TNT_FUNDEF(n) (n->fundef)
#define INFO_TNT_TRAVMODE(n) (n->singlefundef)
#define INFO_TNT_TYPES(n) (n->old_types)

/* TNT_TRAVMODE mode */
#define TNTSF_TRAV_FUNDEFS 0
#define TNTSF_TRAV_SPECIALS 1
#define TNTSF_TRAV_NONE 2

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_TNT_FUNDEF (result) = NULL;
    INFO_TNT_TRAVMODE (result) = TNTSF_TRAV_FUNDEFS;
    INFO_TNT_TYPES (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *  node *TNTarg( node *arg_node, info *arg_info)
 *
 * description:
 *   Checks arg node for avis attribute. if missing create an initialize
 *   new avis node. Also checks for correct back reference.
 *
 ******************************************************************************/
node *
TNTarg (node *arg_node, info *arg_info)
{
    types *old_type;

    DBUG_ENTER ("TNTarg");

    old_type = ARG_TYPE (arg_node);
    DBUG_ASSERT ((old_type != NULL), "old type in arg missing");

    if (AVIS_TYPE (ARG_AVIS (arg_node)) != NULL) {
        AVIS_TYPE (ARG_AVIS (arg_node)) = TYfreeType (AVIS_TYPE (ARG_AVIS (arg_node)));
    }
    AVIS_TYPE (ARG_AVIS (arg_node)) = TYoldType2Type (old_type);
    ARG_TYPE (arg_node) = FREEfreeAllTypes (ARG_TYPE (arg_node));

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TNTvardec( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
TNTvardec (node *arg_node, info *arg_info)
{
    types *old_type;

    DBUG_ENTER ("TNTvardec");

    old_type = VARDEC_TYPE (arg_node);
    DBUG_ASSERT ((VARDEC_TYPE (arg_node) != NULL), "old type in vardec missing");

    if (AVIS_TYPE (VARDEC_AVIS (arg_node)) != NULL) {
        AVIS_TYPE (VARDEC_AVIS (arg_node))
          = TYfreeType (AVIS_TYPE (VARDEC_AVIS (arg_node)));
    }

    AVIS_TYPE (VARDEC_AVIS (arg_node)) = TYoldType2Type (old_type);

    VARDEC_TYPE (arg_node) = FREEfreeAllTypes (old_type);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* TNTfundef( node *arg_node, info *arg_info)
 *
 * description:
 *  traverses arg nodes and block in this order.
 *
 ******************************************************************************/
node *
TNTfundef (node *arg_node, info *arg_info)
{
    types *chain;
    DBUG_ENTER ("TNTfundef");

    INFO_TNT_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /* there are some args */
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* there is a block */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    chain = FUNDEF_TYPES (arg_node);

    if (chain != NULL) {
        DBUG_ASSERT (FUNDEF_RETS (arg_node) != NULL,
                     "Rets missing despite non-NULL FUNDEF_TYPES");
        INFO_TNT_TYPES (arg_info) = chain;
        FUNDEF_TYPES (arg_node) = NULL;

        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    } else {
        DBUG_ASSERT (FUNDEF_RETS (arg_node) == NULL, "missing FUNDEF_TYPES!");
    }

    if ((INFO_TNT_TRAVMODE (arg_info) == TNTSF_TRAV_FUNDEFS)
        && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TNTret( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TNTret (node *arg_node, info *arg_info)
{
    types *old_type;

    DBUG_ENTER ("TNTret");

    old_type = INFO_TNT_TYPES (arg_info);
    DBUG_ASSERT (old_type != NULL, "fewer old types than N_ret nodes!");
    INFO_TNT_TYPES (arg_info) = TYPES_NEXT (old_type);

    if (RET_TYPE (arg_node) != NULL) {
        RET_TYPE (arg_node) = TYfreeType (RET_TYPE (arg_node));
    }
    RET_TYPE (arg_node) = TYoldType2Type (old_type);
    old_type = FREEfreeOneTypes (old_type);

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* TNTblock(node *arg_node, info *arg_info)
 *
 * description:
 *  traverses vardec nodes and assignments in this order.
 *
 ******************************************************************************/
node *
TNTblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TNTblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* there are some vardecs */
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* there is a block */
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *TNTap(node *arg_node, info *arg_info)
 *
 * description:
 *  traverses args and does a recursive call in case of special function
 *  applications.
 *
 ******************************************************************************/
node *
TNTap (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ("TNTap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion (only in single fundef mode) */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (INFO_TNT_TRAVMODE (arg_info) == TNTSF_TRAV_SPECIALS)
        && (AP_FUNDEF (arg_node) != INFO_TNT_FUNDEF (arg_info))) {
        DBUG_PRINT ("TNT", ("traverse in special fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_TNT_TRAVMODE (new_arg_info) = INFO_TNT_TRAVMODE (arg_info);

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("TNT", ("traversal of special fundef %s finished\n",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        new_arg_info = FreeInfo (new_arg_info);
    } else {
        DBUG_PRINT ("TNT", ("do not traverse in normal fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* ToNewTypes(node* syntax_tree)
 *
 * description:
 *   Starts traversal of AST to check for correct Avis nodes in vardec/arg
 *   nodes. all backrefs from N_id or IDS structures are checked for
 *   consistent values.
 *   This traversal is needed for compatiblity with old code without knowledge
 *   of the avis nodes.
 *
 ******************************************************************************/
node *
TNTdoToNewTypes (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("TNTdoToNewTypes");

    DBUG_PRINT ("OPT", ("start checking avis information"));

    if (global.compiler_phase > PH_tc) {
        arg_info = MakeInfo ();
        INFO_TNT_TRAVMODE (arg_info) = TNTSF_TRAV_FUNDEFS;

        TRAVpush (TR_tnt);
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *ToNewTypesOneFunction(node *fundef)
 *
 * description:
 *   same as ToNewTypes, but traverses only the given function including their
 *   (implicit inlined) special functions.
 *
 ******************************************************************************/
node *
TNTdoToNewTypesOneFunction (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("TNTdoToNewTypesOneFunction");

    if (global.compiler_phase > PH_tc) {

        DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                     "TNTdoToNewTypesOneFunction is used for fundef nodes only");

        if (!(FUNDEF_ISLACFUN (fundef))) {
            DBUG_PRINT ("OPT", ("starting avis check for %s", FUNDEF_NAME (fundef)));

            arg_info = MakeInfo ();
            INFO_TNT_TRAVMODE (arg_info) = TNTSF_TRAV_SPECIALS;

            TRAVpush (TR_tnt);
            fundef = TRAVdo (fundef, arg_info);
            TRAVpop ();

            arg_info = FreeInfo (arg_info);
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *ToNewTypesOneFundef(node *fundef)
 *
 * description:
 *   same as ToNewTypes, but traverses only the given single fundef. It does
 *   NOT traverse any special fundefs which may be called within that fundef.
 *
 ******************************************************************************/
node *
TNTdoToNewTypesOneFundef (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("TNTdoToNewTypesOneFundef");

    if (global.compiler_phase > PH_tc) {

        DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                     "TNTdoToNewTypesOneFundef is used for fundef nodes only");

        DBUG_PRINT ("OPT", ("starting avis check for %s", FUNDEF_NAME (fundef)));

        arg_info = MakeInfo ();
        INFO_TNT_TRAVMODE (arg_info) = TNTSF_TRAV_NONE;

        TRAVpush (TR_tnt);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}
