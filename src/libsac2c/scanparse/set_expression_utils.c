#include "set_expression_utils.h"
#include "traverse.h"

#define DBUG_PREFIX "SEUT"
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
 * @file set_expression_utils.c
 *
 * This is a helper module that helps maintaining a list of identifyers 
 * used as generator variables in set-expressions.
 * This alternative representation is need to facilitate easy searching
 * for shape information, particularly in the context of nested set expressions
 * The module also provides some basic AST generation abstractions for 
 * derived expressions such as .... (TB added later :-)
 *
 * Each set expression either
 * case 1:  has a single vector variable { iv -> expr....} or
 * case 2:  a non-empty set of scalar variables { [i,j] -> expr...}.

 * Note here, that all variables can also be dots, i.e. 
 * case 3: { [.,i,.,j] -> expr....} or
 * case 4: { [...,i] -> expr.....}.
 *
 * an idtable is constructed by using SEUTbuildIdTable( vec, oldscope)
 * for each set-notation generator where
 * vec is a node which either should be an N_exprs (cases 2,3,4), or an
 * N_spid (case 1).
 * By providing a non-NULL idtable "oldscope" idtables can be linked
 * to each other to reflect the scoping of nested generator variables.
 * While all identifiers of nested idtables are kept as a linear chain,
 * the scoping nature is internally maintained through additional
 * links.
 * For example:
 *    { [.,l] -> { kv -> { [i,...,j] -> expr( i,j,kv, l) ....}}}
 * will internally create an idtable list of the form:
 *
 *   (i,scal) -> (_dots,vec) -> (j,scal) -> (kv,vec) -> (_dot,scal) -> (l,scal) 
 *       |               |          |        ^   |       ^     |           |
 *       \---------------\----------\--------/   \-------/     \-----------\---> NULL
 *
 * The dual to SEUTbuildIdTable is SEUTfreeIdTable. It frees the entire top 
 * frame(!). Returning to the above example, a single call to SEUTfreeIdTable on the
 * entry (i,scal) will result in:
 *                                          (kv,vec) -> (_dot,scal) -> (l,scal) 
 *                                               |       ^     |           |
 *                                               \-------/     \-----------\---> NULL
 *
 * Entire chains can be freed by calling SEUTfreeIdTableChain.
 *
 * The attributes (scal/vec) can be accessed through SEUTisVector and SEUTisScalar,
 * respectively.
 *
 * SEUTsearchIdInTable( id, from, to) searches for a given id (string) within a
 * given segment of an idtable chain (from->to). It will return the *first* matching
 * identifier! Assuming that the innermost scope is first in the chain, this 
 * matches the binding scope!
 *
 * SEUTcontainsIdFromTable( expr, from, to) checks whether any identifier contained
 * in the idtable chain between from and to is referred to as N_spid within the
 * expression expr.
 *
 * SEUTcountIds counts the number of non-dot idtable entries in the topmost frame!
 * When applied to our example, it will return 2. AFter calling SEUTfreeIdTable
 * as explained above, the result would be 1.
 * 
 * SEUTshapeInfoComplete checks whether for all identifiers in the topmost frame
 * the shape information is non-NULL.
 */

/**
 * set this to defined in order to create explanatory ids. use this only
 * for debugging as it might create very long identifier names.
 */
#define HSD_USE_EXPLANATORY_NAMES

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

typedef enum IDTYPE { ID_notfound = 0, ID_vector = 1, ID_scalar = 2 } idtype;

typedef struct SHPCHAIN {
    node *shape;
    node *code;
    struct SHPCHAIN *next;
} shpchain;

struct IDTABLE {
    char *id;
    idtype type;
    shpchain *shapes;
    struct IDTABLE *next;
    struct IDTABLE *nextframe;
};

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
idtable *
SEUTbuildIdTable (node *vec, idtable *oldscope)
{
    idtable *result = oldscope;

    DBUG_ENTER ();

    if (NODE_TYPE (vec) == N_exprs) {
        while (vec != NULL) {
            node *id = EXPRS_EXPR (vec);
            idtable *newtab = (idtable *)MEMmalloc (sizeof (idtable));

            if (NODE_TYPE (id) == N_spid) {
                newtab->id = STRcpy (SPID_NAME (id));
                newtab->type = ID_scalar;
            } else if (NODE_TYPE (id) == N_dot) {
                if (DOT_NUM (id) == 1) {
                    newtab->id = STRcpy (".");
                    newtab->type = ID_scalar;
                } else {
                    newtab->id = STRcpy (".");
                    newtab->type = ID_vector;
                }
            } else {
                CTIerrorLine (global.linenum, "Found neither id nor dot as index in WL set notation");
                /* we create a dummy entry within the idtable in order */
                /* to go on and search for further errors.             */
                newtab->id = STRcpy ("__non_id_expr");
                newtab->type = ID_scalar;
            }

            newtab->shapes = NULL;
            newtab->next = result;
            newtab->nextframe = oldscope;
            result = newtab;
            vec = EXPRS_NEXT (vec);
            DBUG_PRINT( "adding scalar id \"%s\"", newtab->id);
        }
    } else if (NODE_TYPE (vec) == N_spid) {
        idtable *newtab = (idtable *)MEMmalloc (sizeof (idtable));
        newtab->id = STRcpy (SPID_NAME (vec)) ;
        newtab->type = ID_vector;
        newtab->shapes = NULL;
        newtab->next = result;
        newtab->nextframe = oldscope;
        result = newtab;
        DBUG_PRINT( "adding vector id \"%s\"", newtab->id);
    } else {
        CTIabortLine (global.linenum, "Malformed index vector in WL set notation");
    }

    DBUG_RETURN (result);
}

/**
 * frees all elements in idtable "from" until the element "to" is reached.
 *
 * @param table table to clean up
 * @param until marker where to stop
 */
static
idtable *FreeIdTable (idtable *from, idtable *to)
{
    DBUG_ENTER ();

    while (from != to) {
        idtable *next = from->next;

        /* free shape-chain but NOT shapes itself */
        while (from->shapes != NULL) {
            shpchain *next = from->shapes->next;
            MEMfree (from->shapes);
            from->shapes = next;
        }

        /* free from */
        MEMfree (from->id);
        MEMfree (from);
        from = next;
    }

    DBUG_RETURN (to);
}


idtable *SEUTfreeIdTable (idtable *identry)
{
    DBUG_ENTER ();
    DBUG_RETURN ( identry != NULL ? FreeIdTable (identry, identry->nextframe) : NULL);
}

idtable *SEUTfreeIdTableChain (idtable *identry)
{
    DBUG_ENTER ();
    DBUG_RETURN (FreeIdTable (identry, NULL));
}

bool SEUTisDot (idtable *identry)
{
    DBUG_ENTER ();
    DBUG_RETURN ( STReq (identry->id, "."));
}

bool SEUTisVector (idtable *identry)
{
    DBUG_ENTER ();
    DBUG_RETURN (identry->type == ID_vector);
}


bool SEUTisScalar (idtable *identry)
{
    DBUG_ENTER ();
    DBUG_RETURN (identry->type == ID_scalar);
}


/**
 * checks for id in idtables between "from" and "to".
 *
 * @param id the id
 * @param from the first entry
 * @param to the last entry
 * @return the idtable entry if found, NULL otherwise
 */
idtable *SEUTsearchIdInTable (char *id, idtable *from, idtable *to)
{
    idtable *result = NULL;

    DBUG_ENTER ();

    while (from != to) {
        if (STReq (id, from->id)) {
            result = from;
            break;
        }
        from = from->next;
    }

    DBUG_RETURN (result);
}

struct INFO {
    bool contained;
    idtable *from;
    idtable *to;
};

node *ATravCIFTspid( node * arg_node, info * arg_info)
{
  arg_info->contained = arg_info->contained 
                        || ( SEUTsearchIdInTable (SPID_NAME( arg_node),
                                                  arg_info->from,
                                                  arg_info->to) 
                            != NULL );
  return arg_node;
}
  

bool SEUTcontainsIdFromTable (node *expr, idtable *from, idtable *to)
{
    info arg_info = {FALSE,from,to};
    anontrav_t cift_trav[2]
      = {{N_spid, &ATravCIFTspid}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    TRAVpushAnonymous (cift_trav, &TRAVsons);

    expr = TRAVopt (expr, & arg_info);

    TRAVpop ();

    DBUG_RETURN (arg_info.contained);
}

int SEUTcountIds (idtable *from)
{
    int res=0;
    idtable *to = from ->nextframe;

    DBUG_ENTER ();

    while (from != to) {
        if (!STReq (from->id, "."))
            res++;
        from = from->next;
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
BuildShapeVectorMin (idtable *identry)
{
    node *result = NULL;
    node *index = MakeTmpId ("index_min");
    node *shape = NULL;
    node *expr = NULL;
    node *indexids = NULL;
    shpchain *vectors;

    DBUG_ENTER ();

    vectors = identry->shapes;
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
node * SEUTbuildWLShape (idtable *table)
{
    node *result = NULL;
    idtable *end;

    DBUG_ENTER ();

    end = table->nextframe;
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
                result = BuildShapeVectorMin (table);
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
bool SEUTshapeInfoComplete (idtable *table)
{
    bool result = TRUE;
    idtable *end;

    DBUG_ENTER ();

    end = table->nextframe;
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
void SEUTscanSelectionForShapeInfo( node *idxvec, node *array, idtable *scope)
{
    int poscnt = 0;
    int tripledotflag = 0;
    int exprslen;

    DBUG_ENTER ();

    if ((scope != NULL) && !SEUTcontainsIdFromTable (array, scope, NULL)) {
    if (NODE_TYPE (idxvec) == N_array) {
        idxvec = ARRAY_AELEMS (idxvec);
        exprslen = TCcountExprs (idxvec);
        
        while (idxvec != NULL) {
            if (NODE_TYPE (EXPRS_EXPR (idxvec)) == N_spid) {
                idtable *handle = scope;

                while (handle != NULL) {
                    if (STReq (handle->id, SPID_NAME (EXPRS_EXPR (idxvec)))) {
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
                            DBUG_EXECUTE (PRTdoPrintFile (stderr, shape));
                            break;
                        } else if (handle->type == ID_vector) {
                            CTInoteLine (NODE_LINE (idxvec),
                                         "Set notation index vector '%s' is used in a scalar "
                                         "context.",
                                         handle->id);
                        }
                    }

                    handle = handle->next;
                }
            }

            /* check for occurence of '...' */

            if ((NODE_TYPE (EXPRS_EXPR (idxvec)) == N_dot)
                && (DOT_NUM (EXPRS_EXPR (idxvec)) == 3)) {
                tripledotflag = 1;
            }

            poscnt++;
            idxvec = EXPRS_NEXT (idxvec);
        }
    } else if (NODE_TYPE (idxvec) == N_spid) {

        while (scope != NULL) {
            if (STReq (scope->id, SPID_NAME (idxvec))) {
                DBUG_PRINT ("id \"%s\" found", scope->id);
                if (scope->type == ID_vector) {
                    node *shape
                      = TBmakePrf (F_shape_A, TBmakeExprs (DUPdoDupTree (array), NULL));
                    shpchain *chain = (shpchain *)MEMmalloc (sizeof (shpchain));

                    chain->shape = shape;
                    chain->next = scope->shapes;
                    scope->shapes = chain;
    
                    DBUG_PRINT ("shape added:");
                    DBUG_EXECUTE (PRTdoPrintFile (stderr, shape));
                    break;
                }
            } else if (scope->type == ID_scalar) {
                CTInoteLine (NODE_LINE (idxvec),
                             "Set notation index scalar '%s' is used in a vector "
                             "context.",
                             scope->id);
            }

            scope = scope->next;
        }
    }
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
