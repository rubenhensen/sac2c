#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "namespaces.h"
#include "ctinfo.h"

#include "handle_zero_generator_with_loops.h"

/**
 * This traversal transformes with loops without generators into their default
 * values or creates matching generators.
 * Expressions of the forms:
 *
 *    with {                      with {                    with {
 *     } genarray( shp, def)       } modarray( a)            } fold( fun, neutr)
 *
 * are semantically equivalent to
 *
 *     with {                     a                         neutr
 *       ( . <= iv <= . ) : def
 *     } genarray( shp)
 *
 * Furthermore, the propagate is the identity on its default value.
 *
 * To get the scoping right, we have to be careful on where and how to insert
 * the new expressions in the case of multi generator wls. As an example consider
 *
 * a = 1;
 * b = 5;
 * a,b = with {
 *       } ( genarray( shp, b), fold( +, a))
 *
 * We cannot insert the b = a upfront, as that would introduce a new b which
 * overrides the usage of b in the genarray. We cannot introduce it after the with,
 * as there the value of a has changed. Instead, we have to save the value of a
 * upfront und define the new b after the withloop.
 *
 * a = 1;
 * b = 5;
 * tmp = a;
 * a = with {
 *     } genarray( shp, b);
 * b = tmp;
 *
 * Outline of the Implementation
 * =============================
 *
 */

typedef enum { T_traverse, T_replace } traversal_mode_t;

/**
 * INFO structure
 */
struct INFO {
    node *preassign;
    node *postassign;
    node *lhs;
    node *newres;
    bool xdelete;
    bool exprpos;
    traversal_mode_t withop_traversal_mode;
};

/**
 * INFO macros
 */
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_POSTASSIGN(n) ((n)->postassign)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_NEWRES(n) ((n)->newres)
#define INFO_DELETE(n) ((n)->xdelete)
#define INFO_EXPRPOS(n) ((n)->exprpos)
#define INFO_MODE(n) ((n)->withop_traversal_mode)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NEWRES (result) = NULL;
    INFO_DELETE (result) = FALSE;
    INFO_EXPRPOS (result) = FALSE;
    INFO_MODE (result) = T_traverse;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--**********************************************************************
 *
 * @fn node *HZGWLdoHandleZeroGeneratorWithLoops( node *syntax_tree)
 *
 * @brief starts the removal of zero generator With-Loops
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/

node *
HZGWLdoHandleZeroGeneratorWithLoops (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_hzgwl);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HZGWLwith(node *arg_node, info *arg_info)
 *
 * @brief replaces zero generator withloops by either their default
 *        value or creates a default generator.
 *
 * @param arg_node N_with to be potentially transformed
 * @param arg_info not used
 *
 * @return arg_node
 *
 *****************************************************************************/

static node *
BuildDefault (node *arg_node)
{
    DBUG_ENTER ();

    WITH_PART (arg_node)
      = TBmakePart (NULL, TBmakeWithid (TBmakeSpids (TRAVtmpVar (), NULL), NULL),
                    TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1), TBmakeDot (1), NULL,
                                     NULL));
    WITH_CODE (arg_node) = TBmakeCode (TBmakeBlock (NULL, NULL), NULL);

    PART_CODE (WITH_PART (arg_node)) = WITH_CODE (arg_node);
    CODE_USED (WITH_CODE (arg_node))++;

    DBUG_RETURN (arg_node);
}

static void
MergeNewExprs (node **preassigns, node **postassigns, node *with, node **exprs,
               node **ops, node **lhs)
{
    node *tmp;
    char *tmpvar;

    DBUG_ENTER ();

    if (*exprs != NULL) {
        if ((lhs == NULL) || (*lhs == NULL)) {
            CTIerror (EMPTY_LOC, 
                      "Number of left-hand-side expression does not match "
                      "number of With-Loop operators.");
        } else {
            /*
             * we have to do the tmp-trick here as the compound
             * macro is not a legal l-value.
             */
            tmp = WITHOP_NEXT (*ops);
            MergeNewExprs (preassigns, postassigns, with, &EXPRS_NEXT (*exprs), &tmp,
                           &SPIDS_NEXT (*lhs));
            L_WITHOP_NEXT (*ops, tmp);

            switch (NODE_TYPE (*ops)) {
            case N_genarray:
                EXPRS_NEXT (*exprs) = CODE_CEXPRS (WITH_CODE (with));
                CODE_CEXPRS (WITH_CODE (with)) = *exprs;
                *exprs = NULL;
                break;

            case N_modarray:
            case N_spfold:
            case N_propagate:
                tmp = SPIDS_NEXT (*lhs);
                SPIDS_NEXT (*lhs) = NULL;

                tmpvar = TRAVtmpVar ();

                *preassigns = TBmakeAssign (TBmakeLet (TBmakeSpids (tmpvar, NULL),
                                                       EXPRS_EXPR (*exprs)),
                                            *preassigns);

                *postassigns
                  = TBmakeAssign (TBmakeLet (*lhs, TBmakeSpid (NULL, STRcpy (tmpvar))),
                                  *postassigns);

                *lhs = tmp;
                EXPRS_EXPR (*exprs) = NULL;
                *exprs = FREEdoFreeNode (*exprs);
                *ops = FREEdoFreeNode (*ops);
                break;

            default:
                DBUG_UNREACHABLE ("unhandeled withop type!");
            }
        }
    }

    DBUG_RETURN ();
}

bool
AllPartsEmpty (node *part)
{
    bool result = TRUE;

    DBUG_ENTER ();

    result &= (BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (part))) == NULL);

    if (result && (PART_NEXT (part) != NULL)) {
        result = result && AllPartsEmpty (PART_NEXT (part));
    }

    DBUG_RETURN (result);
}

node *
HZGWLwith (node *arg_node, info *arg_info)
{
    bool exprpos;

    DBUG_ENTER ();

    if ((WITH_PART (arg_node) == NULL) && (WITH_WITHOP (arg_node) != NULL)) {
        /**
         * first we build a default partition.
         */
        arg_node = BuildDefault (arg_node);

        /**
         * Traversing the withops will lead to new
         * cexprs for genarrays and new exprs in
         * for all others. The expressions are stored
         * in INFO_NEWRES.
         */
        INFO_MODE (arg_info) = T_replace;

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        INFO_MODE (arg_info) = T_traverse;

        if (INFO_EXPRPOS (arg_info)) {
            /**
             * this wl is in expression position
             */
            if (TCcountExprs (INFO_NEWRES (arg_info)) != 1) {
                CTIerror (EMPTY_LOC, "Multi-Operator With-Loop used in expression position.");
            }

            switch (NODE_TYPE (WITH_WITHOP (arg_node))) {
            case N_genarray:
                CODE_CEXPRS (WITH_CODE (arg_node)) = INFO_NEWRES (arg_info);
                INFO_NEWRES (arg_info) = NULL;
                break;
            case N_modarray:
            case N_spfold:
            case N_propagate:
                arg_node = FREEdoFreeTree (arg_node);
                arg_node = EXPRS_EXPR (INFO_NEWRES (arg_info));
                EXPRS_EXPR (INFO_NEWRES (arg_info)) = NULL;
                INFO_NEWRES (arg_info) = FREEdoFreeTree (INFO_NEWRES (arg_info));
                break;
            default:
                DBUG_UNREACHABLE ("Unhandeled withop type!");
            }
        } else {
            MergeNewExprs (&INFO_PREASSIGN (arg_info), &INFO_POSTASSIGN (arg_info),
                           arg_node, &INFO_NEWRES (arg_info), &WITH_WITHOP (arg_node),
                           &INFO_LHS (arg_info));
        }

        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        /**
         * from here on, all following WLS are in
         * expression position.
         */
        exprpos = INFO_EXPRPOS (arg_info);
        INFO_EXPRPOS (arg_info) = TRUE;

        arg_node = TRAVcont (arg_node, arg_info);

        INFO_EXPRPOS (arg_info) = exprpos;
    }

    /*
     * if this now is a withloop without operators
     * and without partitions, we scrap it. If it
     * has no operators and all partitions are empty,
     * we scrap it, as well.
     */
    INFO_DELETE (arg_info)
      = ((NODE_TYPE (arg_node) == N_with) && (WITH_WITHOP (arg_node) == NULL)
         && ((WITH_PART (arg_node) == NULL) || AllPartsEmpty (WITH_PART (arg_node))));

    /**
     * if we are going to delete a void withloop that is in expression
     * position or has return values, we throw an error
     */
    if (INFO_DELETE (arg_info) && INFO_EXPRPOS (arg_info)) {
        CTIerror (EMPTY_LOC, "Void With-Loop used in expression position.");
    } else if (INFO_DELETE (arg_info) && (INFO_LHS (arg_info) != NULL)) {
        CTIerror (EMPTY_LOC, "Void With-Loops do not yield any return values.");
    }

    DBUG_RETURN (arg_node);
}

node *
HZGWLgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == T_traverse) {
        arg_node = TRAVcont (arg_node, arg_info);
    } else {
        if (GENARRAY_NEXT (arg_node) != NULL) {
            GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
        }

        if (GENARRAY_DEFAULT (arg_node) == NULL) {
            CTIerror (EMPTY_LOC, "Missing default value for zero-generator withloop.");
        } else {
            INFO_NEWRES (arg_info)
              = TBmakeExprs (DUPdoDupTree (GENARRAY_DEFAULT (arg_node)),
                             INFO_NEWRES (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

node *
HZGWLmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == T_traverse) {
        arg_node = TRAVcont (arg_node, arg_info);
    } else {
        if (MODARRAY_NEXT (arg_node) != NULL) {
            MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
        }

        INFO_NEWRES (arg_info) = TBmakeExprs (DUPdoDupTree (MODARRAY_ARRAY (arg_node)),
                                              INFO_NEWRES (arg_info));
    }

    DBUG_RETURN (arg_node);
}

node *
HZGWLspfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == T_traverse) {
        arg_node = TRAVcont (arg_node, arg_info);
    } else {
        if (SPFOLD_NEXT (arg_node) != NULL) {
            SPFOLD_NEXT (arg_node) = TRAVdo (SPFOLD_NEXT (arg_node), arg_info);
        }

        INFO_NEWRES (arg_info) = TBmakeExprs (DUPdoDupTree (SPFOLD_NEUTRAL (arg_node)),
                                              INFO_NEWRES (arg_info));
    }

    DBUG_RETURN (arg_node);
}

node *
HZGWLpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_MODE (arg_info) == T_traverse) {
        arg_node = TRAVcont (arg_node, arg_info);
    } else {
        if (PROPAGATE_NEXT (arg_node) != NULL) {
            PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
        }

        INFO_NEWRES (arg_info) = TBmakeExprs (DUPdoDupTree (PROPAGATE_DEFAULT (arg_node)),
                                              INFO_NEWRES (arg_info));
    }

    DBUG_RETURN (arg_node);
}

node *
HZGWLlet (node *arg_node, info *arg_info)
{
    node *oldlhs;

    DBUG_ENTER ();

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    /**
     * remember whether we have a wl is non expression
     * position but at let-level.
     */
    INFO_EXPRPOS (arg_info) = (NODE_TYPE (LET_EXPR (arg_node)) != N_with);

    arg_node = TRAVcont (arg_node, arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = oldlhs;

    DBUG_RETURN (arg_node);
}

node *
HZGWLassign (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_POSTASSIGN (arg_info)) {
        tmp = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = INFO_POSTASSIGN (arg_info);
        arg_node = TCappendAssign (arg_node, tmp);

        INFO_POSTASSIGN (arg_info) = NULL;
    }

    if (INFO_DELETE (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_DELETE (arg_info) = FALSE;
    }

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;

        arg_node = TRAVcont (arg_node, arg_info);
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
HZGWLreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * remember whether we have a wl is non expression
     * position but at let-level.
     */
    INFO_EXPRPOS (arg_info) = TRUE;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
