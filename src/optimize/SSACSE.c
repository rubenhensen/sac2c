/*
 *
 * $Log$
 * Revision 1.17  2001/05/23 15:47:55  nmw
 * comments added
 *
 * Revision 1.16  2001/05/18 13:30:42  nmw
 * identifer bypassing for special fundefs implemented
 *
 * Revision 1.15  2001/05/17 11:54:47  nmw
 * inter procedural copy propagation implemented
 *
 * Revision 1.14  2001/05/09 12:21:55  nmw
 * cse checks expressions for their used shape before substituting them
 *
 * Revision 1.13  2001/04/20 11:18:02  nmw
 * unused code removed
 *
 * Revision 1.12  2001/04/19 08:03:33  dkr
 * macro F_PTR used as format string for pointers
 *
 * Revision 1.11  2001/04/18 12:55:42  nmw
 * debug output for OPT traversal added
 *
 * Revision 1.10  2001/04/02 12:00:09  nmw
 * set INFO_SSACSE_ASSIGN in SSACSEassign
 *
 * Revision 1.9  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.8  2001/03/23 09:29:54  nmw
 * SSACSEdo/while removed and copy propagation implemented
 *
 * Revision 1.7  2001/03/22 21:13:23  dkr
 * include of tree.h eliminated
 *
 * Revision 1.6  2001/03/16 11:57:51  nmw
 * AVIS_SSAPHITRAGET type changed
 *
 * Revision 1.5  2001/03/15 14:23:24  nmw
 * SSACSE does not longer modify unique vardecs
 *
 * Revision 1.4  2001/03/12 09:17:05  nmw
 * do not substitute SSAPHITARGET
 *
 * Revision 1.3  2001/03/07 15:58:35  nmw
 * SSA Common Subexpression Elimination implemented
 *
 * Revision 1.2  2001/03/05 17:11:28  nmw
 * no more warnings
 *
 * Revision 1.1  2001/03/05 16:02:25  nmw
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   SSACSE.c
 *
 * prefix: SSACSE
 *
 * description:
 *   This module does the Common Subexpression Elimination in the AST for a
 *   single function (including the special functions in order of application).
 *   Therefore the RHS of each expression is compared (literally !!! - as we
 *   have code in ssa form this is really easy). If there was an identical
 *   expression before, we substitute the current LHS with this old LHS
 *   identifier, like:
 *       a = f(1,3,x);               a = f(1,3,x)
 *       b = 4 + a;          -->     b = 4 + a;
 *       c = f(1,3,x);               <removed>
 *       d = 4 + c;                  <removed, because c == a, so d == b )
 *       e = c + d;                  e = a + b;
 *
 *   the implementation is a stack of lists with available expressions in the
 *   current context (needed for conditionals and with-loops). for each
 *   expression we do an tree-compare with the available CSE we stored before.
 *   in addition to the tree-compare we have to check the types of the
 *   assigned LHS to be equal to avoid wrong substitutions of constant
 *   RHS arrays (e.g. [1,2,3,4] used as [[1,2],[3,4]]).
 *   if we find a matching one, we substitute the LHS otherwise we add this
 *   new expression to the list of available expressions. ##nmw##
 *
 *
 *   It also does a copy propagation for assignments like
 *       a = f(x);                   a = f(x)l
 *       b = a;              -->     <removed>
 *       c = b;                      <removed>
 *       d = g(c);                   d = g(a);
 *
 *   these optimizations does not happen for identifier of global objects or
 *   other unique types, because the current used implementation of objects
 *   does not use a global word object for creation, so we get the following
 *   problem:
 *       obj1  = CreateNewObj();        obj1  = CreateNewObj();
 *       obj1' = modify(1, obj1);  -->  obj1' = modify(obj1);
 *       obj2  = CreateNewObj();        <removed>
 *       obj2' = modify(2, obj2);       obj2' = modify(obj1);
 *                                                    ******
 *   this is wrong code and no more unique (two accesses to obj1)!!!
 *
 *
 *   the copy propagation information is propagates in special fundefs for
 *   the called args (all for cond-funs, loop invariant ones for loop-funs).
 *   for details see SSACSEPropagateSubst2Args() in this file.
 *
 *   if we find results of special fundes that are arguments of the fundef,
 *   we try to avoid a passing of this arg through the special fundef. instead
 *   we use the correct result id directly in the calling context. for details
 *   see SSACSEBuildSubstNodelist().
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSACSE.h"
#include "compare_tree.h"
#include "optimize.h"
#include "typecheck.h"

typedef enum { THENPART, ELSEPART } condpart;

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSACSEids (ids *arg_ids, node *arg_info);

/* functions to handle cseinfo chains */
static node *AddCSEinfo (node *cseinfo, node *let);
static node *CreateNewCSElayer (node *cseinfo);
static node *RemoveTopCSElayer (node *cseinfo);
static ids *SetSubstAttributes (ids *subst, ids *with);

/* helper functions for internal use only */
static node *FindCSE (node *cselist, node *let);
static bool ForbiddenSubstitution (ids *chain);
static bool CmpIdsTypes (ids *ichain1, ids *ichain2);
static node *SSACSEPropagateSubst2Args (node *fun_args, node *ap_args, node *fundef);
static nodelist *SSACSEBuildSubstNodelist (node *return_exprs, node *fundef);
static node *GetResultArgAvis (node *id, condpart cp);
static node *GetApAvisOfArgAvis (node *arg_avis, node *fundef);

/******************************************************************************
 *
 * function:
 *   node *AddCSEinfo(node *cseinfo, node *let)
 *
 * description:
 *   Adds an new let node to the actual cse layer to store an available
 *   expression in the curreent context.
 *
 ******************************************************************************/
static node *
AddCSEinfo (node *cseinfo, node *let)
{
    DBUG_ENTER ("AddCSEinfo");

    DBUG_ASSERT ((cseinfo != NULL), "cseinfo layer stack is NULL");
    DBUG_ASSERT ((let != NULL), "add NULL let to cseinfo layer stack");

    DBUG_PRINT ("SSACSE", ("add let node " F_PTR " to cse layer " F_PTR, let, cseinfo));

    if (CSEINFO_LET (cseinfo) == NULL) {
        /* add letnode top existing empty cseinfo node */
        CSEINFO_LET (cseinfo) = let;
    } else {
        /*
         * create new cseinfo node with let attribute on actual layer
         * and adds it in front of the cseinfo chain.
         */
        cseinfo = MakeCSEinfo (cseinfo, CSEINFO_LAYER (cseinfo), let);
    }

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *CreateNewCSElayer(node *cseinfo)
 *
 * description:
 *   starts a new layer of cse let nodes. this happens when we enter a new
 *   withloop or a then/else part of an conditional.
 *
 ******************************************************************************/
static node *
CreateNewCSElayer (node *cseinfo)
{
    DBUG_ENTER ("CreateNewCSElayer");

    cseinfo = MakeCSEinfo (cseinfo, NULL, NULL);

    DBUG_PRINT ("SSACSE", ("create new cse layer " F_PTR, cseinfo));

    /* set selfreference to mark new starting layer */
    CSEINFO_LAYER (cseinfo) = cseinfo;

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *RemoveTopCSElayer(node *cseinfo)
 *
 * description:
 *   removes all cseinfo nodes from the current cse layer. last cseinfo
 *   node on layer is marked with selfreference in CSEINFO_LAYER.
 *   this happens when we leave a withloop body or then/else parts of a
 *   conditional and the here collected expressions are no longer available.
 *
 ******************************************************************************/
static node *
RemoveTopCSElayer (node *cseinfo)
{
    node *tmp;
    node *freetmp;

    DBUG_ENTER ("RemoveTopCSElayer");

    tmp = cseinfo;

    while (tmp != NULL) {
        freetmp = tmp;
        cseinfo = CSEINFO_NEXT (tmp);

        if (tmp == CSEINFO_LAYER (tmp)) {
            tmp = NULL;
        } else {
            tmp = CSEINFO_NEXT (tmp);
        }
        DBUG_PRINT ("SSACSE", ("removing csenode " F_PTR, freetmp));
        FreeNode (freetmp);
    }

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *FindCSE(node *cselist, node *let)
 *
 * description:
 *   traverses the cseinfo chain until matching expression is found and
 *   return this let node. else return NULL. the matching is computed via
 *   a compare tree betwenn the given let and each stored expression.
 *
 * remark:
 *   the type compare is necessary to avoid wrong array replacements,
 *   e.g. [1,2,3,4] that is used as [[1,2],[3,4]] or [1,2,3,4]. These
 *   expressions uses the same RHS but are assigned to different shaped LHS.
 *   So if we do a simple replacement we get type mismatches.
 *
 ******************************************************************************/
static node *
FindCSE (node *cselist, node *let)
{
    node *match;
    node *csetmp;

    DBUG_ENTER ("FindCSE");
    DBUG_ASSERT ((let != NULL), "FindCSE is called with empty let node");

    match = NULL;
    csetmp = cselist;

    while ((csetmp != NULL) && (match == NULL)) {
        if ((CSEINFO_LET (csetmp) != NULL)
            && (CompareTree (LET_EXPR (let), LET_EXPR (CSEINFO_LET (csetmp))) == CMPT_EQ)
            && (CmpIdsTypes (LET_IDS (let), LET_IDS (CSEINFO_LET (csetmp))) == TRUE)) {
            match = CSEINFO_LET (csetmp);
        }
        csetmp = CSEINFO_NEXT (csetmp);
    }

    DBUG_RETURN (match);
}

/******************************************************************************
 *
 * function:
 *    bool CmpIdsTypes(ids *ichain1, ids *ichain2)
 *
 * description:
 *    compares the types of all ids in two given ids chains.
 *    return TRUE if the types of each two corresponding ids are equal.
 *
 ******************************************************************************/
static bool
CmpIdsTypes (ids *ichain1, ids *ichain2)
{
    bool result;
    DBUG_ENTER ("CmpIdsTypes");

    if (ichain1 != NULL) {
        DBUG_ASSERT ((ichain2 != NULL), "comparing different ids chains");
        if (CmpTypes (VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (IDS_AVIS (ichain1))),
                      VARDEC_OR_ARG_TYPE (AVIS_VARDECORARG (IDS_AVIS (ichain2))))
            == 1) {
            result = CmpIdsTypes (IDS_NEXT (ichain1), IDS_NEXT (ichain2));
        } else {
            result = FALSE;
        }
    } else {
        /* no types are equal */
        DBUG_ASSERT ((ichain2 == NULL), "comparing ids chains of different length");
        result = TRUE;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   ids *SetSubstAttributes(ids *subst, ids *with)
 *
 * description:
 *   set AVIS_SUBST attribute for all ids to the corresponding avis node from
 *   the with-ids chain (that we get from the stored available let expression).
 *
 ******************************************************************************/
static ids *
SetSubstAttributes (ids *subst, ids *with)
{
    ids *tmpsubst;
    ids *tmpwith;

    DBUG_ENTER ("SetSubstAttributes");

    tmpsubst = subst;
    tmpwith = with;

    while (tmpsubst != NULL) {
        DBUG_PRINT ("SSACSE",
                    ("substitute ids %s with %s",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (tmpsubst))),
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (tmpwith)))));

        AVIS_SUBST (IDS_AVIS (tmpsubst)) = IDS_AVIS (tmpwith);

        /* vardec has no definition anymore */
        AVIS_SSAASSIGN (IDS_AVIS (tmpsubst)) = NULL;

        tmpsubst = IDS_NEXT (tmpsubst);
        tmpwith = IDS_NEXT (tmpwith);
    }

    DBUG_RETURN (subst);
}

/******************************************************************************
 *
 * function:
 *   bool ForbiddenSubstitution(ids *chain)
 *
 * description:
 *   Checks if there is any ids in chain with vardec marked as ST_unique so
 *   we should better not substitute it, to avoid problems with global
 *   objects.
 *
 *****************************************************************************/
static bool
ForbiddenSubstitution (ids *chain)
{
    bool res;

    DBUG_ENTER ("ForbiddenSubstitution");
    res = FALSE;
    while ((chain != NULL) && (res == FALSE)) {
        res |= (VARDEC_OR_ARG_ATTRIB (AVIS_VARDECORARG (IDS_AVIS (chain))) == ST_unique);
        chain = IDS_NEXT (chain);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEPropagateSubst2Args(node *fun_args,
 *                                   node *ap_args,
 *                                   node *fundef)
 *
 * description:
 *   propagates substitution information into the called special fundef.
 *   this allows to continue the copy propagation in the called fundef
 *   and reduces the number of variables that have to be transfered
 *   into a special fundef as arg (inter-procedural copy propagation).
 *   this is possible for all args in cond-funs and loop invariant args
 *   of loop-funs.
 *
 * example:
 *   a = expr;                   int f_cond(int x, int y)
 *   b = a;                      {
 *   c = f_cond(a, b);              ...
 *   ...                            return (x + y);
 *                               }
 *
 * will be transformed to
 *   a = expr;                   int f_cond(int x, int y)
 *   b = a;                      {
 *   c = f_cond(a, a);              ...
 *   ...                            return (x + x);
 *                               }
 *
 *   unused code will be removed later by DeadCodeRemoval!
 *
 * implementation:
 *    set the AVIS_SUBST attribute for the args in the called fundef to
 *    matching identical arg in signature or NULL otherwise.
 *
 *****************************************************************************/
static node *
SSACSEPropagateSubst2Args (node *fun_args, node *ap_args, node *fundef)
{
    node *act_fun_arg;
    node *act_ap_arg;
    node *ext_ap_avis;
    node *search_fun_arg;
    node *search_ap_arg;
    bool found_match;

    DBUG_ENTER ("SSACSEPropagateSubst2Args");

    act_fun_arg = fun_args;
    act_ap_arg = ap_args;
    while (act_fun_arg != NULL) {
        /* process all arguments */
        DBUG_ASSERT ((act_ap_arg != NULL), "to few arguments in function application");

        /* init AVIS_SUBST attribute (means no substitution) */
        AVIS_SUBST (ARG_AVIS (act_fun_arg)) = NULL;

        /* get external identifier in function application (here we use it avis) */
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (act_ap_arg)) == N_id),
                     "non N_id node as arg in special function application");
        ext_ap_avis = ID_AVIS (EXPRS_EXPR (act_ap_arg));

        /*
         * now search the application args for identical id to substitute it.
         * this is possible only for args of cond-funs and loopinvariant args
         * of loop-funs.
         */
        if ((FUNDEF_IS_CONDFUN (fundef))
            || ((FUNDEF_IS_LOOPFUN (fundef))
                && (AVIS_SSALPINV (ARG_AVIS (act_fun_arg))))) {
            found_match = FALSE;

            search_fun_arg = fun_args;
            search_ap_arg = ap_args;
            while ((search_fun_arg != act_fun_arg) && (found_match == FALSE)) {
                /* compare identifiers via their avis pointers */
                DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (search_ap_arg)) == N_id),
                             "non N_id node as arg in special function application");
                if (ID_AVIS (EXPRS_EXPR (search_ap_arg)) == ext_ap_avis) {
                    /*
                     * if we find a matching identical id in application, we mark it
                     * for being substituted with the one we found and stop the
                     * further searching.
                     */
                    found_match = TRUE;
                    AVIS_SUBST (ARG_AVIS (act_fun_arg)) = ARG_AVIS (search_fun_arg);
                    DBUG_PRINT ("SSACSE", ("propagate copy propagation info for %s -> %s",
                                           VARDEC_OR_ARG_NAME (act_fun_arg),
                                           VARDEC_OR_ARG_NAME (search_fun_arg)));
                }

                /* parallel traversal to next arg in ap and fundef */
                search_fun_arg = ARG_NEXT (search_fun_arg);
                search_ap_arg = EXPRS_NEXT (search_ap_arg);
            }
        }

        /* parallel traversal to next arg in ap and fundef */
        act_fun_arg = ARG_NEXT (act_fun_arg);
        act_ap_arg = EXPRS_NEXT (act_ap_arg);
    }
    DBUG_ASSERT ((act_ap_arg == NULL), "to many arguments in function application");

    DBUG_RETURN (fun_args);
}

/******************************************************************************
 *
 * function:
 *   nodelist *SSACSEBuildSubstNodelist( node *return_exprs, node *fundef)
 *
 * description:
 *   analyse the return statement of a special fundef for results that
 *   are arguments of this fundef and therefore can directly be used in calling
 *   context. so we build up a nodelist with one element per result expression
 *   and store the avis node of the identifier of the corresponding arg in
 *   the function application or NULL.
 *
 *   this replacement can only be done for special loop-funs (because they are
 *   tail-end recursive only the else part of the conditional is important).
 *   if we have a cond-fun, we can only propagte back args in calling context
 *   if then and else part return the same args as result.
 *
 *   the built nodelist will be passed to the calling let node where the
 *   infered substitutions will be set in the corresponsing avis nodes.
 *
 *   ...
 *   x,y = f_do(a, b, c);         -->        x,y = f_do(a, b, c)
 *   z = x + y;                              z = a + b;
 *   ...                                     ...
 *
 *
 *   int, int f_do(int a, int b, int c)
 *   ...
 *   if (cond) {
 *      ... f_do(a, b, c_SSA_1)
 *   } else {
 *      ...
 *      a_SSA_3 = a;
 *      b_SSA_3 = b;
 *   }
 *   return(a_SSA_3, b_SSA_3, c_SSA_3);
 *
 *****************************************************************************/
static nodelist *
SSACSEBuildSubstNodelist (node *return_exprs, node *fundef)
{
    nodelist *nl;
    node *ext_avis;
    node *avis1;
    node *avis2;

    DBUG_ENTER ("SSACSEBuildSubstNodelist");

    DBUG_ASSERT ((FUNDEF_IS_LACFUN (fundef)),
                 "SSACSEBuildSubstNodelist called for non special fundef");

    if (return_exprs != NULL) {
        /* check for arg as result */
        avis1 = GetResultArgAvis (EXPRS_EXPR (return_exprs), THENPART);
        avis2 = GetResultArgAvis (EXPRS_EXPR (return_exprs), ELSEPART);

        if (((FUNDEF_IS_LOOPFUN (fundef)) && (avis2 != NULL)
             && (AVIS_SSALPINV (avis2) == TRUE))
            || ((FUNDEF_IS_CONDFUN (fundef)) && (avis1 == avis2) && (avis2 != NULL))) {
            /*
             * we get an arg that is result of this fundef,
             * now search for the external corresponding avis node in application
             */
            ext_avis = GetApAvisOfArgAvis (avis2, fundef);
        } else {
            ext_avis = NULL;
        }

        /*
         * add one nodelist node per result in bottom up traversal to get an
         * nodelist we can use in the bottom down travsersal of the corresponding
         * ids chain in the calling let node.
         */
        nl = MakeNodelistNode (ext_avis,
                               SSACSEBuildSubstNodelist (EXPRS_NEXT (return_exprs),
                                                         fundef));
    } else {
        /* no more return expression available */
        nl = NULL;
    }

    DBUG_RETURN (nl);
}

/******************************************************************************
 *
 * function:
 *   node *GetResultArgAvis(node *id, condpart cp)
 *
 * description:
 *   get the avis of a function argument, that is a result via a phi-copy
 *   assignment. condpart selects the phi definition in then or else part.
 *
 *   if id is no phi copy target or points to no arg, a NULL is returned.
 *
 *****************************************************************************/
static node *
GetResultArgAvis (node *id, condpart cp)
{
    node *result;
    node *defassign;

    DBUG_ENTER ("GetResultArgAvis");

    result = NULL;

    DBUG_ASSERT ((NODE_TYPE (id) == N_id), "GetResultArgAvis called for non id node");

    if (AVIS_SSAPHITARGET (ID_AVIS (id)) != PHIT_NONE) {
        /* get the selected definition of phi copy target id */
        if (cp == THENPART) {
            defassign = AVIS_SSAASSIGN (ID_AVIS (id));
        } else {
            defassign = AVIS_SSAASSIGN2 (ID_AVIS (id));
        }

        /* check if id is defined as arg */
        if ((NODE_TYPE (ASSIGN_RHS (defassign)) == N_id)
            && (NODE_TYPE (AVIS_VARDECORARG (ID_AVIS (ASSIGN_RHS (defassign))))
                == N_arg)) {
            result = ID_AVIS (ASSIGN_RHS (defassign));
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *GetApAvisOfArgAvis(node *arg_avis, node *fundef)
 *
 * description:
 *   search for the matching external ap_arg avis to the given fundef arg avis.
 *   we do a parallel traversal of ap exprs chain and the fundef arg chain.
 *
 *****************************************************************************/
static node *
GetApAvisOfArgAvis (node *arg_avis, node *fundef)
{
    node *ap_avis;
    node *exprs_chain;
    node *arg_chain;
    bool cont;

    DBUG_ENTER ("GetApAvisOfArgAvis");

    DBUG_ASSERT ((FUNDEF_IS_LACFUN (fundef)),
                 "GetApAvisOfArgAvis called for non special fundef");

    arg_chain = FUNDEF_ARGS (fundef);

    DBUG_ASSERT ((NODELIST_NEXT (FUNDEF_EXT_ASSIGNS (fundef)) == NULL),
                 "GetApAvisOfArgAvis cannot handle multiple usages "
                 "of special fundefs");

    exprs_chain = AP_ARGS (ASSIGN_RHS (NODELIST_NODE (FUNDEF_EXT_ASSIGNS (fundef))));

    ap_avis = NULL;
    cont = TRUE;
    while ((arg_chain != NULL) && cont) {
        DBUG_ASSERT ((exprs_chain != NULL), "mismatch between ap args and fun args");

        if (ARG_AVIS (arg_chain) == arg_avis) {
            /* we found the matching one */
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs_chain)) == N_id),
                         "non id node in special fundef application");

            ap_avis = ID_AVIS (EXPRS_EXPR (exprs_chain));

            /* stop further searching */
            cont = FALSE;
        }

        arg_chain = ARG_NEXT (arg_chain);
        exprs_chain = EXPRS_NEXT (exprs_chain);
    }

    DBUG_RETURN (ap_avis);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses args for initialization.
 *   traverses block.
 *   does NOT traverse to next fundef (this must by done by the optimization
 *   loop.
 *
 *****************************************************************************/
node *
SSACSEfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEfundef");

    INFO_SSACSE_FUNDEF (arg_info) = arg_node;
    INFO_SSACSE_RESULTARG (arg_info) = NULL;

    if ((FUNDEF_ARGS (arg_node) != NULL) && (!(FUNDEF_IS_LACFUN (arg_node)))) {
        /*
         * traverse args of fundef to init the AVIS_SUBST attribute. this is done
         * here only for normal fundefs. in special fundefs that are traversed via
         * SSACSEap() we do a substitution information propagation (see
         * SSACSEPropagateSubst2Args() ) that sets the correct AVIS_SUBST
         * attributes for all args.
         */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEarg(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses chain of args to init SUBST attribute with NULL.
 *
 *****************************************************************************/
node *
SSACSEarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEarg");

    AVIS_SUBST (ARG_AVIS (arg_node)) = NULL;

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses chain of vardecs for initialization.
 *   starts new cse frame
 *   traverses assignment chain.
 *   remove top cse frame (and so all available expressions defined in this
 *   block)
 *
 *****************************************************************************/
node *
SSACSEblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* traverse vardecs of block */
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    /* start new cse frame */
    INFO_SSACSE_CSE (arg_info) = CreateNewCSElayer (INFO_SSACSE_CSE (arg_info));

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* traverse assignments of block */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    /* remove top cse frame */
    INFO_SSACSE_CSE (arg_info) = RemoveTopCSElayer (INFO_SSACSE_CSE (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse chain of vardecs to init avis subst attribute.
 *
 *
 *****************************************************************************/
node *
SSACSEvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEvardec");

    AVIS_SUBST (VARDEC_AVIS (arg_node)) = NULL;

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEassign (node *arg_node, node *arg_info)
 *
 * description:
 *   traverses assignment chain top-down and removes unused assignment.
 *
 *****************************************************************************/
node *
SSACSEassign (node *arg_node, node *arg_info)
{
    node *old_assign;
    bool remassign;
    node *tmp;

    DBUG_ENTER ("SSACSEassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "assign node without instruction");

    old_assign = INFO_SSACSE_ASSIGN (arg_info);
    INFO_SSACSE_ASSIGN (arg_info) = arg_node;
    INFO_SSACSE_REMASSIGN (arg_info) = FALSE;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    remassign = INFO_SSACSE_REMASSIGN (arg_info);
    INFO_SSACSE_ASSIGN (arg_info) = old_assign;

    /* traverse to next assignment in chain */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* in bottom-up traversal free this assignment if it was marked as unused */
    if (remassign) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEcond(node *arg_node, node *arg_info)
 *
 * description:
 *   the stacking of available cse-expressions for both parts of the
 *   conditional is done by SSACSEblock.
 *   traverses condition, then- and else-part (in this order).
 *
 *****************************************************************************/
node *
SSACSEcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEcond");

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "conditional without condition");
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

    INFO_SSACSE_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEreturn(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses the result expressions and starts analysis of return values
 *  in case of special fundefs.
 *
 *****************************************************************************/
node *
SSACSEreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    /*
     * analyse results and build up RESULTARG nodelist for results
     * that are args and therefore can be used directly in the calling context
     */
    if (FUNDEF_IS_LACFUN (INFO_SSACSE_FUNDEF (arg_info))) {
        INFO_SSACSE_RESULTARG (arg_info)
          = SSACSEBuildSubstNodelist (RETURN_EXPRS (arg_node),
                                      INFO_SSACSE_FUNDEF (arg_info));
    } else {
        INFO_SSACSE_RESULTARG (arg_info) = NULL;
    }

    INFO_SSACSE_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSElet(node *arg_node, node *arg_info)
 *
 * description:
 *   first do a variable substitution on the right side expression (but
 *   only for simple expressions) or do SSACSE for a right-side with-loop
 *   by traversing the RHS.
 *
 *   next compare the right side with all available common subexpressions that
 *   are stored in the current cse list stack.
 *
 *   in a matching case set the necessary SUBST links to the existing variables
 *   and remove this let.
 *
 *   if right side is a simple identifier (so the let is a copy assignment) the
 *   left side identifier is substiuted be the right side identifier.
 *
 *****************************************************************************/
node *
SSACSElet (node *arg_node, node *arg_info)
{
    node *match;

    DBUG_ENTER ("SSACSElet");

    DBUG_PRINT ("SSACSE", ("inspecting expression for cse"));

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");
    DBUG_ASSERT ((LET_IDS (arg_node) != NULL), "let without ids");

    /*
     * traverse right side expression to do variable substitutions
     * or CSE in with-loops/special fundefs
     */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    match = FindCSE (INFO_SSACSE_CSE (arg_info), arg_node);

    /*
     * found matching common subexpression or copy assignment
     * if let is NO phicopytarget
     * and let is NO assignment to some UNIQUE variable
     * do the necessary substitution.
     */
    if ((match != NULL)
        && (AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) == PHIT_NONE)
        && (ForbiddenSubstitution (LET_IDS (arg_node)) == FALSE)) {
        /* set subst attributes for results */
        LET_IDS (arg_node) = SetSubstAttributes (LET_IDS (arg_node), LET_IDS (match));

        DBUG_PRINT ("SSACSE",
                    ("Common subexpression eliminated in line %d", NODE_LINE (arg_node)));
        cse_expr++;

        /* remove assignment */
        INFO_SSACSE_REMASSIGN (arg_info) = TRUE;

    } else if ((NODE_TYPE (LET_EXPR (arg_node)) == N_id)
               && (AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) == PHIT_NONE)
               && (ForbiddenSubstitution (LET_IDS (arg_node)) == FALSE)) {
        /* set subst attributes for results */
        LET_IDS (arg_node)
          = SetSubstAttributes (LET_IDS (arg_node), ID_IDS (LET_EXPR (arg_node)));

        DBUG_PRINT ("SSACSE",
                    ("copy assignment eliminated in line %d", NODE_LINE (arg_node)));
        cse_expr++;

        /* remove copy assignment */
        INFO_SSACSE_REMASSIGN (arg_info) = TRUE;

    } else if ((NODE_TYPE (LET_EXPR (arg_node)) == N_ap)
               && (FUNDEF_IS_LACFUN (AP_FUNDEF (LET_EXPR (arg_node))))) {
        /*
         * traverse the result ids to set the infered subst information stored
         * in INFO_SSACSE_RESULTARG nodelist
         */
        LET_IDS (arg_node) = TravIDS (LET_IDS (arg_node), arg_info);

    } else {
        /* new expression found */
        DBUG_PRINT ("SSACSE", ("add new expression to cselist"));
        INFO_SSACSE_CSE (arg_info) = AddCSEinfo (INFO_SSACSE_CSE (arg_info), arg_node);

        /* do not remove assignment */
        INFO_SSACSE_REMASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEap(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses parameter to do correct variable substitution.
 *   if special function application, start traversal of this fundef
 *
 *****************************************************************************/
node *
SSACSEap (node *arg_node, node *arg_info)
{
    node *new_arg_info;
    DBUG_ENTER ("SSACSEap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    /*
     * when traversing the recursive ap of a special loop fundef,
     * we set RECFUNAP flag to TRUE, to avoid renamings of loop
     * independend args that could be replaced otherwise.
     */
    if ((FUNDEF_IS_LOOPFUN (INFO_SSACSE_FUNDEF (arg_info)))
        && (AP_FUNDEF (arg_node) == INFO_SSACSE_FUNDEF (arg_info))) {
        INFO_SSACSE_RECFUNAP (arg_info) = TRUE;
    } else {
        INFO_SSACSE_RECFUNAP (arg_info) = FALSE;
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* reset traversal flag */
    INFO_SSACSE_RECFUNAP (arg_info) = FALSE;

    /* traverse special fundef without recursion */
    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_SSACSE_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSACSE", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        INFO_SSACSE_MODUL (arg_info)
          = CheckAndDupSpecialFundef (INFO_SSACSE_MODUL (arg_info), AP_FUNDEF (arg_node),
                                      INFO_SSACSE_ASSIGN (arg_info));

        DBUG_ASSERT ((FUNDEF_USED (AP_FUNDEF (arg_node)) == 1),
                     "more than one instance of special function used.");

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start with empty cse stack */
        INFO_SSACSE_CSE (new_arg_info) = NULL;
        INFO_SSACSE_MODUL (new_arg_info) = INFO_SSACSE_MODUL (arg_info);

        /* propagate id substitutions into called fundef */
        FUNDEF_ARGS (AP_FUNDEF (arg_node))
          = SSACSEPropagateSubst2Args (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                       AP_ARGS (arg_node), AP_FUNDEF (arg_node));

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSACSE", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* save RESULTARG nodelist for processing in surrounding let */
        INFO_SSACSE_RESULTARG (arg_info) = INFO_SSACSE_RESULTARG (new_arg_info);

        new_arg_info = FreeTree (new_arg_info);

    } else {
        DBUG_PRINT ("SSACSE", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEid(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse ids data to do substitution.
 *
 *****************************************************************************/
node *
SSACSEid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEid");

    ID_IDS (arg_node) = TravIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSENwith(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse NPart, Nwithop and NCode in this order
 *
 *
 *****************************************************************************/
node *
SSACSENwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSENwith");

    /* traverse and do variable substitution in partitions */
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    /* traverse and do variable substitution in withops */
    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    /* traverse and do cse in code blocks */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSENcode(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse codeblock and expression for each Ncode node
 *
 *
 *****************************************************************************/
node *
SSACSENcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSENcode");

    /* traverse codeblock */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /*traverse expression to do variable substitution */
    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    /* traverse to next node */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static ids *SSACSEids(ids *arg_ids, node *arg_info)
 *
 * description:
 *   traverses chain of ids to do variable substitution as
 *   annotated in AVIS_SUBST attribute.
 *   but do not substitute loop invariant args in a recursive funap (when
 *   INFO_SSACSE_RECFUNAP() == TRUE),
 *
 *   if we have some nodelist stored in INFO_SSACSE_RESULTARG annotate the
 *   stored subst avis information after processing, so futher uses will be
 *   renamed according to this information.
 *
 *****************************************************************************/
static ids *
SSACSEids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSACSEids");

    DBUG_ASSERT ((IDS_AVIS (arg_ids) != NULL), "missing Avis backlink in ids");

    /* check for necessary substitution */
    if ((AVIS_SUBST (IDS_AVIS (arg_ids)) != NULL)
        && (!((INFO_SSACSE_RECFUNAP (arg_info) == TRUE)
              && (AVIS_SSALPINV (IDS_AVIS (arg_ids)) == TRUE)))) {
        DBUG_PRINT ("SSACSE", ("substitution: %s -> %s",
                               VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids))),
                               VARDEC_OR_ARG_NAME (
                                 AVIS_VARDECORARG (AVIS_SUBST (IDS_AVIS (arg_ids))))));

        /* do renaming to new ssa vardec */
        IDS_AVIS (arg_ids) = AVIS_SUBST (IDS_AVIS (arg_ids));

        /* restore all depended attributes with correct values */
        IDS_VARDEC (arg_ids) = AVIS_VARDECORARG (IDS_AVIS (arg_ids));

#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        IDS_NAME (arg_ids) = Free (IDS_NAME (arg_ids));
        IDS_NAME (arg_ids) = StringCopy (VARDEC_OR_ARG_NAME (IDS_VARDEC (arg_ids)));
#endif
    }

    /* process stored INFO_SSACSE_RESULTARG information */
    if (INFO_SSACSE_RESULTARG (arg_info) != NULL) {
        DBUG_ASSERT ((AVIS_SUBST (IDS_AVIS (arg_ids)) == NULL),
                     "there must not exist any subst setting for "
                     "a fresh defined vardec");

        /* set AVIS_SUBST corresponding avis infered by */
        AVIS_SUBST (IDS_AVIS (arg_ids))
          = NODELIST_NODE (INFO_SSACSE_RESULTARG (arg_info));

        /* remove info of this result */
        INFO_SSACSE_RESULTARG (arg_info)
          = FreeNodelistNode (INFO_SSACSE_RESULTARG (arg_info));

#ifndef DBUG_OFF
        if (AVIS_SUBST (IDS_AVIS (arg_ids)) != NULL) {
            DBUG_PRINT ("SSACSE",
                        ("bypassing result %s",
                         VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids)))));
        }
#endif
    }

    /* traverse to next ids in chain */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   implements a simple TravIDS function like Trav for nodes to have
 *   an similar implementation.
 *
 *****************************************************************************/
static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSACSEids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* SSACSE(node* fundef, node* modul)
 *
 * description:
 *   Starts the traversal for a given fundef.
 *   Starting fundef must not be a special fundef (do, while, cond) created by
 *   lac2fun transformation. These "inline" functions will be traversed in
 *   their order of usage.
 *
 *
 *****************************************************************************/
node *
SSACSE (node *fundef, node *modul)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSACSE");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "SSACSE called for non-fundef node");

    DBUG_PRINT ("OPT", ("starting common subexpression elimination (ssa) in function %s",
                        FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if (!(FUNDEF_IS_LACFUN (fundef))) {
        arg_info = MakeInfo ();

        INFO_SSACSE_CSE (arg_info) = NULL;
        INFO_SSACSE_MODUL (arg_info) = modul;

        old_tab = act_tab;
        act_tab = ssacse_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (fundef);
}
