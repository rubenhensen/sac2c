
/*
 * $Log$
 * Revision 2.7  1999/09/01 19:38:45  sbs
 * some errors concerning the handling of WL generators and the elimination
 * of superfluous vardecs solved!
 *
 * Revision 2.6  1999/09/01 14:53:03  sbs
 * Modified Version of IVE: superfluous int[.] vardecs are eliminated now.
 * Basically, this required 2 extensions:
 * 1) after traversing the body of a block the vardecs are traversed a second time.
 *    This eliminates all those vardecs which do not contain VECT in their COLCHAIN.
 * 2) Whenever a Vect2Offset Icm is created, VECT is added to the COLCHAIN.
 *
 * I do hope that these 2 extensions suffice to do it 8-))))
 *
 * Revision 2.5  1999/08/31 10:01:31  jhs
 * switched traversal of block and expr in IdxNcode; now, we do the
 * expr BEFORE we take care of the block...
 *
 * Revision 2.4  1999/07/14 12:11:16  sbs
 * Major re-implementation of IVE!
 * - stacking of ACTCHNs for proper handling of CONDs and LOOPs
 * - iv += 3 for non-maximal iv's are handled correctly now
 * - proper arg_info usage added
 * - ivs of WLs can be eliminated now
 * - some code streamlining, e.g. proper usage of MakeIcm...
 *
 * Revision 2.3  1999/07/07 06:02:56  sbs
 * changed vardec chains into $-stacked chaines.
 * Appropriate handling of cond/ do / while is not yet included.
 *
 * Revision 2.2  1999/04/14 16:23:05  jhs
 * Traversal into NULL with empty arrays removed.
 *
 * Revision 2.1  1999/02/23 12:43:12  sacbase
 * new release made
 *
 * Revision 1.28  1998/12/07 13:45:23  sbs
 * IdxArg patched due to a problem when compiling:
 * int f(int[] v, int[]a)
 * {
 *   return(a[2*v]);
 * }
 *
 * instead of re-using VINFO_VARDEC( vinfo) from an argument
 * the decl was searched within the existing vardecs.
 *
 * Revision 1.27  1998/11/08 15:08:45  dkr
 * IdxNcode:
 *   An assert added to assure non-empty NCODE_CBLOCK fields
 *
 * Revision 1.26  1998/08/21 12:40:41  sbs
 * some dummy assignments in some default-cases inserted
 * for convincing the C compiler that these vars indeed
 * are initialized in any case!
 *
 * Revision 1.25  1998/08/06 21:24:31  dkr
 * fix a bug in IdxArg
 *
 * Revision 1.24  1998/08/06 17:18:21  dkr
 * changed signature of ICM ND_KS_VECT2OFFSET
 *
 * Revision 1.23  1998/07/16 13:47:35  sbs
 * the arraytype for WLs (modarray + genarray)
 * is now deduced from the LHS of the previous Let-Node
 * rather than ( in the old genarray-case) from the type!!!
 * of the first arg of the operation part !!!
 *
 * Revision 1.22  1998/06/07 18:37:05  dkr
 * ChgId renamed in IdxChangeId
 *
 * Revision 1.21  1998/06/06 13:55:30  dkr
 * changed ChdId():
 *   switched to new variable names.
 *
 * Revision 1.20  1998/05/18 08:16:10  srs
 * fixed bug in IdxPrf(), F_modarray
 *
 * Revision 1.19  1998/05/07 16:19:44  dkr
 * added support for new with-loop
 *
 * Revision 1.18  1998/05/04 19:42:18  dkr
 * added StringCopy(...) ...
 *    ... to first argument of each MakeId(...),
 *    ... to first arguemnt of MakeVardec(...) in VardecIdx(), ArgIdx(),
 *    ...
 * to get rid of shared NAME strings
 *
 * Revision 1.17  1998/05/04 17:46:21  sbs
 * LET_VARDEC(act_let)= VardecIdx( LET_VARDEC(act_let),
 *                                         VINFO_TYPE( vinfo),
 *                                         LET_NAME(act_let));
 *
 * inserted in IdxLet
 * this should guarantee that the backref from LHS vars to their
 * declaration is established!
 *
 * Revision 1.16  1997/11/23 18:00:23  dkr
 * in ChgId(): strdup -> StringCopy (strdup do not use malloc !!)
 *
 * Revision 1.15  1997/11/05 11:30:48  dkr
 * removed a bug with NEWTREE
 *
 * Revision 1.14  1997/11/04 13:22:46  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.13  1997/05/29 13:41:08  sbs
 * IVE for modarray integrated
 *
 * Revision 1.12  1997/05/02  15:40:54  sbs
 * IdxId reverted but IdxPrf case Psi adapted for arrays of unknown shape
 *
 * Revision 1.11  1997/05/02  14:38:31  sbs
 * IdxId : set vect   also if the array (of a psi application for example)
 * is of unknown shape!!
 * That can happen if an external function is involved that returns an
 * array of unknown shape, e.g. FibreScanIntArray.
 *
 *
 * ... [eliminated] ...
 *
 *
 * Revision 1.1  1995/06/02  10:06:56  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"
#include "tree.h"
#include "print.h"
#include "traverse.h"
#include "DupTree.h"
#include "index.h"
#include "psi-opt.h"
#include "free.h"

#include "access_macros.h"

#include "compile.h"

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

/*
 * 1) Basics
 * ---------
 *
 * The "index optimization" tries to eliminate index vectors which are
 * only used for array selections.
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
 *            z = idx_psi(a, __i_4_4);
 *
 * For doing so, an attribute "Uses" has to be infered. It is attached to each
 * left hand side of an array assignment, and to each variable/argument declaration
 * of an array.
 * Since we want to eliminate index vectors the attribute attachment is restricted
 * to one-dimensional integer arrays( array identifiers) !
 * The "Uses" attribute consists of a set (chain) of attributes of the kind:
 *
 *     VECT/ IDX(<shape>).
 *
 * In the above example we obtain:
 *
 *            int z;
 *            int[2] i:IDX(int[4,4]) ;
 *            int[4, 4] a;
 *
 *            a= reshape([4,4], [1,2,...,16]);
 *            i:IDX(int[4,4]) = [2,3];
 *            z= a[i];
 *
 * BTW, this can be made visible by using the break option that stops compilation
 * after IVE is done (at the time being this is -b16)!
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
 * During the (bottom up) traversal all information is kept at the N_vardec/N_arg nodes.
 * This is realized by the introduction of a new node: "N_vinfo"
 * which is organized as follows:
 *
 *     info.use (useflag): VECT / IDX
 *     node[0]           : next "N_vinfo node" if existent
 *     node[1]           : withbelonging type if info.use == IDX
 *
 * it can be accessed by:
 *     VINFO_FLAG(n),
 *     VINFO_NEXT(n), and          ( cf. tree_basic.h )
 *     VINFO_TYPE(n)
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
 * the respective IDX(...) attribute is added (without duplicates) to both chains
 * of the variable's declaration.
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
 * The mechanism described so far does not properly support programs that contain
 * program parts that may or may not be "executed", i.e., conditionals or loops.
 * Since we want to replace all psi ops by idx_psi ops, we have to infer all
 * potential uses rather than only those taken by a particular branch. To achieve
 * this, different actual chains for the alternative branches have to be created
 * and merged, e.g.,
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
 * have to be merged, thus yielding IDX(int[8,2]):IDX(int[4,4]) as "actual chain"
 * when leaving the conditional.
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
 * For loops a similar mechanism is rquired; we have to stack the "actual chain"
 * of the assignments that follow the loop, and we have to merge those with
 * the "actual chain" obtained from traversing the loop body.
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
 *                              trv( L; R; ++ R;) $    delete topmost chain & leave loop
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
 *                              trv( L; R;) $    delete topmost chain & leave loop
 *
 * Implementation-wise, we face another problem with loops here:
 * Since the code is changed in the same traversal as the uses attributes are built
 * during the first traversal of loops these changes have to be prevented. This is
 * indicated by a mode-flag in INFO_IVE_MODE( arg_info) which either is set to
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
 * leads to the N_arg node from which we can not reach the function's vardec-section.
 * Therefore, we first traverse a function's arguments and insert backrefs to the
 * N_fundef node. This enables us to find the vardec-section when creating
 * new vardecs for variables which are formal parametes of the function.
 *
 *
 *
 * 4) Inserting new Assignments
 * ----------------------------
 *
 * Apart from the two traversals due to loops ( see above) the insertion of new
 * assignments is done in the same traversal during which the uses-inference is done.
 * The central function for initiating this process id IdxLet. Basically, for each
 * of the variables <var> of the LHS and for each IDX(<shp>) usage, an assignment of the
 * form __<varname>__<shp> = ComputeIdxFromShape( <varname>, <shp>) is inserted.
 * This is achieved by generating an ND_KS_VECT2OFFSET icm.
 * Complementary to this code modification, all F_psi and all F_modarray operations
 * are replaced by their idx counterparts, e.g., an application psi( iv, a) is
 * replaced by idxpsi( __iv__<shp>, a).
 * To avoid superfluous computations of this form, so-called "pure address computations"
 * are identified. These are assignments of the form
 *
 *    iv = iv1;             or of the form
 *    iv = exp1 op exp2;
 * where
 *    iv only has IDX(...) usages and
 *    op is +,-,*,\    (without *,\ between two vectors)
 *
 * These assignments propagate the uses flags and lead to new assignments of the form
 *
 *    __iv__<shp> = __iv1__<shp>;
 *    __iv__<shp> = Vect2Off( iv1) op Vect2Off( iv2);
 * where
 *    Vect2Off( exp) =
 *
 *    / __iv__<shp>                                             iff exp == iv
 *   |  (.((s *shp1 + s )*shp2 + s )...)*shp(n-1) + s           iff exp == s  scalar
 *    \ (.((s0*shp1 + s1)...)*shpm + sm)*shp(m+1)...*shp(n-1)   otherwise, i.e.
 *                                                              exp == [s0, ..., sm]
 *
 * The CENTRAL MECHANISM to achieve these transformations is steered by IdxLet.
 * Prior to the traversal of a RHS IdxLet copies the actual assignment (if required)
 * and then traverses the RHS with INFO_IVE_TRANSFORM_VINFO( arg_info) either
 * containing a pointer to the N_vinfo node containing the shape information required
 * for the transformation process or being NULL in order to indicate that the vector
 * version should be kept as it is.
 * All Idx functions that will be called during that traversal, i.e.
 *     IdxId, IdxNum, IdxPrf, IdxArray, IdxWith, and IdxNwith
 * are steered by that mechanism.
 *
 *
 * 5) Eliminating Superfluous Vardecs
 * ----------------------------------
 *
 * Since DCR will not be run after IVE, IVE tries to eliminate superfluous vardecs
 * of vectors which were successfully eliminated by IVE. Since the original vardecs
 * are needed during the traversal of the body, the vardec elimination is done after
 * traversing the bodies


 */
/*
 * Thus, we finally find the following usages of the arg_info node:
 *
 *    INFO_IVE_FUNDEF  - holds pointer to actual N_fundef
 *                     - is needed for creating backrefs of arg-nodes.
 *    INFO_IVE_VARDECS - holds pointer to the topmost vardec of the function
 *                     - is needed for traversing all vardecs in loops / conditionals
 *    INFO_IVE_MODE    - indicates whether we are interested in uses only (M_uses_only)
 *                       or in code transformations as well (M_uses_and_transform)
 *    INFO_IVE_CURRENTASSIGN - holds the current assign node
 *                           - needed for inserting new assignments and in WLs
 *    INFO_IVE_TRANSFORM_VINFO - indicates whether N_nums, N_prfs, and N_arrays
 *                               should be transformed or not.
 *    INFO_IVE_NON_SCAL_LEN - needed for F_add_SxA, F_add_AxS, F_sub_SxA, and F_sub_AxS
 *                            if the are used for pure iv address computations on ivs
 *                            of non-maximal length! In these situations the length
 *                            of the non-scalar argument has to be known when traversing
 *                            the scalar argument (cf. IdxNum).
 */

/*
 * Some functions for handling N_vinfo nodes:
 * ------------------------------------------
 * node * FindVect( node *N_vinfo_chain)	 : checks whether VECT is in the chain
 *						   of N_vinfo nodes
 * node * FindIdx( node *chain, types *shape)	 : dito for a specific shape
 *
 * node * SetVect( node *chain)			 : inserts VECT-node in chain if
 *                                                 not already present
 * node * SetIdx( node *chain, types *shape)	 : inserts IDX-node if not already present
 */

/*
 *
 *  functionname  : FindVect
 *  arguments     : 1) node * chain
 *  description   : checks whether VECT is in the chain and returns
 *                  either a vinfo-node with DOLLAR-flag (= no VECT in chain)
 *                  or the adress of the VECT-node
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
FindVect (node *chain)
{
    DBUG_ENTER ("FindVect");
    while (VINFO_FLAG (chain) == IDX)
        chain = VINFO_NEXT (chain);
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : EqTypes
 *  arguments     : 1) types * type1
 *                  2) types * type2
 *  description   : compares two types with respect to the shape and
 *                  returnes 1 iff the types are equal, 0 otherwise.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : this is a helper function needed from FindIdx only!
 *
 */

int
EqTypes (types *type1, types *type2)
{
    int i, res;

    DBUG_ENTER ("EqTypes");
    if (TYPES_DIM (type1) == TYPES_DIM (type2)) {
        res = 1;
        DBUG_ASSERT ((TYPES_SHPSEG (type1) != NULL) && (TYPES_SHPSEG (type1) != NULL),
                     "EqTypes used on type without shape");
        for (i = 0; i < TYPES_DIM (type2); i++)
            if (TYPES_SHAPE (type1, i) != TYPES_SHAPE (type2, i))
                res = 0;
    } else {
        res = 0;
    }
    DBUG_RETURN (res);
}

/*
 *
 *  functionname  : FindIdx
 *  arguments     : 1) node * chain
 *                  2) types * idx-shape
 *  description   : checks whether IDX(idx-shape) is in the chain and returns
 *                  either NULL (= IDX(idx-shape) not in chain or the adress
 *                  of the IDX-node
 *  global vars   : ---
 *  internal funs : EqTypes
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
FindIdx (node *chain, types *vshape)
{
    DBUG_ENTER ("FindIdx");
    while ((VINFO_FLAG (chain) != DOLLAR)
           && ((VINFO_FLAG (chain) != IDX) || !EqTypes (VINFO_TYPE (chain), vshape)))
        chain = VINFO_NEXT (chain);
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : SetVect
 *  arguments     : 1) node * chain
 *  description   : inserts a Vect node in the given node-chain if there exists none
 *  global vars   : ---
 *  internal funs : FindVect, GenVect, InsertInChain
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
SetVect (node *chain)
{
    DBUG_ENTER ("SetVect");
    DBUG_PRINT ("IDX", ("VECT assigned"));
    if (VINFO_FLAG (FindVect (chain)) == DOLLAR) {
        chain = MakeVinfo (VECT, NULL, chain, VINFO_DOLLAR (chain));
    }
    DBUG_RETURN (chain);
}

/*
 *
 *  functionname  : SetIdx
 *  arguments     : 1) node * chain
 *                  2) types * shape to be inserted
 *  description   : inserts an IDX(shape) node in the given node-chain
 *                  if there exists none
 *  global vars   : ---
 *  internal funs : FindIdx, GenIdx, InsertInChain
 *  external funs : ---
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
SetIdx (node *chain, types *vartype)
{
    DBUG_ENTER ("SetIdx");
    if (VINFO_FLAG (FindIdx (chain, vartype)) == DOLLAR) {
        chain = MakeVinfo (IDX, vartype, chain, VINFO_DOLLAR (chain));
        DBUG_PRINT ("IDX", ("IDX(%p) assigned", chain));
    }
    DBUG_RETURN (chain);
}

/*
 *
 * Some basic functions for handling N_vinfo chains:
 * -------------------------------------------------
 * node * CutVinfoChn( node * chain)          : cuts off the topmost chain
 *                                              and returns the rest
 * node * AppendVinfoChn( node *ca, node *cb) : links the chain cb to the topmost
 *                                              chain of ca. Expects ca to contain
 *                                              one chain only!
 * node * MergeVinfoChn( node *ca, node *cb)  : merges the entries of the
 *                                              topmost chain from ca with
 *                                              the elements of the topmost
 *                                              chain of cb and returns it
 *                                              followed by the "rest" of cb.
 *                                              Expects ca to contain one chain
 *                                              only!
 *
 ******************************************************************************
 *
 * function:
 *  node *CutVinfoChn( node * chain)
 *
 * description:
 *   if we give a cinfo-chain as argument, e.g.
 *     VECT : IDX([2,2]) : $ : VECT : $
 *   CutVinfoChn cuts off the first list by setting the NEXT pointer of the first
 *   $-symbol to NULL, and returns a pointer to the rest of the chain, e.g.
 *                             VECT : $
 *
 ******************************************************************************/

node *
CutVinfoChn (node *chain)
{
    node *rest;

    DBUG_ENTER ("CutVinfoChn");
    DBUG_ASSERT ((VINFO_DOLLAR (chain) != NULL), "Dollar-ref in vinfo-chain missing!");
    rest = VINFO_NEXT (VINFO_DOLLAR (chain));
    VINFO_NEXT (VINFO_DOLLAR (chain)) = NULL;
    DBUG_RETURN (rest);
}

/******************************************************************************
 *
 * function:
 *  node *AppendVinfoChn( node * ca, node * cb)
 *
 * description:
 *   Expects ca to contain one chain only. If that assumption holds cb is appended
 *   to ca.
 *
 ******************************************************************************/

node *
AppendVinfoChn (node *ca, node *cb)
{
    DBUG_ENTER ("AppendVinfoChn");
    DBUG_ASSERT ((VINFO_DOLLAR (ca) != NULL), "Dollar-ref in vinfo-chain missing!");
    DBUG_ASSERT ((VINFO_NEXT (VINFO_DOLLAR (ca)) == NULL),
                 "AppendVinfoChn called with a first argument"
                 " containing more than one chain!");
    VINFO_NEXT (VINFO_DOLLAR (ca)) = cb;
    DBUG_RETURN (ca);
}

/******************************************************************************
 *
 * function:
 *  node *MergeVinfoChn( node * ca, node * cb)
 *
 * description:
 *   Expects ca to contain one chain only. If that assumption holds the entries
 *   of that chain are merged into the topmost chain of cb.
 *
 ******************************************************************************/

node *
MergeVinfoChn (node *ca, node *cb)
{
    DBUG_ENTER ("MergeVinfoChn");
    DBUG_ASSERT ((VINFO_DOLLAR (ca) != NULL), "Dollar-ref in vinfo-chain missing!");
    DBUG_ASSERT ((VINFO_NEXT (VINFO_DOLLAR (ca)) == NULL),
                 "AppendVinfoChn called with a first argument"
                 " containing more than one chain!");
    while (VINFO_FLAG (ca) != DOLLAR) {
        if (VINFO_FLAG (ca) == VECT)
            cb = SetVect (cb);
        else
            cb = SetIdx (cb, VINFO_TYPE (ca));
        ca = VINFO_NEXT (ca);
    }

    DBUG_RETURN (cb);
}

/*
 *
 * functions that are applied to the entire vardec chain:
 * ------------------------------------------------------
 *
 * node *DuplicateTop( node * actchn)
 * node *SwitchTop( node * actchn)
 * node *MergeTop( node * actchn)
 * node *FreeTop( node * actchn)
 * node *MergeCopyTop( node * actchn)
 *
 * all these functions can be applied to the entire vardec-chain
 * by using the following "higher-order-function":
 */

#define MAP_TO_ALL_VARDEC_ACTCHNS(fun, vardecin)                                         \
    {                                                                                    \
        node *vardec, *actchn;                                                           \
                                                                                         \
        vardec = vardecin;                                                               \
        while (vardec != NULL) {                                                         \
            actchn = VARDEC_ACTCHN (vardec);                                             \
            if (actchn != NULL) {                                                        \
                DBUG_PRINT ("IVE", ("actchn of %s:", VARDEC_NAME (vardec)));             \
                VARDEC_ACTCHN (vardec) = fun (actchn);                                   \
            }                                                                            \
            vardec = VARDEC_NEXT (vardec);                                               \
        }                                                                                \
    }

/******************************************************************************
 *
 * function:
 *  node *DuplicateTop( node * actchn)
 *
 * description:
 *
 ******************************************************************************/

node *
DuplicateTop (node *actchn)
{
    node *copy, *restchn, *res;

    DBUG_ENTER ("DuplicateTop");
    restchn = CutVinfoChn (actchn);
    copy = DupTree (actchn, NULL);
    res = AppendVinfoChn (copy, AppendVinfoChn (actchn, restchn));

    DBUG_PRINT ("IVE", ("DuplicateTop yields:"));
    DBUG_EXECUTE ("IVE", Print (res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node *SwitchTop( node * actchn)
 *
 * description:
 *
 ******************************************************************************/

node *
SwitchTop (node *actchn)
{
    node *scndchn, *restchn, *res;

    DBUG_ENTER ("SwitchTop");
    DBUG_PRINT ("IVE", ("SwitchTop yields:"));

    scndchn = CutVinfoChn (actchn);
    restchn = CutVinfoChn (scndchn);
    res = AppendVinfoChn (scndchn, AppendVinfoChn (actchn, restchn));

    DBUG_EXECUTE ("IVE", Print (res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node *MergeTop( node * actchn)
 *
 * description:
 *
 ******************************************************************************/

node *
MergeTop (node *actchn)
{
    node *restchn, *res;

    DBUG_ENTER ("MergeTop");
    DBUG_PRINT ("IVE", ("MergeTop yields:"));

    restchn = CutVinfoChn (actchn);
    res = MergeVinfoChn (actchn, restchn);
    FreeTree (actchn);

    DBUG_EXECUTE ("IVE", Print (res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node *FreeTop( node * actchn)
 *
 * description:
 *
 ******************************************************************************/

node *
FreeTop (node *actchn)
{
    node *res;

    DBUG_ENTER ("FreeTop");
    DBUG_PRINT ("IVE", ("FreeTop yields:"));

    res = CutVinfoChn (actchn);
    FreeTree (actchn);

    DBUG_EXECUTE ("IVE", Print (res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node *MergeCopyTop( node * actchn)
 *
 * description:
 *
 ******************************************************************************/

node *
MergeCopyTop (node *actchn)
{
    node *restchn, *scndchn, *res;

    DBUG_ENTER ("MergeCopyTop");
    DBUG_PRINT ("IVE", ("MergeCopyTop yields:"));

    scndchn = CutVinfoChn (actchn);
    restchn = CutVinfoChn (scndchn);
    res = AppendVinfoChn (MergeVinfoChn (actchn, scndchn),
                          AppendVinfoChn (actchn, restchn));

    DBUG_EXECUTE ("IVE", Print (res););
    DBUG_RETURN (res);
}

/*
 * "helper" functions used by the traversal functions:
 * ---------------------------------------------------
 *
 *  char *IdxChangeId( char *varname, types *type)   for creating "shapely" names
 *                                                   for indexing vectors
 *
 *  node *VardecIdx(node *vardec, types *type)       for finding / creating V_vardecs
 *                                                   for "shapely" iv's.
 *
 *  node *CreateVect2OffsetIcm(node *vardec, types *type)  for creating Vect2Offset
 *                                                   that initialize "shapely" iv's.
 */

/******************************************************************************
 *
 * function:
 *  char *IdxChangeId( char *varname, types *type)
 *
 * description: appends the shape given by type to the varname; e.g:
 *              test, int[1,4,2,3]  =>  test_1_4_2_3__
 *              does not free the argument space!
 *
 ******************************************************************************/

char *
IdxChangeId (char *varname, types *type)
{
    static char buffer[1024];
    static char buffer2[32];
    int i;
    int *shp;

    DBUG_ENTER ("IdxChangeId");
    sprintf (buffer, "%s", varname);
    shp = SHAPES_SELEMS (type);
    for (i = 0; i < TYPES_DIM (type); i++) {
        sprintf (buffer2, "_%d", shp[i]);
        strcat (buffer, buffer2);
    }
    sprintf (buffer2, "__");
    strcat (buffer, buffer2);
    DBUG_RETURN (StringCopy (buffer));
}

/******************************************************************************
 *
 * function:
 *  node *VardecIdx(node *vardec, types *type)
 *
 * description: vardec points to the N_vardec/ N_arg node of the original
 *    declaration, i.e. the "VECT"-version, of an index variable.
 *    VardecIdx looks up, whether there already exists a declaration of the
 *    "IDX(type)" variant. If so, the pointer to that declaration is returned,
 *    otherwise a new declaration is created and a pointer to it is returned.
 *
 ******************************************************************************/

node *
VardecIdx (node *vardec, types *type)
{
    node *newvardec, *vinfo, *block;
    char *varname;

    DBUG_ENTER ("VardecIdx");

    vinfo = FindIdx (VARDEC_OR_ARG_COLCHN (vardec), type);
    DBUG_ASSERT ((VINFO_FLAG (vinfo) != DOLLAR),
                 "given shape not inserted in collected chain of vardec node!");
    if (VINFO_VARDEC (vinfo) == NULL) {
        /*
         * A vardec does not yet exist !
         */
        varname = IdxChangeId (VARDEC_OR_ARG_NAME (vardec), type);
        if (NODE_TYPE (vardec) == N_vardec) {
            newvardec = MakeVardec (varname, MakeType (T_int, 0, NULL, NULL, NULL),
                                    VARDEC_NEXT (vardec));
            VARDEC_NEXT (vardec) = newvardec;
        } else {
            block = FUNDEF_BODY (ARG_FUNDEF (vardec));
            newvardec = MakeVardec (varname, MakeType (T_int, 0, NULL, NULL, NULL),
                                    BLOCK_VARDEC (block));
            BLOCK_VARDEC (block) = newvardec;
        }
        VINFO_VARDEC (vinfo) = newvardec;
        DBUG_PRINT ("IDX",
                    ("inserting new vardec %s between %s and %s", varname,
                     ((NODE_TYPE (vardec) == N_vardec) ? VARDEC_NAME (vardec) : "NULL"),
                     (VARDEC_NEXT (newvardec) ? VARDEC_NAME (VARDEC_NEXT (newvardec))
                                              : "NULL")));
    }

    DBUG_PRINT ("IDX", ("vinfo %p points to vardec %s", vinfo,
                        VARDEC_NAME (VINFO_VARDEC (vinfo))));

    DBUG_RETURN (VINFO_VARDEC (vinfo));
}

/******************************************************************************
 *
 * function:
 *  node *CreateVect2OffsetIcm(node *vardec, types *type)
 *
 * description: vardec points to the N_vardec/ N_arg node of the original
 *    declaration, i.e. the "VECT"-version, of an index variable.
 *    type indicates which IDX(type) version has to be computed.
 *    CreateVect2OffsetIcm creates the required  ND_KS_VECT2OFFSET - ICM, e.g.,
 *    if   vardec ->  int[2] iv    and   type -> double [4,5,6],
 *    an icm ND_KS_VECT2OFFSET( iv_4_5_6__, iv, 2, 3, 4, 5, 6)  is created.
 *    While doing so it makes sure, that a vardec for iv_3_4_5__ exists and
 *    it insertes back-refs from the N_id nodes of the icm to the respective
 *    vardecs!
 *    Finally, the index variable is marked as VECT since the vector is
 *    needed for this ICM!! (This is crucial for the elimination of superfluous
 *    vector declarations!
 *
 ******************************************************************************/

node *
CreateVect2OffsetIcm (node *vardec, types *type)
{
    node *exprs, *icm, *iv_off_id, *iv_vect_id;
    char *iv_name;
    int i;

    DBUG_ENTER ("CreateVect2OffsetIcm");
    /*
     * First, we create an N_exprs-chain containing the shape of type!
     */
    exprs = NULL;
    for (i = TYPES_DIM (type) - 1; i >= 0; i--) {
        exprs = MakeExprs (MakeNum (TYPES_SHAPE (type, i)), exprs);
    }

    /*
     * Now, we create two N_id nodes containing the name of the iv itself
     * and containing the shapely version of iv, respectively.
     */
    iv_name = VARDEC_OR_ARG_NAME (vardec);

    iv_off_id = MakeId (IdxChangeId (iv_name, type), NULL, ST_regular);
    ID_VARDEC (iv_off_id) = VardecIdx (vardec, type);

    iv_vect_id = MakeId (StringCopy (iv_name), NULL, ST_regular);
    ID_VARDEC (iv_vect_id) = vardec;

    /*
     * Now, we create the desired icm:
     */
    icm = MakeIcm5 ("ND_KS_VECT2OFFSET", iv_off_id, iv_vect_id,
                    MakeNum (VARDEC_OR_ARG_SHAPE (vardec, 0)), MakeNum (TYPES_DIM (type)),
                    exprs);

    /*
     * Finally, we mark vardec as VECT!
     */

    SET_VARDEC_OR_ARG_COLCHN (vardec, SetVect (VARDEC_OR_ARG_COLCHN (vardec)));

    DBUG_RETURN (MakeAssign (icm, NULL));
}

/******************************************************************************
 ***
 ***          Here, the traversal functions start!
 ***          ------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    node *IdxModul(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
IdxModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxModul");
    if (NULL != MODUL_FUNS (arg_node))
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *IdxFundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
IdxFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxFundef");
    /*
     * We have to traverse the args first, since we need backrefs
     * to this fundef-node.
     * For doing so we supply the fundef-node in INFO_IVE_FUNDEF( arg_info).
     */
    if (NULL != FUNDEF_ARGS (arg_node)) {
        INFO_IVE_FUNDEF (arg_info) = arg_node;
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /*
     * The vardec stacking mechanism needed for conditionals and loops
     * requires knowledge of the topmost vardec node which is kept in
     * INFO_IVE_VARDECS( arg_info).
     * Furthermore, the default mode has to be set to M_uses_and_transform!
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_IVE_VARDECS (arg_info) = FUNDEF_VARDEC (arg_node);
        INFO_IVE_MODE (arg_info) = M_uses_and_transform;
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * A second pass through the arguments has to be done in order
     * to insert initialisations for index-args. This can NOT be done
     * when traversing the body since the traversal mechanism would
     * automatically eliminate the freshly inserted assign-nodes.
     * The second pass is indicated to IdxArg by a NULL arg_info!
     */
    if (NULL != FUNDEF_ARGS (arg_node))
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), NULL);

    if (NULL != FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *IdxArg(node *arg_node, node *arg_info)
 *
 * description: Depending on the existance of arg_info, 2 different things happen:
 *   Iff( arg_info) != NULL, backrefs to the actual fundef are inserted and
 *      for integer vectors the ACTCHNs and the COLCHNs are initialized.
 *   Otherwise, the existing COLCHNs are traversed and missing Vardecs of
 *      IDX(type) versions are added.
 *
 ******************************************************************************/

node *
IdxArg (node *arg_node, node *arg_info)
{
    node *newassign, *block, *vinfo;
    int dim;

    DBUG_ENTER ("IdxArg");

    if (arg_info != NULL) {
        /* This is the first pass; insert backref to fundef
         * and make sure that all ARG_COLCHN-nodes are properly
         * instanciated before traversing the body!
         */
        ARG_FUNDEF (arg_node) = INFO_IVE_FUNDEF (arg_info);
        dim = ARG_DIM (arg_node);
        if ((ARG_BASETYPE (arg_node) == T_int)
            && ((dim == 1) || (dim == KNOWN_DIM_OFFSET - 1))) {
            ARG_ACTCHN (arg_node) = MakeVinfoDollar (NULL);
            ARG_COLCHN (arg_node) = MakeVinfoDollar (NULL);
        }
    } else {
        /* This is the second pass; insert index-arg initialisations
         * of the form: ND_KS_VECT2OFFSET( <off-name>, <var-name>, <dim>, <dims>, <shape>)
         */
        block = FUNDEF_BODY (ARG_FUNDEF (arg_node));
        if (block != NULL) { /* insertion not necessary for external decls! */
            vinfo = ARG_COLCHN (arg_node);
            while ((vinfo != NULL) && (VINFO_FLAG (vinfo) != DOLLAR)) {
                /* loop over all "Uses" attributes */
                if (VINFO_FLAG (vinfo) == IDX) {
                    newassign = CreateVect2OffsetIcm (arg_node, VINFO_TYPE (vinfo));

                    ASSIGN_NEXT (newassign) = BLOCK_INSTR (block);
                    BLOCK_INSTR (block) = newassign;
                }
                vinfo = VINFO_NEXT (vinfo);
            }
        }
    }
    if (NULL != ARG_NEXT (arg_node))
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * IdxBlock( node *arg_node, node *arg_info )
 *
 * description:
 *   Make sure that the vardecs are traversed BEFORE the body!
 *   NB: this is done to initialize all int[x] / int[.] vars by DOLLAR!
 *   After the body has been traversed, a sceond traversal through the
 *   vardecs is made in order to eliminate superfluous vardecs of index
 *   vectors that are no longer needed, i.e., which do not have VECT
 *   in their COLCHN.
 *
 ******************************************************************************/

node *
IdxBlock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxBlock");

    /*
     * First pass through the vardecs: all int[.] vars are initialized by $!
     */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    /*
     * Second pass through the vardecs: we eliminate all superfluous int[.]
     * vardecs. This is indicated by arg_info == NULL !
     */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), NULL);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * IdxVardec( node *arg_node, node *arg_info )
 *
 * description:
 *   if( arg_info != NULL)
 *       create an N_vinfo node for all vars with either type int[x]
 *       or type int[.] (needed for the AKD-case!).
 *   else
 *       eliminate all those vardecs, whose COLCHNs do not contain VECT.
 *
 ******************************************************************************/

node *
IdxVardec (node *arg_node, node *arg_info)
{
    int dim;
    node *next;

    DBUG_ENTER ("IdxVardec");

    if (arg_info != NULL) {
        /*
         * This is the first traversal which initializes the int[.]
         * vardecs with $!
         */
        dim = VARDEC_DIM (arg_node);
        if ((VARDEC_BASETYPE (arg_node) == T_int)
            && ((dim == 1) || (dim == KNOWN_DIM_OFFSET - 1))) {
            /* we are dealing with a potential indexing vector ! */
            VARDEC_ACTCHN (arg_node) = MakeVinfoDollar (NULL);
            VARDEC_COLCHN (arg_node) = MakeVinfoDollar (NULL);
        }

        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
        }
    } else {
        /*
         * This is the second traversal which is done after traversing the body
         * and is used to eliminate index vectors which are no longer needed.
         * As criterium for need the existance of VECT in the COLCHN is taken.
         * I hope that this is sufficient, since I add VECT whenever Vect2Offset
         * is introduced ( see "CreateVect2OffsetIcm").
         */
        dim = VARDEC_DIM (arg_node);
        if ((VARDEC_BASETYPE (arg_node) == T_int)
            && ((dim == 1) || (dim == KNOWN_DIM_OFFSET - 1))
            && (VINFO_FLAG (FindVect (VARDEC_COLCHN (arg_node))) == DOLLAR)) {
            /*
             * we are dealing with an indexing vector that is not
             * needed as a vector anymore!
             */
            next = VARDEC_NEXT (arg_node);
            VARDEC_NEXT (arg_node) = NULL;
            FreeTree (arg_node);
            if (next != NULL)
                arg_node = Trav (next, arg_info);
            else
                arg_node = NULL;
        } else {
            if (VARDEC_NEXT (arg_node) != NULL) {
                VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * IdxAssign( node *arg_node, node *arg_info )
 *
 * description:
 *
 ******************************************************************************/

node *
IdxAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxAssign");

    /* Bottom up traversal!! */
    if (NULL != ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }
    /* store the current N_assign in INFO_IVE_CURRENTASSIGN */
    INFO_IVE_CURRENTASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *IdxReturn(node *arg_node, node *arg_info)
 *
 * description:
 *   initiates the uses-collection. In order to guarantee a "VECT" attribution
 *   for array-variables, INFO_IVE_TRANSFORM_VINFO( arg_info) has to be NULL,
 *   when traversing the return expressions!
 *
 ******************************************************************************/

node *
IdxReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxReturn");
    INFO_IVE_TRANSFORM_VINFO (arg_info) = NULL;
    RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *IdxLet(node *arg_node, node *arg_info)
 *
 * description: This is the central mechanism for steering the code
 *    transformation. It Duplicates assignments - if required - and adds
 *    the transformation information into arg_info.
 *
 ******************************************************************************/

node *
IdxLet (node *arg_node, node *arg_info)
{
    ids *vars;
    node *vardec, *vinfo, *act_let, *newassign;
    node *current_assign, *next_assign, *chain, *rest_chain;

    DBUG_ENTER ("IdxLet");
    /*
     * First, we attach the collected uses-attributes( they are in the
     * actual chain of the vardec) to the variables of the LHS of the
     * assignment!
     */
    vars = LET_IDS (arg_node);
    do {
        vardec = IDS_VARDEC (vars);
        if (NODE_TYPE (vardec) == N_vardec) {
            if (VARDEC_COLCHN (vardec) != NULL) {
                /* So we are dealing with a potential index var! */
                chain = VARDEC_ACTCHN (vardec);
                IDS_USE (vars) = chain;
                rest_chain = CutVinfoChn (chain);
                /* Now, re-initialize the actual chain! */
                VARDEC_ACTCHN (vardec) = MakeVinfoDollar (rest_chain);
            }
        } else {
            DBUG_ASSERT ((NODE_TYPE (vardec) == N_arg),
                         "backref from let-var neither vardec nor arg!");
            if (ARG_COLCHN (vardec) != NULL) {
                /* So we are dealing with a potential index var! */
                chain = ARG_ACTCHN (vardec);
                IDS_USE (vars) = chain;
                rest_chain = CutVinfoChn (chain);
                /* Now, re-initialize the actual chain! */
                ARG_ACTCHN (vardec) = MakeVinfoDollar (rest_chain);
            }
        }
        vars = IDS_NEXT (vars);
    } while (vars);

    if (INFO_IVE_MODE (arg_info) == M_uses_and_transform) { /* normal run! */

        /* Now, that we have done all that is needed for the "Uses" inference,
         * we do some modifications of the assignment based on the "Uses" information
         * gained.
         * Therefore, we first have to find out, what kind of RHS we are dealing with.
         *  - If it is (an arithmetic operation( +,-,*,\), a variable or a constant),
         *    AND it is neither F_mul_AxA nor F_div_AxA !!!
         *    AND the LHS variable is NOT used as VECT
         *    AND the LHS variable IS used as IDX(...):
         *    we duplicate the assignment for each IDX(shape) and traverse the new
         *    assignments with INFO_IVE_TRANSFORM_VINFO( arginfo) being set to the
         *    N_vinfo that carries IDX(shape)! This traversal will replace
         *    the index-array operations by integer-index operations.The original
         *    (VECT-) version is eliminated (more precisely: reused for the last
         *    IDX(...) version).
         *  - in all other cases:
         *    for each variable of the LHS that is needed as IDX(shape) we
         *    generate an assignment of the form:
         *    ND_KS_VECT2OFFSET( off-name, var-name, dim, dims, shape)
         *    this instruction will calculate the indices needed from the vector
         *    generated by the RHS.
         *    After doing so, we traverse the assignment with
         *    INFO_IVE_TRANSFORM_VINFO( arginfo) being set to NULL.
         */
        vars = LET_IDS (arg_node); /* pick the first LHS variable */
        vinfo = IDS_USE (vars);    /* pick the "Uses"set from the first LHS var */
        if ((vinfo != NULL) && (VINFO_FLAG (FindVect (vinfo)) == DOLLAR)
            && (VINFO_FLAG (vinfo) == IDX)
            && (((NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
                 && (F_add_SxA <= PRF_PRF (LET_EXPR (arg_node)))
                 && (PRF_PRF (LET_EXPR (arg_node)) <= F_div_AxA)
                 && (PRF_PRF (LET_EXPR (arg_node)) != F_mul_AxA)
                 && (PRF_PRF (LET_EXPR (arg_node)) != F_div_AxA))

                || (NODE_TYPE (LET_EXPR (arg_node)) == N_id)
                || (NODE_TYPE (LET_EXPR (arg_node)) == N_array))) {
            DBUG_ASSERT (((NODE_TYPE (LET_EXPR (arg_node)) == N_id)
                          || (NODE_TYPE (LET_EXPR (arg_node)) == N_array)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_add_SxA)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_add_AxS)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_add_AxA)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_sub_SxA)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_sub_AxS)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_sub_AxA)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_mul_SxA)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_mul_AxS)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_div_SxA)
                          || (PRF_PRF (LET_EXPR (arg_node)) == F_div_AxS)),
                         " wrong prf sequence in \"prf_node_info.mac\"!");
            /* Here, we duplicate the operation
             * as many times as we do have different shapes in LET_USE(arg_node)
             * and successively traverse these assignments supplying the resp. N_vinfo
             * in each call's  INFO_IVE_TRANSFORM_VINFO( arg_info)!
             */
            act_let = arg_node;
            while (VINFO_FLAG (vinfo) != DOLLAR) {
                DBUG_ASSERT (((NODE_TYPE (act_let) == N_let)
                              && (NODE_TYPE (INFO_IVE_CURRENTASSIGN (arg_info))
                                  == N_assign)),
                             "wrong let/assign node generated in IdxLet!");
                if (VINFO_FLAG (VINFO_NEXT (vinfo)) != DOLLAR) {
                    /* There are more indices needed, so we have to duplicate the let
                     * node and repeat the let-traversal until there are no shapes left!
                     * More precisely, we have to copy the Assign node, who is the father
                     * of the let node and whose adress is given by arg_info!
                     */
                    current_assign = INFO_IVE_CURRENTASSIGN (arg_info);
                    next_assign = ASSIGN_NEXT (current_assign);
                    ASSIGN_NEXT (current_assign) = NULL;
                    newassign = DupTree (current_assign, NULL);

                    ASSIGN_NEXT (current_assign) = newassign;
                    ASSIGN_NEXT (newassign) = next_assign;
                }
                /* Now, we transform the RHS of act_let ! */
                INFO_IVE_TRANSFORM_VINFO (arg_info) = vinfo;
                LET_EXPR (act_let) = Trav (LET_EXPR (act_let), arg_info);
                /* Make sure we do have a vardec! */
                LET_NAME (act_let) = IdxChangeId (LET_NAME (act_let), VINFO_TYPE (vinfo));
                LET_VARDEC (act_let)
                  = VardecIdx (LET_VARDEC (act_let), VINFO_TYPE (vinfo));
                vinfo = VINFO_NEXT (vinfo);
                DBUG_ASSERT ((vinfo != NULL),
                             " non $-terminated N_vinfo chain encountered!");
                if (VINFO_FLAG (vinfo) != DOLLAR) {
                    INFO_IVE_CURRENTASSIGN (arg_info) = newassign;
                    act_let = ASSIGN_INSTR (newassign);
                }
            }
        } else {
            /* We do not have a "pure" address calculation here!
             * Therefore, we insert for each shape-index needed for each variable of
             * the LHS an assignment of the form :
             * ND_KS_VECT2OFFSET( <off-name>, <var-name>, <dim>, <dims>, <shape> )
             */
            do { /* loop over all LHS-Vars */
                vinfo = IDS_USE (vars);
                vardec = IDS_VARDEC (vars);

                while (vinfo != NULL) { /* loop over all "Uses" attributes */
                    if (VINFO_FLAG (vinfo) == IDX) {
                        newassign = CreateVect2OffsetIcm (vardec, VINFO_TYPE (vinfo));

                        current_assign = INFO_IVE_CURRENTASSIGN (arg_info);
                        ASSIGN_NEXT (newassign) = ASSIGN_NEXT (current_assign);
                        ASSIGN_NEXT (current_assign) = newassign;

                        INFO_IVE_CURRENTASSIGN (arg_info) = newassign;
                    }
                    vinfo = VINFO_NEXT (vinfo);
                }

                /*
                 * Now, we take care of the "VECT" version!
                 * Make sure, that no transformations are done while traversing
                 * the RHS!
                 */
                INFO_IVE_TRANSFORM_VINFO (arg_info) = NULL;

                LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

                vars = IDS_NEXT (vars);
            } while (vars != NULL);
        }

    } else { /* uses inference only! */
        DBUG_ASSERT (INFO_IVE_MODE (arg_info) == M_uses_only,
                     "MODE-flag is neither M_uses_only nod M_uses_and_transform!");
        /*
         * make sure that no transformations are done while traversing
         * the RHS!
         */
        INFO_IVE_TRANSFORM_VINFO (arg_info) = NULL;

        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxPrf
 *  arguments     :
 *  description   : case prf of:
 *                    psi   : SetIdx
 *                    binop : SetVect
 *                    others: SetVect
 *  global vars   : ive_op
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxPrf (node *arg_node, node *arg_info)
{
    node *arg1, *arg2, *arg3, *vinfo;
    types *type;

    DBUG_ENTER ("IdxPrf");
    /*
     * INFO_IVE_TRANSFORM_VINFO( arg_info) indicates whether this is just a
     * normal traverse (NULL), or the transformation of an arithmetic
     * index calculation (N_vinfo-node).
     */
    switch (PRF_PRF (arg_node)) {
    case F_psi:
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);
        DBUG_ASSERT (((arg2->nodetype == N_id) || (arg2->nodetype == N_array)),
                     "wrong arg in F_psi application");
        if (NODE_TYPE (arg2) == N_id)
            type = ID_TYPE (arg2);
        else
            type = ARRAY_TYPE (arg2);
        /*
         * if the shape of the array is unknown, do not(!) replace
         * psi by idx_psi but mark the selecting vector as VECT !
         * this is done by traversal with NULL instead of vinfo!
         */
        if (TYPES_SHPSEG (type) != NULL) {
            vinfo = MakeVinfo (IDX, type, NULL, NULL);
            INFO_IVE_TRANSFORM_VINFO (arg_info) = vinfo;
            PRF_ARG1 (arg_node) = Trav (arg1, arg_info);
            FreeNode (vinfo);
            if (INFO_IVE_MODE (arg_info) == M_uses_and_transform) {
                PRF_PRF (arg_node) = F_idx_psi;
            }
        } else {
            INFO_IVE_TRANSFORM_VINFO (arg_info) = NULL;
            PRF_ARG1 (arg_node) = Trav (arg1, arg_info);
        }
        INFO_IVE_TRANSFORM_VINFO (arg_info) = NULL;
        PRF_ARG2 (arg_node) = Trav (arg2, arg_info);
        break;
    case F_modarray:
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);
        arg3 = PRF_ARG3 (arg_node);
        DBUG_ASSERT (((arg1->nodetype == N_id) || (arg1->nodetype == N_array)),
                     "wrong arg in F_modarray application");
        if (NODE_TYPE (arg1) == N_id)
            type = ID_TYPE (arg1);
        else
            type = ARRAY_TYPE (arg1);
        /*
         * if the shape of the array is unknown, do not(!) replace
         * modarray by idx_modarray but mark the selecting vector as VECT !
         * this is done by traversal with NULL instead of vinfo!
         */
        if (TYPES_SHPSEG (type) != NULL) {
            vinfo = MakeVinfo (IDX, type, NULL, NULL);
            INFO_IVE_TRANSFORM_VINFO (arg_info) = vinfo;
            PRF_ARG2 (arg_node) = Trav (arg2, arg_info);
            FreeNode (vinfo);
            if (INFO_IVE_MODE (arg_info) == M_uses_and_transform) {
                PRF_PRF (arg_node) = F_idx_modarray;
            }
        } else {
            INFO_IVE_TRANSFORM_VINFO (arg_info) = NULL;
            PRF_ARG2 (arg_node) = Trav (arg2, arg_info);
        }
        INFO_IVE_TRANSFORM_VINFO (arg_info) = NULL;
        PRF_ARG1 (arg_node) = Trav (arg1, arg_info);
        PRF_ARG3 (arg_node) = Trav (arg3, arg_info);
        break;
    case F_add_SxA:
        INFO_IVE_NON_SCAL_LEN (arg_info) = ID_SHAPE (PRF_ARG2 (arg_node), 0);
        if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL)
            PRF_PRF (arg_node) = F_add;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        ive_op++;
        break;
    case F_add_AxS:
        INFO_IVE_NON_SCAL_LEN (arg_info) = ID_SHAPE (PRF_ARG1 (arg_node), 0);
    case F_add_AxA:
        if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL)
            PRF_PRF (arg_node) = F_add;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        ive_op++;
        break;
    case F_sub_SxA:
        INFO_IVE_NON_SCAL_LEN (arg_info) = ID_SHAPE (PRF_ARG2 (arg_node), 0);
        if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL)
            PRF_PRF (arg_node) = F_sub;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        ive_op++;
        break;
    case F_sub_AxS:
        INFO_IVE_NON_SCAL_LEN (arg_info) = ID_SHAPE (PRF_ARG1 (arg_node), 0);
    case F_sub_AxA:
        if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL)
            PRF_PRF (arg_node) = F_sub;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        ive_op++;
        break;
    case F_mul_SxA:
        if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL)
            PRF_PRF (arg_node) = F_mul;
        PRF_ARG2 (arg_node) = Trav (PRF_ARG2 (arg_node), arg_info);
        ive_op++;
        break;
    case F_mul_AxS:
        if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL)
            PRF_PRF (arg_node) = F_mul;
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        ive_op++;
        break;
    case F_div_SxA:
        if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL)
            PRF_PRF (arg_node) = F_div;
        PRF_ARG2 (arg_node) = Trav (PRF_ARG2 (arg_node), arg_info);
        ive_op++;
        break;
    case F_div_AxS:
        if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL)
            PRF_PRF (arg_node) = F_div;
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        ive_op++;
        break;
    default:
        DBUG_ASSERT ((INFO_IVE_TRANSFORM_VINFO (arg_info) == NULL),
                     "Inconsistency between IdxLet and IdxPrf");
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxId
 *  arguments     :
 *  description   : examines whether variable is a one-dimensional array;
 *                  if so, examine INFO_IVE_TRANSFORM_VINFO( arg_info):
 *                    if NULL :
 *                        SetVect on "N_vardec" belonging to the "N_id" node.
 *                    otherwise: change varname according to shape from arg_info!
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IdxId (node *arg_node, node *arg_info)
{
    types *type;
    node *vardec;
    char *newid;

    DBUG_ENTER ("IdxId");
    vardec = ID_VARDEC (arg_node);
    DBUG_ASSERT (((NODE_TYPE (vardec) == N_vardec) || (NODE_TYPE (vardec) == N_arg)),
                 "non vardec/arg node as backref in N_id!");
    if (NODE_TYPE (vardec) == N_vardec) {
        if ((VARDEC_BASETYPE (vardec) == T_int)
            && ((VARDEC_DIM (vardec) == 1)
                || (VARDEC_DIM (vardec) == KNOWN_DIM_OFFSET - 1))) {
            if (INFO_IVE_TRANSFORM_VINFO (arg_info) == NULL) {
                DBUG_PRINT ("IDX", ("assigning VECT to %s:", ID_NAME (arg_node)));
                VARDEC_ACTCHN (vardec) = SetVect (VARDEC_ACTCHN (vardec));
                VARDEC_COLCHN (vardec) = SetVect (VARDEC_COLCHN (vardec));
            } else {
                type = VINFO_TYPE (INFO_IVE_TRANSFORM_VINFO (arg_info));
                DBUG_PRINT ("IDX", ("assigning IDX to %s:", ID_NAME (arg_node)));
                VARDEC_ACTCHN (vardec) = SetIdx (VARDEC_ACTCHN (vardec), type);
                VARDEC_COLCHN (vardec) = SetIdx (VARDEC_COLCHN (vardec), type);
                if (INFO_IVE_MODE (arg_info) == M_uses_and_transform) {
                    newid = IdxChangeId (ID_NAME (arg_node), type);
                    FREE (ID_NAME (arg_node));
                    ID_NAME (arg_node) = newid;
                    /* Now, we have to insert the respective declaration */
                    /* If the declaration does not yet exist, it has to be created! */
                    ID_VARDEC (arg_node) = VardecIdx (vardec, type);
                }
            }
        }
    } else {
        if ((ARG_BASETYPE (vardec) == T_int)
            && ((ARG_DIM (vardec) == 1) || (ARG_DIM (vardec) == KNOWN_DIM_OFFSET - 1))) {
            if (INFO_IVE_TRANSFORM_VINFO (arg_info) == NULL) {
                DBUG_PRINT ("IDX", ("assigning VECT to %s:", ID_NAME (arg_node)));
                ARG_ACTCHN (vardec) = SetVect (ARG_ACTCHN (vardec));
                ARG_COLCHN (vardec) = SetVect (ARG_COLCHN (vardec));
            } else {
                type = VINFO_TYPE (INFO_IVE_TRANSFORM_VINFO (arg_info));
                DBUG_PRINT ("IDX", ("assigning IDX to %s:", ID_NAME (arg_node)));
                ARG_ACTCHN (vardec) = SetIdx (ARG_ACTCHN (vardec), type);
                ARG_COLCHN (vardec) = SetIdx (ARG_COLCHN (vardec), type);
                if (INFO_IVE_MODE (arg_info) == M_uses_and_transform) {
                    newid = IdxChangeId (ID_NAME (arg_node), type);
                    FREE (ID_NAME (arg_node));
                    ID_NAME (arg_node) = newid;
                    /* Now, we have to insert the respective declaration */
                    /* If the declaration does not yet exist, it has to be created! */
                    ID_VARDEC (arg_node) = VardecIdx (vardec, type);
                }
            }
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxArray
 *  arguments     : 1) node*: N_array node
 *                  2) node*: INFO_IVE_TRANSFORM_VINFO( arg_info) = vinfo
 *                  R) node*: index of N_array in unrolling of shape
 *  description   : if vinfo == NULL all Array-Elements are traversed with
 *                  INFO_IVE_TRANSFORM_VINFO( arg_info) NULL, since there may be
 *                  some array_variables;
 *                  otherwise the index of the vector N_array in
 *                  the unrolling of an array of shape from vinfo is calculated; e.g.
 *                  [ 2, 3, 1] in int[7,7,7] => 2*49 + 3*7 +1 = 120
 *                  [ 2] in int[7, 7, 7] => 2*49 = 98
 *                  Since we may have identifyers as components, this calculation
 *                  is not done, but generated as syntax-tree!!!
 *                  WARNING!  this penetrates the flatten-consistency!!
 *  global vars   : ive_expr
 *
 */

node *
IdxArray (node *arg_node, node *arg_info)
{
    int i;
    int *shp;
    node *idx;
    node *expr;

    DBUG_ENTER ("IdxArray");
    if (INFO_IVE_TRANSFORM_VINFO (arg_info) == NULL) {
        if (ARRAY_AELEMS (arg_node) != NULL) {
            ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
        }
    } else {
        expr = ARRAY_AELEMS (arg_node);
        shp = VINFO_SELEMS (INFO_IVE_TRANSFORM_VINFO (arg_info));
        idx = EXPRS_EXPR (expr);
        expr = EXPRS_NEXT (expr);
        for (i = 1; i < VINFO_DIM (INFO_IVE_TRANSFORM_VINFO (arg_info)); i++) {
            if (expr != NULL) {
                DBUG_ASSERT ((NODE_TYPE (expr) == N_exprs),
                             "corrupted syntax tree at N_array(N_exprs expected)!");
                idx = MakeExprs (idx, MakeExprs (MakeNum (shp[i]), NULL));
                idx = MakePrf (F_mul, idx);
                idx = MakeExprs (idx, expr);
                expr = EXPRS_NEXT (expr);
                EXPRS_NEXT (EXPRS_NEXT (idx)) = NULL;
                idx = MakePrf (F_add, idx);
            } else {
                idx = MakeExprs (idx, MakeExprs (MakeNum (shp[i]), NULL));
                idx = MakePrf (F_mul, idx);
            }
        }
        arg_node = idx;
        ive_expr++;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IdxNum
 *  arguments     : 1) node*: N_num node
 *                  2) node*: INFO_IVE_TRANSFORM_VINFO( arg_info) = shape
 *                  R) node*: index of N_array in unrolling of shape
 *  description   : if shape == NULL nothing has to be done; otherwise
 *                  the index of the vector N_array in
 *                  the unrolling of an array of shape is calculated;
 *
 */

node *
IdxNum (node *arg_node, node *arg_info)
{
    int val, i, len_iv, dim_array, sum;
    int *shp;

    DBUG_ENTER ("IdxNum");
    if (INFO_IVE_TRANSFORM_VINFO (arg_info) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (INFO_IVE_TRANSFORM_VINFO (arg_info)) == N_vinfo),
                     "corrupted arg_info node in IdxNum!");
        shp = VINFO_SELEMS (INFO_IVE_TRANSFORM_VINFO (arg_info));
        val = NUM_VAL (arg_node);
        dim_array = VINFO_DIM (INFO_IVE_TRANSFORM_VINFO (arg_info));
        len_iv = INFO_IVE_NON_SCAL_LEN (arg_info);

        sum = val;
        for (i = 1; i < len_iv; i++) {
            sum = (sum * shp[i]) + val;
        }
        for (; i < dim_array; i++) {
            sum = sum * shp[i];
        }
        NUM_VAL (arg_node) = sum;
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxWith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
IdxWith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxWith");
    /* Bottom up traversal; the OPERATOR is a N_modarray,
     * N_genarray, N_foldprf, or N_foldfun node.
     * all these nodes do have ordenary N_block-nodes attached!
     */
    WITH_OPERATOR (arg_node) = Trav (WITH_OPERATOR (arg_node), NULL);

    /* When traversing the generator, we potentially want to insert some
     * index conversions into the body; therfore, we supply the
     * N_let node as surplus argument; it does not suffice to
     * submit the body (N_genarray, N_modarray or N_fold node)
     * since the name of the variable that will be generated/modified
     * is needed for the creation of the ND_KS_USE_GENVAR_OFFSET ICM !
     */
    WITH_GEN (arg_node)
      = Trav (WITH_GEN (arg_node), ASSIGN_INSTR (INFO_IVE_CURRENTASSIGN (arg_info)));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxGenerator(node *arg_node, node *arg_info)
 *
 * description:
 *
 * remark:
 *   arg_info contains prior N_let node.
 *
 ******************************************************************************/

node *
IdxGenerator (node *arg_node, node *arg_info)
{
    node *vardec, *vinfo, *block, *icm_arg, *newassign, *newid, *arrayid;
    node *name_node, *dim_node, *dim_node2, *body, *colvinfo;
    types *artype;
    int i;

    DBUG_ENTER ("IdxGenerator");
    DBUG_ASSERT ((NODE_TYPE (arg_info) == N_let), "arg_info is not a N_let-node");
    DBUG_ASSERT ((NODE_TYPE (LET_EXPR (arg_info)) == N_with),
                 "N_let node in arg_info contains no with-loop");

    body = WITH_OPERATOR (LET_EXPR (arg_info));
    GEN_LEFT (arg_node) = Trav (GEN_LEFT (arg_node), NULL);
    GEN_RIGHT (arg_node) = Trav (GEN_RIGHT (arg_node), NULL);
    vardec = GEN_VARDEC (arg_node);
    vinfo = VARDEC_ACTCHN (vardec);

    /* first, we memorize the actuall chain */
    GEN_USE (arg_node) = vinfo;
    /* then, we remove the actual chain! */
    VARDEC_ACTCHN (vardec) = MakeVinfoDollar (CutVinfoChn (vinfo));

    /* for each IDX-vinfo-node we have to instanciate the respective
     * variable as first statement in the body of the with loop.
     * for doing so, we need the root of the body which is supplied
     * via arg_info by IdxWith!
     */
    DBUG_PRINT ("IDX", ("introducing idx-vars in with-bodies"));
    while (vinfo != NULL) {
        DBUG_PRINT ("IDX", ("examining vinfo(%p)", vinfo));
        if (VINFO_FLAG (vinfo) == IDX) {
            artype = LET_TYPE (arg_info);
            DBUG_ASSERT ((artype != NULL), "missing type-info for LHS of let!");
            switch (body->nodetype) {
            case N_modarray:
                block = MODARRAY_BODY (body);
                break;
            case N_genarray:
                block = GENARRAY_BODY (body);
                break;
            case N_foldprf:
                block = FOLDPRF_BODY (body);
                break;
            case N_foldfun:
                block = FOLDFUN_BODY (body);
                break;
            default:
                DBUG_ASSERT ((0 != 0), "unknown generator type in IdxGenerator");
                /*
                 * the following assignment is used only for convincing the C compiler
                 * that block will be initialized in any case!
                 */
                block = NULL;
            }

            if (((NODE_TYPE (body) == N_modarray) || (NODE_TYPE (body) == N_genarray))
                && EqTypes (VINFO_TYPE (vinfo), artype)) {
                /*
                 * we can reuse the genvar as index directly!
                 * therefore we create an ICM of the form:
                 * ND_KS_USE_GENVAR_OFFSET( <idx-varname>, <result-array-varname>)
                 */
                newid
                  = MakeId (IdxChangeId (GEN_ID (arg_node), artype), NULL, ST_regular);
                colvinfo = FindIdx (VARDEC_COLCHN (vardec), artype);
                DBUG_ASSERT (((colvinfo != NULL) && (VINFO_VARDEC (colvinfo) != NULL)),
                             "missing vardec for IDX variable!");
                ID_VARDEC (newid) = VINFO_VARDEC (colvinfo);
                arrayid = MakeId (StringCopy (LET_NAME (arg_info)), NULL, ST_regular);
                /*
                 * The backref of the arrayid is set wrongly to the actual
                 * integer index-variable. This is done on purpose for
                 * fooling the refcount-inference system.
                 * The "correct" backref would be LET_VARDEC( arg_info) !
                 */
                ID_VARDEC (arrayid) = VINFO_VARDEC (colvinfo);

                CREATE_2_ARY_ICM (newassign, "ND_KS_USE_GENVAR_OFFSET", newid, arrayid);
            } else {
                /*
                 * we have to instanciate the idx-variable by an ICM of the form:
                 * ND_KS_VECT2OFFSET( <off-name>, <var-name>,
                 *                    <dim of var>, <dim of array>, shape_elems)
                 */
                newid = MakeId (IdxChangeId (GEN_ID (arg_node), VINFO_TYPE (vinfo)), NULL,
                                ST_regular);
                colvinfo = FindIdx (VARDEC_COLCHN (vardec), VINFO_TYPE (vinfo));
                DBUG_ASSERT (((colvinfo != NULL) && (VINFO_VARDEC (colvinfo) != NULL)),
                             "missing vardec for IDX variable");
                ID_VARDEC (newid) = VINFO_VARDEC (colvinfo);

                name_node = MakeId (StringCopy (GEN_ID (arg_node)), NULL, ST_regular);
                ID_VARDEC (name_node) = vardec;

                dim_node = MakeNum (VARDEC_SHAPE (vardec, 0));

                dim_node2 = MakeNum (VINFO_DIM (vinfo));

                CREATE_4_ARY_ICM (newassign, "ND_KS_VECT2OFFSET", newid, /* off-name */
                                  name_node,                             /* var-name */
                                  dim_node,                              /* dim of var */
                                  dim_node2); /* dim of array */

                /* Now, we append the shape elems to the ND_KS_VECT2OFFSET-ICM ! */
                for (i = 0; i < VINFO_DIM (vinfo); i++)
                    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (VINFO_SELEMS (vinfo)[i]));
            }
            ASSIGN_NEXT (newassign) = BLOCK_INSTR (block);
            BLOCK_INSTR (block) = newassign;
        }
        vinfo = VINFO_NEXT (vinfo);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxNwith( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
IdxNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxNwith");

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /*
     * Finally, we traverse the Npart nodes in order to eliminate
     * superfluous index-vectors!
     */
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxNpart( node *arg_node, node *arg_info)
 *
 * description:
 *    Here we make sure that all parts of the generator will be traversed
 *    correctly. Furthermore, we eliminate superfluous generator vars
 *    (provided REFCOUNT_PROBLEM_SOLVED holds).
 *
 ******************************************************************************/

node *
IdxNpart (node *arg_node, node *arg_info)
{
    node *vardec, *mem_transform;

    DBUG_ENTER ("IdxNpart");

#ifdef REFCOUNT_PROBLEM_SOLVED
    if (INFO_IVE_MODE (arg_info) == M_uses_and_transform) {
        if (VINFO_FLAG (FindVect (NCODE_USE (NPART_CODE (arg_node)))) == DOLLAR) {
            /*
             * The index vector variable is used as IDX(...) only!
             * => we can eliminate the vector completely!
             */
            FREE (NPART_VEC (arg_node));
        }
    }
#else
    if (INFO_IVE_MODE (arg_info) == M_uses_and_transform) {
        /*
         * This makes sure, that the declaration of the index vector variable
         * will survive the second traversal of the vardecs, even if there is
         * no further reference to it but the one in the generator!
         * IF REFCOUNT_PROBLEM_SOLVED this can be spared since the one in the
         * generator would be deleted anyways ( see above)!!
         */
        vardec = IDS_VARDEC (NPART_VEC (arg_node));
        SET_VARDEC_OR_ARG_COLCHN (vardec, SetVect (VARDEC_COLCHN (vardec)));
    }
#endif

    /*
     * Now, we want to traverse the bounds and filters in order
     * to obtain all potential VECT uses.
     * For preventing any transformations of these, we have to make
     * sure, that (INFO_IVE_TRANSFORM_VINFO( arg_info) == NULL) during
     * that traversal!
     */
    mem_transform = INFO_IVE_TRANSFORM_VINFO (arg_info);
    INFO_IVE_TRANSFORM_VINFO (arg_info) = NULL;
    if (NPART_GEN (arg_node) != NULL)
        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
    INFO_IVE_TRANSFORM_VINFO (arg_info) = mem_transform;

    /*
     * Finally, we take care of any subsequent generators!
     */
    if (NPART_NEXT (arg_node) != NULL)
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IdxNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   arg_info points to the previous N_let node!
 *
 ******************************************************************************/

node *
IdxNcode (node *arg_node, node *arg_info)
{
    node *with, *idx_vardec, *vinfo, *withop_arr, *col_vinfo, *new_assign, *let_node,
      *current_assign, *new_id, *array_id;
    types *arr_type;

    DBUG_ENTER ("IdxNcode");

    /*
     * we traverse the current code block
     */
    current_assign = INFO_IVE_CURRENTASSIGN (arg_info);
    let_node = ASSIGN_INSTR (current_assign);

    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    DBUG_ASSERT (((NODE_TYPE (let_node) == N_let)
                  && (NODE_TYPE (LET_EXPR (let_node)) == N_Nwith)),
                 "arg_info contains no let with a with-loop on the RHS!");

    /*
     * we insert instances of the index-vector-var as first statement of the
     *  current code block.
     */

    with = LET_EXPR (let_node);
    idx_vardec = IDS_VARDEC (NWITH_VEC (with));
    vinfo = VARDEC_ACTCHN (idx_vardec);
    NCODE_USE (arg_node) = vinfo;
    VARDEC_ACTCHN (idx_vardec) = MakeVinfoDollar (CutVinfoChn (vinfo));

    if (INFO_IVE_MODE (arg_info) == M_uses_and_transform) {
        while (VINFO_FLAG (vinfo) != DOLLAR) {

            if (VINFO_FLAG (vinfo) == IDX) {
                arr_type = LET_TYPE (let_node);
                DBUG_ASSERT ((arr_type != NULL), "missing type-info for LHS of let!");

                switch (NWITH_TYPE (with)) {

                case WO_modarray:
                    withop_arr = NWITHOP_ARRAY (NWITH_WITHOP (with));
                    break;

                case WO_genarray:
                    withop_arr = NWITHOP_SHAPE (NWITH_WITHOP (with));
                    DBUG_ASSERT ((NODE_TYPE (withop_arr) == N_array),
                                 "shape of genarray is not N_array");
                    break;

                case WO_foldprf:
                    /* here is no break missing! */
                case WO_foldfun:
                    break;

                default:
                    DBUG_ASSERT ((0), "wrong with-loop type");
                }

                if (((NWITH_TYPE (with) == WO_modarray)
                     || (NWITH_TYPE (with) == WO_genarray))
                    && EqTypes (VINFO_TYPE (vinfo), arr_type)) {

                    /*
                     * we can reuse the genvar as index directly!
                     * therefore we create an ICM of the form:
                     * ND_KS_USE_GENVAR_OFFSET( <idx-varname>, <result-array-varname>)
                     */

                    new_id = MakeId (IdxChangeId (IDS_NAME (NWITH_VEC (with)), arr_type),
                                     NULL, ST_regular);
                    col_vinfo = FindIdx (VARDEC_COLCHN (idx_vardec), arr_type);
                    DBUG_ASSERT (((col_vinfo != NULL)
                                  && (VINFO_VARDEC (col_vinfo) != NULL)),
                                 "missing vardec for IDX variable");
                    ID_VARDEC (new_id) = VINFO_VARDEC (col_vinfo);
                    array_id
                      = MakeId (StringCopy (LET_NAME (let_node)), NULL, ST_regular);

                    /*
                     * The backref of the arrayid is set wrongly to the actual
                     * integer index-variable. This is done on purpose for
                     * fooling the refcount-inference system.
                     * The "correct" backref would be LET_VARDEC( let_node) !
                     */
                    ID_VARDEC (array_id) = VINFO_VARDEC (col_vinfo);

                    new_assign = MakeAssign (MakeIcm2 ("ND_KS_USE_GENVAR_OFFSET", new_id,
                                                       array_id),
                                             NULL);
                } else {
                    /*
                     * we have to instanciate the idx-variable by an ICM of the form:
                     * ND_KS_VECT2OFFSET( <off-name>, <var-name>,
                     *                    <dim of var>, <dim of array>, shape_elems)
                     */
                    new_assign = CreateVect2OffsetIcm (idx_vardec, VINFO_TYPE (vinfo));
                }

                ASSIGN_NEXT (new_assign) = BLOCK_INSTR (NCODE_CBLOCK (arg_node));
                DBUG_ASSERT ((NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (arg_node)))
                              != N_empty),
                             "N_empty node in block found");
                BLOCK_INSTR (NCODE_CBLOCK (arg_node)) = new_assign;
            }

            vinfo = VINFO_NEXT (vinfo);
        }
    }

    /*
     * now we traverse the next code block
     */

    if (NCODE_NEXT (arg_node) != NULL) {
        /* restore valid CURRENTASSIGN!! */
        INFO_IVE_CURRENTASSIGN (arg_info) = current_assign;
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * IdxCond( node * arg_node, node * arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
IdxCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IdxCond");

    /* Now, we duplicate the topmost chain of actchn! */
    MAP_TO_ALL_VARDEC_ACTCHNS (DuplicateTop, INFO_IVE_VARDECS (arg_info));

    if (COND_THEN (arg_node) != NULL)
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    /* Now, we switch the two topmost chains of actchn! */
    MAP_TO_ALL_VARDEC_ACTCHNS (SwitchTop, INFO_IVE_VARDECS (arg_info));

    if (COND_ELSE (arg_node) != NULL)
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    /* Now, we merge the topmost chain of actchn into the rest of actchn! */
    MAP_TO_ALL_VARDEC_ACTCHNS (MergeTop, INFO_IVE_VARDECS (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * IdxWhile( node * arg_node, node * arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
IdxWhile (node *arg_node, node *arg_info)
{
    int old_uses_mode;

    DBUG_ENTER ("IdxWhile");

    /* Now, we duplicate the topmost chain of actchn! */
    MAP_TO_ALL_VARDEC_ACTCHNS (DuplicateTop, INFO_IVE_VARDECS (arg_info));

    /*
     * We have to memorize the old uses_mode in order to
     * prevent code-transformations during the second traversal of the loop
     * in case the entire loop resides within an outer loop whose body is
     * traversed the first time!
     */
    old_uses_mode = INFO_IVE_MODE (arg_info);
    INFO_IVE_MODE (arg_info) = M_uses_only;

    if (WHILE_BODY (arg_node) != NULL)
        WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    /* Now, we merge the topmost chain of actchn into the rest of actchn! */
    MAP_TO_ALL_VARDEC_ACTCHNS (MergeTop, INFO_IVE_VARDECS (arg_info));
    MAP_TO_ALL_VARDEC_ACTCHNS (DuplicateTop, INFO_IVE_VARDECS (arg_info));

    /*
     * Now, we restore the uses_mode to the value set before entering the loop
     * (see comment above!)
     */
    INFO_IVE_MODE (arg_info) = old_uses_mode;
    if (WHILE_BODY (arg_node) != NULL)
        WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    MAP_TO_ALL_VARDEC_ACTCHNS (FreeTop, INFO_IVE_VARDECS (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * IdxDo( node * arg_node, node * arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
IdxDo (node *arg_node, node *arg_info)
{
    int old_uses_mode;

    DBUG_ENTER ("IdxDo");

    /* Now, we duplicate the topmost chain of actchn! */
    MAP_TO_ALL_VARDEC_ACTCHNS (DuplicateTop, INFO_IVE_VARDECS (arg_info));

    /*
     * We have to memorize the old uses_mode in order to
     * prevent code-transformations during the second traversal of the loop
     * in case the entire loop resides within an outer loop whose body is
     * traversed the first time!
     */
    old_uses_mode = INFO_IVE_MODE (arg_info);
    INFO_IVE_MODE (arg_info) = M_uses_only;

    if (DO_BODY (arg_node) != NULL)
        DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    /* Now, we merge the topmost chain of actchn into the rest of actchn! */
    MAP_TO_ALL_VARDEC_ACTCHNS (MergeCopyTop, INFO_IVE_VARDECS (arg_info));

    /*
     * Now, we restore the uses_mode to the value set before entering the loop
     * (see comment above!)
     */
    INFO_IVE_MODE (arg_info) = old_uses_mode;
    if (DO_BODY (arg_node) != NULL)
        DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    MAP_TO_ALL_VARDEC_ACTCHNS (FreeTop, INFO_IVE_VARDECS (arg_info));

    DBUG_RETURN (arg_node);
}
