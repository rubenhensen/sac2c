/** <!--********************************************************************-->
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
 *   current context (needed for conditionals and with-loops). For each
 *   expression we do a tree-compare with the available CSE we stored before.
 *   In addition to the tree-compare we have to check the types of the
 *   assigned LHS to be equal to avoid wrong substitutions of constant
 *   RHS arrays (e.g. [1,2,3,4] used as [[1,2],[3,4]]).
 *   If we find a matching one, we substitute the LHS otherwise we add this
 *   new expression to the list of available expressions.
 *
 *
 *   It also does a copy propagation for assignments like
 *       a = f(x);                   a = f(x)l
 *       b = a;              -->     <removed>
 *       c = b;                      <removed>
 *       d = g(c);                   d = g(a);
 *
 *   These optimizations do not happen for identifiers of global objects or
 *   other unique types, because the current used implementation of objects
 *   does not use a global word object for creation, so we hit the following
 *   problem:
 *       obj1  = CreateNewObj();        obj1  = CreateNewObj();
 *       obj1' = modify(1, obj1);  -->  obj1' = modify(obj1);
 *       obj2  = CreateNewObj();        <removed>
 *       obj2' = modify(2, obj2);       obj2' = modify(obj1);
 *                                                    ******
 *   This is wrong code and no more unique (two accesses to obj1)!!!
 *   if this issue is ressolved you can disable this handling by
 *   defining CREATE_UNIQUE_BY_HEAP
 *
 *   The copy propagation information is propagated into special fundefs for
 *   the called args (all for cond-funs, loop invariant ones for loop-funs).
 *   for details see SSACSEPropagateSubst2Args() in this file.
 *
 *   If we find results of special fundes that are arguments of the fundef,
 *   we try to avoid a passing of this arg through the special fundef. Instead
 *   we use the correct result id directly in the calling context. For details
 *   see SSACSEBuildSubstNodelist().
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "globals.h"
#include "new_types.h"

#define DBUG_PREFIX "CSE"
#include "debug.h"

#include "traverse.h"
#include "free.h"
#include "convert.h"
#include "DupTree.h"
#include "SSACSE.h"
#include "compare_tree.h"
#include "ctinfo.h"
#include "map_fun_trav.h"

/** <!--********************************************************************-->
 *
 * @name CSEINFO structure
 * @{
 *
 *****************************************************************************/
typedef struct CSEINFO {
    struct CSEINFO *nextlayer;
    size_t entries;
    int current;
    node **lets;
} cseinfo;

static cseinfo *
AddCseInfoLayer (cseinfo *nextlayer, size_t entries)
{
    cseinfo *res;

    DBUG_ENTER ();

    res = (cseinfo *)MEMmalloc (sizeof (cseinfo));

    res->nextlayer = nextlayer;
    res->entries = entries;
    res->current = 0;
    res->lets = (node **)MEMmalloc (entries * sizeof (node *));

    DBUG_RETURN (res);
}

static cseinfo *
RemoveTopCseLayer (cseinfo *top)
{
    DBUG_ENTER ();

    cseinfo *res = top->nextlayer;

    top->lets = MEMfree (top->lets);
    top = MEMfree (top);

    DBUG_RETURN (res);
}

static cseinfo *
AddLet (cseinfo *layer, node *let)
{
    DBUG_ENTER ();

    layer->lets[layer->current] = let;
    layer->current += 1;

    DBUG_RETURN (layer);
}

static node *
FindCSE (cseinfo *layer, node *let)
{
    int i;
    int stop = layer->current;
    node *arhs = LET_EXPR (let);
    node *res = NULL;

    DBUG_ENTER ();

    for (i = 0; i < stop; i++) {
        node *brhs = LET_EXPR (layer->lets[i]);
        nodetype ant = NODE_TYPE (arhs);
        nodetype bnt = NODE_TYPE (brhs);
        /**
         *   excluded CSE on F_type_error here as that caused bug 416!
         */
        if ((ant == bnt)
            && (((ant == N_prf) && (PRF_PRF (arhs) == PRF_PRF (brhs))
                 && (PRF_PRF (arhs) != F_type_error))
                || (ant != N_prf))) {
            if (CMPTdoCompareTree (arhs, brhs) == CMPT_EQ) {
                res = layer->lets[i];
                break;
            }
        }
    }

    if ((res == NULL) && (layer->nextlayer != NULL)) {
        res = FindCSE (layer->nextlayer, let);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @}  <!-- CSE structure -->
 *****************************************************************************/

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *ext_assign;
    cseinfo *cse;
    node *assign;
    node *withid;
    bool recfunap;
    nodelist *resultarg;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_EXT_ASSIGN(n) ((n)->ext_assign)
#define INFO_CSE(n) ((n)->cse)
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_RECFUNAP(n) ((n)->recfunap)
#define INFO_RESULTARG(n) ((n)->resultarg)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_EXT_ASSIGN (result) = NULL;
    INFO_CSE (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_WITHID (result) = NULL;
    INFO_RECFUNAP (result) = FALSE;
    INFO_RESULTARG (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

typedef enum { THENPART, ELSEPART } condpart;

/* helper functions for internal use only */
static node *SetSubstAttributes (node *subst, node *with);

static node *PropagateSubst2Args (node *fun_args, node *ap_args, node *fundef);
static node *PropagateIdenticalReturn2Results (node *ap_fundef, node *ids_chain);
static node *PropagateLoopInvariantArgs (node *ids, nodelist **nodes);
static nodelist *BuildSubstNodelist (node *return_exprs, node *fundef, node *ext_assign);
static node *GetResultArgAvis (node *id, condpart cp);
static node *GetApAvisOfArgAvis (node *arg_avis, node *fundef, node *ext_assign);

/******************************************************************************
 *
 * function:
 *   node *SetSubstAttributes(node *subst, node *with)
 *
 * description:
 *   set AVIS_SUBST attribute for all ids to the corresponding avis node from
 *   the with-ids chain (that we get from the stored available let expression).
 *
 ******************************************************************************/
static node *
SetSubstAttributes (node *subst, node *with)
{
    node *tmpsubst;
    node *tmpwith;

    DBUG_ENTER ();

    tmpsubst = subst;
    tmpwith = with;

    while (tmpsubst != NULL) {
        DBUG_PRINT ("substitute ids %s with %s", (IDS_NAME (tmpsubst)),
                    (IDS_NAME (tmpwith)));

        AVIS_SUBST (IDS_AVIS (tmpsubst)) = IDS_AVIS (tmpwith);

        tmpsubst = IDS_NEXT (tmpsubst);
        tmpwith = IDS_NEXT (tmpwith);
    }

    DBUG_RETURN (subst);
}

/******************************************************************************
 *
 * function:
 *   node *PropagateSubst2Args( node *fun_args,
 *                              node *ap_args,
 *                              node *fundef)
 *
 * description:
 *   Propagates substitution information into the called special fundef.
 *   This allows to continue the copy propagation in the called fundef
 *   and reduces the number of variables that have to be transfered
 *   into a special fundef as arg (inter-procedural copy propagation).
 *   This is possible for all args in cond-funs and loop invariant args
 *   of loop-funs.
 *   Moreover, the types of the formal arguments are specialized if possible.
 *
 *   example:
 *       a = 1;                       int f_cond( int[*] x, int[*] y)
 *       b = a;                       { ...
 *       c = f_cond( a, b);             return( x + y);
 *       ...                          }
 *
 *     will be transformed to
 *
 *       a = 1;                       int f_cond( int[] x, int[] y)
 *       b = a;                       { ...
 *       c = f_cond( a, a);             return( x + x);
 *       ...                          }
 *
 *     Unused code will be removed later by DeadCodeRemoval!
 *
 *     Note, that it is (in general) *not* recommended to complain about type
 *     errors in 'f_cond', because 'f_cond' might contain (dead) code in which
 *     'x' or 'y' are not scalars:
 *
 *       if (dim( x) == 1) {
 *         ... fun_for_vectors_only( x) ...
 *       }
 *
 * implementation:
 *   set the AVIS_SUBST attribute for the args in the called fundef to
 *   matching identical arg in signature or NULL otherwise.
 *
 *****************************************************************************/
static node *
PropagateSubst2Args (node *fun_args, node *ap_args, node *fundef)
{
    node *act_fun_arg;
    node *act_ap_arg;
    node *ext_ap_avis;
    ntype *ext_ap_type;
    node *search_fun_arg;
    node *search_ap_arg;
    bool found_match;
    ct_res cmp;
    char *stype1, *stype2;

    DBUG_ENTER ();

    act_fun_arg = fun_args;
    act_ap_arg = ap_args;
    while (act_fun_arg != NULL) {
        /* process all arguments */
        DBUG_ASSERT (act_ap_arg != NULL, "too few arguments in function application");

        /* init AVIS_SUBST attribute (means no substitution) */
        AVIS_SUBST (ARG_AVIS (act_fun_arg)) = NULL;

        /* get external identifier in function application (here we use it avis) */
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (act_ap_arg)) == N_id,
                     "non N_id node as arg in special function application");
        ext_ap_avis = ID_AVIS (EXPRS_EXPR (act_ap_arg));
        ext_ap_type = AVIS_TYPE (ext_ap_avis);
        /*
         * specialize types of formal parameters if possible
         */
        cmp = TYcmpTypes (AVIS_TYPE (ARG_AVIS (act_fun_arg)), ext_ap_type);

        stype1 = TYtype2String (AVIS_TYPE (ARG_AVIS (act_fun_arg)), TRUE, 0);
        stype2 = TYtype2String (ext_ap_type, TRUE, 0);
        if (cmp == TY_gt) {

            /*
             * actual type is subtype of formal type -> specialize
             */
            if ((FUNDEF_ISCONDFUN (fundef))
                || ((FUNDEF_ISLOOPFUN (fundef))
                    && (AVIS_SSALPINV (ARG_AVIS (act_fun_arg))))) {

                DBUG_PRINT ("type of formal LaC-fun (%s) arg specialized in line %zu:"
                            "  %s:%s->%s",
                            CTIitemName (fundef), NODE_LINE (act_fun_arg),
                            ARG_NAME (act_fun_arg), stype1, stype2);

                AVIS_TYPE (ARG_AVIS (act_fun_arg))
                  = TYfreeType (AVIS_TYPE (ARG_AVIS (act_fun_arg)));
                AVIS_TYPE (ARG_AVIS (act_fun_arg)) = TYcopyType (ext_ap_type);
            }
        } else if ((cmp == TY_dis) || (cmp == TY_hcs)) {
            /*
             * special function application with incompatible types found
             */
            DBUG_PRINT ("application of LaC-function %s with incompatible types"
                        " in line %zu:  %s:%s->%s",
                        FUNDEF_NAME (fundef), NODE_LINE (act_fun_arg),
                        ARG_NAME (act_fun_arg), stype1, stype2);
        }
        stype1 = MEMfree (stype1);
        stype2 = MEMfree (stype2);

        /*
         * now search the application args for identical id to substitute it.
         * this is possible only for args of cond-funs and loopinvariant args
         * of loop-funs.
         */
        if ((FUNDEF_ISCONDFUN (fundef))
            || ((FUNDEF_ISLOOPFUN (fundef))
                && (AVIS_SSALPINV (ARG_AVIS (act_fun_arg))))) {
            found_match = FALSE;

            search_fun_arg = fun_args;
            search_ap_arg = ap_args;
            while ((search_fun_arg != act_fun_arg) && (!found_match)) {
                /* compare identifiers via their avis pointers */
                DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (search_ap_arg)) == N_id,
                             "non N_id node as arg in special function application");
                if ((ID_AVIS (EXPRS_EXPR (search_ap_arg)) == ext_ap_avis)
                    && (AVIS_SSALPINV (ARG_AVIS (search_fun_arg)))) {
                    /*
                     * if we find a matching identical loop invariant id
                     * in application, we mark it for being substituted
                     * with the one we found and stop the further searching.
                     */
                    found_match = TRUE;
                    AVIS_SUBST (ARG_AVIS (act_fun_arg)) = ARG_AVIS (search_fun_arg);
                    DBUG_PRINT ("propagate copy propagation info for %s -> %s",
                                VARDEC_OR_ARG_NAME (act_fun_arg),
                                VARDEC_OR_ARG_NAME (search_fun_arg));
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
    DBUG_ASSERT (act_ap_arg == NULL, "too many arguments in function application");

    DBUG_RETURN (fun_args);
}

/******************************************************************************
 *
 * function:
 *   nodelist *BuildSubstNodelist( node *return_exprs, node *fundef, node *ext_assign)
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
BuildSubstNodelist (node *return_exprs, node *fundef, node *ext_assign)
{
    nodelist *nl;
    node *ext_avis;
    node *avis1;
    node *avis2;

    DBUG_ENTER ();

    DBUG_ASSERT (FUNDEF_ISLACFUN (fundef), "Expected LACFUN, but got %s",
                 FUNDEF_NAME (fundef));

    if (return_exprs != NULL) {
        /* check for arg as result */
        avis1 = GetResultArgAvis (EXPRS_EXPR (return_exprs), THENPART);
        avis2 = GetResultArgAvis (EXPRS_EXPR (return_exprs), ELSEPART);

        if (((FUNDEF_ISLOOPFUN (fundef)) && (avis2 != NULL) && (AVIS_SSALPINV (avis2)))
            || ((FUNDEF_ISCONDFUN (fundef)) && (avis1 == avis2) && (avis2 != NULL))) {
            /*
             * we get an arg that is result of this fundef,
             * now search for the external corresponding avis node in application
             */
            ext_avis = GetApAvisOfArgAvis (avis2, fundef, ext_assign);
        } else {
            ext_avis = NULL;
        }

        /*
         * add one nodelist node per result in bottom up traversal to get an
         * nodelist we can use in the bottom down travsersal of the corresponding
         * ids chain in the calling let node.
         */
        nl = TBmakeNodelistNode (ext_avis, BuildSubstNodelist (EXPRS_NEXT (return_exprs),
                                                               fundef, ext_assign));
    } else {
        /* no more return expression available */
        nl = NULL;
    }

    DBUG_RETURN (nl);
}

/******************************************************************************
 *
 * function:
 *   node *PropagateIdenticalReturn2Results(node *ap_fundef, node *ids_chain)
 *
 * description:
 *   searches in the return statement of loops (here the then-part phi-copy-
 *   targets) for identical result expressions and propagates this
 *   information into the result ids_chain by setting the AVIS_SUBST attribute
 *   for later substitutions.
 *
 *****************************************************************************/
static node *
PropagateIdenticalReturn2Results (node *ap_fundef, node *ids_chain)
{
    node *act_result;
    node *act_exprs;
    node *search_result;
    node *search_exprs;

    bool found_match;

    DBUG_ENTER ();

    /* process all identifier of result chain of a loop special fundef */
    act_result = ids_chain;
    act_exprs = RETURN_EXPRS (FUNDEF_RETURN (ap_fundef));

    while ((FUNDEF_ISLOOPFUN (ap_fundef)) && (act_result != NULL)) {
        /* search the the processed results for identical results */
        search_result = ids_chain;
        search_exprs = RETURN_EXPRS (FUNDEF_RETURN (ap_fundef));
        found_match = FALSE;

        while ((search_result != act_result) && (!found_match)) {
            DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (act_exprs)) == N_id,
                         "non id node in return of special fundef (act)");
            DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (search_exprs)) == N_id,
                         "non id node in return of special fundef (search)");

            /*
             * check if both definitions in the else-part are pointing to the
             * same identifier (here compared via the avis node) - if there is
             * already an subst entry (set by the arg-result bypassing) we do
             * not need to set the AVIS_SUBST attribute again.
             */

            if ((AVIS_SUBST (IDS_AVIS (act_result)) == NULL)
                && (NODE_TYPE (FUNCOND_ELSE (
                      ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (act_exprs))))))
                    == N_id)
                && (NODE_TYPE (FUNCOND_ELSE (
                      ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (search_exprs))))))
                    == N_id)
                && (ID_AVIS (FUNCOND_ELSE (
                      ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (act_exprs))))))
                    == ID_AVIS (FUNCOND_ELSE (ASSIGN_RHS (
                         AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (search_exprs)))))))) {
                /* stop further searching */
                found_match = TRUE;
                AVIS_SUBST (IDS_AVIS (act_result)) = IDS_AVIS (search_result);
            }

            /* do a parallel traversal */
            search_result = IDS_NEXT (search_result);
            search_exprs = EXPRS_NEXT (search_exprs);
        }

        /* do a parallel traversal in ids chain and exprs chain */
        act_result = IDS_NEXT (act_result);
        act_exprs = EXPRS_NEXT (act_exprs);
    }

    DBUG_RETURN (ids_chain);
}

/******************************************************************************
 *
 * function:
 *   ids *PropagateLoopInvariantArgs( ids *ids, nodelist **nodes)
 *
 * description:
 *
 *   Propagates the information that a result of a loop is identical to one
 *   of its arguments into the results of the outer application to propagate
 *   this information further.
 *
 *****************************************************************************/

node *
PropagateLoopInvariantArgs (node *ids, nodelist **nodes)
{
    DBUG_ENTER ();

    /* process stored INFO_RESULTARG information if any */
    if ((ids != NULL) && (*nodes != NULL)) {
        DBUG_ASSERT (IDS_AVIS (ids) != NULL, "missing Avis backlink in ids");

        DBUG_PRINT ("Looking at %s", AVIS_NAME (IDS_AVIS (ids)));

        DBUG_ASSERT (AVIS_SUBST (IDS_AVIS (ids)) == NULL,
                     "there must not exist any subst setting for"
                     " a freshly defined vardec");

        /* set AVIS_SUBST corresponding avis infered by */
        AVIS_SUBST (IDS_AVIS (ids)) = NODELIST_NODE ((*nodes));

        /* remove info of this result */
        *nodes = FREEfreeNodelistNode (*nodes);

#ifndef DBUG_OFF
        if (AVIS_SUBST (IDS_AVIS (ids)) != NULL) {
            DBUG_PRINT ("bypassing result %s", AVIS_NAME (IDS_AVIS (ids)));
        }
#endif

        /* continue with next ids in chain */
        IDS_NEXT (ids) = PropagateLoopInvariantArgs (IDS_NEXT (ids), nodes);
    }

    DBUG_RETURN (ids);
}

/******************************************************************************
 *
 * function:
 *   node *GetResultArgAvis(node *id, condpart cp)
 *
 * description:
 *   get the avis of a function argument, that is a result via a phi function
 *   assignment. condpart selects the phi definition in then or else part.
 *
 *   if id is no phi function or points to no arg, a NULL is returned.
 *
 *****************************************************************************/
static node *
GetResultArgAvis (node *id, condpart cp)
{
    node *result;
    node *defassign;

    DBUG_ENTER ();

    result = NULL;

    DBUG_ASSERT (NODE_TYPE (id) == N_id, "GetResultArgAvis called for non id node");

    if (TCisPhiFun (id)) {

        defassign = AVIS_SSAASSIGN (ID_AVIS (id));

        defassign = (ASSIGN_RHS (defassign));

        if (cp == THENPART) {
            defassign = FUNCOND_THEN (defassign);
        } else {
            defassign = FUNCOND_ELSE (defassign);
        }

        /* check if id is defined as arg */
        if ((NODE_TYPE (defassign) == N_id)
            && (NODE_TYPE (AVIS_DECL (ID_AVIS (defassign))) == N_arg)) {
            result = ID_AVIS (defassign);
        }
    }
    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *GetApAvisOfArgAvis(node *arg_avis, node *fundef, node *ext_assign)
 *
 * description:
 *   search for the matching external ap_arg avis to the given fundef arg avis.
 *   we do a parallel traversal of ap exprs chain and the fundef arg chain.
 *
 *****************************************************************************/
static node *
GetApAvisOfArgAvis (node *arg_avis, node *fundef, node *ext_assign)
{
    node *ap_avis;
    node *exprs_chain;
    node *arg_chain;
    bool cont;

    DBUG_ENTER ();

    DBUG_ASSERT (FUNDEF_ISLACFUN (fundef),
                 "GetApAvisOfArgAvis called for non special fundef");

    arg_chain = FUNDEF_ARGS (fundef);

    exprs_chain = AP_ARGS (ASSIGN_RHS (ext_assign));

    ap_avis = NULL;
    cont = TRUE;
    while ((arg_chain != NULL) && cont) {
        DBUG_ASSERT (exprs_chain != NULL, "mismatch between ap args and fun args");

        if (ARG_AVIS (arg_chain) == arg_avis) {
            /* we found the matching one */
            DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (exprs_chain)) == N_id,
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
 *   node *CSEmodule(node *arg_node, info *arg_info)
 *
 * description:
 *   prunes the syntax tree by only going into function defintions
 *
 *****************************************************************************/

node *
CSEmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses args for initialization.
 *   traverses block.
 *   does NOT traverse to next fundef (this must by done by the optimization
 *   loop.
 *
 *****************************************************************************/

node *
CSEfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    INFO_RESULTARG (arg_info) = NULL;

    DBUG_PRINT ("Begin traversing %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    if ((INFO_EXT_ASSIGN (arg_info) != NULL)
        || ((!FUNDEF_ISLACFUN (arg_node)) && (FUNDEF_BODY (arg_node) != NULL))) {
        if (INFO_EXT_ASSIGN (arg_info) == NULL) {
            /*
             * traverse args of fundef to init the AVIS_SUBST attribute. this is done
             * here only for normal fundefs. in special fundefs that are traversed via
             * CSEap() we do a substitution information propagation (see
             * CSEPropagateSubst2Args() ) that sets the correct AVIS_SUBST
             * attributes for all args.
             */
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        }

        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /*
         * Reset AVIS_SUBST
         */
        node *n = FUNDEF_ARGS (arg_node);
        while (n != NULL) {
            AVIS_SUBST (ARG_AVIS (n)) = NULL;
            n = ARG_NEXT (n);
        }
    }

    DBUG_PRINT ("Done traversing %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    if (INFO_EXT_ASSIGN (arg_info) == NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEavis( node *arg_node, info *arg_info)
 *
 * description:
 *   traverses avis to init SUBST attribute with NULL.
 *
 *****************************************************************************/
node *
CSEavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Resetting SUBST field of avis for %s", AVIS_NAME (arg_node));

    AVIS_SUBST (arg_node) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEblock( node *arg_node, info *arg_info)
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
CSEblock (node *arg_node, info *arg_info)
{
    size_t assigns;
    node *oldwithid = NULL;
    node *ivlet = NULL;

    DBUG_ENTER ();

    /* reset AVIS_SUBST for local variables */
    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    if (BLOCK_ASSIGNS (arg_node) != NULL) {
        /*
         * start new cse frame
         */
        assigns = TCcountAssigns (BLOCK_ASSIGNS (arg_node));
        INFO_CSE (arg_info) = AddCseInfoLayer (INFO_CSE (arg_info), assigns + 1);

        /*
         * create fake let node for iv = [i,j,k]
         */
        if (INFO_WITHID (arg_info) != NULL) {
            oldwithid = INFO_WITHID (arg_info);
            INFO_WITHID (arg_info) = NULL;

            if (WITHID_IDS (oldwithid) != NULL) {
                ivlet
                  = TBmakeLet (TBmakeIds (IDS_AVIS (WITHID_VEC (oldwithid)), NULL),
                               TCmakeIntVector (TCids2Exprs (WITHID_IDS (oldwithid))));
                INFO_CSE (arg_info) = AddLet (INFO_CSE (arg_info), ivlet);
            }
        }

        BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

        /*
         * remove the fake assignment of the index scalars to the index vector
         */
        if (oldwithid != NULL) {
            INFO_WITHID (arg_info) = oldwithid;
            ivlet = FREEoptFreeNode(ivlet);
        }

        /*
         * remove top cse frame
         */
        INFO_CSE (arg_info) = RemoveTopCseLayer (INFO_CSE (arg_info));
    }

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEassign (node *arg_node, info *arg_info)
 *
 * description:
 *   traverses assignment chain top-down and removes unused assignment.
 *
 *****************************************************************************/
node *
CSEassign (node *arg_node, info *arg_info)
{
    node *old_assign;

    DBUG_ENTER ();

    DBUG_ASSERT (ASSIGN_STMT (arg_node) != NULL, "assign node without instruction");

    old_assign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    INFO_ASSIGN (arg_info) = old_assign;

    /* traverse to next assignment in chain */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEcond(node *arg_node, info *arg_info)
 *
 * description:
 *   the stacking of available cse-expressions for both parts of the
 *   conditional is done by CSEblock.
 *   traverses condition, then- and else-part (in this order).
 *
 *****************************************************************************/
node *
CSEcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (COND_COND (arg_node) != NULL, "conditional without condition");
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);

    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEreturn(node *arg_node, info *arg_info)
 *
 * description:
 *  traverses the result expressions and starts analysis of return values
 *  in case of special fundefs.
 *
 *****************************************************************************/
node *
CSEreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RETURN_EXPRS (arg_node) = TRAVopt (RETURN_EXPRS (arg_node), arg_info);

    /*
     * analyse results and build up RESULTARG nodelist for results
     * that are args and therefore can be used directly in the calling context
     */
    if (FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info))) {
        INFO_RESULTARG (arg_info)
          = BuildSubstNodelist (RETURN_EXPRS (arg_node), INFO_FUNDEF (arg_info),
                                INFO_EXT_ASSIGN (arg_info));
    } else {
        INFO_RESULTARG (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSElet( node *arg_node, info *arg_info)
 *
 * description:
 *   first do a variable substitution on the right side expression (but
 *   only for simple expressions) or do CSE for a right-side with-loop
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
CSElet (node *arg_node, info *arg_info)
{
    node *match = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at %s in line %zu", AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))),
                NODE_LINE (arg_node));
    /*
     * traverse left-hand side t0 do variable substitutions for extrema
     * and saa
     */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    /*
     * traverse right side expression to do variable substitutions
     * or CSE in with-loops/special fundefs
     */
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /*
     * funconds must never be eliminated by CSE as this corrupts SSA form!!!
     */
    if (NODE_TYPE (LET_EXPR (arg_node)) != N_funcond) {
        match = FindCSE (INFO_CSE (arg_info), arg_node);
    }

    /*
     * found matching common subexpression or copy assignment
     * if let is NO phicopytarget
     * and let is NO assignment to some UNIQUE variable
     * do the necessary substitution.
     */
    if (match != NULL) {
        /* set subst attributes for results */
        LET_IDS (arg_node) = SetSubstAttributes (LET_IDS (arg_node), LET_IDS (match));

        DBUG_PRINT ("Common subexpression eliminated for %s in line %zu",
                    AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))), NODE_LINE (arg_node));
        global.optcounters.cse_expr++;

    } else {
        if ((NODE_TYPE (LET_EXPR (arg_node)) == N_ap)
            && (FUNDEF_ISLACFUN (AP_FUNDEF (LET_EXPR (arg_node))))) {
            /*
             * traverse the result ids to set the inferred subst information stored
             * in INFO_RESULTARG nodelist
             */
            LET_IDS (arg_node) = PropagateLoopInvariantArgs (LET_IDS (arg_node),
                                                             &INFO_RESULTARG (arg_info));

            /*
             * propagate identical results into calling fundef,
             * set according AVIS_SUBST information for duplicate results.
             * DeadCodeRemoval() will remove the unused result later.
             */
            LET_IDS (arg_node)
              = PropagateIdenticalReturn2Results (AP_FUNDEF (LET_EXPR (arg_node)),
                                                  LET_IDS (arg_node));

        } else {
            INFO_CSE (arg_info) = AddLet (INFO_CSE (arg_info), arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEap(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses parameter to do correct variable substitution.
 *   if special function application, start traversal of this fundef
 *
 *****************************************************************************/
node *
CSEap (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (AP_FUNDEF (arg_node) != NULL, "missing fundef in ap-node");
    DBUG_ASSERT (NULL != INFO_ASSIGN (arg_info), "Expected non-NULL INFO_ASSIGN");

    /*
     * when traversing the recursive ap of a LOOPFUN fundef,
     * we set RECFUNAP flag to TRUE, to avoid renamings of
     * loop-independent args that could be replaced otherwise.
     */
    if ((FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)))
        && (AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info))) {
        INFO_RECFUNAP (arg_info) = TRUE;
    } else {
        INFO_RECFUNAP (arg_info) = FALSE;
    }

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    /* reset traversal flag */
    INFO_RECFUNAP (arg_info) = FALSE;

    /* traverse lacfun without recursion */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("traverse in special fundef %s", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* keep this assignment as external assignment */
        INFO_EXT_ASSIGN (new_arg_info) = INFO_ASSIGN (arg_info);

        /* start with empty cse stack */
        INFO_CSE (new_arg_info) = NULL;

        /* propagate id substitutions into called fundef */
        FUNDEF_ARGS (AP_FUNDEF (arg_node))
          = PropagateSubst2Args (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node),
                                 AP_FUNDEF (arg_node));

        /* start traversal of lacfun */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("traversal of lacfun %s finished\n",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));

        /*
         * save RESULTARG nodelist for processing bypassed identifiers
         * in surrounding let
         */
        INFO_RESULTARG (arg_info) = INFO_RESULTARG (new_arg_info);

        new_arg_info = FreeInfo (new_arg_info);
    }
    DBUG_ASSERT (NULL != INFO_ASSIGN (arg_info), "Expected non-NULL INFO_ASSIGN");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEid( node *arg_node, info *arg_info)
 *
 * description:
 *   traverse id data to do substitution.
 *
 *****************************************************************************/
node *
CSEid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (ID_AVIS (arg_node) != NULL, "missing Avis backlink in id");
    DBUG_PRINT ("Looking at %s", AVIS_NAME (ID_AVIS (arg_node)));

    /* check for necessary substitution */
    if ((AVIS_SUBST (ID_AVIS (arg_node)) != NULL)
        && (!((INFO_RECFUNAP (arg_info)) && (AVIS_SSALPINV (ID_AVIS (arg_node)))))) {
        DBUG_PRINT ("Renaming: %s to %s", AVIS_NAME (ID_AVIS (arg_node)),
                    AVIS_NAME (AVIS_SUBST (ID_AVIS (arg_node))));

        /* do renaming to new ssa vardec */
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    /* process stored INFO_RESULTARG information */
    if (INFO_RESULTARG (arg_info) != NULL) {
        DBUG_ASSERT (AVIS_SUBST (ID_AVIS (arg_node)) == NULL,
                     "there must not exist any subst setting for"
                     " a freshly defined vardec");

        /* set AVIS_SUBST corresponding avis inferred by */
        AVIS_SUBST (ID_AVIS (arg_node)) = NODELIST_NODE (INFO_RESULTARG (arg_info));

        /* remove info of this result */
        INFO_RESULTARG (arg_info) = FREEfreeNodelistNode (INFO_RESULTARG (arg_info));

#ifndef DBUG_OFF
        if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
            DBUG_PRINT ("bypassing result %s", AVIS_NAME (ID_AVIS (arg_node)));
        }
#endif
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEwith(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse Part, withop and Code in this order
 *
 *
 *****************************************************************************/
node *
CSEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse and do variable substitution in partitions */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    /* traverse and do variable substitution in withops */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /* traverse and do cse in code blocks */
    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    INFO_WITHID (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEids(node *arg_node, info *arg_info)
 *
 * description:
 *   Performs CSE on MIN/MAX and SHAPE/DIM information as attached to the
 *   vardec. This is done here to ensure that we are in the right scope!
 *
 *****************************************************************************/
node *
CSEids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_node);

    AVIS_DIM (avis) = TRAVopt (AVIS_DIM (avis), arg_info);
    AVIS_SHAPE (avis) = TRAVopt (AVIS_SHAPE (avis), arg_info);
    AVIS_MIN (avis) = TRAVopt (AVIS_MIN (avis), arg_info);
    AVIS_MAX (avis) = TRAVopt (AVIS_MAX (avis), arg_info);
    AVIS_SCALARS (avis) = TRAVopt (AVIS_SCALARS (avis), arg_info);

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEcode(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse codeblock and expressions for each Ncode node
 *
 *
 *****************************************************************************/
node *
CSEcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse codeblock */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    /*traverse expression to do variable substitution */
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    /* traverse to next node */
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CSEdoCommonSubexpressionElimination(node *node)
 *
 * description:
 *   Starts the traversal either module or function based
 *
 *****************************************************************************/

node *
CSEdoCommonSubexpressionElimination (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_cse);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
