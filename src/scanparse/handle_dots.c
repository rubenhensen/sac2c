/*
 *
 * $Log$
 * Revision 1.42  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.41  2004/12/02 15:14:49  sah
 * intermediate fix
 *
 * Revision 1.40  2004/12/01 18:49:01  sah
 * post DK bugfixing
 *
 * Revision 1.39  2004/12/01 14:17:41  sah
 * fixed usage of AP_NAME
 *
 * Revision 1.38  2004/11/25 22:26:47  sah
 * COMPILES!
 *
 *
 * Revision 1.1  2002/07/09 12:54:25  sbs
 * Initial revision
 *
 */

#include "handle_dots.h"
#include "traverse.h"
#include "dbug.h"
#include "free.h"
#include "Error.h"
#include "DupTree.h"
#include "internal_lib.h"
#include "tree_basic.h"

#include <strings.h>

/**
 * @file handle_dots.c
 *
 * This file contains any code needed to eleminate dots within
 * sac source code. Dots can appear in the following positions:
 * - as boundary shortcuts in withloops
 * - to mark free dimensions within a selection
 *
 * Dots at boundary positions within withloops are replaced by the
 * minimal/maximal possible value, eg. 0 and the shape vector. As
 * a side effect, the comparison operators are 'normalized' to <= for
 * lower boundaries and < for the upper ones.
 *
 * Multi dimensional selections are transfomed to their withloop
 * representation, thus eleminating any dots.
 *
 * As well, the new set notation is transformed into its withloop
 * representation, as it usually appears near to multi dimensional
 * selections.
 *
 * After traversal, there should be no more dot nodes within the AST.
 * Otherwise a warning is generated.
 */

/**
 * set this to defined in order to create explanatory ids. use this only
 * for debugging as it might create very long identifier names.
 */
#undef HD_USE_EXPLANATORY_NAMES

/**
 * set this to use build in take/drop instead of withloops.
 *
 * NOTE: the withloop code was never tested and may contain bugs.
 */
#define HD_USE_BUILTIN_TAKEDROP

/**
 * set this to use build in concat instead of withloops.
 *
 * NOTE: the withloop code was never tested and may contain bugs.
 */
#define HD_USE_BUILTIN_CONCAT

/**
 * set this to enable support for vectors as index of a set notation.
 *
 * NOTE: The implementation is far from complete:
 *       -missing default values in resulting WL
 *       -no support of partial selection on arrays of different
 *        dimenionality
 */
#undef HD_SETWL_VECTOR

/**
 * Structures to store all information about dots occuring in select state-
 * ments that are needed to perform the transformation. These structures are
 * built by BuildDotList.
 * All int values are counted beginning with 1. The 0 value is used as
 * flag for non-existent (false).
 */
typedef struct DOTLIST {
    int no;               /* number of dots counted from left */
    int position;         /* position of dot within selection */
    int dottype;          /* type of dot; 1:'.' 3: '...' */
    struct DOTLIST *next; /* for building up a list ;) */
    struct DOTLIST *prev;
} dotlist;

typedef struct DOTINFO {
    dotlist *left;  /* left end of dotlist */
    dotlist *right; /* right end */
    int dotcnt;     /* amount of dots found */
    int tripledot;  /* dotno of tripledot, 0 iff none found */
    int triplepos;  /* position of tripledot, 0 iff none found */
    int selcnt;     /* amount of selectors at all */
} dotinfo;

/**
 * Structures to store ids and shapes during shape-scan. Filled during
 * traversal in HD_sacn mode.
 */
typedef enum TRAVSTATE { HD_sel, HD_scan, HD_default } travstate;
typedef enum IDTYPE { ID_vector, ID_scalar } idtype;

typedef struct SHPCHAIN {
    node *shape;
    node *code;
    struct SHPCHAIN *next;
} shpchain;

typedef struct IDTABLE {
    char *id;
    idtype type;
    shpchain *shapes;
    struct IDTABLE *next;
} idtable;

/**
 * arg_info in this file:
 * DOTSHAPE:    this field is used in order to transport the generic shape
 *              from Nwithid (via Nwith) to Ngenerator, where it may be used
 *              in order to replace . generator boundaries.
 * TRAVSTATE:   this field is used to determine the current traversalmode
 *              HD_sel in normal mode (eliminate dots)
 *              HD_scan in shape scanning mode
 *              HD_default to build default values for withloops
 * IDTABLE:     used to reference the current idtable.
 *
 * ASSIGNS:     stores any assigns that have to be inserted prior to
 *              the current one. Used to build shape for WLs.
 * SETASSIGNS:  stores any assigns that haveo to be inserted prior to
 *              the current set notation assign. Used to pre-flatten
 *              selections in setnotation.
 */

/* INFO structure */
struct INFO {
    node *dotshape;
    travstate state;
    idtable *idtab;
    node *assigns;
    node *setassigns;
};

/* access macros */
#define INFO_HD_DOTSHAPE(n) n->dotshape
#define INFO_HD_TRAVSTATE(n) n->state
#define INFO_HD_IDTABLE(n) n->idtab
#define INFO_HD_ASSIGNS(n) n->assigns
#define INFO_HD_SETASSIGNS(n) n->setassigns

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_HD_DOTSHAPE (result) = NULL;
    INFO_HD_TRAVSTATE (result) = HD_sel;
    INFO_HD_IDTABLE (result) = NULL;
    INFO_HD_ASSIGNS (result) = NULL;
    INFO_HD_SETASSIGNS (result) = NULL;

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
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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

    DBUG_ENTER ("MakeAssignLet");

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
    DBUG_ENTER ("BuildDotList");

    while (tree != NULL) {
        node *handle = EXPRS_EXPR (tree);
        info->selcnt++;

        if (NODE_TYPE (handle) == N_dot) {
            dotlist *newdot = ILIBmalloc (sizeof (dotlist));

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
                    ERROR (global.linenum,
                           ("Multiple occurences of ... are not allowed in a "
                            "single select statement."));
                }
            }
        }

        tree = EXPRS_NEXT (tree);
    }

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("MakeDotInfo");

    result = (dotinfo *)ILIBmalloc (sizeof (dotinfo));

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
    DBUG_ENTER ("FreeDotInfo");

    while (node->left != NULL) {
        dotlist *tmp = node->left;
        node->left = node->left->next;
        ILIBfree (tmp);
    }

    ILIBfree (node);

    DBUG_VOID_RETURN;
}

/**
 * @fn int LDot2Pos(int dot, dotinfo* info)
 *
 * transforms a given number of a dot (counted from the left) into its
 * position within the selection vector. Counting starts at 1.
 *
 * @return the position of the given dot within the selection vector
 */
static int
LDot2Pos (int dot, dotinfo *info)
{
    dotlist *dots = info->left;
    int cnt;

    DBUG_ENTER ("LDot2Pos");

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
static int
RDot2Pos (int dot, dotinfo *info)
{
    dotlist *dots = info->right;
    int cnt;

    DBUG_ENTER ("RDot2Pos");

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
static int
LIsDot (int dot, dotinfo *info)
{
    int result = 0;
    dotlist *list = info->left;

    DBUG_ENTER ("LIsDot");

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
static int
RIsDot (int dot, dotinfo *info)
{
    int result = 0;

    DBUG_ENTER ("RIsDot");

    result = LIsDot (info->selcnt - dot + 1, info);

    if (result != 0) {
        result = info->dotcnt - result + 1;
    }

    DBUG_RETURN (result);
}

/**
 * builds an id with a free name by calling TmpVarName. If
 * HD_USE_EXPLANATORY_NAMES is set, name is appended to the new id,
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

    DBUG_ENTER ("MakeTmpId");

#ifdef HD_USE_EXPLANATORY_NAMES
    result = TBmakeSpid (NULL, ILIBtmpVarName (name));
#else
    result = TBmakeSpid (NULL, ILIBtmpVar ());
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
#ifdef HD_USE_BUILTIN_TAKEDROP
    node *result;

    DBUG_ENTER ("BuildDrop");

    result = MAKE_BIN_PRF (F_drop_SxV, left,
                           MAKE_BIN_PRF (F_drop_SxV,
                                         MAKE_BIN_PRF (F_mul_SxS, TBmakeNum (-1), right),
                                         vector));

    DBUG_RETURN (result);
#else
    /*
     * use function Stdlib:drop instead
     */
    result = TCmakeSpap2 (ILIBstringCopy ("Stdlib"), ILIBstringCopy ("drop"), left,
                          TCmakeSpap2 (ILIBstringCopy ("Stdlib"), ILIBstringCopy ("drop"),
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
#ifdef HD_USE_BUILTIN_CONCAT
    node *result;

    DBUG_ENTER ("BuildConcat");

    result = MAKE_BIN_PRF (F_cat_VxV, a, b);

    DBUG_RETURN (result);
#else
    result = TCmakeSpap2 (ILIBstringCopy ("Stdlib"), ILIBstringCopy ("concat"), a, b);
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
    int cnt;
    int maxdot;
    node *result = NULL;

    DBUG_ENTER ("BuildLeftShape");

    if (info->tripledot == 0)
        maxdot = info->dotcnt;
    else
        maxdot = info->tripledot - 1;

    for (cnt = maxdot; cnt > 0; cnt--) {
        result = TBmakeExprs (MAKE_BIN_PRF (F_sel,
                                            TCmakeFlatArray (
                                              TBmakeExprs (TBmakeNum (LDot2Pos (cnt, info)
                                                                      - 1),
                                                           NULL)),
                                            TBmakePrf (F_shape,
                                                       TBmakeExprs (DUPdoDupTree (array),
                                                                    NULL))),
                              result);
    }

    /* do not create empty array */
    if (result != NULL) {
        result = TCmakeFlatArray (result);
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

    DBUG_ENTER ("BuildMiddleShape");

    shape = TBmakePrf (F_shape, TBmakeExprs (DUPdoDupTree (array), NULL));

    left = TBmakeNum (info->triplepos - 1);

    right = TBmakeNum (info->selcnt - info->triplepos);

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
    int cnt;
    int maxdot;
    node *result = NULL;

    DBUG_ENTER ("BuildRighthape");

    maxdot = info->dotcnt - info->tripledot;

    for (cnt = 1; cnt <= maxdot; cnt++) {
        result = TBmakeExprs (
          MAKE_BIN_PRF (
            F_sel,
            TCmakeFlatArray (TBmakeExprs (
              MAKE_BIN_PRF (
                F_sub_SxS,
                MAKE_BIN_PRF (F_sel, TCmakeFlatArray (TBmakeExprs (TBmakeNum (0), NULL)),
                              TBmakePrf (F_shape,
                                         TBmakeExprs (TBmakePrf (F_shape,
                                                                 TBmakeExprs (DUPdoDupTree (
                                                                                array),
                                                                              NULL)),
                                                      NULL))),
                TBmakeNum (RDot2Pos (cnt, info))),
              NULL)),
            TBmakePrf (F_shape, TBmakeExprs (DUPdoDupTree (array), NULL))),
          result);
    }

    /* do not create empty array */
    if (result != NULL) {
        result = TCmakeFlatArray (result);
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
 * assigns chain and inserted by HDap in front of the selection.
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

    DBUG_ENTER ("BuildShape");

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

    DBUG_ASSERT ((leftshape != NULL), "error building shape: the shape is empty!");

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
    int cnt;
    int maxcnt;
    node *result = NULL;

    DBUG_ENTER ("BuildLeftIndex");

    if (info->tripledot == 0) {
        maxcnt = info->selcnt;
    } else {
        maxcnt = info->triplepos - 1;
    }

    for (cnt = maxcnt; cnt > 0; cnt--) {
        if (LIsDot (cnt, info)) {
            /* Make selection iv[ldot(cnt)-1] */
            result = TBmakeExprs (MAKE_BIN_PRF (F_sel,
                                                TCmakeFlatArray (
                                                  TBmakeExprs (TBmakeNum (
                                                                 LIsDot (cnt, info) - 1),
                                                               NULL)),
                                                DUPdoDupTree (iv)),
                                  result);
        } else {
            result = TBmakeExprs (DUPdoDupTree (TCgetNthExpr (cnt, args)), result);
        }
    }

    result = TCmakeFlatArray (result);

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

    DBUG_ENTER ("BuildMiddleIndex");

    left = TBmakeNum (info->tripledot - 1);
    right = TBmakeNum (info->dotcnt - info->tripledot);

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
    int cnt;
    int maxcnt;
    node *result = NULL;

    DBUG_ENTER ("BuildRightIndex");

    maxcnt = info->selcnt - info->triplepos;

    for (cnt = 1; cnt <= maxcnt; cnt++) {
        if (RIsDot (cnt, info)) {
            /* Make selection iv[selcnt - rdot(cnt)] */
            result = TBmakeExprs (
              MAKE_BIN_PRF (
                F_sel,
                TCmakeFlatArray (TBmakeExprs (
                  MAKE_BIN_PRF (F_sub_SxS,
                                MAKE_BIN_PRF (F_sel,
                                              TCmakeFlatArray (
                                                TBmakeExprs (TBmakeNum (0), NULL)),
                                              TBmakePrf (F_shape,
                                                         TBmakeExprs (DUPdoDupTree (iv),
                                                                      NULL))),
                                TBmakeNum (RIsDot (cnt, info))),
                  NULL)),
                DUPdoDupTree (iv)),
              result);
        } else {
            result
              = TBmakeExprs (DUPdoDupTree (TCgetNthExpr (info->selcnt - cnt + 1, args)),
                             result);
        }
    }

    result = TCmakeFlatArray (result);

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

    DBUG_ENTER ("BuildIndex");

    if (info->triplepos != 1) {
        leftindex = BuildLeftIndex (args, iv, info);
        leftid = MakeTmpId ("left_index");
        BLOCK_INSTR (block)
          = TCappendAssign (BLOCK_INSTR (block),
                            MakeAssignLetNV (ILIBstringCopy (SPID_NAME (leftid)),
                                             leftindex));
    }

    if (info->triplepos != 0) {
        middleindex = BuildMiddleIndex (args, iv, info);
        middleid = MakeTmpId ("middle_index");
        BLOCK_INSTR (block)
          = TCappendAssign (BLOCK_INSTR (block),
                            MakeAssignLetNV (ILIBstringCopy (SPID_NAME (middleid)),
                                             middleindex));
    }

    if ((info->triplepos != 0) && (info->triplepos != info->selcnt)) {
        rightindex = BuildRightIndex (args, iv, info);
        rightid = MakeTmpId ("right_index");
        BLOCK_INSTR (block)
          = TCappendAssign (BLOCK_INSTR (block),
                            MakeAssignLetNV (ILIBstringCopy (SPID_NAME (rightid)),
                                             rightindex));
    }

    if (rightid != NULL) {
        node *tmpid = NULL;

        tmpid = MakeTmpId ("middle_and_right_index");

        BLOCK_INSTR (block)
          = TCappendAssign (BLOCK_INSTR (block),
                            MakeAssignLetNV (ILIBstringCopy (SPID_NAME (tmpid)),
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

            BLOCK_INSTR (block)
              = TCappendAssign (BLOCK_INSTR (block),
                                MakeAssignLetNV (ILIBstringCopy (SPID_NAME (tmpid)),
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

    DBUG_ENTER ("BuildDefaultWithloop");

    result
      = TBmakeWith (TBmakePart (NULL,
                                TBmakeWithid (TBmakeSpids (ILIBtmpVar (), NULL), NULL),
                                TBmakeGenerator (F_le, F_le, TBmakeDot (1), TBmakeDot (1),
                                                 NULL, NULL)),
                    TBmakeCode (MAKE_EMPTY_BLOCK (),
                                TBmakeExprs (TCmakeSpap1 (NULL, ILIBstringCopy ("zero"),
                                                          DUPdoDupTree (array)),
                                             NULL)),
                    TBmakeGenarray (shape, TCmakeSpap1 (NULL, ILIBstringCopy ("zero"),
                                                        DUPdoDupTree (array))));

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

    DBUG_ENTER ("BuildSelectionElementShape");

    shape = MAKE_BIN_PRF (F_drop_SxV, TBmakeNum (info->selcnt),
                          TBmakePrf (F_shape, TBmakeExprs (DUPdoDupTree (array), NULL)));

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

    DBUG_ENTER ("BuildSelectionDefault");

    if (info->triplepos == 0) {
        /* no tripledot, build default */

        node *shape = BuildSelectionElementShape (array, info);

        result = BuildDefaultWithloop (array, shape);
    } else {
        /* default is just a scalar */

        result = TBmakeExprs (DUPdoDupTree (array), NULL);
        result = TBmakeSpap (TBmakeSpid (NULL, ILIBstringCopy ("zero")), result);
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

    DBUG_ENTER ("BuildWithLoop");

    ap = TBmakeSpap (TBmakeSpid (NULL, ILIBstringCopy ("sel")),
                     TBmakeExprs (index, TBmakeExprs (DUPdoDupTree (array), NULL)));

    ids = TBmakeSpids (ILIBstringCopy (SPID_NAME (iv)), NULL);

    result = TBmakeWith (TBmakePart (NULL, TBmakeWithid (ids, NULL),
                                     TBmakeGenerator (F_le, F_le, TBmakeDot (1),
                                                      TBmakeDot (1), NULL, NULL)),
                         TBmakeCode (block, TBmakeExprs (ap, NULL)),
                         TBmakeGenarray (shape, BuildSelectionDefault (array, info)));

    CODE_USED (WITH_CODE (result))++;
    PART_CODE (WITH_PART (result)) = WITH_CODE (result);

    DBUG_RETURN (result);
}

/**
 * appends all ids within ids to the idtable appendto. The gathered
 * information is used to collect shapes of arrays these ids are
 * used on within a selection. New ids are appended on top in order
 * to hide lower ones.
 *
 * @param ids EXPRS node containing ids
 * @param appendto idtable to append the ids to (may be null)
 * @return new idtable containing found ids
 */
static idtable *
BuildIdTable (node *ids, idtable *appendto)
{
    idtable *result = appendto;

    DBUG_ENTER ("BuildIdTable");

    if (NODE_TYPE (ids) == N_exprs) {
        while (ids != NULL) {
            node *id = EXPRS_EXPR (ids);
            idtable *newtab = ILIBmalloc (sizeof (idtable));

            if (NODE_TYPE (id) != N_spid) {
                ERROR (global.linenum, ("found non-id as index in WL set notation"));

                /* we create a dummy entry within the idtable in order */
                /* to go on and search for further errors.             */
                newtab->id = ILIBstringCopy ("_non_id_expr");
            } else {
                newtab->id = ILIBstringCopy (SPID_NAME (id));
            }

            newtab->type = ID_scalar;
            newtab->shapes = NULL;
            newtab->next = result;
            result = newtab;
            ids = EXPRS_NEXT (ids);
        }
    } else if (NODE_TYPE (ids) == N_id)
#ifdef HD_SETWL_VECTOR
    {
        idtable *newtab = ILIBmalloc (sizeof (idtable));
        newtab->id = ILIBstringCopy (SPID_NAME (ids));
        newtab->type = ID_vector;
        newtab->shapes = NULL;
        newtab->next = result;
        result = newtab;
    }
#else
    {
        ABORT (global.linenum, ("vector as index in WL set notation not allowed"));
    }
#endif
    else {
        ABORT (global.linenum, ("malformed index vector in WL set notation"));
    }

    DBUG_RETURN (result);
}

/**
 * checks for id in idtable.
 *
 * @param id the id
 * @param ids the table
 * @return 1 if found, 0 otherwise
 */

static int
IdTableContains (char *id, idtable *ids)
{
    int result = 0;

    DBUG_ENTER ("IdTableContains");

    while (ids != NULL) {
        if (ILIBstringCompare (id, ids->id)) {
            result = 1;
            break;
        }
        ids = ids->next;
    }

    DBUG_RETURN (result);
}

/**
 * frees all elements in idtable until the element until is reached.
 * Used to clean up the idtable after the code of a lamination was
 * parsed. After a clean up until points to the top of the idtable.
 * The shapes stored in the idtable are not freed as they are reused
 * to build the withloop replacing the lamination.
 *
 * @param table table to clean up
 * @param until marker where to stop
 */
static void
FreeIdTable (idtable *table, idtable *until)
{
    DBUG_ENTER ("FreeIdTable");

    while (table != until) {
        idtable *next = table->next;

        /* free shape-chain but NOT shapes itself */
        while (table->shapes != NULL) {
            shpchain *next = table->shapes->next;
            ILIBfree (table->shapes);
            table->shapes = next;
        }

        /* free table */
        ILIBfree (table->id);
        ILIBfree (table);
        table = next;
    }

    DBUG_VOID_RETURN;
}

/**
 * scans a selection vector for occurancies of an id in ids within it
 * and stores the corresponding shape of the array the selection
 * is performed on in ids. Used to gather shape information to build
 * the withloop replacing the lamination.
 * The shape is taken from the corresponding element of array w.r.t.
 * occurencies of any tripledot.
 *
 * @param vector selection vector to scan
 * @param array array the selection operates on
 * @param arg_info info node containing ids to scan
 */
static void
ScanVector (node *vector, node **array, info *arg_info)
{
    int poscnt = 0;
    int tripledotflag = 0;
    int exprslen = TCcountExprs (vector);
    idtable *ids = INFO_HD_IDTABLE (arg_info);
    node *id = NULL;
    node *code = NULL;

    DBUG_ENTER ("ScanVector");

    while (vector != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (vector)) == N_spid) {
            idtable *handle = ids;

            while (handle != NULL) {
                if ((handle->type == ID_scalar)
                    && (ILIBstringCompare (handle->id,
                                           SPID_NAME (EXPRS_EXPR (vector))))) {
                    node *position = NULL;
                    node *shape = NULL;
                    shpchain *chain = NULL;

                    /* create id for array if not already done */
                    if (id == NULL) {
                        id = MakeTmpId ("letassign");
                    }

                    if (tripledotflag) {
                        position
                          = MAKE_BIN_PRF (F_sub_SxS,
                                          TBmakePrf (F_dim,
                                                     TBmakeExprs (DUPdoDupTree (id),
                                                                  NULL)),
                                          TBmakeNum (exprslen - poscnt));
                    } else {
                        position = TBmakeNum (poscnt);
                    }

                    shape
                      = MAKE_BIN_PRF (F_sel,
                                      TCmakeFlatArray (TBmakeExprs (position, NULL)),
                                      TBmakePrf (F_shape,
                                                 TBmakeExprs (DUPdoDupTree (id), NULL)));
                    chain = ILIBmalloc (sizeof (shpchain));

                    chain->shape = shape;
                    chain->next = handle->shapes;
                    handle->shapes = chain;

                    /* insert assign into chain */
                    /* if not already done */
                    if (code == NULL) {
                        code = MakeAssignLetNV (ILIBstringCopy (SPID_NAME (id)), *array);
                        INFO_HD_SETASSIGNS (arg_info)
                          = TCappendAssign (INFO_HD_SETASSIGNS (arg_info), code);
                        *array = id;
                    }
                    break;
                }

                handle = handle->next;
            }
        }

        /* check for occurence of '...' */

        if ((NODE_TYPE (EXPRS_EXPR (vector)) == N_dot)
            && (DOT_NUM (EXPRS_EXPR (vector)) == 3)) {
            tripledotflag = 1;
        }

        poscnt++;
        vector = EXPRS_NEXT (vector);
    }

    DBUG_VOID_RETURN;
}

#ifdef HD_SETWL_VECTOR
/**
 * scans a selection vector given as a single vector variable. If it
 * exists within ids, the corresponding shape is stored in ids.
 *
 * @param id selection vector id
 * @param array array the selection operates on
 * @param ids idtable structure
 */
static void
ScanId (node *id, node **array, info *arg_info)
{
    idtable *ids = INFO_HD_IDTABLE (arg_info);
    DBUG_ENTER ("ScanId");

    while (ids != NULL) {
        if ((ids->type == ID_vector) && (ILIBstringCompare (ids->id, SPID_NAME (id)))) {
            node *id = MakeTmpId ("setassign");
            node *code = MakeAssignLetNV (ILIBstringCopy (SPID_NAME (id)), *array);
            node *shape = TBmakePrf (F_shape, TBmakeExprs (DUPdoDupTree (id), NULL));
            shpchain *chain = ILIBmalloc (sizeof (shpchain));

            chain->shape = shape;
            chain->next = ids->shapes;
            ids->shapes = chain;

            INFO_HD_SETASSIGNS (arg_info)
              = TCappendAssign (INFO_HD_SETASSIGNS (arg_info), code);
            *array = id;

            break;
        }

        ids = ids->next;
    }

    DBUG_VOID_RETURN;
}

/**
 * builds runtime code that calculates the minimum of all shapes
 * found in vectors. This is only used if the selection vector
 * was a single indentifier.
 *
 * @param vectors shapechain containing vectors to build the minimum of
 * @return sac code representing the minimum of all given shapes
 */
static node *
BuildShapeVectorMin (shpchain *vectors)
{
    node *result = NULL;
    node *index = MakeTmpId ("index_min");
    node *shape = NULL;
    node *expr = NULL;
    node *indexids = NULL;

    DBUG_ENTER ("BuildVectorMin");

    indexids = TBmakeSpids (ILIBstringCopy (SPID_NAME (index)), NULL);

    shape = TBmakePrf (F_shape, TBmakeExprs (vectors->shape, NULL));

    expr = MAKE_BIN_PRF (F_sel, DUPdoDupTree (index), vectors->shape);

    vectors = vectors->next;

    while (vectors != NULL) {
        expr = MAKE_BIN_PRF (F_min,
                             MAKE_BIN_PRF (F_sel, DUPdoDupTree (index), vectors->shape),
                             expr);
        vectors = vectors->next;
    }

    result = TBmakeWith (TBmakePart (NULL, TBmakeWithid (indexids, NULL),
                                     TBmakeGenerator (F_le, F_le, TBmakeDot (1),
                                                      TBmakeDot (1), NULL, NULL)),
                         TBmakeCode (MAKE_EMPTY_BLOCK (), TBmakeExprs (expr, NULL)),
                         TBmakeGenarray (shape, TBmakeNum (0)));

    CODE_USED (WITH_CODE (result))++;
    PART_CODE (WITH_PART (result)) = WITH_CODE (result);

    FREEdoFreeTree (index);

    DBUG_RETURN (result);
}

#endif

/**
 * builds the shape for the withloop replacing the lamination. For each
 * identifier within the lamination vector the shape is built as the
 * minimum of all shapes of arrays the identifier is used with within a
 * selection. If the lamination vector is given as a single vector,
 * BuildMinShapeVector is used instead of primitive functions.
 *
 * @param table idtable structure
 * @param end first identifier within idtable not belonging to this lamination
 * @return sac code representing the shape vector
 */
static node *
BuildWLShape (idtable *table, idtable *end)
{
    node *result = NULL;

    DBUG_ENTER ("BuildWLShape");

    if (table->type == ID_scalar) {
        while (table != end) {
            node *shape = NULL;
            shpchain *handle = table->shapes;

            if (handle == NULL) {
                ERROR (global.linenum, ("no shape information found for %s", table->id));
            } else {
                shape = handle->shape;
                handle = handle->next;

                while (handle != NULL) {
                    shape = MAKE_BIN_PRF (F_min, shape, handle->shape);
                    handle = handle->next;
                }
            }

            result = TBmakeExprs (shape, result);
            table = table->next;
        }

        result = TCmakeFlatArray (result);
    }
#ifdef HD_SETWL_VECTOR
    else if (table->type == ID_vector) {
        if (table->shapes == NULL) {
            ERROR (global.linenum, ("no shape information found for %s", table->id));
        } else {
            /*
             * do not build min-WL if there is only one shape
             */

            if (table->shapes->next == NULL) {
                result = table->shapes->shape;
            } else {
                result = BuildShapeVectorMin (table->shapes);
            }
        }
    }
#endif

    DBUG_RETURN (result);
}

/**
 * converts ID nodes within an EXPR chain to an Ids chain.
 *
 * @param exprs EXPR node chain containing ID nodes
 * @return ids chain corresponding to the ID nodes within the EXPR chain
 */
static node *
Exprs2Ids (node *exprs)
{
    node *result = NULL;
    node *handle = NULL;

    DBUG_ENTER ("Exprs2Ids");

    while (exprs != NULL) {
        node *newid = NULL;

        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_spid) {
            newid = TBmakeSpids (ILIBstringCopy (SPID_NAME (EXPRS_EXPR (exprs))), NULL);
        } else {
            /* create dummy id in order to go on until end of phase */
            ERROR (global.linenum, ("found non-id expression in index vector"));
            newid = TBmakeSpids (ILIBstringCopy ("unknown_id"), NULL);
        }

        if (handle == NULL) {
            result = newid;
            handle = newid;
        } else {
            SPIDS_NEXT (handle) = newid;
            handle = newid;
        }

        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * checks for any occurencies of a dot symbol within a set notation
 * selection vector.
 *
 * @param ids EXPRS node containing ids
 * @return number of dots found
 *****************************************************************************/
static int
CountDotsInVector (node *ids)
{
    int result = 0;

    DBUG_ENTER ("CountDotsInVector");

    if (NODE_TYPE (ids) != N_exprs) {
        result = 0;
    } else {
        while (ids != NULL) {
            if (NODE_TYPE (EXPRS_EXPR (ids)) == N_dot)
                result++;
            ids = EXPRS_NEXT (ids);
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * removes all occurences of a dot symbol in the given exprs chain of ids.
 *
 * @param ids exprs chain containing ids and dots
 * @return exprs chain without any dots
 ****************************************************************************/
static node *
RemoveDotsFromVector (node *ids)
{
    node *result;
    node *temp;

    DBUG_ENTER ("RemoveDotsFromVector");

    result = DUPdoDupTree (ids);

    /* first remove leading dots */
    while (result != NULL && NODE_TYPE (EXPRS_EXPR (result)) == N_dot) {
        temp = result;
        result = EXPRS_NEXT (result);
        FREEdoFreeNode (temp);
    }

    /* now remove inner dots */
    temp = result;
    while (EXPRS_NEXT (temp) != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (temp))) == N_dot) {
            node *remove = EXPRS_NEXT (temp);
            EXPRS_NEXT (temp) = EXPRS_NEXT (EXPRS_NEXT (temp));
            FREEdoFreeNode (remove);
        } else {
            temp = EXPRS_NEXT (temp);
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * constructs the permutaed index vector to map the results of a set
 * notation to the set notation with dots.
 *
 * @param ids the indexvector of the set notation
 * @param vect the indexvector of the withloop used for permutation
 ****************************************************************************/
static node *
BuildPermutatedVector (node *ids, node *vect)
{
    node *result = NULL;
    node *trav = ids;
    node *next = NULL;
    int pos = 0;

    DBUG_ENTER ("BuildPermutatedVector");

    /* first scan for non-dot entries */

    while (trav != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (trav)) != N_dot) {
            node *entry
              = MAKE_BIN_PRF (F_sel,
                              TCmakeFlatArray (TBmakeExprs (TBmakeNum (pos), NULL)),
                              DUPdoDupTree (vect));

            if (result == NULL) {
                result = TBmakeExprs (entry, NULL);
                next = result;
            } else {
                EXPRS_NEXT (next) = TBmakeExprs (entry, NULL);
                next = EXPRS_NEXT (next);
            }
        }

        trav = EXPRS_NEXT (trav);
        pos++;
    }

    /* now the same for dots */

    trav = ids;
    pos = 0;

    while (trav != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (trav)) == N_dot) {
            node *entry
              = MAKE_BIN_PRF (F_sel,
                              TCmakeFlatArray (TBmakeExprs (TBmakeNum (pos), NULL)),
                              DUPdoDupTree (vect));

            if (result == NULL) {
                result = TBmakeExprs (entry, NULL);
                next = result;
            } else {
                EXPRS_NEXT (next) = TBmakeExprs (entry, NULL);
                next = EXPRS_NEXT (next);
            }
        }

        trav = EXPRS_NEXT (trav);
        pos++;
    }

    result = TCmakeFlatArray (result);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * constructs the default value of a selection within a setnotation.
 * A default is built for any selection as for the default value, the real
 * value is of no importance and the generated withloops should be removed
 * by the optimizations for all static arrays.
 *
 * @param array AST of the array the selection occurs on
 * @param info dotinfo strcuture of the selection
 * @return AST of a default value
 *****************************************************************************/

static node *
BuildSetNotationDefault (node *array, dotinfo *info)
{
    node *result = NULL;
    node *shape = NULL;

    DBUG_ENTER ("BuildSetNotationDefault");

    if (info->dotcnt == 0) {
        /* no dots at all */

        shape
          = MAKE_BIN_PRF (F_drop_SxV, TBmakeNum (info->selcnt),
                          TBmakePrf (F_shape, TBmakeExprs (DUPdoDupTree (array), NULL)));
    } else if (info->triplepos == 0) {
        /* no tripledot but dots, build default */

        node *leftshape = BuildLeftShape (array, info);
        shape = BuildSelectionElementShape (array, info);

        /* concatenate if not empty */
        if (leftshape != NULL) {
            shape = BuildConcat (leftshape, shape);
        }
    } else {
        /* contains tripledot, so build the full shape */

        shape = BuildShape (array, info);
    }

    result = BuildDefaultWithloop (array, shape);

    DBUG_RETURN (result);
}

/**
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
HDdoEliminateSelDots (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("HDdoEliminateSelDots");

    arg_info = MakeInfo ();
    INFO_HD_TRAVSTATE (arg_info) = HD_sel;

    TRAVpush (TR_hd);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    ABORT_ON_ERROR;

    DBUG_RETURN (arg_node);
}

/**
 * hook for with nodes. Needed to normalize dots within withloop
 * generators. At first, the withop node is traversed in order to
 * get the result shape of the withloop, needed to calculate the
 * replacements. The shape is stored in the arg_info structure.
 * Afterwards the rest is traversed in order to replace the dots.
 *
 * @param arg_node current node within the AST
 * @param arg_info info node
 * @result transformed AST
 */

node *
HDwith (node *arg_node, info *arg_info)
{
    /* INFO_HD_DOTSHAPE is used for '.'-substitution in WLgenerators */
    /* in order to handle nested WLs correct, olddotshape stores not */
    /* processed shapes until this (maybe inner) WL is processed.    */

    node *olddotshape = INFO_HD_DOTSHAPE (arg_info);

    DBUG_ENTER ("HDwith");

    /*
     * by default (TravSons), withop would be traversed last, but
     * some information from withop is needed in order to traverse
     * the rest, so the withop is handeled first.
     */

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_HD_DOTSHAPE (arg_info) = olddotshape;

    DBUG_RETURN (arg_node);
}

/**
 * scans the withop node for the shape of the current withloop and stores
 * it within the arg_info node. For fold withloops a null is stored as
 * there is no shape information in fold withop nodes.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HDgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HDgenarray");

    if (INFO_HD_TRAVSTATE (arg_info) == HD_sel) {
        INFO_HD_DOTSHAPE (arg_info) = DUPdoDupTree (GENARRAY_SHAPE (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * scans the withop node for the shape of the current withloop and stores
 * it within the arg_info node. For fold withloops a null is stored as
 * there is no shape information in fold withop nodes.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HDmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HDmodarray");

    if (INFO_HD_TRAVSTATE (arg_info) == HD_sel) {
        INFO_HD_DOTSHAPE (arg_info)
          = TCmakePrf1 (F_shape, DUPdoDupTree (MODARRAY_ARRAY (arg_node)));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * scans the withop node for the shape of the current withloop and stores
 * it within the arg_info node. For fold withloops a null is stored as
 * there is no shape information in fold withop nodes.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HDfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HDfold");

    if (INFO_HD_TRAVSTATE (arg_info) == HD_sel) {
        INFO_HD_DOTSHAPE (arg_info) = NULL;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * removes the DOTINFO within the info structure, as it is no more needed
 * now.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HDpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HDpart");

    arg_node = TRAVcont (arg_node, arg_info);

    /**
     * the shape info in INFO_HD_DOTSHAPE(arg_info) has been used now!
     * Note here, that it may not be consumed in HDgenerator, as there may exist
     * more than one generators for a single WL now!
     */
    if (INFO_HD_DOTSHAPE (arg_info) != NULL) {
        INFO_HD_DOTSHAPE (arg_info) = FREEdoFreeTree (INFO_HD_DOTSHAPE (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/**
 * replaces dots in generators and normalizes the generator.
 * A dot as left boundary is replaced by 0 * shape, a right boundary
 * dot is replaced by shape. the left comparison operator is normalized
 * to <= by adding 1 to the left boundary if necessary, the right
 * boundary is normalized to < by decreasing the right boundary by 1 if
 * necessary.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return transformed AST
 */
node *
HDgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HDGenerator");

    if (INFO_HD_TRAVSTATE (arg_info) == HD_sel) {
        /*
         * Dots are replaced by the "shape" expressions, that are imported via
         * INFO_HD_DOTSHAPE( arg_info)    (cf. HDWithop),
         * and the bounds are adjusted so that the operator can be
         * "normalized" to:   bound1 <= iv = [...] < bound2     .
         */

        if ((INFO_HD_DOTSHAPE (arg_info) == NULL)
            && (DOT_ISSINGLE (GENERATOR_BOUND1 (arg_node))
                || DOT_ISSINGLE (GENERATOR_BOUND2 (arg_node)))) {
            ABORT (global.linenum, ("dot notation is not allowed in fold with loops"));
        }

        if (DOT_ISSINGLE (GENERATOR_BOUND1 (arg_node))) {
            /* replace "." by "0 * shp" */
            GENERATOR_BOUND1 (arg_node) = FREEdoFreeTree (GENERATOR_BOUND1 (arg_node));
            GENERATOR_BOUND1 (arg_node)
              = TCmakePrf2 (F_mul_SxA, TBmakeNum (0),
                            DUPdoDupTree (INFO_HD_DOTSHAPE (arg_info)));
        }

        if (GENERATOR_OP1 (arg_node) == F_lt) {
            /* make <= from < and add 1 to bound */
            GENERATOR_OP1 (arg_node) = F_le;
            GENERATOR_BOUND1 (arg_node)
              = TCmakePrf2 (F_add_AxS, GENERATOR_BOUND1 (arg_node), TBmakeNum (1));
        }

        if (DOT_ISSINGLE (GENERATOR_BOUND2 (arg_node))) {
            if (GENERATOR_OP2 (arg_node) == F_le) {
                /* make < from <= and replace "." by "shp"  */
                GENERATOR_OP2 (arg_node) = F_lt;
                GENERATOR_BOUND2 (arg_node)
                  = FREEdoFreeTree (GENERATOR_BOUND2 (arg_node));
                GENERATOR_BOUND2 (arg_node) = DUPdoDupTree (INFO_HD_DOTSHAPE (arg_info));
            } else {
                /* replace "." by "shp - 1"  */
                GENERATOR_BOUND2 (arg_node)
                  = FREEdoFreeTree (GENERATOR_BOUND2 (arg_node));
                GENERATOR_BOUND2 (arg_node)
                  = TCmakePrf2 (F_sub_AxS, DUPdoDupTree (INFO_HD_DOTSHAPE (arg_info)),
                                TBmakeNum (1));
            }
        } else {
            if (GENERATOR_OP2 (arg_node) == F_le) {
                /* make < from <= and add 1 to bound */
                GENERATOR_OP2 (arg_node) = F_lt;
                GENERATOR_BOUND2 (arg_node)
                  = TCmakePrf2 (F_add_AxS, GENERATOR_BOUND2 (arg_node), TBmakeNum (1));
            }
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * hook for dot nodes used to generate a warning if an unhandled dot is
 * found.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HDdot (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HDdot");

    if (INFO_HD_TRAVSTATE (arg_info) == HD_sel) {
        if (DOT_NUM (arg_node) == 1) {
            ERROR (global.linenum, ("'.' not allowed here."));
        } else {
            ERROR (global.linenum, ("'...' not allowed here."));
        }
    }

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
HDspap (node *arg_node, info *arg_info)
{
    node *result = arg_node;

    DBUG_ENTER ("HDspap");

    /* only sel statements are of interest here, so just return */
    /* on anything else                                         */
    /* besides ARG1 must be an array. because otherwise there   */
    /* is no possibility to find any dot...                     */

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_sel)
        && (ILIBstringCompare (SPAP_NAME (arg_node), "sel"))
        && (SPAP_MOD (arg_node) == NULL) && (NODE_TYPE (AP_ARG1 (arg_node)) == N_array)) {
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

            FREEdoFreeTree (arg_node);
            FREEdoFreeNode (iv);
        }

        FreeDotInfo (info);
    }

    /* if in HD_scan mode, scan for shapes */

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_scan)
        && (ILIBstringCompare (SPAP_NAME (arg_node), "sel"))
        && (SPAP_MOD (arg_node) == NULL)) {
        if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_array) {
            ScanVector (ARRAY_AELEMS (SPAP_ARG1 (arg_node)), &SPAP_ARG2 (arg_node),
                        arg_info);
        }
#ifdef HD_SETWL_VECTOR
        else if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_id) {
            ScanId (SPAP_ARG1 (arg_node), &SPAP_ARG2 (arg_node), arg_info);
        }
#endif
    }

    /* if in HD_default mode, rebuild selection */

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_default)
        && (ILIBstringCompare (SPAP_NAME (arg_node), "sel"))
        && (SPAP_MOD (arg_node) == NULL) && (NODE_TYPE (AP_ARG1 (arg_node)) == N_array))

    {
        dotinfo *info = MakeDotInfo (ARRAY_AELEMS (SPAP_ARG1 (arg_node)));

        node *defexpr = BuildSetNotationDefault (SPAP_ARG2 (arg_node), info);

        FREEdoFreeTree (result);
        FreeDotInfo (info);

        result = defexpr;
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

/**
 * Used to scan selections for ids found in a prior lamination.
 * Depending on the type of the selection vector, ScanId or ScanVector
 * is called.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HDprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HDprf");

    /* if in HD_scan mode, scan for shapes */

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_scan) && (PRF_PRF (arg_node) == F_sel)) {
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array) {
            ScanVector (ARRAY_AELEMS (PRF_ARG1 (arg_node)), &PRF_ARG2 (arg_node),
                        arg_info);
        }
#ifdef HD_SETWL_VECTOR
        else if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id) {
            ScanId (PRF_ARG1 (arg_node), &PRF_ARG2 (arg_node), arg_info);
        }
#endif
    }

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_default) && (PRF_PRF (arg_node) == F_sel)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array)) {
        dotinfo *info = MakeDotInfo (ARRAY_AELEMS (PRF_ARG1 (arg_node)));

        node *defexpr = BuildSetNotationDefault (PRF_ARG2 (arg_node), info);

        FREEdoFreeTree (arg_node);
        FreeDotInfo (info);

        arg_node = defexpr;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * hook used to insert pending assings created by BuildShape prior
 * to the axis control selection. The code is parsed and afterwards any pending
 * assignments are inserted prior to this node.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @result current node of the AST and inserted assigns
 */
node *
HDassign (node *arg_node, info *arg_info)
{
    node *result = arg_node;
    node *oldassigns = INFO_HD_ASSIGNS (arg_info);
    node *oldsetassigns = INFO_HD_SETASSIGNS (arg_info);
    DBUG_ENTER ("HDassign");

    /* first traverse sons with a fresh assigns-chain */

    INFO_HD_ASSIGNS (arg_info) = NULL;
    INFO_HD_SETASSIGNS (arg_info) = TBmakeEmpty ();
    result = TRAVcont (arg_node, arg_info);

    /* check for assigns that are to be inserted */

    if (INFO_HD_ASSIGNS (arg_info) != NULL) {
        if (NODE_TYPE (INFO_HD_ASSIGNS (arg_info)) == N_empty) {
            /* there is nothing to insert here */
            FREEdoFreeTree (INFO_HD_ASSIGNS (arg_info));
            INFO_HD_ASSIGNS (arg_info) = NULL;
        } else {
            /* we need to traverse them once more in order */
            /* to do dot elemination in wl generators      */
            /* and simplification of boundaries            */
            node *assigns = TRAVdo (INFO_HD_ASSIGNS (arg_info), arg_info);
            result = TCappendAssign (assigns, result);
        }
    }

    /* and again for set assigns */

    if (INFO_HD_SETASSIGNS (arg_info) != NULL) {
        if (NODE_TYPE (INFO_HD_SETASSIGNS (arg_info)) == N_empty) {
            /* there is nothing to insert here */
            FREEdoFreeTree (INFO_HD_SETASSIGNS (arg_info));
            INFO_HD_SETASSIGNS (arg_info) = NULL;
        } else {
            node *assigns = NULL;

            /* traverse until no new assigns to process */
            while ((INFO_HD_SETASSIGNS (arg_info) != NULL)
                   && (NODE_TYPE (INFO_HD_SETASSIGNS (arg_info)) != N_empty)) {
                node *temp = INFO_HD_SETASSIGNS (arg_info);
                INFO_HD_SETASSIGNS (arg_info) = TBmakeEmpty ();
                ;
                temp = TRAVdo (temp, arg_info);
                if (assigns == NULL)
                    assigns = temp;
                else
                    assigns = TCappendAssign (assigns, temp);
            }

            /* append everything */
            result = TCappendAssign (assigns, result);

            /* free last N_empty node */
            FREEdoFreeTree (INFO_HD_SETASSIGNS (arg_info));
        }
    }

    /* reinstall old assigns-chain */

    INFO_HD_ASSIGNS (arg_info) = oldassigns;
    INFO_HD_SETASSIGNS (arg_info) = oldsetassigns;

    DBUG_RETURN (result);
}

/**
 * hook to handle any lamination operators. The inner expression is parsed for
 * occuriencies of ids in the lamination vector. Afterwards the lamination
 * operator is replaced by the corresponding withloop and the inner expression
 * is parsed. To distinguish between parsing for ids and normal dot
 * replacement, an entry within the info node is used.
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
HDsetwl (node *arg_node, info *arg_info)
{
    node *result = NULL;
    travstate oldstate = INFO_HD_TRAVSTATE (arg_info);
    idtable *oldtable = INFO_HD_IDTABLE (arg_info);
    node *shape = NULL;
    node *defexpr = NULL;
    node *ids = NULL;
    int dotcnt;

    DBUG_ENTER ("HDsetwl");

    /* maybe the set-index contains some dots */
    dotcnt = CountDotsInVector (SETWL_VEC (arg_node));

    /* build vector without dots */
    if (dotcnt == 0) {
        ids = DUPdoDupTree (SETWL_VEC (arg_node));
    } else {
        ids = RemoveDotsFromVector (SETWL_VEC (arg_node));
    }

    /* from here on, it is a set notation without any dots */

    INFO_HD_TRAVSTATE (arg_info) = HD_scan;
    INFO_HD_IDTABLE (arg_info) = BuildIdTable (ids, INFO_HD_IDTABLE (arg_info));

    arg_node = TRAVcont (arg_node, arg_info);

    shape = BuildWLShape (INFO_HD_IDTABLE (arg_info), oldtable);

    if (INFO_HD_IDTABLE (arg_info)->type == ID_scalar) {
        result
          = TBmakeWith (TBmakePart (NULL, TBmakeWithid (NULL, Exprs2Ids (ids)),
                                    TBmakeGenerator (F_le, F_le, TBmakeDot (1),
                                                     TBmakeDot (1), NULL, NULL)),
                        TBmakeCode (MAKE_EMPTY_BLOCK (),
                                    TBmakeExprs (DUPdoDupTree (SETWL_EXPR (arg_node)),
                                                 NULL)),
                        TBmakeGenarray (shape, NULL));
    }
#ifdef HD_SETWL_VECTOR
    else {
        node *newids = MakeSpids (ILIBstringCopy (SPID_NAME (ids)), NULL);

        result
          = TBmakeWith (TBmakePart (TBmakeWithid (newids, NULL),
                                    TBmakeGenerator (TBmakeDot (1), TBmakeDot (1), F_le,
                                                     F_le, NULL, NULL),
                                    NULL),
                        TBmakeCode (MAKE_EMPTY_BLOCK (),
                                    TBmakeExprs (DUPdoDupTree (SETWL_EXPR (arg_node)),
                                                 NULL)),
                        TBmakeGenarray (shape, NULL));
    }
#endif

    /* build a default value for the withloop */
    defexpr = DUPdoDupTree (SETWL_EXPR (arg_node));
    INFO_HD_TRAVSTATE (arg_info) = HD_default;
    defexpr = TRAVdo (defexpr, arg_info);
    INFO_HD_TRAVSTATE (arg_info) = HD_scan;

    CODE_USED (WITH_CODE (result))++;
    PART_CODE (WITH_PART (result)) = WITH_CODE (result);
    GENARRAY_DEFAULT (WITH_WITHOP (result)) = defexpr;

    /* check whether we had some dots in order to create */
    /* code to handle the permutation                    */

    if (dotcnt != 0) {
        node *setid = MakeTmpId ("setwithoutdots");
        node *withid = MakeTmpId ("permutationiv");
        node *selvector = BuildPermutatedVector (SETWL_VEC (arg_node), withid);
        node *shape = TBmakePrf (F_shape, TBmakeExprs (DUPdoDupTree (setid), NULL));
        node *shapevector = BuildPermutatedVector (SETWL_VEC (arg_node), shape);
        node *defexpr = NULL;
        node *defshape = NULL;
        node *withids = TBmakeSpids (ILIBstringCopy (SPID_NAME (withid)), NULL);

        /* put the intermediate result into the assigns chain */

        INFO_HD_ASSIGNS (arg_info)
          = TCappendAssign (INFO_HD_ASSIGNS (arg_info),
                            MakeAssignLetNV (ILIBstringCopy (SPID_NAME (setid)), result));

        /* create permutation code */

        /* build the default value */
        defshape
          = MAKE_BIN_PRF (F_drop_SxV, TBmakeNum (TCcountExprs (ids)),
                          TBmakePrf (F_shape, TBmakeExprs (DUPdoDupTree (setid), NULL)));

        defexpr = BuildDefaultWithloop (setid, defshape);

        result
          = TBmakeWith (TBmakePart (NULL, TBmakeWithid (withids, NULL),
                                    TBmakeGenerator (F_le, F_le, TBmakeDot (1),
                                                     TBmakeDot (1), NULL, NULL)),
                        TBmakeCode (MAKE_EMPTY_BLOCK (),
                                    TBmakeExprs (MAKE_BIN_PRF (F_sel, selvector, setid),
                                                 NULL)),
                        TBmakeGenarray (shapevector, defexpr));

        CODE_USED (WITH_CODE (result))++;
        PART_CODE (WITH_PART (result)) = WITH_CODE (result);
    }

    FREEdoFreeTree (arg_node);
    FREEdoFreeTree (ids);

    FreeIdTable (INFO_HD_IDTABLE (arg_info), oldtable);

    INFO_HD_IDTABLE (arg_info) = oldtable;
    INFO_HD_TRAVSTATE (arg_info) = oldstate;

    result = TRAVdo (result, arg_info);

    DBUG_RETURN (result);
}

node *
HDspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("HDspid");

    if (INFO_HD_TRAVSTATE (arg_info) == HD_default) {
        if (IdTableContains (SPID_NAME (arg_node), INFO_HD_IDTABLE (arg_info))) {
            WARN (global.linenum,
                  ("cannot infer default value for %s in set notation, using 0",
                   SPID_NAME (arg_node)));

            FREEdoFreeTree (arg_node);
            arg_node = TBmakeNum (0);
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
