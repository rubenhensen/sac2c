/*
 *
 * $Log$
 * Revision 1.5  2004/11/24 14:05:19  ktr
 * MakeLet permutation.
 *
 * Revision 1.4  2004/11/23 22:15:12  ktr
 * renaming done.
 *
 * Revision 1.3  2004/11/23 20:33:20  ktr
 * COMPILES!!!
 *
 * Revision 1.2  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.1  2004/11/09 22:15:04  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup ec Explicit Copy
 * @ingroup rc
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file explicitcopy.c
 *
 *
 */
#include "explicitcopy.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "new_types.h"
#include "internal_lib.h"

/**
 * INFO structure
 */
struct INFO {
    node *preassign;
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_EMEC_PREASSIGN(n) (n->preassign)
#define INFO_EMEC_FUNDEF(n) (n->fundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_EMEC_PREASSIGN (result) = NULL;
    INFO_EMEC_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECdoExplicitCopy( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMECdoExplicitCopy (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMECdoExplicitCopy");

    DBUG_PRINT ("EMEC", ("Starting explicit copy traversal."));

    info = MakeInfo ();

    TRAVpush (TR_emec);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("EMEC", ("Explicit copy traversal complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Static helper funcions
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CreateCopyId(node *arg_node, info *arg_info)
 *
 * @brief CreateCopyId( a, arg_info) will append INFO_EMEC_PREASSIGN with
 *        a' = copy( a); and returns a'.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
static node *
CreateCopyId (node *oldid, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("CreateCopyId");

    /*
     * Create a new variable for b'
     */
    avis = TBmakeAvis (ILIBtmpVarName (ID_NAME (oldid)),
                       TYcopyType (AVIS_TYPE (ID_AVIS (oldid))));

    FUNDEF_VARDEC (INFO_EMEC_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_EMEC_FUNDEF (arg_info)));

    /*
     * Create copy operation
     */
    INFO_EMEC_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), TCmakePrf1 (F_copy, oldid)),
                      INFO_EMEC_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (avis) = INFO_EMEC_PREASSIGN (arg_info);

    /*
     * Replace oldid with newid
     */
    oldid = TBmakeId (avis);

    DBUG_RETURN (oldid);
}

/******************************************************************************
 *
 * Explicit copy traversal (emec_tab)
 *
 * prefix: EMEC
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMECassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMECassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMECassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_EMEC_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_EMEC_PREASSIGN (arg_info), arg_node);
        INFO_EMEC_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMECfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMECfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("EMEC", ("Traversing function %s", FUNDEF_NAME (arg_node)));

        INFO_EMEC_FUNDEF (arg_info) = arg_node;

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("EMEC", ("Traversing function %s complete.", FUNDEF_NAME (arg_node)));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECprf(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMECprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMECprf");

    switch (PRF_PRF (arg_node)) {
    case F_modarray:
    case F_idx_modarray:
        /*
         * Example:
         *
         * a = modarray( b, iv, val);
         *
         * is transformed info
         *
         * b' = copy( b);
         * a  = modarray( b', iv, val);
         */
        PRF_ARG1 (arg_node) = CreateCopyId (PRF_ARG1 (arg_node), arg_info);
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/* @} */
