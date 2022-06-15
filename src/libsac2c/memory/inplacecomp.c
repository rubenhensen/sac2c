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
 *     HandleBlock. It starts from a WL return expression and follows the definition
 *     chain upwards. First it checks whether we have a pattern like this:
 *
 *     {...
 *       a  = ...
 *       m' = suballoc( A, iv);
 *       m  = fill( copy( a), m');  <- checked so far!
 *     }: m
 *
 *     or (as appears in with3 loops):
 *
 *       a  = ...
 *       m' = suballoc( A, iv);
 *       m  = fill( [ a], m');  <- checked so far!
 *
 *     This is implemented in IsSuballocFill.
 *
 *     Then, we try to get rid of the copy instruction by looking at the
 *     allocation for the variable to be copied: 'a`.
 *
 *     Here, we are looking for
 *
 *        c  = ...
 *        m' = alloc / reuse / alloc_or_reuse (c);
 *        a  = fill( copy( b), m');
 *
 *      or :
 *
 *        c  = ...
 *        m' = alloc / reuse / alloc_or_reuse (c);
 *        ..., a, ...  = = with {...} :(..., gen/modarray(m`), ...);
 *
 *     This is implemented in IsAllocReuseFill.
 *     However, this search needs to be applied recusively iff we have a reuse here!
 *     This loop is implemented in FindSuballocAlternative.
 *
 *     Finally, if a suitable alternative is found, ModifyBlock make the necessary
 *     modifications in the WL operation block.
 *
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
 *                       node *new_ret_avis, node *old_rets)
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
                  node *new_ret_avis, node *old_rets, int depth)
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
    EXPRS_EXPR (old_rets) = TBmakeId (new_ret_avis);

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
 * @fn bool IsAllocReuseFill (node *copy_avis, node **new_copy_avis, node **mem_ass)
 *
 * @brief check whether the given copy_avis points to an assignment as the one
 *        to "a" in the following pattern:
 *
 *    c  = ...
 *    m' = alloc / reuse / alloc_or_reuse (c);
 *    a  = fill( copy( b), m');
 *
 *  or :
 *
 *    c  = ...
 *    m' = alloc / reuse / alloc_or_reuse (c);
 *    ..., a, ...  = = with {...} :(..., gen/modarray(m`), ...);
 *
 *  if found, it returns true and it fills the other return values
 *     *new_copy_avis  =>  N_avis of "c"   iff  reuse   NULL otherwise
 *     *mem_ass        =>  "m' = alloc / reuse/ alloc_or_reuse ( ...)
 *****************************************************************************/
bool IsAllocReuseFill (node *copy_avis, node **new_copy_avis, node **mem_ass)
{
    DBUG_ENTER ();

    bool found = FALSE;
    node *rhs;
    node *mem_ass_l = NULL;
    node *mem_op = NULL;
    node *withop = NULL;
    node *ids;

    rhs = ASSIGN_RHS (AVIS_SSAASSIGN (copy_avis));
    switch (NODE_TYPE (rhs)) {
    case N_prf:
        if (PRF_PRF (rhs) == F_fill) {
            mem_ass_l = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (rhs)));
            mem_op = ASSIGN_RHS (mem_ass_l);
        }
        break;

    case N_with:
    case N_with2:
    case N_with3:
        withop = WITH_OR_WITH2_OR_WITH3_WITHOP (rhs);
        ids = ASSIGN_LHS (AVIS_SSAASSIGN (copy_avis));
        while (IDS_AVIS (ids) != copy_avis) {
            ids = IDS_NEXT (ids);
            withop = WITHOP_NEXT (withop);
        }
        if ((NODE_TYPE (withop) == N_genarray)
            || (NODE_TYPE (withop) == N_modarray)) {
            mem_ass_l = AVIS_SSAASSIGN (ID_AVIS (WITHOP_MEM (withop)));
            mem_op = ASSIGN_RHS (mem_ass_l);
        }
        break;
    default:
        break;
    }

    if ((mem_op != NULL) && 
        ((PRF_PRF (mem_op) == F_alloc)
         || (PRF_PRF (mem_op) == F_reuse)
         || (PRF_PRF (mem_op) == F_alloc_or_reuse))) {
        found = TRUE;
        *mem_ass = mem_ass_l;
        if (PRF_PRF (mem_op) == F_reuse) {
            DBUG_PRINT ("  found  %s = reuse (%s);",
                        IDS_NAME (ASSIGN_LHS (*mem_ass)),
                        ID_NAME (PRF_ARG1 (mem_op)));
            *new_copy_avis = ID_AVIS (PRF_ARG1 (mem_op));
        } else {
            DBUG_PRINT ("  found  %s = alloc / alloc_or_reuse (...);",
                        IDS_NAME (ASSIGN_LHS (*mem_ass)));
            *new_copy_avis = NULL;
        }

        if (withop == NULL) {
            DBUG_PRINT ("         %s = fill ( _, %s);",
                        AVIS_NAME (copy_avis),
                        ID_NAME (PRF_ARG2 (rhs)));
        } else {
            DBUG_PRINT ("         ...,%s,... = with {...} :(..., gen/modarray(%s), ...);",
                        AVIS_NAME (copy_avis),
                        ID_NAME (WITHOP_MEM (withop)));
        }
    }
    DBUG_RETURN (found);
}



/** <!--********************************************************************-->
 *
 * @fn bool IsSuballocFill (node * block, node *avis,
 *                          node **copy_avis, node **mem_ass, int *depth)
 *
 * @brief check whether the given wl_ass points to an assignment as the one
 *        to "m" in the following pattern:
 *  {...
 *    a  = ...
 *    m' = suballoc( A, iv);
 *    m  = fill( copy( a), m');
 *  }: m
 *
 *  or (as appears in with3 loops):
 *
 *    a  = ...
 *    m' = suballoc( A, iv);
 *    m  = fill( [ a], m');
 *
 *  if found, it returns true and it fills the other return values
 *     *copy_avis  =>  N_avis of "a"
 *     *mem_ass    =>  "m' = suballoc( ...);"
 *     *depth      =>  0 / nesting depth of singleton array in "fill" (with3)
 *****************************************************************************/
bool IsSuballocFill (node *block, node *avis,
                     node **copy_avis, node **mem_ass, int *depth)
{
    DBUG_ENTER ();

    node *rhs;
    node *assigns;
    bool isinblock = FALSE;

    rhs = ASSIGN_RHS (AVIS_SSAASSIGN (avis));

    if ((NODE_TYPE (rhs) == N_prf) && (PRF_PRF (rhs) == F_fill)
        && ((((NODE_TYPE (PRF_ARG1 (rhs)) == N_prf)
              && (PRF_PRF (PRF_ARG1 (rhs)) == F_copy)))
            || ((NODE_TYPE (PRF_ARG1 (rhs)) == N_array)
                && idArray (PRF_ARG1 (rhs))))) {

        *depth = 0;
        *copy_avis = ID_AVIS (copyOrArray (PRF_ARG1 (rhs), depth));
        *mem_ass = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (rhs)));

        /*
         * copy_avis must be assigned inside the current block in order to
         * move suballoc in front of a.
         */
        if (AVIS_SSAASSIGN (*copy_avis) != NULL) {
            assigns = BLOCK_ASSIGNS (block);
            while (assigns != NULL) {
                if (assigns == AVIS_SSAASSIGN (*copy_avis)) {
                    isinblock = TRUE;
                    break;
                }
                assigns = ASSIGN_NEXT (assigns);
            }
        }
        isinblock = isinblock
                    && (PRF_PRF (LET_EXPR (ASSIGN_STMT (*mem_ass))) == F_suballoc);
    }
    DBUG_RETURN (isinblock);
}

/** <!--********************************************************************-->
 *
 * @fn node *FindSuballocAlternative (node *copy_avis, node *sub_ass,
 *                                    info* arg_info)
 *
 * @brief try to push the suballoc further up by inspecting the copy_avis.
 *        If its memory is locally allocated (IsAllocReuseFill) update
 *        copy_ass and sub_ass accordingly (done by IsAllocReuseFill),
 *        try pushing recursively further, provided the new allocation
 *        happens through reuse which is signalled by a non-NULL new copy_info.
 *
 *****************************************************************************/
node *FindSuballocAlternative (node *copy_avis, node *sub_ass, info* arg_info)
{
    DBUG_ENTER ();

    bool found;
    node *def;
    node *suballoc;
#ifndef DBUG_OFF
    node *old_sub_ass = sub_ass;
#endif

    suballoc = ASSIGN_RHS (sub_ass);

    INFO_LASTSAFE (arg_info) = NULL;
    INFO_NOUSE (arg_info) = (node *)LUTsearchInLutPp (
                                        INFO_REUSELUT (arg_info),
                                        ID_AVIS (PRF_ARG1 (suballoc)));
    if (INFO_NOUSE (arg_info) == ID_AVIS (PRF_ARG1 (suballoc))) {
        INFO_NOUSE (arg_info) = NULL;
        DBUG_PRINT ("  NOUSE set to NULL.");
    } else {
        DBUG_PRINT ("  NOUSE set to %s.", AVIS_NAME (INFO_NOUSE (arg_info)));
    }

    INFO_OK (arg_info) = TRUE;
    while (INFO_OK (arg_info)) {
        DBUG_PRINT ("  checking definition of '%s`:",
                    AVIS_NAME (copy_avis));
        def = AVIS_SSAASSIGN (copy_avis);
        INFO_NOAP (arg_info) = copy_avis;
        DBUG_PRINT ("  NOAP set to %s.", AVIS_NAME (INFO_NOAP (arg_info)));
        /*
         * BETWEEN def and LASTSAFE:
         *
         * NOUSE must not be used at all!!!
         * NOAP must not be used in function applications
         */
        TRAVpush (TR_ipch);
        ASSIGN_NEXT (def) = TRAVdo (ASSIGN_NEXT (def), arg_info);
        TRAVpop ();

        if (INFO_OK (arg_info)) {
            found = IsAllocReuseFill (copy_avis, &copy_avis, &sub_ass);
            if (found) {
                INFO_LASTSAFE (arg_info) = sub_ass;
                INFO_OK (arg_info) = (copy_avis != NULL);;
            } else {
                INFO_OK (arg_info) = FALSE;
            }
        }
    }

#ifndef DBUG_OFF
    DBUG_EXECUTE (if (sub_ass == old_sub_ass)
                      DBUG_PRINT ("  no alternative found!"););
#endif
    DBUG_RETURN (sub_ass);
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
    node *avis;
    node *cavis;
    node *sub_ass, *new_sub_ass;
#ifndef DBUG_OFF
    node *suballoc;
#endif
    int depth = 0;
    DBUG_ENTER ();

    while (rets != NULL) {

        avis = ID_AVIS (EXPRS_EXPR (rets));

        if ((AVIS_SSAASSIGN (avis) != NULL) 
            && IsSuballocFill (block, avis, &cavis, &sub_ass, &depth)) {
            /*
             * we found:
             *  {...
             *    a  = ...
             *    m' = suballoc( A, iv);        sub_ass: this assignment
             *    m  = fill( copy( a), m');     cavis: N_avis of 'a`
             *  }: m
             */
#ifndef DBUG_OFF
            suballoc = ASSIGN_RHS (sub_ass);
#endif

            DBUG_PRINT ("found  %s = suballoc( %s /*outer*/, %s /*idx*/, %d /*depth*/);",
                        IDS_NAME (ASSIGN_LHS (sub_ass)),
                        ID_NAME (PRF_ARG1 (suballoc)),
                        ID_NAME (PRF_ARG2 (suballoc)),
                        NUM_VAL (PRF_ARG3 (suballoc)));
            DBUG_PRINT ("       %s = fill( copy (%s), %s);",
                        ID_NAME (EXPRS_EXPR (rets)),
                        AVIS_NAME (cavis),
                        IDS_NAME (ASSIGN_LHS (sub_ass)));

            /*
             * Situation recognized, find highest position for suballoc
             */
            new_sub_ass = FindSuballocAlternative (cavis, sub_ass, arg_info);
            if (new_sub_ass != sub_ass) {
                ModifyBlock (new_sub_ass,           // alloc_ass to be modified
                             sub_ass,                // original suballoc_ass (del!)
                             AVIS_SSAASSIGN (avis),  // original fill_ass (del!)
                             cavis,                  // originally copied now returned
                             rets,                   // rets holding the old returned
                             depth);                 // weird depth value....
                INFO_CHANGED (arg_info) = TRUE;
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
        DBUG_PRINT ("  found a use of %s; aborting search!", ID_NAME (arg_node));
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
