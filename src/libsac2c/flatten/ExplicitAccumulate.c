/**
 *
 * @file ExplicitAccumulate.c
 *
 * Summary
 * =======
 *
 * In this traversal applications of fold functions of fold withloops
 * are made explicit in the WL-bodies and a new function F_accu is inserted
 * in code.
 *
 * Ex.:
 *    A = with {
 *          gen (iv) { val = ...;
 *              } : val;
 *        } : fold (op, n);
 *
 * is transformed into
 *
 *    A = with {
 *          gen (iv) { acc   = accu( iv, n);
 *                     val = ...;
 *                     res = op( acc, val);
 *              } : res;
 *        } : fold (op, n);
 *
 * Multi-generator WLs are not supported.
 * In case of multi-operator fold-with-loops, accu needs to return
 * as many values as we have fold-operators and we need folding function
 * applications for all folding operations, e.g.:
 *
 *    A,B = with {
 *             gen (iv) { acc,acc2   = accu( iv, n, n2);
 *                        val = ...;
 *                        val2 = ...;
 *                        res = op (acc, val);
 *                        res2 = op2 (acc2, val2);
 *                 }: (res, res2);
 *           } : (fold (op, n), fold (op2, n2));
 *
 *
 *
 * The function F_accu is used to get the correct
 * accumulation values but it is only pseudo syntax and is elided
 * during compile. The index vector argument is introduced
 * to disable LIR lifting the application of accu out. The neutral
 * arguments are needed in order to allow type inference to predict
 * the number of return values.
 *
 * Besides this basic transformation, we need to support two special
 * cases:
 *
 * I) partial fold function applications:
 *
 * If the fold operation <op> actually is a partial application, e.g.
 *
 * <op> == foo (a, b), then we generate
 *
 * foo (a, b, acc, val) instead of
 * foo (acc, val)
 *
 * Such partial applications have the arguments stored in FOLD_ARGS!
 *
 *
 * II) foldfix:
 *
 * If we have a foldfix instead of a fold (FOLD_GUARD exists!), we make
 * the termination criterion explicit, switch the foldfix into a fold
 * (elide the FOLD_GUARD), and we insert an N_break operator, i.e., we introduce
 * a variable new_s of type bool[] which is computed by using sacprelude::eq
 * and returned for the break operator position:
 *
 * Ex.:
 *    A = with {
 *          gen (iv) { val = ...;
 *              }: val;
 *        } : foldfix (op, n, s);
 *
 * is transformed into
 *
 *    A, tmp = with {
 *               gen (iv) { acc   = accu( iv, n);
 *                          val = ...;
 *                          res = op( acc, val);
 *                          new_s = sacprelude::eq( res, s);
 *                   }: (res, new_s);
 *             } : (fold (op, n), break ());
 *
 * NB: All this needs to happen correctly even if the fold operator is
 * "surrounded" by propagate operators! Therefore we have to be carefull
 * when dealing with FOLD_LHS and FOLD_CEXPR! (cf.bug 376!)
 *
 *
 * Implementation
 * ==============
 *
 * When traversing a with-loop, we first traverse through the WITHOPs.
 * While doing so, EAfold ensures that the relevant code pieces are being
 * constructed:
 * I) INFO_FOLD_ACCU_ASSIGN holds the accu assignment, and
 * II) INFO_FOLD_ASSIGNS holds the actual folding operation applications
 *     as well as a potential break predicate computation
 * These code snippets are being inserted through EAcode and EAassign during
 * a subsequent traversal of the with-loop code.
 *
 * The construction of INFO_FOLD_ACCU_ASSIGN happens in several steps.
 * The initial construction is done in EAfold when looking at the last
 * fold-operator. Once that has happened, EAfold calls
 *       InjectAccuIds (lhs, arg_info)    for every fold operator.
 * It creates a new accu variable and injects it into INFO_FOLD_ACCU_ASSIGN,
 * relying on INFO_FUNDEF for the insertion of the variable declaration.
 * It also relies on INFO_FOLD to inject the neutral element as additional
 * argument to _accu_.
 *
 * The construction of INFO_FOLD_ASSIGNS happens through
 *       InjectFoldFunAssign (avis, cexpr, arg_info)    for every fold operator.
 * Given the accu variable's avis and the body's result in cexpr (N_id node),
 * this function constructs the explicit fold function call. While doing so, it
 * relies on INFO_FUNDEF for the insertion of the variable declaration, on
 * INFO_FOLD for checking whether we have a partial application
 * (FOLD_ARGS != NULL) and for potentially dealing with a foldfix
 * (FOLD_GUARD != NULL).
 *
 * The required adjustments of the cexprs are done in EAfold as well!
 */

#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"

#define DBUG_PREFIX "EA"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "shape.h"
#include "deserialize.h"
#include "namespaces.h"
#include "ExplicitAccumulate.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *wl;
    node *fold;
    node *accu;
    node *assigns;
    node *cexprs;
    node *ids;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_WL(n) (n->wl)
#define INFO_FOLD(n) (n->fold)
#define INFO_FOLD_ACCU_ASSIGN(n) (n->accu)
#define INFO_FOLD_ASSIGNS(n) (n->assigns)
#define INFO_CEXPRS(n) (n->cexprs)
#define INFO_LHS_IDS(n) (n->ids)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_FOLD (result) = NULL;
    INFO_FOLD_ACCU_ASSIGN (result) = NULL;
    INFO_FOLD_ASSIGNS (result) = NULL;
    INFO_CEXPRS (result) = NULL;
    INFO_LHS_IDS (result) = NULL;

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
 * @fn void InjectVardecFor( node *avis, info *arg_info)
 *
 *   @brief creates new vardec for the given avis and inserts it into the
 *          vardec-chain of INFO_FUNDEF( arg_info)
 *
 *   @param  node *avis     :  new variable
 *           info *arg_info :  context info
 *   @return void
 ******************************************************************************/
static void
InjectVardec (node *avis, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
        = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *InjectAccuIds( node *ids, info *arg_info)
 *
 *   @brief creates a new accu variable based on the given N_ids name and
 *          inject an N_ids into INFO_FOLD_ACCU_ASSIGN. Adds neutral element
 *          of INFO_FOLD as additional argument as well.
 *
 *   @param  node *ids      :  old N_ids node to derive the name from
 *           info *arg_info :  context info
 *   @return node *         :  new N_avis node for the fresh accu variable
 ******************************************************************************/
static node *
InjectAccuIds (node *ids, info *arg_info)
{
    node *avis, *lhs, *neutral;

    DBUG_ENTER ();
    neutral = FOLD_NEUTRAL (INFO_FOLD (arg_info));
    DBUG_PRINT ("   injecting neutral argument into INFO_FOLD_ACCU_ASSIGN: %s", ID_NAME (neutral));
    PRF_ARGS (LET_EXPR (ASSIGN_STMT (INFO_FOLD_ACCU_ASSIGN (arg_info))))
        = TCappendExprs (
              PRF_ARGS (LET_EXPR (ASSIGN_STMT (INFO_FOLD_ACCU_ASSIGN (arg_info)))),
              TBmakeExprs (DUPdoDupTree (neutral), NULL));

    DBUG_PRINT ("   injecting new ids into INFO_FOLD_ACCU_ASSIGN");
    avis = TBmakeAvis (TRAVtmpVarName (IDS_NAME (ids)),
                       TYeliminateAKV (AVIS_TYPE (IDS_AVIS (ids))));
    InjectVardec (avis, arg_info);
    AVIS_SSAASSIGN (avis) = INFO_FOLD_ACCU_ASSIGN (arg_info);

    lhs = LET_IDS (ASSIGN_STMT (INFO_FOLD_ACCU_ASSIGN (arg_info)));
    LET_IDS (ASSIGN_STMT (INFO_FOLD_ACCU_ASSIGN (arg_info)))
        = TBmakeIds (avis, lhs);

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *InjectFoldFunAssign (node *acc_avis, node *cexpr_id, info *arg_info);
 *
 *   @brief  creates new assignment containing fold function of withloop
 *
 *   @param  node *acc_avis  :  avis of the accu
 *   @param  node *cexpr_id  :  old cexpr
 *   @param  info *arg_info  :  fold WL context
 *   @return node *          :  new N_assign node
 ******************************************************************************/
static node *
InjectFoldFunAssign (node *acc_avis, node *cexpr_id, info *arg_info)
{
    node *avis, *assign, *args;
    node *guard, *eq_funap, *fixavis, *fixassign;
    DBUG_ENTER ();

    /*
     * create a function application of the form:
     *    <cexpr'> = <fun>( <acc>, <cexpr>);
     * where
     *    <acc>   is the accumulator variable
     *    <fun>   is the name of the (artificially introduced) folding-fun
     *    <cexpr> is the expression in the operation part
     */
    DBUG_PRINT ("   creating fold-fun ap");

    /* create new cexpr id: */
    avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (cexpr_id)),
                       TYcopyType (AVIS_TYPE (acc_avis)));
    InjectVardec (avis, arg_info);

    /* create <avis> = <fun>( <accu>, cexpr_id); */
    args = TBmakeExprs (TBmakeId (acc_avis), TBmakeExprs (cexpr_id, NULL));
    if (FOLD_ARGS (INFO_FOLD (arg_info))) {
        /* Partial ap! Duplicate the partial args exprs from FOLD_ARGS
         * and prepand them to the args:
         */
        DBUG_PRINT ("   injecting args from partial fold-fun ap");
        args = TCappendExprs (DUPdoDupTree (FOLD_ARGS (INFO_FOLD (arg_info))),
                              args);
    }
    assign = TBmakeAssign (
                 TBmakeLet (
                     TBmakeIds (avis, NULL),
                     TBmakeAp (FOLD_FUNDEF (INFO_FOLD (arg_info)),
                               args)),
                 INFO_FOLD_ASSIGNS (arg_info));
    INFO_FOLD_ASSIGNS (arg_info) = assign;

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (avis) = assign;

    guard = FOLD_GUARD (INFO_FOLD (arg_info));
    if (guard != NULL) {
        /*
         * create an assignment <fixbool> = sacprelude::eq( <avis>, fixval);
         */
        DBUG_PRINT ("   creating explicit foldfix check");
        args = TBmakeExprs (TBmakeId (avis),
                            TBmakeExprs (guard, NULL));

        eq_funap = DSdispatchFunCall (NSgetNamespace (global.preludename), "eq", args);
        DBUG_ASSERT (eq_funap != NULL, "%s::eq not found", global.preludename);

        fixavis = TBmakeAvis (TRAVtmpVarName (ID_NAME (guard)),
                              TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));

        InjectVardec (fixavis, arg_info);

        fixassign = TBmakeAssign (TBmakeLet (TBmakeIds (fixavis, NULL), eq_funap),
                                  ASSIGN_NEXT (assign));

        AVIS_SSAASSIGN (fixavis) = fixassign;
        ASSIGN_NEXT (assign) = fixassign;

        // Finally, we replace the guard by the new one!
        FOLD_GUARD (INFO_FOLD (arg_info)) = TBmakeId (fixavis);
    }

    DBUG_RETURN (avis);
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EAmodule(node *arg_node, info *arg_info)
 *
 *   @brief traverses function definitions only!
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
EAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DSinitDeserialize (arg_node);
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }
    DSfinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAfundef(node *arg_node, info *arg_info)
 *
 *   @brief  Traverses FUNDEF body
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
EAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_ASSIGNS (arg_node) = TRAVdo (FUNDEF_ASSIGNS (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAassign(node *arg_node, info *arg_info)
 *
 *   @brief  Traverses in expression
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
EAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);


    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FOLD_ASSIGNS (arg_info) != NULL) {
            ASSIGN_NEXT (arg_node) = INFO_FOLD_ASSIGNS (arg_info);
            INFO_FOLD_ASSIGNS (arg_info) = NULL;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAlet(node *arg_node, info *arg_info)
 *
 *   @brief  Traverses in expression
 *
 *   @param  node *arg_node:  N_let
 *           info *arg_info:  info
 *   @return node *        :  N_let
 ******************************************************************************/

node *
EAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS_IDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAwith(node *arg_node, info *arg_info)
 *
 *   @brief  if current WL is a fold WL modify code.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
EAwith (node *arg_node, info *arg_info)
{
    info *tmp;

    DBUG_ENTER ();

    /* stack arg_info */
    tmp = arg_info;
    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = INFO_FUNDEF (tmp);
    INFO_LHS_IDS (arg_info) = INFO_LHS_IDS (tmp);
    INFO_WL (arg_info) = arg_node;
    INFO_CEXPRS (arg_info) = CODE_CEXPRS (WITH_CODE (arg_node));

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    INFO_CEXPRS (arg_info) = NULL;

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /* pop arg_info */
    arg_info = FreeInfo (arg_info);
    arg_info = tmp;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EApropagate(node *arg_node, info *arg_info)
 *
 *   @brief  modify code.
 *
 *   @param  node *arg_node:  N_propagate
 *           info *arg_info:  N_info
 *   @return node *        :  N_propagate
 ******************************************************************************/

node *
EApropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    INFO_LHS_IDS (arg_info) = IDS_NEXT (INFO_LHS_IDS (arg_info));
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAfold(node *arg_node, info *arg_info)
 *
 *   @brief  modify code.
 *
 *   @param  node *arg_node:  N_fold
 *           info *arg_info:  N_info
 *   @return node *        :  N_fold
 ******************************************************************************/

node *
EAfold (node *arg_node, info *arg_info)
{
    node *lhs, *cexprs;
    node *avis;
    DBUG_ENTER ();

    DBUG_PRINT ("Fold WL found ...");

    INFO_FOLD (arg_info) = arg_node;
    lhs = INFO_LHS_IDS (arg_info);
    cexprs = INFO_CEXPRS (arg_info);

    if (INFO_FOLD_ACCU_ASSIGN (arg_info) == NULL) {
        DBUG_PRINT ("   generating empty accu assign");
        INFO_FOLD_ACCU_ASSIGN (arg_info)
            = TBmakeAssign (
                  TBmakeLet (
                      NULL,
                      TCmakePrf1 (
                          F_accu,
                          DUPdupIdsId (WITH_VEC (INFO_WL (arg_info))))),
                  NULL);
    }

    // create a new accu var and inject it into INFO_FOLD_ACCU_ASSIGN:
    avis = InjectAccuIds (lhs, arg_info);

    // create a new fold function assignment and inject it ito INFO_FOLD_ASSIGNS:
    avis = InjectFoldFunAssign (avis, EXPRS_EXPR1 (cexprs), arg_info);

    // replace the old cexpr with the new one:
    DBUG_PRINT ("   new cexpr: '%s'", AVIS_NAME (avis));
    EXPRS_EXPR1 (cexprs) = TBmakeId (avis);

    if (FOLD_GUARD (arg_node) != NULL) {
        node *avis;
        node *brk;
        node *pred;

        /*
         * InjectFoldFunAssign left the boolean cexpr in FOLD_GUARD.
         * Now, we move it into the cexprs!
         */
        pred = FOLD_GUARD (arg_node);
        FOLD_GUARD (arg_node) = NULL;
        DBUG_PRINT ("   additional break cexpr: '%s'", ID_NAME (pred));
        EXPRS_NEXT (cexprs) = TBmakeExprs (pred,
                                           EXPRS_NEXT (cexprs));

        /*
         * Append break withop
         */
        brk = TBmakeBreak ();
        BREAK_NEXT (brk) = FOLD_NEXT (arg_node);
        FOLD_NEXT (arg_node) = brk;

        /*
         * Append IDS to LHS of assignment
         */
        avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (pred)),
                           TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
        InjectVardec (avis, arg_info);
        AVIS_SSAASSIGN (avis) = AVIS_SSAASSIGN (IDS_AVIS (lhs));
        IDS_NEXT (lhs) = TBmakeIds (avis, IDS_NEXT (lhs));
    }

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
        INFO_LHS_IDS (arg_info) = IDS_NEXT (INFO_LHS_IDS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EAcode(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_code
 *           info *arg_info:  N_info
 *   @return node *        :  N_code
 ******************************************************************************/

node *
EAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* insertion of INFO_FOLD_ACCU_ASSIGN has to happen here as the block can
     * be empty!
     */
    if (INFO_FOLD_ACCU_ASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (INFO_FOLD_ACCU_ASSIGN (arg_info))
            = BLOCK_ASSIGNS (CODE_CBLOCK (arg_node));
        BLOCK_ASSIGNS (CODE_CBLOCK (arg_node))
            = INFO_FOLD_ACCU_ASSIGN (arg_info);
        INFO_FOLD_ACCU_ASSIGN (arg_info) = NULL;
    }

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    DBUG_ASSERT (CODE_NEXT (arg_node) == NULL, "cannot handle multi generator WLs");

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */

/** <!--********************************************************************-->
 *
 * @fn node *ExplicitAccumulate( node *arg_node)
 *
 *   @brief  Starting function of ExplicitAccumulate traversal
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
EAdoExplicitAccumulate (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "ExplicitAccumulate not started with module node");

    DBUG_PRINT ("starting ExplicitAccumulation");

    arg_info = MakeInfo ();

    TRAVpush (TR_ea);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
