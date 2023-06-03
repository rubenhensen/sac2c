/** <!--********************************************************************-->
 *
 * @defgroup ivesli Index Vector Elimintation Split Loop Invariants
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  y  |       |
 * can be called on N_fundef               |   -----   |  y  |       |
 * expects LaC funs                        |   -----   |  -  |       |
 * follows N_ap to LaC funs                |   -----   |  n  |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    | yes |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |  n  |       |
 * utilises SAA annotations                |   -----   |  n  |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    |  y  |       |
 * tolerates flattened Generators          |    yes    |  y  |       |
 * tolerates flattened operation parts     |    yes    |  y  |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |  y  |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |  y  |       |
 * =============================================================================
 * </pre>
 *
 * This module implements the splitting of index computations into loop
 * dependent and loop independent parts.
 *
 * @ingroup ivesli
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ive_split_loop_invariants.c
 *
 * Prefix: IVESLI
 *
 * This phase implementent the splitting of vect2offset operations that
 * depend on values from different loop nesting levels into multiple
 * vect2offset operations that depend only on values of outer loop
 * nesting levels, if possible. As an example, consider the simple case
 * where the selection has the form
 *
 *   x = A[iv + 1]
 *
 * Here, iv is the index vector of a withloop and the 1 obviously is
 * independent of that loop. The naive translation to vect2offsets will
 * yield code similar to
 *
 *   flat_1 = 1;
 *   flat_2 = _add_VxV_( iv, 1);
 *   idx = _vect2offset_( shp, flat_2);
 *   x = _idx_sel_( idx, A);
 *
 * This is translated to
 *
 *   flat_1 = 1;
 *   idx_1 = _vect2offset_( shp, flat_1);
 *   idx_2 = _vect2offset_( sho, iv);
 *   idx = _add_SxS_( idx_1, idx_2);
 *   x = _idx_sel_( idx, A);
 *
 * A consecutive application of LIR/WLIR can then move the idx_1 computation
 * out of the loop. Furthermore, IVERAS homefully may be able to use the
 * withloop index for idx_2.
 *
 * The general case is a bit more complicated. To allow for easy extension
 * and gradual implementation of further optimisation steps, this phase
 * uses a rather general framework. Basically, its operation can be divided
 * into three phases:
 *
 * (1) transformation of the second argument of a _vect2offset_ into a
 *     level-sorted data structure
 *
 * (2) simplification of that structure and optimisation of its composition
 *
 * (3) transformation of the data structure back into _vect2offset_
 *     operations
 *
 * The first step uses an internal datastructure for representing the
 * tree of index computations. The structure has the following format:
 *
 * INDEXLEVEL ---> INDEXVECTOR ---> inverse
 *             |                |-> value
 *             |                |-> next ----> INDEXVECTOR...
 *             |
 *             |-> INDEXCHAIN ---> current --->INDEXSCALAR ---> inverse
 *             |               |                            |-> value
 *             |               |                            |-> next ---> IND...
 *             |               |-> next ------> INDEXCHAIN...
 *             |
 *             |-> next ---> INDEXLEVEL...
 *
 * On the top level, multiple levels for sorting indices into are modelled by
 * means of the INDEXLEVEL data-structure; it stores a chain of index vectors,
 * index scalars and further INDEXLEVEL structures.
 * The INDEXVECTOR structure stores a chain of vectors that contribute to the
 * index at the current level. The field inverse is true if the vector needs
 * to be substracted. The value hold a expression node (N_id) that
 * represents the vectors value.
 * The INDEXCHAIN structure stores a chain of chains of indexscalars. The
 * double chaining is required as index scalars do not occur in isolation but
 * are usually identified with a vector of indices. The outer chain is used to
 * store multiple of those vectors.
 * Finally, the INDEXSCALAR data structure stores individual index scalars.
 * Again, the field inverse is true if the scalar needs to be substracted.
 * The field value points to an expression node (N_id or N_num) that
 * represents the value of the scalar. Lastly, the next field points to the
 * next scalar in the chain. Note that the chain is computed lazily, i.e., it
 * may not have the full length of the corresponding index vector. Furthermore,
 * value field may be NULL if no index scalar is present at that position at
 * that level.
 *
 * The initial sorting is based on data-flow masks that model the scope of
 * variables.
 *****************************************************************************/
#include "ive_split_loop_invariants.h"

#ifndef DBUG_OFF
#include <stdio.h>
#endif /* not DBUG_OFF */

#define DBUG_PREFIX "IVESLI"
#include "debug.h"

#include "ctinfo.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "infer_dfms.h"
#include "remove_dfms.h"
#include "DataFlowMask.h"
#include "pattern_match.h"

/** <!--********************************************************************-->
 *
 * @name local data structures
 * @{
 *
 *****************************************************************************/
struct MASKCHAIN {
    dfmask_t *locals;
    struct MASKCHAIN *next;
};
typedef struct MASKCHAIN maskchain_t;

#define MASKCHAIN_LOCALS(n) ((n)->locals)
#define MASKCHAIN_NEXT(n) ((n)->next)

struct INDEXVECTOR {
    bool inverse;
    node *value;
    struct INDEXVECTOR *next;
};
typedef struct INDEXVECTOR indexvector_t;

#define INDEXVECTOR_INVERSE(n) ((n)->inverse)
#define INDEXVECTOR_VALUE(n) ((n)->value)
#define INDEXVECTOR_NEXT(n) ((n)->next)

struct INDEXSCALAR {
    bool inverse;
    node *value;
    struct INDEXSCALAR *next;
};
typedef struct INDEXSCALAR indexscalar_t;

#define INDEXSCALAR_INVERSE(n) ((n)->inverse)
#define INDEXSCALAR_VALUE(n) ((n)->value)
#define INDEXSCALAR_NEXT(n) ((n)->next)

struct INDEXCHAIN {
    indexscalar_t *current;
    struct INDEXCHAIN *next;
};
typedef struct INDEXCHAIN indexchain_t;

#define INDEXCHAIN_CURRENT(n) ((n)->current)
#define INDEXCHAIN_NEXT(n) ((n)->next)

struct INDEXLEVEL {
    indexvector_t *vector;
    indexchain_t *scalars;
    struct INDEXLEVEL *next;
};
typedef struct INDEXLEVEL indexlevel_t;

#define INDEXLEVEL_VECTOR(n) ((n)->vector)
#define INDEXLEVEL_SCALARS(n) ((n)->scalars)
#define INDEXLEVEL_NEXT(n) ((n)->next)

#define POS_VECTOR -1

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static indexvector_t *
NewIndexVector (node *value, bool inverse, indexvector_t *next)
{
    indexvector_t *result;

    DBUG_ENTER ();

    result = (indexvector_t *)MEMmalloc (sizeof (indexvector_t));

    INDEXVECTOR_VALUE (result) = value;
    INDEXVECTOR_INVERSE (result) = inverse;
    INDEXVECTOR_NEXT (result) = next;

    DBUG_RETURN (result);
}

static indexscalar_t *
NewIndexScalar (node *value, bool inverse, indexscalar_t *next)
{
    indexscalar_t *result;

    DBUG_ENTER ();

    result = (indexscalar_t *)MEMmalloc (sizeof (indexscalar_t));

    INDEXSCALAR_VALUE (result) = value;
    INDEXSCALAR_INVERSE (result) = inverse;
    INDEXSCALAR_NEXT (result) = next;

    DBUG_RETURN (result);
}

static indexchain_t *
NewIndexChain (indexscalar_t *current, indexchain_t *next)
{
    indexchain_t *result;

    DBUG_ENTER ();

    result = (indexchain_t *)MEMmalloc (sizeof (indexchain_t));

    INDEXCHAIN_CURRENT (result) = current;
    INDEXCHAIN_NEXT (result) = next;

    DBUG_RETURN (result);
}

static indexvector_t *
FreeIndexVector (indexvector_t *ivect)
{
    DBUG_ENTER ();

    if (ivect != NULL) {
        INDEXVECTOR_NEXT (ivect) = FreeIndexVector (INDEXVECTOR_NEXT (ivect));
        ivect = MEMfree (ivect);
    }

    DBUG_RETURN (ivect);
}

static indexscalar_t *
FreeIndexScalar (indexscalar_t *iscal)
{
    DBUG_ENTER ();

    if (iscal != NULL) {
        INDEXSCALAR_NEXT (iscal) = FreeIndexScalar (INDEXSCALAR_NEXT (iscal));
        iscal = MEMfree (iscal);
    }

    DBUG_RETURN (iscal);
}

static indexchain_t *
FreeIndexChain (indexchain_t *ichain)
{
    DBUG_ENTER ();

    if (ichain != NULL) {
        INDEXCHAIN_CURRENT (ichain) = FreeIndexScalar (INDEXCHAIN_CURRENT (ichain));
        INDEXCHAIN_NEXT (ichain) = FreeIndexChain (INDEXCHAIN_NEXT (ichain));
        ichain = MEMfree (ichain);
    }

    DBUG_RETURN (ichain);
}

static indexlevel_t *
FreeIndexLevel (indexlevel_t *ilevel)
{
    DBUG_ENTER ();

    if (ilevel != NULL) {
        INDEXLEVEL_VECTOR (ilevel) = FreeIndexVector (INDEXLEVEL_VECTOR (ilevel));
        INDEXLEVEL_SCALARS (ilevel) = FreeIndexChain (INDEXLEVEL_SCALARS (ilevel));
        INDEXLEVEL_NEXT (ilevel) = FreeIndexLevel (INDEXLEVEL_NEXT (ilevel));
        ilevel = MEMfree (ilevel);
    }

    DBUG_RETURN (ilevel);
}

#ifndef DBUG_OFF
static void
PrintNode (node *it)
{
    if (NODE_TYPE (it) == N_num) {
        fprintf (stderr, "%d", NUM_VAL (it));
    } else {
        fprintf (stderr, "%s", ID_NAME (it));
    }
}

static void
PrintIndexVector (indexvector_t *ivect)
{
    if (ivect != NULL) {
        PrintNode (INDEXVECTOR_VALUE (ivect));
        fprintf (stderr, "%s%s", INDEXVECTOR_INVERSE (ivect) ? "(inv)" : "",
                 (INDEXVECTOR_NEXT (ivect) != NULL) ? ", " : "");

        PrintIndexVector (INDEXVECTOR_NEXT (ivect));
    }
}

static void
PrintIndexScalar (indexscalar_t *iscal)
{
    if (iscal != NULL) {
        PrintNode (INDEXSCALAR_VALUE (iscal));
        fprintf (stderr, "%s%s", INDEXSCALAR_INVERSE (iscal) ? "(inv)" : "",
                 (INDEXSCALAR_NEXT (iscal) != NULL) ? ", " : "");

        PrintIndexScalar (INDEXSCALAR_NEXT (iscal));
    }
}

static void
PrintIndexChain (indexchain_t *ichain)
{
    if (ichain != NULL) {
        fprintf (stderr, "{");
        PrintIndexScalar (INDEXCHAIN_CURRENT (ichain));
        fprintf (stderr, "}");

        if (INDEXCHAIN_NEXT (ichain) != NULL) {
            fprintf (stderr, ", ");
            PrintIndexChain (INDEXCHAIN_NEXT (ichain));
        }
    }
}

static void
PrintIndexLevel (indexlevel_t *ilevel)
{
    if (ilevel != NULL) {
        PrintIndexLevel (INDEXLEVEL_NEXT (ilevel));
        fprintf (stderr, "LEVEL:\n  VECTORS: {");
        PrintIndexVector (INDEXLEVEL_VECTOR (ilevel));
        fprintf (stderr, "}\n  SCALARS: [");
        PrintIndexChain (INDEXLEVEL_SCALARS (ilevel));
        fprintf (stderr, "]\n\n");
    }
}
#endif /* not DBUG_OFF */

/** <!--********************************************************************-->
 * @}  <!-- local data structures -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool onefundef;
    node *vardecs;
    maskchain_t *locals;
    node *preassigns;
};

/**
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LOCALS(n) ((n)->locals)
#define INFO_PREASSIGNS(n) ((n)->preassigns)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_VARDECS (result) = NULL;
    INFO_LOCALS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;

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
 * @fn node *IVESLIdoIVESplitLoopInvariants( node *syntax_tree)
 *
 *****************************************************************************/
node *
IVESLIdoIVESplitLoopInvariants (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    INFO_ONEFUNDEF (info) = (NODE_TYPE (syntax_tree) == N_fundef);

    TRAVpush (TR_ivesli);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

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
 * @fn info *EnterLevel(dfmask_t *locals, info *arg_info)
 *
 * @brief Updates the info structure with information for the new nested scope.
 *
 * @param locals   locals mask of the current scope
 * @param arg_info current info structure
 *
 * @return updated info structure
 *
 *****************************************************************************/
static info *
EnterLevel (dfmask_t *locals, info *arg_info)
{
    maskchain_t *xnew;

    DBUG_ENTER ();

    DBUG_PRINT ("   >>> new level");

    xnew = (maskchain_t *)MEMmalloc (sizeof (maskchain_t));

    MASKCHAIN_LOCALS (xnew) = locals;
    MASKCHAIN_NEXT (xnew) = INFO_LOCALS (arg_info);
    INFO_LOCALS (arg_info) = xnew;

    DBUG_RETURN (arg_info);
}

/** <!-- ****************************************************************** -->
 * @fn info *LeaveLevel( info *arg_info)
 *
 * @brief Updates the info structure and removed the top-most scope.
 *
 * @param arg_info info structure to update
 *
 * @return updated info structure
 ******************************************************************************/
static info *
LeaveLevel (info *arg_info)
{
    maskchain_t *old;

    DBUG_ENTER ();

    DBUG_PRINT ("   <<< done level");

    old = INFO_LOCALS (arg_info);

    DBUG_ASSERT (old != NULL, "no more scopes to leave!");

    INFO_LOCALS (arg_info) = MASKCHAIN_NEXT (old);

    old = MEMfree (old);

    DBUG_RETURN (arg_info);
}

static node *
InsertLetAssign (node *op, ntype *restype, info *arg_info)
{
    node *assign, *avis;

    DBUG_ENTER ();

    DBUG_ASSERT (op != NULL, "empty rhs for let expression detected!");

    avis = TBmakeAvis (TRAVtmpVar (), restype);

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), op), NULL);
    AVIS_SSAASSIGN (avis) = assign;

    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));
    INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), assign);

    DBUG_RETURN (TBmakeId (avis));
}

static indexlevel_t *
MaskChainToIndexLevels (maskchain_t *masks)
{
    indexlevel_t *result = NULL;

    DBUG_ENTER ();

    if (masks != NULL) {
        result = (indexlevel_t *)MEMmalloc (sizeof (indexlevel_t));

        INDEXLEVEL_VECTOR (result) = NULL;
        INDEXLEVEL_SCALARS (result) = NULL;
        INDEXLEVEL_NEXT (result) = MaskChainToIndexLevels (MASKCHAIN_NEXT (masks));
    }

    DBUG_RETURN (result);
}

static indexchain_t *
InsertIntoScalars (node *iscal, bool invert, int pos, indexchain_t *chain)
{
    DBUG_ENTER ();

    if (chain == NULL) {
        /* the chain is created on demand */
        chain = NewIndexChain (NULL, NULL);
    }

    if (pos == 0) {
        INDEXCHAIN_CURRENT (chain)
          = NewIndexScalar (DUPdoDupNode (iscal), invert, INDEXCHAIN_CURRENT (chain));
    } else {
        INDEXCHAIN_NEXT (chain)
          = InsertIntoScalars (iscal, invert, pos - 1, INDEXCHAIN_NEXT (chain));
    }

    DBUG_RETURN (chain);
}

static indexlevel_t *
InsertIntoLevel (int pos, node *value, bool invert, indexlevel_t *levels,
                 maskchain_t *masks)
{
    DBUG_ENTER ();

    if ((NODE_TYPE (value) != N_id)
        || ((DFMtestMaskEntry (MASKCHAIN_LOCALS (masks), ID_AVIS (value)))
            || (MASKCHAIN_NEXT (masks) == NULL))) {
        /*
         * the variable is defined at the current level  -OR-
         * there are no more levels
         *
         * => insert here
         */
        if (pos == POS_VECTOR) {
            INDEXLEVEL_VECTOR (levels)
              = NewIndexVector (DUPdoDupNode (value), invert, INDEXLEVEL_VECTOR (levels));
        } else {
            INDEXLEVEL_SCALARS (levels)
              = InsertIntoScalars (value, invert, pos, INDEXLEVEL_SCALARS (levels));
        }
    } else {
        /* the variable is defined further out => insert there */
        INDEXLEVEL_NEXT (levels)
          = InsertIntoLevel (pos, value, invert, INDEXLEVEL_NEXT (levels),
                             MASKCHAIN_NEXT (masks));
    }

    DBUG_RETURN (levels);
}

static bool
SplitComputation (node *index, node **arg1, node **arg2, bool *invert)
{
    static pattern *isAddSub = NULL;
    static node *pPrf;
    bool result = FALSE;
    bool needsInvert = FALSE;

    DBUG_ENTER ();

    if (isAddSub == NULL) {
        isAddSub = PMprf (1, PMAgetNode (&pPrf), 0);
    }

    if (PMmatchFlat (isAddSub, index)) {
        switch (PRF_PRF (pPrf)) {
        case F_sub_SxS:
        case F_sub_VxV:
            needsInvert = TRUE;
            /* Falls through. */

        case F_add_SxS:
        case F_add_VxV:
            *arg1 = PRF_ARG1 (pPrf);
            *arg2 = PRF_ARG2 (pPrf);
            *invert = needsInvert;
            result = TRUE;
            break;

        default:
            result = FALSE;
        }
    }

    DBUG_RETURN (result);
}

/* advance declarations */
static indexlevel_t *SortIndexIntoLevels (node *idx, indexlevel_t *levels, bool invert,
                                          maskchain_t *locals);
static indexlevel_t *
SortIndexScalarsIntoLevelsHelper (node *idx, indexlevel_t *levels, bool invert,
                                  maskchain_t *locals, int pos)
{
    node *arg1, *arg2;
    node *realIdx;
    bool needsInvert;

    DBUG_ENTER ();

    if (idx != NULL) {
        realIdx = (NODE_TYPE (idx) == N_exprs) ? EXPRS_EXPR (idx) : idx;

        /* do this level */
        if (SplitComputation (realIdx, &arg1, &arg2, &needsInvert)) {
            levels = SortIndexScalarsIntoLevelsHelper (arg1, levels, invert, locals, pos);
            levels = SortIndexScalarsIntoLevelsHelper (arg2, levels, invert ^ needsInvert,
                                                       locals, pos);
        } else {
            levels = InsertIntoLevel (pos, realIdx, invert, levels, locals);
        }

        /* do the next one */
        if (NODE_TYPE (idx) == N_exprs) {
            levels = SortIndexScalarsIntoLevelsHelper (EXPRS_NEXT (idx), levels, invert,
                                                       locals, pos + 1);
        }
    }

    DBUG_RETURN (levels);
}

static indexlevel_t *
SortIndexScalarsIntoLevels (node *exprs, indexlevel_t *levels, bool invert,
                            maskchain_t *locals)
{
    DBUG_ENTER ();

    levels = SortIndexScalarsIntoLevelsHelper (exprs, levels, invert, locals, 0);

    DBUG_RETURN (levels);
}

static indexlevel_t *
SortIndexVectorIntoLevels (node *iv, indexlevel_t *levels, bool invert,
                           maskchain_t *locals)
{
    node *arg1, *arg2;
    bool needsInvert;

    DBUG_ENTER ();

    if (SplitComputation (iv, &arg1, &arg2, &needsInvert)) {
        levels = SortIndexIntoLevels (arg1, levels, invert, locals);
        levels = SortIndexIntoLevels (arg2, levels, invert ^ needsInvert, locals);
    } else {
        levels = InsertIntoLevel (POS_VECTOR, iv, invert, levels, locals);
    }

    DBUG_RETURN (levels);
}

static indexlevel_t *
SortIndexIntoLevels (node *idx, indexlevel_t *levels, bool invert, maskchain_t *locals)
{
    static pattern *isScalarizedP = NULL;
    static node *array;

    DBUG_ENTER ();

    DBUG_PRINT (("    ||| sorting index into level"));

    if (isScalarizedP == NULL) {
        isScalarizedP = PMarray (1, PMAgetNode (&array), 0);
    }

    if (PMmatchFlat (isScalarizedP, idx)) {
        DBUG_PRINT (("    ||| index is array node"));
        SortIndexScalarsIntoLevels (ARRAY_AELEMS (array), levels, invert, locals);
    } else {
        DBUG_PRINT (("    ||| index is id node"));
        SortIndexVectorIntoLevels (idx, levels, invert, locals);
    }

    DBUG_RETURN (levels);
}

static int
IndexChainLength (indexchain_t *chain)
{
    int len = 0;

    DBUG_ENTER ();

    if (chain != NULL) {
        len = 1 + IndexChainLength (INDEXCHAIN_NEXT (chain));
    }

    DBUG_RETURN (len);
}

static int
ComputeLevelPadding (indexlevel_t *levels)
{
    int result = 0;

    DBUG_ENTER ();

    if (levels != NULL) {
        result = MAX (result, ComputeLevelPadding (INDEXLEVEL_NEXT (levels)));
        result = MAX (result, IndexChainLength (INDEXLEVEL_SCALARS (levels)));
    }

    DBUG_RETURN (result);
}

static indexscalar_t *
SimplifyScalar (indexscalar_t *scalars, info *arg_info)
{
    indexscalar_t *result;
    indexscalar_t *second;

    DBUG_ENTER ();

    /* combine multiple scalars */
    if (INDEXSCALAR_NEXT (scalars) != NULL) {
        /* first combine the rest */
        INDEXSCALAR_NEXT (scalars)
          = SimplifyScalar (INDEXSCALAR_NEXT (scalars), arg_info);

        /* then combine these two
         *
         * Figuring out the operation:
         *
         *   (inv, inv) => add, result inv
         *   (inv, pos) => sub, result inv
         *   (pos, inv) => sub, result pos
         *   (pos, pos) => add, result pos
         */
        second = INDEXSCALAR_NEXT (scalars);
        result
          = NewIndexScalar (InsertLetAssign (TCmakePrf2 ((INDEXSCALAR_INVERSE (scalars)
                                                          == INDEXSCALAR_INVERSE (second))
                                                           ? F_add_SxS
                                                           : F_sub_SxS,
                                                         INDEXSCALAR_VALUE (scalars),
                                                         INDEXSCALAR_VALUE (second)),
                                             TYmakeAKS (TYmakeSimpleType (T_int),
                                                        SHmakeShape (0)),
                                             arg_info),
                            INDEXSCALAR_INVERSE (scalars), NULL);
        scalars = FreeIndexScalar (scalars);
    } else {
        result = scalars;
    }

    DBUG_RETURN (result);
}

static indexchain_t *
SimplifyChains (indexchain_t *chain, int pad, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (chain != NULL, "no chain to work on?!?");

    /* step 1: simplify or pad current chain */
    if (INDEXCHAIN_CURRENT (chain) == NULL) {
        /* this position is empty, so pad it */
        INDEXCHAIN_CURRENT (chain) = NewIndexScalar (TBmakeNum (0), FALSE, NULL);
    } else {
        INDEXCHAIN_CURRENT (chain)
          = SimplifyScalar (INDEXCHAIN_CURRENT (chain), arg_info);
    }

    /* step 2: potentially pad next chain */
    if ((pad != 1) && (INDEXCHAIN_NEXT (chain) == NULL)) {
        /* insert a new chain element */
        INDEXCHAIN_NEXT (chain) = NewIndexChain (NULL, NULL);
    }

    /* step 3: process next chain element */
    if (pad != 1) {
        INDEXCHAIN_NEXT (chain)
          = SimplifyChains (INDEXCHAIN_NEXT (chain), pad - 1, arg_info);
    }

    DBUG_ASSERT (((pad != 1) || (INDEXCHAIN_NEXT (chain) == NULL)),
                 "wrong padding value!");

    DBUG_RETURN (chain);
}

static indexvector_t *
SimplifyVectors (indexvector_t *vectors, info *arg_info)
{
    indexvector_t *result;
    indexvector_t *second;

    DBUG_ENTER ();

    /* combine multiple vectors */
    if (INDEXVECTOR_NEXT (vectors) != NULL) {
        /* first combine the rest */
        INDEXVECTOR_NEXT (vectors)
          = SimplifyVectors (INDEXVECTOR_NEXT (vectors), arg_info);

        /* then combine these two
         *
         * Figuring out the operation:
         *
         *   (inv, inv) => add, result inv
         *   (inv, pos) => sub, result inv
         *   (pos, inv) => sub, result pos
         *   (pos, pos) => add, result pos
         */
        second = INDEXVECTOR_NEXT (vectors);
        result
          = NewIndexVector (InsertLetAssign (TCmakePrf2 ((INDEXVECTOR_INVERSE (vectors)
                                                          == INDEXVECTOR_INVERSE (second))
                                                           ? F_add_VxV
                                                           : F_sub_VxV,
                                                         INDEXVECTOR_VALUE (vectors),
                                                         INDEXVECTOR_VALUE (second)),
                                             TYcopyType (
                                               ID_NTYPE (INDEXVECTOR_VALUE (vectors))),
                                             arg_info),
                            INDEXVECTOR_INVERSE (vectors), NULL);
        vectors = FreeIndexVector (vectors);
    } else {
        result = vectors;
    }

    DBUG_RETURN (result);
}

static indexlevel_t *
SimplifyLevels (indexlevel_t *levels, int pad, info *arg_info)
{
    DBUG_ENTER ();

    if (levels != NULL) {
        INDEXLEVEL_NEXT (levels)
          = SimplifyLevels (INDEXLEVEL_NEXT (levels), pad, arg_info);
        if (INDEXLEVEL_SCALARS (levels) != NULL) {
            INDEXLEVEL_SCALARS (levels)
              = SimplifyChains (INDEXLEVEL_SCALARS (levels), pad, arg_info);
        }
        if (INDEXLEVEL_VECTOR (levels) != NULL) {
            INDEXLEVEL_VECTOR (levels)
              = SimplifyVectors (INDEXLEVEL_VECTOR (levels), arg_info);
        }
        if ((INDEXLEVEL_VECTOR (levels) == NULL)
            && (INDEXLEVEL_SCALARS (levels) == NULL)) {
            /* empty level => discard */
            indexlevel_t *xthis = levels;
            levels = INDEXLEVEL_NEXT (levels);
            INDEXLEVEL_NEXT (xthis) = NULL;
            xthis = FreeIndexLevel (xthis);
        }
    }

    DBUG_RETURN (levels);
}

static node *
CombineVect2Offsets (node *first, node *second, bool inv, info *arg_info)
{
    node *result;

    DBUG_ENTER ();

    DBUG_ASSERT (second != NULL, "cannot combine with empty vect2offsets.");

    if (first == NULL) {
        if (inv) {
            result
              = InsertLetAssign (TCmakePrf1 (F_neg_S, second),
                                 TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)),
                                 arg_info);
        } else {
            result = second;
        }
    } else {
        result
          = InsertLetAssign (TCmakePrf2 ((inv ? F_sub_SxS : F_add_SxS), first, second),
                             TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)),
                             arg_info);
    }

    DBUG_RETURN (result);
}

static node *
IndexExpr2Offsets (node *expr, node *shp, info *arg_info)
{
    node *result;

    DBUG_ENTER ();

    result
      = InsertLetAssign (TCmakePrf2 (F_vect2offset, shp, expr),
                         TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)), arg_info);

    DBUG_RETURN (result);
}

static node *
IndexScalars2Exprs (indexchain_t *chain, info *arg_info)
{
    node *result = NULL;
    node *tmp;

    DBUG_ENTER ();

    if (chain != NULL) {
        result = IndexScalars2Exprs (INDEXCHAIN_NEXT (chain), arg_info);

        if (INDEXSCALAR_INVERSE (INDEXCHAIN_CURRENT (chain))) {
            tmp = InsertLetAssign (TCmakePrf1 (F_neg_S, INDEXSCALAR_VALUE (
                                                          INDEXCHAIN_CURRENT (chain))),
                                   TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)),
                                   arg_info);
        } else {
            tmp = INDEXSCALAR_VALUE (INDEXCHAIN_CURRENT (chain));
        }

        result = TBmakeExprs (tmp, result);
    }

    DBUG_RETURN (result);
}

static node *
ComputeVect2Offsets (indexlevel_t *levels, node *shp, info *arg_info)
{
    node *partial;
    node *result = NULL;

    DBUG_ENTER ();

    if (levels != NULL) {
        result = ComputeVect2Offsets (INDEXLEVEL_NEXT (levels), shp, arg_info);

        if (INDEXLEVEL_VECTOR (levels) != NULL) {
            partial = IndexExpr2Offsets (INDEXVECTOR_VALUE (INDEXLEVEL_VECTOR (levels)),
                                         DUPdoDupTree (shp), arg_info);
            result
              = CombineVect2Offsets (result, partial,
                                     INDEXVECTOR_INVERSE (INDEXLEVEL_VECTOR (levels)),
                                     arg_info);
        }

        if (INDEXLEVEL_SCALARS (levels) != NULL) {
            partial = TCmakeIntVector (
              IndexScalars2Exprs (INDEXLEVEL_SCALARS (levels), arg_info));
            partial = InsertLetAssign (partial, NTCnewTypeCheck_Expr (partial), arg_info);
            partial = IndexExpr2Offsets (partial, DUPdoDupTree (shp), arg_info);
            result = CombineVect2Offsets (result, partial, FALSE, arg_info);
        }
    }

    DBUG_RETURN (result);
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
 * @fn node *IVESLIfundef(node *arg_node, info *arg_info)
 *
 * @brief Starts the IVESLI traversal
 *
 *****************************************************************************/
node *
IVESLIfundef (node *arg_node, info *arg_info)
{
    dfmask_t *allSetMask;

    DBUG_ENTER ();

    DBUG_PRINT (">>> processing function %s...", CTIitemName (arg_node));

    if (FUNDEF_BODY (arg_node) != NULL) {
        arg_node = INFDFMSdoInferDfms (arg_node, HIDE_LOCALS_WITH | HIDE_LOCALS_WITH2
                                                   | HIDE_LOCALS_WITH3);

        /*
         * This is the failback mask that catches all variables in they have
         * not been found in a previous scope. For simplicity, I use a mask
         * that contains all variables
         */
        allSetMask = DFMgenMaskSet (FUNDEF_DFM_BASE (arg_node));
        arg_info = EnterLevel (allSetMask, arg_info);

        INFO_VARDECS (arg_info) = FUNDEF_VARDECS (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        FUNDEF_VARDECS (arg_node) = INFO_VARDECS (arg_info);

        arg_info = LeaveLevel (arg_info);
        allSetMask = DFMremoveMask (allSetMask);

        arg_node = RDFMSdoRemoveDfms (arg_node);
    }

    DBUG_PRINT ("<<< done function %s...", CTIitemName (arg_node));

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVESLIwith(node *arg_node, info *arg_info)
 *
 * @brief ...
 *
 *****************************************************************************/
node *
IVESLIwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_info = EnterLevel (WITH_LOCAL_MASK (arg_node), arg_info);
    arg_node = TRAVcont (arg_node, arg_info);
    arg_info = LeaveLevel (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVESLIwith2(node *arg_node, info *arg_info)
 *
 * @brief ...
 *
 *****************************************************************************/
node *
IVESLIwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_info = EnterLevel (WITH2_LOCAL_MASK (arg_node), arg_info);
    arg_node = TRAVcont (arg_node, arg_info);
    arg_info = LeaveLevel (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVESLIassign(node *arg_node, info *arg_info)
 *
 * @brief Starts the IVESLI traversal
 *
 *****************************************************************************/
node *
IVESLIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* bottom up traversal to ease inserting of pre-assigns */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_ASSERT (INFO_PREASSIGNS (arg_info) == NULL, "left over preassigns!");

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVESLIprf(node *arg_node, info *arg_info)
 *
 * @brief Starts the IVESLI traversal
 *
 *****************************************************************************/
node *
IVESLIprf (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_vect2offset) {
        DBUG_PRINT (("||| processing vect2offset phase 1 (inference)"));

        indexlevel_t *levels = MaskChainToIndexLevels (INFO_LOCALS (arg_info));
        levels = SortIndexIntoLevels (PRF_ARG2 (arg_node), levels, FALSE,
                                      INFO_LOCALS (arg_info));

        DBUG_EXECUTE (PrintIndexLevel (levels));

        DBUG_PRINT (("||| processing vect2offset phase 2 (simplify)"));

        levels = SimplifyLevels (levels, ComputeLevelPadding (levels), arg_info);

        DBUG_EXECUTE (PrintIndexLevel (levels));

        DBUG_PRINT (("||| processing vect2offset phase 3 (replace)"));
        new_node = ComputeVect2Offsets (levels, PRF_ARG1 (arg_node), arg_info);
        arg_node = FREEdoFreeNode (arg_node);

        if (new_node == NULL) {
            /* If after reassembly the vect2offset chain is empty, this can only
             * mean that the vect2offset before was empty, as well. Instead of
             * producing the empty vect2offset again, I directly insert its
             * scalar value: 0
             */
            new_node = TBmakeNum (0);
        }

        levels = FreeIndexLevel (levels);
    } else {
        new_node = arg_node;
    }

    DBUG_RETURN (new_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
