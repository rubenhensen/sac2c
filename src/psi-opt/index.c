/*
 *
 * $Log$
 * Revision 3.74  2005/08/20 20:08:32  sah
 * IVE-rewrite: skeleton implementation
 *
 * Revision 3.73  2005/08/20 19:20:48  sah
 * IVE-rewrite: disabled old IVE code
 *
 */

#include <stdio.h>
#include <string.h>
#include "index.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "shape.h"
#include "new_types.h"
#include "dbug.h"
#include "ctinfo.h"
#include "print.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "convert.h"
#include "NameTuplesUtils.h"
#include "index_infer.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 * a) we can not eliminate index-vectors-variables of WLs
 *    completely, since refcount inference relies on their
 *    existence. As soon as that bug is fixed, we can try
 *
 *    #define REFCOUNT_PROBLEM_SOLVED
 *
 *    which will trigger the elimination of them.
 */

/**
 *
 * @defgroup ive IVE
 * @ingroup opt
 *
 * @brief The "index vector elimination" (IVE for short) tries to
 *        eliminate index vectors which are used for array selections only.
 * <pre>
 * Example:
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            i = [2,3];
 *            z = a[i];
 *
 * is transformed into:
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            __i_4_4 = 11;
 *            z = idx_sel(a, __i_4_4);
 * </pre>
 *
 * @{
 */

/**
 *
 * @file index.c
 *
 *  This file contains the implementation of IVE (index vector elimination).
 *
 *
 * <pre>
 *
 * 1) Basics
 * ---------
 *
 * For eliminating index vectors, "Uses" attributes are inferred for all integer
 * vectors. They are attached to each left hand side of assignments and to
 * variable/argument declarations.
 * Since we want to eliminate index vectors the attribute attachment is
 * restricted to one-dimensional integer arrays( array identifiers)!
 * The "Uses" attribute consists of a set (chain) of attributes of the kind:
 *
 *     VECT/ IDX(<shape>).
 *
 * For
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            i = [2,3];
 *            z = a[i];
 *
 * we obtain:
 *
 *            int z;
 *            int[2] i:IDX(int[4,4]) ;
 *            int[4, 4] a;
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            i:IDX(int[4,4]) = [2,3];
 *            z= a[i];
 *
 * BTW, this can be made visible by using the break option that stops
 * compilation after IVE is done (at the time being this is -b16)!
 *
 *
 *
 * 2) Attaching the "Uses" attribute
 * ---------------------------------
 *
 * 2a) assignment chaines
 * ----------------------
 *
 * The "Uses" information is stored as follows:
 * During the (bottom up) traversal all information is kept at the
 * N_vardec/N_arg nodes.
 * This is realized by the introduction of a new node: "N_vinfo"
 * which is organized as follows:
 *
 *     info.use (useflag): VECT / IDX
 *     node[0]           : next "N_vinfo node" if existent
 *     node[1]           : withbelonging shape if info.use == IDX
 *
 * it can be accessed by:
 *     VINFO_FLAG(n),
 *     VINFO_NEXT(n), and          ( cf. tree_basic.h )
 *     VINFO_SHAPE(n)
 *
 * There are two chains of such nodes attached to the N_vardec/N_arg nodes:
 * The "actual ones" and the "collected ones". The "actual ones" contain all
 * uses-infos collected during the (bottom up) traversal since the last
 * assignment to that particular variable. In contrast, the "collected ones"
 * chain collects all uses-infos within the function body.
 * They can be accessed by :
 *     VARDEC_ACTCHN(n), and
 *     VARDEC_COLCHN(n), or        ( cf. tree_basic.h )
 *     ARG_ACTCHN(n), and
 *     ARG_COLCHN(n) respectively.
 *
 * Whenever an array identifier is met in index-position of an array-selection,
 * the respective IDX(...) attribute is added (without duplicates) to both
 * chains of the variable's declaration.
 *
 * When meeting a "N_let" with an array variable to be defined, the actual info
 * node(s) attached to the variable's declaration( "actual chain")
 * is(are) inserted in the ids structure of the N_let-node [LET_IDS(n)] at
 * ids->use [IDS_USE(i)] => [LET_USE(n)]    ( cf. tree_compound.h )
 * and the "actual chain" is freed before traversing the RHS of the N_let.
 *
 *
 * 2b) conditionals
 * ----------------
 *
 * The mechanism described so far does not properly support programs that
 * contain program parts that may or may not be "executed", i.e., conditionals
 * or loops.
 * Since we want to replace all sel ops by idx_sel ops, we have to infer all
 * potential uses rather than only those taken by a particular branch. To
 * achieve this, different actual chains for the alternative branches have to
 * be created and merged, e.g.,
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            b= reshape([8,2], [1,2,...,16]);
 *            iv = [1];
 *            if( ...)
 *              iv = [9];
 *            else
 *              z = b[iv];
 *            a[iv];
 *
 * should lead to
 *
 *            int[2] iv:IDX(int[8,2]):IDX(int[4,4]) ;
 *            int[4, 4] a;
 *            int[8, 2] b;
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            b= reshape([8,2], [1,2,...,16]);
 *            iv:IDX(int[8,2]):IDX(int[4,4]) = [1,1];
 *            if( ...)
 *              iv:IDX(int[4,4]) = [2,2];
 *            else
 *              z = b[iv];
 *            a[iv];
 *
 * To get these results, we have to traverse both branches of the
 * conditional starting out with an "actual chain" gained by traversing the
 * rest, which in our example is: IDX(int[4,4]).
 * The results -- empty / IDX(int[8,2]):IDX(int[4,4])  in our example --
 * have to be merged, thus yielding IDX(int[8,2]):IDX(int[4,4]) as "actual
 * chain" when leaving the conditional.
 *
 * The implementation of this requires as many "actual chaines" to be kept
 * simultaneously as we have nestings of conditionals. Therefore, we need a
 * stacking mechanism. This is done by adding a third type of vinfo node via
 * a new flag DOLLAR, which marks the end of the current "actual chain".
 *
 * In order to make an easy access to different chains feasable, each N_vinfo
 * node gets a pointer to its bottom. It is kept in
 *
 *    VINFO_DOLLAR(n)       ( cmp. tree_basic.h )
 *
 * Thus, for consistency reasons, an empty chain is initialized with a
 * DOLLAR-N_vinfo which contains a self reference in the VINFO_DOLLAR attribute.
 *
 *
 * When traversing a conditional, we proceed as follows:
 * First of all, we copy all actual chaines of ALL vardecs(!). This requires
 * a pointer to the topmost vardec of the actual functions which is kept in
 * INFO_IVE_VARDECS( arg_info). For iv of the example given above, we get:
 *
 *  IDX(int[4,4]):$:IDX(int[4,4]):$    (bottom considered to be right!!)
 *
 * Traversing the then-part, we obtain
 *
 *  $:IDX(int[4,4]):$
 *
 * since the topmost "actual chain" is attached to the let in the then-part!
 * After that, the kept "actual chain" of the assignments after the
 * conditional and the topmost "actual chain" are switched:
 *
 *  IDX(int[4,4]):$:$
 *
 * Traversing the else-part yields
 *
 *  IDX(int[8,2]):IDX(int[4,4]):$:$
 *
 * due to the access on b. Finally, the two results have to merged and the
 * result replaces the two "actual chains" for the different branches:
 *
 *  IDX(int[8,2]):IDX(int[4,4]):$
 *
 * With this "actual chain" the normal assignment-mechanism proceeds.
 *
 * To get a more formal specification of the algorithm, let's assume
 * A , T, E, and R are sequences of assignments which may contain conditionals
 * and loops by themselves. Then a program (fragment)
 *
 *     A;
 *     if() {
 *       T;
 *     }
 *     else {
 *       E;
 *     }
 *     R;
 *
 * leeds to the following constellations of "actual chaines":
 *
 *                       trav( R;) $     enter conditional
 *           trav( R;) $ trav( R;) $     copy actual chains
 *        trav( T; R;) $ trav( R;) $     traverse then-part
 *        trav( R;) $ trav( T; R;) $     switch chains
 *     trav( E; R;) $ trav( T; R;) $     traverse else-part
 *           trav( E; R; ++ T; R;) $     merge chains & leave conditional
 *
 *
 *
 *
 *
 * 2c) loops
 * ---------
 *
 * For loops a similar mechanism is rquired; we have to stack the "actual
 * chain" of the assignments that follow the loop, and we have to merge those
 * with the "actual chain" obtained from traversing the loop body.
 *
 * For example, consider the following code fragment:
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            b= reshape([8,2], [1,2,...,16]);
 *            iv = [1];
 *            while( ...) {
 *              iv = [2];
 *              z = b[iv];
 *            }
 *            a[iv];
 *
 * Traversing "a[iv]" we get the following "actual chain":
 *
 *  IDX(int[4,4]):$
 *
 * We enter the loop body with:
 *
 *  IDX(int[4,4]):$IDX(int[4,4]):$
 *
 * which leads to an attributation of the assignment "iv = [2];" by
 * IDX(int[8,2]):IDX(int[4,4]) and finally results in:
 *
 *  $:IDX(int[4,4]):$
 *
 * The subsequent merge yields:
 *
 * IDX(int[4,4]):$
 *
 * So that the entire program segment will be attributed as follows
 *
 *            int[2] iv:IDX(int[8,2]):IDX(int[4,4]) ;
 *            int[4, 4] a;
 *            int[8, 2] b;
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            b= reshape([8,2], [1,2,...,16]);
 *            iv:IDX(int[4,4]) = [1];
 *            while( ...) {
 *              iv:IDX(int[8,2]):IDX(int[4,4]) = [2];
 *              z = b[iv];
 *            }
 *            a[iv];
 *
 * Unfortunately, this mechanism does not suffice.....
 * The problem that may arise originates from the fact that the execution
 * of the loop body may be followed by another execution of the loop body
 * in which case some uses annotations at array definitions in the loop
 * body may be missing. This happens if an array is used before it is defined
 * within a loop. Consider the following example:
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            b= reshape([8,2], [1,2,...,16]);
 *            iv = [1];
 *            while( ...) {
 *              z = b[iv];
 *              iv = [2];
 *            }
 *            a[iv];
 *
 * A straight-forward application of the mechanism described above would yield:
 *
 *            int[2] iv:IDX(int[8,2]):IDX(int[4,4]) ;
 *            int[4, 4] a;
 *            int[8, 2] b;
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            b= reshape([8,2], [1,2,...,16]);
 *            iv:IDX(int[8,2]):IDX(int[4,4]) = [1];
 *            while( ...) {
 *              z = b[iv];
 *              iv:IDX(int[4,4]) = [2];
 *            }
 *            a[iv];
 *
 * What is missing here is an IDX(int[8,2]) attribute at the assignment to iv
 * within the loop body.
 * Therefore, we unfortunately have to traverse the loop body two times!
 * Rather than going through the details of the above example, a more
 * formal description similar to that in the end of 2b) is given here:
 *
 * A program fragment
 *
 *     A;
 *     while() {
 *       L;
 *     }
 *     R;
 *
 * leeds to the following constellations of "actual chaines":
 *
 *                                       trv( R;) $    enter while-loop
 *                            trv( R;) $ trv( R;) $    copy actual chain
 *                         trv( L; R;) $ trv( R;) $    traverse loop body
 *                              trv( L; R; ++ R;) $    merge chains
 *          trv( L; R; ++ R;) $ trv( L; R; ++ R;) $    copy actual chain
 *     trv( L; (L; R; ++ R;)) $ trv( L; R; ++ R;) $    traverse loop body again
 *                              trv( L; R; ++ R;) $    delete topmost chain
 *                                                        and leave loop
 *
 * Since for do-loops we know that the loop will be evaluated at least once,
 * for these loops
 *
 *     A;
 *     do {
 *       L;
 *     } while();
 *     R;
 *
 * leeds to the following constellations of "actual chaines":
 *
 *                                 trv( R;) $    enter do-loop
 *                      trv( R;) $ trv( R;) $    copy actual chain
 *                   trv( L; R;) $ trv( R;) $    traverse loop body
 *          trv( L; R; ++ R;) $ trv( L; R;) $    merge chains and copy old top
 *     trv( L; (L; R; ++ R;)) $ trv( L; R;) $    traverse loop body
 *                              trv( L; R;) $    delete topmost chain
 *                                                   and leave loop
 *
 * Implementation-wise, we face another problem with loops here:
 * Since the code is changed in the same traversal as the uses attributes are
 * built during the first traversal of loops these changes have to be
 * prevented. This is indicated by a mode-flag in INFO_IVE_MODE(arg_info)
 * which either is set to
 *     M_uses_and_transform    or to      M_uses_only
 *
 *
 * 3) Inserting new variable declarations
 * --------------------------------------
 *
 * Since all variables have backreferences to their declarations, any
 * new declarations have to be created when the first occurence of the
 * identifier is met. The adresses of these N_vardecs are stored in the
 * respective N_vinfo-nodes of the collected-chain. It can be accessed by:
 *
 *     VINFO_VARDEC(n)             ( cmp. tree_basic.h )
 *
 * Creating the new vardec's while traversing the var-usages requires a pointer
 * to the vardec-section of the function. In case of local variables this can
 * be achieved easily be following the N_id's backref to its N_vardec.
 * For function arguments the situation is more complicated. Their backref
 * leads to the N_arg node from which we can not reach the function's
 * vardec-section.
 * Therefore, we first traverse a function's arguments and insert backrefs to
 * the N_fundef node. This enables us to find the vardec-section when creating
 * new vardecs for variables which are formal parametes of the function.
 *
 *
 *
 * 4) Inserting new Assignments
 * ----------------------------
 *
 * Apart from the two traversals due to loops ( see above) the insertion of new
 * assignments is done in the same traversal during which the uses-inference is
 * done.
 * The central function for initiating this process id IdxLet. Basically, for
 * each of the variables <var> of the LHS and for each IDX(<shp>) usage, an
 * assignment of the form
 *    __<varname>__<shp> = ComputeIdxFromShape( <varname>, <shp>)
 * is inserted.
 * This is achieved by generating an ND_VECT2OFFSET icm.
 * Complementary to this code modification, all F_sel and all F_modarray
 * operations are replaced by their idx counterparts, e.g., an application
 *   sel( iv, a)   is replaced by   idxsel( __iv__<shp>, a)   .
 * To avoid superfluous computations of this form, so-called "pure address
 * computations" are identified. These are assignments of the form
 *
 *    iv = iv1;             or of the form
 *    iv = exp1 op exp2;
 * where
 *    iv only has IDX(...) usages and
 *    op is +,-,*,\    (without *,\ between two vectors)
 *
 * These assignments propagate the uses flags and lead to new assignments of
 * the form
 *
 *    __iv__<shp> = __iv1__<shp>;
 *    __iv__<shp> = Vect2Off( iv1) op Vect2Off( iv2);
 * where
 *    Vect2Off( exp) =
 *
 *   / __iv__<shp>                                             iff exp == iv
 *  |  (.((s *shp1 + s )*shp2 + s )...)*shp(n-1) + s           iff exp == s (scalar)
 *   \ (.((s0*shp1 + s1)...)*shpm + sm)*shp(m+1)...*shp(n-1)   otherwise, i.e.
 *                                                             exp == [s0, ..., sm]
 *
 * The CENTRAL MECHANISM to achieve these transformations is steered by IdxLet.
 * Prior to the traversal of a RHS IDXLet copies the actual assignment (if
 * required) and then traverses the RHS with INFO_IVE_TRANSFORM_VINFO(arg_info)
 * either containing a pointer to the N_vinfo node containing the shape
 * information required for the transformation process or being NULL in order
 * to indicate that the vector version should be kept as it is.
 * All IDX functions that will be called during that traversal, i.e.
 *     IDXid, IDXnum, IDXprf, IDXarray, IDXwith, and IDXwith
 * are steered by that mechanism.
 *
 *
 * 5) Eliminating Superfluous Vardecs
 * ----------------------------------
 *
 * Since DCR will not be run after IVE, IVE tries to eliminate superfluous
 * vardecs of vectors which were successfully eliminated by IVE. Since the
 * original vardecs are needed during the traversal of the body, the vardec
 * elimination is done after traversing the bodies
 *
 *
 *******************************************************************************
 *
 * Thus, we finally find the following usages of the arg_info node:
 *
 *    INFO_IVE_FUNDEF  - holds pointer to actual N_fundef
 *                     - is needed for creating backrefs of arg-nodes.
 *                       and to insert new vardecs for lifted arguments
 *    INFO_IVE_VARDECS - holds pointer to the topmost vardec of the function
 *                     - is needed for traversing all vardecs in loops/conds
 *    INFO_IVE_PRE_ASSIGNS - contains all newly created assignments before
 *                           they are inserted into the AST by IDXassign()
 *    INFO_IVE_MODE    - indicates whether we are interested in uses only
 *                       (M_uses_only) or in code transformations as well
 *                       (M_uses_and_transform)
 *    INFO_IVE_CURRENTASSIGN - holds the current assign node
 *                           - needed for inserting new assignments and in WLs
 *                             dkr: it would be nicer to use something like
 *                                  INFO_IVE_POST_ASSIGNS here (see ..._PRE_...)
 *    INFO_IVE_TRANSFORM_VINFO - indicates whether N_nums, N_prfs, and N_arrays
 *                               should be transformed or not.
 *    INFO_IVE_NON_SCAL_LEN - needed for F_add_SxA, F_add_AxS, F_sub_SxA, and
 *                            F_sub_AxS if they are used for pure iv address
 *                            computations on ivs of non-maximal length!
 *                            In these situations the length of the non-scalar
 *                            argument has to be known when traversing the
 *                            scalar argument (cf. IDXnum).
 *
 * </pre>
 */

/**
 * counts the number of index vectors removed during optimization.
 */
static int ive_expr;

/**
 * helper functions
 */

/** <!--********************************************************************-->
 *
 * @fn char *IVEChangeId( char *varname, shape *shp)
 *
 * @brief appends the shape given by type to the varname.
 *
 *    Example:
 *    test, int[1,4,2,3]  =>  test_1_4_2_3__
 *    does not free the argument space!
 *
 ******************************************************************************/

char *
IVEchangeId (char *varname, shape *shp)
{
    static char buffer[1024];
    static char buffer2[32];
    int i;

    DBUG_ENTER ("IVEchangeId");

    sprintf (buffer, "%s", varname);
    for (i = 0; i < SHgetDim (shp); i++) {
        sprintf (buffer2, "_%d", SHgetExtent (shp, i));
        strcat (buffer, buffer2);
    }
    sprintf (buffer2, "__");
    strcat (buffer, buffer2);

    DBUG_RETURN (ILIBstringCopy (buffer));
}

/**
 * traversal functions
 */

node *
IVEfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEfundef");

    DBUG_RETURN (arg_node);
}

node *
IVEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEids");

    DBUG_RETURN (arg_node);
}

node *
IVEprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEprf");

    DBUG_RETURN (arg_node);
}

node *
IVEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEassign");

    DBUG_RETURN (arg_node);
}

node *
IVEap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEap");

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief function triggering the index vector elimination on the
 *        syntax tree. the traversal relies on the information
 *        infered by index vector elimination inference.
 *
 * @param syntax_tree N_module node
 *
 * @return transformed syntax tree
 ******************************************************************************/
node *
IVEdoIndexVectorElimination (node *syntax_tree)
{
    DBUG_ENTER ("IndexVectorElimination");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "IVE is intended to run on the entire tree");

    TRAVpush (TR_ive);

    syntax_tree = TRAVdo (syntax_tree, NULL);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/*@}*/ /* defgroup ive */
