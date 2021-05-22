/** <!--******************************************************************-->
 *
 * @file signature_simplification.c
 *
 * description:
 *   This optimization can be called from a module or function context In 
 *   case of the latter INFO_ONEFUNDEF is set to TRUE.
 *   The overall aim is to remove unused arguments of of functions as well
 *   as constant return values.
 *
 *   Initially, the idea was to use that for all functions that are local,
 *   i.e. not provided and not exported. However, that is a tricky business.
 *   Non-LaC-funs potentially have multiple call sites and they can be
 *   overloaded which is why they are always connected to their wrapper function.
 *   These wrapper functions assume a fixed number of arguments to be used
 *   for all instances. So changing just a single instance, while being 
 *   conceptually imaginable, would render these wrapoper functions non-expandable
 *   as all the code-generation assumes that all instances have the same number of
 *   arguments.
 *   Even if we restrict ourselves to function instances that have been
 *   statically dispatched, we would need to detach them from their wrappers
 *   and as such create a new breed of detached functions.
 *
 *   Given all these difficulties, we restrict ourselves to LaC funs.
 *   Cond-funs have exactly one call site, Loop funs have two. Furthermore,
 *   all LaC-funs are not overloadable, i.e., they have no wrappers.
 *
 * First, while traversing through the function body, all unused function
 * arguments are removed from the signature of the calling contexts and
 * constant scalar return values are inserted in the calling context.
 * Finally, constant scalar return values are removed from the
 * return-exprs-chain.
 * After modifying all function bodies, all function signatures (args and rets)
 * are modified.
 *
 * Unused function arguments are marked by (ARG_ISINUSE == FALSE), which
 * is set in SSADeadCodeRemoval.
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"

#define DBUG_PREFIX "SISI"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "shape.h"
#include "namespaces.h"
#include "tree_compound.h"
#include "inferneedcounters.h"
#include "lacfun_utilities.h"

#include "signature_simplification.h"

typedef enum { infer, simplify } travphases;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool remex;
    bool retex;
    bool isapnode;
    node *rets;
    node *apfunrets;
    bool remassign;
    bool idslet;
    node *preassign;
    travphases travphase;
    bool onefundef;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_RETURNEXPRS(n) (n->retex)
#define INFO_REMOVEEXPRS(n) (n->remex)
#define INFO_ISAPNODE(n) (n->isapnode)
#define INFO_RETS(n) (n->rets)
#define INFO_APFUNRETS(n) (n->apfunrets)
#define INFO_IDSLET(n) (n->idslet)
#define INFO_PREASSIGN(n) (n->preassign)

#define INFO_TRAVPHASE(n) (n->travphase)
#define INFO_ONEFUNDEF(n) (n->onefundef)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();
    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_RETURNEXPRS (result) = FALSE;
    INFO_REMOVEEXPRS (result) = FALSE;
    INFO_ISAPNODE (result) = FALSE;
    INFO_RETS (result) = NULL;
    INFO_APFUNRETS (result) = NULL;
    INFO_IDSLET (result) = FALSE;
    INFO_PREASSIGN (result) = NULL;

    INFO_TRAVPHASE (result) = infer;
    INFO_ONEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--***********************************************************************-->
 *
 * @fn node *IntegrateNewAssigns( node *assgns, node *arg_node, info *arg_info)
 *
 * @brief Insert new assignments into the called function
 *
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
IntegrateNewAssigns (node *assgns, node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ();

    if (NULL != assgns) {
        fundef = AP_FUNDEF (ASSIGN_STMT (arg_node));
        while (NULL != assgns) {
            fundef = LFUinsertAssignIntoLacfun (fundef, assgns,
                                                IDS_AVIS (ASSIGN_STMT (assgns)));
            assgns = ASSIGN_NEXT (assgns);
        }
        AP_FUNDEF (ASSIGN_STMT (arg_node)) = fundef;
    }

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *SISIdoSignatureSimplification(node *arg_node)
 *
 * @brief starting point of traversal
 *
 * @param N_fundef or N_module to be traversed
 *
 * @result
 *
 ******************************************************************************/
node *
SISIdoSignatureSimplification (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = (N_fundef == NODE_TYPE (arg_node));

    TRAVpush (TR_sisi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *SISImodule(node *arg_node, info *arg_info)
 *
 * @brief traverses in funs
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_TRAVPHASE (arg_info) = infer;
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    INFO_TRAVPHASE (arg_info) = simplify;
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *SISIfundef(node *arg_node, info *arg_info)
 *
 * @brief top-down: traverse body, bottom-up: traverse rets and args
 *
 * @param arg_node fundef to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Begin %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));
    if (INFO_TRAVPHASE (arg_info) == infer) {
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_sisi);
    } else if (INFO_TRAVPHASE (arg_info) == simplify) {
        INFO_FUNDEF (arg_info) = arg_node;

        INFO_RETS (arg_info) = FUNDEF_RETS (arg_node);

        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
        if (!INFO_ONEFUNDEF (arg_info)) {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }

        INFO_FUNDEF (arg_info) = arg_node;

        // Tranverse all lacfuns
        if ((FUNDEF_ISLACFUN (arg_node))) {
#ifdef FIXME
            if ((FUNDEF_ISLACFUN (arg_node))
                || ((!STReq ("main", FUNDEF_NAME (arg_node)))
                    && (!NSequals (NSgetRootNamespace (), FUNDEF_NS (arg_node)))
                    && (!FUNDEF_ISEXPORTED (arg_node))
                    && (!FUNDEF_ISPROVIDED (arg_node)))) {
#endif // FIXME  }
                FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);
            }

            if ((FUNDEF_BODY (arg_node) != NULL)) {
#ifdef FIXME
                if ((FUNDEF_BODY (arg_node) != NULL)
                    && ((FUNDEF_ISLACFUN (arg_node))
                        || ((!FUNDEF_ISEXPORTED (arg_node))
                            && (!FUNDEF_ISPROVIDED (arg_node))))) {
                    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
#endif // FIXME  }
                }
            } else {
                DBUG_UNREACHABLE ("Unexpected traversal phase!");
            }
            DBUG_PRINT ("End %s %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                        FUNDEF_NAME (arg_node));

            DBUG_RETURN (arg_node);
        }

        /** <!--***********************************************************************-->
         *
         * @fn node *SISIarg(node *arg_node, info *arg_info)
         *
         * @brief Delete unused arguments from args-chain.
         *
         *    Do not change signatures of exported or provided functions. Leave also
         *    LAC-functions as they are (ensured in SISIfundef).
         *
         * @param arg_node args-chain to be traversed
         * @param arg_info
         *
         * @result
         *
         ******************************************************************************/
        node *SISIarg (node * arg_node, info * arg_info)
        {
            node *tmp;

            DBUG_ENTER ();

            ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

            /*
             * now bottom-up traversal
             */

            DBUG_PRINT ("arg_chain element %s has AVIS_NEEDCOUNT=%d",
                        AVIS_NAME (ARG_AVIS ((arg_node))),
                        AVIS_NEEDCOUNT (ARG_AVIS (arg_node)));
            if (AVIS_NEEDCOUNT (ARG_AVIS (arg_node)) == 0) {
                /*
                 * we are dealing with an unused argument
                 * delete argument from chain
                 */
                DBUG_PRINT ("Removing arg_chain element %s",
                            AVIS_NAME (ARG_AVIS ((arg_node))));
                tmp = ARG_NEXT (arg_node);
                ARG_NEXT (arg_node) = NULL;
                arg_node = FREEdoFreeNode (arg_node);
                arg_node = tmp;

                global.optcounters.sisi_expr++;
            }

            DBUG_RETURN (arg_node);
        }

        /** <!--***********************************************************************-->
         *
         * @fn node *SISIblock(node *arg_node, info *arg_info)
         *
         * @brief
         *
         * @param arg_node node to be traversed
         * @param arg_info
         *
         * @result
         *
         ******************************************************************************/
        node *SISIblock (node * arg_node, info * arg_info)
        {
            DBUG_ENTER ();

            BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

            DBUG_RETURN (arg_node);
        }

        /** <!--***********************************************************************-->
         *
         * @fn node *SISIassign(node *arg_node, info *arg_info)
         *
         * @brief traverse in assignments, insert new assignments, if necessary,
         *        before the function call.
         *
         * @param arg_node node to be traversed
         * @param arg_info
         *
         * @result
         *
         ******************************************************************************/
        node *SISIassign (node * arg_node, info * arg_info)
        {
            DBUG_ENTER ();

            INFO_PREASSIGN (arg_info) = NULL;

            ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

            /* integrate post assignments, if any, after current assignment */
            arg_node
              = IntegrateNewAssigns (INFO_PREASSIGN (arg_info), arg_node, arg_info);
            INFO_PREASSIGN (arg_info) = NULL;

            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

            DBUG_RETURN (arg_node);
        }

        /** <!--***********************************************************************-->
         *
         * @fn node *SISIlet(node *arg_node, info *arg_info)
         *
         * @brief traverse first in rhs, then in lhs
         *
         * @param arg_node node to be traversed
         * @param arg_info
         *
         * @result
         *
         ******************************************************************************/
        node *SISIlet (node * arg_node, info * arg_info)
        {
            DBUG_ENTER ();

            INFO_ISAPNODE (arg_info) = FALSE;

            LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

            if ((INFO_ISAPNODE (arg_info)) && (INFO_APFUNRETS (arg_info) != NULL)) {

                INFO_IDSLET (arg_info) = TRUE;
                LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
                INFO_IDSLET (arg_info) = FALSE;
            }

            DBUG_RETURN (arg_node);
        }

        /** <!--***********************************************************************-->
         *
         * @fn node *SISIap(node *arg_node, info *arg_info)
         *
         * @brief check fundef-args for unused arguments,
         *      remove corresponding exprs from args-chain
         *
         * @param arg_node node to be traversed
         * @param arg_info
         *
         * @result
         *
         ******************************************************************************/
        node *SISIap (node * arg_node, info * arg_info)
        {
            node *fundef, *fun_args, *curr_args, *new_args, *tmp;
            DBUG_ENTER ();

            fundef = AP_FUNDEF (arg_node);

#ifdef FIXME
            if ((FUNDEF_ISLACFUN (fundef))
                || ((!FUNDEF_ISPROVIDED (fundef)) && (!FUNDEF_ISEXPORTED (fundef))
                    && (!FUNDEF_ISOBJINITFUN (fundef)) && (!FUNDEF_ISEXTERN (fundef)))) {
#endif // FIXME }
                if ((FUNDEF_ISLACFUN (fundef))) {

                    INFO_APFUNRETS (arg_info) = FUNDEF_RETS (AP_FUNDEF (arg_node));
                    fun_args = FUNDEF_ARGS (fundef);
                    curr_args = AP_ARGS (arg_node);
                    new_args = NULL;
                    AP_ARGS (arg_node) = NULL;

                    while (fun_args != NULL) {

                        if (AVIS_NEEDCOUNT (ARG_AVIS (fun_args)) > 0) {
                            if (NULL == new_args) {
                                new_args = curr_args;
                                AP_ARGS (arg_node) = new_args;
                            } else {
                                EXPRS_NEXT (new_args) = curr_args;
                                new_args = EXPRS_NEXT (new_args);
                            }
                            curr_args = EXPRS_NEXT (curr_args);
                        } else {

                            /*
                             * argument is marked as not needed
                             */

                            tmp = curr_args;
                            DBUG_PRINT ("Removing AP_ARG %s in call to %s",
                                        AVIS_NAME (ID_AVIS (EXPRS_EXPR (curr_args))),
                                        FUNDEF_NAME (fundef));
                            curr_args = EXPRS_NEXT (curr_args);
                            EXPRS_NEXT (tmp) = NULL;
                            tmp = FREEdoFreeNode (tmp);
                            if (new_args != NULL) {
                                EXPRS_NEXT (new_args) = NULL;
                            }
                        }
                        fun_args = ARG_NEXT (fun_args);
                    }

                    INFO_ISAPNODE (arg_info) = TRUE;
                }

                DBUG_RETURN (arg_node);
            }

            /**
             *<!--***********************************************************************-->
             *
             * @fn node *SISIids(node *arg_node, info *arg_info)
             *
             * @brief removes ids-nodes with corresponding scalar akv-types in ret-chain
             *
             * @param arg_node node to be traversed
             * @param arg_info
             *
             * @result
             *
             ******************************************************************************/
            node *SISIids (node * arg_node, info * arg_info)
            {
                node *succ, *ret, *assign_let;
                constant *new_co;
                DBUG_ENTER ();

                if (INFO_IDSLET (arg_info)) {
                    ret = INFO_APFUNRETS (arg_info);
                    if (RET_NEXT (INFO_APFUNRETS (arg_info)) != NULL) {
                        INFO_APFUNRETS (arg_info) = RET_NEXT (INFO_APFUNRETS (arg_info));
                    } else {
                        DBUG_ASSERT (IDS_NEXT (arg_node) == NULL,
                                     "ret and ids do not fit together");
                    }

                    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

                    /*
                     * bottom-up traversal
                     */

                    if ((TYisAKV (RET_TYPE (ret))) && (0 == TYgetDim (RET_TYPE (ret)))) {
                        new_co = TYgetValue (RET_TYPE (ret));

                        DBUG_PRINT ("identifier %s marked as constant",
                                    VARDEC_OR_ARG_NAME (AVIS_DECL (IDS_AVIS (arg_node))));

                        /*
                         * create one let assign for constant definition,
                         * reuse old avis/vardec
                         */
                        assign_let
                          = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (arg_node),
                                                                NULL),
                                                     COconstant2AST (new_co)),
                                          NULL);
                        AVIS_SSAASSIGN (IDS_AVIS (arg_node)) = assign_let;

                        /*
                         * append new copy assignment to then-part block
                         */
                        INFO_PREASSIGN (arg_info)
                          = TCappendAssign (INFO_PREASSIGN (arg_info), assign_let);

                        DBUG_PRINT ("create constant assignment for %s",
                                    (IDS_NAME (arg_node)));

                        /*
                         * Remove current ids
                         */
                        DBUG_PRINT ("Removing %s", AVIS_NAME (IDS_AVIS (arg_node)));
                        succ = IDS_NEXT (arg_node);
                        IDS_NEXT (arg_node) = NULL;
                        arg_node = FREEdoFreeNode (arg_node);
                        arg_node = succ;

                        global.optcounters.sisi_expr++;
                        FUNDEF_WASOPTIMIZED (INFO_FUNDEF (arg_info)) = TRUE;
                    }
                }

                DBUG_RETURN (arg_node);
            }

            /**
             *<!--***********************************************************************-->
             *
             * @fn node *SISIreturn(node *arg_node, info *arg_info)
             *
             * @brief traverses in exprs-chain
             *
             * @param arg_node node to be traversed
             * @param arg_info
             *
             * @result
             *
             ******************************************************************************/
            node *SISIreturn (node * arg_node, info * arg_info)
            {
                DBUG_ENTER ();

#ifdef FIXME
                if ((FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info)))
                    || ((!STReq ("main", FUNDEF_NAME (INFO_FUNDEF (arg_info))))
                        && (!NSequals (NSgetRootNamespace (),
                                       FUNDEF_NS (INFO_FUNDEF (arg_info))))
                        && (!FUNDEF_ISEXPORTED (INFO_FUNDEF (arg_info)))
                        && (!FUNDEF_ISPROVIDED (INFO_FUNDEF (arg_info))))) {
#endif // FIXME
                    if ((FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info)))) {

                        INFO_RETURNEXPRS (arg_info) = TRUE;

                        RETURN_EXPRS (arg_node)
                          = TRAVopt (RETURN_EXPRS (arg_node), arg_info);
                        INFO_RETURNEXPRS (arg_info) = FALSE;
                    }

                    DBUG_RETURN (arg_node);
                }

                /**
                 *<!--***********************************************************************-->
                 *
                 * @fn node *SISIret(node *arg_node, info *arg_info)
                 *
                 * @brief remove marked (already removed) rets
                 *
                 * @param arg_node node to be traversed
                 * @param arg_info
                 *
                 * @result
                 *
                 ******************************************************************************/
                node *SISIret (node * arg_node, info * arg_info)
                {
                    node *tmp;

                    DBUG_ENTER ();

                    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

                    /*
                     * remove bottom-up rets
                     */
                    if (RET_WASREMOVED (arg_node)) {
                        tmp = RET_NEXT (arg_node);
                        RET_NEXT (arg_node) = NULL;
                        arg_node = FREEdoFreeNode (arg_node);
                        arg_node = tmp;
                    }

                    DBUG_RETURN (arg_node);
                }

                /**
                 *<!--***********************************************************************-->
                 *
                 * @fn node *SISIexprs(node *arg_node, info *arg_info)
                 *
                 * @brief top-down: find and mark exprs to be removed,
                 *        bottom-up: remove marked exprs
                 *
                 * @param arg_node node to be traversed
                 * @param arg_info
                 *
                 * @result
                 *
                 ******************************************************************************/
                node *SISIexprs (node * arg_node, info * arg_info)
                {
                    bool remove;
                    node *ret;
                    node *tmp;

                    DBUG_ENTER ();

                    if (INFO_RETURNEXPRS (arg_info) == TRUE) {

                        INFO_REMOVEEXPRS (arg_info) = FALSE;

                        ret = INFO_RETS (arg_info);

                        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);

                        remove = INFO_REMOVEEXPRS (arg_info);

                        if (EXPRS_NEXT (arg_node) != NULL) {
                            INFO_RETS (arg_info) = RET_NEXT (INFO_RETS (arg_info));
                            EXPRS_NEXT (arg_node)
                              = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
                        }
                        /*
                         * remove bottom-up exprs
                         */

                        if (remove) {
                            tmp = EXPRS_NEXT (arg_node);
                            EXPRS_EXPR (arg_node) = NULL;
                            arg_node = FREEdoFreeNode (arg_node);
                            arg_node = tmp;

                            RET_WASREMOVED (ret) = TRUE;
                        }
                    }
                    DBUG_RETURN (arg_node);
                }

                /**
                 *<!--***********************************************************************-->
                 *
                 * @fn node *SISIid(node *arg_node, info *arg_info)
                 *
                 * @brief sets flag if id is scalar akv-type
                 *
                 * @param arg_node node to be traversed
                 * @param arg_info
                 *
                 * @result
                 *
                 ******************************************************************************/
                node *SISIid (node * arg_node, info * arg_info)
                {
                    DBUG_ENTER ();

                    if ((TYisAKV (AVIS_TYPE (ID_AVIS (arg_node))))
                        && (0 == TYgetDim (AVIS_TYPE (ID_AVIS (arg_node))))) {

                        INFO_REMOVEEXPRS (arg_info) = TRUE;
                        DBUG_PRINT ("Marking scalar constant %s for removal",
                                    AVIS_NAME (ID_AVIS (arg_node)));
                    }

                    DBUG_RETURN (arg_node);
                }

#undef DBUG_PREFIX
