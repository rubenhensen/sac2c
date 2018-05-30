#include "handle_set_expressions.h"
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
typedef enum TRAVSTATE { HSE_scan, HSE_default } travstate;
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
    travstate state;
    idtable *idtab;
    node *wlshape;
};

/* access macros */
#define INFO_HSE_TRAVSTATE(n) ((n)->state)
#define INFO_HSE_IDTABLE(n) ((n)->idtab)
#define INFO_HSE_WLSHAPE(n) ((n)->wlshape)

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

    INFO_HSE_TRAVSTATE (result) = HSE_scan;
    INFO_HSE_IDTABLE (result) = NULL;
    INFO_HSE_WLSHAPE (result) = NULL;

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
 * builds an id with a free name by calling TmpVarName. If
 * HSE_USE_EXPLANATORY_NAMES is set, name is appended to the new id,
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

#ifdef HSE_USE_EXPLANATORY_NAMES
    result = TBmakeSpid (NULL, TRAVtmpVarName (name));
#else
    result = TBmakeSpid (NULL, TRAVtmpVar ());
#endif

    DBUG_RETURN (result);
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

    DBUG_ENTER ();

    if (NODE_TYPE (ids) == N_exprs) {
        while (ids != NULL) {
            node *id = EXPRS_EXPR (ids);
            idtable *newtab = (idtable *)MEMmalloc (sizeof (idtable));

            if (NODE_TYPE (id) != N_spid) {
                CTIerrorLine (global.linenum, "Found non-id as index in WL set notation");

                /* we create a dummy entry within the idtable in order */
                /* to go on and search for further errors.             */
                newtab->id = STRcpy ("_non_id_expr");
            } else {
                newtab->id = STRcpy (SPID_NAME (id));
            }

            newtab->type = ID_scalar;
            newtab->shapes = NULL;
            newtab->next = result;
            result = newtab;
            ids = EXPRS_NEXT (ids);
        }
    } else if (NODE_TYPE (ids) == N_spid)
#ifdef HSE_SETWL_VECTOR
    {
        idtable *newtab = (idtable *)MEMmalloc (sizeof (idtable));
        newtab->id = STRcpy (SPID_NAME (ids));
        newtab->type = ID_vector;
        newtab->shapes = NULL;
        newtab->next = result;
        result = newtab;
    }
#else
    {
        CTIabortLine (global.linenum, "Vector as index in WL set notation not allowed");
    }
#endif
    else {
        CTIabortLine (global.linenum, "Malformed index vector in WL set notation");
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

static idtype
IdTableContains (char *id, idtable *ids)
{
    idtype result = ID_notfound;

    DBUG_ENTER ();

    while (ids != NULL) {
        if (STReq (id, ids->id)) {
            result = ids->type;
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
    DBUG_ENTER ();

    while (table != until) {
        idtable *next = table->next;

        /* free shape-chain but NOT shapes itself */
        while (table->shapes != NULL) {
            shpchain *next = table->shapes->next;
            MEMfree (table->shapes);
            table->shapes = next;
        }

        /* free table */
        MEMfree (table->id);
        MEMfree (table);
        table = next;
    }

    DBUG_RETURN ();
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
ScanVector (node *vector, node *array, info *arg_info)
{
    int poscnt = 0;
    int tripledotflag = 0;
    int exprslen = TCcountExprs (vector);
    idtable *ids = INFO_HSE_IDTABLE (arg_info);

    DBUG_ENTER ();

    while (vector != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (vector)) == N_spid) {
            idtable *handle = ids;

            while (handle != NULL) {
                if (STReq (handle->id, SPID_NAME (EXPRS_EXPR (vector)))) {
                    if (handle->type == ID_scalar) {
                        node *position = NULL;
                        node *shape = NULL;
                        shpchain *chain = NULL;

                        if (tripledotflag) {
                            position
                              = MAKE_BIN_PRF (F_sub_SxS,
                                              TBmakePrf (F_dim_A,
                                                         TBmakeExprs (DUPdoDupTree (
                                                                        array),
                                                                      NULL)),
                                              TBmakeNum (exprslen - poscnt));
                        } else {
                            position = TBmakeNum (poscnt);
                        }

                        shape
                          = MAKE_BIN_PRF (F_sel_VxA,
                                          TCmakeIntVector (TBmakeExprs (position, NULL)),
                                          TBmakePrf (F_shape_A,
                                                     TBmakeExprs (DUPdoDupTree (array),
                                                                  NULL)));
                        chain = (shpchain *)MEMmalloc (sizeof (shpchain));

                        chain->shape = shape;
                        chain->next = handle->shapes;
                        handle->shapes = chain;

                        break;
                    } else if (handle->type == ID_vector) {
                        CTInoteLine (NODE_LINE (vector),
                                     "Set notation index vector '%s' is used in a scalar "
                                     "context.",
                                     handle->id);
                    }
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

    DBUG_RETURN ();
}

#ifdef HSE_SETWL_VECTOR
/**
 * scans a selection vector given as a single vector variable. If it
 * exists within ids, the corresponding shape is stored in ids.
 *
 * @param id selection vector id
 * @param array array the selection operates on
 * @param ids idtable structure
 */
static void
ScanId (node *id, node *array, info *arg_info)
{
    idtable *ids = INFO_HSE_IDTABLE (arg_info);
    DBUG_ENTER ();

    while (ids != NULL) {
        if (STReq (ids->id, SPID_NAME (id))) {
            if (ids->type == ID_vector) {
                node *shape
                  = TBmakePrf (F_shape_A, TBmakeExprs (DUPdoDupTree (array), NULL));
                shpchain *chain = (shpchain *)MEMmalloc (sizeof (shpchain));

                chain->shape = shape;
                chain->next = ids->shapes;
                ids->shapes = chain;

                break;
            }
        } else if (ids->type == ID_scalar) {
            CTInoteLine (NODE_LINE (id),
                         "Set notation index scalar '%s' is used in a vector "
                         "context.",
                         ids->id);
        }

        ids = ids->next;
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    indexids = TBmakeSpids (STRcpy (SPID_NAME (index)), NULL);

    shape = TBmakePrf (F_shape_A, TBmakeExprs (DUPdoDupTree (vectors->shape), NULL));

    expr = MAKE_BIN_PRF (F_sel_VxA, DUPdoDupTree (index), DUPdoDupTree (vectors->shape));

    vectors = vectors->next;

    while (vectors != NULL) {
        expr = MAKE_BIN_PRF (F_min_SxS,
                             MAKE_BIN_PRF (F_sel_VxA, DUPdoDupTree (index),
                                           DUPdoDupTree (vectors->shape)),
                             expr);
        vectors = vectors->next;
    }

    result = TBmakeWith (TBmakePart (NULL, TBmakeWithid (indexids, NULL),
                                     TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1),
                                                      TBmakeDot (1), NULL, NULL)),
                         TBmakeCode (MAKE_EMPTY_BLOCK (), TBmakeExprs (expr, NULL)),
                         TBmakeGenarray (shape, NULL));

    GENARRAY_DEFAULT (WITH_WITHOP (result)) = TBmakeNum (0);
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

    DBUG_ENTER ();

    if (table->type == ID_scalar) {
        while (table != end) {
            node *shape = NULL;
            shpchain *handle = table->shapes;

            if (handle == NULL) {
                CTIerrorLine (global.linenum,
                              "No shape information found for index "
                              "scalar '%s'.",
                              table->id);
            } else {
                shape = handle->shape;
                handle = handle->next;

                while (handle != NULL) {
                    shape = MAKE_BIN_PRF (F_min_SxS, shape, handle->shape);
                    handle = handle->next;
                }
            }

            result = TBmakeExprs (shape, result);
            table = table->next;
        }

        result = TCmakeIntVector (result);
    }
#ifdef HSE_SETWL_VECTOR
    else if (table->type == ID_vector) {
        if (table->shapes == NULL) {
            CTIerrorLine (global.linenum,
                          "No shape information found for index "
                          "vector '%s'.",
                          table->id);
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

/** <!--********************************************************************-->
 * checks for any occurrences of a dot symbol within a set notation
 * selection vector.
 *
 * @param ids EXPRS node containing ids
 * @return number of dots found
 *****************************************************************************/
static int
CountDotsInVector (node *ids)
{
    int result = 0;

    DBUG_ENTER ();

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
 * checks for any occurrences of a triple-dot symbol within a set notation
 * selection vector.
 *
 * @param ids EXPRS node containing ids
 * @return whether a triple dot was found
 *****************************************************************************/
static bool
ContainsTripleDot (node *ids)
{
    bool result = FALSE;

    DBUG_ENTER ();

    if (NODE_TYPE (ids) != N_exprs) {
        result = FALSE;
    } else {
        while (ids != NULL) {
            if ((NODE_TYPE (EXPRS_EXPR (ids)) == N_dot)
                && (DOT_NUM (EXPRS_EXPR (ids)) == 3)) {
                result = TRUE;
                break;
            }
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

    DBUG_ENTER ();

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
 * constructs the inverse permutation of the index vector to map the results
 * of a set notation to the set notation with dots.
 *
 * @param ids the indexvector of the set notation
 * @param vect the indexvector of the withloop used for permutation
 ****************************************************************************/
static node *
BuildInversePermutatedVector (node *ids, node *vect)
{
    node *result = NULL;
    node *left = NULL;
    node *left_expr = NULL;
    node *trav = ids;
    int single_pre_t = 0, triple = 0, others_pre_t = 0;
    int single_post_t = 0, others_post_t = 0;
    int pos = 0, dpos = 0, allpos = 0;
    int cnt;

    DBUG_ENTER ();

    /*
     * collect stats about this vector
     */
    while (trav != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (trav)) == N_dot) {
            if (DOT_NUM (EXPRS_EXPR (trav)) == 3) {
                triple++;
                if (triple > 1) {
                    CTIerrorLine (global.linenum,
                                  "Multiple occurrences of ... are not allowed in a "
                                  "single set notation statement.");
                }
                trav = EXPRS_NEXT (trav);
                break;
            } else {
                single_pre_t++;
            }
        } else {
            others_pre_t++;
        }

        trav = EXPRS_NEXT (trav);
    }

    while (trav != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (trav)) == N_dot) {
            if (DOT_NUM (EXPRS_EXPR (trav)) == 3) {
                triple++;
                if (triple > 1) {
                    CTIerrorLine (global.linenum,
                                  "Multiple occurrences of ... are not allowed in a "
                                  "single set notation statement.");
                }
            } else {
                single_post_t++;
            }
        } else {
            others_post_t++;
        }

        trav = EXPRS_NEXT (trav);
    }

    /* construct an exprs chain to hold the result. it needs to be long
     * enough to hold all not-dot axes as these are to the left of any
     * dot axes, in particular the .... Furthermore, it needs to have some
     * space for the dots to the left of the ... */
    for (cnt = 0; cnt < others_pre_t + others_post_t + single_pre_t; cnt++) {
        result = TBmakeExprs (NULL, result);
    }

    /* now fill it */
    trav = ids;

    while (trav != NULL) {
        int target;
        node *texpr;

        if (NODE_TYPE (EXPRS_EXPR (trav)) == N_dot) {
            if (DOT_NUM (EXPRS_EXPR (trav)) == 3) {
                break;
            }

            target = others_pre_t + others_post_t + dpos;
            dpos++;
        } else {
            target = pos;
            pos++;
        }
        node *entry
          = MAKE_BIN_PRF (F_sel_VxA,
                          TCmakeIntVector (TBmakeExprs (TBmakeNum (allpos), NULL)),
                          DUPdoDupTree (vect));

        texpr = TCgetNthExprs (target, result);

        DBUG_ASSERT (texpr != NULL, "bad permutation");
        DBUG_ASSERT (EXPRS_EXPR (texpr) == NULL, "slot taken in permutation");
        EXPRS_EXPR (texpr) = entry;

        trav = EXPRS_NEXT (trav);
        allpos++;
    }

    if (trav != NULL) {
        DBUG_ASSERT (triple > 0, "weird set notation");

        left_expr = result;

        left
          = TCmakePrf2 (F_cat_VxV, TCmakeIntVector (result),
                        TCmakePrf2 (F_take_SxV,
                                    TCmakePrf2 (F_sub_SxS,
                                                TCmakePrf2 (F_sel_VxA,
                                                            TCmakeIntVector (
                                                              TBmakeExprs (TBmakeNum (0),
                                                                           NULL)),
                                                            TCmakePrf1 (F_shape_A,
                                                                        DUPdoDupTree (
                                                                          vect))),
                                                TBmakeNum (single_pre_t + single_post_t
                                                           + others_pre_t
                                                           + others_post_t)),
                                    TCmakePrf2 (F_drop_SxV, TBmakeNum (allpos),
                                                DUPdoDupTree (vect))));
        result = NULL;

        trav = EXPRS_NEXT (trav);
    }

    /* now take care of the rest */
    for (cnt = 0; cnt < single_post_t; cnt++) {
        result = TBmakeExprs (NULL, result);
    }

    allpos = 0;
    while (trav != NULL) {
        node *target;

        node *entry
          = MAKE_BIN_PRF (F_sel_VxA,
                          TCmakeIntVector (
                            TBmakeExprs (TCmakePrf2 (F_sub_SxS,
                                                     TCmakePrf2 (F_sel_VxA,
                                                                 TCmakeIntVector (
                                                                   TBmakeExprs (TBmakeNum (
                                                                                  0),
                                                                                NULL)),
                                                                 TCmakePrf1 (F_shape_A,
                                                                             DUPdoDupTree (
                                                                               vect))),
                                                     TBmakeNum (single_post_t
                                                                + others_post_t
                                                                - allpos)),
                                         NULL)),
                          DUPdoDupTree (vect));

        if (NODE_TYPE (EXPRS_EXPR (trav)) == N_dot) {
            if (DOT_NUM (EXPRS_EXPR (trav)) == 3) {
                break;
            }

            target = TCgetNthExprs (dpos, result);
            DBUG_ASSERT (target != NULL, "permutation wrong");
            DBUG_ASSERT (EXPRS_EXPR (target) == NULL, "double entry");
            EXPRS_EXPR (target) = entry;
            dpos++;
        } else {
            target = TCgetNthExprs (pos, left_expr);
            DBUG_ASSERT (target != NULL, "permutation wrong");
            DBUG_ASSERT (EXPRS_EXPR (target) == NULL, "double entry");
            EXPRS_EXPR (target) = entry;
            pos++;
        }

        trav = EXPRS_NEXT (trav);
        allpos++;
    }

    if (left != NULL) {
        if (result == NULL) {
            result = left;
        } else {
            result = TCmakePrf2 (F_cat_VxV, left, result);
        }
    } else {
        result = TCmakeIntVector (result);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * constructs the permutation of the shape vector to map the results of a set
 * notation to the set notation with dots.
 *
 * @param ids the shape vector of the withloop representing the set notation
 * @param vect the shape vector of the withloop used for permutation
 ****************************************************************************/
static node *
BuildPermutatedVector (node *ids, node *vect)
{
    node *result = NULL;
    node *left = NULL;
    node *trav = ids;
    node *next = NULL;
    int single = 0, triple = 0, others = 0;
    int pos = 0, dpos = 0;

    DBUG_ENTER ();

    /*
     * first count the non-dot entries and the number of
     * single dots and whether we have a tripledot
     */
    while (trav != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (trav)) == N_dot) {
            if (DOT_NUM (EXPRS_EXPR (trav)) == 3) {
                triple++;
                if (triple > 1) {
                    CTIerrorLine (global.linenum,
                                  "Multiple occurrences of ... are not allowed in a "
                                  "single set notation statement.");
                }
            } else {
                single++;
            }
        } else {
            others++;
        }

        trav = EXPRS_NEXT (trav);
    }

    /* now build everything up to the ... if there is one */
    trav = ids;

    while (trav != NULL) {
        int target;

        if (NODE_TYPE (EXPRS_EXPR (trav)) == N_dot) {
            if (DOT_NUM (EXPRS_EXPR (trav)) == 3) {
                break;
            }

            target = others + dpos;
            dpos++;
        } else {
            target = pos;
            pos++;
        }
        node *entry
          = MAKE_BIN_PRF (F_sel_VxA,
                          TCmakeIntVector (TBmakeExprs (TBmakeNum (target), NULL)),
                          DUPdoDupTree (vect));

        if (result == NULL) {
            result = TBmakeExprs (entry, NULL);
            next = result;
        } else {
            EXPRS_NEXT (next) = TBmakeExprs (entry, NULL);
            next = EXPRS_NEXT (next);
        }

        trav = EXPRS_NEXT (trav);
    }

    if (trav != NULL) {
        DBUG_ASSERT (triple > 0, "weird set notation");

        left
          = TCmakePrf2 (F_cat_VxV, TCmakeIntVector (result),
                        TCmakePrf2 (F_take_SxV,
                                    TCmakePrf2 (F_sub_SxS,
                                                TCmakePrf2 (F_sel_VxA,
                                                            TCmakeIntVector (
                                                              TBmakeExprs (TBmakeNum (0),
                                                                           NULL)),
                                                            TCmakePrf1 (F_shape_A,
                                                                        DUPdoDupTree (
                                                                          vect))),
                                                TBmakeNum (single + others)),
                                    TCmakePrf2 (F_drop_SxV, TBmakeNum (dpos + others),
                                                DUPdoDupTree (vect))));
        result = NULL;

        trav = EXPRS_NEXT (trav);
    }

    /* now take care of the rest */

    while (trav != NULL) {
        node *target;

        if (NODE_TYPE (EXPRS_EXPR (trav)) == N_dot) {
            if (DOT_NUM (EXPRS_EXPR (trav)) == 3) {
                break;
            }

            target
              = TCmakePrf2 (F_add_SxS,
                            TCmakePrf2 (F_sub_SxS,
                                        TCmakePrf2 (F_sel_VxA,
                                                    TCmakeIntVector (
                                                      TBmakeExprs (TBmakeNum (0), NULL)),
                                                    TCmakePrf1 (F_shape_A,
                                                                DUPdoDupTree (vect))),
                                        TBmakeNum (others + single + 1)),
                            TBmakeNum (dpos));
            dpos++;
        } else {
            target = TBmakeNum (pos);
            pos++;
        }
        node *entry
          = MAKE_BIN_PRF (F_sel_VxA, TCmakeIntVector (TBmakeExprs (target, NULL)),
                          DUPdoDupTree (vect));

        if (result == NULL) {
            result = TBmakeExprs (entry, NULL);
            next = result;
        } else {
            EXPRS_NEXT (next) = TBmakeExprs (entry, NULL);
            next = EXPRS_NEXT (next);
        }

        trav = EXPRS_NEXT (trav);
    }

    if (result != NULL) {
        result = TCmakeIntVector (result);
    }

    if (left != NULL) {
        if (result == NULL) {
            result = left;
        } else {
            result = TCmakePrf2 (F_cat_VxV, left, result);
        }
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

    arg_info = MakeInfo ();
    INFO_HSE_TRAVSTATE (arg_info) = HSE_scan;

    TRAVpush (TR_hse);

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
HSEspap (node *arg_node, info *arg_info)
{
    node *result = arg_node;

    DBUG_ENTER ();

    /* if in HSE_scan mode, scan for shapes */

    if ((INFO_HSE_TRAVSTATE (arg_info) == HSE_scan) && (STReq (SPAP_NAME (arg_node), "sel"))
        && (SPAP_NS (arg_node) == NULL)) {
        if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_array) {
            ScanVector (ARRAY_AELEMS (SPAP_ARG1 (arg_node)), SPAP_ARG2 (arg_node),
                        arg_info);
        }
#ifdef HSE_SETWL_VECTOR
        else if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_spid) {
            ScanId (SPAP_ARG1 (arg_node), SPAP_ARG2 (arg_node), arg_info);
        }
#endif
    }

    /*
     * if in HSE_default mode, build default
     */
    if ((INFO_HSE_TRAVSTATE (arg_info) == HSE_default)
        && (STReq (SPAP_NAME (arg_node), "sel")) && (SPAP_NS (arg_node) == NULL)) {
        if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_array) {
            /*
             * found a selection using a selection
             * vector build of scalars and maybe dots.
             */
            dotinfo *info = MakeDotInfo (ARRAY_AELEMS (SPAP_ARG1 (arg_node)));
            node *defshape = NULL;

            /*
             * LEVEL I: handle all dots from the right
             *          to the middle ...
             */

            dotlist *dots = info->right;

            while ((dots != NULL) && (dots->dottype == 1)) {
                defshape = TBmakeExprs (TCmakePrf2 (F_sel_VxA,
                                                    TCmakeIntVector (
                                                      TBmakeExprs (TBmakeNum (
                                                                     dots->position - 1),
                                                                   NULL)),
                                                    TCmakePrf1 (F_shape_A,
                                                                DUPdoDupTree (
                                                                  SPAP_ARG2 (arg_node)))),
                                        defshape);

                dots = dots->prev;
            }

            defshape = TCmakeIntVector (defshape);

            /*
             * LEVEL II: handle the ... in the middle, if it exists
             */

            if (info->triplepos != 0) {
                node *middleshape;

                /*
                 * take( dim(A) - (selcnt - triplepos), shape(A))
                 */
                middleshape
                  = TCmakePrf2 (F_take_SxV,
                                TCmakePrf2 (F_sub_SxS,
                                            TCmakePrf1 (F_dim_A, DUPdoDupTree (
                                                                   SPAP_ARG2 (arg_node))),
                                            TBmakeNum (info->selcnt - info->triplepos)),
                                TCmakePrf1 (F_shape_A,
                                            DUPdoDupTree (SPAP_ARG2 (arg_node))));
                /*
                 * drop( triplepos, ...)
                 */
                middleshape
                  = TCmakePrf2 (F_drop_SxV, TBmakeNum (info->triplepos - 1), middleshape);
                /*
                 * combine shapes
                 */
                defshape = TCmakePrf2 (F_cat_VxV, middleshape, defshape);

                /*
                 * move on to next dot
                 */
                dots = dots->prev;
            }

            /*
             * LEVEL III: continue handling dots on the
             *            left side of ... if any
             */
            if (dots != NULL) {
                node *leftshape = NULL;

                while ((dots != NULL) && (dots->dottype == 1)) {
                    leftshape
                      = TBmakeExprs (TCmakePrf2 (F_sel_VxA,
                                                 TCmakeIntVector (
                                                   TBmakeExprs (TBmakeNum (dots->position
                                                                           - 1),
                                                                NULL)),
                                                 TCmakePrf1 (F_shape_A,
                                                             DUPdoDupTree (
                                                               SPAP_ARG2 (arg_node)))),
                                     leftshape);

                    dots = dots->prev;
                }

                leftshape = TCmakeIntVector (leftshape);

                defshape = TCmakePrf2 (F_cat_VxV, leftshape, defshape);
            }

            /*
             * LEVEL IV: if there was no ...
             *           add the non handeled dimensions
             */

            if (info->triplepos == 0) {
                defshape = TCmakePrf2 (F_cat_VxV, defshape,
                                       TCmakePrf2 (F_drop_SxV, TBmakeNum (info->selcnt),
                                                   TCmakePrf1 (F_shape_A,
                                                               DUPdoDupTree (
                                                                 SPAP_ARG2 (arg_node)))));
            }

            /*
             * use the shape to build a default wl
             */
            result = BuildDefaultWithloop (SPAP_ARG2 (arg_node), defshape);

            FreeDotInfo (info);
        } else if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_spid) {
            idtype type = IdTableContains (SPID_NAME (SPAP_ARG1 (arg_node)),
                                           INFO_HSE_IDTABLE (arg_info));

            if (type == ID_vector) {
                /*
                 * found a setwl vector as index for a selection
                 * so replace the selection by a withloop
                 * with shape
                 * drop (_sel_( [0], shape( wlindex)), shape( array))
                 */

                node *wlshape
                  = TCmakePrf2 (F_drop_SxV,
                                TCmakePrf2 (F_sel_VxA,
                                            TCmakeIntVector (
                                              TBmakeExprs (TBmakeNum (0), NULL)),
                                            TCmakePrf1 (F_shape_A,
                                                        DUPdoDupTree (
                                                          INFO_HSE_WLSHAPE (arg_info)))),
                                TCmakePrf1 (F_shape_A,
                                            DUPdoDupTree (SPAP_ARG2 (arg_node))));

                result = BuildDefaultWithloop (SPAP_ARG2 (arg_node), wlshape);

                arg_node = FREEdoFreeTree (arg_node);
            } else if (type == ID_scalar) {
                CTIerrorLine (NODE_LINE (SPAP_ARG2 (arg_node)),
                              "identifier '%s' defined as scalar in set notation is "
                              "used as an index vector in a selection or the selection "
                              "operates on a scalar index. To disambiguate, "
                              "use '[%s]' instead.",
                              SPID_NAME (SPAP_ARG1 (arg_node)),
                              SPID_NAME (SPAP_ARG1 (arg_node)));

                /*
                 * we create some dummy code here, just to go on
                 * an look for errors
                 */
                arg_node = FREEdoFreeTree (arg_node);
                result = TBmakeNum (0);
            }
        }
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
HSEprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* if in HSE_scan mode, scan for shapes */

    if ((INFO_HSE_TRAVSTATE (arg_info) == HSE_scan) && (PRF_PRF (arg_node) == F_sel_VxA)) {
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array) {
            ScanVector (ARRAY_AELEMS (PRF_ARG1 (arg_node)), PRF_ARG2 (arg_node),
                        arg_info);
        }
#ifdef HSE_SETWL_VECTOR
        else if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_spid) {
            ScanId (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info);
        }
#endif
    }

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
    node *result = NULL;
    travstate oldstate = INFO_HSE_TRAVSTATE (arg_info);
    idtable *oldtable = INFO_HSE_IDTABLE (arg_info);
    node *oldshape = INFO_HSE_WLSHAPE (arg_info);
    node *defexpr = NULL;
    node *ids = NULL;
    int dotcnt;

    DBUG_ENTER ();

    /* maybe the set-index contains some dots */
    dotcnt = CountDotsInVector (SETWL_VEC (arg_node));

    /* build vector without dots */
    if (dotcnt == 0) {
        ids = DUPdoDupTree (SETWL_VEC (arg_node));
    } else {
        ids = RemoveDotsFromVector (SETWL_VEC (arg_node));
    }

    /* from here on, it is a set notation without any dots */

    INFO_HSE_TRAVSTATE (arg_info) = HSE_scan;
    INFO_HSE_IDTABLE (arg_info) = BuildIdTable (ids, INFO_HSE_IDTABLE (arg_info));

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_HSE_WLSHAPE (arg_info) = BuildWLShape (INFO_HSE_IDTABLE (arg_info), oldtable);

    if (INFO_HSE_WLSHAPE (arg_info) != NULL) {
        if (INFO_HSE_IDTABLE (arg_info)->type == ID_scalar) {
            result
              = TBmakeWith (TBmakePart (NULL, TBmakeWithid (NULL, Exprs2Ids (ids)),
                                        TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1),
                                                         TBmakeDot (1), NULL, NULL)),
                            TBmakeCode (MAKE_EMPTY_BLOCK (),
                                        TBmakeExprs (DUPdoDupTree (SETWL_EXPR (arg_node)),
                                                     NULL)),
                            TBmakeGenarray (DUPdoDupTree (INFO_HSE_WLSHAPE (arg_info)),
                                            NULL));
        }
#ifdef HSE_SETWL_VECTOR
        else {
            node *newids = TBmakeSpids (STRcpy (SPID_NAME (ids)), NULL);

            result
              = TBmakeWith (TBmakePart (NULL, TBmakeWithid (newids, NULL),
                                        TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1),
                                                         TBmakeDot (1), NULL, NULL)),
                            TBmakeCode (MAKE_EMPTY_BLOCK (),
                                        TBmakeExprs (DUPdoDupTree (SETWL_EXPR (arg_node)),
                                                     NULL)),
                            TBmakeGenarray (DUPdoDupTree (INFO_HSE_WLSHAPE (arg_info)),
                                            NULL));
        }
#endif

        /* build a default value for the withloop */
        defexpr = DUPdoDupTree (SETWL_EXPR (arg_node));
        INFO_HSE_TRAVSTATE (arg_info) = HSE_default;
        defexpr = TRAVdo (defexpr, arg_info);
        INFO_HSE_TRAVSTATE (arg_info) = HSE_scan;

        CODE_USED (WITH_CODE (result))++;
        PART_CODE (WITH_PART (result)) = WITH_CODE (result);
        GENARRAY_DEFAULT (WITH_WITHOP (result)) = defexpr;

        /* check whether we had some dots in order to create */
        /* code to handle the permutation                    */

        if (dotcnt != 0) {
            node *intermediate = result;
            node *withid = MakeTmpId ("permutationiv");
            node *selvector = BuildInversePermutatedVector (SETWL_VEC (arg_node), withid);
            node *shape
              = TBmakePrf (F_shape_A, TBmakeExprs (DUPdoDupTree (intermediate), NULL));
            node *shapevector = BuildPermutatedVector (SETWL_VEC (arg_node), shape);
            node *defexpr = NULL;
            node *defshape = NULL;
            node *withids = TBmakeSpids (STRcpy (SPID_NAME (withid)), NULL);

            /* create permutation code */

            /* build the default value */
            if (ContainsTripleDot (SETWL_VEC (arg_node))) {
                defexpr = TCmakeSpap1 (NSgetNamespace (global.preludename),
                                       STRcpy ("zero"), DUPdoDupTree (intermediate));
            } else {
                defshape
                  = MAKE_BIN_PRF (F_drop_SxV,
                                  TBmakeNum (TCcountExprs (SETWL_VEC (arg_node))),
                                  TBmakePrf (F_shape_A,
                                             TBmakeExprs (DUPdoDupTree (intermediate),
                                                          NULL)));

                defexpr = BuildDefaultWithloop (intermediate, defshape);
            }

            result
              = TBmakeWith (TBmakePart (NULL, TBmakeWithid (withids, NULL),
                                        TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1),
                                                         TBmakeDot (1), NULL, NULL)),
                            TBmakeCode (MAKE_EMPTY_BLOCK (),
                                        TBmakeExprs (TCmakeSpap2 (NULL, STRcpy ("sel"),
                                                                  selvector,
                                                                  intermediate),
                                                     NULL)),
                            TBmakeGenarray (shapevector, NULL));

            GENARRAY_DEFAULT (WITH_WITHOP (result)) = defexpr;
            CODE_USED (WITH_CODE (result))++;
            PART_CODE (WITH_PART (result)) = WITH_CODE (result);
        }

        FREEdoFreeTree (arg_node);
        FREEdoFreeTree (ids);

        FreeIdTable (INFO_HSE_IDTABLE (arg_info), oldtable);

        INFO_HSE_WLSHAPE (arg_info) = FREEdoFreeTree (INFO_HSE_WLSHAPE (arg_info));
    }

    INFO_HSE_IDTABLE (arg_info) = oldtable;
    INFO_HSE_TRAVSTATE (arg_info) = oldstate;
    INFO_HSE_WLSHAPE (arg_info) = oldshape;

    if (result != NULL) {
        arg_node = TRAVdo (result, arg_info);
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

    if (INFO_HSE_TRAVSTATE (arg_info) == HSE_default) {
        idtype type = IdTableContains (SPID_NAME (arg_node), INFO_HSE_IDTABLE (arg_info));

        if (type == ID_scalar) {
            CTInoteLine (NODE_LINE (arg_node),
                         "Cannot infer default for '%s' as it is used as argument to"
                         " a non-selection operation, using 0 as fallback",
                         SPID_NAME (arg_node));

            FREEdoFreeTree (arg_node);
            arg_node = TBmakeNum (0);
        } else if (type == ID_vector) {
            CTInoteLine (NODE_LINE (arg_node),
                         "Cannot infer default for '%s' as it is used as argument to"
                         " a non-selection operation, using 0-vector as fallback",
                         SPID_NAME (arg_node));

            FREEdoFreeTree (arg_node);
            /*
             * build 0 * wlshape instead of vector
             */
            arg_node = TBmakePrf (F_mul_SxV,
                                  TBmakeExprs (TBmakeNum (0),
                                               TBmakeExprs (DUPdoDupTree (
                                                              INFO_HSE_WLSHAPE (arg_info)),
                                                            NULL)));
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
