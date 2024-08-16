#include <stdio.h>

#include "globals.h"

#define DBUG_PREFIX "FLATG"
#include "debug.h"

#include "new_types.h"
#include "types.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "handle_mops.h"
#include "while2do.h"
#include "handle_condexpr.h"
#include "namespaces.h"
#include "shape.h"
#include "phase.h"
#include "flatten.h"
#include "algebraic_wlfi.h"
#include "new_typecheck.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 */

/*
 * This phase flattens WL generators and N_prf nodes.
 * Sorry about the name, folks...
 *
 */

/**
 * INFO structure
 */
struct INFO {
    node *vardecs;
    node *preassignsprf;
    node *preassignswith;
    node *lhs;
    bool assignisnwith;
    bool exprsisinprf;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_PREASSIGNSWITH(n) (n->preassignswith)
#define INFO_PREASSIGNSPRF(n) (n->preassignsprf)
#define INFO_LHS(n) (n->lhs)
#define INFO_ASSIGNISNWITH(n) (n->assignisnwith)
#define INFO_EXPRSISINPRF(n) (n->exprsisinprf)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNSWITH (result) = NULL;
    INFO_PREASSIGNSPRF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_ASSIGNISNWITH (result) = FALSE;
    INFO_EXPRSISINPRF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *expression2Avis(node *arg_node, node **vardecs,
 *                                  node **preassigns, ntype *restype)
 *
 *   @brief  Generates (usually) an assignment for the expression at arg_node.
 *           E.g., if arg_node is:
 *
 *            _max_VxV_(a, b);
 *
 *          it will look like this on the way out:
 *           TYPE  TMP;
 *            ...
 *
 *            TMP = _max_VxV_(a, b);
 *            TMP
 *
 *   NB: It is the caller's responsibility to DUP the arg_node,
 *       if required.
 *
 *   @param  node *arg_node: a node to be flattened.
 *           node **vardecs: a pointer to a vardecs chain that
 *                           will have a new vardec appended to it.
 *           node **preassigns: a pointer to a preassigns chain that
 *                           will have a new assign appended to it.
 *           node *restype:  the ntype of TMP.
 *                          NB. restype is now computed directly here!!
 *
 *
 *   @return node *node:      N_avis node for TMP.
 *
 ******************************************************************************/
node *
FLATGexpression2Avis (node *arg_node, node **vardecs, node **preassigns, ntype *restype)
{
    node *avis;
    node *nas;

    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_id:
        avis = ID_AVIS (arg_node);
        // Caller may have constructed restype for us, but we do not use it.
        restype = (NULL != restype) ? TYfreeType (restype) : NULL;
        break;

    case N_avis:
        avis = arg_node;
        // Caller may have constructed restype for us, but we do not use it.
        restype = (NULL != restype) ? TYfreeType (restype) : NULL;
        break;

    default:
        if (NULL == restype) {
            restype = NTCnewTypeCheck_Expr (arg_node);
            // This may be a product type or a simple type. Disambiguate here.
            if (TYisProd (restype)) {
                DBUG_ASSERT (1 == TYgetProductSize (restype), "expected one result type");
                restype = TYgetProductMember (restype, 0);
            }
        }
        avis = TBmakeAvis (TRAVtmpVar (), restype);
        *vardecs = TBmakeVardec (avis, *vardecs);
        nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), arg_node), NULL);
        AVIS_SSAASSIGN (avis) = nas;
        *preassigns = TCappendAssign (*preassigns, nas);
        DBUG_PRINT ("Generated assign for %s", AVIS_NAME (avis));
        break;
    }

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATGflattenExprsChain(node *arg_node, node **vardecs,
 *                                  node **preassigns, ntype *restype)
 *
 *   @brief  Flattens each N_exprs node in arg_node.
 *
 *           e.g., if arg_node is this at entry:
 *
 *            z = [ 42, colsx, 666];
 *
 *          it will look like this at exit:
 *
 *            t1 = 42;
 *            t2 = 666;
 *            z = [ t1, colsx, t2];
 *
 *
 *   @param  node *arg_node: an N_exprs node whose elements are to be flattened.
 *           node **vardecs: a pointer to a vardecs chain that
 *                           will have a new vardec appended to it.
 *           node **preassigns: a pointer to a preassigns chain that
 *                           will have a new assign appended to it.
 *           node *restype:  the ntype of TMP.
 *
 *   @return node *node:     New N_exprs chain, appropriately DUPed.
 *
 ******************************************************************************/
node *
FLATGflattenExprsChain (node *arg_node, node **vardecs, node **preassigns, ntype *restype)
{
    node *exprs;
    node *expr;
    node *z;

    DBUG_ENTER ();

    exprs = DUPdoDupTree (arg_node);
    z = exprs;

    while (NULL != exprs) {
        expr = EXPRS_EXPR (exprs);
        expr = FLATGexpression2Avis (expr, vardecs, preassigns, restype);
        EXPRS_EXPR (exprs) = TBmakeId (expr);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *FLATGflattenBound( node *arg_node, info *arg_info)
 *
 *   @brief  Flattens the WL bound at arg_node.
 *           I.e., if the generator looks like this on entry:
 *            s0 = _idx_shape_sel(0,x);
 *            s1 = _idx_shape_sel(1,x);
 *            z = with {
 *             (. <= iv < [s0, s1]) ...
 *            }
 *
 *          it will look like this on the way out:
 *            int[2] TMP;
 *            ...
 *            s0 = _idx_shape_sel(0,x);
 *            s1 = _idx_shape_sel(1,x);
 *            TMP = [s0, s1];
 *            z = with {
 *             (. <= iv < TMP) ...
 *            }
 *
 *          The only rationale for this change is to ensure that
 *          WL bounds are named. This allows us to associate an
 *          N_avis node with each bound, which will be used to
 *          store AVIS_MIN and AVIS_MAX for the bound.
 *          These fields, in turn, will be used by the constant
 *          folder to remove guards and do other swell optimizations.
 *
 *          Things are made a bit messier by the requirement
 *          of placing the new TMP assigns before the N_assign
 *          of the N_with, in a nested environment, such as this one
 *          (the second WLs are the S+V statements:
 *
 *          Compile this with these options to see what has
 *          to happen:
 *
 *          sac2c nestedwl.sac -doswlf -nowlur -doswlf -v1
 *                -b11:saacyc:flt -noprelude
 *
 *  use Array: {sel,shape,iota,+,*};
 *  use StdIO: {print};
 *
 *  int[*] id(int[*] y)
 *  { return(y);
 *  }
 *
 *  int main()
 *  {
 *   A = iota (id (25));
 *   B = 20 + iota( id(25));
 *   C = with {
 *        (. <= iv <= .)
 *           {
 *            e1 = A[iv] + [1,2,3,4,5];
 *            e2 = B[iv] + [1,2,3,4,50];
 *           } : e1 + e2;
 *       } : genarray([25]);
 *
 *   print(C);
 *  return(0);
 *  }
 *
 *
 *   @param  node *arg_node: a WL PART BOUND to be flattened.
 *           info *arg_info:
 *
 *   @return node *node:      N_id node for flattened bound
 ******************************************************************************/
static node *
FLATGflattenBound (node *arg_node, info *arg_info)
{
    node *avis;
    node *nas;
    node *res;
    shape *shp;
    size_t xrho;

    DBUG_ENTER ();

    res = arg_node;
    if (NULL != arg_node) {
        switch (NODE_TYPE (arg_node)) {
        case N_array:
            xrho = TCcountExprs (ARRAY_AELEMS (arg_node));
            shp = SHcreateShape (1, xrho);
            avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int), shp));
            INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));
            nas
              = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), DUPdoDupTree (arg_node)),
                              NULL);
            AVIS_SSAASSIGN (avis) = nas;
            INFO_PREASSIGNSWITH (arg_info)
              = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), nas);

            res = TBmakeId (avis);
            FREEdoFreeTree (arg_node);
            DBUG_PRINT ("Generated avis for: %s, of shape %zu", AVIS_NAME (avis), xrho);
            break;
        case N_id:
            break;
        default:
            DBUG_UNREACHABLE ("FLATGflattenBound expected N_array or N_id");
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGdoFlatten(node *arg_node)
 *
 * description:
 *   eliminates nested function applications:
 *   flattens WL generator bounds, step, width.
 *
 *
 ******************************************************************************/

node *
FLATGdoFlatten (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ();

    info_node = MakeInfo ();

    TRAVpush (TR_flatg);
    arg_node = TRAVdo (arg_node, info_node);
    TRAVpop ();

    info_node = FreeInfo (info_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGmodule(node *arg_node, info *arg_info)
 *
 * description:
 *   this function is needed to limit the traversal to the FUNS-son of
 *   N_module!
 *
 ******************************************************************************/

node *
FLATGmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   - calls TRAVdo to flatten the user defined functions if function body is not
 *   empty and resets tos after flatten of the function's body.
 *   - the formal parameters of a function will be traversed to put their names
 *   on the stack!
 *
 ******************************************************************************/

node *
FLATGfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    /*
     * Do not flatten imported functions. These functions have already been
     * flattened and if this is done again there may arise name clashes.
     * A new temp variable __flat42 may conflict with __flat42 which was
     * inserted in the first flatten phase (module compiliation).
     * Furthermore, imported code contains IDS nodes instead of SPIDS nodes!
     * This may lead to problems when this traversal is run.
     */
    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_WASIMPORTED (arg_node))
        && (!FUNDEF_ISWRAPPERFUN (arg_node)) && (FUNDEF_ISLOCAL (arg_node))) {
        INFO_VARDECS (arg_info) = NULL;
        DBUG_PRINT ("flattening function %s:", FUNDEF_NAME (arg_node));
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /* Append new vardecs, if any were generated, to existing vardec chain. */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDECS (FUNDEF_BODY (arg_node))
          = TCappendVardec (BLOCK_VARDECS (FUNDEF_BODY (arg_node)),
                            INFO_VARDECS (arg_info));
        INFO_VARDECS (arg_info) = NULL;
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    /*
     * Proceed with the next function...
     */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATGpart(node *arg_node, info *arg_info)
 *
 * @brief traverse the partition
 *        This is needed to handle nested WLs, such as:
 *
 *        use Array: {genarray,<=,+,iota,sel};
 *        use StdIO: {print};
 *
 *        int[*] id(int[*] y)
 *        { return(y);
 *        }
 *
 *        int main()
 *        {
 *         x = id(genarray([10,10],4));
 *
 *          z = with {
 *             (. <= iv <= .) : x[iv] + iota(3);
 *                } : genarray(_shape_A_(x), [1,2, 3]);
 *
 *                print(z);
 *                return(0);
 *                }
 *
 *
 *        We have to collect the new flattened variable assignments
 *        for all partitions, then insert them before the N_with.
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATGpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    /* We have to traverse the generators last */
    PART_GENERATOR (arg_node) = TRAVopt (PART_GENERATOR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node FLATGwith( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        We need a fresh PREASSIGNWITH chain, because we have to collect
 *        the preassigns for all partitions, then pass them back
 *        to our N_assign node.
 *
 *****************************************************************************/
node *
FLATGwith (node *arg_node, info *arg_info)
{
    info *new_info;

    DBUG_ENTER ();

    new_info = MakeInfo ();
    INFO_VARDECS (new_info) = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    /* Traverse all partitions in this N_with. */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), new_info);

    INFO_VARDECS (arg_info) = INFO_VARDECS (new_info);
    INFO_PREASSIGNSWITH (arg_info)
      = TCappendAssign (INFO_PREASSIGNSWITH (new_info), INFO_PREASSIGNSWITH (arg_info));
    new_info = FreeInfo (new_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node FLATGassign( node *arg_node, info *arg_info)
 *
 * @brief performs a depth-first traversal.
 *        Prepends new assign nodes ahead of this node.
 *        The check on node type is to ensure that we
 *        are in the right place for the prepends.
 *        The nested WLs are the problem here...
 *
 *****************************************************************************/
node *
FLATGassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();
    if ((N_let == NODE_TYPE (ASSIGN_STMT (arg_node)))
        && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg_node))))) {
        INFO_ASSIGNISNWITH (arg_info) = TRUE;
        DBUG_PRINT ("Traversing N_assign for %s",
                    AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node)))));
    }

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_ASSIGNISNWITH (arg_info) && (NULL != INFO_PREASSIGNSWITH (arg_info))) {
        arg_node = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), arg_node);
        INFO_PREASSIGNSWITH (arg_info) = NULL;
        INFO_ASSIGNISNWITH (arg_info) = FALSE;
    }

    if (NULL != INFO_PREASSIGNSPRF (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNSPRF (arg_info), arg_node);
        INFO_PREASSIGNSPRF (arg_info) = NULL;
    }

    /* Traverse remaining assigns in this block. */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATGgenerator(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens N_generator
 *   all non-N_ID-nodes are removed and the operators are changed
 *   to <= and < if possible (bounds != NULL).
 *
 ******************************************************************************/

node *
FLATGgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    GENERATOR_BOUND1 (arg_node)
      = FLATGflattenBound (GENERATOR_BOUND1 (arg_node), arg_info);
    GENERATOR_BOUND2 (arg_node)
      = FLATGflattenBound (GENERATOR_BOUND2 (arg_node), arg_info);
    GENERATOR_STEP (arg_node) = FLATGflattenBound (GENERATOR_STEP (arg_node), arg_info);
    GENERATOR_WIDTH (arg_node) = FLATGflattenBound (GENERATOR_WIDTH (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGcond(node *arg_node, info *arg_info)
 *
 * description: Traverse conditional
 *
 ******************************************************************************/

node *
FLATGcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    COND_COND (arg_node) = TRAVopt (COND_COND (arg_node), arg_info);
    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGfuncond(node *arg_node, info *arg_info)
 *
 * description: Traverse funcond
 *
 ******************************************************************************/

node *
FLATGfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNCOND_IF (arg_node) = TRAVopt (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGdo(node *arg_node, info *arg_info)
 *
 * description: Traverse do-loop
 *
 ******************************************************************************/

node *
FLATGdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DO_COND (arg_node) = TRAVopt (DO_COND (arg_node), arg_info);
    DO_BODY (arg_node) = TRAVopt (DO_BODY (arg_node), arg_info);
    DO_SKIP (arg_node) = TRAVopt (DO_SKIP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGwhile(node *arg_node, info *arg_info)
 *
 * description: Traverse while-loop
 *
 ******************************************************************************/

node *
FLATGwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGlet(node *arg_node, info *arg_info)
 *
 * description: Traverse N_let
 *
 ******************************************************************************/

node *
FLATGlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    DBUG_PRINT ("Looking at %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGprf(node *arg_node, info *arg_info)
 *
 * description: Traverse primitive functions, flattening all
 *              constants into N_id nodes.
 *
 *              Some primitives do not like to have their arguments
 *              flattened. Those are ignored here.
 *
 *              idx_shape_sel() is anomalous, in that we should flatten
 *              PRF_ARG2, but not PRF_ARG1. If that proves to be a problem,
 *              it will have to be fixed here. Or else, we could housebreak
 *              the function itself.
 *
 ******************************************************************************/

node *
FLATGprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    default:
        INFO_EXPRSISINPRF (arg_info) = TRUE;
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        INFO_EXPRSISINPRF (arg_info) = FALSE;
        break;
    case F_dispatch_error:
    case F_conditional_error:
    case F_idx_shape_sel:
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGexprs(node *arg_node, info *arg_info)
 *
 * description: Flatten all non-N_id PRFARG elements.
 *              We skip N_exprs nodes if they are not
 *              N_prf arguments, e.g, something hanging off AVIS_SHAPE.
 *
 ******************************************************************************/

node *
FLATGexprs (node *arg_node, info *arg_info)
{
    node *expr;
    bool doflatten;

    DBUG_ENTER ();

    if (INFO_EXPRSISINPRF (arg_info)) {
        expr = EXPRS_EXPR (arg_node);
        doflatten
          = ((NODE_TYPE (expr) == N_numbyte) || (NODE_TYPE (expr) == N_numshort)
             || (NODE_TYPE (expr) == N_numint) || (NODE_TYPE (expr) == N_numlong)
             || (NODE_TYPE (expr) == N_numlonglong) || (NODE_TYPE (expr) == N_numubyte)
             || (NODE_TYPE (expr) == N_numushort) || (NODE_TYPE (expr) == N_numuint)
             || (NODE_TYPE (expr) == N_numulong) || (NODE_TYPE (expr) == N_numulonglong)
             || (NODE_TYPE (expr) == N_num) || (NODE_TYPE (expr) == N_float)
             || (NODE_TYPE (expr) == N_double) || (NODE_TYPE (expr) == N_bool)
             || (NODE_TYPE (expr) == N_char) || (NODE_TYPE (expr) == N_str)
             || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_spap)
             || (NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_with)
             || (NODE_TYPE (expr) == N_cast) || (NODE_TYPE (expr) == N_nested_init));

        if (doflatten) {
            DBUG_PRINT ("Flattening N_prf for %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            expr = FLATGexpression2Avis (expr, &INFO_VARDECS (arg_info),
                                         &INFO_PREASSIGNSPRF (arg_info),
                                         TYmakeAUD (TYmakeSimpleType (T_unknown)));
            expr = TBmakeId (expr);
            EXPRS_EXPR (arg_node) = expr;
        }

        EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
