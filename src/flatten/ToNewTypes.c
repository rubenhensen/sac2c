/*
 * $Log$
 * Revision 1.3  2004/11/24 14:58:23  mwe
 * SacDevCamp: compiles!!!
 *
 * Revision 1.2  2004/11/19 13:41:00  mwe
 * TravSons in TNTarray, TNTcast, TNTtypedef added
 *
 * Revision 1.1  2004/11/18 14:33:57  mwe
 * Initial revision
 *
 * Revision 1.6  2004/09/30 15:14:39  sbs
 * eliminated FunTypes from ALL but wrapper functions
 * (memory concerns!)
 * Now, the function signatures of individual instances are
 * stored in the AVIS_TYPE and FUNDEF_RET_TYPE only!!!!!
 *
 * Revision 1.5  2004/09/29 13:47:32  sah
 * extended the ntype generation to
 * function and return types
 *
 * Revision 1.4  2004/08/29 18:08:38  sah
 * added some DBUG_PRINTS
 *
 * Revision 1.3  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PE I
 *
 * Revision 1.2  2004/07/05 17:22:22  sbs
 * some DBUG_PRINTs added.
 *
 * Revision 1.1  2004/01/28 17:01:32  skt
 * Initial revision
 *
 *
 *
 *
 *
 ************ Attention! ************
 * File was moved from ../tree
 * following older Revisions can be found there
 *
 *
 * Revision 1.12  2003/09/25 18:35:37  dkr
 * ntype for N_arg and N_vardec are build from oldtypes now
 *
 * Revision 1.11  2002/10/16 14:33:20  sbs
 * CAVobjdef added.
 *
 * Revision 1.10  2002/08/05 09:52:04  sbs
 * eliminated the requirement for Nwithid nodes to always have both,
 * an IDS and a VEC attribute. This change is motivated by the requirement
 * to convert to SSA form prior to type checking. Furthermore, not being
 * restricted to the AKS case anymore, we cannot guarantee the existance
 * of the IDS attribute in all cases anymore !!!!
 *
 * Revision 1.9  2001/05/17 11:18:10  dkr
 * FREE(info) replaced by FreeTree(info)
 *
 * Revision 1.8  2001/04/24 16:08:12  nmw
 * CheckAvisSingleFundef renamed to CheckAvisOneFunction
 * CheckAvisOneFundef added
 *
 * Revision 1.7  2001/04/18 12:58:47  nmw
 * additional traversal setup function for single fundef traversal added
 *
 * Revision 1.6  2001/03/29 09:12:38  nmw
 * tabs2spaces done
 *
 * Revision 1.5  2001/03/22 20:03:49  dkr
 * include of tree.h eliminated
 *
 * Revision 1.4  2001/02/20 15:51:02  nmw
 * debug output added
 *
 * Revision 1.3  2001/02/15 16:56:36  nmw
 * some DBUG_ASSERTS added
 *
 * Revision 1.2  2001/02/13 15:16:34  nmw
 * CheckAvis traversal implemented
 *
 * Revision 1.1  2001/02/12 16:59:27  nmw
 * Initial revision
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
 *   This module restores the AVIS attribute in N_id, N_vardec/N_arg
 *   when old code did not updates all references correctly.
 *
 *
 *****************************************************************************/
#define NEW_INFO

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "new_types.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "ToNewTypes.h"

/**
 * use of INFO structure in this file:
 *
 * node*      FUNDEF            (current working fundef)
 * bool       SINGLEFUNDEF      (traversal mode: all fundefs/single fundef)
 */

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool singlefundef;
};

/**
 * INFO macros
 */
#define INFO_TNT_FUNDEF(n) (n->fundef)
#define INFO_TNT_SINGLEFUNDEF(n) (n->singlefundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TNT_FUNDEF (result) = NULL;
    INFO_TNT_SINGLEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/* TNT_SINGLEFUNDEF mode */
#define TNTSF_TRAV_FUNDEFS 0
#define TNTSF_TRAV_SPECIALS 1
#define TNTSF_TRAV_NONE 2

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
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("TNTarg");

    DBUG_ASSERT ((ARG_AVIS (arg_node) != NULL), "Missing avis in arg found!");

    DBUG_ASSERT ((AVIS_DECL (ARG_AVIS (arg_node)) == arg_node),
                 "wrong backreference from avis to arg node!");

    /* create ntype if it is missing */
    if (AVIS_TYPE (ARG_AVIS (arg_node)) == NULL) {
        if (ARG_NAME (arg_node) != NULL) {
            DBUG_PRINT ("TNT", ("missing ntype in arg %s added.", ARG_NAME (arg_node)));
            DBUG_ASSERT ((ARG_TYPE (arg_node) != NULL), "old type in arg missing");
            AVIS_TYPE (ARG_AVIS (arg_node)) = TYoldType2Type (ARG_TYPE (arg_node));
        } else {
            /* TYOldType2Type() can not handle T_dots yet... 8-(( */
            DBUG_PRINT ("TNT", ("Found arg of type T_dots"));
        }
    } else {
        DBUG_EXECUTE ("TNT", tmp_str = TYtype2String (AVIS_TYPE (ARG_AVIS (arg_node)),
                                                      FALSE, 0););

        DBUG_PRINT ("TNT", ("reusing ntype %s of arg %s", tmp_str, ARG_NAME (arg_node)));
        DBUG_EXECUTE ("TNT", tmp_str = ILIBfree (tmp_str););
    }

#ifdef MWE_NTYPE_READY
    if ((ARG_TYPE (arg_node) != NULL) && (compiler_phase > PH_typecheck)) {
        ARG_TYPE (arg_node) = FREEfreeAllTypes (ARG_TYPE (arg_node));
    }
#endif

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
 *   Checks vardec node for avis attribute. if missing create an initialize
 *   new avis node. Also checks for correct back reference.
 *
 ******************************************************************************/
node *
TNTvardec (node *arg_node, info *arg_info)
{
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("TNTvardec");

    DBUG_ASSERT ((VARDEC_AVIS (arg_node) != NULL), "Missing avis node in vardec found");

    DBUG_ASSERT ((AVIS_DECL (VARDEC_AVIS (arg_node)) == arg_node),
                 "wrong backreference from avis to vardec node!");

    /* create ntype if it is missing */
    if (AVIS_TYPE (VARDEC_AVIS (arg_node)) == NULL) {
        DBUG_PRINT ("TNT", ("missing ntype in vardec %s added.", VARDEC_NAME (arg_node)));
        DBUG_ASSERT ((VARDEC_TYPE (arg_node) != NULL), "old type in vardec missing");
        AVIS_TYPE (VARDEC_AVIS (arg_node)) = TYoldType2Type (VARDEC_TYPE (arg_node));
    } else {
        DBUG_EXECUTE ("TNT", tmp_str = TYtype2String (AVIS_TYPE (VARDEC_AVIS (arg_node)),
                                                      FALSE, 0););

        DBUG_PRINT ("TNT",
                    ("reusing ntype %s of vardec %s", tmp_str, VARDEC_NAME (arg_node)));
        DBUG_EXECUTE ("TNT", tmp_str = ILIBfree (tmp_str););
    }

#ifdef MWE_NTYPE_READY
    if ((VARDEC_TYPE (arg_node) != NULL) && (compiler_phase > PH_typecheck))
        VARDEC_TYPE (arg_node) = FREEfreeAllTypes (VARDEC_TYPE (arg_node));
#endif

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TNTobjdef(node *arg_node, info *arg_info)
 *
 * description:
 *   Checks objdef node for avis attribute. if missing create an initialize
 *   new avis node. Also checks for correct back reference.
 *   Create ntype-structure in avis attribute if it is missing.
 *
 ******************************************************************************/
node *
TNTobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TNTobjdef");

    DBUG_ASSERT ((OBJDEF_AVIS (arg_node) != NULL), "missing avis in objdef found.");

    DBUG_ASSERT ((AVIS_DECL (OBJDEF_AVIS (arg_node)) == arg_node),
                 "wrong backreference from avis to objdef node!");

    DBUG_ASSERT ((AVIS_TYPE (OBJDEF_AVIS (arg_node)) != NULL),
                 "missing ntype in avis found");

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TNTid(node *arg_node, info *arg_info)
 *
 * description:
 *   checks for consistent back reference from N_id node to N_arg or N_vardec
 *   node together with back reference to N_avis node. Here implemented in
 *   TNTids.
 *
 ******************************************************************************/
node *
TNTid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TNTid");

    if (ID_AVIS (arg_node) != DECL_AVIS (AVIS_DECL (ID_AVIS (arg_node)))) {
        /* wrong back reference */
        DBUG_PRINT ("TNT", ("backreference from ids %s to N_avis (" F_PTR ") corrected.",
                            AVIS_NAME (ID_AVIS (arg_node)), (ID_AVIS (arg_node))));
        ID_AVIS (arg_node) = DECL_AVIS (AVIS_DECL (ID_AVIS (arg_node)));
    } else {
        DBUG_PRINT ("TNT", ("backreference from ids %s to N_avis ok.",
                            AVIS_NAME (ID_AVIS (arg_node))));
    }

    DBUG_ASSERT ((ID_AVIS (arg_node) != NULL), "AVIS reference still unset.");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TNTlet(node *arg_node, info *arg_info)
 *
 * description:
 *   starts traversal in ids chain.
 *
 ******************************************************************************/
node *
TNTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TNTlet");

    if (LET_IDS (arg_node) != NULL) {
        /* there are some ids */
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TNTNwithid(node *arg_node, info *arg_info)
 *
 * description:
 *   starts traversal for ids chains in Nwithid nodes.
 *
 ******************************************************************************/
node *
TNTwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TNTNwithid");

    if (WITHID_VEC (arg_node) != NULL) {
        WITHID_VEC (arg_node) = TRAVdo (WITHID_VEC (arg_node), arg_info);
    }

    if (WITHID_IDS (arg_node)) {
        WITHID_IDS (arg_node) = TRAVdo (WITHID_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TNTarray *arg_node, info *arg_info)
 *
 * description:
 *   sets ntype-structure if missing and remove types-structure
 *
 ******************************************************************************/
node *
TNTarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TNTarray");

    if (ARRAY_NTYPE (arg_node) == NULL) {

        DBUG_ASSERT ((ARRAY_TYPE (arg_node) != NULL),
                     "whether 'types' or 'ntype' structure found");

        ARRAY_NTYPE (arg_node) = TYoldType2Type (ARRAY_TYPE (arg_node));
    }

#ifdef MWE_NTYPE_READY
    if ((ARRAY_TYPE (arg_node) != NULL) && (compiler_phase > PH_typecheck))
        ARRAY_TYPE (arg_node) = FREEfreeAllTypes (ARRAY_TYPE (arg_node));
#endif

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TNTcast *arg_node, info *arg_info)
 *
 * description:
 *   sets ntype-structure if missing and remove types-structure
 *
 ******************************************************************************/
node *
TNTcast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TNTcast");

    DBUG_ASSERT ((CAST_NTYPE (arg_node) != NULL), "N_cast without ntype found");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TNTtypedef *arg_node, info *arg_info)
 *
 * description:
 *   sets ntype-structure if missing and remove types-structure
 *
 ******************************************************************************/
node *
TNTtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TNTtypedef");

    DBUG_ASSERT ((TYPEDEF_NTYPE (arg_node) != NULL), "N_typedef without ntype found");

    if (TYPEDEF_NEXT (arg_node) != NULL)
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

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
    node *ret, *tmp, *tmp2;
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

    if ((INFO_TNT_SINGLEFUNDEF (arg_info) == TNTSF_TRAV_FUNDEFS)
        && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if (FUNDEF_RETS (arg_node) == NULL) {
        DBUG_ASSERT ((FUNDEF_TYPES (arg_node) != NULL), "old type in arg missing");

        chain = FUNDEF_TYPES (arg_node);

        tmp2 = ret = TBmakeRet (TYoldType2Type (chain), NULL);
        chain = TYPES_NEXT (chain);
        while (chain != NULL) {
            tmp = TBmakeRet (TYoldType2Type (chain), NULL);
            RET_NEXT (tmp2) = tmp;
            tmp2 = tmp;
            chain = TYPES_NEXT (chain);
        }

        FUNDEF_RETS (arg_node) = ret;
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
        && (INFO_TNT_SINGLEFUNDEF (arg_info) == TNTSF_TRAV_SPECIALS)
        && (AP_FUNDEF (arg_node) != INFO_TNT_FUNDEF (arg_info))) {
        DBUG_PRINT ("TNT", ("traverse in special fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_TNT_SINGLEFUNDEF (new_arg_info) = INFO_TNT_SINGLEFUNDEF (arg_info);

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
 *   node *TNTids(node *arg_ids, info *arg_info)
 *
 * description:
 *   checks for consistent back reference from ids node to N_arg or N_vardec
 *   node together with back reference to N_avis node. Traversal sheme like
 *   AST Trav.
 *
 ******************************************************************************/
node *
TNTids (node *arg_ids, info *arg_info)
{
    DBUG_ENTER ("TNTids");

    if (IDS_AVIS (arg_ids) != DECL_AVIS (AVIS_DECL (IDS_VARDEC (arg_ids)))) {
        /* wrong back reference */
        DBUG_PRINT ("TNT", ("backreference from ids %s to N_avis (" F_PTR ") corrected.",
                            AVIS_NAME (IDS_AVIS (arg_ids)), IDS_AVIS (arg_ids)));
        IDS_AVIS (arg_ids) = DECL_AVIS (AVIS_DECL (IDS_AVIS (arg_ids)));
    } else {
        DBUG_PRINT ("TNT", ("backreference from ids %s to N_avis ok.",
                            AVIS_NAME (IDS_AVIS (arg_ids))));
    }

    DBUG_ASSERT ((IDS_AVIS (arg_ids) != NULL), "AVIS reference still unset.");

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TRAVdo (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
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

    arg_info = MakeInfo ();
    INFO_TNT_SINGLEFUNDEF (arg_info) = TNTSF_TRAV_FUNDEFS;

    TRAVpush (TR_tnt);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

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

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "TNTdoToNewTypesOneFunction is used for fundef nodes only");

    if (!(FUNDEF_ISLACFUN (fundef))) {
        DBUG_PRINT ("OPT", ("starting avis check for %s", FUNDEF_NAME (fundef)));

        arg_info = MakeInfo ();
        INFO_TNT_SINGLEFUNDEF (arg_info) = TNTSF_TRAV_SPECIALS;

        TRAVpush (TR_tnt);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *ToNewTypesOneFundef(node *fundef)
 *
 * description:
 *   same as ToNewTypes, but traverses only the given single fundef but not the
 *   called included special fundefs.
 *
 ******************************************************************************/
node *
TNTdoToNewTypesOneFundef (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("TNTdoToNewTypesOneFundef");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "TNTdoToNewTypesOneFundef is used for fundef nodes only");

    DBUG_PRINT ("OPT", ("starting avis check for %s", FUNDEF_NAME (fundef)));

    arg_info = MakeInfo ();
    INFO_TNT_SINGLEFUNDEF (arg_info) = TNTSF_TRAV_NONE;

    TRAVpush (TR_tnt);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (fundef);
}
