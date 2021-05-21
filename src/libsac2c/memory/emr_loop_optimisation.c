/**
 * @file
 * @defgroup emr
 * @ingroup mm
 *
 * EMR Loop Memory Propagation
 *
 * This traversal lifts with-loop memory allocations out of the
 * inner-most loop. This is achieved by creating new arguments to the
 * loop function and updating the ERC value of the with-loop operation.
 *
 * This traversal can only work if RS (Reuse Optimisation) *and*
 * LRO (Loop Reuse Optimisation) are activated. We intentionally avoid
 * creating allocation calls at this stage, and leave this to the two
 * mentioned traversals.
 *
 * runs in two modes - INFO_CONTEXT is:
 *   EMRL_rec : fundef top-down traversal looking for loop funs, and
 *              updating fundef args and rec loop ap. We unset fundef_erc
 *              as these are no longer needed.
 *   EMRL_ap  : fundef top-down looking at fun_body at initial loop app -
 *              if the fundef has an updated signature, update the app
 *              args
 * @{
 */
#include "emr_loop_optimisation.h"
#include "emr_utils.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"

#define DBUG_PREFIX "EMRL"
#include "debug.h"

#include "print.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"

/**
 * @brief Traversal context
 *
 * We either are searching for loop-functions and their recursive
 * applications, or for the initial application.
 */
typedef enum emrl_context {EMRL_rec, EMRL_ap} emrl_context_t;

/**
 * Node of Stack Structure which is used to temporally
 * hold a new EMR tmp variable and the associated WL operation
 * node. See EMRL related functions for more info.
 */
typedef struct stack_node_s {
    node *wl;      /**< either a N_modarray or N_genarray */
    node *avis;    /**< our new avis */
    struct stack_node_s *next;
} stack_node_t;

/**
 * INFO structure
 */
struct INFO {
    node *fundef;           /**< current fundef */
    node *args;             /**< new args for current fundef */
    node *vardecs;          /**< new vardecs for current fundef */
    node *lhs;              /**< LHS of N_let */
    emrl_context_t context; /**< traversal context/mode */
    stack_node_t *stack;    /**< stack for (N_withop, N_avis) pairs */
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_ARGS(n) ((n)->args)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_CONTEXT(n) ((n)->context)
#define INFO_STACK(n) ((n)->stack)

/**
 * INFO create function
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_CONTEXT (result) = EMRL_rec;
    INFO_STACK (result) = NULL;

    DBUG_RETURN (result);
}

/**
 * INFO free function
 */
static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * @brief add a node to the top of the stack (creates the stack if first node)
 *
 * @param stack Stack head
 * @param wl Either N_modarray or N_genarray node
 * @param avis N_avis node
 * @return Top of the stack
 */
static stack_node_t *
stack_push (stack_node_t *stack, node *wl, node *avis)
{
    stack_node_t * new;
    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (wl) == N_genarray || NODE_TYPE (wl) == N_modarray),
                 "Second argument has wrong node_type!");
    DBUG_ASSERT ((NODE_TYPE (avis) == N_avis), "Third argument has wrong node_type!");

    new = MEMmalloc (sizeof (stack_node_t));
    new->wl = wl;
    new->avis = avis;
    new->next = stack;

    DBUG_RETURN (new);
}

/**
 * @brief drop (free) top-most node from stack
 *
 * @param stack Stack head
 * @return Next node in stack or NULL
 */
static stack_node_t *
stack_drop (stack_node_t *stack)
{
    stack_node_t * tmp;
    DBUG_ENTER ();

    tmp = stack->next;
    MEMfree (stack);

    DBUG_RETURN (tmp);
}

/**
 * @brief Clear (free) complete stack (all nodes)
 *
 * @param stack Stack head
 * @return NULL
 */
static stack_node_t *
stack_clear (stack_node_t *stack)
{
    DBUG_ENTER ();

    while (stack != NULL) {
        stack = stack_drop (stack);
    }

    DBUG_RETURN (stack);
}

/**
 * @brief Create a new temporary avis which copies the ntype of
 *        an existing avis
 *
 * @param type Some NType
 * @return a new avis
 */
static inline node *
createTmpAvis (ntype *type)
{
    node *avis;

    avis = TBmakeAvis (TRAVtmpVarName ("emr_tmp"), TYcopyType (type));

    DBUG_PRINT (" created %s var", AVIS_NAME (avis));

    return avis;
}

/**
 * @brief Collect LHS of N_let and traverse the exprs
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRLlet (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse only N_withop nodes
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRLwith (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    /* we must traverse withops first */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /* in some instances we may 'lift' a loop which is called from within
     * a WL. In order to update the arguments of its application we need
     * to traverse through the N_blocks of the WL.
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief determine if we need to lift an allocation
 *
 * If we are in the EMRL_rec context and the N_genarray has no RC/ERCs, we
 * create a new ERC. We *do not* add this to the ERC N_genarray, but place it
 * into a stack with a pointer to N_withop.
 *
 * For every traversal we move to the next LHS and N_genarray.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRLgenarray (node * arg_node, info * arg_info)
{
    node * new_avis;
    DBUG_ENTER ();

    if (INFO_CONTEXT (arg_info) == EMRL_rec
        && FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
        && GENARRAY_RC (arg_node) == NULL
        && GENARRAY_ERC (arg_node) == NULL
        && TYisAKS (IDS_NTYPE (INFO_LHS (arg_info)))) {
        DBUG_PRINT (" genarray in loopfun has no RCs or ERCs, generating tmp one!");

        /* the new avis must have the same type/shape as genarray shape */
        new_avis = createTmpAvis (IDS_NTYPE (INFO_LHS (arg_info)));

        /* add to stack - this will be used in N_ap */
        INFO_STACK (arg_info) = stack_push (INFO_STACK (arg_info), arg_node, new_avis);
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief determine if we need to lift an allocation
 *
 * If we are in the EMRL_rec context and the N_modarray has no RC/ERCs, we
 * create a new ERC. We *do not* add this to the ERC N_modarray, but place it
 * into a stack with a pointer to N_withop.
 *
 * For every traversal we move to the next LHS and N_modarray.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRLmodarray (node * arg_node, info * arg_info)
{
    node * new_avis;
    DBUG_ENTER ();

    if (INFO_CONTEXT (arg_info) == EMRL_rec
        && FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
        && MODARRAY_RC (arg_node) == NULL
        && MODARRAY_ERC (arg_node) == NULL
        && TYisAKS (IDS_NTYPE (INFO_LHS (arg_info)))) {
        DBUG_PRINT (" modarray in loopfun has no RCs or ERCs, generating tmp one!");

        /* the new avis must have the same type/shape as modarray shape */
        new_avis = createTmpAvis (IDS_NTYPE (INFO_LHS (arg_info)));

        /* add to stack - this will be used in N_ap */
        INFO_STACK (arg_info) = stack_push (INFO_STACK (arg_info), arg_node, new_avis);
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief extend either loop function recursive or initial applications
 *        with new arguments
 *
 * If we are in the EMRL_rec context and we've found the loop function
 * recursive call, we go through INFO_STACK looking for free variables within
 * the loop function. If we find candidates, we add the our N_avis from N_withop
 * as an ERC to the N_withop, and as an argument to the loop function (via
 * INFO_ARGS). We also update the recursive call with our candidate free variable.
 *
 * If we are in the EMRL_ap context, we check if the N_ap's N_fundef has new
 * arguments and create a new one for the current context. We add these to the
 * current N_fundef vardecs via INFO_VARDECS.
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRLap (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))) {
        DBUG_PRINT ("checking application of %s ...", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /* check to see if we have found the recursive loopfun call */
        if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)
                && INFO_CONTEXT (arg_info) == EMRL_rec) {
            DBUG_PRINT ("  this is the recursive loop application");

            if (INFO_STACK (arg_info) != NULL) {
                node * rec_filt, * find;

                DBUG_PRINT ("  have found tmp vars at application's fundef");

                /* we filter out current application args */
                rec_filt = filterDuplicateId (AP_ARGS (arg_node),
                                              &FUNDEF_ERC (INFO_FUNDEF (arg_info)));

                /* for each (wl, erc_avis) in the stack, find a suitable var in function
                 * free variables and replace it. if none can be found, drop/free
                 * erc_avis. We pop the stack on each iteration.                        */
                DBUG_EXECUTE (if (rec_filt != NULL) { PRTdoPrint (rec_filt); });
                while (INFO_STACK (arg_info) != NULL) {
                    find = isSameShapeAvis (INFO_STACK (arg_info)->avis, rec_filt);
                    DBUG_EXECUTE (if (find != NULL) { PRTdoPrint (find);});
                    if (find == NULL) {
                        DBUG_PRINT ("  no suitable free variable found for %s, dropping "
                                    "it",
                                    AVIS_NAME (INFO_STACK (arg_info)->avis));
                        INFO_STACK (arg_info)->avis
                          = FREEdoFreeTree (INFO_STACK (arg_info)->avis);
                    } else {
                        DBUG_PRINT ("  found free variable %s for %s at loop rec-call", ID_NAME (find), AVIS_NAME (INFO_STACK (arg_info)->avis));
                        L_WITHOP_ERC (INFO_STACK (arg_info)->wl,
                                      TBmakeExprs (TBmakeId (INFO_STACK (arg_info)->avis),
                                                   NULL));
                        INFO_ARGS (arg_info)
                          = TCappendArgs (INFO_ARGS (arg_info),
                                          TBmakeArg (INFO_STACK (arg_info)->avis, NULL));
                        AP_ARGS (arg_node)
                          = TCappendExprs (AP_ARGS (arg_node),
                                           TBmakeExprs (DUPdoDupNode (find), NULL));
                    }
                    INFO_STACK (arg_info) = stack_drop (INFO_STACK (arg_info));
                }

                /* fundef-level ERCs are no longer needed after this point */
                FUNDEF_ERC (INFO_FUNDEF (arg_info))
                  = FREEdoFreeTree (FUNDEF_ERC (INFO_FUNDEF (arg_info)));
            }
        } else if (INFO_CONTEXT (arg_info) == EMRL_ap) { /* we are at the initial application */
            size_t ap_arg_len = TCcountExprs (AP_ARGS (arg_node));
            size_t fun_arg_len = TCcountArgs (FUNDEF_ARGS (AP_FUNDEF (arg_node)));

            if (ap_arg_len != fun_arg_len) {
                DBUG_PRINT ("  number args for ap do not match fundef: %zu != %zu",
                            ap_arg_len, fun_arg_len);

                /* we *always* append new args on fundef */
                for (; ap_arg_len < fun_arg_len; ap_arg_len++) {
                    node *tmp
                      = TCgetNthArg (ap_arg_len, FUNDEF_ARGS (AP_FUNDEF (arg_node)));
                    node *new_avis = NULL;
                    node *new_vardec = NULL;
                    DBUG_PRINT ("  creating a new arg...");

                    /* we create a new variable and declare it to have the same shape as
                     * the emr tmp variable in the fundef args. Within ELMR
                     * (loopreuseopt.c) this variable is then allocated using that shape
                     * information.
                     */
                    new_avis = TBmakeAvis (TRAVtmpVarName ("emr_lifted"),
                                           TYcopyType (ARG_NTYPE (tmp)));
                    AVIS_ISALLOCLIFT (new_avis) = TRUE;

                    AP_ARGS (arg_node)
                      = TCappendExprs (AP_ARGS (arg_node),
                                       TBmakeExprs (TBmakeId (new_avis), NULL));

                    new_vardec = TBmakeVardec (new_avis, NULL);
                    AVIS_DECLTYPE (VARDEC_AVIS (new_vardec))
                      = TYcopyType (ARG_NTYPE (tmp));
                    INFO_VARDECS (arg_info)
                      = TCappendVardec (INFO_VARDECS (arg_info), new_vardec);
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief traverse only N_fundef body and update N_args and N_vardecs where
 *        appropriate
 *
 * @note
 * We currently do not handle the case where the inner most loop, is within
 * a N_cond. That is to say, we do lift the allocation, but only to the level
 * of the N_cond, and not further _up_. This is in itself the problem, which
 * more generally concerns itself with how many levels up we want to lift
 * allocations. Currently we only do this one-level up, and for the examples
 * we use (Livermore Loops, Rodinia, SaC Demos, etc.) this gives good results.
 * If we want to better handle N_cond, we need to consider this. This issue also
 * becomes more complicated as the N_cond might in one branch do some WL operation,
 * and in the other nothing. Do we staticly introduce another allocation outside
 * the N_cond, risking that it never gets used? (2018, Hans)
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
EMRLfundef (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("at N_fundef %s ...", FUNDEF_NAME (arg_node));
        if (FUNDEF_ISLOOPFUN (arg_node)) {
            DBUG_PRINT ("  found loopfun, inspecting body...");
        } else {
            DBUG_PRINT ("  inspecting body...");
        }

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* append collected vardecs to fundef */
        if (INFO_VARDECS (arg_info) != NULL) {
            FUNDEF_VARDECS (arg_node)
              = TCappendVardec (FUNDEF_VARDECS (arg_node), INFO_VARDECS (arg_info));
            INFO_VARDECS (arg_info) = NULL;
        }

        /* append collected args to fundef */
        if (INFO_ARGS (arg_info) != NULL) {
            FUNDEF_ARGS (arg_node)
              = TCappendArgs (FUNDEF_ARGS (arg_node), INFO_ARGS (arg_info));
            INFO_ARGS (arg_info) = NULL;

            /* mark fundef as having been touched by EMRL - this used later in CUDA MEMRT */
            FUNDEF_ISEMRLIFTED (arg_node) = TRUE;
        }

        INFO_FUNDEF (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief driver function for EMRL traversal
 *
 * @param syntax_tree
 * @return
 */
node *
EMRLdoExtendLoopMemoryPropagation (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_emrl);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    // make sure the stack is cleared
    INFO_STACK (arg_info) = stack_clear (INFO_STACK (arg_info));

    // switch to update mode
    INFO_CONTEXT (arg_info) = EMRL_ap;
    DBUG_PRINT ("Repeating traversal, updating loop N_ap");

    TRAVpush (TR_emrl);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/* @} */

#undef DBUG_PREFIX
