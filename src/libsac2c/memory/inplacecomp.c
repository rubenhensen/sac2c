/** <!--********************************************************************-->
 *
 * @defgroup ipc Inplace Computation
 *
 * @ingroup mm
 *
 * @{
 *     Concept:
 *    
 *     This traversal tries to propagate upwards suballoc operations to
 *     get rid of copy operations whenever possible
 *
 *     We look for patterns like this:
 *
 *     a = with {
 *           ( lb <= iv < ub) {
 *              assigns;
 *              res_mem = alloc (rc, def-shp);
 *              res = fill (expr, res_mem);  || ...res... = with {}:
 *                                           || (..., gen/modarray(res_mem), ...);
 *              inner_mem = suballoc (a_mem, iv, d);
 *              inner = fill (copy (res), inner_mem);
 *           } : inner;
 *         } : genarray( shp, def, a_mem);
 *
 *     and replace it with
 *
 *     a = with {
 *           ( lb <= iv < ub) {
 *              IPC [[ assigns ]] ;
 *              res_mem = suballoc (a_mem, iv, d);         <<<< change!
 *              res = fill (expr, res_mem); || ...
 *              //  inner_mem = suballoc (a_mem, iv, d);   <<<< delete!
 *              //  inner = fill (copy (res), inner_mem);  <<<< delete!
 *           } : res;                                      <<<< change!
 *         } : genarray( shp, def, a_mem);
 *     
 *     We can also deal with res_mem being a reuse of some other array!
 *     In that case, we simply follow the reused identifier further up.
 *
 *     Implementation:
 *
 *     The entire search and replacement is implemented in the function
 *     HandleBlock:
 *     It starts from the return expression of WL code in search for
 *     the above pattern. Once a suitable candidate is found (res_mem from
 *     above), its defining assignment is stored in INFO_LASTSAFE!
 *     If that is the case ModifyBlock is being called to actually perform
 *     the changes explained above.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file inplacecomp.c
 *
 * Prefix: IPC
 *
 *****************************************************************************/
#include "inplacecomp.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "IPC"
#include "debug.h"

#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "pattern_match.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *lhs;
    lut_t *reuselut;
    node *predavis;
    node *memavis;
    node *rcavis;

    bool ok;
    node *nouse;
    node *noap;
    node *lastsafe;
    bool changed;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_REUSELUT(n) ((n)->reuselut)
#define INFO_PREDAVIS(n) ((n)->predavis)
#define INFO_MEMAVIS(n) ((n)->memavis)
#define INFO_RCAVIS(n) ((n)->rcavis)
#define INFO_OK(n) ((n)->ok)
#define INFO_NOUSE(n) ((n)->nouse)
#define INFO_NOAP(n) ((n)->noap)
#define INFO_LASTSAFE(n) ((n)->lastsafe)
#define INFO_CHANGED(n) ((n)->changed)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_LHS (result) = NULL;
    INFO_REUSELUT (result) = NULL;
    INFO_PREDAVIS (result) = NULL;
    INFO_MEMAVIS (result) = NULL;
    INFO_RCAVIS (result) = NULL;
    INFO_OK (result) = FALSE;
    INFO_NOUSE (result) = NULL;
    INFO_NOAP (result) = NULL;
    INFO_LASTSAFE (result) = NULL;
    INFO_CHANGED (result) = FALSE;

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

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IPCdoInplaceComputation( node *syntax_tree)
 *
 * @brief starting point of Inplace Computation traversal
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
IPCdoInplaceComputation (node *syntax_tree)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Inplace Computation  optimization...");

    TRAVpush (TR_ipc);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("Inplace Computation optimization complete.");

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *copyOrArray(node *val, int *depth)
 *
 * @brief expects to obtain either 
 *        F_copy ( N_id)     or
 *        [...[ N_id ]...]   (all arrays are singletons!)
 *        as first argument.
 * @param val 
 * @return returns N_id node and increments depth by the nesting of the N_array
 *         nodes (if val is an N_array)
 *
 *****************************************************************************/

static node *
copyOrArray (node *val, int *depth)
{
    DBUG_ENTER ();

    if (NODE_TYPE (val) == N_prf) {
        DBUG_ASSERT (PRF_PRF (val) == F_copy, "Expected copy prf");
        val = PRF_ARG1 (val);
    } else if (NODE_TYPE (val) == N_array) {
        while (NODE_TYPE (val) == N_array) {
            DBUG_ASSERT (NODE_TYPE (ARRAY_AELEMS (val)) == N_exprs, "Broken ast?");
            DBUG_ASSERT (EXPRS_NEXT (ARRAY_AELEMS (val)) == NULL,
                         "Can not perform ipc on [ a, b]");
            (*depth)++;
            val = EXPRS_EXPR (ARRAY_AELEMS (val));
        }
    } else {
        DBUG_UNREACHABLE ("Unexpected node");
    }

    DBUG_ASSERT (NODE_TYPE (val) == N_id, "Unexpected node expected an N_id");

    DBUG_RETURN (val);
}

/** <!--********************************************************************-->
 *
 * @fn node *idArray(node *arg_node)
 *
 * @brief accepts arbitrary nodes. Returns true, iff the argument is either
 *        N_id     or
 *        [...[N_id]...]    (all arrays are singletons!)
 *
 *****************************************************************************/

static bool
idArray (node *array)
{
    bool ok = TRUE;
    DBUG_ENTER ();

    while (NODE_TYPE (array) == N_array) {
        DBUG_ASSERT (NODE_TYPE (ARRAY_AELEMS (array)) == N_exprs, "Broken ast?");
        if (NULL != EXPRS_NEXT (ARRAY_AELEMS (array))) {
            ok = FALSE;
        }
        array = EXPRS_EXPR (ARRAY_AELEMS (array));
    }

    if (NODE_TYPE (array) != N_id) {
        ok = FALSE;
    }

    DBUG_RETURN (ok);
}
/** <!--********************************************************************-->
 *
 * @fn node *removeArrayIndirectionFromSuballoc( node *suballoc, int depth)
 *
 * @brief Remove the array indirection from suballoc.
 *        It expects to obtain
 *        F_suballoc (arg1, ..., argn)
 *        If (n >= 4), it expects either
 *        suballoc ( _, _, _, F_shape ( F_genarray ( array, _)))   (pat) or
 *        suballoc ( _, _, _, F_shape ( array))                    (pat2)
 *        Then, we elide depth-many elements of that array!
 *
 *****************************************************************************/
static node *
removeArrayIndirectionFromSuballoc (node *suballoc, int depth)
{
    int one = 1, three = 3;
    DBUG_ENTER ();

    if ((depth > 0) && (TCcountExprs (PRF_ARGS (suballoc))) >= 4) {
        node *array, *exprs;
        pattern *pat, *pat2;

        pat = PMprf (1, PMAisPrf (F_suballoc), 2, PMskipN (&three, 0),
                     PMprf (1, PMAisPrf (F_shape_A), 1,
                            PMprf (1, PMAisPrf (F_genarray), 2,
                                   PMarray (1, PMAgetNode (&array), 0),
                                   PMskipN (&one, 0))));

        pat2 = PMprf (1, PMAisPrf (F_suballoc), 2, PMskipN (&three, 0),
                      PMarray (1, PMAgetNode (&array), 0));

        if (PMmatchFlat (pat, suballoc) || PMmatchFlat (pat2, suballoc)) {
        }

        pat = PMfree (pat);
        pat2 = PMfree (pat2);

        DBUG_ASSERT (NODE_TYPE (array) == N_array, "Can not remove array indirection if "
                                                   "I can not find the array");

        exprs = ARRAY_AELEMS (array);

        while (depth > 0) {
            exprs = FREEdoFreeNode (exprs);
            depth--;
        }

        ARRAY_AELEMS (array) = exprs;
    }

    DBUG_RETURN (suballoc);
}

/** <!--********************************************************************-->
 *
 * @fn void ModifyBlock (node *alloc_ass, node *suballoc_ass, node *fill_ass,
 *                       node *new_ret, node *old_rets)
 *
 * @brief performs the 4 modifications on the assignment chain staring with 
 *        the assignment that is to be changed into a suballoc:
 *        1) change alloc_ass RHS into ia copy of suballoc_ass
 *        2) delete suballoc_ass
 *        3) delete fill_ass
 *        4) change the ret in old_rets into new_ret
 *
 *        This also does some weird stuff that I (sbs) do not understand:
 *        - it strips suballoc_ass of potential extra arguments?!
 *        - in case the allocated value is a scalar, it turns it into a 
 *          one element vector?????!
 *
 *****************************************************************************/
static void
ModifyBlock (node *alloc_ass, node *suballoc_ass, node *fill_ass,
                  node *new_ret, node *old_rets, int depth)
{
    DBUG_ENTER ();

    int changed = 0;
    ntype *type = NULL;
    /*
     * Replace some alloc or reuse or alloc_or_reuse with suballoc
     */
    DBUG_PRINT ("  changing allocation of '%s` to suballoc",
                IDS_NAME (ASSIGN_LHS (alloc_ass)));
    ASSIGN_RHS (alloc_ass) = FREEdoFreeNode (ASSIGN_RHS (alloc_ass));
    ASSIGN_RHS (alloc_ass) = removeArrayIndirectionFromSuballoc (
                               DUPdoDupNode (ASSIGN_RHS (suballoc_ass)),
                               depth);

    /*
     * Are we suballocing a scaler?
     * If so make it a [1] array.
     */
    type = AVIS_TYPE (IDS_AVIS (ASSIGN_LHS (alloc_ass)));
    if (TUisScalar (type)) {
        AVIS_TYPE (IDS_AVIS (ASSIGN_LHS (alloc_ass)))
          = TYmakeAKS (TYgetScalar (type), SHcreateShape (1, 1));
    }

    /*
     * Replace CEXPR
     */
    EXPRS_EXPR (old_rets) = FREEdoFreeNode (EXPRS_EXPR (old_rets));
    EXPRS_EXPR (old_rets) = DUPdoDupNode (new_ret);

    /*
     * Remove old suballoc/fill(copy) combination
     */
    while (alloc_ass != NULL) {
        if ((ASSIGN_NEXT (alloc_ass) == suballoc_ass)
            || (ASSIGN_NEXT (alloc_ass) == fill_ass)) {
            ASSIGN_NEXT (alloc_ass) = FREEdoFreeNode (ASSIGN_NEXT (alloc_ass));
            changed++;
        } else {
            alloc_ass = ASSIGN_NEXT (alloc_ass);
        }
    }
    DBUG_ASSERT (changed==2, "ModifyBlock failed; found %d of 2 assignments"
                             " to delete", changed);
    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *HandleBlock(node *arg_node)
 *
 * @brief The main part of this traversal
 *        Are rets performing in-place computation?
 *
 *****************************************************************************/
static node *
HandleBlock (node *block, node *rets, info *arg_info)
{
    int depth = 0;
    DBUG_ENTER ();

    while (rets != NULL) {
        node *cid;
        node *wlass;
        node *rhs;
        node *mem;
        node *val;
        node *cval;
        node *memass;
        node *memop;
        node *avis;
        bool isinblock;
        node *assigns;

        cid = EXPRS_EXPR (rets);
        wlass = AVIS_SSAASSIGN (ID_AVIS (cid));

        if (wlass != NULL) {
            rhs = ASSIGN_RHS (wlass);

            if ((NODE_TYPE (rhs) == N_prf) && (PRF_PRF (rhs) == F_fill)
                && ((((NODE_TYPE (PRF_ARG1 (rhs)) == N_prf)
                      && (PRF_PRF (PRF_ARG1 (rhs)) == F_copy)))
                    || ((NODE_TYPE (PRF_ARG1 (rhs)) == N_array)
                        && idArray (PRF_ARG1 (rhs))))) {
                /*
                 * Search for suballoc situation
                 * {...
                 *   a  = ...
                 *   m' = suballoc( A, iv);
                 *   m  = fill( copy( a), m');  <- checked so far!
                 * }: m
                 *
                 * or (as appears in with3 loops:
                 *
                 *   a  = ...
                 *   m' = suballoc( A, iv);
                 *   m  = fill( [ a], m');  <- checked so far!
                 */
                val = PRF_ARG1 (rhs);                      // copy(a) or [a]
                mem = PRF_ARG2 (rhs);                      // m'
                cval = copyOrArray (val, &depth);          // a
                avis = ID_AVIS (cval);                     // avis of a
                memass = AVIS_SSAASSIGN (ID_AVIS (mem));   // m' = <suballoc?>
                memop = LET_EXPR (ASSIGN_STMT (memass));   // <suballoc?>

                /*
                 * a must be assigned inside the current block in order to
                 * move suballoc in front of a.
                 */
                isinblock = FALSE;
                if (AVIS_SSAASSIGN (avis) != NULL) {
                    assigns = BLOCK_ASSIGNS (block);
                    while (assigns != NULL) {
                        if (assigns == AVIS_SSAASSIGN (avis)) {
                            isinblock = TRUE;
                            break;
                        }
                        assigns = ASSIGN_NEXT (assigns);
                    }
                }

                if ((isinblock) && (PRF_PRF (memop) == F_suballoc)) {
                    /*
                     * Situation recognized, find highest position for suballoc
                     */
                    DBUG_PRINT ("found  %s = suballoc( %s /*outer*/, %s /*idx*/, %d /*depth*/);",
                                IDS_NAME (LET_IDS (ASSIGN_STMT (memass))),
                                ID_NAME (PRF_ARG1 (memop)),
                                ID_NAME (PRF_ARG2 (memop)),
                                NUM_VAL (PRF_ARG3 (memop)));
                    DBUG_PRINT ("       %s = fill( copy (%s), %s);",
                                ID_NAME (cid),
                                AVIS_NAME (avis),
                                IDS_NAME (LET_IDS (ASSIGN_STMT (memass))));

                    node *def = AVIS_SSAASSIGN (avis);
                    INFO_LASTSAFE (arg_info) = NULL;
                    INFO_NOUSE (arg_info)
                      = (node *)LUTsearchInLutPp (INFO_REUSELUT (arg_info),
                                                  ID_AVIS (PRF_ARG1 (memop)));
                    if (INFO_NOUSE (arg_info) == ID_AVIS (PRF_ARG1 (memop))) {
                        INFO_NOUSE (arg_info) = NULL;
                    }
                    INFO_NOAP (arg_info) = NULL;

                    /*
                     * BETWEEN def and LASTSAFE:
                     *
                     * NOUSE must not be used at all!!!
                     * NOAP must not be used in function applications
                     */
                    INFO_OK (arg_info) = TRUE;

                    while (INFO_OK (arg_info)) {
                        DBUG_PRINT ("  checking definition of '%s`:",
                                    AVIS_NAME (avis));
                        TRAVpush (TR_ipch);
                        ASSIGN_NEXT (def) = TRAVdo (ASSIGN_NEXT (def), arg_info);
                        TRAVpop ();

                        if (INFO_OK (arg_info)) {
                            node *defrhs = ASSIGN_RHS (def);
                            node *withop, *ids;
                            switch (NODE_TYPE (defrhs)) {
                            case N_prf:
                                if (PRF_PRF (defrhs) == F_fill) {
                                    node *memass
                                      = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (defrhs)));
                                    node *memop = ASSIGN_RHS (memass);
                                    if ((PRF_PRF (memop) == F_alloc)
                                        || (PRF_PRF (memop) == F_reuse)
                                        || (PRF_PRF (memop) == F_alloc_or_reuse)) {
                                        INFO_LASTSAFE (arg_info) = memass;
                                        if (PRF_PRF (memop) == F_reuse) {
                                            DBUG_PRINT ("  found  %s = reuse (%s);",
                                                        ID_NAME (PRF_ARG2 (defrhs)),
                                                        ID_NAME (PRF_ARG1 (memop)));
                                            avis = ID_AVIS (PRF_ARG1 (memop));
                                            def = AVIS_SSAASSIGN (
                                              ID_AVIS (PRF_ARG1 (memop)));
                                            INFO_NOAP (arg_info)
                                              = ID_AVIS (PRF_ARG1 (memop));
                                        } else {
                                            DBUG_PRINT ("  found  %s = alloc / alloc_or_reuse (...);",
                                                        ID_NAME (PRF_ARG2 (defrhs)));
                                            INFO_OK (arg_info) = FALSE;
                                        }
                                        DBUG_PRINT ("         %s = fill ( _, %s);",
                                                    AVIS_NAME (avis),
                                                    ID_NAME (PRF_ARG2 (defrhs)));
                                    } else {
                                        INFO_OK (arg_info) = FALSE;
                                    }
                                } else {
                                    INFO_OK (arg_info) = FALSE;
                                }
                                break;

                            case N_with:
                            case N_with2:
                            case N_with3:
                                withop = WITH_OR_WITH2_OR_WITH3_WITHOP (defrhs);
                                ids = ASSIGN_LHS (def);
                                while (IDS_AVIS (ids) != avis) {
                                    ids = IDS_NEXT (ids);
                                    withop = WITHOP_NEXT (withop);
                                }
                                if ((NODE_TYPE (withop) == N_genarray)
                                    || (NODE_TYPE (withop) == N_modarray)) {
                                    node *memass
                                      = AVIS_SSAASSIGN (ID_AVIS (WITHOP_MEM (withop)));
                                    node *memop = ASSIGN_RHS (memass);
                                    if ((PRF_PRF (memop) == F_alloc)
                                        || (PRF_PRF (memop) == F_reuse)
                                        || (PRF_PRF (memop) == F_alloc_or_reuse)) {
                                        INFO_LASTSAFE (arg_info) = memass;
                                        if (PRF_PRF (memop) == F_reuse) {
                                            DBUG_PRINT ("  found  %s = reuse (%s);",
                                                        ID_NAME (WITHOP_MEM (withop)),
                                                        ID_NAME (PRF_ARG1 (memop)));
                                            avis = ID_AVIS (PRF_ARG1 (memop));
                                            def = AVIS_SSAASSIGN (
                                              ID_AVIS (PRF_ARG1 (memop)));
                                            INFO_NOAP (arg_info)
                                              = ID_AVIS (PRF_ARG1 (memop));
                                        } else {
                                            DBUG_PRINT ("  found  %s = alloc / alloc_or_reuse (...);",
                                                        ID_NAME (WITHOP_MEM (withop)));
                                            INFO_OK (arg_info) = FALSE;
                                        }
                                        DBUG_PRINT ("         ...,%s,... = with {...} :(..., gen/modarray(%s), ...);",
                                                    AVIS_NAME (avis),
                                                    ID_NAME (WITHOP_MEM (withop)));
                                    } else {
                                        INFO_OK (arg_info) = FALSE;
                                    }
                                } else {
                                    INFO_OK (arg_info) = FALSE;
                                }
                                break;

                            default:
                                INFO_OK (arg_info) = FALSE;
                                break;
                            }
                        }
                    }

                    if (INFO_LASTSAFE (arg_info) != NULL) {
                        ModifyBlock (INFO_LASTSAFE (arg_info), // alloc_ass to be modified
                                     memass,                   // original suballoc_ass
                                     wlass,                    // original fill_ass
                                     cval,                     // originally copied now returned
                                     rets,                     // rets holding the old returned
                                     depth);                   // weird depth value....
                        INFO_CHANGED (arg_info) = TRUE;
                    }
                }
                break;
            }
        }
        rets = EXPRS_NEXT (rets);
    }

    DBUG_RETURN ((node *)NULL);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IPCap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * CONDFUNs are traversed in order of appearance
     */
    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {

        if (STRsub ("ReuseCond", FUNDEF_NAME (AP_FUNDEF (arg_node)))) {
            /*
             * Transform predavis, memavis and rcavis before traversing REUSECOND
             */
            node *funargs, *apargs;
            funargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
            apargs = AP_ARGS (arg_node);

            while (apargs != NULL) {

                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_PREDAVIS (arg_info)) {
                    INFO_PREDAVIS (arg_info) = ARG_AVIS (funargs);
                }
                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_MEMAVIS (arg_info)) {
                    INFO_MEMAVIS (arg_info) = ARG_AVIS (funargs);
                }
                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_RCAVIS (arg_info)) {
                    INFO_RCAVIS (arg_info) = ARG_AVIS (funargs);
                }

                apargs = EXPRS_NEXT (apargs);
                funargs = ARG_NEXT (funargs);
            }
        }
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IPCcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPCcond (node *arg_node, info *arg_info)
{
    lut_t *oldlut;

    DBUG_ENTER ();

    oldlut = INFO_REUSELUT (arg_info);
    INFO_REUSELUT (arg_info) = LUTduplicateLut (oldlut);

    if ((NODE_TYPE (COND_COND (arg_node)) == N_id)
        && (ID_AVIS (COND_COND (arg_node)) == INFO_PREDAVIS (arg_info))) {
        /*
         * b = reuse( a);
         *
         * Insert (memavis, rcavis) into REUSELUT
         */
        LUTinsertIntoLutP (INFO_REUSELUT (arg_info), INFO_MEMAVIS (arg_info),
                           INFO_RCAVIS (arg_info));
    }
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_REUSELUT (arg_info) = LUTremoveLut (INFO_REUSELUT (arg_info));
    INFO_REUSELUT (arg_info) = oldlut;

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IPCcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Traverse into CBLOCK in order to apply datareuse in nested with-loops
     */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    HandleBlock (CODE_CBLOCK (arg_node), CODE_CEXPRS (arg_node), arg_info);

    /*
     * Traverse next code
     */
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IPCrange( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPCrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

    INFO_CHANGED (arg_info) = TRUE;
    while (INFO_CHANGED (arg_info)) {
        INFO_CHANGED (arg_info) = FALSE;
        HandleBlock (RANGE_BODY (arg_node), RANGE_RESULTS (arg_node), arg_info);
    }

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IPCfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * CONDFUNs may only be traversed from AP-nodes
     */
    if ((!FUNDEF_ISCONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info = MakeInfo (arg_node);

            if (arg_info != NULL) {
                INFO_PREDAVIS (info) = INFO_PREDAVIS (arg_info);
                INFO_MEMAVIS (info) = INFO_MEMAVIS (arg_info);
                INFO_RCAVIS (info) = INFO_RCAVIS (arg_info);
            }

            INFO_REUSELUT (info) = LUTgenerateLut ();

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            INFO_REUSELUT (info) = LUTremoveLut (INFO_REUSELUT (info));

            info = FreeInfo (info);
        }
    }

    /*
     * Traverse next fundef
     */
    if (arg_info == NULL) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IPClet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IPCprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_reuse:
        /*
         * b = reuse( a);
         *
         * Insert (b, a) into REUSELUT
         */
        LUTinsertIntoLutP (INFO_REUSELUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                           ID_AVIS (PRF_ARG1 (arg_node)));
        break;

    case F_fill:
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_prf) {
            /*
             * c = fill( PRF, b);
             */
            node *prf = PRF_ARG1 (arg_node);
            switch (PRF_PRF (prf)) {

            case F_isreused:
                /*
                 * c = fill( isreused( mem, rc), c')
                 *
                 * put ( c, mem, rc) into ( predavis, memavis, rcavis)
                 */
                INFO_PREDAVIS (arg_info) = IDS_AVIS (INFO_LHS (arg_info));
                INFO_MEMAVIS (arg_info) = ID_AVIS (PRF_ARG1 (prf));
                INFO_RCAVIS (arg_info) = ID_AVIS (PRF_ARG2 (prf));
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/******************************************************************************
 *
 * @name IPC helper traversal
 *
 * @{
 *     This traversal checks wether the N_avis given in
 *       INFO_NOUSE or the N_avis given in 
 *       INFO_NOAP appear on the RHS of the given N_assign chain.
 *     The search for INFO_NOAP is restricted to udf function arguments,
 *     and that of INFO_NOUSE is restricted to uses outside of udf
 *     function arguments!
 *     if any of these two is found, INFO_OK is set to FALSE!
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IPCHap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPCHap (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ();

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);

        tmp = INFO_NOUSE (arg_info);
        INFO_NOUSE (arg_info) = INFO_NOAP (arg_info);
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
        INFO_NOUSE (arg_info) = tmp;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IPCHassign( node *arg_node, info *arg_info)
 *
 * @brief top-down traversal
 *
 *****************************************************************************/
node *
IPCHassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (arg_node != INFO_LASTSAFE (arg_info)) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IPCHid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IPCHid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ID_AVIS (arg_node) == INFO_NOUSE (arg_info)) {
        INFO_OK (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Inplace Computation -->
 *****************************************************************************/

#undef DBUG_PREFIX
