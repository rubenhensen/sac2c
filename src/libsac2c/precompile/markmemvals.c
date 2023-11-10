/**
 *
 * @file markmemvals.c
 *
 * 1. In this traversal all fill operations and unnecessary MemVal-variables
 *    are removed by means of a substitution traversal.
 *
 *    Ex.:
 *      c = alloc();
 *      a = fill( ..., c)
 *
 *    is transformed into
 *      c = alloc();
 *      c = ...;
 *
 *    All subsequent references to a are renamed into references to c.
 *
 *    As genarray-withloops are nothing but fancy fill-operations after
 *    the explicit alloc-operations are inserted, their result is treated
 *    exactly like the fill operation's result.
 *
 * 2. If ExplicitAccumulate was applied:
 *    All subsequent references to left hand side of accu operations
 *    in Withloops are renamed into references to the corresponding
 *    return values of the fold operators.
 *    The corresponding cexpr is also be renamed.
 *
 *
 *    Ex.:
 *      A,B,C = with(iv)
 *               gen:{ a,b   = accu( iv);
 *                     val1  = ...;
 *                     val2  = ...;
 *                     val3  = ...;
 *                     emal1 = alloc(...);
 *                     emal1 = op1( a, val1);
 *                     emal2 = alloc(...);
 *                     emal2 = op2( b, val2);
 *                   }: emal1, emal2, val3
 *              fold( op1, n1)
 *              fold( op2, n2)
 *              genarray( shp);
 *
 *    is transformed into
 *      A,B,C = with(iv)
 *               gen:{ A,B   = accu( iv);
 *                     val1  = ...;
 *                     val2  = ...;
 *                     val3  = ...;
 *                     emal1 = alloc(...);
 *                     emal1 = op1( A, val1);
 *                     emal2 = alloc(...);
 *                     emal2 = op2( B, val2);
 *                     A     = emal1;
 *                     B     = emal2;
 *                   }: A, B, val3
 *              fold( op1, n1)
 *              fold( op2, n2)
 *              genarray( shp);
 *
 * 3. For all functions where output parameters are mapped to input parameters
 *    the variable assigned to in an application is renamed to the variable in
 *    the corresponding argument position.
 *
 *    Example:
 *
 *      a, b = foo( c, d, e);   where int, int foo( int x, int y, int z)
 *                              and the first return value is mapped to the y
 *                              parameter.
 *
 *    is transformed into
 *
 *      d, b = foo( c, d, e);
 *
 *    This transformation is used in two situations:
 *    a) when the linksign pragma is used.
 *    b) in SPMD functions
 *
 * 4. For with3 loops, the results of fill operations of genarray operations
 *    are removed from the return expression and the ret chain. As we have
 *    passed the memory in in the first place, there is no need to pass the
 *    change to a part of it back out again.
 *
 *    Descriptors of with3 suballocs are moved around
 *
 */
#include "markmemvals.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "str.h"
#include "globals.h"
#include "memory.h"
#include "LookUpTable.h"
#include "print.h"
#include "DupTree.h"
#include "free.h"
#include "scheduling.h"
#include "new_types.h"

#define DBUG_PREFIX "MMV"
#include "debug.h"

/**
 * INFO structure
 */
struct INFO {
    lut_t *lut;
    lut_t *a2a_lut; /* avis of param point to avis of arg */
    node *lhs;
    node *lhs_wl;
    node *withop;
    node *fundef;
    node *prop_in;
    bool toplevel;
    int with;
    node *vardecs;
};

#define INFO_LUT(n) ((n)->lut)
#define INFO_A2A_LUT(n) ((n)->a2a_lut)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_LHS_WL(n) ((n)->lhs_wl)
#define INFO_WITHOP(n) ((n)->withop)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PROP_IN(n) ((n)->prop_in)
#define INFO_TOPLEVEL(n) ((n)->toplevel)
#define INFO_WITH(n) ((n)->with)
#define INFO_VARDECS(n) ((n)->vardecs)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LUT (result) = LUTgenerateLut ();
    INFO_A2A_LUT (result) = LUTgenerateLut ();
    INFO_LHS (result) = NULL;
    INFO_LHS_WL (result) = NULL;
    INFO_WITHOP (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PROP_IN (result) = NULL;
    INFO_TOPLEVEL (result) = TRUE;
    INFO_WITH (result) = 0;
    INFO_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_LUT (info) = LUTremoveLut (INFO_LUT (info));
    INFO_A2A_LUT (info) = LUTremoveLut (INFO_A2A_LUT (info));
    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/
/** <!--******************************************************************-->
 *
 * @fn MMVblock
 *
 *  @brief Traverses a block's instructions and the vardec afterwards
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVblock (node *arg_node, info *arg_info)
{
    node *vardecs;
    DBUG_ENTER ();

    vardecs = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDECS (arg_node)
          = TCappendVardec (INFO_VARDECS (arg_info), BLOCK_VARDECS (arg_node));
        INFO_VARDECS (arg_info) = NULL;
    }

    INFO_VARDECS (arg_info) = vardecs;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVdo
 *
 *  @brief Traverses a do loop's bodies and the condition afterwards
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);

    DO_SKIP (arg_node) = TRAVopt (DO_SKIP (arg_node), arg_info);

    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVfundef
 *
 *  @brief Traverses a FUNDEF's body and clears the LUT afterwards.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return the given fundef with substituted identifiers.
 *
 ***************************************************************************/
node *
MMVfundef (node *arg_node, info *arg_info)
{
    anontrav_t anon[2] = {{N_ids, &MMVids}, {(nodetype)0, NULL}};
    DBUG_ENTER ();

    /*
     * traverse body
     */
    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("traversing body of function \"%s\":\n", FUNDEF_NAME (arg_node));
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /*
     * Ensure all lhss have been changed.
     * In some cases the lhs in one side of ifs may be missed.
     */
    DBUG_PRINT ("traversing body of function \"%s\": fixing missing LHS adjustments\n", FUNDEF_NAME (arg_node));
    TRAVpushAnonymous (anon, &TRAVsons);
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    TRAVpop ();

    /*
     * for regular functions go on, otherwise fix rets
     */
    if (INFO_TOPLEVEL (arg_info)) {
        INFO_LUT (arg_info) = LUTremoveContentLut (INFO_LUT (arg_info));
        INFO_A2A_LUT (arg_info) = LUTremoveContentLut (INFO_A2A_LUT (arg_info));

        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        DBUG_PRINT ("traversing body of function \"%s\": fixing returns\n", FUNDEF_NAME (arg_node));
        FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);
    }
    DBUG_PRINT ("traversing body of function \"%s\": done\n", FUNDEF_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *MMVmodule( node *arg_node, info *arg_info)
 *
 * @brief Traverses only the fundef chain and leaves out special thread
 *        functions as these are traversed inline.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return the given module with all functions processed.
 ******************************************************************************/
node *
MMVmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVlet
 *
 *  @brief Traverses right hand side and substitutes left hand side
 *         identifiers.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return modified let node.
 *
 ***************************************************************************/
node *
MMVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MMVids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MMVids (node *arg_node, info *arg_info)
{
    node *newavis;

    DBUG_ENTER ();

    /*
     * We rename the LHS ids here. As there might be multiply nested
     * WLs on the RHS, we can get renaming chains! Therefore, we have
     * to rename until we have reached the final renaming.
     */
    newavis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (arg_node));
    while (newavis != IDS_AVIS (arg_node)) {
        IDS_AVIS (arg_node) = newavis;
        newavis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (arg_node));
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVid
 *
 *  @brief Substitutes the current reference with a reference from the
 *         LUT if possible
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return potentially, a new N_id node
 *
 ***************************************************************************/
node *
MMVid (node *arg_node, info *arg_info)
{
    node *newavis;

    DBUG_ENTER ();

    /*
     * Due to nested WLs, we might have renaming chains! So we
     * need to keep renaming until we reach a fixpoint.
     */

    DBUG_PRINT ("Looking for replacement of id \"%s\"\n", ID_NAME (arg_node));
    newavis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));

    while (newavis != ID_AVIS (arg_node)) {
        DBUG_PRINT ("Found \"%s\" now looking for further replacements\n", AVIS_NAME (newavis));
        ID_AVIS (arg_node) = newavis;

        newavis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
    }
    DBUG_PRINT ("No (further) replacements\n");

    DBUG_RETURN (arg_node);
}

static lut_t *
pairArgs2Args (lut_t *lut, node *args_in, node *args_out)
{
    DBUG_ENTER ();

    if (args_in != NULL) {
        DBUG_ASSERT (args_out != NULL, "params and args should be the same length");
        lut = pairArgs2Args (lut, ARG_NEXT (args_in), EXPRS_NEXT (args_out));
        lut
          = LUTinsertIntoLutP (lut, ID_AVIS (EXPRS_EXPR (args_out)), ARG_AVIS (args_in));
    }

    DBUG_RETURN (lut);
}

/** <!--********************************************************************-->
 *
 * @fn node *MMVap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MMVap (node *arg_node, info *arg_info)
{
    node *exprs, *args;
    bool toplevel;

    DBUG_ENTER ();

    /*
     * traverse special thread functions inline
     */
    if (FUNDEF_WASWITH3BODY (AP_FUNDEF (arg_node))) {
        lut_t *lut_stack = INFO_A2A_LUT (arg_info);
        toplevel = INFO_TOPLEVEL (arg_info);
        INFO_TOPLEVEL (arg_info) = FALSE;

        INFO_A2A_LUT (arg_info) = LUTgenerateLut ();

        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

        INFO_A2A_LUT (arg_info)
          = pairArgs2Args (INFO_A2A_LUT (arg_info), FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                           AP_ARGS (arg_node));

        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

        INFO_A2A_LUT (arg_info) = LUTremoveLut (INFO_A2A_LUT (arg_info));
        INFO_A2A_LUT (arg_info) = lut_stack;

        INFO_TOPLEVEL (arg_info) = toplevel;
    }

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    exprs = AP_ARGS (arg_node);
    args = FUNDEF_ARGS (AP_FUNDEF (arg_node));

    while (args != NULL) {
        if (ARG_HASLINKSIGNINFO (args)) {
            node *rets, *ids;

            rets = FUNDEF_RETS (AP_FUNDEF (arg_node));
            ids = INFO_LHS (arg_info);

            while (rets != NULL) {
                if (RET_HASLINKSIGNINFO (rets)
                    && (RET_LINKSIGN (rets) == ARG_LINKSIGN (args))) {
                    /*
                     * a = f( b)   where a === b
                     *
                     * rename: a -> b
                     */
                    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (ids),
                                       ID_NAME (EXPRS_EXPR (exprs)));

                    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids),
                                       ID_AVIS (EXPRS_EXPR (exprs)));
                }

                rets = RET_NEXT (rets);
                ids = IDS_NEXT (ids);
            }
        }

        args = ARG_NEXT (args);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfFill
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfFill (node *arg_node, info *arg_info)
{
    node *temp;

    DBUG_ENTER ();

    /*
     * a = fill( ..., b')
     *
     * rename: b' -> b
     */
    PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);

    /*
     * a = fill( ..., b)
     *
     * rename: a -> b
     */
    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (PRF_ARG2 (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (PRF_ARG2 (arg_node)));

    /*
     * eliminate the fill operation
     *
     * b = ...;
     */
    temp = PRF_ARG1 (arg_node);
    PRF_ARG1 (arg_node) = NULL;
    arg_node = FREEdoFreeTree (arg_node);
    arg_node = temp;

    /*
     * Traverse the new rhs
     */
    arg_node = TRAVopt (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfAccu
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfAccu (node *arg_node, info *arg_info)
{
    node *withop;
    node *ids_assign, *ids_wl;
    node *argexprs;

    DBUG_ENTER ();

    /*
     * A,B = with(iv)
     *        gen:{
     *             a = accu( iv, b);
     *             ...
     *             }...
     *            fold( op1, n1)
     *            ...
     *
     *     rename: a -> A
     */
    /*
     * Do not change with3 accu as thread function creates new scope.
     */
    if (INFO_WITH (arg_info) != 3) {
        ids_assign = INFO_LHS (arg_info);
        ids_wl = INFO_LHS_WL (arg_info);
        withop = INFO_WITHOP (arg_info);

        DBUG_ASSERT (withop != NULL, "F_accu without withloop");

        // check neutral arguments
        argexprs = PRF_EXPRS2 (arg_node); // skip iv
        argexprs = TRAVdo (argexprs, arg_info);

        while (withop != NULL) {
            if (NODE_TYPE (withop) == N_fold) {
                DBUG_ASSERT (ids_wl != NULL, "ids of wl is missing");
                DBUG_ASSERT (ids_assign != NULL, "ids of assign is missing");

                LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (ids_assign),
                                   IDS_NAME (ids_wl));

                LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids_assign),
                                   IDS_AVIS (ids_wl));

                ids_assign = IDS_NEXT (ids_assign);
            }
            ids_wl = IDS_NEXT (ids_wl);
            withop = WITHOP_NEXT (withop);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfSuballoc
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfSuballoc (node *arg_node, info *arg_info)
{
    node *withop = NULL;
    node *ids_wl = NULL;
    node *avis = NULL;

    DBUG_ENTER ();

    /*
     * a = suballoc( rc, A, idx)
     *
     * -or-
     *
     * a = suballoc( rc, A, idx, def)
     */

    /*
     * rename RHS
     */
    PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

    /*
     * for the C99 backend, all subarray identifiers for different
     * code blocks need to be unified, as only one is allocated in
     * the end. Furthermore, the shape of this single subarray
     * identifier is set once before the withloop. To do this, we
     * annotate it at the GENARRAY. We can furthermore remove the
     * default argument to the suballoc, as suballoc does not
     * initialise the shape in these cases.
     *
     * For the mutc backend, there is one subvar per thread. Thus,
     * we do not need to annotate the subvar here. Instead, we
     * leave the default element at the suballoc to be able to
     * issue the corresponding shape initialisation ICM later on.
     */
    /*
     * Find subarray identifier for this suballoc
     */

    if ((global.backend != BE_mutc) || global.mutc_suballoc_desc_one_level_up) {
        ids_wl = INFO_LHS_WL (arg_info);
        withop = INFO_WITHOP (arg_info);

        while (ids_wl != NULL) {
            node *newavis;

            if (global.mutc_suballoc_desc_one_level_up) {
                newavis = (node *)
                  LUTsearchInLutPp (INFO_LUT (arg_info),
                                    LUTsearchInLutPp (INFO_A2A_LUT (arg_info),
                                                      LUTsearchInLutPp (INFO_LUT (
                                                                          arg_info),
                                                                        IDS_AVIS (
                                                                          ids_wl))));
            } else {
                newavis
                  = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (ids_wl));
            }

            if ((newavis == ID_AVIS (PRF_ARG2 (arg_node)))
                || ((INFO_WITH (arg_info) == 3)
                    && (STReq (AVIS_NAME (newavis),
                               AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))))))) {
                /*
                 * Set a as new subarray identifier if none is set yet
                 */
                if (WITHOP_SUB (withop) == NULL) {
                    L_WITHOP_SUB (withop, TBmakeId (IDS_AVIS (INFO_LHS (arg_info))));
                    if (INFO_WITH (arg_info) == 3) {
                        AVIS_SUBALLOC (ID_AVIS (WITHOP_SUB (withop))) = TRUE;
                    }
                } else if ((INFO_WITH (arg_info) == 3)
                           && (global.mutc_suballoc_desc_one_level_up)) {
                    /* Add vardec to this context */
                    DBUG_PRINT ("Addd new vardec: %s",
                                AVIS_NAME (ID_AVIS (WITHOP_SUB (withop))));
                    INFO_VARDECS (arg_info)
                      = TBmakeVardec (DUPdoDupNode (ID_AVIS (WITHOP_SUB (withop))),
                                      INFO_VARDECS (arg_info));
                }
                avis = ID_AVIS (WITHOP_SUB (withop));
                break;
            }
            withop = WITHOP_NEXT (withop);
            ids_wl = IDS_NEXT (ids_wl);
        }

        if ((global.backend != BE_mutc) || (avis != NULL)) {
            DBUG_ASSERT (avis != NULL, "No subarray identifier found!");

            /*
             * Insert pair (a, A_sub) into LUT
             */
            LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                               AVIS_NAME (avis));

            LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)), avis);
        }
        /*
         * Scrap the 3rd/4th argument, as this suballoc does not need to
         * set the shape descriptor.
         */
        if (global.backend == BE_c99 || global.backend == BE_cuda
            || global.backend == BE_cudahybrid) {
            if (PRF_EXPRS4 (arg_node) != NULL) {
                PRF_EXPRS4 (arg_node) = FREEdoFreeTree (PRF_EXPRS4 (arg_node));
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfWLAssign
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfWLAssign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * a' = wl_assign( a, A, iv);
     *
     * 1. rename RHS
     * => a' = wl_assign( a, A)
     */
    PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

    /*
     * 2. insert pair ( a', a) into LUT
     */
    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (PRF_ARG1 (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (PRF_ARG1 (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfPropObjIn
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfPropObjIn (node *arg_node, info *arg_info)
{
    node *ids_assign;
    node *args;

    DBUG_ENTER ();

    /*
     * A,B = with(iv)
     *        gen:{
     *             a',... = prop_obj_in( iv, a, ...);
     *             ...
     *             a''    = f(a');
     *             ...
     *             a'''   = prop_obj_out( a'');
     *             }...
     *            propagate( a)
     *            ...
     *
     *     rename: a' -> a
     */

    ids_assign = INFO_LHS (arg_info);
    args = EXPRS_NEXT (PRF_ARGS (arg_node));

    while (args != NULL) {
        DBUG_ASSERT (ids_assign != NULL, "ids of assign is missing");
        LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (ids_assign),
                           ID_NAME (EXPRS_EXPR (args)));
        LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids_assign),
                           ID_AVIS (EXPRS_EXPR (args)));
        ids_assign = IDS_NEXT (ids_assign);
        args = EXPRS_NEXT (args);
    }

    INFO_PROP_IN (arg_info) = arg_node;

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfPropObjOut
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfPropObjOut (node *arg_node, info *arg_info)
{
    node *ids_assign;
    node *args;

    DBUG_ENTER ();

    /*
     * A,B = with(iv)
     *        gen:{
     *             a,... = prop_obj_in( iv, a, ...);
     *             ...
     *             a''    = f(a);
     *             ...
     *             a'''   = prop_obj_out( a'');
     *             } : a''', ...
     *            propagate( a)
     *            ...
     *
     *     rename: a''' -> a
     */

    ids_assign = INFO_LHS (arg_info);
    args = EXPRS_NEXT (PRF_ARGS (INFO_PROP_IN (arg_info)));

    while (args != NULL) {
        DBUG_ASSERT (ids_assign != NULL, "ids of assign is missing");
        DBUG_PRINT ("renaming %s -> %s", IDS_NAME (ids_assign),
                    ID_NAME (EXPRS_EXPR (args)));
        LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (ids_assign),
                           ID_NAME (EXPRS_EXPR (args)));
        LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids_assign),
                           ID_AVIS (EXPRS_EXPR (args)));
        ids_assign = IDS_NEXT (ids_assign);
        args = EXPRS_NEXT (args);
    }

    INFO_PROP_IN (arg_info) = NULL;

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MMVprfGuard( node *arg_node, info *arg_info)
 *
 * @brief x1', ..., xn' = guard (x1, ..., xn, p)
 * - rename x1, ..., xn, p
 * - insert (xi', xi) into LUT
 *
 *****************************************************************************/
static node *
MMVprfGuard (node *arg_node, info *arg_info)
{
    node *lhs, *args, *e;

    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

    lhs = INFO_LHS (arg_info);
    args = PRF_ARGS (arg_node);

    DBUG_ASSERT (TCcountIds (lhs) == TCcountExprs (args) - 1,
                 "guard function should return n-1 values");

    while (lhs != NULL) {
        e = EXPRS_EXPR (args);
        LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (lhs), ID_NAME (e));
        LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (lhs), ID_AVIS (e));
        lhs = IDS_NEXT (lhs);
        args = EXPRS_NEXT (args);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MMVprfAfterGuard( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
static node *
MMVprfAfterGuard (node *arg_node, info *arg_info)
{
    node *v;
    node *a;

    DBUG_ENTER ();

    /*
     * v = guard(a,p1,..pn);
     *
     * 1. rename a,p1,..,pn
     * 2. Insert (v,a) into LUT
     */
    PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

    v = INFO_LHS (arg_info);
    a = PRF_ARG1 (arg_node);
    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (v), ID_NAME (a));
    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (v), ID_AVIS (a));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprf
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         prf is a F_fill.
 *         Adds the LHS and the corresponding LHS of current WL into LUT
 *         if this prf is a F_accu.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_fill:
        arg_node = MMVprfFill (arg_node, arg_info);
        break;

    case F_accu:
        arg_node = MMVprfAccu (arg_node, arg_info);
        break;

    case F_suballoc:
        arg_node = MMVprfSuballoc (arg_node, arg_info);
        break;

    case F_wl_assign:
        arg_node = MMVprfWLAssign (arg_node, arg_info);
        break;

    case F_prop_obj_in:
        arg_node = MMVprfPropObjIn (arg_node, arg_info);
        break;

    case F_prop_obj_out:
        arg_node = MMVprfPropObjOut (arg_node, arg_info);
        break;

    case F_guard:
        arg_node = MMVprfGuard (arg_node, arg_info);
        break;

    case F_afterguard:
        arg_node = MMVprfAfterGuard (arg_node, arg_info);
        break;

    default:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwith (node *arg_node, info *arg_info)
{
    node *withop;
    node *lhs;
    node *prop_in;
    int with;

    DBUG_ENTER ();

    /* stack lhs and withop of surrounding WL */
    lhs = INFO_LHS_WL (arg_info);
    withop = INFO_WITHOP (arg_info);
    prop_in = INFO_PROP_IN (arg_info);
    with = INFO_WITH (arg_info);

    INFO_LHS_WL (arg_info) = INFO_LHS (arg_info);
    INFO_WITHOP (arg_info) = WITH_WITHOP (arg_node);
    INFO_WITH (arg_info) = 1;

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    INFO_WITHOP (arg_info) = withop;
    INFO_LHS_WL (arg_info) = lhs;
    INFO_PROP_IN (arg_info) = prop_in;
    INFO_WITH (arg_info) = with;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith2
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwith2 (node *arg_node, info *arg_info)
{
    node *withop;
    node *lhs;
    node *prop_in;
    int with;

    DBUG_ENTER ();

    /*
     * stack lhs and withop of surrounding WL
     */
    lhs = INFO_LHS_WL (arg_info);
    withop = INFO_WITHOP (arg_info);
    prop_in = INFO_PROP_IN (arg_info);
    with = INFO_WITH (arg_info);

    INFO_LHS_WL (arg_info) = INFO_LHS (arg_info);
    INFO_WITHOP (arg_info) = WITH2_WITHOP (arg_node);
    INFO_WITH (arg_info) = 2;

    WITH2_WITHOP (arg_node) = TRAVopt (WITH2_WITHOP (arg_node), arg_info);
    WITH2_SEGS (arg_node) = TRAVopt (WITH2_SEGS (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), arg_info);

    INFO_WITHOP (arg_info) = withop;
    INFO_LHS_WL (arg_info) = lhs;
    INFO_PROP_IN (arg_info) = prop_in;
    INFO_WITH (arg_info) = with;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith3
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwith3 (node *arg_node, info *arg_info)
{
    node *withop;
    node *lhs;
    node *prop_in;
    int with;

    DBUG_ENTER ();

    /*
     * stack lhs and withop of surrounding WL
     */
    lhs = INFO_LHS_WL (arg_info);
    withop = INFO_WITHOP (arg_info);
    prop_in = INFO_PROP_IN (arg_info);
    with = INFO_WITH (arg_info);

    INFO_LHS_WL (arg_info) = INFO_LHS (arg_info);
    INFO_WITHOP (arg_info) = WITH3_OPERATIONS (arg_node);
    INFO_WITH (arg_info) = 3;

    WITH3_OPERATIONS (arg_node) = TRAVopt (WITH3_OPERATIONS (arg_node), arg_info);
    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    INFO_WITHOP (arg_info) = withop;
    INFO_LHS_WL (arg_info) = lhs;
    INFO_PROP_IN (arg_info) = prop_in;
    INFO_WITH (arg_info) = with;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwlseg
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 ***************************************************************************/
node *
MMVwlseg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (arg_node)
          = SCHmarkmemvalsScheduling (WLSEG_SCHEDULING (arg_node), INFO_LUT (arg_info));
        WLSEG_TASKSEL (arg_node)
          = SCHmarkmemvalsTasksel (WLSEG_TASKSEL (arg_node), INFO_LUT (arg_info));
    }

    WLSEG_CONTENTS (arg_node) = TRAVdo (WLSEG_CONTENTS (arg_node), arg_info);
    WLSEG_IDXINF (arg_node) = TRAVopt (WLSEG_IDXINF (arg_node), arg_info);
    WLSEG_IDXSUP (arg_node) = TRAVopt (WLSEG_IDXSUP (arg_node), arg_info);

    WLSEG_NEXT (arg_node) = TRAVopt (WLSEG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVgenarray
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);

    GENARRAY_MEM (arg_node) = TRAVdo (GENARRAY_MEM (arg_node), arg_info);

    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (GENARRAY_MEM (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (GENARRAY_MEM (arg_node)));

    if (GENARRAY_NEXT (arg_node) != NULL) {
        DBUG_ASSERT (IDS_NEXT (INFO_LHS (arg_info))!=NULL,
                     "with-loop has more operators than LHS variables\n");
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVpropagate
 *
 *  @brief Just shifts the arguments and goes to the next WO.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PROPAGATE_DEFAULT (arg_node) = TRAVdo (PROPAGATE_DEFAULT (arg_node), arg_info);

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVmodarray
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    MODARRAY_MEM (arg_node) = TRAVdo (MODARRAY_MEM (arg_node), arg_info);

    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (MODARRAY_MEM (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (MODARRAY_MEM (arg_node)));

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVbreak
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BREAK_MEM (arg_node) = TRAVdo (BREAK_MEM (arg_node), arg_info);

    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (BREAK_MEM (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (BREAK_MEM (arg_node)));

    if (BREAK_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        BREAK_NEXT (arg_node) = TRAVdo (BREAK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVfold
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         withop is either WO_genarray or WO_modarray.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
    FOLD_INITIAL (arg_node) = TRAVopt (FOLD_INITIAL (arg_node), arg_info);

    if (FOLD_ISPARTIALFOLD (arg_node)) {
        FOLD_PARTIALMEM (arg_node) = TRAVdo (FOLD_PARTIALMEM (arg_node), arg_info);

        LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                           ID_NAME (FOLD_PARTIALMEM (arg_node)));

        LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                           ID_AVIS (FOLD_PARTIALMEM (arg_node)));
    }

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVcode
 *
 *  @brief Substitutes a CEXPR reference with the corresponding reference
 *         on LHS of current withloop if the corresponding WL operation
 *         is fold.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVcode (node *arg_node, info *arg_info)
{
    node *wlids;
    node *cexprs;
    node *withop;

    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);

    /*
     * if at least one WL-operator ist fold the accumulation results
     * must assigned back to the accumulation variable
     *
     * A,B = with(iv)
     *         gen:{ res1 = ...;
     *               res2 = ...;
     *               A    = res1   <---!!!
     *             } : A, res2
     *       fold( op1, n1)
     *       ...
     *
     */
    wlids = INFO_LHS_WL (arg_info);
    cexprs = CODE_CEXPRS (arg_node);
    withop = INFO_WITHOP (arg_info);

    while (withop != NULL) {
        if (NODE_TYPE (withop) == N_fold && !FOLD_ISPARTIALFOLD (withop)) {
            BLOCK_ASSIGNS (CODE_CBLOCK (arg_node))
              = TCappendAssign (BLOCK_ASSIGNS (CODE_CBLOCK (arg_node)),
                                TBmakeAssign (TBmakeLet (DUPdoDupNode (wlids),
                                                         DUPdoDupNode (
                                                           EXPRS_EXPR (cexprs))),
                                              NULL));

            ID_AVIS (EXPRS_EXPR (cexprs)) = IDS_AVIS (wlids);
        }
        wlids = IDS_NEXT (wlids);
        cexprs = EXPRS_NEXT (cexprs);
        withop = WITHOP_NEXT (withop);
    }

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *MMVreturn( node *arg_node, info *arg_info)
 *
 * @brief Removes results of fill operations to submemory from the return
 *        expressions
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/
node *
MMVreturn (node *arg_node, info *arg_info)
{
    node *withops, *exprs;

    DBUG_ENTER ();

    /*
     * 1) rename RHS
     */
    RETURN_EXPRS (arg_node) = TRAVopt (RETURN_EXPRS (arg_node), arg_info);

    if (!INFO_TOPLEVEL (arg_info)) {
        /*
         * 2) remove results of genarray fill operation as
         *    the memory has been passed in and doesn't need
         *    to be passed out again
         *
         */
        withops = INFO_WITHOP (arg_info);
        exprs = RETURN_EXPRS (arg_node);

        while (exprs != NULL) {
            DBUG_ASSERT (withops != NULL, "more results in threadfun than withops!");

            if ((NODE_TYPE (withops) == N_genarray) || (NODE_TYPE (withops) == N_fold)) {
                /*
                 * handle first in chain properly
                 */
                if (RETURN_EXPRS (arg_node) == exprs) {
                    RETURN_EXPRS (arg_node) = FREEdoFreeNode (RETURN_EXPRS (arg_node));
                    exprs = RETURN_EXPRS (arg_node);
                } else {
                    exprs = FREEdoFreeNode (exprs);
                }
            } else {
                exprs = EXPRS_NEXT (exprs);
            }

            withops = WITHOP_NEXT (withops);
        }

        DBUG_ASSERT (withops == NULL, "more withops than results in threadfun!");
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *MMVret( node *arg_node, info *arg_info)
 *
 * @brief Removes results of fill operations to submemory from the ret chain
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 ******************************************************************************/
node *
MMVret (node *arg_node, info *arg_info)
{
    node *withops;

    DBUG_ENTER ();

    withops = INFO_WITHOP (arg_info);

    DBUG_ASSERT (withops != NULL, "more rets in threadfun than withops!");

    INFO_WITHOP (arg_info) = WITHOP_NEXT (INFO_WITHOP (arg_info));
    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);
    INFO_WITHOP (arg_info) = withops;

    /*
     * For genarrays, we remove the correspoding return.
     *
     */
    if ((NODE_TYPE (withops) == N_genarray) || (NODE_TYPE (withops) == N_fold)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */

/** <!--******************************************************************-->
 *
 * @fn MarkMemVals
 *
 *  @brief Starting function of MarkMemVals traversal
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree
 *
 ***************************************************************************/
node *
MMVdoMarkMemVals (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_mmv);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
