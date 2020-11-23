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
 * NB. General assumptions made in these functions about the calling
 *     envionment:
 *
 *      1. POLYS sets up AVIS_NPART for WITHID_xxx nodes.
 *         Partitions can change underfoot by (at least) PWLF,
 *         so caution is advised.
 *
 *      2. POLYS sets up FUNDEF_CALLAP and FUNDEF_CALLERFUNDEF.
 *
 *      3. All callers must SKIP chained assigns. Failure to do so will result
 *         in complaints from ISL about "unknown identifier".
 *
 * NB.     Interprocedural affine function construction
 *         (Look at ~/sac/testsuite/optimizations/pogo/condfun.sac to see
 *          why we need to look at the LACFUN caller to build
 *          an affine function tree for a shape vector.
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
#include "compare_tree.h"
#include "ctype.h"

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
 * @return The uppermost N_id in the chain
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
    avis = TUnode2Avis (arg_node);
    if ((NULL != avis) && (N_avis == NODE_TYPE (avis))) {
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
 * @fn bool PHUTisFundefKludge()
 *
 * @brief ISL gets uppity about fundef names that are
 *        not acceptable identifiers (on their planet).
 *        For example, an Array version of != causes trouble with
 *        ipbb.breaks.sac.
 *
 *        Until we come up with a new scheme for mapping
 *        SAC fundef::variable names to/from ISL names, we just
 *        ignore troublesome fundef names.
 *
 *        This function alerts its caller to skip such functions
 *
 * @param arg_node: An N_fundef
 *
 * @return TRUE is fundef is safe for ISL inclusion
 *
 ******************************************************************************/
bool
PHUTisFundefKludge (node *arg_node)
{
    bool z;
    char c;

    DBUG_ENTER ();

    c = FUNDEF_NAME (arg_node)[0];
    z = isalpha (c) || ('_' == c);

    if (!z) {
        DBUG_PRINT ("Function %s is not suitable for ISL because of its odd name",
                    FUNDEF_NAME (arg_node));
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
    size_t n;
    size_t j;

    DBUG_ENTER ();
    n = TCcountExprs (arg_node);
    for (j = 0; j < n; j++) {
        PRTdoPrint (TCtakeDropExprs (1, j, arg_node));
    }

    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *Node2Value( node *arg_node)
 *
 * @brief Find N_avis node for arg_node, unless it is
 *        an N_num/N_bool, in which case we create an N_num
 *
 * @param An N_avis, N_id, N_num, N_bool, or N_ids node
 *
 * @return An N_id if the argument is an N_id, N_avis, or N_ids.
 *         Otherwise, the associated N_avis node,
 *         except that we return N_num for AKV values
 *
 ******************************************************************************/
static node *
Node2Value (node *arg_node)
{
    node *z = NULL;
    constant *con = NULL;

    DBUG_ENTER ();

    if (NULL != arg_node) {
        z = TUnode2Avis (arg_node);
        if (NULL != z) {
            if (TYisAKV (AVIS_TYPE (z))) {
                if (TUisIntScalar (AVIS_TYPE (z))) {
                    z = TBmakeNum (TUakvScalInt2Int (AVIS_TYPE (z)));
                } else {
                    if (TUisBoolScalar (AVIS_TYPE (z))) {
                        con = TYgetValue (AVIS_TYPE (z)); // con is NOT a copy!
                        z = TBmakeNum (COconst2Int (con));
                    } else {
                        DBUG_ASSERT (FALSE, "Expected N_num or N_bool");
                    }
                }
            } else {
                DBUG_ASSERT (N_avis == NODE_TYPE (z), "Expected N_avis from TUnode2Avis");
                z = TBmakeId (z);
            }
        } else {
            switch (NODE_TYPE (arg_node)) {
            case N_num:
                z = DUPdoDupNode (arg_node);
                break;

            case N_bool:
                z = TBmakeNum ((int)BOOL_VAL (arg_node)); //FIXME there may be a better solution, but this temporarily fixes the warning
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

    avis = TUnode2Avis (arg_node);
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

    avis = TUnode2Avis (arg_node);
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
    case F_div_SxS:
        z = "/";
        break;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn void PHUTclearAvisIslAttributesOne( lut_t *varlut)
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
        DBUG_PRINT ("Clearing AVIS_ISLCLASS, AVIS_ISLTREE in variable %s",
                    AVIS_NAME (avis2));
        AVIS_ISLCLASS (avis2) = AVIS_ISLCLASSUNDEFINED;
        AVIS_ISLTREE (avis2)
          = (NULL != AVIS_ISLTREE (avis2)) ? FREEdoFreeTree (AVIS_ISLTREE (avis2)) : NULL;
        AVIS_STRIDESIGNUM (avis2) = 0;
    }

    DBUG_RETURN (z);
}

void
PHUTpolyEpilogOne (lut_t *varlut)
{
    node *z;
    node *head = NULL;

    DBUG_ENTER ();

    z = (node *)LUTfoldLutP (varlut, head, ClearAvisIslAttributesOne);

    DBUG_PRINT ("Removing content from VARLUT");
    LUTremoveContentLut (varlut);

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
 *         Any non-scalar values are ignored
 *
 ******************************************************************************/
static void *
GetIslSetVariablesFromLutOne (void *rest, void *fundef, void *avis)
{
    node *z = NULL;

    DBUG_ENTER ();

    if (NULL != avis) {
        DBUG_PRINT ("Found variable %s in VARLUT", AVIS_NAME (avis));
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
 * @fn void printIslName( FILE *handle, node *avis, lut_t *varlut)
 *
 * @brief print (to file) an ISL-acceptable, unique version of avisname.
 *        For fundefname of "foo", and avisname of nid", we generate:
 *          _3_foo_nid, where 3 is the length of "foo".
 *          The _s are to keep ISL happy, and to perhaps make
 *          the string more human-readable
 *
 * @param handle: a file handle for the output
 * @param avis: the SAC N_avis for the variable
 * @param varlut: The LUT containing the avis list and their fundefs
 *
 * @return void
 *         Side effect: print the generated ISL name.
 *
 ******************************************************************************/
static void
printIslName (FILE *handle, node *avis, lut_t *varlut)
{
    node *fn;
    size_t nmlen;

    DBUG_ENTER ();

    fn = (node *)LUTsearchInLutPp (varlut, avis);
    DBUG_ASSERT (avis != fn, "Did not find %s", AVIS_NAME (avis));
    nmlen = strlen (FUNDEF_NAME (fn));
    fprintf (handle, "_%zu_%s_%s", nmlen, FUNDEF_NAME (fn), AVIS_NAME (avis));

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
 *              lb <=     arg1 <     ub
 *
 * @param arg1, arg2: N_id, N_avis or N_num nodes
 *        nprf1, nprf2: the function(s) to be used, or NULL
 *        ids must be N_id or N_avis
 *
 * @return The result is a one-element N_exprs comprising an N_exprs chain
 *         of the constraint, which will eventually
 *         be assembled, then turned into text and written to file for passing to ISL.
 *         The two-deep nesting is so that we can introduce "and" conjunctions
 *         among the contraints when constructing the ISL input.
 *
 *         If arg1, or arg2 is AKV, we replace it by its value.
 *
 ******************************************************************************/
static node *
BuildIslSimpleConstraint (node *ids, prf nprf1, node *arg1, prf nprf2, node *arg2)
{
    node *z;
    node *idsv;
    node *idsavis;
    node *arg1v;
    node *arg2v;

    DBUG_ENTER ();

    idsv = Node2Value (ids);
    DBUG_ASSERT (NULL != idsv, "Expected non-NULL ids");
    arg1v = Node2Value (arg1);
    arg2v = Node2Value (arg2);
    idsavis = TUnode2Avis (ids);
    DBUG_PRINT ("Generating constraint for %s", AVIS_NAME (idsavis));

    z = TBmakeExprs (TBmakeId (idsavis), NULL);
    z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf1, NULL), NULL));
    z = TCappendExprs (z, TBmakeExprs (arg1v, NULL));

    if (NOPRFOP != nprf2) {
        z = TCappendExprs (z, TBmakeExprs (TBmakePrf (nprf2, NULL), NULL));
        if (NULL != arg2v) { // kludge for disjunction F_or_SxS
            z = TCappendExprs (z, TBmakeExprs (arg2v, NULL));
        }
    }

    z = TBmakeExprs (z, NULL);
    DBUG_EXECUTE (PHUTprintIslAffineFunctionTree (z));
    DBUG_PRINT ("Finished generating constraint for %s", AVIS_NAME (idsavis));

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

    idsv = Node2Value (ids);
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
 * @fn node *findBoundEl( node *arg_node, node *bnd, size_t k...)
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
findBoundEl (node *arg_node, node *bnd, size_t k)
{
    node *z = NULL;

    DBUG_ENTER ();

    if ((NULL != AVIS_NPART (arg_node)) && (NULL != bnd)) {
        z = TCgetNthExprsExpr (k, ARRAY_AELEMS (bnd));
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUTcollectWlGenerator( node *arg_node...)
 *
 * @brief arg_node is a WITHID element for a WL, shown here as iv.
 *        The slightly messy part here is non-unit stride
 *        and/or width, which entail
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
    size_t k;
    bool isIdsMember;
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
        k = LFUindexOfMemberIds (arg_node, WITHID_IDS (PART_WITHID (partn)), &isIdsMember);
        if (isIdsMember) {
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
                                      AVIS_ISLCLASSEXISTENTIAL)) {
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

            // Generate: ivw >= 0   and   ivw < wid
            ivwavis
              = TBmakeAvis (TRAVtmpVarName ("IVW"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            if (PHUTinsertVarIntoLut (ivwavis, varlut, fundef,
                                      AVIS_ISLCLASSEXISTENTIAL)) {
                z = DUPdoDupTree (AVIS_ISLTREE (ivwavis));
                res = TCappendExprs (res, z);
                z = BuildIslSimpleConstraint (ivwavis, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                z = BuildIslSimpleConstraint (ivwavis, F_lt_SxS, widel, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
            }

            // Generate iv' = lb + stp * N
            //          N >= 0
            navis = TBmakeAvis (TRAVtmpVarName ("N"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            if (PHUTinsertVarIntoLut (navis, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL)) {
                z = DUPdoDupTree (AVIS_ISLTREE (navis));
                res = TCappendExprs (res, z);
                z = BuildIslStrideConstraint (ivpavis, F_eq_SxS, DUPdoDupNode (lbel),
                                              F_add_SxS, stpel, F_mul_SxS, navis);
                res = TCappendExprs (res, z);
                // Generate N >= 0
                z = BuildIslSimpleConstraint (navis, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
            }

            // Generate iv = iv' + ivw
            z = BuildIslSimpleConstraint (ivavis, F_eq_SxS, ivpavis, F_add_SxS, ivwavis);
            res = TCappendExprs (res, z);

            // Generate iv >= lb  and  iv <  ub
            z = BuildIslSimpleConstraint (ivavis, F_ge_SxS, DUPdoDupNode (lbel), NOPRFOP,
                                          NULL);
            res = TCappendExprs (res, z);
            z = BuildIslSimpleConstraint (ivavis, F_lt_SxS, DUPdoDupNode (ubel), NOPRFOP,
                                          NULL);
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
    size_t n;
    size_t i;
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
WriteSetVariables (FILE *handle, node *idlist, lut_t *varlut)
{
    node *avis;
    size_t i;
    size_t n;
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
            printIslName (handle, avis, varlut);
            fprintf (handle, "\n");
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
WriteExistsSetVariables (FILE *handle, node *idlist, lut_t *varlut)
{
    node *avis;
    char *funname;
    size_t i;
    size_t n;
    bool z;

    DBUG_ENTER ();
    n = TCcountExprs (idlist);
    z = 0 != n;

    for (i = 0; i < n; i = i + 2) {
        avis = ID_AVIS (TCgetNthExprsExpr (i + 1, idlist));
        if (AVIS_ISLCLASSEXISTENTIAL == AVIS_ISLCLASS (avis)) {
            funname = STR_STRING (TCgetNthExprsExpr (i, idlist));
            fprintf (handle, " exists ");
            printIslName (handle, avis, varlut);
            fprintf (handle, " :\n");
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
WriteParameterVariables (FILE *handle, node *idlist, lut_t *varlut)
{
    node *avis;
    size_t i;
    size_t n;
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
            printIslName (handle, avis, varlut);
            fprintf (handle, "\n");
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
printIslArg (FILE *handle, node *expr, lut_t *varlut)
{

    DBUG_ENTER ();

    switch (NODE_TYPE (expr)) {
    case N_id:
        printIslName (handle, ID_AVIS (expr), varlut);
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

static void
EmitOneConstraint (FILE *handle, size_t mone, node *exprsone, lut_t *varlut)
{ // Bog-standard constraints
    node *expr;
    size_t k;
    bool wasor;

    DBUG_ENTER ();

    for (k = 0; k < mone; k++) { // Emit one constraint
        expr = TCgetNthExprsExpr (k, exprsone);
        switch (NODE_TYPE (expr)) {
        default:
            DBUG_ASSERT (FALSE, "Unexpected constraint node type");
            break;

        case N_id:
        case N_num:
        case N_bool:
            printIslArg (handle, expr, varlut);
            break;

        case N_prf:
            switch (PRF_PRF (expr)) {
            default:
                fprintf (handle, "%s", Prf2Isl (PRF_PRF (expr)));
                break;
            case F_or_SxS:
                fprintf (handle, "\n  or \n ");
                wasor = TRUE;
                break;
            case F_min_SxS:
            case F_max_SxS:
            case F_neq_SxS:
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

    DBUG_RETURN ();
}

void
PHUTwriteUnionSet (FILE *handle, node *exprs, lut_t *varlut, char *tag, bool isunionset,
                   char *lhsname)
{
    size_t j;
    size_t m;
    size_t mone;
    size_t n;
    node *idlist;
    node *exprsone;
    node *avis;
    char *txt;
    bool wasor;

    DBUG_ENTER ();

    idlist = GetIslSetVariablesFromLut (varlut);
    n = TCcountExprs (idlist);
    if (0 != n) {

        fprintf (handle, "\n# %s: %s\n\n", tag, lhsname); // Write vars
        WriteParameterVariables (handle, idlist, varlut);
        fprintf (handle, " \n { \n");
        WriteSetVariables (handle, idlist, varlut);
        if (!isunionset) { // Write union map
            fprintf (handle, " ->");
            WriteSetVariables (handle, idlist, varlut);
        }
        fprintf (handle, " :\n");
        // Write existentially qualified vars
        WriteExistsSetVariables (handle, idlist, varlut);
        fprintf (handle, "\n");

        // Append constraints
        m = TCcountExprs (exprs);
        for (j = 0; j < m; j++) {
            wasor = FALSE;
            exprsone = TCgetNthExprsExpr (j, exprs);
            DBUG_ASSERT (N_exprs == NODE_TYPE (exprsone), "Wrong constraint type");
            mone = TCcountExprs (exprsone);

            if (5 == TCcountExprs (exprsone)) {
                switch (ISLFN (exprsone)) {

                case F_min_SxS: // Non-infix fns, e.g., min( x,y)
                case F_max_SxS:
                    avis = ID_AVIS (ISLVAR (exprsone));
                    printIslName (handle, avis, varlut);
                    // FIXME: It would be nice to fix up the hard-coded offsets below.
                    fprintf (handle, "%s",
                             Prf2Isl (PRF_PRF (TCgetNthExprsExpr (1, exprsone))));
                    fprintf (handle, "%s(",
                             Prf2Isl (PRF_PRF (TCgetNthExprsExpr (3, exprsone))));
                    printIslArg (handle, TCgetNthExprsExpr (2, exprsone), varlut);
                    fprintf (handle, ",");
                    printIslArg (handle, TCgetNthExprsExpr (4, exprsone), varlut);
                    fprintf (handle, ")");
                    break;

                default:
                    EmitOneConstraint (handle, mone, exprsone, varlut);
                    break;
                }
            } else {
                if (F_neq_SxS == PRF_PRF (ISLEQ (exprsone))) {
                    // Kludge. Sorry.
                    fprintf (handle, "not( ");
                    printIslArg (handle, ISLVAR (exprsone), varlut); // LARG
                    fprintf (handle, " = ");
                    printIslArg (handle, ISLLARG (exprsone), varlut); // RARG
                    fprintf (handle, ")");
                } else {
                    EmitOneConstraint (handle, mone, exprsone, varlut);
                }
            }

            if (j < (m - 1)) { // Handle conjunctions of constraints
                txt = wasor ? "" : "\n and\n ";
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
    case F_div_SxS:
    case F_noteminval:
    case F_notemaxval:
    case F_mask_SxSxS:
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
    case F_sel_VxA:
    case F_idx_sel:
    case F_div_SxS:
    case F_mask_SxSxS: // Triadic, but close enough
        z = TRUE;
        break;

    case F_abs_S: // Monadic
    case F_neg_S:
    case F_non_neg_val_S:
    case F_shape_A:
    case F_not_S:
    case F_dim_A:
    case F_noteminval: // dyadic, but we ignore PRF_ARG2
    case F_notemaxval: // dyadic, but we ignore PRF_ARG2
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
 *        dim() shape(), etc., produce integer results, so we accept them,
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

    // All argument types good for these primitives
    z = (F_dim_A == PRF_PRF (arg_node)) || (F_sel_VxA == PRF_PRF (arg_node))
        || (F_idx_sel == PRF_PRF (arg_node)) || (F_shape_A == PRF_PRF (arg_node));

    if (!z) { // These primitives require Boolean or Int arguments
        avis = TUnode2Avis (PRF_ARG1 (arg_node));
        z = z || TUisBoolScalar (AVIS_TYPE (avis)) || TUisIntScalar (AVIS_TYPE (avis));
        if (isDyadicPrf (PRF_PRF (arg_node))) {
            avis = TUnode2Avis (PRF_ARG2 (arg_node));
            z = z
                && (TUisBoolScalar (AVIS_TYPE (avis))
                    || TUisIntScalar (AVIS_TYPE (avis)));
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleNid( )
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

    DBUG_ENTER ();

    DBUG_PRINT ("Entering for lhs=%s", AVIS_NAME (arg_node));
    res = BuildIslSimpleConstraint (arg_node, F_eq_SxS, rhs, NOPRFOP, NULL);
    DBUG_PRINT ("Leaving for lhs=%s", AVIS_NAME (arg_node));

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
    node *avis;
    node *z = NULL;
    int li;

    DBUG_ENTER ();

    avis = TUnode2Avis (nid);
    DBUG_PRINT ("Looking at variable %s in LOOPFUN %s", AVIS_NAME (avis),
                FUNDEF_NAME (fundef));

    // If the variable is loop-invariant, we are done, except for
    // grabbing outside initial value.
    // See unit test:
    // ~/sac/testsuite/optimizations/pogorelationals/SCSprf_lt_SxS.LIR.sac

    li = LFUisLoopInvariantArg (avis, fundef);
    if (0 == li) {
        rcv = LFUarg2Rcv (avis, fundef);
        DBUG_PRINT ("LACFUN %s loop-dependent arg %s has recursive call value of %s",
                    FUNDEF_NAME (fundef), AVIS_NAME (avis), AVIS_NAME (ID_AVIS (rcv)));
        z = PHUTanalyzeLoopDependentVariable (nid, rcv, fundef, varlut, loopcount, res);
        res = TCappendExprs (res, z);
    }

    if (1 == li) {
        DBUG_PRINT ("LACFUN %s arg %s is loop-independent", FUNDEF_NAME (fundef),
                    AVIS_NAME (avis));
        // Propagate caller's Loop-independent value into LOOPFUN.
        res = BuildIslSimpleConstraint (nid, F_eq_SxS, calleriv, NOPRFOP, NULL);
    }

    if (-1 == li) {
        DBUG_PRINT ("LACFUN %s arg %s may or may not be loop-independent",
                    FUNDEF_NAME (fundef), AVIS_NAME (avis));
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
    argvar = LFUrcv2Arg (rcv, fundef);

    // II to caller's II
    res = LFUarg2Caller (argvar, fundef);

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
    node *calleravis;
    node *outerexprs;
    node *callerfundef;
    node *avis;

    DBUG_ENTER ();

    avis = TUnode2Avis (nid);
    // N_assign of External call (N_ap) to LACFUN
    ext_assign = FUNDEF_CALLAP (fundef);
    DBUG_ASSERT ((NULL != ext_assign) == FUNDEF_ISLACFUN (fundef),
                 "Only lacfun must have ext_assign");
    if (NULL != ext_assign) { // Current function may not be a LACFUN
        DBUG_ASSERT (N_assign == NODE_TYPE (ext_assign),
                     "Expected FUNDEF_CALLAP to be N_assign");
        DBUG_ASSERT (AP_FUNDEF (LET_EXPR (ASSIGN_STMT (ext_assign))) == fundef,
                     "Expected FUNDEF_CALLAP to point to its N_ap node");

        // Find caller's value for avis.
        calleravis = rcv2CallerVar (avis, fundef);
        calleravis = TUnode2Avis (PHUTskipChainedAssigns (calleravis));
        callerfundef = FUNDEF_CALLERFUNDEF (fundef);
        outerexprs = PHUTcollectAffineExprsLocal (calleravis, callerfundef, varlut, NULL,
                                                  AVIS_ISLCLASSEXISTENTIAL, loopcount);

        DBUG_PRINT ("Building AFT for LACFUN var %s from caller's %s", AVIS_NAME (avis),
                    AVIS_NAME (calleravis));

        if (FUNDEF_ISCONDFUN (fundef)) {
            res = PHUThandleCondfunArg (nid, fundef, varlut, res, ext_assign,
                                        callerfundef, outerexprs, calleravis);
            res = TCappendExprs (res, outerexprs);
        }

        if (FUNDEF_ISLOOPFUN (fundef)) {
            res = PHUThandleLoopfunArg (nid, fundef, varlut, res, ext_assign,
                                        callerfundef, outerexprs, calleravis, loopcount);
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
        rhs = TBmakeNum (TUakvScalInt2Int (AVIS_TYPE (arg_node)));
    }
    z = BuildIslSimpleConstraint (arg_node, F_eq_SxS, rhs, NOPRFOP, NULL);
    res = TCappendExprs (res, z);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleSelectWithShape( node *ids, node *arg_node...)
 *
 * @brief Handler for compositions of indexing on shape:
 *        Currently supported:
 *          shpel = idx_sel( offset, _shape_A_( var));
 *          shpel = _sel_VxA_( iv, _shape_A_( var));
 *          Both of these will generate shpel >= 0.
 *
 * @param  ids: the N_ids node for the result of the sel()
 *         arg_node: PRF_ARG2 for sel()
 *
 * @return An N_exprs chain, representing ISL constraints
 *         on arg_node and ids. Else, NULL.
 *
 ******************************************************************************/
static node *
HandleSelectWithShape (node *ids, node *arg_node)
{
    node *z = NULL;
    pattern *pat;
    node *m = NULL;

    DBUG_ENTER ();

    // x = _shape_A_( m );
    pat = PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&m), 0));

    if (PMmatchFlat (pat, arg_node)) {
        z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP, NULL);
    }
    pat = PMfree (pat);

    DBUG_PRINT ("Leaving HandleSelectWithShape for %s", AVIS_NAME (ID_AVIS (arg_node)));

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn int PHUTsignum(...)
 *
 * @brief Signum for arg:
 *         arg < 0  --> -1
 *         arg = 0  -->  0
 *         arg > 0  -->  1
 *         unknown  -->  2
 *
 * @param arg: an N_id or N_avis
 * @param aft: the Affine Function Tree associated with arg
 * @param fundef: The N_fundef we are in
 * @param varlut: The LUT for PHUT and friends
 * @param ids: The LHS for this primitive
 *
 * @result signum arg, if we can find it, else 2.
 *
 ******************************************************************************/
int
PHUTsignum (node *arg, node *aft, node *fundef, lut_t *varlut, node *ids)
{
    int res = 2;

    DBUG_ENTER ();

    // If arg2 > 0 --->   z = 1
    if (PHUTisPositive (arg, aft, fundef, varlut)) {
        res = 1;
    } else { // If arg2 < 0 --->   z = -1
        if (PHUTisNegative (arg, aft, fundef, varlut)) {
            res = -1;
        } else {
            if (SCSisConstantZero (arg)) {
                res = 0;
            }
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleSelectWithIota(...)
 *
 * @brief Handler for sel() from iota(N)
 *
 *          Q = iota(n);
 *          ids = idx_sel( offset, Q);  OR
 *          ids = _sel_VxA_( iv, Q);
 *
 * @param  ids:  The N_ids node that defines ids.
 * @param  q: The N_id node that is PRF_ARG2 of the sel()
 * @param  loopcount: The loopcount to be used for the APV in a loopfun,
 *                    else UNR_NONE.
 *
 * @return: If q is a with-loop, and that
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
HandleSelectWithIota (node *ids, node *q, node *fundef, lut_t *varlut, int loopcount)
{
    node *res = NULL;
    node *z = NULL;
    node *s = NULL;
    node *wl = NULL;
    node *lb = NULL;
    node *ub = NULL;
    node *lbub = NULL;
    node *withidids = NULL;
    size_t idsidx;
    bool isIdsMember;

    DBUG_ENTER ();

    s = WLUTgetGenarrayScalar (q, FALSE);
    if (NULL != s) { // s is a member of N_WITHIDS.
        // Find the WL, if it exists
        wl = WLUTid2With (q);
        if (N_with == NODE_TYPE (wl)) {
            // Find generator elements for LB and UB.
            withidids = WITHID_IDS (PART_WITHID (WITH_PART (wl)));
            idsidx = TClookupIdsNode (withidids, s, &isIdsMember);
            DBUG_ASSERT (isIdsMember, "Could not find withidids element");
            lbub = GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (wl)));
            if (N_array == NODE_TYPE (lbub)) {
                // Emit lb <= s
                lb = TCgetNthExprsExpr (idsidx, ARRAY_AELEMS (lbub));
                z = PHUTcollectAffineExprsLocal (lb, fundef, varlut, NULL,
                                                 AVIS_ISLCLASSEXISTENTIAL, loopcount);
                res = TCappendExprs (res, z);
                z = PHUTcollectAffineExprsLocal (s, fundef, varlut, NULL,
                                                 AVIS_ISLCLASSSETVARIABLE, loopcount);
                res = TCappendExprs (res, z);
                // Emit s >= lb
                z = BuildIslSimpleConstraint (s, F_ge_SxS, lb, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
            }

            lbub = GENERATOR_BOUND2 (PART_GENERATOR (WITH_PART (wl)));
            if (N_array == NODE_TYPE (lbub)) {
                // Emit s < ub
                ub = TCgetNthExprsExpr (idsidx, ARRAY_AELEMS (lbub));
                z = PHUTcollectAffineExprsLocal (ub, fundef, varlut, NULL,
                                                 AVIS_ISLCLASSEXISTENTIAL, loopcount);
                res = TCappendExprs (res, z);
                // Emit s < ub
                z = BuildIslSimpleConstraint (s, F_lt_SxS, ub, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
            }
            // Emit ids = s (in iv=[i,j,s,k] in the WL generator)
            z = BuildIslSimpleConstraint (ids, F_eq_SxS, s, NOPRFOP, NULL);
            res = TCappendExprs (res, z);
        }
    }

    DBUG_PRINT ("Leaving for lhs=%s", AVIS_NAME (IDS_AVIS (ids)));

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUTprf( node *arg_node...
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
 *
 * @return An N_exprs chain, representing a polylib Matrix row, appended to res.
 *         All the N_exprs chain values will be N_num nodes.
 *
 ******************************************************************************/
static node *
PHUTprf (node *arg_node, node *rhs, node *fundef, lut_t *varlut, node *res, int loopcount)
{
    node *assgn;
    node *ids = NULL;
    node *argavis;
    node *z = NULL;
    node *z2 = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *arg3 = NULL;
    node *arg1aft = NULL;
    node *arg2aft = NULL;
    node *arg3aft = NULL;
    int sig;

    DBUG_ENTER ();

    assgn = AVIS_SSAASSIGN (arg_node);
    if (NULL != assgn) {
        ids = LET_IDS (ASSIGN_STMT (assgn));
        if ((PHUTisCompatibleAffinePrf (PRF_PRF (rhs)))
            && (PHUTisCompatibleAffineTypes (rhs))) {
            DBUG_PRINT ("Called with ids=%s", AVIS_NAME (IDS_AVIS (ids)));

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

            case F_div_SxS:
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, PRF_PRF (rhs), arg2);
                res = TCappendExprs (res, z);
                break;

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
                z = BuildIslSimpleConstraint (arg1, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                z = BuildIslSimpleConstraint (arg1, F_le_SxS, TBmakeNum (1), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                break;

            case F_abs_S: // z >= 0
                z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                              NULL);
                res = TCappendExprs (res, z);
                z = NULL;
                sig = PHUTsignum (arg1, arg1aft, fundef, varlut, ids);
                if (1 == sig) { // z = arg
                    z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, NOPRFOP, NULL);
                } else {
                    if (-1 == sig) { // z = 0 - arg
                        z = BuildIslSimpleConstraint (ids, F_eq_SxS, TBmakeNum (0),
                                                      F_sub_SxS, arg1);
                    }
                }
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
                sig = PHUTsignum (arg2, arg2aft, fundef, varlut, ids);

                z = NULL;
                if (1 == sig) { // z < arg2
                    z = BuildIslSimpleConstraint (ids, F_le_SxS, arg2, NOPRFOP, NULL);
                } else {
                    if (-1 == sig) { // z < 0 - arg2
                        z = BuildIslSimpleConstraint (ids, F_lt_SxS, TBmakeNum (0),
                                                      F_sub_SxS, arg2);
                    }
                }
                res = TCappendExprs (res, z);
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
                if ((PHUTisNonNegative (arg1, arg1aft, fundef, varlut))
                    && (PHUTisPositive (arg2, arg2aft, fundef, varlut))) {
                    // z >= 0
                    z = BuildIslSimpleConstraint (ids, F_ge_SxS, TBmakeNum (0), NOPRFOP,
                                                  NULL);
                    res = TCappendExprs (res, z);
                    // z < PRF_ARG2( rhs)
                    z = BuildIslSimpleConstraint (ids, F_lt_SxS, arg2, NOPRFOP, NULL);
                    res = TCappendExprs (res, z);
                }
                break;

            case F_neq_SxS:
                // We do not know stride
                z = PHUThandleRelational (0, arg1, arg2, PRF_PRF (rhs));
                res = TCappendExprs (res, z);
                break;

            case F_lt_SxS:
            case F_le_SxS:
            case F_eq_SxS:
            case F_ge_SxS:
            case F_gt_SxS:
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
                z = HandleSelectWithIota (ids, arg2, fundef, varlut, loopcount);
                res = TCappendExprs (res, z);

                z2 = HandleSelectWithShape (ids, arg2);
                res = TCappendExprs (res, z2);

                // We may want to clean up IdxselArrayOfEqualElements
                // and call it here, in case we have sel(iv, [ i,i,i,i...,i]

                if ((NULL == z) && (NULL == z2)) { // unknown sel
                    // We know nothing about arg2, so we make it a parameter
                    AVIS_ISLCLASS (IDS_AVIS (ids)) = AVIS_ISLCLASSPARAMETER;
                }
                break;

            case F_noteminval:
            case F_notemaxval:
                // These will be dead soon, but they're not dead yet
                // We treat them the same as simple assign
                // and ignore PRF_ARG2.
                z = BuildIslSimpleConstraint (ids, F_eq_SxS, arg1, NOPRFOP, NULL);
                res = TCappendExprs (res, z);
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

            case F_mask_SxSxS:
                // This is an attempt to deal with unit test
                // pwlf/TakeScalarOvertake.nostdlib.sac, in which
                // we have a take() PRG_ARG1 with unknown signum.
                // I think this is unlikely to arise in realistic
                // applications, just as rotate/shift with counts
                // of unknown signum are unlikely, but the PWLF
                // inference code wants to do a fold, which is utterly wrong.
                // The idea here is to make the result an ISL parameter,
                // which should defeat the evil spirits of PWLF inference.
                //
                // This seems to work as desired. Background:
                // If we encounter an N_prf for which we are unable to
                // collect any useful information, we make its result
                // an ISL parameter, rather than an existential variable.

                AVIS_ISLCLASS (IDS_AVIS (ids)) = AVIS_ISLCLASSPARAMETER;
                break;

            default:
                DBUG_UNREACHABLE ("Nprf coding time, senor");
                break;
            }

            DBUG_PRINT ("Finished with ids=%s", AVIS_NAME (IDS_AVIS (ids)));
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
        if (NULL != AVIS_ISLTREE (avis)) {
            DBUG_ASSERT (CMPT_EQ == CMPTdoCompareTree (AVIS_ISLTREE (avis), aft),
                         "Different ISLTREE values");
        } else {
            AVIS_ISLTREE (avis) = DUPdoDupTree (aft);
        }
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
    bool isIdsMember;

    DBUG_ENTER ();

    nid = PHUTskipChainedAssigns (arg_node);
    avis = TUnode2Avis (nid);
    if ((NULL != avis)) { // N_num exits here
        DBUG_PRINT ("Looking at %s", AVIS_NAME (avis));
        assgn = AVIS_SSAASSIGN (avis);
        cls = (AVIS_ISLCLASSUNDEFINED != islclass) ? islclass : AVIS_ISLCLASSEXISTENTIAL;
        if (NULL != AVIS_ISLTREE (avis)) {
            res2 = DUPdoDupTree (AVIS_ISLTREE (avis));
        } else {
            if (PHUTinsertVarIntoLut (avis, varlut, fundef, cls)) {
                // Handle RHS for all modes.
                if ((NULL != assgn) && (N_let == NODE_TYPE (ASSIGN_STMT (assgn)))) {
                    rhs = LET_EXPR (ASSIGN_STMT (assgn));

                    switch (NODE_TYPE (rhs)) {
                    case N_id: // straight assign: var2 = var1.
                        res2 = HandleNid (avis, rhs, fundef, varlut, loopcount);
                        break;
                    case N_prf:
                        res2 = PHUTprf (avis, rhs, fundef, varlut, NULL, loopcount);
                        break;
                    case N_bool:
                    case N_num:
                        res2 = HandleNumber (avis, rhs, fundef, varlut, NULL);
                        break;

                    case N_ap: // The buck stops here - we can't look any further.
                        AVIS_ISLCLASS (avis) = AVIS_ISLCLASSPARAMETER;
                        break;
                    default:
                        break;
                    }
                } else {
                    DBUG_ASSERT (NULL == assgn, "Confusion about AVIS_SSAASSIGN");
                    // This may be a WITHID_IDS member, a function parameter,
                    // or a WITHID_VEC. If the latter, we ignore it.
                    // If it is constant, then we treat it as a number.
                    if (TYisAKV (AVIS_TYPE (avis))) {
                        res3 = HandleNumber (avis, rhs, fundef, varlut, NULL);
                        res2 = TCappendExprs (res2, res3);
                    } else {
                        // This presumes that AVIS_NPART is maintained properly
                        npart = AVIS_NPART (avis);
                        if ((NULL != npart)
                            && (avis != IDS_AVIS (WITHID_VEC (PART_WITHID (npart))))) {
                            LFUindexOfMemberIds (avis, WITHID_IDS (PART_WITHID (npart)), &isIdsMember);
                            if (isIdsMember) {
                                // arg_node is a withid element.
                                res3 = PHUTcollectWlGenerator (avis, fundef, varlut, NULL,
                                                               loopcount);
                                res2 = TCappendExprs (res2, res3);
                            } else {
                                // non-constant function parameter
                                DBUG_ASSERT (FALSE, "Coding time for lacfun args");
                            }
                        } else { // Not WITHID. Must be function parameter
                            res3 = PHUThandleLacfunArg (nid, fundef, varlut, NULL,
                                                        loopcount);
                            res2 = TCappendExprs (res2, res3);
                        }
                    }
                }
            }
        }
        PHUTsetIslTree (avis, res2);
        res = TCappendExprs (res, res2);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprs(...)
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
 * @fn node *PHUThandleRelational( stridesignum, arg1, arg2, relprf)
 *
 * @brief F_neq_SxS is not supported directly by ISL
 *
 *        If we know stridesignum for arg1 or arg2, we can
 *        replace the != by < or >.
 *
 * @param arg1: PRF_ARG1 for a guard/relational primitive
 *              or N_avis for same
 * @param arg2: PRF_ARG2 for a guard/relational primitive
 *              or N_avis for same
 * @param fundef: The N_fundef for the current function, for debugging
 * @param relprf:  Either PRF_PRF( arg_node) or its companion function,
 *                e.g., if PRF_PRF is _gt_SxS_, its companion is _le_SxS.
 * @param stridesignum: 1 or -1 for positive or negative stride; 0 if
 *                       stride is unknown.
 *
 * @return A maximal N_exprs chain of expressions for F_neq_SxS
 *
 ******************************************************************************/
node *
PHUThandleRelational (int stridesignum, node *arg1, node *arg2, prf relprf)
{
    node *z = NULL;
    node *arg1avis;
    node *arg2avis;
    int s1;
    int s2;

    DBUG_ENTER ();

    arg1avis = TUnode2Avis (arg1);
    arg2avis = TUnode2Avis (arg2);

    if (F_neq_SxS != relprf) {
        z = BuildIslSimpleConstraint (arg1avis, relprf, arg2avis, NOPRFOP, NULL);
    } else {
        s1 = AVIS_STRIDESIGNUM (arg1avis);
        s2 = AVIS_STRIDESIGNUM (arg2avis);
        if ((0 != s1) && (0 != s2)) {
            DBUG_PRINT ("Both condprf( %s,%s) arguments have stride: arg1=%d, arg2=%d",
                        AVIS_NAME (arg1avis), AVIS_NAME (arg2avis),
                        AVIS_STRIDESIGNUM (arg1avis), AVIS_STRIDESIGNUM (arg2avis));
        }

        // If only one argument has known stride, we are in good shape
        if ((0 != s1) && (0 == s2)) {
            relprf = (s1 > 0) ? F_lt_SxS : F_gt_SxS;
        }
        if ((0 != s2) && (0 == s1)) {
            relprf = (s2 > 0) ? F_gt_SxS : F_lt_SxS;
        }

        z = BuildIslSimpleConstraint (arg1avis, relprf, arg2avis, NOPRFOP, NULL);
    }

    DBUG_RETURN (z);
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
 * @param stridesignum: 1 or -1 for positive or negative stride; 0 if
 *                      stride is unknown.
 *
 * @return A maximal N_exprs chain of expressions for the guard.
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForGuard (prf fn, node *arg1, node *arg2, node *fundef, prf relfn,
                                 lut_t *varlut, int stridesignum)
{
    node *z = NULL;

    DBUG_ENTER ();

    arg1 = PHUTskipChainedAssigns (arg1);
    PHUTinsertVarIntoLut (arg1, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
    if (isDyadicPrf (fn)) {
        arg2 = PHUTskipChainedAssigns (arg2);
        PHUTinsertVarIntoLut (arg2, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
    }
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
            z = BuildIslSimpleConstraint (arg1, relfn, arg2, NOPRFOP, NULL);
        }
        break;

    case F_eq_SxS:
        z = BuildIslSimpleConstraint (arg1, relfn, arg2, NOPRFOP, NULL);
        break;

    case F_neq_SxS:
        z = PHUThandleRelational (stridesignum, arg1, arg2, relfn);
        break;

    default:
        DBUG_UNREACHABLE ("Coding time for guard polyhedron");
        break;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn int PHUTcheckIntersection()
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
 * @param: setvaravis - if not NULL, set AVIS_ISLCLASS(setvaravis)
 *         to set variable for this intersect only. Reset ISLCLASS when done.
 *         This is used for internal PHUT functions, such as
 *         PHUTisPositive.
 *
 * @result: a POLY_RET return code
 *
 * NB. PHUTcheckIntersection frees the N_exprs argument nodes
 *
 *****************************************************************************/
int
PHUTcheckIntersection (node *exprspwl, node *exprscwl, node *exprsfn, node *exprscfn,
                       lut_t *varlut, char opcode, char *lhsname, node *setvaravis)
{
    int islcl=0;

    int res = POLY_RET_INVALID;

    DBUG_ENTER ();

    if (NULL != setvaravis) {
        islcl = AVIS_ISLCLASS (setvaravis);
        AVIS_ISLCLASS (setvaravis) = AVIS_ISLCLASSSETVARIABLE;
    }
    res
      = ISLUgetSetIntersections (exprspwl, exprscwl, exprsfn, exprscfn, varlut, lhsname);
    DBUG_PRINT ("ISLU intersection result is %d", res);
    exprspwl = (NULL != exprspwl) ? FREEdoFreeTree (exprspwl) : NULL;
    exprscwl = (NULL != exprscwl) ? FREEdoFreeTree (exprscwl) : NULL;
    exprsfn = (NULL != exprsfn) ? FREEdoFreeTree (exprsfn) : NULL;
    exprscfn = (NULL != exprscfn) ? FREEdoFreeTree (exprscfn) : NULL;
    if (NULL != setvaravis) { // Restore isl class
        AVIS_ISLCLASS (setvaravis) = islcl;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTgenerateAffineExprsForPwlfIntersect( node *pwliv, node *cwliv,
 *           lut_t *varlut)
 *
 * @brief Construct an ISL constraint for the intersect of a consumerWL
 *
 * @param pwliv: The N_avis for the producerWL generator index vector
 * @param cwliv: The N_avis for the consumerWL index vector.
 * @param varlut: The lut we use to collect set variable names.
 * @param fundef: the N_fundef for the function containing cwliv and pwliv
 *
 * @return An N_exprs chain for the intersect, stating that:
 *         cwliv == pwliv
 *
 ******************************************************************************/
node *
PHUTgenerateAffineExprsForPwlfIntersect (node *pwliv, node *cwliv, lut_t *varlut,
                                         node *fundef)
{
    node *res;

    DBUG_ENTER ();

    PHUTinsertVarIntoLut (pwliv, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
    PHUTinsertVarIntoLut (cwliv, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
    res = BuildIslSimpleConstraint (cwliv, F_eq_SxS, pwliv, NOPRFOP, NULL);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTfindLoopDependentVarinAft()
 *
 * @brief Find a loop-dependent variable, arg_node, in aft
 *
 *        E.g., in a -doctz environment, we might have:
 *
 *         int Loop( II)
 *           II'  = II + 1
 *           II'' = II' - lim
 *           ZERO = 0;
 *           if( II'' != ZERO) ...Loop( II')
 *
 *         Here, given II'', we want to return II
 *
 * @param arg_node: An N_id from condprf, e.g., II'' or ZERO
 * @param aft: An Affine Function Tree (AFT), as stored in
 *        AVIS_ISLTREE
 * @param fundef: The Loopfun's N_fundef node
 *
 * @return If we find a loop-dependent variable for arg_node,
 *         return its N_avis node; else NULL.
 *
 ******************************************************************************/
node *
PHUTfindLoopDependentVarinAft (node *arg_node, node *aft, node *fundef)
{
    node *res = NULL;
    node *aftone;
    size_t cnt;

    DBUG_ENTER ();

    while ((NULL == res) && (NULL != aft)) {
        aftone = EXPRS_EXPR (aft); // one-take aft
        cnt = TCcountExprs (aftone);
        switch (cnt) {
        default: // We are confused. Do nothing
            break;

        case 5:
            if (ID_AVIS (ISLVAR (aftone)) == ID_AVIS (arg_node)) {
                // we have arg_node = ...
                res = LFUrcv2Arg (ISLLARG (aftone), fundef);
                res = (NULL != res) ? res : LFUrcv2Arg (ISLRARG (aftone), fundef);
                if (NULL != res) {
                    // This is an attempt to propagate stride information to
                    // condprf in a -doctz-like environment.
                    AVIS_STRIDESIGNUM (ID_AVIS (arg_node)) = AVIS_STRIDESIGNUM (res);
                }
            }
            break;
        }
        aft = EXPRS_NEXT (aft);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *PHUTcollectCondprf()
 *
 * @brief Collect AFT constraints for condprf in loopfun
 *
 * @param fundef: the N_fundef for the loopfun
 * @param varlut: The lut we use to collect set variable names.
 * @param loopcount: used by subfns
 * @param docondprf: TRUE if we are to generate constraints for
 *        condprf itself (NOT to be used with PHUTgetLoopCount,
 *        or we get infinite loops!)
 *
 * @return The AFT for the condprf, or NULL
 *
 * NB. Side effect: set ISLCLASS, if we find a set variable.
 *
 ******************************************************************************/
node *
PHUTcollectCondprf (node *fundef, lut_t *varlut, int loopcount, bool docondprf)
{
    node *res = NULL;
    node *resel;
    node *condprf;
    node *condass;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *arg4rcv = NULL;
    node *setvar = NULL;
    node *lhsavis;
    int stridesignum = 0;

    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (fundef)) {
        condprf = LFUfindLoopfunConditional (fundef);
        condass = ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (condprf)));
        condprf = LET_EXPR (condass);
        docondprf = docondprf && (N_prf == NODE_TYPE (condprf));

        if ((N_prf == NODE_TYPE (condprf))
            && (PHUTisCompatibleAffinePrf (PRF_PRF (condprf)))
            && (PHUTisCompatibleAffineTypes (condprf))) {
            lhsavis = IDS_AVIS (LET_IDS (condass));

            // Find the loop-dependent variable to use as the set variable
            arg1 = PRF_ARG1 (condprf);
            resel = PHUTcollectAffineExprsLocal (arg1, fundef, varlut, NULL,
                                                 AVIS_ISLCLASSEXISTENTIAL, loopcount);
            setvar = PHUTfindLoopDependentVarinAft (arg1, resel, fundef);
            if (NULL != setvar) {
                AVIS_ISLCLASS (setvar) = AVIS_ISLCLASSSETVARIABLE;
                stridesignum = AVIS_STRIDESIGNUM (setvar);
            }
            res = TCappendExprs (res, resel);

            if (isDyadicPrf (PRF_PRF (condprf))) {
                arg2 = PRF_ARG2 (condprf);
                resel = PHUTcollectAffineExprsLocal (arg2, fundef, varlut, NULL,
                                                     AVIS_ISLCLASSEXISTENTIAL, loopcount);
                if (NULL == setvar) {
                    setvar = PHUTfindLoopDependentVarinAft (arg2, resel, fundef);
                    if (NULL != setvar) {
                        AVIS_ISLCLASS (setvar) = AVIS_ISLCLASSSETVARIABLE;
                        stridesignum = AVIS_STRIDESIGNUM (setvar);
                    }
                }
                res = TCappendExprs (res, resel);
            }
        }

        if (docondprf) {
            // If a condprf arg is the rcv, replace it by its arg
            arg4rcv = LFUrcv2Arg (arg1, fundef);
            if (NULL != arg4rcv) {
                arg1 = arg4rcv;
            }

            arg4rcv = LFUrcv2Arg (arg2, fundef);
            if (NULL != arg4rcv) {
                arg2 = arg4rcv;
            }

            resel = PHUThandleRelational (stridesignum, arg1, arg2, PRF_PRF (condprf));
            res = TCappendExprs (res, resel);
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *PHUTanalyzeLoopDependentVariable()
 *
 * @brief Analyze a loop-dependent loopfun argument (not necessarily the
 *        one controlling the loop count)
 *
 * @param  vid - the N_arg variable we are analyzing;
 *               vid is already in varlut.
 * @param: rcv - the recursive call value that corresponds to vid
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
 *         This lets us generate, for positive stride, where
 *         v0 is the initial value, and vz is the loop limit:
 *
 *           vid >= v0
 *           vid < v0 + (stride * loopcount)
 *
 *         Negative stride is the same, except the signs are reversed:
 *
 *           vid <= v0
 *           vid >  v0 + (stride * loopcount)
 *
 *
 ******************************************************************************/
node *
PHUTanalyzeLoopDependentVariable (node *vid, node *rcv, node *fundef, lut_t *varlut,
                                  int loopcount, node *aft)
{
    node *resel = NULL;
    node *rcvel = NULL;
    node *vidavis = NULL;
    node *arg = NULL;
    node *exprs;
    node *res = NULL;
    node *vzavis = NULL;
    node *v0avis = NULL;
    node *strideid = NULL;
    node *lpavis = NULL;
    prf prfiv;
    prf prfz;
    int stridesignum = 0; // -1 for negative, 1 for positive, 0 for unknown or 0.
    int lpcount = UNR_NONE;

    DBUG_ENTER ();

    vidavis = TUnode2Avis (PHUTskipChainedAssigns (vid));
    rcvel = PHUTskipChainedAssigns (rcv);

    if (NULL != AVIS_ISLTREE (ID_AVIS (rcvel))) {
        res = DUPdoDupTree (AVIS_ISLTREE (ID_AVIS (rcvel)));
    } else {
        // Recursive call variable is existential.
        res = PHUTcollectAffineExprsLocal (rcvel, fundef, varlut, NULL,
                                           AVIS_ISLCLASSEXISTENTIAL, loopcount);

        // trace rcv back to calling environment
        strideid = NULL;
        exprs = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (rcvel))));
        arg = LFUrcv2Arg (rcvel, fundef);
        strideid = LFUgetStrideInfo (exprs, arg, &stridesignum, aft, fundef, varlut);

        // If we know stride sign & loop count, we can generate an ISL directive
        if (0 != stridesignum) {
            prfiv = (stridesignum > 0) ? F_ge_SxS : F_le_SxS;
            AVIS_STRIDESIGNUM (ID_AVIS (rcvel)) = stridesignum;
            AVIS_STRIDESIGNUM (vidavis) = stridesignum;

            //  rcv = v0
            v0avis = rcv2CallerVar (rcv, fundef);
            resel
              = PHUTcollectAffineExprsLocal (v0avis, FUNDEF_CALLERFUNDEF (fundef), varlut,
                                             NULL, AVIS_ISLCLASSEXISTENTIAL, loopcount);
            res = TCappendExprs (res, resel);

            // Build: vid >= v0 or vid <= v0, where v0 is initial value
            resel = BuildIslSimpleConstraint (vidavis, prfiv, v0avis, NOPRFOP, NULL);
            res = TCappendExprs (res, resel);

            vzavis = TBmakeAvis (TRAVtmpVarName ("VZ"),
                                 TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            PHUTinsertVarIntoLut (vzavis, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);

            // Constrain loopcount to a non-negative value
            // If constant, also constrain its maximum value
            lpavis = TBmakeAvis (TRAVtmpVarName ("LOOPCT"),
                                 TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
            PHUTinsertVarIntoLut (lpavis, varlut, fundef, AVIS_ISLCLASSEXISTENTIAL);
            // If this is a loopfun with known loopcount, use it.
            lpcount = FUNDEF_LOOPCOUNT (fundef);
            if ((FUNDEF_ISLOOPFUN (fundef)) && (UNR_NONE != lpcount)) {
                resel = BuildIslSimpleConstraint (lpavis, F_lt_SxS, TBmakeNum (lpcount),
                                                  NOPRFOP, NULL);
                res = TCappendExprs (res, resel);
            }
            resel
              = BuildIslSimpleConstraint (lpavis, F_ge_SxS, TBmakeNum (0), NOPRFOP, NULL);
            res = TCappendExprs (res, resel);

            // Generate vz >= 0
            resel
              = BuildIslSimpleConstraint (vzavis, F_ge_SxS, TBmakeNum (0), NOPRFOP, NULL);
            res = TCappendExprs (res, resel);

            // Generate vz = v0 + stride * loopcount
            // where N is existential.
            resel = BuildIslStrideConstraint (vidavis, F_eq_SxS, v0avis, F_add_SxS,
                                              strideid, F_mul_SxS, lpavis);
            res = TCappendExprs (res, resel);

            // Generate vid < (or <=) vz
            prfz = (stridesignum > 0) ? F_lt_SxS : F_le_SxS;
            resel = BuildIslSimpleConstraint (vidavis, prfz, vzavis, NOPRFOP, NULL);
            res = TCappendExprs (res, resel);
        }
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
 *  This code requires that one argument to the LOOPFUN's condprf
 *  is loop-dependent, and that the other is not.
 *  It would be nice to relax this requirement, to allow both
 *  arguments to be loop-dependent, so that we can
 *  maybe handle the Bodo1.sac unit test.
 *
 ******************************************************************************/
int
PHUTgetLoopCount (node *fundef, lut_t *varlut)
{

    node *res = NULL;
    node *resel = NULL;
    char *str = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *condprf;
    int z = UNR_NONE;
    int stridesignum = 0;

    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (fundef)) {
        if (UNR_NONE != FUNDEF_LOOPCOUNT (fundef)) {
            z = FUNDEF_LOOPCOUNT (fundef);
            DBUG_PRINT ("Using FUNDEF_LOOPCOUNT (%s) of %d", FUNDEF_NAME (fundef), z);
        } else {
            res = PHUTcollectCondprf (fundef, varlut, UNR_NONE, FALSE);
            condprf = LFUfindLoopfunConditional (fundef);
            condprf = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (condprf))));

            // N_arg for arg1 or arg2 must be a set variable
            // If we know that one of them is loop-invariant,
            // we blindly pick the other as the set variable.
            // This is needed for -doctz, at least until
            // the invariance analysis improves enough to handle
            // that case.
            //
            // If arg1 and arg2 are both loop-dependent, we do nothing.
            //

            arg1 = PHUTskipChainedAssigns (PRF_ARG1 (condprf));
            if (isDyadicPrf (PRF_PRF (condprf))) {
                arg2 = PHUTskipChainedAssigns (PRF_ARG2 (condprf));
            }

            // Build constraint for condprf
            resel = PHUThandleRelational (stridesignum, arg1, arg2, PRF_PRF (condprf));
            res = TCappendExprs (res, resel);
            str = ISLUexprs2String (res, varlut, "LoopCount", TRUE, FUNDEF_NAME (fundef));
            z = ISLUgetLoopCount (str, varlut);
            DBUG_PRINT ("Loop count for %s is %d", FUNDEF_NAME (fundef), z);
            DBUG_ASSERT ((UNR_NONE == z) || (0 < z), "Got negative loop count!");
            MEMfree (str);
            z = (UNR_NONE != z) ? z + 1 : z; // ISL loop count is one too low
        }
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool PHUTisPositive(...)
 *
 * function: Predicate for determining if an argument is known to
 *           be positive.
 *
 * @param: arg_node: An N_exprs ISL chain
 * @param: aft: An AFT
 * @param: fundef : The N_fundef we are in.
 * @param: varlut: The LUT used by PHUT/POGO.
 *
 * result: True if argument is known to be positive.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is non-positive.
 *
 * NB. This function is used in PHUT for mod() and aplmod(), where
 *     SCSisPositive is too weak to do the job.
 *
 *     It must only be called from code that invokes PHUT. I.e.,
 *     the caller maintains AVIS_NPART and FUNDEF_CALLAP.
 *
 *     In particular, all elements of arg_node are already in the LUT.
 *     This is why we can't just call PHUTgenerateAffineExprs - it would
 *     do nothing.
 *
 *****************************************************************************/
bool
PHUTisPositive (node *arg_node, node *aft, node *fundef, lut_t *varlut)
{
    bool z;
    node *exprsFn;
    node *zro;
    node *arg1;
    int emp = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    z = SCSisPositive (arg_node);
    if (!z) {
        arg1 = PHUTskipChainedAssigns (arg_node);
        zro = TBmakeNum (0);
        // We look for NULL intersect on the dual function: arg1 <= 0
        exprsFn = PHUTgenerateAffineExprsForGuard (F_le_SxS, arg1, zro, fundef, F_le_SxS,
                                                   varlut, 0);
        emp = PHUTcheckIntersection (DUPdoDupTree (aft), NULL, exprsFn, NULL, varlut,
                                     POLY_OPCODE_INTERSECT, "PHUTisPositive",
                                     ID_AVIS (arg1));
        z = 0 != (emp & POLY_RET_EMPTYSET_BCF);
        FREEdoFreeNode (zro);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool PHUTisNegative(...)
 *
 * function: Predicate for determining if an argument is known to
 *           be negative.
 *
 * @param: arg_node: An N_exprs ISL chain
 * @param: aft: An AFT
 * @param: fundef : The N_fundef we are in.
 * @param: varlut: The LUT used by PHUT/POGO.
 *
 * result: True if argument is known to be negative.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is non-negative.
 *
 * NB. This function is used in PHUT for mod() and aplmod(), where
 *     SCSisNegative is too weak to do the job.
 *
 *     It must only be called from code that invokes PHUT. I.e.,
 *     the caller maintains AVIS_NPART and FUNDEF_CALLAP.
 *
 *     In particular, all elements of arg_node are already in the LUT.
 *     This is why we can't just call PHUTgenerateAffineExprs - it would
 *     do nothing.
 *
 *****************************************************************************/
bool
PHUTisNegative (node *arg_node, node *aft, node *fundef, lut_t *varlut)
{
    bool z;
    node *exprsFn;
    node *zro;
    node *arg1;
    int emp = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    z = SCSisNegative (arg_node);
    if (!z) {
        arg1 = PHUTskipChainedAssigns (arg_node);
        zro = TBmakeNum (0);
        // We look for NULL intersect on the dual function: arg1 >= 0
        exprsFn = PHUTgenerateAffineExprsForGuard (F_ge_SxS, arg1, zro, fundef, F_ge_SxS,
                                                   varlut, 0);
        emp = PHUTcheckIntersection (DUPdoDupTree (aft), NULL, exprsFn, NULL, varlut,
                                     POLY_OPCODE_INTERSECT, "PHUTisNegative",
                                     ID_AVIS (arg1));
        z = 0 != (emp & POLY_RET_EMPTYSET_BCF);
        FREEdoFreeNode (zro);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool PHUTisNonPositive(...)
 *
 * function: Predicate for determining if an argument is known to
 *           be non-positive.
 *
 * @param: arg_node: An N_exprs ISL chain
 * @param: aft: An AFT
 * @param: fundef : The N_fundef we are in.
 * @param: varlut: The LUT used by PHUT/POGO.
 *
 * result: True if argument is known to be non-positive.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is positive.
 *
 * NB. This function is used in PHUT for mod() and aplmod(), where
 *     SCSisNonPositive is too weak to do the job.
 *
 *     It must only be called from code that invokes PHUT. I.e.,
 *     the caller maintains AVIS_NPART and FUNDEF_CALLAP.
 *
 *     In particular, all elements of arg_node are already in the LUT.
 *     This is why we can't just call PHUTgenerateAffineExprs - it would
 *     do nothing.
 *
 *****************************************************************************/
bool
PHUTisNonPositive (node *arg_node, node *aft, node *fundef, lut_t *varlut)
{
    bool z;
    node *exprsFn;
    node *zro;
    node *arg1;
    int emp = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    z = SCSisNonPositive (arg_node);
    if (!z) {
        arg1 = PHUTskipChainedAssigns (arg_node);
        zro = TBmakeNum (0);
        // We look for NULL intersect on the dual function  arg1 > 0
        exprsFn = PHUTgenerateAffineExprsForGuard (F_gt_SxS, arg1, zro, fundef, F_gt_SxS,
                                                   varlut, 0);
        emp = PHUTcheckIntersection (DUPdoDupTree (aft), NULL, exprsFn, NULL, varlut,
                                     POLY_OPCODE_INTERSECT, "PHUTisNonPositive",
                                     ID_AVIS (arg1));
        z = 0 != (emp & POLY_RET_EMPTYSET_BCF);
        FREEdoFreeNode (zro);
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 * bool PHUTisNonNegative(...)
 *
 * function: Predicate for determining if an argument is known to
 *           be non-negative.
 *
 * @param: arg_node: An N_exprs ISL chain
 * @param: aft: An AFT
 * @param: fundef : The N_fundef we are in.
 * @param: varlut: The LUT used by PHUT/POGO.
 *
 * result: True if argument is known to be non-negative.
 *         Else false.
 *
 *         NB. False does NOT imply that the argument is negative.
 *
 * NB. This function is used in PHUT for mod() and aplmod(), where
 *     SCSisNonNegative is too weak to do the job.
 *
 *     It must only be called from code that invokes PHUT. I.e.,
 *     the caller maintains AVIS_NPART and FUNDEF_CALLAP.
 *
 *     In particular, all elements of arg_node are already in the LUT.
 *     This is why we can't just call PHUTgenerateAffineExprs - it would
 *     do nothing.
 *
 *****************************************************************************/
bool
PHUTisNonNegative (node *arg_node, node *aft, node *fundef, lut_t *varlut)
{
    bool z;
    node *exprsFn;
    node *zro;
    node *arg1;
    int emp = POLY_RET_UNKNOWN;

    DBUG_ENTER ();

    z = SCSisNonNegative (arg_node);
    if (!z) {
        arg1 = PHUTskipChainedAssigns (arg_node);
        zro = TBmakeNum (0);
        // We look for NULL intersect on the dual function  arg1 < 0
        exprsFn = PHUTgenerateAffineExprsForGuard (F_lt_SxS, arg1, zro, fundef, F_lt_SxS,
                                                   varlut, 0);
        emp = PHUTcheckIntersection (DUPdoDupTree (aft), NULL, exprsFn, NULL, varlut,
                                     POLY_OPCODE_INTERSECT, "PHUTisNonNegative",
                                     ID_AVIS (arg1));
        z = 0 != (emp & POLY_RET_EMPTYSET_BCF);
        FREEdoFreeNode (zro);
    }

    DBUG_RETURN (z);
}

#ifdef DEADCODE // may want to be revived someday
/** <!-- ****************************************************************** -->
 * @fn node *PHUTfreeAvisIslFields( node *avis)
 *
 * @brief Free any ISL-related fields in N_avis avis
 *
 * @brief avis
 *
 ******************************************************************************/
node *
PHUTfreeAvisIslFields (node *avis)
{

    DBUG_ENTER ();

    AVIS_ISLTREE (avis) = (NULL != AVIS_ISLTREE (avis)) ? FREEdoFreeTree (avis) : NULL;
    AVIS_ISLCLASS (avis) = AVIS_ISLCLASSUNDEFINED;

    DBUG_RETURN (avis);
}
#endif // DEADCODE // may want to be revived someday

#undef DBUG_PREFIX
