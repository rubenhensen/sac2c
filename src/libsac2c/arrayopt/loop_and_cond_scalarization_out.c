/**
 *
 * @file loop_and_cond_scalarization_out.c
 *
 * See comments in loop_and_cond_scalarization_in.c for a general
 * description of this pair of optimizations.
 *
 * The basic idea in this traversal is that we have a call
 * to a lacfun that returns some small arrays, such as shape vectors.
 * If all uses of those vectors within the lacfun can be transformed
 * to scalarized form, then we can avoid the memory management overhead
 * associated with creating and destroying those arrays.
 *
 * Here is a simple example:
 *
 *   shp, z = loop( args); NB. shp is shape(z);
 *   size = shp[0];
 *   res = sum( take( [size - 1], z));
 *
 * The loop has to generate a shape vector on every iteration,
 * which makes it very slow. Hence, we transform the outer code this
 * way:
 *
 *   s0, shp, z = loop( args); NB. shp is shape(z);
 *   shp = [ s0];
 *   size = shp[0];
 *   res = sum( take( [size - 1], z));
 *
 * CF and DCR turn this into:
 *
 *   s0, shp, z = loop( args); NB. shp is shape(z);
 *   res = sum( take( [s0 - 1], z));
 *
 * and SISI will eventually get rid of shp.
 *
 * The lacfun itself is transformed similarly. For a condfun, we start with:
 *
 *   int[1], int[.] condfun( args)
 *   {
 *    if( cond) {
 *      shpt = [20];
 *      rest = iota( shpt[0]);
 *    } else {
 *      shpf = [40];
 *      resf = iota( shpf[0]);
 *    }
 *
 *    zval = cond ? rest : resf;
 *    zshp = cond ? shpt : shpf;
 *    return( zshp, zval);
 *    }
 *
 *  We want to eliminate the int[1] result, so we scalarize both
 *  legs of zshp, inserting appropriate sel() instructions into both
 *  legs of the IF, add a new condfun at the end to pick the right
 *  scalar, and prefix a new result element:
 *
 *   int[1], int[.] condfun( args)
 *   {
 *    if( cond) {
 *      shpt = [20];
 *      rest = iota( shpt[0]);
 *      s0t = shpt[0];
 *    } else {
 *      shpf = [40];
 *      resf = iota( shpf[0]);
 *      s0f = shpf[0];
 *    }
 *
 *    zval = cond ? rest : resf;
 *    zshp = cond ? shpt : shpf;
 *    zs0  = cond ? s0t  : s0f;
 *
 *    return( zs0, zshp, zval);
 *    }
 *
 *  Loopfuns work similarly, except the N_cond is near
 *  the end of the function. We start with:
 *
 *  int main()
 *  {
 *    int[1] shp;
 *    int[.] ovec;
 *    ovec = iota(5);
 *    shp, vec = loop( ovec);
 *    ...
 *   }
 *
 *   int[1], int[.] loop( vec)
 *   {
 *     vec2 = vec ++ [42];
 *     shp1 = shape( vec2);
 *     ... code ...
 *     if( cond) {
 *        shp2, vec3 = loop( vec2);
 *     } else {
 *     }
 *       zvec = cond ? vec3 : vec2;
 *       zshp = cond ? shp2 : shp1;
 *       return( zshp, zvec);
 *   }
 *
 *   LACSO turns this info:
 *
 *  int main()
 *  {
 *    int[1] shp;
 *    int[.] ovec;
 *    int    s0;
 *    ovec = iota(5);
 *    s0, oldshp, vec = loop( ovec);
 *    shp = [s0];
 *    ...
 *   }
 *
 *   int, int[1], int[.] loop( vec)
 *   {
 *     vec2 = vec ++ [42];
 *     shp1 = shape( vec2);
 *     ... code ...
 *     s0 = shp1[0];
 *     if( cond) {
 *        s0prime, shp2, vec3 = loop( vec2);
 *     } else {
 *     }
 *       zvec = cond ? vec3 : vec2;
 *       zshp = cond ? shp2 : shp1;
 *       sshp = cond ? s0prime :s0;
 *       return( sshp, zshp, zvec);
 *   }
 *
 *  If all references to zshp are eliminated by other optimizations,
 *  then zshp (the intp1[]) will be removed by SISI.
 *
 *  Similarly, if all references to shp in main() are replaced by
 *  references to s0, then the shp N_array will be eliminated.
 *
 *
 * Comments on the implementation:
 *
 * 1. When we find an N_ap that calls a LACFUN, we traverse the
 *    lacfun. That traversal builds all the code changes on
 *    the downward traversal, and puts all lacfun code mods
 *    into their proper locations on the way back up.
 *
 * 2. We examine each lacfun N_return argument to see if it is
 *    a small array.
 *
 * 3. For any small array result, we find the FALSE leg of
 *    the recursive call test, and generate a scalarized version
 *    of that result at that point.
 *
 */

#define DBUG_PREFIX "LACSO"
#include "debug.h"

#include "types.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "constants.h"
#include "globals.h"
#include "memory.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "LookUpTable.h"
#include "pattern_match.h"
#include "flattengenerators.h"
#include "lacfun_utilities.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *reccall;
    node *vardecs;
    node *preassignsthen;
    node *preassignselse;
    node *ap;
    node *newfunconds;
    node *funcondexprs;
    node *rets;
    node *reccallerids;
    node *callervardecs;
    node *letids;
    node *assign;
    node *fda;
    bool findingreturnids;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_RECCALL(n) ((n)->reccall)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNSTHEN(n) ((n)->preassignsthen)
#define INFO_PREASSIGNSELSE(n) ((n)->preassignselse)
#define INFO_AP(n) ((n)->ap)
#define INFO_NEWFUNCONDS(n) ((n)->newfunconds)
#define INFO_FUNCONDEXPRS(n) ((n)->funcondexprs)
#define INFO_RETS(n) ((n)->rets)
#define INFO_RECCALLERIDS(n) ((n)->reccallerids)
#define INFO_CALLERVARDECS(n) ((n)->callervardecs)
#define INFO_LETIDS(n) ((n)->letids)
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_FDA(n) ((n)->fda)
#define INFO_FINDINGRETURNIDS(n) ((n)->findingreturnids)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_RECCALL (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNSTHEN (result) = NULL;
    INFO_PREASSIGNSELSE (result) = NULL;
    INFO_AP (result) = NULL;
    INFO_NEWFUNCONDS (result) = NULL;
    INFO_FUNCONDEXPRS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_RECCALLERIDS (result) = NULL;
    INFO_CALLERVARDECS (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_FDA (result) = NULL;
    INFO_FINDINGRETURNIDS (result) = FALSE;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/******************************************************************************
 *
 * function: node *BuildFunconds( node *avis,
 *                                node *newexprsthen, node *newexprselse,
 *                                info *arg_info)
 *
 * description: Build new funcond N_assign nodes to be placed above
 *              the N_return of the lacfun.
 *
 *              Build an N_exprs chain of scalars, to be prefixed to the
 *              N_return of the lacfun, and N_ret elements to be
 *              prefixed to FUNDEF_RETS.
 *
 *              Build an N_array of those elements, which may be
 *              needed as AVIS_SHAPE of another N_return element,
 *              if this non-scalarized result element is acting as an
 *              AVIS_SHAPE for that one. If we do not make this
 *              replacement, then we will be unable to eliminate
 *              the non-scalarized result element.
 *
 *              Build vardecs, avis, and ids nodes for the caller's
 *              new results.
 *
 *              We can safely reuse the FUNCOND_IF from the existing N_return's
 *              assign, because all of the FUNCOND_IFs are the same in a lacfun.
 *
 * @params: avis: A non-scalarized element of the original RETURN_EXPRS chain.
 *
 *****************************************************************************/
static node *
BuildFunconds (node *avis, node *newexprsthen, node *newexprselse, info *arg_info)
{
    node *fc;
    node *assgns = NULL;
    node *funcond;
    node *calleravis;
    ntype *typ;
    node *narr;
    node *elems = NULL;
    shape *shp;

    DBUG_ENTER ();

    while (NULL != newexprsthen) {
        // We depend on strict structure of this part of the AST.
        funcond = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avis)));
        DBUG_ASSERT (N_funcond == NODE_TYPE (funcond),
                     "Did not find N_funcond at N_return");
        fc = TBmakeFuncond (DUPdoDupNode (FUNCOND_IF (funcond)),
                            EXPRS_EXPR (newexprsthen), EXPRS_EXPR (newexprselse));
        fc = FLATGexpression2Avis (fc, &INFO_VARDECS (arg_info), &assgns, NULL);
        INFO_FUNCONDEXPRS (arg_info) = TCappendExprs (INFO_FUNCONDEXPRS (arg_info),
                                                      TBmakeExprs (TBmakeId (fc), NULL));
        typ = TYcopyType (AVIS_TYPE (fc));
        INFO_RETS (arg_info) = TCappendRet (INFO_RETS (arg_info), TBmakeRet (typ, NULL));
        calleravis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (fc)));
        INFO_CALLERVARDECS (arg_info)
          = TBmakeVardec (calleravis, INFO_CALLERVARDECS (arg_info));
        elems = TCappendExprs (elems, TBmakeExprs (TBmakeId (calleravis), NULL));

        newexprsthen = EXPRS_NEXT (newexprsthen);
        newexprselse = EXPRS_NEXT (newexprselse);
    }

    // We use AVIS_LACSO as a short-term hideyhole for the scalarized N_array,
    // as a way to tie the scalarized N_id nodes to the non-scalarized
    // original array.
    //
    // I.e., the use of AVIS_LACSO is emphemeral, as it exists only within this
    // traversal. It you can think of a nice way to tie those
    // together without using an N_avis element, please feel free to
    // change it.
    //
    typ = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (avis))), SHcreateShape (0));
    shp = TYgetShape (AVIS_TYPE (avis));
    narr = TBmakeArray (typ, SHcopyShape (shp), elems);
    AVIS_LACSO (IDS_AVIS (INFO_LETIDS (arg_info))) = narr;

    DBUG_RETURN (assgns);
}

/******************************************************************************
 *
 * function: node *ReplaceAvisShapeByScalars( node *arg_node,
 *                                             node *avis, node *scalars)
 *
 * description: For all vardecs in fundef arg_node, replace any AVIS_SHAPE
 *              nodes that refer to avis by scalars.
 *              This is a fix for Bug #1028.
 *
 * @result: All the real work is side effects in the calling function's
 *          VARDEC_AVIS nodes.
 *
 *****************************************************************************/
static void
ReplaceAvisShapeByScalars (node *arg_node, node *avis, node *scalars)
{
    node *vardecs;

    DBUG_ENTER ();

    vardecs = BLOCK_VARDECS (FUNDEF_BODY (arg_node));
    ;
    while (NULL != vardecs) {
        if ((NULL != AVIS_SHAPE (VARDEC_AVIS (vardecs)))
            && (N_id == NODE_TYPE (AVIS_SHAPE (VARDEC_AVIS (vardecs))))
            && (avis == ID_AVIS (AVIS_SHAPE (VARDEC_AVIS (vardecs))))) {
            DBUG_PRINT ("In vardec %s, replacing AVIS_SHAPE %s by scalars",
                        AVIS_NAME (VARDEC_AVIS (vardecs)), AVIS_NAME (avis));
            FREEdoFreeNode (AVIS_SHAPE (VARDEC_AVIS (vardecs)));
            AVIS_SHAPE (VARDEC_AVIS (vardecs)) = DUPdoDupNode (scalars);
        }
        vardecs = VARDEC_NEXT (vardecs);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function: node *BuildExternalAssigns(node *arg_node, info *arg_info)
 *
 * description: If arg_node N_assign is an N_ap calling a lacfun, examine all
 *              of its LET_IDS elements. Skip this if we are looking
 *              at the recursive call of a loopfun.
 *
 *              For any newly-scalarized result elements, generate the
 *              old result as an N_array. For example, if we started with:
 *
 *                int[2] vec;
 *                vec = lacfun( xxx);
 *
 *              When we get here, we have:
 *
 *                int[2] vec{ scalars: [s0, s1]};
 *                int    s0;
 *                int    s1;
 *                s0, s1, vec = lacfun( xxx);
 *
 *              This code replaces that with this:
 *
 *                int[2] vec;
 *                int    s0,
 *                int    s1;
 *                s0, s1, junk = lacfun( xxx);
 *                vec = [ s0, s1];
 *
 *             NB. We steal the old vec and reuse it.
 *
 *             Finally, we traverse the FUNDEF_VARDEC chain for the
 *             lacfun, and replace any AVIS_SHAPE references to vec
 *             with a copy of the N_array [ s0, s1].
 *             This is required because of interactions with ISAA,
 *             which places saabind() immediately
 *
 *
 *****************************************************************************/
static node *
BuildExternalAssigns (node *arg_node, info *arg_info)
{
    node *ids;
    node *newassigns = NULL;
    node *let;
    node *dummyavis;
    node *dummyvardec;
    node *idsavis;
    node *newids = NULL;
    node *scalars;

    DBUG_ENTER ();

    if ((N_let == NODE_TYPE (ASSIGN_STMT (arg_node)))
        && (N_ap == NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg_node))))
        && (AP_FUNDEF (LET_EXPR (ASSIGN_STMT (arg_node))) != INFO_FUNDEF (arg_info))
        && (FUNDEF_ISLACFUN (AP_FUNDEF (LET_EXPR (ASSIGN_STMT (arg_node)))))) {
        ids = LET_IDS (ASSIGN_STMT (arg_node));
        while (NULL != ids) {
            idsavis = IDS_AVIS (ids);
            scalars = AVIS_LACSO (idsavis);
            if (NULL != scalars) {
                DBUG_PRINT ("found scalarized result %s", AVIS_NAME (idsavis));
                newids = TCappendIds (newids,
                                      TCcreateIdsChainFromExprs (ARRAY_AELEMS (scalars)));
                let = TBmakeLet (TBmakeIds (idsavis, NULL), scalars);
                AVIS_LACSO (idsavis) = NULL;

                // Replace the current result with a dummy argument.
                dummyavis = TBmakeAvis (TRAVtmpVarName ("dummy"),
                                        TYcopyType (AVIS_TYPE (idsavis)));
                dummyvardec = TBmakeVardec (dummyavis, NULL);
                AVIS_DECL (dummyavis) = dummyvardec;
                AVIS_SSAASSIGN (dummyavis) = AVIS_SSAASSIGN (idsavis);
                INFO_VARDECS (arg_info)
                  = TCappendVardec (dummyvardec, INFO_VARDECS (arg_info));

                newassigns = TBmakeAssign (let, newassigns);
                AVIS_SSAASSIGN (idsavis) = newassigns;
                IDS_AVIS (ids) = dummyavis;

                // Replace all lacfun vardec AVIS_SHAPE references to
                // idsavis by its N_array equivalent.
                ReplaceAvisShapeByScalars (INFO_FUNDEF (arg_info), idsavis, scalars);
            }
            ids = IDS_NEXT (ids);
        }

        if (NULL != newassigns) {
            LET_IDS (ASSIGN_STMT (arg_node))
              = TCappendIds (newids, LET_IDS (ASSIGN_STMT (arg_node)));
            LET_IDS (ASSIGN_STMT (arg_node))
              = LFUcorrectSSAAssigns (LET_IDS (ASSIGN_STMT (arg_node)), arg_node);

            // Append new assigns
            ASSIGN_NEXT (arg_node) = TCappendAssign (newassigns, ASSIGN_NEXT (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function: node *BuildNarrayForAvisSonFromExprs( node *arg_node)
 *
 * description: Build N_array of scalarized names from N_exprs
 *              chain.
 *
 *              Right now, we only need this for AVIS_SHAPE,
 *              but if we ever propagate AVIS_MIN/MAX out of
 *              lacfuns, they will need the same treatment.
 *
 *****************************************************************************/
static node *
BuildNarrayForAvisSonFromExprs (node *arg_node)
{
    node *avis;
    node *narr;
    ntype *typ;

    DBUG_ENTER ();

    // First element will do as well as any for type/shape.
    avis = ID_AVIS (EXPRS_EXPR (arg_node));
    typ = TYcopyType (AVIS_TYPE (avis));

    narr = TCmakeVector (TYmakeAKS (TYcopyType (TYgetScalar (typ)), SHmakeShape (0)),
                         DUPdoDupTree (arg_node));

    DBUG_RETURN (narr);
}

/******************************************************************************
 *
 * function: node *BuildNarrayForAvisSonFromAssigns( arg_node, node *avis);
 *
 * description: Build N_array of scalarized names from N_assign
 *              chain.
 *
 *              Right now, we only need this for AVIS_SHAPE,
 *              but if we ever propagate AVIS_MIN/MAX out of
 *              lacfuns, they will need the same treatment.
 *
 * @param: arg_node - an N_assign chain of lacso scalars.
 * @param: resavis: the N_avis of the result.
 *         This is used to create the correct N_array result shape.
 *         This is needed because if have a non-vector result
 *         (e.g., int[2,3]), we have to build the N_array with
 *         that same shape.
 *
 *****************************************************************************/
static node *
BuildNarrayForAvisSonFromAssigns (node *arg_node, node *resavis)
{
    node *elems = NULL;
    node *narr;
    node *elavis;
    ntype *typ;
    shape *shp;

    DBUG_ENTER ();

    while (NULL != arg_node) {
        elavis = IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node)));
        elems = TCappendExprs (elems, TBmakeExprs (TBmakeId (elavis), NULL));
        arg_node = ASSIGN_NEXT (arg_node);
    }

    typ = TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (resavis))), SHcreateShape (0));
    shp = TYgetShape (AVIS_TYPE (resavis));
    narr = TBmakeArray (typ, SHcopyShape (shp), elems);

    DBUG_RETURN (narr);
}

/******************************************************************************
 *
 * function: node *ReplaceNidAvisShapeByNarray( node *avis)
 *
 * description: If this avis has an N_id for its AVIS_SHAPE,
 *              and that N_id has non-NULL AVIS_LACSO, replace
 *              our AVIS_SHAPE by the AVIS_LACSO N_array value,
 *              and free the old N_id and AVIS_LACSO.
 *
 * @result: the updated avis node, purely for clarity.
 *
 *****************************************************************************/
static node *
ReplaceNidAvisShapeByNarray (node *avis)
{
    node *narr;

    DBUG_ENTER ();

    if ((NULL != AVIS_SHAPE (avis)) && (N_id == NODE_TYPE (AVIS_SHAPE (avis)))) {
        DBUG_PRINT ("Found N_id AVIS_SHAPE in result element %s", AVIS_NAME (avis));
        narr = AVIS_LACSO (ID_AVIS (AVIS_SHAPE (avis)));
        if (NULL != narr) {
            DBUG_ASSERT (N_array == NODE_TYPE (narr), "Expected N_array AVIS_LACSO on "
                                                      "now-scalarized N_return element");
            DBUG_PRINT ("Replacing result AVIS_SHAPE( %s) by N_array", AVIS_NAME (avis));
            AVIS_LACSO (ID_AVIS (AVIS_SHAPE (avis))) = NULL;        // Kill N_array
            AVIS_SHAPE (avis) = FREEdoFreeNode (AVIS_SHAPE (avis)); // Kill N_id
            AVIS_SHAPE (avis) = narr;                               // Move N_array
        }
    }

    DBUG_RETURN (avis);
}

/******************************************************************************
 *
 * function: node *ReplaceNidsAvisShapeByNarray( node *ids)
 *
 * description: Replace all AVIS_SHAPES by Narrays in this IDS chain.
 *
 * @result: The updated N_ids chain.
 *
 *****************************************************************************/
static node *
ReplaceNidsAvisShapeByNarray (node *arg_node)
{
    node *ids;

    DBUG_ENTER ();

    ids = arg_node;
    while (NULL != ids) {
        IDS_AVIS (ids) = ReplaceNidAvisShapeByNarray (IDS_AVIS (ids));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LACSOmodule(node *arg_node, info *arg_info)
 *
 * description:
 *   prunes the syntax tree by only going into function defintions
 *
 *****************************************************************************/
node *
LACSOmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOfundef( node *arg_node, info *arg_info)
 *
 * @brief: We get called here two ways:
 *         The usual way is from TRAV.
 *         The other way is from an N_ap call of a lacfun. For this case,
 *         do not traverse other functions.
 *
 *         If we have new result elements, prefix them to our result list
 *
 *****************************************************************************/
node *
LACSOfundef (node *arg_node, info *arg_info)
{
    info *old_info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting to traverse %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    old_info = arg_info;
    arg_info = MakeInfo ();
    INFO_FUNDEF (arg_info) = arg_node;

    // Imports
    INFO_AP (arg_info) = INFO_AP (old_info);
    INFO_FDA (arg_info) = INFO_FDA (old_info);
    INFO_FINDINGRETURNIDS (arg_info) = INFO_FINDINGRETURNIDS (old_info);
    INFO_LETIDS (arg_info) = INFO_LETIDS (old_info);
    INFO_FDA (arg_info) = INFO_FDA (old_info);

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (NULL != INFO_VARDECS (arg_info)) {
        FUNDEF_VARDECS (arg_node)
          = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDECS (arg_node));
        INFO_VARDECS (arg_info) = NULL;
    }

    if (NULL != INFO_RETS (arg_info)) {
        FUNDEF_RETS (arg_node)
          = TCappendRet (INFO_RETS (arg_info), FUNDEF_RETS (arg_node));
        INFO_RETS (arg_info) = NULL;
    }

    // Exports
    if (NULL != INFO_CALLERVARDECS (arg_info)) {
        INFO_VARDECS (old_info)
          = TCappendVardec (INFO_CALLERVARDECS (arg_info), INFO_VARDECS (old_info));
        INFO_CALLERVARDECS (arg_info) = NULL;
    }

    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));

    if (NULL == INFO_AP (arg_info)) { // Normal traversals only
        DBUG_PRINT ("Traversing %s LOCALFUNS,FUNDEF_NEXT", FUNDEF_NAME (arg_node));
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOid( node *arg_node, info *arg_info)
 *
 * @brief We do work here when traversing a lacfun's
 *        N_return's N_exprs chain. Otherwise, we merely continue.
 *
 *****************************************************************************/
node *
LACSOid (node *arg_node, info *arg_info)
{
    shape *shp;
    shape *ravelshp;
    node *avis;
    node *funcond;
    node *newexprsthen;
    node *newexprselse;
    node *nassgn;
    node *narr;
    int len;

    DBUG_ENTER ();

    if (NULL != INFO_AP (arg_info) && (INFO_FINDINGRETURNIDS (arg_info))) {
        avis = ID_AVIS (arg_node);
        DBUG_PRINT ("inspecting return value: %s", AVIS_NAME (avis));

        /* Does this LACFUN result meet our criteria for LACS? */
        if ((TUshapeKnown (AVIS_TYPE (avis)) && (!AVIS_ISSCALARIZED (avis))
             && (!TYisAKV (AVIS_TYPE (avis))) && // No constants need apply.
             (TYgetDim (AVIS_TYPE (avis)) > 0))) {
            shp = TYgetShape (AVIS_TYPE (avis));
            len = SHgetUnrLen (shp);
            if ((len > 0) && (len <= global.minarray)) {
                DBUG_PRINT ("Scalarizing lacfun result: %s", AVIS_NAME (avis));
                AVIS_ISSCALARIZED (avis) = TRUE;
                global.optcounters.lacso_expr += 1;

                // We depend on strict structure of this part of the AST.
                funcond = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avis)));
                DBUG_ASSERT (N_funcond == NODE_TYPE (funcond),
                             "Did not find N_funcond at N_return");

                DBUG_ASSERT (0 != SHgetDim (shp), "Why scalarize a scalar?");
                ravelshp = SHcreateShape (1, SHgetUnrLen (shp));
                newexprsthen = LFUscalarizeArray (ID_AVIS (FUNCOND_THEN (funcond)),
                                                  &INFO_PREASSIGNSTHEN (arg_info),
                                                  &INFO_VARDECS (arg_info), ravelshp);

                if (NULL != INFO_FDA (arg_info)) { // May be CONDFUN
                    DBUG_PRINT ("attaching THEN N_array to AVIS_LACSO( %s)",
                                AVIS_NAME (avis));
                    DBUG_ASSERT (NULL == AVIS_LACSO (IDS_AVIS (INFO_FDA (arg_info))),
                                 "Design blunder");
                    narr = BuildNarrayForAvisSonFromExprs (newexprsthen);
                    AVIS_LACSO (IDS_AVIS (INFO_FDA (arg_info))) = narr;
                }

                // For the recursive loopfun call
                INFO_RECCALLERIDS (arg_info)
                  = TCappendIds (INFO_RECCALLERIDS (arg_info),
                                 TCcreateIdsChainFromExprs (newexprsthen));

                newexprselse = LFUscalarizeArray (ID_AVIS (FUNCOND_ELSE (funcond)),
                                                  &INFO_PREASSIGNSELSE (arg_info),
                                                  &INFO_VARDECS (arg_info), ravelshp);

                ravelshp = SHfreeShape (ravelshp);

                // Build new funconds for scalarized avis.
                nassgn = BuildFunconds (avis, newexprsthen, newexprselse, arg_info);

                DBUG_PRINT ("attaching N_array to AVIS_LACSO( %s)", AVIS_NAME (avis));
                DBUG_ASSERT (NULL == AVIS_LACSO (avis), "Design blunder");
                narr = BuildNarrayForAvisSonFromAssigns (nassgn, avis);
                AVIS_LACSO (avis) = narr;
                INFO_NEWFUNCONDS (arg_info)
                  = TCappendAssign (INFO_NEWFUNCONDS (arg_info), nassgn);

            } else {
                DBUG_PRINT ("not scalarized: %s", AVIS_NAME (ID_AVIS (arg_node)));
            }
        } else {
            DBUG_PRINT ("result: %s - shape unknown or scalar", AVIS_NAME (avis));
        }

        avis = ReplaceNidAvisShapeByNarray (avis);

        // Next recursive call result element.
        INFO_FDA (arg_info)
          = (NULL != INFO_FDA (arg_info)) ? IDS_NEXT (INFO_FDA (arg_info)) : NULL;

        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOassign( node *arg_node, info *arg_info)
 *
 * @brief If we return from looking at an N_return,
 *        we may have to insert new N_funconds BEFORE it.
 *
 *        If we return from an N_ap calling a lacfun, we may have
 *        to insert assigns for zero or more now-scalarized
 *        return values.
 *
 * @note  We do this bottom-up, as the N_return generates assigns
 *        that have to be inserted at N_cond and N_funcond nodes.
 *
 *****************************************************************************/
node *
LACSOassign (node *arg_node, info *arg_info)
{
    node *nxt;
    node *oldassign;

    DBUG_ENTER ();

    oldassign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    // Please new funconds immediately before the N_return.
    if ((NULL != INFO_NEWFUNCONDS (arg_info))
        && (N_return != NODE_TYPE (ASSIGN_STMT (arg_node)))) {
        nxt = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = TCappendAssign (INFO_NEWFUNCONDS (arg_info), nxt);
        INFO_NEWFUNCONDS (arg_info) = NULL;
    }

    // Place LOOPFUN preassigns immediately before the N_cond.
    if ((NULL != INFO_PREASSIGNSELSE (arg_info))
        && (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))) && (NULL != ASSIGN_NEXT (arg_node))
        && (N_cond == NODE_TYPE (ASSIGN_STMT (ASSIGN_NEXT (arg_node))))) {
        nxt = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = TCappendAssign (INFO_PREASSIGNSELSE (arg_info), nxt);
        INFO_PREASSIGNSELSE (arg_info) = NULL;
    }

    arg_node = BuildExternalAssigns (arg_node, arg_info);
    INFO_ASSIGN (arg_info) = oldassign;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOlet( node *arg_node, info *arg_info)
 *
 * @brief We pass the N_ap LHS to LACOap in INFO_LET.
 *        The scalarization code will mark elements of the LHS, then
 *        we can make this replacement, here and in LACSOassign:
 *
 *          resa, resb = lacfun(...);
 *        -->
 *          sa0, sa1, sb0, sb1, sb2 = lacfun();
 *          resa = [ sa0, sa1];
 *          resb = [ sb0, sb1, sb2];
 *
 *        We also build the N_array assign chain for the scalarized
 *        result array, remove the non-scalarized result from
 *        the result chain, and emit an N_array assign to
 *        rebuild the non-scalarized result from its new scalars.
 *
 *****************************************************************************/
node *
LACSOlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    arg_node = TRAVsons (arg_node, arg_info);
    INFO_LETIDS (arg_info) = NULL;

    // Prefix new results to recursive loopfun call
    if ((NULL != INFO_RECCALLERIDS (arg_info))
        && (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)))
        && (N_ap == NODE_TYPE (LET_EXPR (arg_node)))) {
        LET_IDS (arg_node)
          = TCappendIds (INFO_RECCALLERIDS (arg_info), LET_IDS (arg_node));
        INFO_RECCALLERIDS (arg_info) = NULL;

        LET_IDS (arg_node) = ReplaceNidsAvisShapeByNarray (LET_IDS (arg_node));

        LET_IDS (arg_node)
          = LFUcorrectSSAAssigns (LET_IDS (arg_node), INFO_ASSIGN (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOap( node *arg_node, info *arg_info)
 *
 * @brief If this a non-recursive call to a lacfun, we traverse
 *        it to find its return values. Ignore calls of lacfuns
 *        within a lacfun: They will be dealt with during the
 *        normal fundef chain traversal.
 *
 *        That will generate the required lacfun changes.
 *
 *        It will also generate side effects to be applied
 *        in the calling function.
 *
 *****************************************************************************/
node *
LACSOap (node *arg_node, info *arg_info)
{
    node *lacfundef;
    node *rec;

    DBUG_ENTER ();

    lacfundef = AP_FUNDEF (arg_node);

    /* non-recursive LACFUN calls only */
    if ((FUNDEF_ISLACFUN (lacfundef)) && (NULL == INFO_AP (arg_info))
        && (lacfundef != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("Found LACFUN: %s call from: %s", FUNDEF_NAME (lacfundef),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));

        DBUG_ASSERT (NULL == INFO_AP (arg_info), "Should not recurse in N_ap");
        DBUG_ASSERT (NULL == INFO_RECCALL (arg_info), "INFO_RECCALL not NULL");
        DBUG_ASSERT (NULL == INFO_NEWFUNCONDS (arg_info), "INFO_NEWFUNCONDS not NULL");

        INFO_AP (arg_info) = arg_node;
        rec = LFUfindRecursiveCallAssign (lacfundef);
        INFO_FDA (arg_info) = (NULL != rec) ? LET_IDS (ASSIGN_STMT (rec)) : NULL;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_FDA (arg_info) = NULL;
        INFO_AP (arg_info) = NULL;

        FUNDEF_RETURN (AP_FUNDEF (arg_node)) = LFUfindFundefReturn (AP_FUNDEF (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOfuncond( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
LACSOfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVsons (arg_node, arg_info);

    if (NULL != INFO_AP (arg_info) && FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("Got to funcond");
        if (NULL != INFO_PREASSIGNSTHEN (arg_info)) {
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOcond( node *arg_node, info *arg_info)
 *
 * @brief: If we scalarized an N_return element's legs,
 *         append them to the end of the N_cond's THEN/ELSE blocks.
 *
 *         And, on the way in, save the COND_COND. This should be
 *         safe, because there can only be one N_cond per lacfun.
 *
 *         If we are handling a LOOPFUN, then PREASSIGNSELSE goes
 *         before the N_cond, and the PREASSIGNSTHEN are not used.
 *
 *****************************************************************************/
node *
LACSOcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NULL != INFO_AP (arg_info) && FUNDEF_ISCONDFUN (INFO_FUNDEF (arg_info))) {
        if (NULL != INFO_PREASSIGNSTHEN (arg_info)) {
            DBUG_PRINT ("Appending to COND_THEN");
            BLOCK_ASSIGNS (COND_THEN (arg_node))
              = TCappendAssign (BLOCK_ASSIGNS (COND_THEN (arg_node)),
                                INFO_PREASSIGNSTHEN (arg_info));
            INFO_PREASSIGNSTHEN (arg_info) = NULL;
        }

        if (NULL != INFO_PREASSIGNSELSE (arg_info)) {
            DBUG_PRINT ("Appending to COND_ELSE");
            BLOCK_ASSIGNS (COND_ELSE (arg_node))
              = TCappendAssign (BLOCK_ASSIGNS (COND_ELSE (arg_node)),
                                INFO_PREASSIGNSELSE (arg_info));
            INFO_PREASSIGNSELSE (arg_info) = NULL;
        }
    }

    arg_node = TRAVsons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
LACSOprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *LACSOreturn( node *arg_node, info *arg_info)
 *
 * @brief If we are in a lacfun, called by an N_ap ( as opposed to
 *        a normal traversal of LOCALFUNS), decide which result
 *        elements are worthy of study, and deal with them.
 *
 *        When we find new result funcond elements:
 *          - prefix them to our return list
 *          - prefix the fundef's N_rets list with the new result types
 *          - FIXME if a LOOPFUN, do the same for the recursive call
 *          - FIXME prefix the N_ap return with the new results.
 *
 *****************************************************************************/
node *
LACSOreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NULL != INFO_AP (arg_info) && FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info))) {
        INFO_FINDINGRETURNIDS (arg_info) = TRUE;
        RETURN_EXPRS (arg_node) = TRAVopt (RETURN_EXPRS (arg_node), arg_info);
        if (NULL != INFO_FUNCONDEXPRS (arg_info)) {
            RETURN_EXPRS (arg_node)
              = TCappendExprs (INFO_FUNCONDEXPRS (arg_info), RETURN_EXPRS (arg_node));
            INFO_FUNCONDEXPRS (arg_info) = NULL;
        }
        INFO_FINDINGRETURNIDS (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *LACSOdoLoopAndCondScalarization( node *arg_node)
 *
 *   @brief This traversal eliminates arrays within loops and conditionals
 *          for one function or module.
 *   @param arg_node
 *   @return modified AST.
 *
 *****************************************************************************/
node *
LACSOdoLoopAndCondScalarization (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_lacso);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
