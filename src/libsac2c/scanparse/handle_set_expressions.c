#include "handle_set_expressions.h"
#include "traverse.h"

#define DBUG_PREFIX "HSE"
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
 * @file handle_set_expressions.c
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
#define HSE_USE_EXPLANATORY_NAMES

/**
 * set this to enable support for vectors as index of a set notation.
 *
 * NOTE: The implementation is far from complete:
 *       -no support of partial selection on arrays of different
 *        dimenionality
 */
#define HSE_SETWL_VECTOR

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
 * traversal in HSE_scan mode.
 */
typedef enum TRAVSTATE { HSE_none, HSE_scan, HSE_default } travstate;
typedef enum IDTYPE { ID_notfound = 0, ID_vector = 1, ID_scalar = 2 } idtype;

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
 * TRAVSTATE:   this field is used to determine the current traversalmode
 *              HSE_scan in shape scanning mode
 *              HSE_default to build default values for withloops
 * IDTABLE:     used to reference the current idtable.
 */

/* INFO structure */
struct INFO {
    bool outside;
    bool lastpart;
    bool genref;
    node *vec;
    node *copy_from;
    node *part;
    node *code;
    node *withop;
    struct INFO *next;
};

/* access macros */
#define INFO_HSE_OUTSIDE(n) ((n)->outside)
#define INFO_HSE_LASTPART(n) ((n)->lastpart)
#define INFO_HSE_GENREF(n) ((n)->genref)
#define INFO_HSE_VEC(n) ((n)->vec)
#define INFO_HSE_COPY_FROM(n) ((n)->copy_from)
#define INFO_HSE_PART(n) ((n)->part)
#define INFO_HSE_CODE(n) ((n)->code)
#define INFO_HSE_WITHOP(n) ((n)->withop)
#define INFO_HSE_NEXT(n) ((n)->next)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (info * previous)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_HSE_OUTSIDE (result) = TRUE;
    INFO_HSE_LASTPART (result) = FALSE;
    INFO_HSE_GENREF (result) = FALSE;
    INFO_HSE_VEC (result) = NULL;
    INFO_HSE_COPY_FROM (result) = NULL;
    INFO_HSE_PART (result) = NULL;
    INFO_HSE_CODE (result) = NULL;
    INFO_HSE_WITHOP (result) = NULL;
    INFO_HSE_NEXT (result) = previous;

    DBUG_RETURN (result);
}

/**
 * frees an info structure.
 *
 * @param info the info structure to free
 */
static info *
FreeInfo (info *arg_info)
{
    info *old;
    DBUG_ENTER ();

    old = INFO_HSE_NEXT (arg_info);
    arg_info = MEMfree (arg_info);

    DBUG_RETURN (old);
}

static node *
BuildDefault (node *expr)
{
    DBUG_ENTER ();
    DBUG_RETURN (expr);
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

    DBUG_ENTER ();

    while (exprs != NULL) {
        node *newid = NULL;

        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_spid) {
            newid = TBmakeSpids (STRcpy (SPID_NAME (EXPRS_EXPR (exprs))), NULL);
        } else {
            /* create dummy id in order to go on until end of phase */
            CTIerrorLine (global.linenum, "Found non-id expression in index vector");
            newid = TBmakeSpids (STRcpy ("unknown_id"), NULL);
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


/**
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
HSEdoEliminateSetExpressions (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

DBUG_EXECUTE(
    arg_info = MakeInfo (NULL);

    TRAVpush (TR_hse);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info););

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
HSEspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
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
HSEprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
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
HSEsetwl (node *arg_node, info *arg_info)
{
    node *code, *part;
    info *oldinfo = NULL;
    DBUG_ENTER ();

    if (INFO_HSE_OUTSIDE (arg_info)) {
        DBUG_PRINT ("looking at Set-Expression in line %d:", global.linenum);
        oldinfo = arg_info;
        arg_info = MakeInfo (oldinfo);
    } else {
        DBUG_PRINT ("next partition of Set-Expression in line %d:", global.linenum);
        INFO_HSE_OUTSIDE (arg_info) = TRUE;
    }

    INFO_HSE_LASTPART (arg_info) = (SETWL_NEXT (arg_node) == NULL);
    INFO_HSE_VEC (arg_info) = SETWL_VEC (arg_node);

    /* traverse the generator (may contain setwls!) */
    SETWL_GENERATOR (arg_node) = TRAVdo (SETWL_GENERATOR (arg_node), arg_info);

    /* traverse the expression (may contain setwls!)
     * This traversal also infers INFO_HSE_GENREF and INFO_HSE_COPY_FROM
     * in case INFO_HSE_LASTPART is TRUE!
     */
    SETWL_EXPR (arg_node) = TRAVdo (SETWL_EXPR (arg_node), arg_info);

    /* We build the WLs bottom up! */
    INFO_HSE_OUTSIDE (arg_info) = FALSE;
    SETWL_NEXT (arg_node) = TRAVopt (SETWL_NEXT (arg_node), arg_info);

    /* Now, we add a new partition to arg_info, iff we are 
     *   not the last partition
     *   or, in case we are the last part, we refer to the generator variable(s)
     *   and do so in a way different from a copying expression (ie a[iv])
     */
    if (!INFO_HSE_LASTPART (arg_info)
         || (INFO_HSE_GENREF (arg_info) && (INFO_HSE_COPY_FROM (arg_info) == NULL))) {

        code = TBmakeCode ( MAKE_EMPTY_BLOCK (),
                            TBmakeExprs (DUPdoDupTree (SETWL_EXPR (arg_node)),
                                         NULL));
        CODE_USED (code)++;
        CODE_NEXT (code) = INFO_HSE_CODE (arg_info);
        INFO_HSE_CODE (arg_info) = code;

        if (NODE_TYPE (SETWL_VEC (arg_node)) == N_exprs) {
            part = TBmakePart (code,
                               TBmakeWithid (NULL, Exprs2Ids (DUPdoDupTree (SETWL_VEC (arg_node)))),
                               DUPdoDupTree (SETWL_GENERATOR (arg_node)));
        } else {
            part = TBmakePart (code,
                               TBmakeWithid (TBmakeSpids (STRcpy (SPID_NAME (SETWL_VEC (arg_node))), NULL), NULL),
                               DUPdoDupTree (SETWL_GENERATOR (arg_node)));
        }
        PART_NEXT (part) = INFO_HSE_PART (arg_info);
        INFO_HSE_PART (arg_info) = part;

    }

    /* if we are the last partition, we generate the operator part now: */
    if (INFO_HSE_LASTPART (arg_info)) {
        if (INFO_HSE_GENREF (arg_info)) {
            if (INFO_HSE_COPY_FROM (arg_info) != NULL) {
                /* we can generate a modarray WL and elide the last partition */
                INFO_HSE_WITHOP (arg_info) = TBmakeModarray (DUPdoDupTree (INFO_HSE_COPY_FROM (arg_info)));
            } else {
                INFO_HSE_WITHOP (arg_info)
                    = TBmakeGenarray (DUPdoDupTree (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))),
                                      BuildDefault (DUPdoDupTree (SETWL_EXPR (arg_node)) ) );
            }
        } else {
            /* As there is no reference to the generator, we use the expression as default */
            INFO_HSE_WITHOP (arg_info) = TBmakeGenarray (DUPdoDupTree (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))),
                                                         DUPdoDupTree (SETWL_EXPR (arg_node)) );
        }
    }

    FREEdoFreeTree (arg_node);

    if (oldinfo != NULL) {
        arg_info = FreeInfo( arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief hook to replace ids bound by setwl notation
 *        within default values.
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 *
 * @return transformed AST
 */
node *
HSEspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
