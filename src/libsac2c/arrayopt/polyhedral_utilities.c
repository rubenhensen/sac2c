/** <!--********************************************************************-->
 *
 * @defgroup phut polyhedral analysis utility functions
 *
 *  Overview: These functions are intended to provide useful
 *            manipulation services for polyhedron-based analysis.
 *            At present, we support only ISL.
 *
 * ISL input data Terminology, taken from
 *  http://barvinok.gforge.inria.fr/tutorial.pdf,
 *      Sven Verdoolaege's ISCC tutorial.
 *
 * A triangular "statement instance set" such as S:
 *
 *     for (i = 1; i <= n; ++i)
 *       for (j = 1; j <= i; ++j)
 *        **  S  **
 *
 * is represented as this input to ISL:
 *
 *          [n] -> { S[i,j] : 1 <= i <= n and 1 <= j <= i }
 *                            ---- Presburger formula
 *                     --- set variables
 *                   - optional name of space
 *           - symbolic constants
 *
 *    A Presburger formula is a union of one or more constraints.
 *
 * NB. This code assumes, in at least one place, that AVIS_NPART is
 *     maintained for all WITHID elements by the calling traversal.
 *
 * NB. General assumptions made in these functions about the calling
 *     envionment:
 *
 *      1. The caller must maintain AVIS_NPART for WITHID_xxx nodes.
 *         It would be nice if this were maintained throughout compilation,
 *         but things can change underfoot so that we are unsure where we should
 *         invalidate it. Hence, we assume the calling traversal
 *         will pass through N_part codes on its way down, and set AVIS_NPART.
 *         Similarly, on its way up, it will unset AVIS_NPART.
 *
 *         The main problem with maintaining AVIS_NPART is that WITHID nodes
 *         are NOT SSA, so multiple N_part nodes share the
 *         same WITHID N_avis nodes.
 *
 *      2. Similarly, the caller must maintain FUNDEF_CALLAP and
 *         FUNDEF_CALLERFUNDEF
 *
 *      3. All callers must SKIP chained assigns. Failure to do this will result
 *         in complaints from ISL about "unknown identifier".
 *
 * NB.     Interprocedural affine function construction
 *         (Look at ~/sac/testsuite/optimizations/pogo/condfun.sac to see
 *          why we need to look at the LACFUN caller to build
 *          an affine function tree for XXX's shape vector.
 *
 *
 * NB. A few notes on LOOPFUNs:
 *
 *     All loops in SAC are translated into a common form, the LOOPFUN.
 *     LOOPFUNs are implemented as tail recursions.
 *
 *     A tail recursion executes the loop's block of code, then
 *     decides whether recursion is needed or not. Hence, special
 *     code is required to check for a zero-trip loop. This is done
 *     by a CONDFUN preceeding the call to the LOOPFUN.
 *
 *     We perform Loop count determination only when the
 *     relational (condprf) controlling the recursion is quite simple:
 *
 *       K' = K + N  or
 *       K' = K - N  or
 *       K' = N + K
 *
 *     where K is the loop-carried variable, and N is a
 *     loop-independent variable.
 *
 *     The code generated for ISL uses K, rather than K', even
 *     though the relational (condprf) uses K'.
 *     This compensates for the "extra" trip through the code.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file polyhedral_utilities.c
 *
 * Prefix: PHUT
 *
 *****************************************************************************/

#include "globals.h"

#define DBUG_PREFIX "PHUT"
#include <stdlib.h>
#include <sys/wait.h>
#include "debug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"
#include "free.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "new_types.h"
#include "constants.h"
#include "new_typecheck.h"
#include "narray_utilities.h"
#include "DupTree.h"
#include "print.h"
#include "system.h"
#include "filemgr.h"
#include "sys/param.h"
#include "polyhedral_utilities.h"
#include "polyhedral_defs.h"
#include "symbolic_constant_simplification.h"
#include "with_loop_utilities.h"
#include "lacfun_utilities.h"
#include "LookUpTable.h"
#include "polyhedral_guard_optimization.h"
#include "isl_utilities.h"

// There is a not-an-N_prf value for N_prf somewhere, but I can't find it.
#define NOPRFOP 0

// We generate simple ISL expressions of the form:
//    var =  larg fn rarg;
//    and
//    var = rarg;
#define ISLVAR(z) (EXPRS_EXPR (z))
#define ISLEQ(z) (EXPRS_EXPR (EXPRS_NEXT (z)))
#define ISLLARG(z) (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (z))))
#define ISLFN(z) (PRF_PRF (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (z))))))
#define ISLRARG(z) (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (z))))))

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

static int islvarnum;

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUTskipChainedAssigns( node *arg_node)
 *
 * @brief Skip intermediate N_id node assigns for arg_node N_id.
 *        E.g., if we have:
 *            x = foo();
 *            y = x;
 *            z = y;
 *        then PHUTskipChainedAssigns( z) will give us x.
 *
 * @param  arg_node: The N_id we want to chase
 * @return The last N_id in the chain
 *
 * NB. I thought there was some way to make PM do this, but I can't
 *     make it work now. Hence, this crude function.
 *
 ******************************************************************************/
node *
PHUTskipChainedAssigns (node *arg_node)
{
    node *z;
    node *avis;
    node *rhs = NULL;
    node *assgn;

    DBUG_ENTER ();

    z = arg_node;
    if (N_id == NODE_TYPE (arg_node)) {
        avis = ID_AVIS (arg_node);
        assgn = AVIS_SSAASSIGN (avis);
        if ((NULL != assgn) && (N_let == NODE_TYPE (ASSIGN_STMT (assgn)))) {
            rhs = LET_EXPR (ASSIGN_STMT (assgn));
            if (N_id == NODE_TYPE (rhs)) {
                z = PHUTskipChainedAssigns (rhs);
            }
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn void PHUTprintIslAffineFunctionTree( node *arg_node)
 *
 * @brief  Utility for printing an ISL affine function tree in slightly
 *         more human-readable form.
 *
 * @param An N_exprs chain for ISL
 *
 * @return none
 *
 ******************************************************************************/
void
PHUTprintIslAffineFunctionTree (node *arg_node)
{
    int n;
    int j;

    DBUG_ENTER ();
    n = TCcountExprs (arg_node);
    for (j = 0; j < n; j++) {
        PRTdoPrint (TCtakeDropExprs (1, j, arg_node));
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *Node2Avis( node *arg_node)
 *
 * @brief Find N_avis node for arg_node
 *
 * @param An N_avis, N_id, or N_ids node
 *
 * @return the associated N_avis node, or NULL if this is an N_num
 *         or if arg_node is NULL
 *
 ******************************************************************************/
static node *
Node2Avis (node *arg_node)
{
    node *avis = NULL;

    DBUG_ENTER ();

    if (NULL != arg_node) {
        switch (NODE_TYPE (arg_node)) {
        case N_id:
            avis = ID_AVIS (arg_node);
            break;

        case N_ids:
            avis = IDS_AVIS (arg_node);
            break;

        case N_avis:
            avis = arg_node;
            break;

        case N_num:
        case N_bool:
            break;

        default:
            DBUG_ASSERT (NULL != avis, "Expected N_id, N_avis, or N_ids node");
        }
    }

    DBUG_RETURN (avis);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *Node2Value( node *arg_node)
 *
 * @brief Find N_avis node for arg_node, unless it is
 *        an N_num/N_bool, in which case we XXX
 *
 * @param An N_avis, N_id, N_num, N_bool, or N_ids node
 *
 * @return An N_id if the argument is an N_id, N_avis, or N_ids.
 *         Otherwise, the associated N_avis node,
 *
 ******************************************************************************/
static node *
Node2Value (node *arg_node)
{
    node *z = NULL;
    constant *con = NULL;

    DBUG_ENTER ();

    if (NULL != arg_node) {
        z = Node2Avis (arg_node);
        if (NULL != z) {
            if (TYisAKV (AVIS_TYPE (z))) {
                if (TUisIntScalar (AVIS_TYPE (z))) {
                    // DEADCODE pogo/condfun.sac AVIS_ISLCLASS( z) =
                    // AVIS_ISLCLASSEXISTENTIAL;
                    z = TBmakeNum (TUtype2Int (AVIS_TYPE (z)));
                } else {
                    if (TUisBoolScalar (AVIS_TYPE (z))) {
                        con = TYgetValue (AVIS_TYPE (z)); // con is NOT a copy!
                        z = TBmakeNum (COconst2Int (con));
                    } else {
                        DBUG_ASSERT (FALSE, "Expected N_num or N_bool");
                    }
                }
            } else {
                DBUG_ASSERT (N_avis == NODE_TYPE (z), "Expected N_avis from Node2Avis");
                z = TBmakeId (z);
            }
        } else {
            switch (NODE_TYPE (arg_node)) {
            case N_num:
                z = DUPdoDupNode (arg_node);
                break;

            case N_bool:
                z = TBmakeNum (BOOL_VAL (arg_node));
                break;

            default:
                DBUG_ASSERT (FALSE,
                             "Expected N_id, N_avis, N_ids, N_num, or N_bool node");
            }
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool PHUTsetIslClass( node *arg_node, int islclass)
 *
 * @brief Set AVIS_ISLCLASS for arg_node, unless it is already set.
 *
 * @param arg_node: An N_id or N_avis. Or, just to make life more fun,
 *                  it may be an N_num or N_bool, which we ignore.
 *        islclass: the AVIS_ISLCLASS to be set for this variable.
 *
 * @return void
 *
 ******************************************************************************/
void
PHUTsetIslClass (node *arg_node, int islclass)
{
    node *avis;

    DBUG_ENTER ();

    avis = Node2Avis (arg_node);
    if ((NULL != avis) && (AVIS_ISLCLASSUNDEFINED == AVIS_ISLCLASS (avis))) {
        AVIS_ISLCLASS (avis) = islclass;
        DBUG_PRINT ("Set %s ISLCLASS to %d", AVIS_NAME (avis), AVIS_ISLCLASS (avis));
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool PHUTinsertVarIntoLut( node *arg_node, lut_t *varlut, node *fundef)
 *
 * @brief Insert N_id or N_avis into varlut, if not already there.
 *        We use varlut LUT solely as a way to build the set of unique
 *        set variable names.
 *
 * @param arg_node: An N_id or N_avis. Or, just to make life more fun,
 *                  it may be an N_num or N_bool, which we ignore.
 *        varlut: The address of the LUT we use for collecting names
 *        fundef: The fundef containing arg_node.
 *                We store fundef and arg_node in the LUT, then
 *                catenate them when building the
 *                ISL input variables: [fundef,avis].
 *                We have to do this in order
 *                to disambiguate duplicate names across LACFUN calls.
 *        islclass: the AVIS_ISLCLASS to be associated with this variable.
 *
 * @return TRUE if we the variables was inserted; FALSE if it was already there
 *         or not a variable.
 *
 ******************************************************************************/
bool
PHUTinsertVarIntoLut (node *arg_node, lut_t *varlut, node *fundef, int islclass)
{
    node *avis;
    node *founditem = NULL;
    bool z = FALSE;

    DBUG_ENTER ();

    avis = Node2Avis (arg_node);
    if (NULL != avis) {
        DBUG_ASSERT (NULL != varlut, "NULL VARLUT");
        LUTupdateLutP (varlut, avis, fundef, (void **)&founditem);
        z = (NULL == founditem);
        if (z) {
            PHUTsetIslClass (avis, islclass);
            DBUG_PRINT ("Inserted %s:%s into VARLUT with ISLCLASS %d",
                        FUNDEF_NAME (fundef), AVIS_NAME (avis), AVIS_ISLCLASS (avis));
        } else {
            DBUG_PRINT ("%s:%s already in VARLUT with ISLCLASS %d", FUNDEF_NAME (fundef),
                        AVIS_NAME (avis), AVIS_ISLCLASS (avis));
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn char *Prf2Isl( prf arg_node)
 *
 * @brief Convert an N_prf PRF_PRF into
 *
 * @param arg_node an N_prf.
 *
 * @return An pointer to a character string representing some affine
 *         function on scalar arguments.
 *
 ******************************************************************************/
static char *
Prf2Isl (prf arg_node)
{
    char *z = NULL;

    DBUG_ENTER ();

    switch (arg_node) {

    default:
        DBUG_ASSERT (FALSE, "Did not find affine function in table");
        break;

    case F_lt_SxS:
    case F_val_lt_val_SxS:
        z = "<";
        break;

    case F_le_SxS:
    case F_val_le_val_SxS:
        z = "<=";
        break;

    case F_eq_SxS:
        z = "=";
        break;
    case F_ge_SxS:
        z = ">=";
        break;
    case F_gt_SxS:
        z = ">";
        break;
    case F_add_SxS:
        z = "+";
        break;
    case F_sub_SxS:
        z = "-";
        break;
    case F_mul_SxS:
        z = "*";
        break;
    case F_min_SxS:
        z = "min";
        break;
    case F_max_SxS:
        z = "max";
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn void PHUTclearAvisIslAttributes( lut_t *varlut)
 *
 * @brief Clear PHUT/ISL_related attributes in N_avis nodes.
 *        This performs most cleanup after a single optimization
 *        attempt.
 *
 * @param varlut: The varlut that was populated with the N_avis
 *        nodes relevant to the AFT we are now done with.
 *
 * @return void
 *
 ******************************************************************************/
static void *
ClearAvisIslAttributesOne (void *rest, void *fundef, void *avis)
{ // helper function
    node *z = NULL;
    node *avis2;

    DBUG_ENTER ();

    avis2 = (node *)avis;

    if (NULL != avis2) {
        DBUG_PRINT ("Clearing AVIS_ISLCLASS, AVIS_ISLTREE in function %s, variable %s",
                    FUNDEF_NAME ((node *)fundef), AVIS_NAME (avis2));
        AVIS_ISLCLASS (avis2) = AVIS_ISLCLASSUNDEFINED;
        AVIS_ISLTREE (avis2)
          = (NULL != AVIS_ISLTREE (avis2)) ? FREEdoFreeTree (AVIS_ISLTREE (avis2)) : NULL;
    }

    DBUG_RETURN (z);
}

void
PHUTclearAvisIslAttributes (lut_t *varlut)
{
    node *z;
    node *head = NULL;

    DBUG_ENTER ();

    z = (node *)LUTfoldLutP (varlut, head, ClearAvisIslAttributesOne);

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn void GetIslSetVariablesFromLut(
 *
 * @brief Build the set variables used by the constraints.
 *
 *        The result is an N_exprs chain of the N_avis nodes comprising the
 *        set variables, prefixed by the fundef name. E.g.,
 *        if i,j,k are called by foo, foo, and goo, we end up with:
 *
 *          [ [foo,i], [goo,j], goo,k]]
 *
 * @param varlut: a lut of nodes that comprise the ISL set variables.
 *
 * @return An N_exprs chain of the above nodes
 *
 ******************************************************************************/
static void *
GetIslSetVariablesFromLutOne (void *rest, void *fundef, void *avis)
{
    node *z = NULL;

    DBUG_ENTER ();

    if (NULL != avis) {
        DBUG_PRINT ("Found function %s, variable %s in VARLUT",
                    FUNDEF_NAME ((node *)fundef), AVIS_NAME (avis));
        z = TBmakeExprs (TBmakeStr (FUNDEF_NAME ((node *)fundef)),
                         TBmakeExprs (TBmakeId (avis), NULL));
        z = TCappendExprs (z, rest);
    }

    DBUG_RETURN (z);
}

static node *
GetIslSetVariablesFromLut (lut_t *varlut)
{
    node *z;
    node *head = NULL;

    DBUG_ENTER ();

    z = (node *)LUTfoldLutP (varlut, head, GetIslSetVariablesFromLutOne);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn void printIslName( FILE *handle, node *avis)
 *
 * @brief print (to file) an ISL-acceptable, unique version of avisname.
 *        For fundefname of "foo", and avisname of nid", we generate:
 *          _3_foo_nid, where 3 is the length of "foo".
 *
 * @param handle: a file handle for the output
 *        avis: the SAC N_avis for the variable
 *
 * @return void
 *         Side effect: print the generated ISL name.
 *         Side effect: Set AVIS_ISLINDEX, if not already set.
 *
 *         NB. AVIS_ISLINDEX is used only to provide a unique ISL name for
 *             each variable in an Affine Function Tree(AFT). Since each call
 *             to ISL (via sacislinterface) is independent of the others,
 *             we could use the AVIS's address for this purpose. The idea
 *             here is to come up with something that is slightly more
 *             human-readable (for debugging only). It could also
 *             serve to identify those nodes that already possess an AFT, but
 *             we do not do that yet.
 *
 ******************************************************************************/
static void
printIslName (FILE *handle, node *avis)
{

    DBUG_ENTER ();

    if (0 == AVIS_ISLINDEX (avis)) {
        islvarnum++;
        AVIS_ISLINDEX (avis) = islvarnum;
        DBUG_PRINT ("Generated V%d for %s", AVIS_ISLINDEX (avis), AVIS_NAME (avis));
    }

    fprintf (handle, "V%d", AVIS_ISLINDEX (avis));

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *BuildIslSimpleConstraint()
 *
 * @brief Build one constraint for ISL as a one-element N_exprs chain, containing
 *        an N_exprs chain of the constraint.
 *
 *        For ids nprf1  arg1 nprf2 arg2, we get, e.g.,
 *              0 <=     arg1 <     arg2
 *
 * @param ids, arg1, arg2: N_id, N_avis or N_num nodes
 *        nprf1, nprf2: the function(s) to be used, or NULL
 *
 * @return The result is a one-element N_exprs comprising an N_exprs chain
 *         of the constraint, which will eventually
 *         be assembled, then turned into text and written to file for passing to ISL.
 *         The two-deep nesting is so that we can introduce "and" conjunctions
 *         among the contraints when constructing the ISL input.
 *
 *         If ids, arg1, or arg2 is AKV, we replace it by its value.
 *
 ******************************************************************************/
static node *
BuildIslSimpleConstraint (node *ids, prf nprf1, node *arg1, prf nprf2, node *arg2)
{
    node *z;
    node *idsv;
    node *arg1v;
    node *arg2v;

    DBUG_ENTER ();

    DBUG_PRINT ("Generating simple constraint");
    idsv = Node2Avis (ids);
    idsv = (NULL == idsv) ? ids : idsv; // N_num support
    if ((NULL != idsv) && (N_avis == NODE_TYPE (idsv))) {
        idsv = TBmakeId (idsv);
    }
    DBUG_ASSERT (NULL != idsv, "Expected non-NULL ids");
    arg1v = Node2Value (arg1);
    arg2v = Node2Value (arg2);

    z = TBmakeExprs (idsv, NULL);
    z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf1, NULL), NULL));
    z = TCappendExprs (z, TBmakeExprs (arg1v, NULL));

    if (NOPRFOP != nprf2) {
        z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf2, NULL), NULL));
        if (NULL != arg2v) { // kludge for disjunction F_or_SxS
            z = TCappendExprs (z, TBmakeExprs (arg2v, NULL));
        }
    }

    z = TBmakeExprs (z, NULL);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *BuildIslStrideConstraint()
 *
 * @brief Build a constraint for ISL with a non-unit stride.
 *
 * @param ids, arg1, arg2: N_id, N_avis or N_num nodes
 *        nprf1, nprf2: the function(s) to be used, or NULL
 *
 * @return The result is a one-element N_exprs comprising an N_exprs chain
 *         of the constraint.
 *         The two-deep nesting allows us to introduce "and" conjunctions
 *         among the contraints when constructing the ISL input.
 *
 *         If ids, arg1, or arg2 is AKV, we replace it by its value.
 *
 ******************************************************************************/
static node *
BuildIslStrideConstraint (node *ids, prf nprf1, node *arg1, prf nprf2, node *arg2,
                          prf nprf3, node *arg3)
{ // Vaguely like BuildIslSimpleConstraint, but with one more prf and arg.
    node *z;
    node *idsv;
    node *arg1v;
    node *arg2v;
    node *arg3v;

    DBUG_ENTER ();

    idsv = Node2Avis (ids);
    idsv = (NULL == idsv) ? ids : idsv; // N_num support
    if ((NULL != idsv) && (N_avis == NODE_TYPE (idsv))) {
        DBUG_PRINT ("Generated ISL stride constraint for %s", AVIS_NAME (idsv));
        idsv = TBmakeId (idsv);
    }
    DBUG_ASSERT (NULL != idsv, "Expected non-NULL ids");
    arg1v = Node2Value (arg1);
    arg2v = Node2Value (arg2);
    arg3v = Node2Value (arg3);

    z = TBmakeExprs (idsv, NULL);
    z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf1, NULL), NULL));
    z = TCappendExprs (z, TBmakeExprs (arg1v, NULL));

    if (NOPRFOP != nprf2) {
        z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf2, NULL), NULL));
        if (NULL != arg2v) { // kludge for disjunction F_or_SxS
            z = TCappendExprs (z, TBmakeExprs (arg2v, NULL));
        }
    }

    if (NOPRFOP != nprf3) {
        z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf3, NULL), NULL));
    }
    z = TCappendExprs (z, TBmakeExprs (arg3v, NULL));

    z = TBmakeExprs (z, NULL);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *findBoundEl( node *arg_node, node *bnd, int k...)
 *
 * @brief arg_node may represent a WITHID element for a WL.
 *        If so, generate an ISL constraint of:
 *
 *             k = WITHID_IDS iota arg_node
 *             arg_node >= LB[k]
 *
 * @param arg_node: an N_avis for the WITHID_IDS
 *        bnd: GENERATOR_BOUND1 or GENERATOR_BOUND2
 *        k: index into WITHID_IDS, which may be -1.
 *
 * @return If arg_node is a member of a WITHID, LB[k] or UB[k]
 *         Otherwise, NULL.
 *
 ******************************************************************************/
static node *
findBoundEl (node *arg_node, node *bnd, int k)
{
    node *z = NULL;

    DBUG_ENTER ();

    if ((NULL != AVIS_NPART (arg_node)) && (NULL != bnd)) {
        if (-1 != k) {
            z = TCgetNthExprsExpr (k, ARRAY_AELEMS (bnd));
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUTcollectWlGenerator( node *arg_node...)
 *
 * @brief arg_node is a WITHID element for a WL, shown here as iv.
 *        The slightly messy part here is non-unit stride and/or width, which entail
 *        adding two new set variables, iv', iv'', and ivw.
 *
 *        Assume we have this WL generator:
 *
 *          lb <= iv < ub stride stp width wid
 *
 *        We generate ISL constraints as follows:
 *
 *          N >= 0
 *          iv' = lb + stp * N   for some N
 *          0 <= ivw < wid
 *          iv = iv' + ivw
 *          lb <= iv < ub
 *
 *        We also have to ensure that these constraints are generated
 *        whenever there is ANY reference to a WITHID_IDS.
 *
 *        A key simplification here is that SAC requires that wid <= stp,
 *        and that wid and stp be constants.
 *
 *        NB. lb, ub, stp, and wid are N_num or N_id nodes.
 *
 *        See also S.B. Scholz: SAC - Efficient Support... p14
 *
 * @param An N_avis
 * @param res: an N_exprs chain for the growing result, initially NULL.
 *
 * @return If arg_node is a member of a WITHID, an N_exprs chain for
 *         the above relational, catenated to the incoming res.
 *
 *         Otherwise, NULL.
 *
 ******************************************************************************/
static node *
Node2Num (node *arg_node)
{ // Try to convert arg_node to an N_num; otherwise, just return arg_node
    node *z;
    pattern *pat;
    constant *con = NULL;
    pat = PMconst (1, PMAgetVal (&con), 0);
    if (PMmatchFlat (pat, arg_node)) {
        z = TBmakeNum (COconst2Int (con));
        con = COfreeConstant (con);
    } else {
        z = arg_node;
    }
    pat = PMfree (pat);

    return (z);
}

static node *
StepOrWidthHelper (node *stpwid)
{ // Find GENERATOR_STEP or GENERATOR_WIDTH element as an N_num.
    // If stpwid is NULL, generate a value of 1 for it.
    node *z;

    z = (NULL != stpwid) ? Node2Num (stpwid) : TBmakeNum (1);

    return (z);
}

node *
PHUTcollectWlGenerator (node *arg_node, node *fundef, lut_t *varlut, node *res,
                        int loopcount)
{
    node *z = NULL;
    node *res2 = NULL;
    node *partn;
    node *lbel;
    node *ubel;
    node *ivavis;
    int k;
    node *lb;
    node *ub;
    node *stp;
    node *wid;
    node *stpel;
    node *widel;
    node *ivpavis;
    node *ivppavis;
    node *ivwavis;
    node *navis;

    DBUG_ENTER ();

    partn = AVIS_NPART (arg_node);
    if ((NULL != partn)) {
        k = LFUindexOfMemberIds (arg_node, WITHID_IDS (PART_WITHID (partn)));
        if (-1 != k) {
            DBUG_PRINT ("Generating generator constraints for %s", AVIS_NAME (arg_node));
            ivavis = TCgetNthIds (k, WITHID_IDS (PART_WITHID (partn)));
            lb = WLUTfindArrayForBound (GENERATOR_BOUND1 (PART_GENERATOR (partn)));
            ub = WLUTfindArrayForBound (GENERATOR_BOUND2 (PART_GENERATOR (partn)));
            stp = WLUTfindArrayForBound (GENERATOR_STEP (PART_GENERATOR (partn)));
            wid = WLUTfindArrayForBound (GENERATOR_WIDTH (PART_GENERATOR (partn)));
            lbel = findBoundEl (arg_node, lb, k);
            lbel = PHUTskipChainedAssigns (lbel);
            lbel = Node2Num (lbel);
            ubel = findBoundEl (arg_node, ub, k);
            ubel = PHUTskipChainedAssigns (ubel);

            stpel = findBoundEl (arg_node, stp, k);
            stpel = StepOrWidthHelper (stpel);
            stpel = PHUTskipChainedAssigns (stpel);
            widel = findBoundEl (arg_node, wid, k);
            widel = StepOrWidthHelper (widel);
            widel = PHUTskipChainedAssigns (widel);

            res2 = PHUTcollectAffineExprsLocal (lbel, fundef, varlut, NULL,
                                                AVIS_ISLCLASSUNDEFINED, loopcount);
            res = TCappendExprs (res, res2);
            res2 = PHUTcollectAffineExprsLocal (ubel, fundef, varlut, NULL,
                                                AVIS_ISLCLASSUNDEFINED, loopcount);
            res = TCappendExprs (res, res2);

            // Generate new set variables for ISL. Apart from ISL analysis, these
            // names are unused. I.e., we do not generate any code that uses them.
            ivpavis
              = TBmakeAvis (TRAVtmpVarName ("IV"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            if (PHUTinsertVarIntoLut (ivpavis, varlut, fundef,
                                      AVIS_ISLCLASSSETVARIABLE)) {
                z = DUPdoDupTree (AVIS_ISLTREE (ivpavis));
                res = TCappendExprs (res, z);
            }

            ivppavis
              = TBmakeAvis (TRAVtmpVarName ("IVP"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            if (PHUTinsertVarIntoLut (ivppavis, varlut, fundef,
                                      AVIS_ISLCLASSEXISTENTIAL)) {
                z = DUPdoDupTree (AVIS_ISLTREE (ivppavis));
                res = TCappendExprs (res, z);
            }

            // Generate: 0 <= ivw < wid
            ivwavis
              = TBmakeAvis (TRAVtmpVarName ("IVW"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            if (PHUTinsertVarIntoLut (ivwavis, varlut, fundef,
                                      AVIS_ISLCLASSSETVARIABLE)) {
                z = DUPdoDupTree (AVIS_ISLTREE (ivwavis));
                res = TCappendExprs (res, z);
                z = BuildIslSimpleConstraint (TBmakeNum (0), F_le_SxS, ivwavis, F_lt_SxS,
                                              widel);
                res = TCappendExprs (res, z);
            }

            // Generate iv' = lb + stp * N
            //          N >= 0
            navis = TBmakeAvis (TRAVtmpVarName ("N"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            if (PHUTinsertVarIntoLut (navis, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL)) {
                z = DUPdoDupTree (AVIS_ISLTREE (navis));
                res = TCappendExprs (res, z);
                // Generate N >= 0
                z = BuildIslSimpleConstraint (navis, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                z = BuildIslStrideConstraint (ivpavis, F_eq_SxS, DUPdoDupNode (lbel),
                                              F_add_SxS, stpel, F_mul_SxS, navis);
                res = TCappendExprs (res, z);
            }

            // Generate iv = iv' + ivw
            z = BuildIslSimpleConstraint (ivavis, F_eq_SxS, ivpavis, F_add_SxS, ivwavis);
            res = TCappendExprs (res, z);

            // Generate lb <= iv < ub
            z = BuildIslSimpleConstraint (DUPdoDupNode (lbel), F_le_SxS, ivavis, F_lt_SxS,
                                          DUPdoDupNode (ubel));
            res = TCappendExprs (res, z);
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn int writeSetVariables()
 *
 * @brief Write set variables in idlist as:
 *
 *          [x,y,z]
 *
 *        This handles ISL parameters and bog-standard variables.
 *
 * @param handle: output file handle
 * @param idlist: N_exprs chain of set variables
 *
 * @return int
 *
 ******************************************************************************/
static int
CountVariablesInIslclass (node *idlist, int islclass)
{ // +/idlist member islclass

    node *avis;
    char *fn;
    int n;
    int i;
    int z = 0;

    DBUG_ENTER ();

    n = TCcountExprs (idlist);
    for (i = 0; i < n; i = i + 2) {
        fn = STR_STRING (TCgetNthExprsExpr (i, idlist));
        avis = ID_AVIS (TCgetNthExprsExpr (i + 1, idlist));
        DBUG_PRINT ("AVIS_ISLCLASS(%s) in fundef %s is %d", AVIS_NAME (avis), fn,
                    AVIS_ISLCLASS (avis));
        z = (islclass == AVIS_ISLCLASS (avis)) ? z + 1 : z;
        DBUG_ASSERT ((AVIS_ISLCLASSEXISTENTIAL == AVIS_ISLCLASS (avis))
                       || (AVIS_ISLCLASSPARAMETER == AVIS_ISLCLASS (avis))
                       || (AVIS_ISLCLASSSETVARIABLE == AVIS_ISLCLASS (avis)),
                     "Failed to set AVIS_ISLCLASS");
    }

    DBUG_RETURN (z);
}

static void
WriteSetVariables (FILE *handle, node *idlist)
{
    node *avis;
    int i;
    int n;
    int inclass;
    int numleft;
    char *funname;

    DBUG_ENTER ();
    // Append ISL versions of set variables:  [i,j,k...]
    // These come in pairs: [fundefname,i], [fundefname,j]...

    fprintf (handle, " [\n    ");
    n = TCcountExprs (idlist);
    inclass = CountVariablesInIslclass (idlist, AVIS_ISLCLASSSETVARIABLE);
    numleft = inclass;

    for (i = 0; i < n; i = i + 2) {
        funname = STR_STRING (TCgetNthExprsExpr (i, idlist));
        avis = ID_AVIS (TCgetNthExprsExpr (i + 1, idlist));
        if (AVIS_ISLCLASSSETVARIABLE == AVIS_ISLCLASS (avis)) {
            // Print SAC set variable info
            printIslName (handle, avis);
            fprintf (handle, " # %s:%s\n", funname, AVIS_NAME (avis));
            numleft--;
            if (numleft > 0) {
                fprintf (handle, "  , ");
            }
        }
    }

    fprintf (handle, " ]\n");

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @fn bool writeExistsSetVariables()
 *
 * @brief Write " : exists " set variables in idlist as:
 *
 *           : (
 *             exists  x :
 *             exists  y :
 *             ...
 *             exists  z :
 *           [and then later on, if we had any existential variables,
 *            a closing parenthesis.]
 *
 *        These are what ISL calls "existentially qualified" variables.
 * @param handle: output file handle
 * @param idlist: N_exprs chain of set variables
 *
 * @return TRUE if we have to write a closing parenthesis
 *
 *
 ******************************************************************************/
static bool
WriteExistsSetVariables (FILE *handle, node *idlist)
{
    node *avis;
    char *funname;
    int i;
    int n;
    bool z;

    DBUG_ENTER ();
    n = TCcountExprs (idlist);
    z = 0 != n;

    for (i = 0; i < n; i = i + 2) {
        avis = ID_AVIS (TCgetNthExprsExpr (i + 1, idlist));
        if (AVIS_ISLCLASSEXISTENTIAL == AVIS_ISLCLASS (avis)) {
            funname = STR_STRING (TCgetNthExprsExpr (i, idlist));
            fprintf (handle, " exists ");
            printIslName (handle, avis);
            fprintf (handle, " : # %s:%s\n", funname, AVIS_NAME (avis));
        }
    }

    DBUG_RETURN (0 != n);
}

/** <!-- ****************************************************************** -->
 * @fn void writeParameterVariables()
 *
 * @brief Write parameter variables in idlist as:
 *
 *    [x,y,z] ->
 *
 * @param handle: output file handle
 * @param idlist: N_exprs chain of set variables
 *
 * @return void
 *
 *
 ******************************************************************************/
static void
WriteParameterVariables (FILE *handle, node *idlist)
{
    node *avis;
    char *funname;
    int i;
    int n;
    int inclass;
    int numleft;

    DBUG_ENTER ();
    n = TCcountExprs (idlist);
    inclass = CountVariablesInIslclass (idlist, AVIS_ISLCLASSPARAMETER);
    numleft = inclass;
    if (0 != inclass) {
        fprintf (handle, "\n# Parameters\n [\n   ");
    }
    for (i = 0; i < n; i = i + 2) {
        avis = ID_AVIS (TCgetNthExprsExpr (i + 1, idlist));
        if (AVIS_ISLCLASSPARAMETER == AVIS_ISLCLASS (avis)) {
            printIslName (handle, avis);
            funname = STR_STRING (TCgetNthExprsExpr (i, idlist));
            fprintf (handle, " # %s:%s\n", funname, AVIS_NAME (avis));
            numleft--;
            if (numleft > 0) {
                fprintf (handle, " , ");
            }
        }
    }

    if (0 != inclass) {
        fprintf (handle, " ] -> \n");
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @fn void PHUTwriteUnionSet()
 *
 * @brief Append one ISL set of polyhedra to input file, from exprs and
 *        set variable list in varlut, as a union set or a union map,
 *        depending on isrelation.
 *        If exprs is NULL, do nothing.
 *
 *        For a union set, we generate, ignoring comments:
 *
 *          { [x,y,z] : exists APVVAR : x > y and z < 42 }
 *
 *        For a relation, AKA union_map, we generate:
 *
 *          { [x,y,z] -> [x,y,z] : x > y }
 *
 * @param handle: output file handle
 * @param exprs: N_exprs chain of N_exprs chains. Each pair of elements of exprs
 *               is a single constraint.
 * @param tag: character vector tag for debugging only
 * @param isunionset: If TRUE, write a union set;
 *                    If FALSE, write a union map.
 * @param lhsname: AVIS_NAME of the LHS expression we are trying to simplify
 *
 * @return void
 *
 ******************************************************************************/
void
printIslArg (FILE *handle, node *expr)
{

    DBUG_ENTER ();

    switch (NODE_TYPE (expr)) {
    case N_id:
        printIslName (handle, ID_AVIS (expr));
        break;

    case N_num:
        fprintf (handle, "%d", NUM_VAL (expr));
        break;

    case N_bool:
        fprintf (handle, "%s", BOOL_VAL (expr) ? "1" : "0");
        break;

    default:
        DBUG_ASSERT (FALSE, "bad switch");
        break;
    }

    DBUG_RETURN ();
}

void
PHUTwriteUnionSet (FILE *handle, node *exprs, lut_t *varlut, char *tag, bool isunionset,
                   char *lhsname)
{
    int j;
    int k;
    int m;
    int mone;
    int n;
    node *idlist;
    node *expr;
    node *exprsone;
    node *avis;
    char *txt;
    bool wasor;

    DBUG_ENTER ();

    idlist = GetIslSetVariablesFromLut (varlut);
    n = TCcountExprs (idlist);
    if (0 != n) {

        fprintf (handle, "\n# %s: %s\n\n", tag, lhsname); // Write vars
        WriteParameterVariables (handle, idlist);         // Write parameter variables
        fprintf (handle, " \n { \n");
        WriteSetVariables (handle, idlist);
        if (!isunionset) { // Write union map
            fprintf (handle, " ->");
            WriteSetVariables (handle, idlist);
        }
        fprintf (handle, " :\n");
        // Write existentially qualified vars
        WriteExistsSetVariables (handle, idlist);
        fprintf (handle, "\n");

        // Append constraints
        m = TCcountExprs (exprs);
        for (j = 0; j < m; j++) {
            wasor = FALSE;
            exprsone = TCgetNthExprsExpr (j, exprs);
            DBUG_ASSERT (N_exprs == NODE_TYPE (exprsone), "Wrong constraint type");
            mone = TCcountExprs (exprsone);

            // min() and max() are non-infix fns
            if ((5 == TCcountExprs (exprsone))
                && ((F_min_SxS == ISLFN (exprsone)) || (F_max_SxS == ISLFN (exprsone)))) {
                avis = ID_AVIS (TCgetNthExprsExpr (0, exprsone));
                printIslName (handle, avis);
                fprintf (handle, "%s",
                         Prf2Isl (PRF_PRF (TCgetNthExprsExpr (1, exprsone)))); // =
                // min/max
                fprintf (handle, "%s(",
                         Prf2Isl (PRF_PRF (TCgetNthExprsExpr (3, exprsone))));
                printIslArg (handle, TCgetNthExprsExpr (2, exprsone));
                fprintf (handle, ",");
                printIslArg (handle, TCgetNthExprsExpr (4, exprsone));
                fprintf (handle, ")");
            } else {
                for (k = 0; k < mone; k++) { // Emit one constraint
                    expr = TCgetNthExprsExpr (k, exprsone);
                    switch (NODE_TYPE (expr)) {
                    default:
                        DBUG_ASSERT (FALSE, "Unexpected constraint node type");
                        break;

                    case N_id:
                    case N_num:
                    case N_bool:
                        printIslArg (handle, expr);
                        break;

                    case N_prf:
                        switch (PRF_PRF (expr)) {
                        default:
                            fprintf (handle, "%s", Prf2Isl (PRF_PRF (expr)));
                            break;
                        case F_or_SxS:
                            fprintf (handle, "\n   or\n ");
                            wasor = TRUE;
                            break;
                        case F_min_SxS:
                        case F_max_SxS:
                            DBUG_ASSERT (FALSE, "coding error");
                            break;
                        }
                        break;

                    case N_char: // Support for disjunction: ISL does not support !=
                        DBUG_ASSERT ('|' == CHAR_VAL (expr), "Expected disjunction |");
                        wasor = TRUE;
                        break;
                    }
                    fprintf (handle, " ");
                }
            }

            if (j < (m - 1)) { // Handle conjunctions of constraints
                txt = wasor ? "" : "\n   and\n ";
                wasor = FALSE;
                fprintf (handle, "%s", txt);
            }
        }

        fprintf (handle, "\n  }\n\n");
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool PHUTisCompatibleAffinePrf( prf nprf)
 *
 * @brief Predicate for affine node finding.
 *
 * @param A PRF_PRF
 *
 * @return TRUE if nprf member fns
 *
 ******************************************************************************/
bool
PHUTisCompatibleAffinePrf (prf nprf)
{
    bool z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_idx_sel:
    case F_sel_VxA:
    case F_lt_SxS:
    case F_le_SxS:
    case F_eq_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
    case F_neq_SxS:
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_non_neg_val_S:
    case F_min_SxS:
    case F_max_SxS:
    case F_mod_SxS:
    case F_aplmod_SxS:
    case F_abs_S:
    case F_neg_S:
    case F_shape_A:
    case F_not_S:
    case F_saabind:
    case F_dim_A:
        z = TRUE;
        break;

    default:
        z = FALSE;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool isDyadicPrf( prf nprf)
 *
 * @brief Predicate for monadic vs. dyadic prfs
 *
 * @param An N_prf
 *
 * @return TRUE if nprf is dyadic
 *
 ******************************************************************************/
static bool
isDyadicPrf (prf nprf)
{
    bool z;

    DBUG_ENTER ();

    switch (nprf) {
    case F_lt_SxS:
    case F_le_SxS:
    case F_eq_SxS:
    case F_ge_SxS:
    case F_gt_SxS:
    case F_neq_SxS:
    case F_val_lt_val_SxS:
    case F_val_le_val_SxS:
    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_min_SxS:
    case F_max_SxS:
    case F_mod_SxS:
    case F_aplmod_SxS:
        z = TRUE;
        break;

    case F_abs_S: // Monadic
    case F_neg_S:
    case F_non_neg_val_S:
    case F_shape_A:
    case F_not_S:
    case F_dim_A:
        z = FALSE;
        break;

    default:
        z = FALSE;
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn bool PHUTisCompatibleAffineTypes( node *arg_node)
 *
 * @brief ISL only supports integer (and Booleans, due to
 *        coercions made here), so we forbid other types.
 *
 *        dim() and shape() produce integer results, so we accept them,
 *        regardless of argument type.
 *
 * @param arg_node: An N_prf
 *
 * @return TRUE if arg_node arguments are all Boolean and/or integer
 *
 ******************************************************************************/
bool
PHUTisCompatibleAffineTypes (node *arg_node)
{
    bool z;
    node *avis;

    DBUG_ENTER ();

    z = (F_dim_A == PRF_PRF (arg_node)) || (F_shape_A == PRF_PRF (arg_node));
    avis = Node2Avis (PRF_ARG1 (arg_node));
    z = z || TUisBoolScalar (AVIS_TYPE (avis)) || TUisIntScalar (AVIS_TYPE (avis));
    if (isDyadicPrf (PRF_PRF (arg_node))) {
        avis = Node2Avis (PRF_ARG2 (arg_node));
        z = z && (TUisBoolScalar (AVIS_TYPE (avis)) || TUisIntScalar (AVIS_TYPE (avis)));
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleNid( node *arg_node, node *rhs, node *fundef, lut_t *varlut)
 *
 *
 * @brief Handler for N_id = N_id
 *        Generate: arg_node = rhs
 *        Set the ISL variable type. If we have no new information about
 *         the variable, then it is an ISL parameter. Otherwise, it's
 *         existential.
 *
 * @param  arg_node: The N_avis for an assign with N_id as rhs
 * @param  rhs: The N_id
 * @param  loopcount: The loopcount to be used for the APV in a loopfun,
 *                    or -1.
 *
 * NB. In normal operation, calls to this code should be infrequent, due to
 *     the work done by CSE and VP.
 *
 ******************************************************************************/
static node *
HandleNid (node *arg_node, node *rhs, node *fundef, lut_t *varlut, int loopcount)
{
    node *res = NULL;
    int islclass;

    DBUG_ENTER ();

    DBUG_PRINT ("Entering HandleNid for lhs=%s", AVIS_NAME (arg_node));
#ifdef PREMATURE // Kills PHUThandleAPV
    islclass = (LFUisLoopFunDependent (fundef, rhs)) ? AVIS_ISLCLASSSETVARIABLE
                                                     : AVIS_ISLCLASSEXISTENTIAL;
#else  // PREMATURE // Kills PHUThandleAPV
    islclass = AVIS_ISLCLASSEXISTENTIAL;
#endif // PREMATURE // Kills PHUThandleAPV
    if (PHUTinsertVarIntoLut (rhs, varlut, fundef, islclass)) {
        res = PHUTcollectAffineExprsLocal (rhs, fundef, varlut, NULL,
                                           AVIS_ISLCLASSUNDEFINED, loopcount);
        AVIS_ISLCLASS (arg_node)
          = (NULL == res) ? AVIS_ISLCLASSPARAMETER : AVIS_ISLCLASSSETVARIABLE;
        res = BuildIslSimpleConstraint (arg_node, F_eq_SxS, rhs, NOPRFOP, NULL);
        PHUTsetIslTree (ID_AVIS (arg_node), res);
    } else {
        DBUG_ASSERT (NULL != AVIS_ISLTREE (ID_AVIS (arg_node)), "No ISLTREE found");
        res = DUPdoDupTree (AVIS_ISLTREE (ID_AVIS (arg_node)));
    }

    DBUG_PRINT ("Leaving HandleNid for lhs=%s", AVIS_NAME (arg_node));

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *collectInitialValueExprs()
 *
 * @brief Given initialvalue inside a LACFUN, collect the exprs chain
 *        to create the initial value in its caller.
 * @param: calleriv: Name of initial value in lacfun caller
 * @param: callerfundef: lacfun caller
 * @param: varlut: LUT for names
 * @param: loopcount: The loopcount to be used by the APV in a loopfun,
 *                    or -1.
 *
 * @return An N_exprs chain representing an ISL constraint for the
 *         initial value of inneriv.
 *
 ******************************************************************************/
static node *
collectInitialValueExprs (node *calleriv, node *callerfundef, lut_t *varlut,
                          int loopcount)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = PHUTcollectAffineExprsLocal (calleriv, callerfundef, varlut, NULL,
                                       AVIS_ISLCLASSUNDEFINED, loopcount);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUThandleLoopfunArg(...)
 *
 * @brief We are in a LOOPFUN.
 *
 * @param  nid: An N_id with no AVIS_SSAASSIGN, but we
 *         know that nid is not a WITHID element, so we assume
 *         we are in a LACFUN (or defined fun), looking at one of the
 *         function's arguments.
 *         nid is already in varlut.
 *
 * @param  res: Incoming N_exprs chain for ISL
 * @param  callerassign: The N_assign node of the external N_ap call to this
 *                       LACFUN
 * @param: callerfundef: The N_fundef node of the LACFUN's calling function
 * @param  outerexprs: The ISL N_exprs chain for the external initial value
 * @param  calleriv: The N_id node representing avis' initial value, from the
 *                       external caller.
 * @param  loopcount: The loopcount to be used for the APV in a loopfun,
 *                    or -1 if otherwise.
 *
 * @return An N_exprs chain representing the ISL induction-variable constraint,
 *         appended to res.
 *         Otherwise, incoming res.
 *
 * NB. If the increment is positive, we generate, where rcv is the
 *     recursive call assign:
 *
 *         rcv' >= calleriv
 *         rcv' = calleriv + N * increment // where N may be unknown
 *         rcv' [dualprf] lim                  // This is for induction variable only
 *         rcv' = rcv
 *
 *     If the increment is negative, we generate:
 *
 *         rcv' <= calleriv
 *         rcv' = calleriv + N * increment // where N may be unknown
 *         rcv' [dualprf] lim                  // This is for induction variable only
 *         rcv' = rcv
 *
 *      dualprf is the dual to the relational function for the LACFUN's N_cond;
 *        e.g., if the condfun is _lt_SxS_, the dualprf is _ge_SxS.
 *      initialvalue is the initial value of the loop-induction variable,
 *      from the LACFUN caller.
 *
 *      lim is the non-induction-variable argument to the condfun.
 *
 ******************************************************************************/
static node *
PHUThandleLoopfunArg (node *nid, node *fundef, lut_t *varlut, node *res,
                      node *callerassign, node *callerfundef, node *outerexprs,
                      node *calleriv, int loopcount)
{
    node *rcv = NULL;
    node *reccallargs = NULL;
    node *reccallass = NULL;
    node *avis;
    node *z = NULL;

    DBUG_ENTER ();

    avis = Node2Avis (nid);
    DBUG_PRINT ("Looking at variable %s in LOOPFUN %s", AVIS_NAME (avis),
                FUNDEF_NAME (fundef));

    // If the variable is loop-independent, we are done, except for
    // grabbing outside initial value.
    // See unit test:
    // ~/sac/testsuite/optimizations/pogorelationals/SCSprf_lt_SxS.LIR.sac
    reccallass = LFUfindRecursiveCallAssign (fundef);
    reccallargs = AP_ARGS (LET_EXPR (ASSIGN_STMT (reccallass)));
    rcv = LFUgetRecursiveCallVariableFromArgs (avis, fundef, reccallargs);
    if (LFUisLoopFunDependent (fundef, nid)) {
        DBUG_PRINT ("LACFUN %s arg %s has recursive call value of %s",
                    FUNDEF_NAME (fundef), AVIS_NAME (avis), AVIS_NAME (ID_AVIS (rcv)));
        z = PHUTanalyzeLoopDependentVariable (nid, rcv, fundef, varlut, loopcount);
        res = TCappendExprs (res, z);

    } else {
        DBUG_PRINT ("LACFUN %s arg %s is loop-independent", FUNDEF_NAME (fundef),
                    AVIS_NAME (avis));
        // Propagate caller's Loop-independent value into LOOPFUN.
        res = BuildIslSimpleConstraint (nid, F_eq_SxS, calleriv, NOPRFOP, NULL);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUThandleCondfunArg(...)
 *
 * @brief We are in a CONDFUN
 *
 * @param  nid: An N_id with no AVIS_SSAASSIGN, but we
 *         know that nid is not a WITHID element, so we assume
 *         we are in a LACFUN (or defined fun), looking at one of the
 *         function's arguments.
 *         nid is already in varlut.
 *
 * @param  res: Incoming N_exprs chain for ISL
 * @param  callerassign: The N_assign node of the external N_ap call to this
 *                       LACFUN
 * @param: callerfundef: The N_fundef node of the LACFUN's calling function
 * @param  outerexprs: The ISL N_exprs chain for the external initial value
 * @param  calleriv: The N_id node representing avis' initial value, from the
 *                       external caller.
 *
 * @return An N_exprs chain representing the ISL induction-variable constraint,
 *         appended to res.
 *         Otherwise, incoming res.
 *
 ******************************************************************************/
static node *
PHUThandleCondfunArg (node *nid, node *fundef, lut_t *varlut, node *res,
                      node *callerassign, node *callerfundef, node *outerexprs,
                      node *calleriv)
{
    DBUG_ENTER ();

    // Create inneriv = calleriv;
    res = BuildIslSimpleConstraint (nid, F_eq_SxS, calleriv, NOPRFOP, NULL);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node* rcv2CallerVar(...)
 *
 * @brief Recursive call variable to caller's variable
 *
 * @param  rcv: An N_id or N_avis, a recursive call variable.
 *
 * @param  fundef: The N_fundef containing rcv.
 *
 * @return If rcv is a recursive call variable in a loopfun,
 *         return the corresponding N_id in the calling function.
 *         Otherwise, return NULL.
 *
 ******************************************************************************/
static node *
rcv2CallerVar (node *rcv, node *fundef)
{
    node *argvar;
    node *res = NULL;

    DBUG_ENTER ();

    // II' -> II  (because we search FUNDEF_ARGS)
    argvar = LFUgetArgFromRecursiveCallVariable (rcv, fundef);
    res = LFUgetCallerVariableFromArg (argvar, fundef);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node node *PHUThandleLacfunArg()
 *
 * @brief Handle a LOOPFUN or CONDFUN argument.
 *
 * @param  avis: an N_id or N_avis  with no AVIS_SSAASSIGN. We
 *         know that it is not a WITHID element, so we assume
 *         we are in a LACFUN (or defined fun), looking at one of the
 *         function's arguments.
 *         avis is already in varlut.
 * @param  loopcount: the loopcount to be used for the APV in a
 *         loopfun, or -1 otherwise.
 *
 *         Our condfun unit test is
 *         ~/sac/testsuite/optimizations/pogo/condfun.sac.
 *         Loopfun unit tests are in the same folder.
 *
 * @param  res: Incoming N_exprs chain for ISL
 *
 * @return An N_exprs chain representing an ISL constraint, appended to res.
 *         Otherwise, incoming res.
 *
 ******************************************************************************/
static node *
PHUThandleLacfunArg (node *nid, node *fundef, lut_t *varlut, node *res, int loopcount)
{
    node *ext_assign;
    node *calleriv;
    node *outerexprs;
    node *callerfundef;
    node *avis;

    DBUG_ENTER ();

    avis = Node2Avis (nid);
    // N_assign of External call (N_ap) to LACFUN
    ext_assign = FUNDEF_CALLAP (fundef);
    if (NULL != ext_assign) { // Current function may not be a LACFUN
        DBUG_ASSERT (N_assign == NODE_TYPE (ext_assign),
                     "Expected FUNDEF_CALLAP to be N_assign");
        DBUG_ASSERT (AP_FUNDEF (LET_EXPR (ASSIGN_STMT (ext_assign))) == fundef,
                     "Expected FUNDEF_CALLAP to point to its N_ap node");

        // Find caller's value for avis.
        calleriv = rcv2CallerVar (avis, fundef);
        callerfundef = FUNDEF_CALLERFUNDEF (fundef);
        outerexprs = PHUTgenerateAffineExprs (calleriv, callerfundef, varlut,
                                              AVIS_ISLCLASSEXISTENTIAL, loopcount);

        DBUG_PRINT ("Building AFT for LACFUN var %s from caller's %s", AVIS_NAME (avis),
                    AVIS_NAME (calleriv));

        if (FUNDEF_ISCONDFUN (fundef)) {
            res = PHUThandleCondfunArg (nid, fundef, varlut, res, ext_assign,
                                        callerfundef, outerexprs, calleriv);
            res = TCappendExprs (res, outerexprs);
        }

        if (FUNDEF_ISLOOPFUN (fundef)) {
            res = PHUThandleLoopfunArg (nid, fundef, varlut, res, ext_assign,
                                        callerfundef, outerexprs, calleriv, loopcount);
            res = TCappendExprs (res, outerexprs);
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleNumber( node *arg_node...
 *
 * @brief Handler for N_id = N_num or N_bool
 *
 * @param  arg_node: The N_avis for an assign with N_num/N_bool as rhs
 * @param  rhs: The N_num/N_bool
 *         If rhs is NULL, then arg_node may be AKV, in which we
 *         treat its value as rhs.
 * @param  res: Incoming N_exprs chain
 *
 * @return An N_exprs chain representing an ISL constraint, appended to res.
 *         All the N_exprs chain values will be N_num or N_bool nodes.
 *         Otherwise, incoming res.
 *
 ******************************************************************************/
static node *
HandleNumber (node *arg_node, node *rhs, node *fundef, lut_t *varlut, node *res)
{
    node *z;

    DBUG_ENTER ();

    DBUG_PRINT ("HandleNumber for lhs=%s", AVIS_NAME (arg_node));
    if ((NULL == rhs) && TYisAKV (AVIS_TYPE (arg_node))) {
        AVIS_ISLCLASS (arg_node) = AVIS_ISLCLASSEXISTENTIAL;
        rhs = TBmakeNum (TUtype2Int (AVIS_TYPE (arg_node)));
    }
    z = BuildIslSimpleConstraint (arg_node, F_eq_SxS, rhs, NOPRFOP, NULL);
    res = TCappendExprs (res, z);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleCompositionWithShape( node *arg_node...)
 *
 * @brief Handler for compositions of indexing on shape:
 *        Currently supported:
 *          shpel = idx_sel( offset, _shape_A_( var));
 *          shpel = _sel_VxA_( iv, _shape_A_( var));
 *          Both of these will generate shpel >= 0.
 *
 * @param  arg_node: The N_avis for an assign.
 * @param  rhs: The assign's rhs
 * @param  res: Incoming N_exprs chain
 * @param  loopcount: The loopcount to be used for the APV in a loopfun;
 *                    else -1.
 *
 * @return An N_exprs chain (or NULL)
 *         representing an ISL constraint, appended to res.
 *         Otherwise, incoming res.
 *
 ******************************************************************************/
static node *
HandleCompositionWithShape (node *arg_node, node *rhs, node *fundef, lut_t *varlut,
                            node *res, int loopcount)
{
    node *z = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    node *iv = NULL;
    node *x = NULL;
    node *m = NULL;

    DBUG_ENTER ();

    /* z = _sel_VxA_( iv, x); */
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&x), 0));
    /* z = _idx_sel( offset, x); */
    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&x), 0));
    /* x = _shape_A_( m ); */
    pat3 = PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&m), 0));

    if ((PMmatchFlat (pat1, rhs) || PMmatchFlat (pat2, rhs)) && PMmatchFlat (pat3, x)) {
        z = BuildIslSimpleConstraint (arg_node, F_ge_SxS, TBmakeNum (0), NOPRFOP, NULL);
        res = TCappendExprs (res, z);
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_PRINT ("Leaving HandleCompositionWithShape for lhs=%s", AVIS_NAME (arg_node));

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleIota( node *arg_node...)
 *
 * @brief Handler for indexing from iota(N)
 *        Currently supported:
 *          Q = genarray([shp], Scalar);
 *          el = idx_sel( offset, Q);  OR
 *          el = _sel_VxA_( iv, Q);
 *
 * @param  arg_node: The N_ids node that defines el.
 * @param  loopcount: The loopcount to be used for the APV in a loopfun,
 *                    else -1.
 *
 * @return: If arg_node selects from a with-loop, and that
 *          loop is simple (See WLUTgetGenarrayScalar for details),
 *          we return constraints on the values in the with-loop.
 *          Else NULL.
 *
 * NB.     This code assumes that -check c will ensure that
 *         shp satisfies rank, shape, and value requirements
 *         ( index error checking) for the index operation.
 *
 ******************************************************************************/
static node *
HandleIota (node *arg_node, node *fundef, lut_t *varlut, int loopcount)
{
    node *res = NULL;
    node *z = NULL;
    pattern *pat1;
    pattern *pat2;
    node *iv = NULL;
    node *q = NULL;
    node *s = NULL;
    node *wl = NULL;
    node *lb = NULL;
    node *ub = NULL;
    node *lbub = NULL;
    node *withidids = NULL;
    node *id;
    int idsidx;

    DBUG_ENTER ();

    id = TBmakeId (IDS_AVIS (arg_node));
    /* z = _sel_VxA_( iv, q); */
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&q), 0));
    /* z = _idx_sel( offset, q); */
    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&q), 0));
    if ((PMmatchFlat (pat1, id) || PMmatchFlat (pat2, id))) {
        // We are doing a select from a with-loop.
        s = WLUTgetGenarrayScalar (q, FALSE);
        if (NULL != s) { // s is a member of N_WITHIDS.
            // Find the WL
            wl = WLUTid2With (q);
            if (N_with == NODE_TYPE (wl)) {
                // Find generator elements for LB and UB.
                withidids = WITHID_IDS (PART_WITHID (WITH_PART (wl)));
                idsidx = TClookupIdsNode (withidids, s);
                DBUG_ASSERT (-1 != idsidx, "Could not find withidids element");
                lbub = GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (wl)));
                if (N_array == NODE_TYPE (lbub)) {
                    // Emit lb <= s
                    lb = TCgetNthExprsExpr (idsidx, ARRAY_AELEMS (lbub));
                    lb = PHUTskipChainedAssigns (lb);
                    z = PHUTcollectAffineExprsLocal (lb, fundef, varlut, NULL,
                                                     AVIS_ISLCLASSEXISTENTIAL, loopcount);
                    res = TCappendExprs (res, z);
                    z = PHUTcollectAffineExprsLocal (s, fundef, varlut, NULL,
                                                     AVIS_ISLCLASSSETVARIABLE, loopcount);
                    res = TCappendExprs (res, z);
                    z = BuildIslSimpleConstraint (lb, F_le_SxS, s, NOPRFOP, NULL);
                    res = TCappendExprs (res, z);
                }

                lbub = GENERATOR_BOUND2 (PART_GENERATOR (WITH_PART (wl)));
                if (N_array == NODE_TYPE (lbub)) {
                    // Emit s < ub
                    ub = TCgetNthExprsExpr (idsidx, ARRAY_AELEMS (lbub));
                    ub = PHUTskipChainedAssigns (ub);
                    z = PHUTcollectAffineExprsLocal (ub, fundef, varlut, NULL,
                                                     AVIS_ISLCLASSEXISTENTIAL, loopcount);
                    res = TCappendExprs (res, z);
                    z = BuildIslSimpleConstraint (s, F_lt_SxS, ub, NOPRFOP, NULL);
                    res = TCappendExprs (res, z);
                }
            }
            // Now make el = s (in iv=[i,j,s,k] in the WL generator)
            z = BuildIslSimpleConstraint (arg_node, F_eq_SxS, s, NOPRFOP, NULL);
            res = TCappendExprs (res, z);
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_PRINT ("Leaving HandleIota for lhs=%s", AVIS_NAME (ID_AVIS (id)));
    id = FREEdoFreeNode (id);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleNprf( node *arg_node...
 *
 * @brief If rhs is an affine function, generate ISL contraints for it.
 *
 *         z = x + y;
 *         z = x - y;
 *         z = x * const
 *         z = const * x;
 *         z =   - y;
 *         z = non_neg_val( y);
 *
 *
 * @param  arg_node: The N_avis for an assign with N_prf as rhs
 * @param  rhs; the rhs of the N_assign
 * @param  res: Incoming N_exprs chain
 * @param  loopcount: The loopcount to be used in the APV for a loopfun;
 *                    else -1.
 *
 * @return An N_exprs chain, representing a polylib Matrix row, appended to res.
 *         All the N_exprs chain values will be N_num nodes.
 *
 ******************************************************************************/
static node *
HandleNprf (node *arg_node, node *rhs, node *fundef, lut_t *varlut, node *res,
            int loopcount)
{
    node *assgn;
    node *ids = NULL;
    node *argavis;
    node *z = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *arg3 = NULL;
    node *arg1aft = NULL;
    node *arg2aft = NULL;
    node *arg3aft = NULL;

    DBUG_ENTER ();

    assgn = AVIS_SSAASSIGN (arg_node);
    if (NULL != assgn) {
        ids = LET_IDS (ASSIGN_STMT (assgn));
        if ((PHUTisCompatibleAffinePrf (PRF_PRF (rhs)))
            && (PHUTisCompatibleAffineTypes (rhs))) {
            DBUG_PRINT ("Entering HandleNprf for ids=%s", AVIS_NAME (IDS_AVIS (ids)));

            // Deal with PRF_ARGs
            arg1 = PHUTskipChainedAssigns (PRF_ARG1 (rhs));
            arg1aft = PHUTcollectAffineExprsLocal (arg1, fundef, varlut, NULL,
                                                   AVIS_ISLCLASSEXISTENTIAL, loopcount);
            res = TCappendExprs (res, arg1aft);
            if (isDyadicPrf (PRF_PRF (rhs))) {
                arg2 = PHUTskipChainedAssigns (PRF_ARG2 (rhs));
                arg2aft
                  = PHUTcollectAffineExprsLocal (arg2, fundef, varlut, NULL,
                                                 AVIS_ISLCLASSEXISTENTIAL, loopcount);
                res = TCappendExprs (res, arg2aft);
            }

            switch (PRF_PRF (rhs)) {
            case F_add_SxS:
            case F_sub_SxS:
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, PRF_PRF (rhs), arg2);
                res = TCappendExprs (res, z);
                break;

            case F_non_neg_val_S:
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
                z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                break;

            case F_min_SxS:
            case F_max_SxS:
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, PRF_PRF (rhs), arg2);
                res = TCappendExprs (res, z);
                break;

            case F_mul_SxS: // We need one constant argument
                argavis = ID_AVIS (arg1);
                if ((TYisAKV (AVIS_TYPE (argavis)))
                    || (TYisAKV (AVIS_TYPE (ID_AVIS (arg2))))) {
                    // z = ( const * y)
                    z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, PRF_PRF (rhs),
                                                  arg2);
                    res = TCappendExprs (res, z);
                }
                break;

            case F_shape_A:
                argavis = ID_AVIS (arg1);
                if ((TYisAKS (AVIS_TYPE (argavis))) || (TYisAKV (AVIS_TYPE (argavis)))) {
                    // z >= 0
                    z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                                  NULL);
                    res = TCappendExprs (res, z);
                    // maximum: z < shape
                    if ((NULL != AVIS_SHAPE (argavis))
                        && (N_id == NODE_TYPE (AVIS_SHAPE (argavis)))) {
                        z = BuildIslSimpleConstraint (ids, F_lt_SxS, AVIS_SHAPE (argavis),
                                                      NOPRFOP, NULL);
                        res = TCappendExprs (res, z);
                    }
                }
                break;

            case F_dim_A:
                // z >= 0
                z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                break;

            case F_not_S: // Treat Booleans as integers(for POGO)
                // z = !y  --->  ( z = 1 - y) and (y >= 0) and (y <= 1)
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, TBmakeNum (1), F_sub_SxS,
                                              arg1);
                res = TCappendExprs (res, z);
                z = BuildIslSimpleConstraint (PRF_ARG1 (rhs), F_ge_SxS, TBmakeNum (0),
                                              NOPRFOP, NULL);
                res = TCappendExprs (res, z);
                z = BuildIslSimpleConstraint (PRF_ARG1 (rhs), F_le_SxS, TBmakeNum (1),
                                              NOPRFOP, NULL);
                res = TCappendExprs (res, z);
                break;

            case F_abs_S: // z >= 0
                z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                break;

            case F_aplmod_SxS:
                /*
                 *               Per the APL ISO N8485 standard:
                 *               For aplmod modulus: z = aplmod( x, y),
                 *               if ((x >= 0) && (y >= 0)) || (y >= 1)
                 *               then z >= 0.
                 *               [Not sure about the above, and I can't find N8485.]
                 *
                 *               Bear in mind that aplmod reverses the APL argument order,
                 *               so that the modulus is arg2.
                 *
                 *               SHARP APL says that the result (ignoring tolerant
                 * comparison) is always between 0 and the modulus. Ergo, ( z >= 0) and (
                 * z < abs( arg2)). This is the definition we will use.
                 */
                // mimumum:  z >= 0
                z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);

                // maximum:  z < abs( arg2)
                // So, if arg2 > 0 --->   z < arg2
                //     If arg2 < 0 --->   z < -arg2
                if (POGOisPositive (arg2, arg2aft, fundef, varlut)) {
                    z = BuildIslSimpleConstraint (ids, F_lt_SxS, arg2, NOPRFOP, NULL);
                    res = TCappendExprs (res, z);
                } else {
                    z = BuildIslSimpleConstraint (ids, F_lt_SxS, TBmakeNum (0), F_sub_SxS,
                                                  arg2);
                    res = TCappendExprs (res, z);
                }
                break;

            case F_mod_SxS:
                /*
                 *               For C99, if (arg1 >=0 ) && ( arg2 > 0), then:
                 *                        0 <= z < arg2
                 *
                 *               Apparently C99 chooses result sign to match
                 *               that of PRF_ARG1, but we sidestep that issue here.
                 *               FIXME: Perhaps extend this code to deal with negative
                 *               PRF_ARG1?
                 */
                // minimum
                if ((POGOisNonNegative (arg1, arg1aft, fundef, varlut))
                    && (POGOisPositive (arg2, arg2aft, fundef, varlut))) {
                    // z >= 0
                    z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                                  NULL);
                    res = TCappendExprs (res, z);
                    // z < PRF_ARG2( rhs)
                    z = BuildIslSimpleConstraint (ids, F_lt_SxS, arg2, NOPRFOP, NULL);
                    res = TCappendExprs (res, z);
                }
                break;

            case F_lt_SxS:
            case F_le_SxS:
            case F_eq_SxS:
            case F_ge_SxS:
            case F_gt_SxS:
            case F_neq_SxS:
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, PRF_PRF (rhs), arg2);
                res = TCappendExprs (res, z);
                break;

            case F_val_lt_val_SxS:
            case F_val_le_val_SxS:
                // Treat a guard within an affine expression as assignment.
                // z = PRF_ARG1;
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
                z = BuildIslSimpleConstraint (arg1, PRF_PRF (rhs), arg2, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
                break;

            case F_neg_S: //  z = -y;  -->  z = 0 - y
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, TBmakeNum (0), F_sub_SxS,
                                              arg1);
                res = TCappendExprs (res, z);
                break;

            case F_idx_sel:
            case F_sel_VxA:
                res = TCappendExprs (res, HandleIota (ids, fundef, varlut, loopcount));
                break;

            case F_saabind: // z = PRF_ARG3( rhs);
                // We need to capture the relationship among initial and final values
                // of LACFUN parameters. E.g., in simpleNonconstantUp.sac, we have a
                // loop from START to START+5, where the value of START is the result
                // of a function call [id(0)].
                arg3 = PHUTskipChainedAssigns (PRF_ARG3 (rhs));
                arg3aft
                  = PHUTcollectAffineExprsLocal (arg3, fundef, varlut, NULL,
                                                 AVIS_ISLCLASSEXISTENTIAL, loopcount);
                res = TCappendExprs (res, arg3aft);
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg3, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
                break;

            default:
                DBUG_UNREACHABLE ("Nprf coding time, senor");
                break;
            }

            DBUG_PRINT ("Leaving HandleNprf for ids=%s", AVIS_NAME (IDS_AVIS (ids)));
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn void PHUTsetIslTree( node *avis, node *aft)
 *
 * @brief Set AVIS_ISLTREE for avis, if currently NULL, to
 *        a copy of aft, the affine function tree that creates avis.
 *        If not NULL, get very upset.
 *
 *        If aft is NULL, do nothing.
 *
 * @param avis: an N_avis node
 * @param aft: the Affine Function Tree for this N_avis node.
 *
 * @return void
 *
 ******************************************************************************/
void
PHUTsetIslTree (node *avis, node *aft)
{
    DBUG_ENTER ();

    if (NULL != aft) {
        DBUG_ASSERT (NULL == AVIS_ISLTREE (avis), "AVIS_ISLTREE not NULL");
#ifdef FIXME // probably bad idea
        AVIS_ISLTREE (avis) = DUPdoDupTree (aft);
#endif // FIXME // probably bad idea
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUTcollectAffineExprsLocal( ...)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *
 * @param arg_node: an N_id node or an N_avis node or an N_ids node
 * @param res: Partially built N_exprs chain
 * @param islclass: optional islclass value to be set on
 *                  arg_node.
 * @param loopcount: the loopcount to be used in generating the APV
 *                   for a loopfun, or -1 otherwise.
 *
 * @return A maximal N_exprs chain of expressions, appended to the incoming res.
 *
 *         See PHUTcollectExprs for more details.
 *
 ******************************************************************************/
node *
PHUTcollectAffineExprsLocal (node *arg_node, node *fundef, lut_t *varlut, node *res,
                             int islclass, int loopcount)
{
    node *assgn = NULL;
    node *res2 = NULL;
    node *res3 = NULL;
    node *rhs = NULL;
    node *avis = NULL;
    node *npart = NULL;
    node *nid;
    int cls;

    DBUG_ENTER ();

    nid = PHUTskipChainedAssigns (arg_node);
    avis = Node2Avis (nid);
    if ((NULL != avis)) { // N_num exits here
        DBUG_PRINT ("Looking at %s", AVIS_NAME (avis));
        assgn = AVIS_SSAASSIGN (avis);
        cls = (AVIS_ISLCLASSUNDEFINED != islclass) ? islclass : AVIS_ISLCLASSEXISTENTIAL;
        if (NULL != AVIS_ISLTREE (avis)) {
            res2 = DUPdoDupTree (AVIS_ISLTREE (avis));
        } else {
            int fixme;
#ifdef FIXME
            if (PHUTinsertVarIntoLut (avis, varlut, fundef, cls)) {
#else  // FIXME
            PHUTinsertVarIntoLut (avis, varlut, fundef, cls);
            if (TRUE) {
#endif //  FIXME
       // Handle RHS for all modes.
                if ((NULL != assgn) && (N_let == NODE_TYPE (ASSIGN_STMT (assgn)))) {
                    rhs = LET_EXPR (ASSIGN_STMT (assgn));

                    switch (NODE_TYPE (rhs)) {
                    case N_id: // straight assign: var2 = var1.
                        res2 = HandleNid (avis, rhs, fundef, varlut, loopcount);
                        PHUTsetIslTree (avis, res2);
                        break;

                    case N_prf:
                        res2 = HandleNprf (avis, rhs, fundef, varlut, NULL, loopcount);
                        res3 = HandleCompositionWithShape (avis, rhs, fundef, varlut,
                                                           NULL, loopcount);
                        res2 = TCappendExprs (res2, res3);
                        PHUTsetIslTree (avis, res2);
                        break;

                    case N_bool:
                    case N_num:
                        res2 = HandleNumber (avis, rhs, fundef, varlut, NULL);
                        PHUTsetIslTree (avis, res2);
                        break;

                    case N_ap: // The buck stops here - we can't look any further.
                        AVIS_ISLCLASS (avis) = AVIS_ISLCLASSPARAMETER;
                        break;

                    default:
                        break;
                    }
                } else {
                    DBUG_ASSERT (NULL == assgn, "Confusion about AVIS_SSAASSIGN");
                    // This may be a WITHID or a function parameter.
                    // If it is constant, then we treat it as a number.
                    if (TYisAKV (AVIS_TYPE (avis))) {
                        res3 = HandleNumber (avis, rhs, fundef, varlut, NULL);
                        res2 = TCappendExprs (res2, res3);
                        PHUTsetIslTree (avis, res2);
                    } else {
                        // This presumes that AVIS_NPART is being maintained by our
                        // caller.
                        npart = AVIS_NPART (avis);
                        if (NULL != npart) {
                            if (-1
                                != LFUindexOfMemberIds (avis, WITHID_IDS (
                                                                PART_WITHID (npart)))) {
                                // arg_node is a withid element.
                                res3 = PHUTcollectWlGenerator (avis, fundef, varlut, NULL,
                                                               loopcount);
                                res2 = TCappendExprs (res2, res3);
                                PHUTsetIslTree (avis, res3);
                            } else {
                                // non-constant function parameter
                                DBUG_ASSERT (FALSE, "Coding time for lacfun args");
                            }
                        } else { // Not WITHID. Must be function parameter
                            res3 = PHUThandleLacfunArg (nid, fundef, varlut, NULL,
                                                        loopcount);
                            res2 = TCappendExprs (res2, res3);
                            PHUTsetIslTree (avis, res2);
                        }
                    }
                }
            }
        }
    }

    res = TCappendExprs (res, res2);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprs( node *arg_node, node *fundef, lut_t *varlut,
 *            int islclass)
 *
 * @brief Does a recursive search, starting at arg_node,
 *        for a maximal chain of affine instructions.
 *
 * @param arg_node: an N_id node or N_avis node.
 * @param fundef: The N_fundef for the current function. Used only for debugging.
 * @param varlut: The address of a LUT that serves to accumulate names for ISL
 * @param islclass: The AVIS_ISLCLASS value to be set for arg_node.
 * @param loopcount: The loopcount to be used by the APV in a loopfun;
 *                   else -1.
 *
 * @return A maximal N_exprs chain of expressions, whose elements may be
 *         either N_num or N_id nodes.
 *
 * Some examples:
 *
 * This is nakedCWLSimple.sac, from PWLF unit tests:
 *
 * int main()
 * {
 *   XXX = Array::iota(50);
 *   z = _sel_VxA_( [42], XXX);
 *   z = _sub_SxS_( z, 42);
 *   return(z);
 * }
 *
 * The naked _sel_() (Effectively, the consumerWL) has a polyhedron of 42;
 * XXX has generators of [0] and [50];
 *
 * PWLF has to compute the lower and upper bounds of the intersection of
 * these.
 *
 *
 * This is from AWLF unit test time2code.sac. If compiled with -doawlf -nowlf,
 * it generates this code:
 *
 *     p = _aplmod_SxS_( -1, m);   NB. AVIS_MIN(p) = 0;
 *     iv2 = _notemaxval( iv1, m); NB. aka AVIS_MAX( iv2) = m;
 *     iv3 = iv2 - p;
 *     iv4, p = _val_lt_val_SxS_( iv3, m);
 *
 *  What we want to generate for this, eventually, is two sets
 *  of constraints:
 *
 * Set A:
 *   AVIS_MIN(p)                  -->   p   >= 0
 *   iv3 = iv2 - p;               -->   iv3 = iv2 - p
 *   iv2 = _notemaxval( iv1, m);  -->   m   >= iv2 + 1
 *
 * Set B:
 *   val_lt_val_SxS_( iv3, m)     -->   iv3 >= m    NB. Must prove converse
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprs (node *arg_node, node *fundef, lut_t *varlut, int islclass,
                         int loopcount)
{
    node *res = NULL;

    DBUG_ENTER ();

    res
      = PHUTcollectAffineExprsLocal (arg_node, fundef, varlut, res, islclass, loopcount);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForGuard(...)
 *
 * @brief Construct an ISL transform for an N_prf guard or relational
 *
 * @param fn: PRF_PRF for a guard/relational primitive
 * @param arg1: PRF_ARG1 for a guard/relational primitive
 * @param arg2: PRF_ARG2 for a guard/relational primitive
 * @param fundef: The N_fundef for the current function, for debugging
 * @param relfn:  Either PRF_PRF( arg_node) or its companion function,
 *                e.g., if PRF_PRF is _gt_SxS_, its companion is _le_SxS.
 * @param stridesign: 1 or -1 for positive or negative stride; 0 if
 *                    stride is unknown.
 *                    Used only for X != Y
 *
 * @return A maximal N_exprs chain of expressions for the guard.
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForGuard (prf fn, node *arg1, node *arg2, node *fundef, prf relfn,
                                 lut_t *varlut, int stridesign)
{
    node *z = NULL;
    prf relprf;

    DBUG_ENTER ();

    arg1 = PHUTskipChainedAssigns (arg1);
    PHUTinsertVarIntoLut (arg1, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
    switch (relfn) {
    case F_non_neg_val_S:
        z = BuildIslSimpleConstraint (arg1, F_ge_SxS, TBmakeNum (0), NOPRFOP, NULL);
        break;

    case F_gt_SxS:
    case F_ge_SxS:
    case F_val_lt_val_SxS:
    case F_lt_SxS:
    case F_val_le_val_SxS:
    case F_le_SxS:
        if (F_non_neg_val_S == fn) { // fn vs. relfn
            z = BuildIslSimpleConstraint (arg1, F_lt_SxS, TBmakeNum (0), NOPRFOP, NULL);
        } else {
            arg2 = PHUTskipChainedAssigns (arg2);
            PHUTinsertVarIntoLut (arg2, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
            z = BuildIslSimpleConstraint (arg1, relfn, arg2, NOPRFOP, NULL);
        }
        break;

    case F_eq_SxS:
        arg2 = PHUTskipChainedAssigns (arg2);
        PHUTinsertVarIntoLut (arg2, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
        z = BuildIslSimpleConstraint (arg1, relfn, arg2, NOPRFOP, NULL);
        break;

    case F_neq_SxS:
        // This is harder. We need to construct a union
        // of (x<y) OR (x>y).
        arg2 = PHUTskipChainedAssigns (arg2);
        PHUTinsertVarIntoLut (arg2, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
        if (stridesign == 0) { // do not know stride, or may not be in for-loop
            z = BuildIslStrideConstraint (arg1, F_gt_SxS, arg2, F_or_SxS, arg1, F_lt_SxS,
                                          arg2);
        } else { // stride is known to be + or -
            relprf = (stridesign > 0) ? F_lt_SxS : F_gt_SxS;
            z = BuildIslSimpleConstraint (arg1, relprf, arg2, NOPRFOP, NULL);
        }
        break;

    default:
        DBUG_UNREACHABLE ("Coding time for guard polyhedron");
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForCondprf(...)
 *
 * @brief Construct an ISL transform for the condprf that controls
 *        iteration in a LOOPFUN
 *
 * @param fn: PRF_PRF for the condprf
 * @param arg1: PRF_ARG1 for the condprf
 * @param arg2: PRF_ARG2 for the condprf
 * @param fundef: The N_fundef for the current function, for debugging
 * @param relfn:  Either PRF_PRF( arg_node) or its companion function,
 *                e.g., if PRF_PRF is _gt_SxS_, its companion is _le_SxS.
 * @param stridesign: 1 or -1 for positive or negative stride; 0 if
 *                    stride is unknown.
 *                    Used only for X != Y
 *
 * @return A maximal N_exprs chain of expressions for the condprf.
 *
 * NB. The tricky part here is that we have to use the incoming
 *     loop-carried variable, and since we are using tail recursion,
 *     it has already been incremented/decremented.
 *     See various unit tests testsuite/optimization/plur.
 *
 *
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForCondprf (prf fn, node *arg1, node *arg2, node *fundef,
                                   prf relfn, lut_t *varlut, int stridesignum,
                                   node *strideid)
{
    node *z = NULL;

    DBUG_ENTER ();

    z = PHUTgenerateAffineExprsForGuard (fn, arg1, arg2, fundef, fn, varlut,
                                         stridesignum);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node
 *
 * @brief Compute the intersections among ISL polyhedra:
 *
 *          exprs1, exprs2, exprsintr
 *
 *        and, if exprs4 != NULL:
 *
 *          exprs1, exprs2, exprs4
 *
 *        The result is an element of sacpolylibinterface.h.
 *
 *        The monotonically increasing global.polylib_filenumber
 *        is for debugging, as we will generate MANY files.
 *
 * @param: exprspwl, exprscwl, exprsintr, exprs4 - integer N_exprs chains of
 *         n ISL input sets.
 * @param: varlut: a lut containing the N_avis names referenced
 *         by the affine function trees in the exprs chains.
 *         This becomes the "set variable" list for ISL.
 * @param: opcode - the POLY_OPCODE to select a sacislinterface operation.
 * @param: lhsname - the AVIS_NAME of the LHS of the expression we
 *         are trying to simplify.
 *
 * @result: a POLY_RET return code
 *
 * NB. PHUTcheckIntersection frees the N_exprs argument nodes
 *
 *****************************************************************************/
int
PHUTcheckIntersection (node *exprspwl, node *exprscwl, node *exprsfn, node *exprscfn,
                       lut_t *varlut, char opcode, char *lhsname)
{

    int res = POLY_RET_INVALID;

    DBUG_ENTER ();

    res
      = ISLUgetSetIntersections (exprspwl, exprscwl, exprsfn, exprscfn, varlut, lhsname);
    DBUG_PRINT ("ISLU intersection result is %d", res);
    exprspwl = (NULL != exprspwl) ? FREEdoFreeTree (exprspwl) : NULL;
    exprscwl = (NULL != exprscwl) ? FREEdoFreeTree (exprscwl) : NULL;
    exprsfn = (NULL != exprsfn) ? FREEdoFreeTree (exprsfn) : NULL;
    exprscfn = (NULL != exprscfn) ? FREEdoFreeTree (exprscfn) : NULL;

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForPwlfIntersect( node *cwliv, node *pwliv,
 *           lut_t *varlut)
 *
 * @brief Construct an ISL constraint for the intersect of a consumerWL
 *
 * @param cwliv: The N_avis for the consumerWL index vector.
 * @param pwliv: THe N_avis for the producerWL generator index vector
 * @param varlut: The lut we use to collect set variable names.
 * @param fundef: the N_fundef for the function containing cwliv and pwliv
 *
 * @return An N_exprs chain for the intersect, stating that:
 *         cwliv == pwliv
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForPwlfIntersect (node *cwliv, node *pwliv, lut_t *varlut,
                                         node *fundef)
{
    node *res;

    DBUG_ENTER ();

    PHUTinsertVarIntoLut (cwliv, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
    PHUTinsertVarIntoLut (pwliv, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
    res = BuildIslSimpleConstraint (cwliv, F_eq_SxS, pwliv, NOPRFOP, NULL);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node * extractInitialValue( node *outerexprs, node *outerinitialvalue)
 *
 * @brief Extract initial value for outerinitialvalue from outerexprs
 *
 * @param  outerexprs: an ISL N_exprs chain
 *         of the form  outerinitialvalue = iv
 * @param  outerinitialvalue: an N_id or N_avis node
 *
 * @return the iv, N_id for the initial value.
 *         Complain bitterly if not found.
 *
 ******************************************************************************/
static node *
extractInitialValue (node *outerexprs, node *outerinitialvalue)
{

    node *z = NULL;
    node *expr = NULL;
    node *oavis = NULL;
    int numexprs, curexprs;

    DBUG_ENTER ();

    oavis = Node2Avis (outerinitialvalue);
    numexprs = TCcountExprs (outerexprs);
    curexprs = 0;
    while ((NULL == z) && (curexprs < numexprs)) {
        expr = TCgetNthExprsExpr (curexprs, outerexprs);
        if (oavis == ID_AVIS (TCgetNthExprsExpr (0, expr))) {
            z = DUPdoDupNode (TCgetNthExprsExpr (2, expr));
        }
        curexprs++;
    }

    DBUG_ASSERT (NULL != z, "Could not find outer initial value");

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUThandleAPV(...)
 *
 * @brief Search exprsall for a PHUT assign to a loop-carried
 *        variable, II, of the form:
 *
 *            II' = II + stride
 *            II' = II - stride
 *            II' = stride  + II
 *
 *        where II' is the loopfun's recursive call N_id, stride
 *        is the stride,
 *        and II is the loopfun's argument corresponding to II'.
 *        I.e., we are looking for an arithmetic progression vector (APV).
 *
 * @param exprsall: An ISL N_exprs chain
 * @param fundef: the N_fundef for the LOOPFUN
 * @param varlut: The PHUT LUT
 * @param stridesign: The address of the for-loop stride signum,
 *        which we set if we can determine it.
 * @param lcv: the loop-carried variable that we found, or NULL
 * @param strideout: The N_id of the stride we found, or NULL.
 * @param loopcount: The loopcount to be used by the APV for a
 *                   loopfun, or -1.
 *
 * @return N_exprs chain elements if we can analyze the APV, of the form:
 *
 *           initialvalue prfi lcv
 *           lcv <= initialvalue + (abs(stride)*LIM) // Stride > 0
 *           or
 *           lcv >= initialvalue - (abs(stride)*LIM) // stride < 0
 *
 *         where LIM is an "existential variable". I.e., we do not
 *         know its value.
 *
 *         or
 *           NULL, if the analysis fails
 *
 *         prfi, prfz are based on the loop-terminating conditional,
 *         and on the sign of the stride.
 *
 * NB. There are several code patterns that may appear in LOOPFUNs,
 *     where II is the loop-carried argument and II' is the
 *     recursive call variable.
 *
 *      1.  II' = II +- stride  // This is the common case
 *          if( II' < LIMIT) {
 *              ...Loop( II')
 *          }
 *
 *
 *      2. II' = II +- stride  // This appears in unit test plur/bug1155.sac
 *                             // It appears to arise from using II++,
 *                             // instead of II = II + stride.
 *                             // It also arises with -doctz
 *         IItmp = II +- stride
 *         if( IItmp < 0) {
 *              ...Loop( II')
 *          }
 *
 *         In this case, we have to find the definition of II', and
 *         use it as the basis for determining stride and stridesign.
 *
 *      This appears to be introduced by AL and/or DL.
 *      Therefore, we have to look for either II or II' in the if() tree.
 *
 ******************************************************************************/
node *
PHUThandleAPV (node *exprsall, node *fundef, lut_t *varlut, int *stridesign, node **lcv,
               node **strideidout, int loopcount)
{
    node *res = NULL;
    node *exprs = NULL;
    node *strideid = NULL;
    node *exprsres = NULL;
    node *exprslarg = NULL;
    node *exprsrarg = NULL;
    node *reccallargs = NULL;
    node *limavis = NULL;
    node *limid = NULL;
    node *calleriv = NULL;
    node *outerexprs = NULL;
    node *inneriv = NULL;
    node *resel = NULL;
    node *argvar = NULL;
    node *iiprime = NULL;
    prf exprspfn;
    prf prfi;
    prf prfl;
    prf prfz;
    int mathsignum = 0;
    int stridesignum = 0;

    DBUG_ENTER ();

    reccallargs = AP_ARGS (LET_EXPR (ASSIGN_STMT (LFUfindRecursiveCallAssign (fundef))));

    while (NULL != exprsall) {
        exprs = EXPRS_EXPR (exprsall);
        strideid = NULL;
        exprsres = ISLVAR (exprs);
        exprslarg = ISLLARG (exprs);
        if (3 < TCcountExprs (exprs)) {
            exprspfn = ISLFN (exprs);
            exprsrarg = ISLRARG (exprs);
            argvar = LFUgetArgFromRecursiveCallVariable (exprsres, fundef);
            if (NULL != argvar) {
                // Look for a function of a loop-dependent call argument.
                // (Code pattern 1, above).
                // If we find one, the argument that is NOT loop-dependent
                // might be the increment/decrement.
                // Determine stride N_id and sign of stride op.
                if ((N_id == NODE_TYPE (exprslarg))
                    && ((F_add_SxS == exprspfn) || (F_sub_SxS == exprspfn))
                    && (argvar == ID_AVIS (exprslarg))) {
                    strideid = exprsrarg; //   II' = II +- stride
                    mathsignum = (exprspfn == F_sub_SxS) ? -1 : 1;
                } else if ((N_id == NODE_TYPE (exprsrarg)) && (F_add_SxS == exprspfn)
                           && (argvar == ID_AVIS (exprsrarg))) {
                    strideid = exprslarg; //   II' = stride + II
                    mathsignum = 1;
                }

            } else {
                // Look for code pattern 2.
                //
                // Try IItmp = II +- stride
                if ((N_id == NODE_TYPE (exprslarg))
                    && (LFUisLoopFunDependent (fundef, exprslarg))
                    && ((F_add_SxS == exprspfn) || (F_sub_SxS == exprspfn))
                    && (LFUisAvisMemberArg (ID_AVIS (exprslarg), FUNDEF_ARGS (fundef)))) {

                    // Find II' = II +- stride   or  II' = stride + II
                    iiprime
                      = LFUgetRecursiveCallVariableFromArgs (exprslarg, fundef,
                                                             AP_ARGS (
                                                               FUNDEF_LOOPRECURSIVEAP (
                                                                 fundef)));
                    strideid = LFUgetStrideForAffineFun (iiprime, exprslarg);
                    mathsignum = LFUgetMathSignumForAffineFun (iiprime, exprslarg);
                    argvar = ID_AVIS (exprslarg);

                } else { // Try IItmp = stride + II
                    if ((N_id == NODE_TYPE (exprsrarg))
                        && (LFUisLoopFunDependent (fundef, exprsrarg))
                        && ((F_add_SxS == exprspfn))
                        && (LFUisAvisMemberArg (ID_AVIS (exprsrarg),
                                                FUNDEF_ARGS (fundef)))) {
                        iiprime
                          = LFUgetRecursiveCallVariableFromArgs (exprsrarg, fundef,
                                                                 AP_ARGS (
                                                                   FUNDEF_LOOPRECURSIVEAP (
                                                                     fundef)));
                        strideid = LFUgetStrideForAffineFun (iiprime, exprsrarg);
                        mathsignum = LFUgetMathSignumForAffineFun (iiprime, exprsrarg);
                        argvar = ID_AVIS (exprsrarg);
                    }
                }
            }

            // Find sign of stride, correct for F_sub.
            stridesignum = (NULL == strideid) ? 0
                                              : mathsignum * SCSisPositive (strideid)
                                                  ? 1
                                                  : SCSisNegative (strideid) ? -1 : 0;

            // If we know sign of stride & loop count, we generate an ISL directive
            if ((0 != stridesignum)) {
                prfi = (stridesignum > 0) ? F_ge_SxS : F_le_SxS;
                prfz = (stridesignum > 0) ? F_lt_SxS : F_gt_SxS;
                prfl = (stridesignum > 0) ? F_le_SxS : F_ge_SxS;

                AVIS_ISLCLASS (argvar) = AVIS_ISLCLASSSETVARIABLE;

                // Find initial value of argvar in caller
                calleriv = rcv2CallerVar (argvar, fundef);
                outerexprs
                  = collectInitialValueExprs (calleriv, FUNDEF_CALLERFUNDEF (fundef),
                                              varlut, loopcount);
                limavis
                  = TBmakeAvis (TRAVtmpVarName ("LIM"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                limid = TBmakeId (limavis);
                PHUTinsertVarIntoLut (limavis, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
                inneriv = extractInitialValue (outerexprs, calleriv);

                // Construct, e.g., II >= 0  or II <= 0
                resel = BuildIslSimpleConstraint (argvar, prfi, inneriv, NOPRFOP, NULL);
                res = TCappendExprs (res, resel);

                // lastvalue = initialvalue + stride loopcount;
                resel = BuildIslStrideConstraint (argvar, prfl, inneriv, F_add_SxS,
                                                  strideid, NOPRFOP, limid);
                res = TCappendExprs (res, resel);
            }
        }
        exprsall = EXPRS_NEXT (exprsall);
    }
    *stridesign = stridesignum;
    *lcv = argvar; // II, not II'
    *strideidout = strideid;

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUTanalyzeLoopDependentVariable()
 *
 * @brief Analyze ANY loop-dependent value (not necessarily the
 *        one controlling the loop count)
 *
 * @param  nid - the parameter variable we are analyzing
 *               nid is already in varlut.
 * @param: rcv - the recursive call version of nid
 * @param: initialexprs - The ISL exprs chain for the initial value of the loop
 * @param: initialvalue - The external N_id name of the initial value
 *               of the loop
 * @param: loopcount - the loopcount to be used for the arithmetic
 *               progression vector we generate, or -1 otherwise.
 *
 * @return An N_exprs chain representing an ISL constraint for the
 *         loop induction variable, or NULL, if we are unable to
 *         deduce that.
 *
 *         We obtain the ISL exprs chain for the loop itself,
 *         then decide whether the stride is positive or negative.
 *
 *         This lets us generate, for positive stride:
 *
 *           nid >= initialvalue
 *           nid < initialvalue + (stride *loopcount)
 *
 *         Negative stride is the same, except the signs are reversed:
 *
 *           nid <= initialvalue
 *           nid >  initialvalue + (stride *loopcount)
 *
 *
 ******************************************************************************/
node *
PHUTanalyzeLoopDependentVariable (node *nid, node *rcv, node *fundef, lut_t *varlut,
                                  int loopcount)
{
    node *resel = NULL;
    node *rcvel = NULL;
    node *exprs;
    node *res = NULL;
    node *exprslarg = NULL;
    node *exprsrarg = NULL;
    node *exprsvar = NULL;
    node *limavis = NULL;
    node *calleriv = NULL;
    node *outerexprs = NULL;
    node *strideid = NULL;
    node *lpcnt = NULL;
    node *lpavis = NULL;
    node *lb;
    node *ub;
    node *navis;
    prf exprspfn = NOPRFOP;
    prf prfi;
    prf prfz;
    int stridesignum = 0; // -1 for negative, 1 for positive, 0 for unknown or 0.
    int mathsign = 1;
    bool swap;

    DBUG_ENTER ();

    resel = PHUTskipChainedAssigns (nid);
    rcvel = PHUTskipChainedAssigns (rcv);

    // Recursive call variable is existential.
    if (PHUTinsertVarIntoLut (rcvel, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL)) {
        // trace rcv back
        swap = FALSE;
        strideid = NULL;

        exprs = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (rcvel))));
        exprspfn = PRF_PRF (exprs);
        exprsvar = nid;
        exprslarg = PRF_ARG1 (exprs);
        exprsrarg = PRF_ARG2 (exprs);

        // Find value of stride. We are looking for: rcv = nid +- something, etc.
        if ((F_add_SxS == exprspfn) && (N_id == NODE_TYPE (exprslarg))
            && (ID_AVIS (nid) == ID_AVIS (exprslarg))) {
            strideid = exprsrarg; // nid + something
        } else if ((F_add_SxS == exprspfn) && (N_id == NODE_TYPE (exprsrarg))
                   && (ID_AVIS (nid) == ID_AVIS (exprsrarg))) {
            strideid = exprslarg; // something + nid
        } else if ((F_sub_SxS == exprspfn) && (N_id == NODE_TYPE (exprslarg))
                   && (ID_AVIS (nid) == ID_AVIS (exprslarg))) {
            strideid = exprsrarg; // nid - something
            mathsign = -1;
        }

        // Find sign of stride, correct for F_sub.
        if (NULL != strideid) {
            stridesignum = mathsign * SCSisPositive (strideid)
                             ? 1
                             : SCSisNegative (strideid) ? -1 : 0;
        }

        // If we know stride sign & loop count, we can generate an ISL directive
        if (0 != stridesignum) {
            prfi = (stridesignum > 0) ? F_le_SxS : F_lt_SxS;
            prfz = (stridesignum > 0) ? F_lt_SxS : F_le_SxS;

            //  rcv = calleriv
            calleriv = rcv2CallerVar (rcv, fundef);
            outerexprs
              = PHUTgenerateAffineExprs (calleriv, FUNDEF_CALLERFUNDEF (fundef), varlut,
                                         AVIS_ISLCLASSEXISTENTIAL, loopcount);
            res = TCappendExprs (res, outerexprs);

            // Build: initial value prfi nid, e.g., 0 <= II
            resel = BuildIslSimpleConstraint (calleriv, prfi, nid, NOPRFOP, NULL);
            res = TCappendExprs (res, resel);

            // Build: rcv prf2 limit.        E.g., II' < limit
            // lastvalue = calleriv + stride*loopcount;
            // If we know the loopcount, and we are NOT dealing
            // with the COND_COND that controls the loop, we specify
            // it in the code. If we are dealing with the COND_COND,
            // we leave the loopcount unknown. If we did specify the
            // loopcount there, POGO would happily replace the
            // COND_COND by TRUE, thereby creating an infinite loop in
            // the generated code.
            lpavis = TBmakeAvis (TRAVtmpVarName ("LOOPCT"),
                                 TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            PHUTinsertVarIntoLut (lpavis, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
            lpcnt = (-1 != loopcount) ? TBmakeNum (loopcount) : lpavis;

            limavis
              = TBmakeAvis (TRAVtmpVarName ("LIM2"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            PHUTinsertVarIntoLut (limavis, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);

            resel = BuildIslStrideConstraint (limavis, F_eq_SxS, calleriv, F_add_SxS,
                                              strideid, F_mul_SxS, lpcnt);
            res = TCappendExprs (res, resel);

            lb = (1 == stridesignum) ? calleriv : TBmakeId (limavis);
            ub = (1 == stridesignum) ? TBmakeId (limavis) : calleriv;

            if ((NULL != lb) && (NULL != ub)) {
                // iv = lb + stp * N
                navis
                  = TBmakeAvis (TRAVtmpVarName ("N"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                PHUTinsertVarIntoLut (navis, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
                resel = BuildIslStrideConstraint (nid, F_eq_SxS, lb, F_add_SxS, strideid,
                                                  F_mul_SxS, navis);
                res = TCappendExprs (res, resel);

                // iv <= nid < lastvalue
                resel = BuildIslSimpleConstraint (lb, prfi, nid, prfz, ub);
                res = TCappendExprs (res, resel);
            }
        }
        PHUTsetIslTree (ID_AVIS (rcvel), res);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn int PHUTgetLoopCount( ...)
 *
 * @brief Try to compute loop count for a LOOPFUN, using polyhedral methods.
 *
 * @param fundef: An N_fundef for a LOOPFUN
 * @param varlut: the LUT for polyhedral analysis, e.g., that set up by POGO.
 *
 * @return The loop count, or UNR_NONE if fundef is not a LOOPFUN,
 *         or if we are unable to determine it.
 *
 * NB. This function makes the strong assumption that it is operating
 *     within the POLYS framework. If not, you will get VERY wrong answers.
 *
 * @algorithm:
 *
 *   Find lcv, loop-carried variable for condprf, the conditional that limits
 *   recursion in the loop. FIXME: I am not sure what to do if there is more
 *   than one of these. For now, I plan to ignore it, and just take the
 *   first candidate.
 *
 *   We have to ensure that lcv is the first entry in the ISL set variable list,
 *   due to the crude way I grab the loop-count result from ISL.
 *
 *
 ******************************************************************************/
int
PHUTgetLoopCount (node *fundef, lut_t *varlut)
{
    node *condvar = NULL;
    node *condprf = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *ex1 = NULL;
    node *ex2 = NULL;
    node *exFn = NULL;
    node *exprs = NULL;
    node *lcv1 = NULL;
    node *lcv2 = NULL;
    char *str = NULL;
    node *resel1 = NULL;
    node *resel2 = NULL;
    node *strideid1 = NULL;
    node *strideid2 = NULL;
    node *strideid = NULL;
    int stridesign1 = 0;
    int stridesign2 = 0;
    int stridesignum = 0;
    int z = UNR_NONE;
    int loopcount = -1;

    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (fundef)) {
        if (UNR_NONE != FUNDEF_LOOPCOUNT (fundef)) {
            z = FUNDEF_LOOPCOUNT (fundef);
        } else {
            condvar = LFUfindLacfunConditional (fundef);
            condprf = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (condvar))));

            if ((N_prf == NODE_TYPE (condprf))
                && (PHUTisCompatibleAffinePrf (PRF_PRF (condprf)))
                && (PHUTisCompatibleAffineTypes (condprf))) {

                arg1 = PHUTskipChainedAssigns (PRF_ARG1 (condprf));
                ex1 = PHUTgenerateAffineExprs (arg1, fundef, varlut,
                                               AVIS_ISLCLASSEXISTENTIAL, loopcount);
                resel1 = PHUThandleAPV (ex1, fundef, varlut, &stridesign1, &lcv1,
                                        &strideid1, loopcount);

                arg2 = PHUTskipChainedAssigns (PRF_ARG2 (condprf));
                ex2 = PHUTgenerateAffineExprs (arg2, fundef, varlut,
                                               AVIS_ISLCLASSEXISTENTIAL, loopcount);
                resel2 = PHUThandleAPV (ex2, fundef, varlut, &stridesign2, &lcv2,
                                        &strideid2, loopcount);

                // Things get messy if we somehow get differing non-zero values
                // for stridesign back from PHUThandleAPV.
                // I am not sure how to handle this, so we will deal with it
                // when/if it arises.
                DBUG_ASSERT ((stridesign1 == stridesign2)
                               || (0 != (stridesign1 + stridesign2)),
                             "Got mixed strides for condprf in LOOPFUN");
                stridesignum = (0 != stridesign1) ? stridesign1 : stridesign2;
                strideid = (0 != stridesign1) ? strideid1 : strideid2;

                exFn = PHUTgenerateAffineExprsForCondprf (PRF_PRF (condprf), arg1, arg2,
                                                          fundef, PRF_PRF (condprf),
                                                          varlut, stridesignum, strideid);
                exprs = TCappendExprs (ex1, ex2);
                exprs = TCappendExprs (exprs, exFn);
                exprs = TCappendExprs (exprs, resel1);
                exprs = TCappendExprs (exprs, resel2);
                str = ISLUexprs2String (exprs, varlut, "LoopCount", TRUE,
                                        FUNDEF_NAME (fundef));
                z = ISLUgetLoopCount (str, varlut);
                DBUG_PRINT ("ISLU computed loop count for %s as %d", FUNDEF_NAME (fundef),
                            z);
                DBUG_ASSERT ((UNR_NONE == z) || (0 < z), "ISL got negative loop count!");
                MEMfree (str);
            }
        }
    }

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
