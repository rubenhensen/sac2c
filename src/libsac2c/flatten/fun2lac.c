/*****************************************************************************
 *
 * $Id$
 *
 * file:   fun2lac.c
 *
 * prefix: FUN2LAC
 *
 * description:
 *   This compiler module implements the reconversion of conditionals and
 *   loops from their true functional representation generated by routines
 *   found in lac2fun.c into an explicit representation suitable for code
 *   generation.
 *
 *   After a large-scale re-organization of this phase, loop and cond
 *   functions are merely transformed into loops and conditionals while
 *   inlining is postponed to a separate application of function inlining.
 *
 *****************************************************************************/

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "types.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "globals.h"
#include "LookUpTable.h"
#include "rename.h"

/*
 * Here, we retransform (tail-end recursive) loop functions into do-loops.
 * The function is, however, preserved for the time being. Only its body
 * is transformed. The function itself will be inlined in a subsequent
 * using standard function inlining.
 *
 * Consider a tail-end-recursive do-function of the form
 *
 *   a,b = DoFun( c, d, e)
 *   {
 *     <assigns>
 *     if (p) {
 *       <rc-assings-then>
 *       x, y = DoFun( C, D, e);
 *     }
 *     else {
 *       <rc-assigns-else>
 *     }
 *     return( x, y);
 *   }
 *
 * On first glance, one might consider a transformtion into
 *
 *   a,b = DoFun( c, d, e)
 *   {
 *     goto label;
 *     do {
 *       <rc-assings-then>
 *       c = C;
 *       d = D;
 *      label:
 *       <assigns>
 *     } while (p);
 *     <rc-assigns-else>
 *     return(x,y);
 *
 * where each all those parameters that are not just passed through
 * without modification (such as "e" in our example) are re-assigned
 * in the start of the loop just before the label.
 *
 * Unfortunately, function inlining REQUIRES NO re-assignments to arguments
 * to occur in the function body !!
 *
 * As a consequence, we need to introduce fresh names for all those
 * arguments that require re-asssignment within the loop:
 *
 *   a,b = DoFun( c, d, e)
 *   {
 *     c' = c;
 *     d' = d;
 *     goto label;
 *     do {
 *       <rc-assings-then> | [c->c',d->d']
 *       c' = C;           | [c->c',d->d']
 *       d' = D;           | [c->c',d->d']
 *      label:
 *       <assigns>         | [c->c',d->d']
 *     } while (p);        | [c->c',d->d']
 *     <rc-assigns-else>   | [c->c',d->d']
 *     return(x,y);        | [c->c',d->d']
 *
 *
 * Unfortunately, there is a rather weird case that requires further attention:
 * Function arguments may go unchanged to the recursive call, but in a different
 * order:
 *
 *   a,b = DoFun( c, d, e)
 *   {
 *     <assigns>
 *     if (p) {
 *       <rc-assings-then>
 *       x, y = DoFun( d, c, e);
 *     }
 *     else {
 *       <rc-assigns-else>
 *     }
 *     return( x, y);
 *   }
 *
 * In this case, our scheme would lead to an errorneous second assignment
 * to d':
 *
 *   a,b = DoFun( c, d, e)
 *   {
 *     c' = c;
 *     d' = d;
 *     goto label;
 *     do {
 *       <rc-assings-then> | [c->c',d->d']
 *       c' = d';
 *       d' = c';          <<<< WRONG !!!
 *      label:
 *       <assigns>         | [c->c',d->d']
 *     } while (p);        | [c->c',d->d']
 *     <rc-assigns-else>   | [c->c',d->d']
 *     return(x,y);        | [c->c',d->d']
 *
 * We avoid this problem by explicitly introducing a renaming assignment:
 *
 *   a,b = DoFun( c, d, e)
 *   {
 *     c' = c;
 *     d' = d;
 *     goto label;
 *     do {
 *       <rc-assings-then> | [c->c',d->d']
 *       c^ = c';
 *       c' = d';
 *       d' = c^;
 *      label:
 *       <assigns>         | [c->c',d->d']
 *     } while (p);        | [c->c',d->d']
 *     <rc-assigns-else>   | [c->c',d->d']
 *     return(x,y);        | [c->c',d->d']
 *
 */

static lut_t *f2l_lut = NULL;

/*
 * INFO structure
 */

struct INFO {
    bool below_cond;
    node *fundef;
    node *returnn;
    node *cond;
    node *recap;
    node *recarg;
    node *new_vardecs;
    node *new_topassigns;
    node *new_botassigns;
    node *new_auxassigns;
};

/*
 * INFO macros
 */

#define INFO_BELOW_COND(n) n->below_cond
#define INFO_FUNDEF(n) n->fundef
#define INFO_RETURN(n) n->returnn
#define INFO_COND(n) n->cond
#define INFO_RECAP(n) n->recap
#define INFO_RECARG(n) n->recarg
#define INFO_NEW_VARDECS(n) n->new_vardecs
#define INFO_NEW_TOPASSIGNS(n) n->new_topassigns
#define INFO_NEW_BOTASSIGNS(n) n->new_botassigns
#define INFO_NEW_AUXASSIGNS(n) n->new_auxassigns

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_BELOW_COND (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_COND (result) = NULL;
    INFO_RECAP (result) = NULL;
    INFO_RECARG (result) = NULL;
    INFO_RETURN (result) = NULL;
    INFO_NEW_VARDECS (result) = NULL;
    INFO_NEW_TOPASSIGNS (result) = NULL;
    INFO_NEW_BOTASSIGNS (result) = NULL;
    INFO_NEW_AUXASSIGNS (result) = NULL;

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
SearchStoreVar (node *avis, node *assigns)
{
    node *tmp;
    node *res;

    DBUG_ENTER ();

    res = NULL;
    tmp = assigns;

    while (tmp != NULL) {
        if (avis == ID_AVIS (LET_EXPR (ASSIGN_INSTR (tmp)))) {
            res = IDS_AVIS (LET_IDS (ASSIGN_INSTR (tmp)));
            tmp = NULL;
        } else {
            tmp = ASSIGN_NEXT (tmp);
        }
    }

    DBUG_RETURN (res);
}

node *
F2Lassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (NODE_TYPE (ASSIGN_INSTR (arg_node))) {
    case N_return:
        INFO_RETURN (arg_info) = arg_node;
        arg_node = NULL;
        break;
    case N_cond:
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        DBUG_ASSERT (ASSIGN_NEXT (arg_node) != NULL,
                     "Cond node is last assignment in chain");

        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        INFO_COND (arg_info) = arg_node;
        arg_node = NULL;
        break;
    case N_let:
        if (INFO_BELOW_COND (arg_info) && (ASSIGN_NEXT (arg_node) == NULL)) {
            DBUG_ASSERT (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_ap,
                         "Last assignment in then-part not function call");

            DBUG_ASSERT (AP_FUNDEF (LET_EXPR (ASSIGN_INSTR (arg_node)))
                           == INFO_FUNDEF (arg_info),
                         "Last assignment in then-part not recursive call");

            INFO_RECAP (arg_info) = arg_node;
            arg_node = NULL;
        } else {
            if (ASSIGN_NEXT (arg_node) != NULL) {
                ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
            }
        }
        break;
    default:
        DBUG_ASSERT (FALSE, "Control flow should not reach here");
    }

    DBUG_RETURN (arg_node);
}

node *
F2Lcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (!INFO_BELOW_COND (arg_info), "Nested conditional found.");

    INFO_BELOW_COND (arg_info) = TRUE;

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_BELOW_COND (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

static node *
Arg2Var (node *arg, info *arg_info)
{
    char *new_name;
    node *new_avis;

    DBUG_ENTER ();

    new_name = TRAVtmpVarName (ARG_NAME (arg));
    new_avis = TBmakeAvis (new_name, TYcopyType (AVIS_TYPE (ARG_AVIS (arg))));

    INFO_NEW_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_NEW_VARDECS (arg_info));

    DBUG_RETURN (new_avis);
}

node *
F2Larg (node *arg_node, info *arg_info)
{
    node *recarg, *new_avis, *tmp_avis;
    bool needs_aux_assign;

    DBUG_ENTER ();

    recarg = EXPRS_EXPR (INFO_RECARG (arg_info));
    new_avis = NULL;
    needs_aux_assign = FALSE;

    if (ID_AVIS (recarg) != ARG_AVIS (arg_node)) {
        new_avis = Arg2Var (arg_node, arg_info);
        f2l_lut = LUTinsertIntoLutP (f2l_lut, ARG_AVIS (arg_node), new_avis);
        needs_aux_assign = (NULL != LUTsearchInLutP (f2l_lut, ID_AVIS (recarg)));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        INFO_RECARG (arg_info) = EXPRS_NEXT (INFO_RECARG (arg_info));
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    if (new_avis != NULL) {
        INFO_NEW_TOPASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                     TBmakeId (ARG_AVIS (arg_node))),
                          INFO_NEW_TOPASSIGNS (arg_info));

        if (needs_aux_assign) {
            tmp_avis = SearchStoreVar (LUTsearchInLutPp (f2l_lut, ID_AVIS (recarg)),
                                       INFO_NEW_AUXASSIGNS (arg_info));

            if (tmp_avis == NULL) {
                tmp_avis = Arg2Var (arg_node, arg_info);
                INFO_NEW_AUXASSIGNS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (tmp_avis, NULL),
                                             TBmakeId (
                                               LUTsearchInLutPp (f2l_lut,
                                                                 ID_AVIS (recarg)))),
                                  INFO_NEW_AUXASSIGNS (arg_info));
            }

            INFO_NEW_BOTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), TBmakeId (tmp_avis)),
                              INFO_NEW_BOTASSIGNS (arg_info));
        } else {
            INFO_NEW_BOTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                         TBmakeId (
                                           LUTsearchInLutPp (f2l_lut, ID_AVIS (recarg)))),
                              INFO_NEW_BOTASSIGNS (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

static node *
TransformIntoDoLoop (node *arg_node, info *arg_info)
{
    node *loop, *fun_body;
    node *body_assigns, *then_assigns, *else_assigns, *return_assign, *loop_pred;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    if (FUNDEF_ARGS (arg_node) != NULL) {
        INFO_RECARG (arg_info)
          = AP_ARGS (LET_EXPR (ASSIGN_INSTR (INFO_RECAP (arg_info))));
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    body_assigns = RENdoRenameLut (FUNDEF_INSTR (arg_node), f2l_lut);
    FUNDEF_INSTR (arg_node) = NULL;

    return_assign = RENdoRenameLut (INFO_RETURN (arg_info), f2l_lut);
    INFO_RETURN (arg_info) = NULL;

    loop_pred = RENdoRenameLut (COND_COND (ASSIGN_INSTR (INFO_COND (arg_info))), f2l_lut);
    COND_COND (ASSIGN_INSTR (INFO_COND (arg_info))) = NULL;

    then_assigns = BLOCK_ASSIGNS (COND_THEN (ASSIGN_INSTR (INFO_COND (arg_info))));
    BLOCK_ASSIGNS (COND_THEN (ASSIGN_INSTR (INFO_COND (arg_info)))) = NULL;

    if ((then_assigns != NULL) && (NODE_TYPE (then_assigns) == N_assign)) {
        then_assigns = RENdoRenameLut (then_assigns, f2l_lut);
    } else {
        then_assigns = NULL;
    }

    else_assigns = BLOCK_ASSIGNS (COND_ELSE (ASSIGN_INSTR (INFO_COND (arg_info))));
    BLOCK_ASSIGNS (COND_ELSE (ASSIGN_INSTR (INFO_COND (arg_info)))) = NULL;

    if ((else_assigns != NULL) && (NODE_TYPE (else_assigns) == N_assign)) {
        else_assigns = RENdoRenameLut (else_assigns, f2l_lut);
    } else {
        else_assigns = NULL;
    }

    /*
     * The above strange code is necessary because empty assign chains are
     * represented
     * by the N_empty node rather than a NULL pointer. This should be changed!
     */

    INFO_COND (arg_info) = FREEdoFreeTree (INFO_COND (arg_info));
    INFO_RECAP (arg_info) = FREEdoFreeTree (INFO_RECAP (arg_info));

    f2l_lut = LUTremoveContentLut (f2l_lut);

    then_assigns
      = TCappendAssign (then_assigns, TCappendAssign (INFO_NEW_AUXASSIGNS (arg_info),
                                                      INFO_NEW_BOTASSIGNS (arg_info)));

    INFO_NEW_AUXASSIGNS (arg_info) = NULL;
    INFO_NEW_BOTASSIGNS (arg_info) = NULL;

    loop = TBmakeDo (loop_pred, TBmakeBlock (body_assigns, NULL));

    DO_ISCUDARIZABLE (loop) = FUNDEF_ISCUDALACFUN (arg_node);

    if (then_assigns != NULL) {
        DO_SKIP (loop) = TBmakeBlock (then_assigns, NULL);
        DO_LABEL (loop) = TRAVtmpVarName ("label");
    }

    fun_body = TBmakeAssign (loop, TCappendAssign (else_assigns, return_assign));

    FUNDEF_INSTR (arg_node) = TCappendAssign (INFO_NEW_TOPASSIGNS (arg_info), fun_body);

    FUNDEF_RETURN (arg_node) = ASSIGN_INSTR (return_assign);

    INFO_NEW_TOPASSIGNS (arg_info) = NULL;

    FUNDEF_VARDEC (arg_node)
      = TCappendVardec (INFO_NEW_VARDECS (arg_info), FUNDEF_VARDEC (arg_node));
    INFO_NEW_VARDECS (arg_info) = NULL;

    INFO_FUNDEF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACfundef( node *arg_node, info *arg_info)
 *
 * description:
 *   This function traverses the function body of any function that does NOT
 *   represent an abstracted conditional or loop.
 *
 ******************************************************************************/

node *
F2Lfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISCONDFUN (arg_node)) {
        FUNDEF_ISCONDFUN (arg_node) = FALSE;
        FUNDEF_ISLACINLINE (arg_node) = TRUE;
        /*
         * After the funconds have already been removed by UndoSSAtransform,
         * there is hardly anything to do here.
         */
    } else {
        if (FUNDEF_ISLOOPFUN (arg_node)) {
            arg_node = TransformIntoDoLoop (arg_node, arg_info);
            FUNDEF_ISLOOPFUN (arg_node) = FALSE;
            FUNDEF_ISLACINLINE (arg_node) = TRUE;
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FUN2LACmodul( node *arg_node, info *arg_info)
 *
 * description:
 *   This function traverses all function definitions under a N_modul
 *   node.
 *
 ******************************************************************************/

node *
F2Lmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Fun2Lac( node *syntax_tree)
 *
 * description:
 *   This function initiates the tree traversal process for reconverting
 *   functional representations of loops and conditionals into their
 *   explicit representations.
 *
 ******************************************************************************/

node *
F2LdoFun2Lac (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    f2l_lut = LUTgenerateLut ();

    TRAVpush (TR_f2l);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    f2l_lut = LUTremoveLut (f2l_lut);
    FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
