/*
 *
 * $Log$
 * Revision 1.47  2005/08/20 23:59:08  sah
 * fixed a DBUG message
 *
 * Revision 1.46  2005/07/19 22:49:04  sbs
 * funconds fixed
 * before, exprs chains were still expected.
 *
 * Revision 1.45  2005/07/18 18:24:14  sbs
 * eliminated FUNDEF_EXT_ASSIGN
 *
 * Revision 1.44  2005/03/04 21:21:42  cg
 * Useless conditional eliminated.
 * Integration of silently duplicated LaC funs at the end of the
 * fundef chain added.
 *
 * Revision 1.43  2004/11/26 19:43:43  khf
 * SacDevCamp04: COMPILES!!!!
 *
 * Revision 1.42  2004/11/26 15:00:42  khf
 * SacDevCamp04: COMPILES!!!
 *
 * Revision 1.41  2004/11/10 18:27:29  mwe
 * code for type upgrade added
 * use ntype-structure instead of type-structure
 * new code deactivated by MWE_NTYPE_READY
 *
 * Revision 1.40  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.39  2004/07/07 15:43:36  mwe
 * last changes undone (all changes connected to new type representation with ntype*)
 *
 * Revision 1.37  2004/05/04 14:27:30  khf
 * NCODE_CEXPR in SSACSENcode() replaced by NCODE_CEXPRS
 *
 * Revision 1.36  2004/03/06 20:06:40  mwe
 * CVPfuncond added
 *
 * Revision 1.35  2004/03/05 19:14:27  mwe
 * representation of conditional changed
 * using N_funcond node instead of phi
 *
 * Revision 1.34  2004/03/02 17:26:32  mwe
 * constant propagation deactivated
 *
 * Revision 1.33  2004/02/06 14:19:33  mwe
 * remove usage of PHIASSIGN and ASSIGN2
 * implement usage of primitive phi function instead
 *
 * Revision 1.32  2004/01/05 01:49:38  dkrHH
 * comment corrected
 *
 * Revision 1.31  2003/09/25 18:37:57  dkr
 * aggressive type propagation enabled
 *
 * Revision 1.30  2003/03/12 14:49:23  dkr
 * space in DBUG_PRINT added
 *
 * Revision 1.29  2002/10/11 16:27:14  dkr
 * unused var removed
 *
 * Revision 1.28  2002/10/10 23:56:18  dkr
 * comments added
 * type propagation modified
 *
 * Revision 1.27  2002/10/10 00:43:42  dkr
 * SSACSElet(): DBUG_PRINT modified
 *
 * Revision 1.26  2002/10/09 22:17:39  dkr
 * SSACSElet() modified: type propagation added
 * SSACSEPropagateSubst2Args() modified: type propagation added
 *
 * Revision 1.25  2002/09/03 18:49:58  dkr
 * some cosmetical changes done
 *
 * Revision 1.24  2002/09/03 11:56:54  dkr
 * - CompareTypesImplementation() modified.
 * - SSACSElet(): support for dynamic shapes added
 *
 * Revision 1.23  2001/07/10 11:27:44  nmw
 * ifdef added to disable uniqueness special handling in cse and lir
 * so long expressions with unique identifiers are not touched at all
 *
 * Revision 1.22  2001/06/01 10:01:30  nmw
 * insert N_empty node in empty blocks
 *
 * Revision 1.21  2001/05/31 14:53:28  nmw
 * CmpTypes() replaced by CompareTypesImplementation() to allow assignments
 * to types with same internal representation (e.g. double[2] and Complex)
 *
 * Revision 1.20  2001/05/30 15:50:55  nmw
 * ASSERT when substituting identifier of different types
 *
 * Revision 1.19  2001/05/28 09:03:58  nmw
 * CSE now removes duplicated results from special fundefs
 *
 * Revision 1.18  2001/05/25 08:43:13  nmw
 *
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
 * WARNING: This travesal works with new types only
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
#include "internal_lib.h"
#include "globals.h"
#include "new_types.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "convert.h"
#include "DupTree.h"
#include "SSACSE.h"
#include "compare_tree.h"
#include "optimize.h"
#include "ctinfo.h"

/*
 * INFO structure
 */
struct INFO {
    bool remassign;
    node *fundef;
    node *ext_assign;
    node *cse;
    node *module;
    node *assign;
    bool recfunap;
    nodelist *resultarg;
};

/*
 * INFO macros
 */
#define INFO_CSE_REMASSIGN(n) (n->remassign)
#define INFO_CSE_FUNDEF(n) (n->fundef)
#define INFO_CSE_EXT_ASSIGN(n) (n->ext_assign)
#define INFO_CSE_CSE(n) (n->cse)
#define INFO_CSE_MODULE(n) (n->module)
#define INFO_CSE_ASSIGN(n) (n->assign)
#define INFO_CSE_RECFUNAP(n) (n->recfunap)
#define INFO_CSE_RESULTARG(n) (n->resultarg)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_CSE_REMASSIGN (result) = FALSE;
    INFO_CSE_FUNDEF (result) = NULL;
    INFO_CSE_EXT_ASSIGN (result) = NULL;
    INFO_CSE_CSE (result) = NULL;
    INFO_CSE_MODULE (result) = NULL;
    INFO_CSE_ASSIGN (result) = NULL;
    INFO_CSE_RECFUNAP (result) = FALSE;
    INFO_CSE_RESULTARG (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

typedef enum { THENPART, ELSEPART } condpart;

/* functions to handle cseinfo chains */
static node *AddCseinfo (node *cseinfo, node *let);
static node *CreateNewCselayer (node *cseinfo);
static node *RemoveTopCselayer (node *cseinfo);
static node *SetSubstAttributes (node *subst, node *with);

/* helper functions for internal use only */
static node *FindCse (node *cselist, node *let);

static bool CmpIdsTypes (node *ichain1, node *ichain2);
static node *PropagateSubst2Args (node *fun_args, node *ap_args, node *fundef);
static node *PropagateReturn2Results (node *ap_fundef, node *ids_chain);
static nodelist *BuildSubstNodelist (node *return_exprs, node *fundef, node *ext_assign);
static node *GetResultArgAvis (node *id, condpart cp);
static node *GetApAvisOfArgAvis (node *arg_avis, node *fundef, node *ext_assign);

/******************************************************************************
 *
 * function:
 *   node *AddCseinfo(node *cseinfo, node *let)
 *
 * description:
 *   Adds an new let node to the actual cse layer to store an available
 *   expression in the curreent context.
 *
 ******************************************************************************/
static node *
AddCseinfo (node *cseinfo, node *let)
{
    DBUG_ENTER ("AddCseInfo");

    DBUG_ASSERT ((cseinfo != NULL), "cseinfo layer stack is NULL");
    DBUG_ASSERT ((let != NULL), "add NULL let to cseinfo layer stack");

    DBUG_PRINT ("CSE", ("add let node " F_PTR " to cse layer " F_PTR, let, cseinfo));

    if (CSEINFO_LET (cseinfo) == NULL) {
        /* add letnode top existing empty cseinfo node */
        CSEINFO_LET (cseinfo) = let;
    } else {
        /*
         * create new cseinfo node with let attribute on actual layer
         * and adds it in front of the cseinfo chain.
         */
        cseinfo = TBmakeCseinfo (CSEINFO_LAYER (cseinfo), let, cseinfo);
    }

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *CreateNewCselayer(node *cseinfo)
 *
 * description:
 *   starts a new layer of cse let nodes. this happens when we enter a new
 *   withloop or a then/else part of an conditional.
 *
 ******************************************************************************/
static node *
CreateNewCselayer (node *cseinfo)
{
    DBUG_ENTER ("CreateNewCselayer");

    cseinfo = TBmakeCseinfo (NULL, NULL, cseinfo);

    DBUG_PRINT ("CSE", ("create new cse layer " F_PTR, cseinfo));

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
RemoveTopCselayer (node *cseinfo)
{
    node *tmp;
    node *freetmp;

    DBUG_ENTER ("RemoveTopCselayer");

    tmp = cseinfo;

    while (tmp != NULL) {
        freetmp = tmp;
        cseinfo = CSEINFO_NEXT (tmp);

        if (tmp == CSEINFO_LAYER (tmp)) {
            tmp = NULL;
        } else {
            tmp = CSEINFO_NEXT (tmp);
        }
        DBUG_PRINT ("CSE", ("removing csenode " F_PTR, freetmp));
        FREEdoFreeNode (freetmp);
    }

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *FindCse(node *cselist, node *let)
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
FindCse (node *cselist, node *let)
{
    node *match;
    node *csetmp;

    DBUG_ENTER ("FindCse");
    DBUG_ASSERT ((let != NULL), "FindCse is called with empty let node");

    match = NULL;
    csetmp = cselist;

    while ((csetmp != NULL) && (match == NULL)) {
        if ((CSEINFO_LET (csetmp) != NULL)
            && (CMPTdoCompareTree (LET_EXPR (let), LET_EXPR (CSEINFO_LET (csetmp)))
                == CMPT_EQ)
            && CmpIdsTypes (LET_IDS (let), LET_IDS (CSEINFO_LET (csetmp)))) {
            match = CSEINFO_LET (csetmp);
        }
        csetmp = CSEINFO_NEXT (csetmp);
    }

    DBUG_RETURN (match);
}

/******************************************************************************
 *
 * function:
 *    bool CmpIdsTypes( ids *ichain1, ids *ichain2)
 *
 * description:
 *    compares the types of all ids in two given ids chains.
 *    return TRUE if the types of each two corresponding ids are equal.
 *
 ******************************************************************************/
static bool
CmpIdsTypes (node *ichain1, node *ichain2)
{
    bool result;

    DBUG_ENTER ("CmpIdsTypes");

    if (ichain1 != NULL) {
        DBUG_ASSERT ((ichain2 != NULL), "comparing different ids chains");
        if (TYcmpTypes (IDS_NTYPE (ichain1), IDS_NTYPE (ichain2)) == TY_eq) {
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

    DBUG_ENTER ("SetSubstAttributes");

    tmpsubst = subst;
    tmpwith = with;

    while (tmpsubst != NULL) {
        DBUG_PRINT ("CSE", ("substitute ids %s with %s", (IDS_NAME (tmpsubst)),
                            (IDS_NAME (tmpwith))));

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

    DBUG_ENTER ("PropagateSubst2Args");

    act_fun_arg = fun_args;
    act_ap_arg = ap_args;
    while (act_fun_arg != NULL) {
        /* process all arguments */
        DBUG_ASSERT ((act_ap_arg != NULL), "too few arguments in function application");

        /* init AVIS_SUBST attribute (means no substitution) */
        AVIS_SUBST (ARG_AVIS (act_fun_arg)) = NULL;

        /* get external identifier in function application (here we use it avis) */
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (act_ap_arg)) == N_id),
                     "non N_id node as arg in special function application");
        ext_ap_avis = ID_AVIS (EXPRS_EXPR (act_ap_arg));
        ext_ap_type = AVIS_TYPE (ext_ap_avis);
        /*
         * specialize types of formal parameters if possible
         */
        cmp = TYcmpTypes (AVIS_TYPE (ARG_AVIS (act_fun_arg)), ext_ap_type);

        stype1 = TYtype2String (AVIS_TYPE (ARG_AVIS (act_fun_arg)), TRUE, 0);
        stype2 = TYtype2String (ext_ap_type, TRUE, 0);
        if ((cmp == TY_eq) || (cmp == TY_gt)) {

            DBUG_PRINT ("CSE", ("type of formal LaC-fun (%s) arg specialized in line %d:"
                                "  %s:%s->%s",
                                CTIitemName (fundef), NODE_LINE (act_fun_arg),
                                ARG_NAME (act_fun_arg), stype1, stype2));
            /*
             * actual type is subtype of formal type -> specialize
             */
            AVIS_TYPE (ARG_AVIS (act_fun_arg))
              = TYfreeType (AVIS_TYPE (ARG_AVIS (act_fun_arg)));
            AVIS_TYPE (ARG_AVIS (act_fun_arg)) = TYcopyType (ext_ap_type);
        } else if ((cmp == TY_dis) || (TY_hcs)) {
            /*
             * special function application with incompatible types found
             */
            DBUG_PRINT ("CSE", ("application of LaC-function %s with incompatible types"
                                " in line %d:  %s:%s->%s",
                                FUNDEF_NAME (fundef), NODE_LINE (act_fun_arg),
                                ARG_NAME (act_fun_arg), stype1, stype2));
        }
        stype1 = ILIBfree (stype1);
        stype2 = ILIBfree (stype2);

        /*
         * now search the application args for identical id to substitute it.
         * this is possible only for args of cond-funs and loopinvariant args
         * of loop-funs.
         */
        if ((FUNDEF_ISCONDFUN (fundef))
            || ((FUNDEF_ISDOFUN (fundef)) && (AVIS_SSALPINV (ARG_AVIS (act_fun_arg))))) {
            found_match = FALSE;

            search_fun_arg = fun_args;
            search_ap_arg = ap_args;
            while ((search_fun_arg != act_fun_arg) && (!found_match)) {
                /* compare identifiers via their avis pointers */
                DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (search_ap_arg)) == N_id),
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
                    DBUG_PRINT ("CSE", ("propagate copy propagation info for %s -> %s",
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
    DBUG_ASSERT ((act_ap_arg == NULL), "too many arguments in function application");

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

    DBUG_ENTER ("BuildSubstNodelist");

    DBUG_ASSERT ((FUNDEF_ISLACFUN (fundef)),
                 "BuildSubstNodelist called for non special fundef");

    if (return_exprs != NULL) {
        /* check for arg as result */
        avis1 = GetResultArgAvis (EXPRS_EXPR (return_exprs), THENPART);
        avis2 = GetResultArgAvis (EXPRS_EXPR (return_exprs), ELSEPART);

        if (((FUNDEF_ISDOFUN (fundef)) && (avis2 != NULL) && (AVIS_SSALPINV (avis2)))
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
 *   node *PropagateReturn2Results(node *ap_fundef, node *ids_chain)
 *
 * description:
 *   searches in the return statement of loops (here the then-part phi-copy-
 *   targets) for identical result expressions and propagates this
 *   information into the result ids_chain by setting the AVIS_SUBST attribute
 *   for later substitutions.
 *
 *****************************************************************************/
static node *
PropagateReturn2Results (node *ap_fundef, node *ids_chain)
{
    node *act_result;
    node *act_exprs;
    node *search_result;
    node *search_exprs;

    bool found_match;

    DBUG_ENTER ("PropagateReturn2Results");

    /* process all identifier of result chain of a loop special fundef */
    act_result = ids_chain;
    act_exprs = RETURN_EXPRS (FUNDEF_RETURN (ap_fundef));

    while ((FUNDEF_ISDOFUN (ap_fundef)) && (act_result != NULL)) {
        /* search the the processed results for identical results */
        search_result = ids_chain;
        search_exprs = RETURN_EXPRS (FUNDEF_RETURN (ap_fundef));
        found_match = FALSE;

        while ((search_result != act_result) && (!found_match)) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (act_exprs)) == N_id),
                         "non id node in return of special fundef (act)");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (search_exprs)) == N_id),
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

    DBUG_ENTER ("GetResultArgAvis");

    result = NULL;

    DBUG_ASSERT ((NODE_TYPE (id) == N_id), "GetResultArgAvis called for non id node");

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

    DBUG_ENTER ("GetApAvisOfArgAvis");

    DBUG_ASSERT ((FUNDEF_ISLACFUN (fundef)),
                 "GetApAvisOfArgAvis called for non special fundef");

    arg_chain = FUNDEF_ARGS (fundef);

    exprs_chain = AP_ARGS (ASSIGN_RHS (ext_assign));

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
    DBUG_ENTER ("CSEfundef");

    INFO_CSE_FUNDEF (arg_info) = arg_node;
    INFO_CSE_RESULTARG (arg_info) = NULL;

    if ((FUNDEF_ARGS (arg_node) != NULL) && (!(FUNDEF_ISLACFUN (arg_node)))) {
        /*
         * traverse args of fundef to init the AVIS_SUBST attribute. this is done
         * here only for normal fundefs. in special fundefs that are traversed via
         * CSEap() we do a substitution information propagation (see
         * CSEPropagateSubst2Args() ) that sets the correct AVIS_SUBST
         * attributes for all args.
         */
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEarg( node *arg_node, info *arg_info)
 *
 * description:
 *   traverses chain of args to init SUBST attribute with NULL.
 *
 *****************************************************************************/
node *
CSEarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSEarg");

    AVIS_SUBST (ARG_AVIS (arg_node)) = NULL;

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }
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
    DBUG_ENTER ("CSEblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* traverse vardecs of block */
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    /* start new cse frame */
    INFO_CSE_CSE (arg_info) = CreateNewCselayer (INFO_CSE_CSE (arg_info));

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* traverse assignments of block */
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    } else {
        /* insert at least the N_empty node in an empty block */
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    /* remove top cse frame */
    INFO_CSE_CSE (arg_info) = RemoveTopCselayer (INFO_CSE_CSE (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEvardec(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse chain of vardecs to init avis subst attribute.
 *
 *
 *****************************************************************************/
node *
CSEvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSEvardec");

    AVIS_SUBST (VARDEC_AVIS (arg_node)) = NULL;

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

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
    bool remassign;
    node *tmp;

    DBUG_ENTER ("SSEassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "assign node without instruction");

    old_assign = INFO_CSE_ASSIGN (arg_info);
    INFO_CSE_ASSIGN (arg_info) = arg_node;
    INFO_CSE_REMASSIGN (arg_info) = FALSE;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    remassign = INFO_CSE_REMASSIGN (arg_info);
    INFO_CSE_ASSIGN (arg_info) = old_assign;

    /* traverse to next assignment in chain */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* in bottom-up traversal free this assignment if it was marked as unused */
    if (remassign) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FREEdoFreeNode (tmp);
    }

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
    DBUG_ENTER ("CSEcond");

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "conditional without condition");
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    INFO_CSE_REMASSIGN (arg_info) = FALSE;

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
    DBUG_ENTER ("CSEreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    /*
     * analyse results and build up RESULTARG nodelist for results
     * that are args and therefore can be used directly in the calling context
     */
    if (FUNDEF_ISLACFUN (INFO_CSE_FUNDEF (arg_info))) {
        INFO_CSE_RESULTARG (arg_info)
          = BuildSubstNodelist (RETURN_EXPRS (arg_node), INFO_CSE_FUNDEF (arg_info),
                                INFO_CSE_EXT_ASSIGN (arg_info));
    } else {
        INFO_CSE_RESULTARG (arg_info) = NULL;
    }

    INFO_CSE_REMASSIGN (arg_info) = FALSE;

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
    node *match;
    nodetype nt_expr;

    DBUG_ENTER ("CSElet");

    DBUG_PRINT ("CSE", ("inspecting expression for cse"));

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");
    DBUG_ASSERT ((LET_IDS (arg_node) != NULL), "let without ids");

    /*
     * traverse right side expression to do variable substitutions
     * or CSE in with-loops/special fundefs
     */
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    match = FindCse (INFO_CSE_CSE (arg_info), arg_node);

    nt_expr = NODE_TYPE (LET_EXPR (arg_node));

    /*
     * found matching common subexpression or copy assignment
     * if let is NO phicopytarget
     * and let is NO assignment to some UNIQUE variable
     * do the necessary substitution.
     */
    if ((match != NULL)) {
        /* set subst attributes for results */
        LET_IDS (arg_node) = SetSubstAttributes (LET_IDS (arg_node), LET_IDS (match));

        DBUG_PRINT ("CSE",
                    ("Common subexpression eliminated in line %d", NODE_LINE (arg_node)));
        cse_expr++;

        /* remove assignment */
        INFO_CSE_REMASSIGN (arg_info) = TRUE;
    } else if ((nt_expr == N_ap) && (FUNDEF_ISLACFUN (AP_FUNDEF (LET_EXPR (arg_node))))) {

        /*
         * traverse the result ids to set the infered subst information stored
         * in INFO_CSE_RESULTARG nodelist
         */
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);

        /*
         * propagate identical results into calling fundef,
         * set according AVIS_SUBST information for duplicate results.
         * DeadCodeRemoval() will remove the unused result later.
         */
        LET_IDS (arg_node)
          = PropagateReturn2Results (AP_FUNDEF (LET_EXPR (arg_node)), LET_IDS (arg_node));

    } else {
        /* new expression found */
        DBUG_PRINT ("CSE", ("add new expression to cselist"));
        INFO_CSE_CSE (arg_info) = AddCseinfo (INFO_CSE_CSE (arg_info), arg_node);

        /* do not remove assignment */
        INFO_CSE_REMASSIGN (arg_info) = FALSE;
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
    DBUG_ENTER ("CSEap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    /*
     * when traversing the recursive ap of a special loop fundef,
     * we set RECFUNAP flag to TRUE, to avoid renamings of loop
     * independend args that could be replaced otherwise.
     */
    if ((FUNDEF_ISDOFUN (INFO_CSE_FUNDEF (arg_info)))
        && (AP_FUNDEF (arg_node) == INFO_CSE_FUNDEF (arg_info))) {
        INFO_CSE_RECFUNAP (arg_info) = TRUE;
    } else {
        INFO_CSE_RECFUNAP (arg_info) = FALSE;
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /* reset traversal flag */
    INFO_CSE_RECFUNAP (arg_info) = FALSE;

    /* traverse special fundef without recursion */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_CSE_FUNDEF (arg_info))) {
        DBUG_PRINT ("CSE", ("traverse in special fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* keep this assignment as external assignment */
        INFO_CSE_EXT_ASSIGN (new_arg_info) = INFO_CSE_ASSIGN (arg_info);

        /* start with empty cse stack */
        INFO_CSE_CSE (new_arg_info) = NULL;
        INFO_CSE_MODULE (new_arg_info) = INFO_CSE_MODULE (arg_info);

        /* propagate id substitutions into called fundef */
        FUNDEF_ARGS (AP_FUNDEF (arg_node))
          = PropagateSubst2Args (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node),
                                 AP_FUNDEF (arg_node));

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("CSE", ("traversal of special fundef %s finished\n",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /*
         * save RESULTARG nodelist for processing bypassed identifiers
         * in surrounding let
         */
        INFO_CSE_RESULTARG (arg_info) = INFO_CSE_RESULTARG (new_arg_info);

        new_arg_info = FreeInfo (new_arg_info);

    } else {
        DBUG_PRINT ("CSE", ("do not traverse in normal fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   ids *CSEids( ids *arg_ids, info *arg_info)
 *
 * description:
 *   Traverses chain of ids to do variable substitution as annotated in
 *   AVIS_SUBST attribute, but does not substitute loop invariant args in a
 *   recursive funap (iff INFO_CSE_RECFUNAP() is TRUE),
 *
 *   If we have some nodelist stored in INFO_CSE_RESULTARG annotate the
 *   stored subst avis information after processing, so further uses will be
 *   renamed according to this information.
 *
 *****************************************************************************/

node *
CSEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSEids");

    DBUG_ASSERT ((IDS_AVIS (arg_node) != NULL), "missing Avis backlink in ids");

    /* check for necessary substitution */
    if ((AVIS_SUBST (IDS_AVIS (arg_node)) != NULL)
        && (!((INFO_CSE_RECFUNAP (arg_info)) && (AVIS_SSALPINV (IDS_AVIS (arg_node)))))) {
        DBUG_PRINT ("CSE",
                    ("substitution: %s -> %s",
                     VARDEC_OR_ARG_NAME (AVIS_DECL (IDS_AVIS (arg_node))),
                     VARDEC_OR_ARG_NAME (AVIS_DECL (AVIS_SUBST (IDS_AVIS (arg_node))))));

        /* do renaming to new ssa vardec */
        IDS_AVIS (arg_node) = AVIS_SUBST (IDS_AVIS (arg_node));
    }

    /* process stored INFO_CSE_RESULTARG information */
    if (INFO_CSE_RESULTARG (arg_info) != NULL) {
        DBUG_ASSERT ((AVIS_SUBST (IDS_AVIS (arg_node)) == NULL),
                     "there must not exist any subst setting for"
                     " a fresh defined vardec");

        /* set AVIS_SUBST corresponding avis infered by */
        AVIS_SUBST (IDS_AVIS (arg_node)) = NODELIST_NODE (INFO_CSE_RESULTARG (arg_info));

        /* remove info of this result */
        INFO_CSE_RESULTARG (arg_info)
          = FREEfreeNodelistNode (INFO_CSE_RESULTARG (arg_info));

#ifndef DBUG_OFF
        if (AVIS_SUBST (IDS_AVIS (arg_node)) != NULL) {
            DBUG_PRINT ("CSE", ("bypassing result %s",
                                VARDEC_OR_ARG_NAME (AVIS_DECL (IDS_AVIS (arg_node)))));
        }
#endif
    }

    /* traverse to next ids in chain */
    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CSEid( node *arg_node, info *arg_info)
 *
 * description:
 *   traverse ids data to do substitution.
 *
 *****************************************************************************/
node *
CSEid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSEid");

    DBUG_ASSERT ((ID_AVIS (arg_node) != NULL), "missing Avis backlink in id");

    /* check for necessary substitution */
    if ((AVIS_SUBST (ID_AVIS (arg_node)) != NULL)
        && (!((INFO_CSE_RECFUNAP (arg_info)) && (AVIS_SSALPINV (ID_AVIS (arg_node)))))) {
        DBUG_PRINT ("CSE",
                    ("substitution: %s -> %s",
                     VARDEC_OR_ARG_NAME (AVIS_DECL (ID_AVIS (arg_node))),
                     VARDEC_OR_ARG_NAME (AVIS_DECL (AVIS_SUBST (ID_AVIS (arg_node))))));

        /* do renaming to new ssa vardec */
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    /* process stored INFO_CSE_RESULTARG information */
    if (INFO_CSE_RESULTARG (arg_info) != NULL) {
        DBUG_ASSERT ((AVIS_SUBST (ID_AVIS (arg_node)) == NULL),
                     "there must not exist any subst setting for"
                     " a fresh defined vardec");

        /* set AVIS_SUBST corresponding avis infered by */
        AVIS_SUBST (ID_AVIS (arg_node)) = NODELIST_NODE (INFO_CSE_RESULTARG (arg_info));

        /* remove info of this result */
        INFO_CSE_RESULTARG (arg_info)
          = FREEfreeNodelistNode (INFO_CSE_RESULTARG (arg_info));

#ifndef DBUG_OFF
        if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
            DBUG_PRINT ("CSE", ("bypassing result %s",
                                VARDEC_OR_ARG_NAME (AVIS_DECL (ID_AVIS (arg_node)))));
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
    DBUG_ENTER ("CSEwith");

    /* traverse and do variable substitution in partitions */
    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    /* traverse and do variable substitution in withops */
    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    /* traverse and do cse in code blocks */
    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

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
    DBUG_ENTER ("CSEcode");

    /* traverse codeblock */
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    /*traverse expression to do variable substitution */
    if (CODE_CEXPRS (arg_node) != NULL) {
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    }

    /* traverse to next node */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CSEdoCse(node* fundef, node* modul)
 *
 * description:
 *   Starts the traversal for a given fundef.
 *   Starting fundef must not be a special fundef (do, while, cond) created by
 *   lac2fun transformation. These "inline" functions will be traversed in
 *   their order of usage.
 *
 *****************************************************************************/
node *
CSEdoCse (node *fundef, node *module)
{
    info *arg_info;

    DBUG_ENTER ("CSEdoCse");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "CSE called for non-fundef node");

    DBUG_PRINT ("OPT", ("starting common subexpression elimination (ssa) in function %s",
                        FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if (!(FUNDEF_ISLACFUN (fundef))) {
        DBUG_ASSERT ((global.optimize.dodcr), "CSE requieres DCR");

        arg_info = MakeInfo ();

        INFO_CSE_CSE (arg_info) = NULL;
        INFO_CSE_MODULE (arg_info) = module;

        TRAVpush (TR_cse);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}
