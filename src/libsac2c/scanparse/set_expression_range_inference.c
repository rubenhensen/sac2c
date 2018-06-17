#include "set_expression_range_inference.h"
#include "traverse.h"

#define DBUG_PREFIX "SERI"
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
#include "print.h"

#include <strings.h>

/**
 * @file set_expression_range_inference.c
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
 * arg_info in this file:
 * IDS:    N_exprs list of lhs identifiers of the setwl.
 * LENTD:  an expression that computes the length of the triple-dot
 *         match (potentially at runtime)
 */

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

/* INFO structure */
struct INFO {
    bool lbmissing;
    bool ubmissing;
    bool islastpart;
    idtable *idtable;
    node *shp;
    struct INFO *next;
};

/* access macros */
#define INFO_SERI_LBMISSING(n) ((n)->lbmissing)
#define INFO_SERI_UBMISSING(n) ((n)->ubmissing)
#define INFO_SERI_ISLASTPART(n) ((n)->islastpart)
#define INFO_SERI_IDTABLE(n) ((n)->idtable)
#define INFO_SERI_SHP(n) ((n)->shp)
#define INFO_SERI_NEXT(n) ((n)->next)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (info *oldinfo)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SERI_LBMISSING (result) = FALSE;
    INFO_SERI_UBMISSING (result) = FALSE;
    INFO_SERI_ISLASTPART (result) = FALSE;
    INFO_SERI_IDTABLE (result) = NULL;
    INFO_SERI_SHP (result) = NULL;
    INFO_SERI_NEXT (result) = oldinfo;

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
    info *next;

    DBUG_ENTER ();

    next = INFO_SERI_NEXT (arg_info);
    arg_info = MEMfree (arg_info);

    DBUG_RETURN (next);
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
            DBUG_PRINT( "adding scalar id \"%s\"", newtab->id);
        }
    } else if (NODE_TYPE (ids) == N_spid) {
        idtable *newtab = (idtable *)MEMmalloc (sizeof (idtable));
        newtab->id = STRcpy (SPID_NAME (ids));
        newtab->type = ID_vector;
        newtab->shapes = NULL;
        newtab->next = result;
        result = newtab;
        DBUG_PRINT( "adding vector id \"%s\"", newtab->id);
    } else {
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


static int
CountIdsInIdTable (info *arg_info)
{
    int res=0;
    idtable *first, *last;

    DBUG_ENTER ();

    first = INFO_SERI_IDTABLE (arg_info);
    last = INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info));
    while (first != last) {
        first = first->next;
        res++;
    }

    DBUG_RETURN (res);
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
    } else if (table->type == ID_vector) {
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

    DBUG_RETURN (result);
}

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
static bool
ShapeInfoComplete (idtable *table, idtable *end)
{
    bool result = TRUE;

    DBUG_ENTER ();

    if (table->type == ID_scalar) {
        while (table != end) {
            if (table->shapes == NULL) {
                result = FALSE;
            }
            table = table->next;
        }

    } else if (table->type == ID_vector) {
        if (table->shapes == NULL) {
            result = FALSE;
        }
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
    idtable *ids = INFO_SERI_IDTABLE (arg_info);

    DBUG_ENTER ();

    while (vector != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (vector)) == N_spid) {
            idtable *handle = ids;

            while (handle != NULL) {
                if (STReq (handle->id, SPID_NAME (EXPRS_EXPR (vector)))) {
                    DBUG_PRINT ("id \"%s\" found", handle->id);
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

                        DBUG_PRINT ("shape added:");
                        DBUG_EXECUTE (PRTdoPrint( shape));
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
    idtable *ids = INFO_SERI_IDTABLE (arg_info);
    DBUG_ENTER ();

    while (ids != NULL) {
        if (STReq (ids->id, SPID_NAME (id))) {
            DBUG_PRINT ("id \"%s\" found", ids->id);
            if (ids->type == ID_vector) {
                node *shape
                  = TBmakePrf (F_shape_A, TBmakeExprs (DUPdoDupTree (array), NULL));
                shpchain *chain = (shpchain *)MEMmalloc (sizeof (shpchain));

                chain->shape = shape;
                chain->next = ids->shapes;
                ids->shapes = chain;

                DBUG_PRINT ("shape added:");
                DBUG_EXECUTE (PRTdoPrint( shape));
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
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
SERIdoInferRanges (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

#if 0
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_setwl,
                "SERIdoInferRanges called on non-Nsetwl node!");
#endif

DBUG_EXECUTE( 
    arg_info = MakeInfo (NULL);
    TRAVpush (TR_seri);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info););

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
SERIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_sel_VxA) {
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array) {
            DBUG_PRINT_TAG ("SERI_ACT", "Primitive selection with N_array index found; scanning for genvars..." );
            ScanVector (ARRAY_AELEMS (PRF_ARG1 (arg_node)), PRF_ARG2 (arg_node),
                        arg_info);
        }
        else if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_spid) {
            DBUG_PRINT_TAG ("SERI_ACT", "Primitive selection with N_spid index found; scanning for genvar..." );
            ScanId (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info);
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);

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
SERIspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (STReq (SPAP_NAME (arg_node), "sel")
        && (SPAP_NS (arg_node) == NULL)) {
        if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_array) {
            DBUG_PRINT_TAG ("SERI_ACT", "Spap selection with N_array index found; scanning for genvars..." );
            ScanVector (ARRAY_AELEMS (SPAP_ARG1 (arg_node)), SPAP_ARG2 (arg_node),
                        arg_info);
        } else if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_spid) {
            DBUG_PRINT_TAG ("SERI_ACT", "Spap selection with N_spid index found; scanning for genvar..." );
            ScanId (SPAP_ARG1 (arg_node), SPAP_ARG2 (arg_node), arg_info);
        }
    } 

    /* Check for nested info */

    SPAP_ARGS (arg_node) = TRAVdo (SPAP_ARGS (arg_node), arg_info);

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
SERIsetwl (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("looking at Set-Expression in line %d:", global.linenum);

    /* we do this bottom up to ensure we have a shape! Only the last
     * partition is guaranteed to produce an upper bound!
     * A reference to that upper bound is put into INFO_SERI_SHP of
     * the *surrounding* arg_info, i.e. into the *current* arg_info.
     */
    if (SETWL_NEXT (arg_node) != NULL) {
        SETWL_NEXT (arg_node) = TRAVdo (SETWL_NEXT (arg_node), arg_info);
        DBUG_ASSERT (INFO_SERI_SHP (arg_info) != NULL, "last setWL partition"
                     " did not insert shape!");
    }

    arg_info = MakeInfo (arg_info);
    DBUG_PUSH( "-d,SERI,SERI_ACT");

    /*
     * First, we check which ranges are missing
     */
    INFO_SERI_LBMISSING( arg_info) = 
        ( (SETWL_GENERATOR (arg_node) == NULL)
          || (GENERATOR_BOUND1 (SETWL_GENERATOR (arg_node)) == NULL) );
    DBUG_PRINT( " lower bound is %s!", (INFO_SERI_LBMISSING( arg_info)? "missing" : "present"));

    INFO_SERI_UBMISSING( arg_info) = 
        ( (SETWL_GENERATOR (arg_node) == NULL)
          || (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node)) == NULL) );
    DBUG_PRINT( " upper bound is %s!", (INFO_SERI_UBMISSING( arg_info)? "missing" : "present"));

    if (INFO_SERI_LBMISSING( arg_info) || INFO_SERI_UBMISSING( arg_info)) {
        /*
         * If we need to infer ranges we need to prepand
         * new ids to INFO_SETWL_IDTABLE:
         */
        DBUG_PRINT( "adding new ids to idtable");
        INFO_SERI_IDTABLE (arg_info) = BuildIdTable (SETWL_VEC (arg_node),
                                                     INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info)));
    } else {
        INFO_SERI_IDTABLE (arg_info) = INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info));
    }
    DBUG_PRINT( "traversing expression...");
    SETWL_EXPR (arg_node) = TRAVdo (SETWL_EXPR (arg_node), arg_info);
    if (SETWL_GENERATOR (arg_node) == NULL) {
        SETWL_GENERATOR (arg_node) = TBmakeGenerator (F_noop, F_noop, NULL, NULL, NULL, NULL);
    }
    INFO_SERI_ISLASTPART (arg_info) = (SETWL_NEXT (arg_node) == NULL);

    SETWL_GENERATOR (arg_node) = TRAVdo (SETWL_GENERATOR (arg_node), arg_info);

    FreeIdTable (INFO_SERI_IDTABLE (arg_info), INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info)));
    INFO_SERI_IDTABLE (arg_info) = NULL;
    arg_info = FreeInfo (arg_info);
    DBUG_POP();

    DBUG_RETURN (arg_node);
}


/**
 * hook to insert the inferred shape knowledge, or, in the lack thereof, 
 * dots UNLESS we are the last partition in which cazse the lack of bounds
 * will raise an error.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return transformed AST
 */
node *
SERIgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("SERI_ACT", "traversing generator...");
    if (INFO_SERI_UBMISSING (arg_info)) {
        if (ShapeInfoComplete (INFO_SERI_IDTABLE (arg_info),
                               INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info)))) {
            /* We now insert the information we found */
            GENERATOR_BOUND2 (arg_node)
                = BuildWLShape (INFO_SERI_IDTABLE (arg_info),
                                INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info)));
            GENERATOR_OP2 (arg_node) = F_wl_lt;
        } else {
            if (INFO_SERI_ISLASTPART (arg_info)) {
                CTIerror ("Unable to infer upper bound for final partition of"
                          " set expression; please specify an upper bound.");
            } else {
                if (INFO_SERI_IDTABLE (arg_info)->type == ID_vector) {
                    CTIwarn ("Unable to infer upper bound for a partition of"
                             " a set expression; using \".\" instead.");
                    GENERATOR_BOUND2 (arg_node) = TBmakeDot (1);
                    GENERATOR_OP2 (arg_node) = F_wl_le;
                } else if (INFO_SERI_IDTABLE (arg_info)->type == ID_scalar) {
                    CTIwarn ("Unable to infer upper bound for a partition of"
                             " a set expression; using upper bound from last partition instead.");
                    GENERATOR_BOUND2 (arg_node)
                        = TCmakePrf2 (F_take_SxV,
                                      TBmakeNum (CountIdsInIdTable (arg_info)),
                                      DUPdoDupTree (INFO_SERI_SHP (INFO_SERI_NEXT (arg_info))));
                    GENERATOR_OP2 (arg_node) = F_wl_lt;
                }
            }
        }
        
    } else {
        GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);
    }

    if (INFO_SERI_ISLASTPART (arg_info)) {
        /* Now, we add the shape information into the "surrounding" arg_info!
         * We need to put it there as it is needed by all other partitions of the
         * current setwl as well and the current arg_info will be gone as soon as we
         * leave the last partition!
         */
        INFO_SERI_SHP (INFO_SERI_NEXT (arg_info)) = GENERATOR_BOUND2 (arg_node);
    }

    if (INFO_SERI_LBMISSING (arg_info)) {
        if ( FALSE) {
            /* We now insert the information we found */
        } else {
            if (INFO_SERI_IDTABLE (arg_info)->type == ID_vector) {
                CTInote ("Unable to infer lower bound for a partition of"
                         " a set expression; using \".\" instead.");
                GENERATOR_BOUND1 (arg_node) = TBmakeDot (1);
            } else if (INFO_SERI_IDTABLE (arg_info)->type == ID_scalar) {
                CTInote ("Unable to infer lower bound for a partition of"
                         " a set expression; using vector of 0s instead.");
                GENERATOR_BOUND1 (arg_node)
                    = TCcreateIntVector (CountIdsInIdTable (arg_info),0,0);
            }
            GENERATOR_OP1 (arg_node) = F_wl_le;
        }
        
    } else {
        GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);
    }

    GENERATOR_STEP (arg_node) = TRAVopt (GENERATOR_STEP (arg_node), arg_info);

    GENERATOR_WIDTH (arg_node) = TRAVopt (GENERATOR_WIDTH (arg_node), arg_info);

    DBUG_PRINT_TAG ("SERI_ACT", "traversing generator done");
    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
