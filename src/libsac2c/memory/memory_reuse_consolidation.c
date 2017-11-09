/**
 * @defgroup mrc Memory Reuse Consolidation
 *
 * @ingroup mrc
 *
 * @{
 */

/**
 * @file memory_reuse_consolidation.c
 *
 * Prefix: MRC
 */
#include "memory_reuse_consolidation.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"

#define DBUG_PREFIX "MRC"
#include "debug.h"

#include "print.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"

/*
 * INFO structure
 */
struct INFO {
    node *grc;
};

/*
 * INFO macros
 */
#define INFO_GRC(x) ((x)->grc)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_GRC (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static bool
ShapeMatch (ntype *t1, ntype *t2)
{
    ntype *aks1, *aks2;
    bool res;

    DBUG_ENTER ();

    aks1 = TYeliminateAKV (t1);
    aks2 = TYeliminateAKV (t2);

    res = TYisAKS (aks1) && TYeqTypes (aks1, aks2);

    aks1 = TYfreeType (aks1);
    aks2 = TYfreeType (aks2);

    DBUG_RETURN (res);
}

static bool
MRChaveSameShape (node *a_expr, node *b_expr)
{
    return ((ShapeMatch (ID_NTYPE (a_expr), IDS_NTYPE (b_expr))
             || TCshapeVarsMatch (ID_AVIS (a_expr), IDS_AVIS (b_expr)))
            && TUeqElementSize (ID_NTYPE (a_expr), IDS_NTYPE (b_expr)));
}

/**
 * @param a N_exprs which contains the WL ERCs
 * @param b N_exprs which contains all called ERCs within function scope
 */
static node *
findRCDifference (node *a_exprs, node *b_exprs)
{
    int a_length;

    DBUG_ENTER ();

    a_length = TCcountExprs (a_exprs);

    for (int i = 0; i < a_length; i++) {
        b_exprs = TCfilterExprsArg (&MRChaveSameShape,
                                    EXPRS_EXPR (TCgetNthExprs (i, a_exprs)), &b_exprs);
    }

    DBUG_RETURN (b_exprs);
}

node *
MRCfundef (node *arg_node, info *arg_info)
{
    info *tmp_info;

    DBUG_ENTER ();

    tmp_info = arg_info;
    arg_info = MakeInfo ();

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    arg_info = FreeInfo (arg_info);
    arg_info = tmp_info;

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
MRCwith (node *arg_node, info *arg_info)
{
    node *arg_withop;

    DBUG_ENTER ();

    if (WITH_WITHOP (arg_node) != NULL
        && (NODE_TYPE (WITH_WITHOP (arg_node)) == N_genarray
            || NODE_TYPE (WITH_WITHOP (arg_node)) == N_modarray)) {
        arg_withop = WITH_WITHOP (arg_node);
        if (WITHOP_RC (arg_withop) != NULL) {
            L_WITHOP_RC (arg_withop,
                         findRCDifference (WITHOP_RC (arg_withop), INFO_GRC (arg_info)));
        }

        DBUG_PRINT ("With-op RCs after diffing: ");
        DBUG_EXECUTE (if (WITHOP_RC (arg_withop) != NULL) {
            PRTdoPrintFile (stderr, WITHOP_RC (arg_withop));
        });

        if (INFO_GRC (arg_info) == NULL) {
        } else {
            INFO_GRC (arg_info)
              = TCappendExprs (INFO_GRC (arg_info), WITHOP_RC (arg_withop));
        }

        DBUG_PRINT ("All used RCs after diffing: ");
        DBUG_EXECUTE (if (INFO_GRC (arg_info) != NULL) {
            PRTdoPrintFile (stderr, INFO_GRC (arg_info));
        });
    }

    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
MRCdoRefConsolidation (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_mrc);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

    /* @} */

#undef DBUG_PREFIX
