/*****************************************************************************
 *
 * $Id$
 *
 * file:   SSALUR.c
 *
 * prefix: SSALUR
 *
 * description:
 *
 *   This module implements loop-unrolling for special do-functions in ssa
 *   form. All while loops have been removed and converted to do-loops before
 *   so we have to deal only with the do loops.
 *   We also do the withloop unrolling by using the existing implementation
 *   in WLUnroll().
 *   If we can infere the number of loops and if this number is smaller than
 *   the specified maximum unrolling (maxlur and maxwlur parameter) we
 *   duplicate the code for this number of times.
 *
 *   To have all necessary information about constant data, you should do
 *   a SSAConstantFolding traversal first.
 *
 *****************************************************************************/

#include "types.h"
#include "new_types.h"
#include "shape.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSALUR.h"
#include "constants.h"
#include "math.h"
#include "ctinfo.h"
#include "SSAWLUnroll.h"

/*
 * INFO structure and macros
 */

#include "SSALUR_info.h"

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_ASSIGN (result) = NULL;
    INFO_EXT_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;
    INFO_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

#define UNR_NONE -1

/* type to perform loop unrolling operations on */
typedef long loopc_t;

/* helper functions for internal use only */
static loopc_t SSALURGetDoLoopUnrolling (node *fundef, node *ext_assign);
static node *FindCondAssign (node *assigns);
static bool SSALURIsLURPredicate (node *expr);
static bool SSALURGetLoopIdentifier (node *predicate, node **id);
static bool SSALURAnalyseLURPredicate (node *expr, prf loop_prf, loopc_t init_counter,
                                       loopc_t loop_increment, prf *pred_prf,
                                       loopc_t *term_counter);
static bool SSALURIsLURModifier (node *modifier);
static bool SSALURAnalyseLURModifier (node *modifier, node **id, prf *loop_prf,
                                      loopc_t *inc);
static bool SSALURGetConstantArg (node *id, node *fundef, node *ext_assign,
                                  loopc_t *init_counter);
static bool SSALURIsEqualParameterPosition (node *arg_chain, node *loop_entrance_arg,
                                            node *param_chain, node *loop_rec_param);
static loopc_t SSALURCalcUnrolling (loopc_t init_counter, loopc_t term_counter,
                                    loopc_t loop_increment, prf loop_prf, prf pred_prf);
static void SSALURGetPredicateData (node *expr, prf *pred, loopc_t *term);
static double CalcMulUnroll (loopc_t init, loopc_t inc, loopc_t term);
static node *SSALURUnrollLoopBody (node *fundef, loopc_t unrolling);
static node *SSALURCreateCopyAssignments (node *arg_chain, node *rec_chain);

/******************************************************************************
 *
 * function:
 *   loopc_t SSALURGetDoLoopUnrolling(node *fundef, node *ext_assign)
 *
 * description:
 *   checks the given fundef for unrolling.
 *
 *   the used do-loop-normal form:
 *
 *   ...
 *   x = f_do(..., init_counter, ...);
 *   ...
 *
 *
 *     res_t f_do(..., int loop_entrance_counter_id, ...) {
 *             LOOP-BODY;
 *             loop_counter_id = modifier_expr (loop_entrance_counter_id,
 *                                              loop_prf,
 *                                              loop_increment);
 *             LOOP-BODY;
 *             cond_id = predicate_expr(loop_counter_id,
 *                                      pred_prf,
 *                                      term_counter);
 *             LOOP-BODY;
 *             if (cond_id) {
 *               r1 = f_do(..., loop_counter_id, ...)
 *             } else {
 *               r2 = ...
 *             }
               r = (cond_id ? r1 : r2);
 *             return(r);
 *           }
 *
 *
 *
 * returns:
 *   number of unrollings of the do-loop-body or UNR_NONE if no unrolling is
 *   possible.
 *
 ******************************************************************************/
static loopc_t
SSALURGetDoLoopUnrolling (node *fundef, node *ext_assign)
{
    node *cond_assign;              /* N_assign */
    node *then_instr;               /* N_assign */
    node *condition;                /* N_expr */
    node *predicate_assign;         /* N_assign */
    node *predicate;                /* N_expr */
    node *modifier_assign;          /* N_assign */
    node *modifier;                 /* N_expr */
    node *loop_entrance_counter_id; /* N_id */
    node *loop_counter_id;          /* N_id */
    loopc_t init_counter;           /* initial loop counter */
    loopc_t term_counter;           /* final loop counter */
    loopc_t loop_increment;         /* loop instance increment */
    prf pred_prf;                   /* primitive comp. function */
    prf loop_prf;                   /* primitive function for counter modify */
    bool error;
    loopc_t unroll;

    DBUG_ENTER ("SSALURGetDoLoopUnrolling");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSALURGetDoLoopUnrolling called for non-fundef node");

    /* check for do special fundef */
    if (!FUNDEF_ISDOFUN (fundef)) {
        DBUG_PRINT ("SSALUR", ("no do-loop special fundef"));
        DBUG_RETURN (UNR_NONE);
    }

    DBUG_ASSERT ((FUNDEF_BODY (fundef) != NULL), "function with body required");

    /* search conditional in do fundef */
    cond_assign = FindCondAssign (BLOCK_INSTR (FUNDEF_BODY (fundef)));

    DBUG_ASSERT ((cond_assign != NULL), "do special fundef without conditional");

    /* get condition */
    condition = COND_COND (ASSIGN_INSTR (cond_assign));

    DBUG_ASSERT ((condition != NULL), "missing condition in conditional");

    /* check for identifier as condition */
    if (NODE_TYPE (condition) != N_id) {
        DBUG_PRINT ("SSALUR", ("condition is no identifier"));
        DBUG_RETURN (UNR_NONE);
    }

    /* identifier must be a localy defined vardec  - no arg */
    if (NODE_TYPE (AVIS_DECL (ID_AVIS (condition))) != N_vardec) {
        DBUG_PRINT ("SSALUR", ("identifier is no locally defined vardec"));
        DBUG_RETURN (UNR_NONE);
    }

    /* get defining assignment */
    predicate_assign = AVIS_SSAASSIGN (ID_AVIS (condition));

    DBUG_ASSERT ((predicate_assign != NULL), "missing SSAASSIGN attribute for condition");

    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (predicate_assign)) == N_let),
                 "definition assignment without let");

    /* check predicate for having the correct form */
    predicate = LET_EXPR (ASSIGN_INSTR (predicate_assign));
    if (!SSALURIsLURPredicate (predicate)) {
        DBUG_PRINT ("SSALUR", ("predicate has incorrect form"));
        DBUG_RETURN (UNR_NONE);
    }

    /* get identifier from loop predicate */
    error = SSALURGetLoopIdentifier (predicate, &loop_counter_id);

    DBUG_ASSERT ((error == FALSE), "unexpected error occured during analysis");

    /* check loop counter identifier to be locally defined vardec */
    if (loop_counter_id == NULL)
        DBUG_RETURN (UNR_NONE);
    if (NODE_TYPE (AVIS_DECL (ID_AVIS (loop_counter_id))) != N_vardec) {
        DBUG_PRINT ("SSALUR", ("loop counter is no locally defined vardec"));
        DBUG_RETURN (UNR_NONE);
    }

    /* get defining assignment */
    modifier_assign = AVIS_SSAASSIGN (ID_AVIS (loop_counter_id));
    if (modifier_assign == NULL) {
        DBUG_PRINT ("SSALUR",
                    ("missing SSAASSIGN attribute for condition (possibly withid)"));
        DBUG_RETURN (UNR_NONE);
    }

    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (modifier_assign)) == N_let),
                 "definition assignment without let");

    /* check loop count modify expression for having the correct form */
    modifier = LET_EXPR (ASSIGN_INSTR (modifier_assign));
    if (!(SSALURIsLURModifier (modifier))) {
        DBUG_PRINT ("SSALUR", ("loop count modifier has incorrect form"));
        DBUG_RETURN (UNR_NONE);
    }

    /* get parameter from loop normal-form modify expression */
    error = SSALURAnalyseLURModifier (modifier, &loop_entrance_counter_id, &loop_prf,
                                      &loop_increment);

    DBUG_ASSERT ((error == FALSE), "unexpected error occured during analysis");

    /* check loop entrance identifier to be a (external constant) arg */
    if (!(SSALURGetConstantArg (loop_entrance_counter_id, fundef, ext_assign,
                                &init_counter))) {
        DBUG_PRINT ("SSALUR", ("loop entrance counter is no constant arg"));
        DBUG_RETURN (UNR_NONE);
    }

    /* extract recursive call behind cond  */
    then_instr = COND_THENINSTR (ASSIGN_INSTR (cond_assign));
    while ((then_instr != NULL)
           && (NODE_TYPE (ASSIGN_INSTR (then_instr)) == N_annotate)) {
        then_instr = ASSIGN_NEXT (then_instr);
    }
    DBUG_ASSERT ((NODE_TYPE (then_instr) == N_assign),
                 "cond of loop fun w/o N_assign in then body");
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (then_instr)) == N_let),
                 "cond of loop fun w/o N_let in then body");
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (then_instr)) == N_ap),
                 "cond of loop fun w/o N_ap in then body");
    DBUG_ASSERT ((AP_FUNDEF (ASSIGN_RHS (then_instr)) == fundef),
                 "cond of loop fun w/o recursiv call in then body");
    /*
     * check arg position of entrance identifier and position of
     * loop count identifier in and recursive call to be equal
     */
    if (!(SSALURIsEqualParameterPosition (FUNDEF_ARGS (fundef),
                                          AVIS_DECL (ID_AVIS (loop_entrance_counter_id)),
                                          AP_ARGS (LET_EXPR (ASSIGN_INSTR (then_instr))),
                                          AVIS_DECL (ID_AVIS (loop_counter_id))))) {
        DBUG_PRINT ("SSALUR", ("arg and recursive parameter position don't match"));
        DBUG_RETURN (UNR_NONE);
    }

    /* get parameter from loop normal-form predicate */
    if (!(SSALURAnalyseLURPredicate (predicate, loop_prf, init_counter, loop_increment,
                                     &pred_prf, &term_counter))) {
        DBUG_PRINT ("SSALUR", ("predicate cannot be unrolled with given modifier"));
        DBUG_RETURN (UNR_NONE);
    }

    /* calc unrolling number */
    unroll = SSALURCalcUnrolling (init_counter, term_counter, loop_increment, loop_prf,
                                  pred_prf);

    DBUG_RETURN (unroll);
}

/******************************************************************************
 *
 * function:
 *   node *FindCondAssign(node *assigns)
 *
 * description:
 *   looks for a cond node in assignment chain and returns it (or NULL if
 *   search fails)
 *
 ******************************************************************************/
static node *
FindCondAssign (node *assigns)
{
    DBUG_ENTER ("FindCond");

    while ((assigns != NULL) && (NODE_TYPE (ASSIGN_INSTR (assigns)) != N_cond)) {
        assigns = ASSIGN_NEXT (assigns);
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   bool SSALURIsLURPredicate(predicate)
 *
 * description:
 *   checks the given expression to be of the form:
 *    expr = id    {<=, >=, <, >, ==, != } const
 * or expr = const {<=, >=, <, >, ==, != } id
 *
 ******************************************************************************/
static bool
SSALURIsLURPredicate (node *predicate)
{
    node *arg1;
    node *arg2;
    prf comp_prf;

    DBUG_ENTER ("SSALURIsLURPredicate");

    /* expression must be a primitive function */
    if (NODE_TYPE (predicate) != N_prf) {
        DBUG_PRINT ("SSALUR", ("predicat expression without prf"));
        DBUG_RETURN (FALSE);
    }

    /* prf must be one of the comparison prfs */
    comp_prf = PRF_PRF (predicate);

    if ((comp_prf != F_le) && (comp_prf != F_lt) && (comp_prf != F_ge)
        && (comp_prf != F_gt) && (comp_prf != F_neq)) {
        DBUG_PRINT ("SSALUR", ("predicate with non comparision prf"));
        DBUG_RETURN (FALSE);
    }

    /* args must be one constant (N_num) and one identifier (N_id) node */
    DBUG_ASSERT ((PRF_ARGS (predicate) != NULL),
                 "missing arguments to primitive function");

    DBUG_ASSERT ((EXPRS_NEXT (PRF_ARGS (predicate)) != NULL),
                 "missing second arg of primitive function");

    arg1 = EXPRS_EXPR (PRF_ARGS (predicate));
    arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (predicate)));

    if (((NODE_TYPE (arg1) == N_num) && (NODE_TYPE (arg2) == N_id))
        || ((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_num))) {

        DBUG_PRINT ("SSALUR", ("loop predicate has correct form"));
        DBUG_RETURN (TRUE);
    } else {
        DBUG_PRINT ("SSALUR", ("loop predicate without id and constant args"));

        DBUG_RETURN (FALSE);
    }
}

/******************************************************************************
 *
 * function:
 *   bool SSALURAnalyseLURPredicate(node *expr,
 *                                  prf loop_prf,
 *                                  loopc_t init_counter,
 *                                  loopc_t loop_increment,
 *                                  prf *pred_prf,
 *                                  loopc_t term_counter)
 *
 * description:
 *   analyse loop terminating predicate to infere loop termination counter and
 *   the loop termination condition (pred_prf [must be <=, >= for normal form]).
 *   the correct form of the expression has been validated before.
 *
 ******************************************************************************/
static bool
SSALURAnalyseLURPredicate (node *expr, prf loop_prf, loopc_t init_counter,
                           loopc_t loop_increment, prf *pred_prf, loopc_t *term_counter)
{
    prf pred;
    loopc_t term;
    bool result;

    DBUG_ENTER ("SSALURAnalyseLURPredicate");

    /* get predicate/term constant for the form: id <prf> const */
    SSALURGetPredicateData (expr, &pred, &term);

    /* transform prf to <=, >= by adjusting term constant */
    switch (pred) {
    case F_le:
        result = TRUE;
        break;

    case F_ge:
        result = TRUE;
        break;

    case F_lt:
        /* include upper border (only for add, mul) */
        if ((loop_prf == F_add_SxS) || (loop_prf == F_mul_SxS)) {
            pred = F_le;
            term = term - 1;
            result = TRUE;
        } else {
            result = FALSE;
        }
        break;

    case F_gt:
        /* include lower border: only for sub */
        if (loop_prf == F_sub_SxS) {
            pred = F_ge;
            term = term + 1;
            result = TRUE;
        } else {
            result = FALSE;
        }
        break;

    case F_neq:
        if ((loop_prf == F_add_SxS) && ((init_counter + loop_increment) <= term)
            && (((term - init_counter) % loop_increment) == 0)) {
            /* change F_neq to F_le for increments */
            pred = F_le;
            term = term - 1;
            result = TRUE;
        } else if ((loop_prf == F_sub_SxS) && (term <= (init_counter - loop_increment))
                   && (((init_counter - term) % loop_increment) == 0)) {
            /* change F_neq to F_ge for decrements */
            pred = F_ge;
            term = term + 1;
            result = TRUE;
        } else {
            result = FALSE;
        }
        break;

    default:
        DBUG_ASSERT ((FALSE), "unsupported comparison function");
        result = FALSE;
    }

    /* set result data */
    *pred_prf = pred;
    *term_counter = term;

#ifndef DBUG_OFF
    if (result) {
        DBUG_PRINT ("SSALUR", ("predicate: id %s %d", global.mdb_prf[pred], term));
    }
#endif

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   void SSALURGetPredicateData(node *expr,
 *                               prf *pred,
 *                               loopc_t *term)
 *
 * description:
 *   get prf and const from expression and adjust prf to the form:
 *     id <prf> const
 *   by inverting the comparison function if necessary.
 *
 ******************************************************************************/
static void
SSALURGetPredicateData (node *expr, prf *pred, loopc_t *term)
{
    node *arg1;
    node *arg2;

    DBUG_ENTER ("SSALURGetPredicateData");

    arg1 = EXPRS_EXPR (PRF_ARGS (expr));
    arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (expr)));

    /* get primitive comparison function from AST */
    *pred = PRF_PRF (expr);

    /* get constant from AST */
    if (NODE_TYPE (arg1) == N_id) {
        /* first arg is identifier */
        *term = (loopc_t)NUM_VAL (arg2);
    } else {
        /* second arg is identifier */
        *term = (loopc_t)NUM_VAL (arg1);

        /* change prf to have normal form cond = id <prf> const */
        switch (*pred) {
        case F_lt:
            *pred = F_gt;
            break;

        case F_le:
            *pred = F_ge;
            break;

        case F_gt:
            *pred = F_lt;
            break;

        case F_ge:
            *pred = F_le;
            break;

        default:; /* no change necessary */
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   bool SSALURIsLURModifier(node *modifier)
 *
 * description:
 *   checks the given expression to have the correct form like
 *     expr = id {+, -, *} const
 *  or expr = const {+, -, *} id
 *
 ******************************************************************************/
static bool
SSALURIsLURModifier (node *modifier)
{
    node *arg1;
    node *arg2;
    prf mod_prf;

    DBUG_ENTER ("SSALURIsLURModifier");

    /* expression must be a primitive function */
    if (NODE_TYPE (modifier) != N_prf) {
        DBUG_PRINT ("SSALUR", ("modifier expression without prf"));
        DBUG_RETURN (FALSE);
    }

    /* prf must be one of +, -, * */
    mod_prf = PRF_PRF (modifier);

    if ((mod_prf != F_add_SxS) && (mod_prf != F_sub_SxS) && (mod_prf != F_mul_SxS)) {
        DBUG_PRINT ("SSALUR", ("modifier with unsupported prf"));
        DBUG_RETURN (FALSE);
    }

    /* args must be one constant (N_num) and one identifier (N_id) node */
    DBUG_ASSERT ((PRF_ARGS (modifier) != NULL),
                 "missing arguments to primitive function");

    DBUG_ASSERT ((EXPRS_NEXT (PRF_ARGS (modifier)) != NULL),
                 "missing second arg of primitive function");

    arg1 = EXPRS_EXPR (PRF_ARGS (modifier));
    arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (modifier)));

    /* if prf is F_sub the first arg must be the identifier */
    if (((NODE_TYPE (arg1) == N_num) && (NODE_TYPE (arg2) == N_id)
         && (mod_prf != F_sub_SxS))
        || ((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_num))) {

        DBUG_PRINT ("SSALUR", ("loop modifier has correct form"));
        DBUG_RETURN (TRUE);
    } else {
        DBUG_PRINT ("SSALUR", ("loop modifier without id and constant args"));

        DBUG_RETURN (FALSE);
    }
}

/******************************************************************************
 *
 * function:
 *   bool SSALURGetLoopIdentifier(node *predicate, node **id)
 *
 * description:
 *   get loop identifier from termination condition.
 *
 ******************************************************************************/
static bool
SSALURGetLoopIdentifier (node *predicate, node **id)
{
    DBUG_ENTER ("SSALURGetLoopIdentifier");

    if (NODE_TYPE (EXPRS_EXPR (PRF_ARGS (predicate))) == N_id) {
        /* first parameter is identifier */
        *id = EXPRS_EXPR (PRF_ARGS (predicate));
    } else {
        /* second parameter is identifier */
        *id = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (predicate)));
    }

    DBUG_PRINT ("SSALUR", ("loop identifier: %s", AVIS_NAME (ID_AVIS ((*id)))));

    DBUG_ASSERT ((NODE_TYPE (*id) == N_id), "node must be N_id");

    DBUG_RETURN (FALSE);
}

/******************************************************************************
 *
 * function:
 *   bool SSALURAnalyseLURModifier(node *modifier,
 *                                 node **id,
 *                                 prf *loop_prf,
 *                                 loopc_t *inc)
 *
 * description:
 *   analyse loop modifier expression and return:
 *     1. loop entrance id
 *     2. loop modifiert prf
 *     3. loop increment/decrement/factor (depending on prf)
 *   the correct form has been checked previously
 *
 ******************************************************************************/
static bool
SSALURAnalyseLURModifier (node *modifier, node **id, prf *loop_prf, loopc_t *inc)
{
    node *arg1;
    node *arg2;

    DBUG_ENTER ("SSALURAnalyseLURModifier");

    *loop_prf = PRF_PRF (modifier);

    arg1 = EXPRS_EXPR (PRF_ARGS (modifier));
    arg2 = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (modifier)));

    /* identifier is first arg */
    if (NODE_TYPE (arg1) == N_id) {
        *id = arg1;
        *inc = NUM_VAL (arg2);
    } else {
        /* identifier is second arg */
        *id = arg2;
        *inc = NUM_VAL (arg1);
    }

    DBUG_PRINT ("SSALUR", ("LUR modifier: %s %s %d", AVIS_NAME (ID_AVIS ((*id))),
                           global.mdb_prf[(*loop_prf)], (*inc)));

    DBUG_RETURN (FALSE);
}

/******************************************************************************
 *
 * function:
 *   bool SSALURGetConstantArg(node *id, node *fundef, node *ext_assign,
 *                             loopc_t *init_counter)
 *
 * description:
 *   checks, if given id is an argument with external constant value.
 *   because loop-variant args are not attributed with SSACONST we have to
 *   find the matching argument in the external special fundef call to get
 *   the right constant value.
 *
 ******************************************************************************/
static bool
SSALURGetConstantArg (node *id, node *fundef, node *ext_assign, loopc_t *init_counter)
{
    node *arg_chain;
    node *param_chain;
    node *param;
    int pos;
    int i;
    constant *co;
    node *num;

    DBUG_ENTER ("SSALURGetConstantArg");

    /* check if id is an arg of this fundef */
    if (NODE_TYPE (AVIS_DECL (ID_AVIS (id))) != N_arg) {
        DBUG_PRINT ("SSALUR",
                    ("identifier %s is no fundef argument", AVIS_NAME (ID_AVIS (id))));
        DBUG_RETURN (FALSE);
    }

    /* get argument position in fundef arg chain */
    arg_chain = FUNDEF_ARGS (fundef);
    pos = 1;
    while ((arg_chain != NULL) && (arg_chain != AVIS_DECL (ID_AVIS (id)))) {
        arg_chain = ARG_NEXT (arg_chain);
        pos++;
    }

    DBUG_ASSERT ((arg_chain != NULL), "arg not found in fundef arg chain");

    /* get matching parameter expr-node */
    param_chain = AP_ARGS (ASSIGN_RHS (ext_assign));

    for (i = 1; i < pos; i++) {
        param_chain = EXPRS_NEXT (param_chain);
    }

    DBUG_ASSERT ((param_chain != NULL), "missing matching parameter");
    param = EXPRS_EXPR (param_chain);

    /* check parameter to be constant */
    if (!(COisConstant (param))) {
        DBUG_PRINT ("SSALUR", ("external parameter is not constant"));
        DBUG_RETURN (FALSE);
    }

    /* get constant value (conversion with constant resolves propagated data */
    co = COaST2Constant (param);
    num = COconstant2AST (co);
    co = COfreeConstant (co);
    if (NODE_TYPE (num) != N_num) {
        num = FREEdoFreeNode (num);
        DBUG_PRINT ("SSALUR", ("external parameter is no numercial constant"));
        DBUG_RETURN (FALSE);
    }

    /* set result value */
    *init_counter = (loopc_t)NUM_VAL (num);

    /* free temp. data */
    num = FREEdoFreeNode (num);

    DBUG_PRINT ("SSALUR", ("loop entrance counter: %s = %d", AVIS_NAME (ID_AVIS (id)),
                           (*init_counter)));

    DBUG_RETURN (TRUE);
}

/******************************************************************************
 *
 * function:
 *   bool SSALURIsEqualParameterPosition(node *arg_chain,
 *                                       node *loop_entrance_arg,
 *                                       node *param_chain,
 *                                       node *loop_rec_param)
 *
 * description:
 *   checks, if loop_entrance_parameter and loop_recursive call parameter
 *   have the same place in argument list.
 *
 ******************************************************************************/
static bool
SSALURIsEqualParameterPosition (node *arg_chain, node *loop_entrance_arg,
                                node *param_chain, node *loop_rec_param)
{
    bool result;
    DBUG_ENTER ("SSALURIsEqualParameterPosition");

    if (loop_entrance_arg == arg_chain) {
        if (loop_rec_param == AVIS_DECL (ID_AVIS (EXPRS_EXPR (param_chain)))) {
            DBUG_PRINT ("SSALUR", ("arg and rec. parameter position match"));
            result = TRUE;
        } else {
            DBUG_PRINT ("SSALUR", ("arg and rec. parameter position don't match"));
            result = FALSE;
        }
    } else {
        /* traverse to next arg */
        DBUG_ASSERT ((arg_chain != NULL), "arg not found in argument chain");
        DBUG_ASSERT ((param_chain != NULL), "different counts of args");

        result
          = SSALURIsEqualParameterPosition (ARG_NEXT (arg_chain), loop_entrance_arg,
                                            EXPRS_NEXT (param_chain), loop_rec_param);
    }
    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   loopc_t SSALURCalcUnrolling(loopc_t init_counter,
 *                               loopc_t term_counter,
 *                               loopc_t loop_increment,
 *                               prf loop_prf,
 *                               prf pred_prf)
 *
 * description:
 *   calculates number of necessary loop unrollings based on the infered
 *   parameters of a do-loop in normal form.
 * returns number of unrollings or UNR_NONE if number of unrollings cannot be
 *   calculated.
 *
 ******************************************************************************/
static loopc_t
SSALURCalcUnrolling (loopc_t init_counter, loopc_t term_counter, loopc_t loop_increment,
                     prf loop_prf, prf pred_prf)
{
    loopc_t unrolling;
    double mul_unr;
    DBUG_ENTER ("SSALURCalcUnrolling");

    /* some security checks for parameter to avoid math errors */
    if (((loop_prf == F_add_SxS) || (loop_prf == F_sub_SxS)) && (loop_increment <= 0)) {
        DBUG_PRINT ("SSALUR", ("illegal increment %d for add/sub", loop_increment));
        DBUG_RETURN (UNR_NONE);
    }

    if ((loop_prf == F_mul_SxS)
        && ((loop_increment <= 1) || (init_counter <= 0) || (term_counter <= 0))) {
        DBUG_PRINT ("SSALUR", ("illegal data for mul"));
        DBUG_RETURN (UNR_NONE);
    }

    /* calc unrolling for the different operations */
    switch (loop_prf) {
    case F_add_SxS:
        if ((((term_counter - init_counter) / loop_increment) > 0)
            && (pred_prf == F_le)) {
            unrolling = ((term_counter - init_counter) / loop_increment) + 1;
        } else if ((((term_counter - init_counter) / loop_increment) > 0)
                   && (pred_prf == F_ge)) {
            unrolling = UNR_NONE;
        } else {
            unrolling = 1;
        }
        break;

    case F_sub_SxS:
        if ((((init_counter - term_counter) / loop_increment) > 0)
            && (pred_prf == F_ge)) {
            unrolling = ((init_counter - term_counter) / loop_increment) + 1;
        } else if ((((term_counter - init_counter) / loop_increment) > 0)
                   && (pred_prf == F_le)) {
            unrolling = UNR_NONE;
        } else {
            unrolling = 1;
        }
        break;

    case F_mul_SxS:
        mul_unr = CalcMulUnroll (init_counter, loop_increment, term_counter);
        if ((mul_unr > 0) && (pred_prf == F_le)) {
            unrolling = (loopc_t) (floor (mul_unr) + 1);
        } else {
            unrolling = UNR_NONE;
        }
        break;

    default:
        unrolling = UNR_NONE;
    }

    DBUG_RETURN (unrolling);
}

/******************************************************************************
 *
 * function:
 *   double CalcMulUnroll(loopc_t init, loopc_t inc, loopc_t, loopc_t term)
 *
 * description:
 *   computes res = ln(term/init) / ln(inc)
 *
 *                 term
 *   res = log   (------)
 *           inc   init
 *
 ******************************************************************************/
static double
CalcMulUnroll (loopc_t init, loopc_t inc, loopc_t term)
{
    double res;

    DBUG_ENTER ("CalcMulUnroll");

    res = log ((double)(term / init)) / log ((double)(inc));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SSALURUnrollLoopBody(node *fundef, loopc_t unrolling)
 *
 * description:
 *   does the unrolling of the do-loop body for "unrolling" times.
 *   therefore we duplicate the body and insert a list of copy assignments
 *   to get the right references between loop-end and loop-start.
 *   this duplicating destroys the ssa form that has to be restored afterwards.
 *   the contained conditional is set to FALSE to skip recursion.
 *   More precisely, we transform our pattern:
 *
 *     res_t f_do(..., int loop_entrance_counter_id, ...) {
 *             LOOP-BODY;
 *             loop_counter_id = modifier_expr (loop_entrance_counter_id,
 *                                              loop_prf,
 *                                              loop_increment);
 *             LOOP-BODY;
 *             cond_id = predicate_expr(loop_counter_id,
 *                                      pred_prf,
 *                                      term_counter);
 *             LOOP-BODY;
 *             if (cond_id) {
 *               r1 = f_do(..., loop_counter_id, ...)
 *             } else {
 *               r2 = ...
 *             }
 *             r = (cond_id ? r1 : r2);
 *             return(r);
 *           }
 *
 *   into:
 *
 *     res_t f_do(..., int loop_entrance_counter_id, ...) {
 *       /     LOOP-BODY;
 *       |     loop_counter_id = modifier_expr (loop_entrance_counter_id,
 *       |                                      loop_prf,
 *       |                                      loop_increment);
 *       |     LOOP-BODY;
 *       |     cond_id = predicate_expr(loop_counter_id,
 *  n-times                             pred_prf,
 *       |                              term_counter);
 *       |     LOOP-BODY;
 *       |     ...
 *       |     loop_entrance_counter_id = loop_counter_id;
 *       |     ...
 *       \_
 *             tmp_var = false;
 *             if (tmp_var) {
 *               r1 = f_do(..., loop_counter_id, ...)
 *             } else {
 *               r2 = ...
 *             }
 *             r = (cond_id ? r1 : r2);
 *             return(r);
 *           }
 *
 * !!! WARNING - this transformation destroys the ssa form              !!!
 * !!! the ssa form has to be restored before leaving this optimization !!!
 *
 ******************************************************************************/
static node *
SSALURUnrollLoopBody (node *fundef, loopc_t unrolling)
{
    node *loop_body;
    node *then_instr; /* N_assign */
    node *cond_assign;
    node *last;
    node *new_body;
    node *predavis;
    node *predass;

    DBUG_ENTER ("SSALURUnrollLoopBody");

    DBUG_ASSERT ((unrolling >= 1), "unsupported unrolling number");

    /* separate loop body assignment chain */
    loop_body = BLOCK_INSTR (FUNDEF_BODY (fundef));

    last = loop_body;
    cond_assign = ASSIGN_NEXT (last);
    while ((cond_assign != NULL) && (NODE_TYPE (ASSIGN_INSTR (cond_assign)) != N_cond)) {
        last = cond_assign;
        cond_assign = ASSIGN_NEXT (cond_assign);
    }

    /*
     * last points to the last assignment of the loop body
     * and cond_assign to the conditional of the loop
     */
    DBUG_ASSERT ((last != NULL), "error: missing loop body");
    DBUG_ASSERT ((cond_assign != NULL), "error: missing conditional in loop");

    /* unchain loop body */
    ASSIGN_NEXT (last) = NULL;

    /* build up unrolled body */
    new_body = NULL;
    if (unrolling == 1) {
        /* only one time used */
        new_body = loop_body;
    } else {
        /* unrolling */

        /* extract recursive call behind cond */
        then_instr = COND_THENINSTR (ASSIGN_INSTR (cond_assign));
        DBUG_ASSERT ((NODE_TYPE (then_instr) == N_assign),
                     "cond of loop fun w/o N_assign in then body");
        DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (then_instr)) == N_let),
                     "cond of loop fun w/o N_let in then body");
        DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (then_instr)) == N_ap),
                     "cond of loop fun w/o N_ap in then body");
        DBUG_ASSERT ((AP_FUNDEF (ASSIGN_RHS (then_instr)) == fundef),
                     "cond of loop fun w/o recursiv call in then body");

        /* append copy assignments to loop-body */
        loop_body
          = TCappendAssign (loop_body, SSALURCreateCopyAssignments (FUNDEF_ARGS (fundef),
                                                                    AP_ARGS (ASSIGN_RHS (
                                                                      then_instr))));

        new_body = NULL;

        do {
            new_body = TCappendAssign (DUPdoDupTree (loop_body), new_body);
            unrolling--;
        } while (unrolling > 1);

        /* finally reuse oringinal loop body as last instance */
        new_body = TCappendAssign (loop_body, new_body);
    }

    /* set condition of conditional to false -> no more recursion */
    predavis = TBmakeAvis (ILIBtmpVar (), TYmakeAKV (TYmakeSimpleType (T_bool),
                                                     COmakeFalse (SHmakeShape (0))));

    FUNDEF_VARDEC (fundef) = TBmakeVardec (predavis, FUNDEF_VARDEC (fundef));

    predass = TBmakeAssign (TBmakeLet (TBmakeIds (predavis, NULL), TBmakeBool (FALSE)),
                            cond_assign);
    AVIS_SSAASSIGN (predavis) = predass;

    COND_COND (ASSIGN_INSTR (cond_assign))
      = FREEdoFreeTree (COND_COND (ASSIGN_INSTR (cond_assign)));

    COND_COND (ASSIGN_INSTR (cond_assign)) = TBmakeId (predavis);

    cond_assign = ASSIGN_NEXT (cond_assign);
    while (NODE_TYPE (ASSIGN_INSTR (cond_assign)) == N_let) {
        DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (cond_assign)) == N_funcond,
                     "All node between COND and RETURN must be FUNCOND");

        FUNCOND_IF (ASSIGN_RHS (cond_assign))
          = FREEdoFreeTree (FUNCOND_IF (ASSIGN_RHS (cond_assign)));

        FUNCOND_IF (ASSIGN_RHS (cond_assign)) = TBmakeId (predavis);

        cond_assign = ASSIGN_NEXT (cond_assign);
    }

    /* append rest of fundef assignment chain */
    new_body = TCappendAssign (new_body, predass);

    /* add new body to toplevel block of function */
    BLOCK_INSTR (FUNDEF_BODY (fundef)) = new_body;

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *SSALURCreateCopyAssignments(node *arg_chain, node *rec_chain)
 *
 * description:
 *   this functions builds up an assignment chain of copy assignments for
 *   all identifiers used in the recursive call to have loop back to the
 *   args in the functions signature.
 *
 ******************************************************************************/
static node *
SSALURCreateCopyAssignments (node *arg_chain, node *rec_chain)
{
    node *copy_assigns;
    node *right_id;

    DBUG_ENTER ("SSALURCreateCopyAssignments");

    if (arg_chain != NULL) {
        /* process further identifiers in chain */
        copy_assigns
          = SSALURCreateCopyAssignments (ARG_NEXT (arg_chain), EXPRS_NEXT (rec_chain));

        /* make right identifer as used in recursive call */
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (rec_chain)) == N_id),
                     "non id node as paramter in recursive call");
        right_id = TBmakeId (ID_AVIS (EXPRS_EXPR (rec_chain)));

        /* make copy assignment */
        copy_assigns
          = TBmakeAssign (TBmakeLet (TBmakeIds (ARG_AVIS (arg_chain), NULL), right_id),
                          copy_assigns);
    } else {
        DBUG_ASSERT ((rec_chain == NULL),
                     "different chains of args and calling parameters");
        copy_assigns = NULL;
    }

    DBUG_RETURN (copy_assigns);
}

/* traversal functions for SSALUR travsersal */
/******************************************************************************
 *
 * function:
 *   node *LURfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   - traverse fundef and subordinated special fundefs for LUR and WLUR
 *   - analyse fundef for unrolling
 *   - do the infered unrolling
 *
 ******************************************************************************/
node *
LURfundef (node *arg_node, info *arg_info)
{
    loopc_t unrolling;
    int start_wlunr_expr;
    int start_lunr_expr;

    DBUG_ENTER ("LURfundef");

    INFO_FUNDEF (arg_info) = arg_node;
    /* save start values of opt counters */
    start_wlunr_expr = global.optcounters.wlunr_expr;
    start_lunr_expr = global.optcounters.lunr_expr;

    /*
     * traverse body to get wlur (and special fundefs unrolled)
     * the withloop unrolling itself destroys the ssa form of the AST
     * but because the withloop are not insteresting for the do-loop
     * unrolling we need not to restore the ssa form now (we will do it
     * after the do loop unrolling, because do-loop unrolling destroys
     * the ssa form, too).
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /* analyse fundef for possible unrolling */
    unrolling = SSALURGetDoLoopUnrolling (arg_node, INFO_EXT_ASSIGN (arg_info));

    if (unrolling != UNR_NONE) {
        if (unrolling <= global.unrnum) {
            DBUG_PRINT ("SSALUR", ("unrolling loop %s %d times ", FUNDEF_NAME (arg_node),
                                   unrolling));

            global.optcounters.lunr_expr++;

            /* start do-loop unrolling - this leads to non ssa form code */
            arg_node = SSALURUnrollLoopBody (arg_node, unrolling);

        } else {
            DBUG_PRINT ("SSALUR",
                        ("no unrolling of %s: should be %d (but set to maxlur %d)",
                         FUNDEF_NAME (arg_node), unrolling, global.unrnum));

            if (unrolling <= 32) {
                CTInote ("LUR: -maxlur %d would unroll loop", unrolling);
                /*
                 * We use the hard-wired constant 32 here because otherwise we become
                 * annoyed by messages like "-maxlur 1000000000 would unroll loop".
                 */
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LURassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses assignment chain and integrate unrolled with-loop code
 *
 ******************************************************************************/
node *
LURassign (node *arg_node, info *arg_info)
{
    node *pre_assigns;
    node *tmp;
    node *old_assign;

    DBUG_ENTER ("LURassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "assign node without instruction");

    /* stack actual assign */
    old_assign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    pre_assigns = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;

    /* restore stacked assign */
    INFO_ASSIGN (arg_info) = old_assign;

    /* traverse to next assignment in chain */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* integrate pre_assignments in assignment chain and remove this assign */
    if (pre_assigns != NULL) {
        tmp = arg_node;
        arg_node = TCappendAssign (pre_assigns, ASSIGN_NEXT (arg_node));
        tmp = FREEdoFreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LURap(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses the args and starts the traversal in the called fundef if it is
 *   a special fundef.
 *
 ******************************************************************************/
node *
LURap (node *arg_node, info *arg_info)
{
    info *new_arg_info;
    DBUG_ENTER ("LURap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSALUR", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_EXT_ASSIGN (new_arg_info) = INFO_ASSIGN (arg_info);

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSALUR", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
        new_arg_info = FreeInfo (new_arg_info);

    } else {
        DBUG_PRINT ("SSALUR", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALoopUnrolling(node* fundef)
 *
 * description:
 *   starts the LoopUnrolling traversal for the given fundef. Does not start
 *   in special fundefs.
 *
 ******************************************************************************/
node *
LURdoLoopUnrolling (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("SSALoopUnrolling");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "SSALUR called for non-fundef node");

    global.valid_ssaform = FALSE;

    /* do not start traversal in special functions */
    if (!(FUNDEF_ISLACFUN (fundef))) {
        arg_info = MakeInfo ();

        TRAVpush (TR_lur);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}
