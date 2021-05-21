#include "handle_selection_dots.h"
#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "free.h"
#include "ctinfo.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "namespaces.h"
#include "new_types.h"
#include "globals.h"
#include "tree_compound.h"

#include <strings.h>

/**
 * @file handle_selection_dots.c
 *
 * This file implements the elimination of dots within selections.
 *
 * Multi dimensional selections are transfomed to their withloop
 * representation, thus eleminating all dots.
 *
 * The selection pattern we support consists of a list of 
 * either expressions, single- or triple-dopts. We can have at
 * most one triple-dot though.
 *
 * For example:
 *
 *    array[[ x, ., y, ..., z, ., ., r]]
 *
 * translates into:
 *
 * with {
 *    (. <= index <= .) : array[idx];
 * } : genarray( shp, default);
 *
 * where
 *    shp == [ shape(array)[1] ] 
 *           ++ drop( 3, drop( -4, shape(array))) 
 *           ++ [ shape(array)[dim(array)-3], shape(array)[dim(array)-2]]
 *
 *    idx == [ x, index[1], y]
 *           ++ drop( 1, drop( -2, index))
 *           ++ [ z, index[shape(index)-2], index[shape(index)-1], r]
 *
 *    default == zero (array);
 *
 *
 * Note here, that this traversal generates WLs with generators
 * that contain dots!
 * => HWLDdoEliminateWithLoopDots needs to be run *after* this traversal!
 *
 * Implementation notes:
 *-----------------------
 *
 *   We store the selection vector ([ x, ., y, ..., z, ., ., r]) in our example
 *   in a special data structure "dotinfo".
 *   It is populated through a function "MakeDotInfo" which in turn uses
 *   "BuildDotList".
 *
 *   The shape vector "shp" is computed by the local function "BuildShape"
 *   which uses "BuildLeftShape", "BuildMiddleShape", and "BuildRightShape"
 *   to construct the three parts shown above.
 *
 *   The selection index "idx" is computed by "BuildIndex" which, in turn,
 *   uses "BuildLeftIndex", "BuildMiddleIndex", and "BuildRightIndex"
 *   to construct the three parts shown above.
 *
 *   The default value is constructed by "BuildSelectionDefault". While this
 *   in case of the presence of a triple-dot (as above) is simple, it gets
 *   a little more intricate in cases without a triple-dot. Those are covered
 *   by using the functionis "BuildSelectionElementShape" and 
 *   "BuildDefaultWithloop".
 *
 *   Finally, the function "BuildWithLoop" constructs the overall result
 *   with-loop shown for our example above.
 *
 */

/**
 * set this to defined in order to create explanatory ids. use this only
 * for debugging as it might create very long identifier names.
 */
#define HSD_USE_EXPLANATORY_NAMES

/**
 * set this to use build in take/drop instead of withloops.
 *
 * NOTE: the withloop code was never tested and may contain bugs.
 */
#define HSD_USE_BUILTIN_TAKEDROP

/**
 * set this to use build in concat instead of withloops.
 *
 * NOTE: the withloop code was never tested and may contain bugs.
 */
#define HSD_USE_BUILTIN_CONCAT

/**
 * Structures to store all information about dots occuring in select state-
 * ments that are needed to perform the transformation. These structures are
 * built by BuildDotList.
 * All int values are counted beginning with 1. The 0 value is used as
 * flag for non-existent (false).
 */
typedef struct DOTLIST {
    size_t no;               /* number of dots counted from left */
    size_t position;         /* position of dot within selection */
    int dottype;             /* type of dot; 1:'.' 3: '...' */
    struct DOTLIST *next;    /* for building up a list ;) */
    struct DOTLIST *prev;
} dotlist;

typedef struct DOTINFO {
    dotlist *left;     /* left end of dotlist */
    dotlist *right;    /* right end */
    size_t dotcnt;     /* amount of dots found */
    size_t tripledot;  /* dotno of tripledot, 0 iff none found */
    size_t triplepos;  /* position of tripledot, 0 iff none found */
    size_t selcnt;     /* amount of selectors at all */
} dotinfo;

/**
 * arg_info in this file:
 */

/* INFO structure */
struct INFO {
    node *dummy;
};

/* access macros */
#define INFO_HSD_DUMMY(n) ((n)->dummy)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    DBUG_RETURN (result);
}

/**
 * frees an info structure.
 *
 * @param info the info structure to free
 */
static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * builds an assign-let construct.
 *
 * @param var_name the name of the identifier to use
 * @param let_expr right side expression of the assignment
 *
 * @return the ready built assignment to be inserted into the AST
 */
static node *
MakeAssignLetNV (char *var_name, node *let_expr)
{
    node *tmp_ids;
    node *tmp_node;

    DBUG_ENTER ();

    tmp_ids = TBmakeSpids (var_name, NULL);

    tmp_node = TBmakeLet (tmp_ids, let_expr);
    tmp_node = TBmakeAssign (tmp_node, NULL);

    DBUG_RETURN (tmp_node);
}

/**
 * collects the needed dot information by traversing the AST:
 * - counts the total amount of dots found (. and ..)
 * - stores for each dot ist position within the selection vector
 * - stores the position of ... if any found
 * - counts the amount of expressions within the selection vector
 *
 * @param tree part of the AST, usually a vector of expressions linked by
 *             EXPRS nodes.
 * @param info dotinfo structure for storing the information found
 */
static void
BuildDotList (node *tree, dotinfo *info)
{
    DBUG_ENTER ();

    while (tree != NULL) {
        node *handle = EXPRS_EXPR (tree);
        info->selcnt++;

        if (NODE_TYPE (handle) == N_dot) {
            dotlist *newdot = (dotlist *)MEMmalloc (sizeof (dotlist));

            info->dotcnt++;
            newdot->position = info->selcnt;
            newdot->no = info->dotcnt;
            newdot->dottype = DOT_NUM (handle);
            newdot->next = NULL;

            if (info->right == NULL) {
                newdot->prev = NULL;
                info->left = newdot;
                info->right = newdot;
            } else {
                newdot->prev = info->right;
                newdot->prev->next = newdot;
                info->right = newdot;
            }
            if (newdot->dottype == 3) {
                if (info->tripledot == 0) {
                    info->tripledot = info->dotcnt;
                    info->triplepos = info->selcnt;
                } else {
                    /* there are multiple occurences of '...' */
                    CTIerrorLine (global.linenum,
                                  "Multiple occurences of ... are not allowed in a "
                                  "single select statement.");
                }
            }
        }

        tree = EXPRS_NEXT (tree);
    }

    DBUG_RETURN ();
}

/**
 * wrapper function for BuildDotList. Creates an empty dotinfo structure
 * and initializes it with legal values prior to calling BuildDotList.
 *
 * @param args part of the AST, usually a selection vector
 *
 * @return ready built dotinfo structure
 */
static dotinfo *
MakeDotInfo (node *args)
{
    dotinfo *result;

    DBUG_ENTER ();

    result = (dotinfo *)MEMmalloc (sizeof (dotinfo));

    result->left = NULL;
    result->right = NULL;
    result->dotcnt = 0;
    result->tripledot = 0;
    result->triplepos = 0;
    result->selcnt = 0;

    BuildDotList (args, result);

    DBUG_RETURN (result);
}

/**
 * frees all memory allocated by a dotinfo structure, including the inner
 * linked list.
 *
 * @param node dotinfo structure to clear
 */
static void
FreeDotInfo (dotinfo *node)
{
    DBUG_ENTER ();

    while (node->left != NULL) {
        dotlist *tmp = node->left;
        node->left = node->left->next;
        tmp = MEMfree (tmp);
    }

    node = (dotinfo *)MEMfree ((void *)node);

    DBUG_RETURN ();
}

/**
 * @fn int LDot2Pos(size_t dot, dotinfo* info)
 *
 * transforms a given number of a dot (counted from the left) into its
 * position within the selection vector. Counting starts at 1.
 *
 * @return the position of the given dot within the selection vector
 */
static size_t
LDot2Pos (size_t dot, dotinfo *info)
{
    dotlist *dots = info->left;
    size_t cnt;

    DBUG_ENTER ();

    for (cnt = 1; cnt < dot; cnt++) {
        dots = dots->next;
    }

    DBUG_RETURN (dots->position);
}

/**
 * transforms a given number of a dot (counted from the right) into its
 * postion within the selection vector. Counting starts with 1.
 *
 * @return the position of the given dot within the selection vector
 */
static size_t
RDot2Pos (size_t dot, dotinfo *info)
{
    dotlist *dots = info->right;
    size_t cnt;

    DBUG_ENTER ();

    for (cnt = 1; cnt < dot; cnt++) {
        dots = dots->prev;
    }

    DBUG_RETURN (info->selcnt - dots->position + 1);
}

/**
 * checks whether the expression at position dot within the selection vector
 * is a dot or not. If it is a dot, its number counted from the left is
 * returned, zero otherwise. Counting starts with one.
 *
 * @return dot position counted from left or zero if not a dot
 */
static size_t
LIsDot (size_t dot, dotinfo *info)
{
    size_t result = 0;
    dotlist *list = info->left;

    DBUG_ENTER ();

    while ((list != NULL) && (list->position <= dot)) {
        if (list->position == dot) {
            result = list->no;
            break;
        }

        list = list->next;
    }

    DBUG_RETURN (result);
}

/**
 * Checks whether the expression at position dot within the selection vector
 * is a dot or not. If it is a dot, its number counted from the right is
 * returned, zero otherwise. Counting starts at one.
 *
 * @return position counted from the right or zero
 */
static size_t
RIsDot (size_t dot, dotinfo *info)
{
    size_t result = 0;

    DBUG_ENTER ();

    result = LIsDot (info->selcnt - dot + 1, info);

    if (result != 0) {
        result = info->dotcnt - result + 1;
    }

    DBUG_RETURN (result);
}

/**
 * builds an id with a free name by calling TmpVarName. If
 * HSD_USE_EXPLANATORY_NAMES is set, name is appended to the new id,
 * Use this feature only for debugging, as it might create very long
 * identifier names.
 *
 * @param name explanatory name of the identifier
 * @return a new created unique id node
 */
static node *
MakeTmpId (char *name)
{
    node *result;

    DBUG_ENTER ();

#ifdef HSD_USE_EXPLANATORY_NAMES
    result = TBmakeSpid (NULL, TRAVtmpVarName (name));
#else
    result = TBmakeSpid (NULL, TRAVtmpVar ());
#endif

    DBUG_RETURN (result);
}

/**
 * builds code for a drop operation used to isolate the middle part
 * of an index or shape vector.
 *
 * @param left a sac expression that can be evaluated to an integer. Amount
 *             of elements to drop on the left side.
 * @param right a sac expression that can be evaluated to an integer. Amount
 *              of elements to drop on the right side.
 * @param vector vector to operate on
 * @return part of the AST that evaluates to the requested desired middle part.
 */
static node *
BuildDrop (node *left, node *right, node *vector)
{
#ifdef HSD_USE_BUILTIN_TAKEDROP
    node *result;

    DBUG_ENTER ();

    result = MAKE_BIN_PRF (F_drop_SxV, left,
                           MAKE_BIN_PRF (F_drop_SxV,
                                         MAKE_BIN_PRF (F_mul_SxS, TBmakeNum (-1), right),
                                         vector));

    DBUG_RETURN (result);
#else
    /*
     * use function Stdlib:drop instead
     */
    result = TCmakeSpap2 (STRcpy ("Stdlib"), STRcpy ("drop"), left,
                          TCmakeSpap2 (STRcpy ("Stdlib"), STRcpy ("drop"),
                                       MAKE_BIN_PRF (F_mul_SxS, TBmakeNum (-1), right),
                                       vector));
#endif
}

/**
 * builds code that concatenates two vectors.
 *
 * @param a sac expression that evaluates to a vector.
 * @param b sac expression that evaluates to a vector.
 * @return part if the AST containing the code concatenating the two
 *         vectors
 */
static node *
BuildConcat (node *a, node *b)
{
#ifdef HSD_USE_BUILTIN_CONCAT
    node *result;

    DBUG_ENTER ();

    result = MAKE_BIN_PRF (F_cat_VxV, a, b);

    DBUG_RETURN (result);
#else
    result = TCmakeSpap2 (STRcpy ("Stdlib"), STRcpy ("concat"), a, b);
#endif
}

/**
 * builds the left part of the result shape vector containing any
 * shape information based upon dots found before a tripledot. For
 * every dot within the selection vector, the corresponding shape
 * of the current array is inserted. The shape at non-dot positions
 * is ignored.
 *
 * @param array the id of the array on that the selection operates
 * @param info the dotinfo structure
 * @return sac code representing the shape vector
 */
static node *
BuildLeftShape (node *array, dotinfo *info)
{
    size_t cnt;
    size_t maxdot;
    node *result = NULL;

    DBUG_ENTER ();

    if (info->tripledot == 0)
        maxdot = info->dotcnt;
    else
        maxdot = info->tripledot - 1;

    for (cnt = maxdot; cnt > 0; cnt--) {
        result = TBmakeExprs (MAKE_BIN_PRF (F_sel_VxA,
                                            TCmakeIntVector (
                                              TBmakeExprs (TBmakeNum ((int)LDot2Pos (cnt, info)
                                                                      - 1),
                                                           NULL)),
                                            TBmakePrf (F_shape_A,
                                                       TBmakeExprs (DUPdoDupTree (array),
                                                                    NULL))),
                              result);
    }

    /* do not create empty array */
    if (result != NULL) {
        result = TCmakeIntVector (result);
    }

    DBUG_RETURN (result);
}

/**
 * Builds the middle part of the result shape vector. The middle shape is
 * built by removing all elements corresponding to entries wihtin the
 * selection vector occuring prior to the triple dot from the left and
 * removing those entries occuring past the triple dot from the right.
 *
 * @param array array that the selection operates on
 * @param info dotinfo structure
 * @return sac code representing the middle shape vector
 */
static node *
BuildMiddleShape (node *array, dotinfo *info)
{
    node *result = NULL;
    node *shape = NULL;
    node *left = NULL;
    node *right = NULL;

    DBUG_ENTER ();

    shape = TBmakePrf (F_shape_A, TBmakeExprs (DUPdoDupTree (array), NULL));

    left = TBmakeNum ((int)info->triplepos - 1);

    right = TBmakeNum ((int)(info->selcnt - info->triplepos));

    result = BuildDrop (left, right, shape);

    DBUG_RETURN (result);
}

/**
 * builds the right part of the result shape vector. For every dot occuring
 * on the right side of the triple dot, the corresponding shape of the
 * array the selection operartes on, is inserted. The shape information
 * corresponding to non dot entries is ignored.
 *
 * @param array array the selection operates on
 * @param info dotinfo structure
 * @return sac representation of the right result shape vector
 */
static node *
BuildRightShape (node *array, dotinfo *info)
{
    size_t cnt;
    size_t maxdot;
    node *result = NULL;

    DBUG_ENTER ();

    maxdot = info->dotcnt - info->tripledot;

    for (cnt = 1; cnt <= maxdot; cnt++) {
        result = TBmakeExprs (
          MAKE_BIN_PRF (
            F_sel_VxA,
            TCmakeIntVector (TBmakeExprs (
              MAKE_BIN_PRF (
                F_sub_SxS,
                MAKE_BIN_PRF (F_sel_VxA,
                              TCmakeIntVector (TBmakeExprs (TBmakeNum (0), NULL)),
                              TBmakePrf (F_shape_A,
                                         TBmakeExprs (TBmakePrf (F_shape_A,
                                                                 TBmakeExprs (DUPdoDupTree (
                                                                                array),
                                                                              NULL)),
                                                      NULL))),
                TBmakeNum ((int)RDot2Pos (cnt, info))),
              NULL)),
            TBmakePrf (F_shape_A, TBmakeExprs (DUPdoDupTree (array), NULL))),
          result);
    }

    /* do not create empty array */
    if (result != NULL) {
        result = TCmakeIntVector (result);
    }

    DBUG_RETURN (result);
}

/**
 * Build the result shape vector of the selection. The shape consists of
 * three parts, the left, middle and right shape vector. See
 * BuildMiddle/Left/RightShape for details. The parts are assigned to
 * temporary identifiers and concatenated by runtime code. See
 * BuildConcat for details.
 * In order to insert the new identifiers, they are added to the
 * assigns chain and inserted by HSDap in front of the selection.
 *
 * @param array array the selection operates on
 * @param info dotinfo structure
 * @result sac representation of the result shape vector
 */
static node *
BuildShape (node *array, dotinfo *info)
{
    node *leftshape = NULL;
    node *middleshape = NULL;
    node *rightshape = NULL;

    DBUG_ENTER ();

    if (info->triplepos != 1) {
        leftshape = BuildLeftShape (array, info);
    }

    if (info->triplepos != 0) {
        middleshape = BuildMiddleShape (array, info);
    }

    if ((info->triplepos != 0) && (info->triplepos != info->selcnt)) {
        rightshape = BuildRightShape (array, info);
    }

    if (rightshape != NULL) {
        middleshape = BuildConcat (middleshape, rightshape);
        rightshape = NULL;
    }

    if (middleshape != NULL) {
        if (leftshape == NULL) {
            leftshape = middleshape;
            middleshape = NULL;
        } else {
            leftshape = BuildConcat (leftshape, middleshape);
            middleshape = NULL;
        }
    }

    DBUG_ASSERT (leftshape != NULL, "error building shape: the shape is empty!");

    DBUG_RETURN (leftshape);
}

/**
 * builds the left part of the index vector that is used to select each
 * element of the result within the withloop. For every dot within the
 * selection vector, the corresponding part of the withloops index vector
 * is inserted, otherwise the element within the selection vector is inserted.
 *
 * @param args selection vector the selection operates on
 * @param iv identifier of the withloop index vector
 * @param info dotinfo structure
 * @return left part of the index vector
 */
static node *
BuildLeftIndex (node *args, node *iv, dotinfo *info)
{
    size_t cnt;
    size_t maxcnt;
    node *result = NULL;

    DBUG_ENTER ();

    if (info->tripledot == 0) {
        maxcnt = info->selcnt;
    } else {
        maxcnt = info->triplepos - 1;
    }

    for (cnt = maxcnt; cnt > 0; cnt--) {
        if (LIsDot (cnt, info)) {
            /* Make selection iv[ldot(cnt)-1] */
            result = TBmakeExprs (MAKE_BIN_PRF (F_sel_VxA,
                                                TCmakeIntVector (
                                                  TBmakeExprs (TBmakeNum (
                                                                 (int)LIsDot (cnt, info) - 1),
                                                               NULL)),
                                                DUPdoDupTree (iv)),
                                  result);
        } else {
            result
              = TBmakeExprs (DUPdoDupTree (TCgetNthExprsExpr (cnt - 1, args)), result);
        }
    }

    result = TCmakeIntVector (result);

    DBUG_RETURN (result);
}

/**
 * build the middle part of the index vector used within the withloop
 * to select the elements of the result. The middle part is constructed
 * by dropping the elements not belonging to the triple dot, thus already
 * matched by another element of the selection vector.
 *
 * @param args selection vector the selection operates on
 * @param iv id of the withloops index vector
 * @param info dotinfo structure
 * @return sac representation of the middle index vector
 */
static node *
BuildMiddleIndex (node *args, node *iv, dotinfo *info)
{
    node *result = NULL;
    node *left = NULL;
    node *right = NULL;

    DBUG_ENTER ();

    left = TBmakeNum ((int)info->tripledot - 1);
    right = TBmakeNum ((int)(info->dotcnt - info->tripledot));

    result = BuildDrop (left, right, DUPdoDupTree (iv));

    DBUG_RETURN (result);
}

/**
 * builds the right part of selection vector used within the withloop
 * to select every element of the result. For every dot within the
 * selection vector the corresponding element of the indexvector is inserted.
 * For non dot elements, those are inserted.
 *
 * @param args selection vector the selection operates on
 * @param iv identifier of the withloops indexvector
 * @param info dotinfo structure
 * @return sac representation of the index vectors right part
 */
static node *
BuildRightIndex (node *args, node *iv, dotinfo *info)
{
    size_t cnt;
    size_t maxcnt;
    node *result = NULL;

    DBUG_ENTER ();

    maxcnt = info->selcnt - info->triplepos;

    for (cnt = 1; cnt <= maxcnt; cnt++) {
        if (RIsDot (cnt, info)) {
            /* Make selection iv[selcnt - rdot(cnt)] */
            result = TBmakeExprs (
              MAKE_BIN_PRF (
                F_sel_VxA,
                TCmakeIntVector (TBmakeExprs (
                  MAKE_BIN_PRF (F_sub_SxS,
                                MAKE_BIN_PRF (F_sel_VxA,
                                              TCmakeIntVector (
                                                TBmakeExprs (TBmakeNum (0), NULL)),
                                              TBmakePrf (F_shape_A,
                                                         TBmakeExprs (DUPdoDupTree (iv),
                                                                      NULL))),
                                TBmakeNum ((int)RIsDot (cnt, info))),
                  NULL)),
                DUPdoDupTree (iv)),
              result);
        } else {
            result
              = TBmakeExprs (DUPdoDupTree (TCgetNthExprsExpr (info->selcnt - cnt, args)),
                             result);
        }
    }

    result = TCmakeVector (TYmakeAUD (TYmakeSimpleType (T_unknown)), result);

    DBUG_RETURN (result);
}

/**
 * builds the indexvector used within the withloop to select each element
 * of the result. The indexvector is built out of three parts. See
 * BuildMiddle/Left/RightIndex for details. The three parts are concatenated
 * during runtime. The sac code is built by BuildConcat.
 *
 * @param args selection vector the selection operates on
 * @param iv identifier of the withloops index vector
 * @param block returns the code block of the withloop used to concatenate
 *              the three parts during runtime
 * @param info dotinfo structure
 * @return identifiert of the selection index created
 */
static node *
BuildIndex (node *args, node *iv, node *block, dotinfo *info)
{
    node *leftindex = NULL;
    node *leftid = NULL;
    node *middleindex = NULL;
    node *middleid = NULL;
    node *rightindex = NULL;
    node *rightid = NULL;

    DBUG_ENTER ();

    if (info->triplepos != 1) {
        leftindex = BuildLeftIndex (args, iv, info);
        leftid = MakeTmpId ("left_index");
        BLOCK_ASSIGNS (block)
          = TCappendAssign (BLOCK_ASSIGNS (block),
                            MakeAssignLetNV (STRcpy (SPID_NAME (leftid)), leftindex));
    }

    if (info->triplepos != 0) {
        middleindex = BuildMiddleIndex (args, iv, info);
        middleid = MakeTmpId ("middle_index");
        BLOCK_ASSIGNS (block)
          = TCappendAssign (BLOCK_ASSIGNS (block),
                            MakeAssignLetNV (STRcpy (SPID_NAME (middleid)), middleindex));
    }

    if ((info->triplepos != 0) && (info->triplepos != info->selcnt)) {
        rightindex = BuildRightIndex (args, iv, info);
        rightid = MakeTmpId ("right_index");
        BLOCK_ASSIGNS (block)
          = TCappendAssign (BLOCK_ASSIGNS (block),
                            MakeAssignLetNV (STRcpy (SPID_NAME (rightid)), rightindex));
    }

    if (rightid != NULL) {
        node *tmpid = NULL;

        tmpid = MakeTmpId ("middle_and_right_index");

        BLOCK_ASSIGNS (block)
          = TCappendAssign (BLOCK_ASSIGNS (block),
                            MakeAssignLetNV (STRcpy (SPID_NAME (tmpid)),
                                             BuildConcat (middleid, rightid)));

        middleid = tmpid;
        rightid = NULL;
    }

    if (middleid != NULL) {
        if (leftid == NULL) {
            leftid = middleid;
            middleid = NULL;
        } else {
            node *tmpid = NULL;

            tmpid = MakeTmpId ("complete_index");

            BLOCK_ASSIGNS (block)
              = TCappendAssign (BLOCK_ASSIGNS (block),
                                MakeAssignLetNV (STRcpy (SPID_NAME (tmpid)),
                                                 BuildConcat (leftid, middleid)));

            leftid = tmpid;
            middleid = NULL;
        }
    }

    DBUG_ASSERT (leftid != NULL, "error building index: the index is empty!");

    DBUG_RETURN (leftid);
}

/**
 * builds a withloop generating an array containing
 * zeroes with given shape
 *
 * @param array AST node of the array
 * @param shape AST node of shape, is consumed
 */

static node *
BuildDefaultWithloop (node *array, node *shape)
{
    node *result = NULL;

    DBUG_ENTER ();

    result
      = TBmakeWith (TBmakePart (NULL,
                                TBmakeWithid (TBmakeSpids (TRAVtmpVar (), NULL), NULL),
                                TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1),
                                                 TBmakeDot (1), NULL, NULL)),
                    TBmakeCode (MAKE_EMPTY_BLOCK (),
                                TBmakeExprs (TCmakeSpap1 (NSgetNamespace (
                                                            global.preludename),
                                                          STRcpy ("zero"),
                                                          DUPdoDupTree (array)),
                                             NULL)),
                    TBmakeGenarray (shape, NULL));

    GENARRAY_DEFAULT (WITH_WITHOP (result))
      = TCmakeSpap1 (NSgetNamespace (global.preludename), STRcpy ("zero"),
                     DUPdoDupTree (array));

    CODE_USED (WITH_CODE (result))++;
    PART_CODE (WITH_PART (result)) = WITH_CODE (result);

    DBUG_RETURN (result);
}

/**
 * builds the shape of an selected element. There is no expansion
 * of dot shapes.
 *
 * @param array AST of the array the selection takes place on
 * @param info corresponding dotinfo structure
 * @return AST of the shape
 */

static node *
BuildSelectionElementShape (node *array, dotinfo *info)
{
    node *shape = NULL;

    DBUG_ENTER ();

    shape
      = MAKE_BIN_PRF (F_drop_SxV, TBmakeNum ((int)info->selcnt),
                      TBmakePrf (F_shape_A, TBmakeExprs (DUPdoDupTree (array), NULL)));

    DBUG_RETURN (shape);
}

/**
 * builds a default value for the selection.
 * @param array AST node of the array
 * @param info dotinfo structure of the array
 */

static node *
BuildSelectionDefault (node *array, dotinfo *info)
{
    node *result = NULL;

    DBUG_ENTER ();

    if (info->triplepos == 0) {
        /* no tripledot, build default */

        node *shape = BuildSelectionElementShape (array, info);

        result = BuildDefaultWithloop (array, shape);
    } else {
        /* default is just a scalar */

        result = TBmakeExprs (DUPdoDupTree (array), NULL);
        result
          = TBmakeSpap (TBmakeSpid (NSgetNamespace (global.preludename), STRcpy ("zero")),
                        result);
    }

    DBUG_RETURN (result);
}

/**
 * builds the withloop construct replacing the selection.
 *
 * @param shape shape vector of the withloop
 * @param iv identifier of the index vector
 * @param array array the withloop operates ib
 * @param index index of the selection
 * @param block the withloops inner code block
 * @return sac code of the withloop
 */
static node *
BuildWithLoop (node *shape, node *iv, node *array, node *index, node *block,
               dotinfo *info)
{
    node *result;
    node *ap;
    node *ids;

    DBUG_ENTER ();

    ap = TBmakeSpap (TBmakeSpid (NULL, STRcpy ("sel")),
                     TBmakeExprs (index, TBmakeExprs (DUPdoDupTree (array), NULL)));

    ids = TBmakeSpids (STRcpy (SPID_NAME (iv)), NULL);

    result = TBmakeWith (TBmakePart (NULL, TBmakeWithid (ids, NULL),
                                     TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1),
                                                      TBmakeDot (1), NULL, NULL)),
                         TBmakeCode (block, TBmakeExprs (ap, NULL)),
                         TBmakeGenarray (shape, NULL));

    GENARRAY_DEFAULT (WITH_WITHOP (result)) = BuildSelectionDefault (array, info);

    CODE_USED (WITH_CODE (result))++;
    PART_CODE (WITH_PART (result)) = WITH_CODE (result);

    DBUG_RETURN (result);
}




/**
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
HSDdoEliminateSelDots (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_hsd);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}

/**
 * hook to handle axis control selections. Any selection found is parsed
 * for dots and if any found replaced by the matching withloop.
 * If currently scanning for selection ids, the ids of the selection are
 * passed to ScanId/ScanVector depending on the selection vector type.
 * All other AP nodes are passed without any further action.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return transformed AST
 */
node *
HSDspap (node *arg_node, info *arg_info)
{
    node *result = arg_node;

    DBUG_ENTER ();

    /* only sel statements are of interest here, so just return */
    /* on anything else                                         */
    /* besides ARG1 must be an array. because otherwise there   */
    /* is no possibility to find any dot...                     */

    if (STReq (SPAP_NAME (arg_node), "sel")
        && (SPAP_NS (arg_node) == NULL)
        && (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_array)) {
        dotinfo *info = MakeDotInfo (ARRAY_AELEMS (SPAP_ARG1 (arg_node)));

        if (info->dotcnt != 0) {
            node *shape;
            node *iv;
            node *index;
            node *block;

            iv = MakeTmpId ("index");
            block = MAKE_EMPTY_BLOCK ();
            shape = BuildShape (SPAP_ARG2 (arg_node), info);

            index = BuildIndex (ARRAY_AELEMS (SPAP_ARG1 (arg_node)), iv, block, info);

            result = BuildWithLoop (shape, iv, SPAP_ARG2 (arg_node), index, block, info);

            arg_node = FREEdoFreeTree (arg_node);
            iv = FREEdoFreeNode (iv);
        }

        FreeDotInfo (info);
    }

    /* Now we traverse our result in order to handle any */
    /* dots inside.                                      */

    if (result != NULL) {
        if (NODE_TYPE (result) == N_spap) {
            if (SPAP_ARGS (result) != NULL)
                SPAP_ARGS (result) = TRAVdo (SPAP_ARGS (result), arg_info);
        } else {
            result = TRAVdo (result, arg_info);
        }
    }

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
