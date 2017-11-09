/**
 * @defgroup mrc Memory Reuse Pre-allocation
 *
 * @ingroup mrc
 *
 * @{
 */

/**
 * @file memory_reuse_preallocation.c
 *
 * Prefix: MRP
 */
#include "memory_reuse_preallocation.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"

#define DBUG_PREFIX "MRP"
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
    int in_loopfun;
    node *unused_erc;
    node *used_rcs;
    node *fundef;
    node *tmp_params;
};

/*
 * INFO macros
 */
#define INFO_IN_LOOPFUN(x) ((x)->in_loopfun)
#define INFO_FUNDEF(x) ((x)->fundef)
#define INFO_UNUSED_ERC(x) ((x)->unused_erc)
#define INFO_USED_RCS(x) ((x)->used_rcs)
#define INFO_TMP_PARAMS(x) ((x)->tmp_params)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_IN_LOOPFUN (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_UNUSED_ERC (result) = NULL;
    INFO_USED_RCS (result) = NULL;
    INFO_TMP_PARAMS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
FindUniqOfAvis (node *id, node *exprs, node *result)
{
    DBUG_ENTER ();

    if (exprs != NULL) {
        if (EXPRS_NEXT (exprs) != NULL) {
            result = FindUniqOfAvis (id, EXPRS_NEXT (exprs), result);
        }

        if (ID_AVIS (EXPRS_EXPR (exprs)) != ID_AVIS (id)) {
            DBUG_PRINT ("       found!\n");
            if (result == NULL) {
                result = TBmakeExprs (EXPRS_EXPR (exprs), NULL);
            } else {
                result = TCappendExprs (result, TBmakeExprs (EXPRS_EXPR (exprs), NULL));
            }
        }
    } else {
        result = TCappendExprs (result, TBmakeExprs (id, NULL));
    }

    DBUG_RETURN (result);
}

static node *
MRPid (node *id, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("    looking at %s", ID_NAME (id));
    INFO_UNUSED_ERC (arg_info)
      = FindUniqOfAvis (id, INFO_USED_RCS (arg_info), INFO_UNUSED_ERC (arg_info));
    DBUG_RETURN (id);
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
    return !((ShapeMatch (ID_NTYPE (a_expr), ID_NTYPE (b_expr))
              || TCshapeVarsMatch (ID_AVIS (a_expr), ID_AVIS (b_expr)))
             && TUeqElementSize (ID_NTYPE (a_expr), ID_NTYPE (b_expr)));
}

/**
 * @param a N_exprs which contains the WL ERCs
 * @param b N_exprs which contains all called ERCs within function scope
 */
static node *
findRCDifference (node *a_exprs, node *b_exprs)
{
    int a_length;
    node *res = NULL;

    DBUG_ENTER ();

    a_length = TCcountExprs (a_exprs);
    DBUG_PRINT ("Diffing: witherc[%d] usedrc[%d]", a_length, TCcountExprs (b_exprs));
    DBUG_EXECUTE (if (a_exprs != NULL) {
        PRTdoPrintFile (stderr, a_exprs);
    } if (b_exprs != NULL) { PRTdoPrintFile (stderr, b_exprs); });

    if (b_exprs != NULL) {
        for (int i = 0; i < a_length; i++) {
            res
              = TCappendExprs (res,
                               TCfilterExprsArg (&MRChaveSameShape,
                                                 EXPRS_EXPR (TCgetNthExprs (i, a_exprs)),
                                                 &b_exprs));
        }
    } else {
        res = a_exprs;
    }

    DBUG_RETURN (res);
}

node *
MRPfundef (node *arg_node, info *arg_info)
{
    info *tmp_info;

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("\nchecking function %s ...", FUNDEF_NAME (arg_node));
        tmp_info = arg_info;
        arg_info = MakeInfo ();

        if (FUNDEF_ISLOOPFUN (arg_node)) {
            fprintf (stderr, "Found loopfun: \n");
            PRTdoPrintFile (stderr, FUNDEF_ERC (arg_node));
            INFO_IN_LOOPFUN (arg_info) = TRUE;
            INFO_FUNDEF (arg_info) = arg_node;
        }

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        arg_info = FreeInfo (arg_info);
        arg_info = tmp_info;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
MRPap (node *arg_node, info *arg_info)
{
    node *tmp;
    node *tmp_loopfun;

    DBUG_ENTER ();

    // if( INFO_IN_LOOPFUN( arg_info) && AP_FUNDEF( arg_node) == INFO_FUNDEF( arg_info))
    //{ // found recursive loopfun call
    //  if( FUNDEF_ERC( INFO_FUNDEF( arg_info)) != NULL && INFO_TMP_PARAMS( arg_info) !=
    //  NULL)
    //  {
    //    tmp = findRCDifference( FUNDEF_ERC( INFO_FUNDEF( arg_info)), INFO_USED_RCS(
    //    arg_info)); tmp = findRCDifference( INFO_TMP_PARAMS( arg_info), tmp); if ( tmp
    //    != NULL)
    //    {
    //      //tmp_loopfun = DUPdoDupTree( INFO_FUNDEF( arg_info));

    //    }
    //  } else {
    //    // no reuse possible
    //  }
    //}

    DBUG_RETURN (arg_node);
}

node *
MRPwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
node *
MRPgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (GENARRAY_RC (arg_node) == NULL) {
        DBUG_PRINT ("Genarray has no RCs! Finding suitable ERCs as alternative...");

        anontrav_t mrptrav[2] = {{N_id, &MRPid}, {(nodetype)0, NULL}};
        TRAVpushAnonymous (mrptrav, &TRAVsons);
        GENARRAY_ERC (arg_node) = TRAVopt (GENARRAY_ERC (arg_node), arg_info);
        TRAVpop ();

        if (INFO_UNUSED_ERC (arg_info) == NULL) {
            DBUG_PRINT ("  found *no* unused ERCs!");
            if (INFO_IN_LOOPFUN (arg_info)
                && FUNDEF_ERC (INFO_FUNDEF (arg_info)) != NULL) {
                // create a new avis of the same type as the with idx
                // new_avis = DUPdoDupNode( WITHOP_IDX( arg_withop));
                // AVIS_NAME( new_avis) = TRAVtmpVarName( "tmp");
                // fprintf(stderr, "WTF: %s %s\n", AVIS_NAME( new_avis), TYtype2String(
                // AVIS_TYPE( new_avis), FALSE, 0));  L_WITHOP_RC( arg_withop, TBmakeExprs(
                // TBmakeId( new_avis), NULL));  INFO_TMP_PARAMS( arg_info) = TCappendExprs(
                // INFO_TMP_PARAMS( arg_info), WITHOP_RC( arg_withop));
            } else {
                DBUG_PRINT ("  not in loopfun, skipping...");
            }
        } else { // we have possible candidates
            DBUG_PRINT ("  found matching ERCs!");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_UNUSED_ERC (arg_info)););
            GENARRAY_RC (arg_node)
              = TBmakeExprs (EXPRS_EXPR (INFO_UNUSED_ERC (arg_info)), NULL);
            if (INFO_USED_RCS (arg_info) == NULL) {
                INFO_USED_RCS (arg_info)
                  = TBmakeExprs (EXPRS_EXPR (INFO_UNUSED_ERC (arg_info)), NULL);
            } else {
                INFO_USED_RCS (arg_info)
                  = TCappendExprs (INFO_USED_RCS (arg_info),
                                   TBmakeExprs (EXPRS_EXPR (INFO_UNUSED_ERC (arg_info)),
                                                NULL));
            }
            INFO_UNUSED_ERC (arg_info) = NULL;
        }
    } else {
        DBUG_PRINT ("With-op has RCs!");
    }

    // if( GENARRAY_ERC( arg_node) != NULL)
    // GENARRAY_ERC( arg_node) = FREEdoFreeTree( GENARRAY_ERC( arg_node));

    DBUG_PRINT ("With-op RC candidates after diffing: ");
    DBUG_EXECUTE (if (GENARRAY_RC (arg_node) != NULL) {
        PRTdoPrintFile (stderr, GENARRAY_RC (arg_node));
    });

    DBUG_PRINT ("Used RCs after diffing: ");
    DBUG_EXECUTE (if (INFO_USED_RCS (arg_info) != NULL) {
        PRTdoPrintFile (stderr, INFO_USED_RCS (arg_info));
    });

    DBUG_PRINT ("With-op temporary RCs after diffing: ");
    DBUG_EXECUTE (if (INFO_TMP_PARAMS (arg_info) != NULL) {
        PRTdoPrintFile (stderr, INFO_TMP_PARAMS (arg_info));
    });

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MRPmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (MODARRAY_RC (arg_node) == NULL) {
        DBUG_PRINT ("Genarray has no RCs! Finding suitable ERCs as alternative...");

        anontrav_t mrptrav[2] = {{N_id, &MRPid}, {(nodetype)0, NULL}};
        TRAVpushAnonymous (mrptrav, &TRAVsons);
        MODARRAY_ERC (arg_node) = TRAVopt (MODARRAY_ERC (arg_node), arg_info);
        TRAVpop ();

        if (INFO_UNUSED_ERC (arg_info) == NULL) {
            DBUG_PRINT ("  found *no* unused ERCs!");
            if (INFO_IN_LOOPFUN (arg_info)
                && FUNDEF_ERC (INFO_FUNDEF (arg_info)) != NULL) {
                // create a new avis of the same type as the with idx
                // new_avis = DUPdoDupNode( WITHOP_IDX( arg_withop));
                // AVIS_NAME( new_avis) = TRAVtmpVarName( "tmp");
                // fprintf(stderr, "WTF: %s %s\n", AVIS_NAME( new_avis), TYtype2String(
                // AVIS_TYPE( new_avis), FALSE, 0));  L_WITHOP_RC( arg_withop, TBmakeExprs(
                // TBmakeId( new_avis), NULL));  INFO_TMP_PARAMS( arg_info) = TCappendExprs(
                // INFO_TMP_PARAMS( arg_info), WITHOP_RC( arg_withop));
            } else {
                DBUG_PRINT ("  not in loopfun, skipping...");
            }
        } else { // we have possible candidates
            DBUG_PRINT ("  found matching ERCs!");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_UNUSED_ERC (arg_info)););
            MODARRAY_RC (arg_node)
              = TBmakeExprs (EXPRS_EXPR (INFO_UNUSED_ERC (arg_info)), NULL);
            if (INFO_USED_RCS (arg_info) == NULL) {
                INFO_USED_RCS (arg_info)
                  = TBmakeExprs (EXPRS_EXPR (INFO_UNUSED_ERC (arg_info)), NULL);
            } else {
                INFO_USED_RCS (arg_info)
                  = TCappendExprs (INFO_USED_RCS (arg_info),
                                   TBmakeExprs (EXPRS_EXPR (INFO_UNUSED_ERC (arg_info)),
                                                NULL));
            }
            INFO_UNUSED_ERC (arg_info) = NULL;
        }
    } else {
        DBUG_PRINT ("With-op has RCs!");
    }

    if (MODARRAY_ERC (arg_node) != NULL)
        MODARRAY_ERC (arg_node) = FREEdoFreeTree (MODARRAY_ERC (arg_node));

    DBUG_PRINT ("With-op RC candidates after diffing: ");
    DBUG_EXECUTE (if (MODARRAY_RC (arg_node) != NULL) {
        PRTdoPrintFile (stderr, MODARRAY_RC (arg_node));
    });

    DBUG_PRINT ("Used RCs after diffing: ");
    DBUG_EXECUTE (if (INFO_USED_RCS (arg_info) != NULL) {
        PRTdoPrintFile (stderr, INFO_USED_RCS (arg_info));
    });

    DBUG_PRINT ("With-op temporary RCs after diffing: ");
    DBUG_EXECUTE (if (INFO_TMP_PARAMS (arg_info) != NULL) {
        PRTdoPrintFile (stderr, INFO_TMP_PARAMS (arg_info));
    });

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MRPdoPreallocation (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_mrp);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

    /* @} */

#undef DBUG_PREFIX
