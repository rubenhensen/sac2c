/**
 * @defgroup emr
 *
 * @ingroup mm
 *
 * @{
 */

/**
 * @file
 */
#include <sys/queue.h>

#include "emr_loop_optimisation.h"

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
 * Node of Stack Structure which is used to temporally
 * hold a new EMR tmp variable and the associated WL operation
 * node. See EMRL related functions for more info.
 */
typedef struct wl_emr_stack_node_s {
    node * wl;
    node * avis;
    SLIST_ENTRY (wl_emr_stack_node_s) nodes;
} wl_emr_stack_node_t;

/**
 * Stack Structure based on wl_emr_stack_node_s.
 */
typedef SLIST_HEAD (wl_emr_stack_s, wl_emr_stack_node_s) wl_emr_stack_t;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lhs;
    wl_emr_stack_t *stack;
    bool do_update;
    node *tmp_rcs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_STACK(n) ((n)->stack)
#define INFO_DO_UPDATE(n) ((n)->do_update)
#define INFO_TMP_RCS(n) ((n)->tmp_rcs)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;
    wl_emr_stack_t * newstack;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    newstack = MEMmalloc (sizeof (wl_emr_stack_t));

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_STACK (result) = newstack;
    INFO_DO_UPDATE (result) = FALSE;
    INFO_TMP_RCS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_STACK (info) = MEMfree (INFO_STACK (info));
    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * @brief
 *
 * @param exprs N_exprs chain
 * @param id N_id
 *
 * @return true or false
 */
static bool
doAvisMatch (node * exprs, node * id)
{
    if (exprs == NULL) {
        return FALSE;
    } else {
        if (ID_AVIS (id) == ID_AVIS (EXPRS_EXPR (exprs))) {
            return TRUE;
        } else {
            return doAvisMatch (EXPRS_NEXT (exprs), id);
        }
    }
}

/**
 * @brief
 *
 * @param fexprs N_exprs chain of N_id that are to be found
 * @param exprs N_exprs chain of N_id that is to be filtered
 *
 * @return node N_exprs chain after filtering
 */
static node *
filterDuplicateArgs (node * fexprs, node ** exprs)
{
    node * filtered;
    DBUG_ENTER ();

    DBUG_PRINT ("filtering out duplicate N_avis");

    filtered = TCfilterExprsArg (doAvisMatch, fexprs, exprs);

    /* we delete all duplicates from col */
    if (filtered != NULL) {
        DBUG_PRINT ("  found and removed the following duplicates:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, filtered));
        filtered = FREEdoFreeTree (filtered);
    }

    DBUG_RETURN (*exprs);
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

/**
 * EMR Loop Memory Propagation
 *
 * This traversal lifts with-loop memory allocations out of the
 * inner-most loop. This is achieved by creating new arguments to the
 * loop function and updating the RC value of the with-loop operator.
 *
 * This traversal can only work if RS (Reuse Optimisation) *and*
 * LRO (Loop Reuse Optimisation) are activated (functioning?). We
 * intentionally avoid creating allocation calls at this stage, and leave
 * this to the two mentioned traversals.
 *
 * runs in two modes - INFO_DO_UPDATE is:
 *   FALSE : fundef top-down traversal looking for loop funs, and
 *           updating fundef args and rec loop ap. We unset fundef_erc
 *           as these are no longer needed.
 *   TRUE  : fundef top-down looking at fun_body at initial loop app -
 *           if the fundef has an updated signature, update the app
 *           args
 */
node *EMRLdoExtendLoopMemoryPropagation (node *syntax_tree)
{
    info *arg_info;
    wl_emr_stack_node_t * tmp;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    SLIST_INIT (INFO_STACK (arg_info));

    TRAVpush (TR_emrl);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    while (!SLIST_EMPTY (INFO_STACK (arg_info)))
    {
        tmp = SLIST_FIRST (INFO_STACK (arg_info));
        SLIST_REMOVE_HEAD (INFO_STACK (arg_info), nodes);
        tmp = MEMfree (tmp);
    }

    INFO_DO_UPDATE (arg_info) = TRUE;
    DBUG_PRINT ("Repeating traversal, updating loop N_ap");

    TRAVpush (TR_emrl);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/**
 * @brief
 *
 * @param avis
 * @param exprs
 *
 * @return
 */
static node *
isSameShapeAvis (node * avis, node * exprs)
{
    node * ret;
    DBUG_ENTER ();

    if (exprs == NULL) {
        ret = NULL;
    } else {
        if ((ShapeMatch (AVIS_TYPE (avis), ID_NTYPE (EXPRS_EXPR (exprs)))
              || TCshapeVarsMatch (avis, ID_AVIS (EXPRS_EXPR (exprs))))
             && TUeqElementSize (AVIS_TYPE (avis), ID_NTYPE (EXPRS_EXPR (exprs))))
            ret = EXPRS_EXPR (exprs);
        else
            ret = isSameShapeAvis (avis, EXPRS_NEXT (exprs));
    }

    DBUG_RETURN (ret);
}

/**
 * @brief
 *
 * @param id
 * @param exprs
 *
 * @return
 */
static node *
isSameShape (node * id, node * exprs)
{
    node * ret;
    DBUG_ENTER ();

    ret = isSameShapeAvis (ID_AVIS (id), exprs);

    DBUG_RETURN (ret);
}

node *
EMRLlet (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
EMRLwith (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
EMRLgenarray (node * arg_node, info * arg_info)
{
    node * new_avis;
    DBUG_ENTER ();

    if (!INFO_DO_UPDATE (arg_info)
            && FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
            && GENARRAY_RC (arg_node) == NULL
            && GENARRAY_ERC (arg_node) == NULL
            && TYisAKS (IDS_NTYPE (INFO_LHS (arg_info)))) {
        DBUG_PRINT (" genarray in loopfun has no RCs or ERCs, generating tmp one!");

        // the new avis must have the same type/shape as genarray shape
        new_avis = TBmakeAvis ( TRAVtmpVarName ("emr_tmp"),
                TYcopyType (IDS_NTYPE (INFO_LHS (arg_info))));

        DBUG_PRINT (" created %s var", AVIS_NAME (new_avis));
        wl_emr_stack_node_t * node = MEMmalloc (sizeof (wl_emr_stack_node_t));
        node->wl = arg_node;
        node->avis = new_avis;
        SLIST_INSERT_HEAD (INFO_STACK (arg_info), node, nodes);
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EMRLmodarray (node * arg_node, info * arg_info)
{
    node * new_avis;
    DBUG_ENTER ();

    if (!INFO_DO_UPDATE (arg_info)
            && FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
            && MODARRAY_RC (arg_node) == NULL
            && MODARRAY_ERC (arg_node) == NULL
            && TYisAKS (IDS_NTYPE (INFO_LHS (arg_info)))) {
        DBUG_PRINT (" modarray in loopfun has no RCs or ERCs, generating tmp one!");

        // the new avis must have the same type/shape as genarray shape
        new_avis = TBmakeAvis ( TRAVtmpVarName ("emr_tmp"),
                TYcopyType (IDS_NTYPE (INFO_LHS (arg_info))));

        DBUG_PRINT (" created %s var", AVIS_NAME (new_avis));
        wl_emr_stack_node_t * node = MEMmalloc (sizeof (wl_emr_stack_node_t));
        node->wl = arg_node;
        node->avis = new_avis;
        SLIST_INSERT_HEAD (INFO_STACK (arg_info), node, nodes);
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EMRLap (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))) {
        DBUG_PRINT ("checking application of %s ...", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /* check to see if we have found the recursive loopfun call */
        if (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info)
                && !INFO_DO_UPDATE (arg_info)) {
            DBUG_PRINT ("  this is the recursive loop application");

            if (INFO_STACK (arg_info) != NULL) {
                wl_emr_stack_node_t * tmpnode;
                node * rec_filt, * find;

                DBUG_PRINT ("  have found tmp vars at application's fundef");

                /* we filter out current application args */
                rec_filt = filterDuplicateArgs (AP_ARGS (arg_node), &FUNDEF_ERC (INFO_FUNDEF (arg_info)));

                /* for each (wl, erc_avis) in the stack, find a suitable var in function free variables and replace it.
                 * if none can be found, drop/free erc_avis. We pop the stack on each iteration.                        */
                while (!SLIST_EMPTY (INFO_STACK (arg_info)))
                {
                    tmpnode = SLIST_FIRST (INFO_STACK (arg_info));
                    find = isSameShapeAvis (tmpnode->avis, rec_filt);
                    if (find == NULL) {
                        DBUG_PRINT ("  no suitable free variable found for %s, dropping", AVIS_NAME (tmpnode->avis));
                        tmpnode->avis = FREEdoFreeTree (tmpnode->avis);
                    } else {
                        L_WITHOP_ERC (tmpnode->wl, TBmakeExprs (TBmakeId (tmpnode->avis), NULL));
                        FUNDEF_ARGS (INFO_FUNDEF (arg_info)) = TCappendArgs (FUNDEF_ARGS (INFO_FUNDEF (arg_info)), TBmakeArg (tmpnode->avis, NULL));
                        AP_ARGS (arg_node) = TCappendExprs (AP_ARGS (arg_node), TBmakeExprs (DUPdoDupNode (find), NULL));
                    }
                    SLIST_REMOVE_HEAD (INFO_STACK (arg_info), nodes);
                    tmpnode = MEMfree (tmpnode);
                }

                DBUG_PRINT ("  fun args are now:");
                DBUG_EXECUTE (if (FUNDEF_ARGS (INFO_FUNDEF (arg_info)) != NULL) {
                        PRTdoPrintFile (stderr, FUNDEF_ARGS (INFO_FUNDEF (arg_info)));});

                DBUG_PRINT ("  app args are now:");
                DBUG_EXECUTE (if (AP_ARGS (arg_node) != NULL) {
                        PRTdoPrintFile (stderr, AP_ARGS (arg_node));});

                /* clear the fundef ERC */
                FUNDEF_ERC (INFO_FUNDEF (arg_info)) = FREEdoFreeTree (FUNDEF_ERC (INFO_FUNDEF (arg_info)));
            }
        } else if (INFO_DO_UPDATE (arg_info)) { /* we are at the initial application */
            node * new_avis = NULL;
            node * new_vardec = NULL;
            int ap_arg_len = TCcountExprs (AP_ARGS (arg_node));
            int fun_arg_len = TCcountArgs (FUNDEF_ARGS (AP_FUNDEF (arg_node)));

            if (ap_arg_len != fun_arg_len) {
                DBUG_PRINT ("  number args for ap do not match fundef: %d != %d", ap_arg_len, fun_arg_len);

                /* we *always* append new args on fundef */
                for (; ap_arg_len < fun_arg_len; ap_arg_len++)
                {
                    /* we create a new variable and declare it to have the same shape as the
                     * emr tmp variable in the fundef args. Within ELMR this variable is then
                     * allocated using that shape information.
                     */
                    node * tmp = TCgetNthArg (ap_arg_len, FUNDEF_ARGS (AP_FUNDEF (arg_node)));
                    DBUG_PRINT ("  creating a new arg...");

                    /* the new avis must have the same type/shape as tmp arg in the fundef */
                    new_avis = TBmakeAvis ( TRAVtmpVarName ("emr_lifted"),
                            TYcopyType (ARG_NTYPE (tmp)));
                    AVIS_ISALLOCLIFT (new_avis) = TRUE;

                    /* append the avis to the args of ap */
                    AP_ARGS (arg_node) = TCappendExprs (AP_ARGS (arg_node), TBmakeExprs (TBmakeId (new_avis), NULL));

                    /* append to the current fundef's vardecs */
                    new_vardec = TBmakeVardec (new_avis, NULL);
                    AVIS_DECLTYPE (VARDEC_AVIS (new_vardec)) = TYcopyType (ARG_NTYPE (tmp));
                    INFO_FUNDEF (arg_info) = TCaddVardecs (INFO_FUNDEF (arg_info), new_vardec);
                    DBUG_PRINT ("  appended %s to fundef %s vardecs", AVIS_NAME (new_avis), FUNDEF_NAME (INFO_FUNDEF (arg_info)));
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

node *
EMRLfundef (node * arg_node, info * arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("at N_fundef %s ...", FUNDEF_NAME (arg_node));
        INFO_FUNDEF (arg_info) = arg_node;

        if (FUNDEF_ISLOOPFUN (arg_node)) {
            DBUG_PRINT ("  found loopfun, inspecting body...");
        } else {
            DBUG_PRINT ("  inspecting body...");
        }

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_FUNDEF (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}


/* @} */

#undef DBUG_PREFIX
