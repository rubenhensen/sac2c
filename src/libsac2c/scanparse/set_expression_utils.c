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
#include <limits.h>

/**
 * @file set_expression_utils.c
 *
 * This is a helper module that helps maintaining a list of identifyers 
 * used as generator variables in set-expressions.
 * This alternative representation is needed to facilitate easy searching
 * for shape information, particularly in the context of nested set expressions
 * The module also provides some basic AST generation abstractions for 
 * derived expressions such as SEUTgenShape for computing the inferred upper bound.
 *
 * Each set expression either
 * case 1:  has a single vector variable { iv -> expr....} or
 * case 2:  a non-empty set of scalar variables { [i,j] -> expr...}.

 * Note here, that all variables can also be dots, i.e. 
 * case 3: { [.,i,.,j] -> expr....} or
 * case 4: { [...,i] -> expr.....}.
 *
 * However, we do not record any dots here as they are irrelevant
 * for the shape inference.
 *
 * SEUTbuildIdTable (vec, oldscope) takes any of the above (1-4) but internally
 * strips away all dots.
 * vec is a node which either should be an N_exprs (cases 2,3,4), or an
 * N_spid (case 1).
 * By providing a non-NULL idtable "oldscope" idtables can be linked
 * to each other to reflect the scoping of nested generator variables.
 * While all identifiers of nested idtables are kept as a linear chain,
 * the scoping nature is internally maintained through additional
 * links.
 * For example:
 *    { [k,.,l] -> { kv -> { [i,...,j] -> expr( i,j,kv, l) ....}}}
 * will internally create an idtable list of the form:
 *
 *   (i,scal) -> (j,scal) -> (kv,vec) -> (k,scal) -> (l,scal) 
 *       |           |        ^   |       ^  |           |
 *       \-----------\--------/   \-------/  \-----------\---> NULL
 *
 * The dual to SEUTbuildIdTable is SEUTfreeIdTable. It frees the entire top 
 * frame(!). Returning to the above example, a single call to SEUTfreeIdTable on the
 * entry (i,scal) will result in:
 *                           (kv,vec) -> (k,scal) -> (l,scal) 
 *                                |       ^  |           |
 *                                \-------/  \-----------\---> NULL
 *
 * Entire chains can be freed by calling SEUTfreeIdTableChain.
 *
 * The attributes (scal/vec) can be accessed through SEUTisVector and SEUTisScalar,
 * respectively.
 *
 * SEUTsearchIdInTable (id, from, to) searches for a given id (string) within a
 * given segment of an idtable chain (from->to). It will return the *first* matching
 * identifier! Assuming that the innermost scope is first in the chain, this 
 * matches the binding scope!
 *
 * SEUTcontainsIdFromTable (expr, from, to) checks whether any identifier contained
 * in the idtable chain between from and to is referred to as N_spid within the
 * expression expr.
 *
 * SEUTcountIds counts the number of idtable entries in the topmost frame!
 * When applied to our example, it will return 2. AFter calling SEUTfreeIdTable
 * as explained above, the result would be 1.
 * 
 * SEUTshapeInfoComplete checks whether for all identifiers in the topmost frame
 * the shape information is non-NULL.
 *
 * SEUTscanSelectionForShapeInfo (idxvec, arg, scope) extracts shape info for
 * all ids that are contained in the idxvec and which are found in the scope.
 * Once found, the corresponding shape info from the arg-shape is inserted
 * into the idtable.
 *
 * SEUTgenShape (identry) builds the shape expression that will serve as upper bound.
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
 * As we change the element ranges from int to size_t we have several values
 * that are of size_t but still need to go into an N_num, ie. need to be
 * int. While the cases in this particular file denote dimensionalities
 * as they occur in shape strings and should never be that high, we want
 * to capture cases if values get out of bound. Hence we wrap downcasts
 * into DBUG_ASSERTS.
 */
static inline int
ConvSI (size_t x) {
 DBUG_ASSERT (x < INT_MAX, "size_t-to-int convertion failed");
 return (int)x;
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
idtable *
SEUTbuildIdTable (node *vec, idtable *oldscope)
{
    idtable *result = oldscope;

    DBUG_ENTER ();

    if (NODE_TYPE (vec) == N_exprs) {
        while (vec != NULL) {
            node *id = EXPRS_EXPR (vec);

            if (NODE_TYPE (id) == N_spid) {
                idtable *newtab = (idtable *)MEMmalloc (sizeof (idtable));
                newtab->id = STRcpy (SPID_NAME (id));
                newtab->type = ID_scalar;
                newtab->shapes = NULL;
                newtab->next = result;
                newtab->nextframe = oldscope;
                result = newtab;
                DBUG_PRINT ("adding scalar id \"%s\"", newtab->id);
            } else if (NODE_TYPE (id) != N_dot) {
                CTIerror (LINE_TO_LOC (global.linenum), "Found neither id nor dot as index in WL set notation");
            }

            vec = EXPRS_NEXT (vec);
        }
    } else if (NODE_TYPE (vec) == N_spid) {
        idtable *newtab = (idtable *)MEMmalloc (sizeof (idtable));
        newtab->id = STRcpy (SPID_NAME (vec)) ;
        newtab->type = ID_vector;
        newtab->shapes = NULL;
        newtab->next = result;
        newtab->nextframe = oldscope;
        result = newtab;
        DBUG_PRINT ("adding vector id \"%s\"", newtab->id);
    } else {
        CTIabort (LINE_TO_LOC (global.linenum), "Malformed index vector in WL set notation");
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

struct CIFTINFO {
    bool contained;
    idtable *from;
    idtable *to;
};

struct RBZINFO {
    node *spid;
    node *subst;
    bool found;
};

struct INFO {
    struct CIFTINFO cift;
    struct RBZINFO rbz;
};

#define INFO_CIFT_CONTAINED(n) ((n)->cift.contained)
#define INFO_CIFT_FROM(n) ((n)->cift.from)
#define INFO_CIFT_TO(n) ((n)->cift.to)

node *ATravCIFTspid (node * arg_node, info * arg_info)
{
  INFO_CIFT_CONTAINED (arg_info)
      = INFO_CIFT_CONTAINED (arg_info)
        || ( SEUTsearchIdInTable (SPID_NAME (arg_node),
                                  INFO_CIFT_FROM (arg_info),
                                  INFO_CIFT_TO (arg_info)) 
             != NULL );
  return arg_node;
}
  

bool SEUTcontainsIdFromTable (node *expr, idtable *from, idtable *to)
{
    info arg_info = {{FALSE,from,to},{NULL,NULL,FALSE}};
    anontrav_t cift_trav[2]
      = {{N_spid, &ATravCIFTspid}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    TRAVpushAnonymous (cift_trav, &TRAVsons);

    expr = TRAVopt (expr, &arg_info);

    TRAVpop ();

    DBUG_RETURN (INFO_CIFT_CONTAINED (&arg_info));
}

int SEUTcountIds (idtable *from)
{
    int res=0;
    idtable *to = from->nextframe;

    DBUG_ENTER ();

    while (from != to) {
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

    shape = TCmakePrf1 (F_shape_A, DUPdoDupTree (vectors->shape));

    expr = TCmakePrf2 (F_sel_VxA, DUPdoDupTree (index), DUPdoDupTree (vectors->shape));

    vectors = vectors->next;

    while (vectors != NULL) {
        shape = TCmakePrf2 (F_min_VxV,
                            TCmakePrf1 (F_shape_A, DUPdoDupTree (vectors->shape)),
                            shape);
        expr = TCmakePrf2 (F_min_SxS,
                           TCmakePrf2 (F_sel_VxA,
                                       DUPdoDupTree (index),
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
 * builds the shape for the set expression. For each
 * identifier within the vector the shape is built as the
 * minimum of all shapes of arrays the identifier is used with within a
 * selection. If the vector is given as a single vector,
 * BuildMinShapeVector is used instead of primitive functions.
 *
 * @param table idtable structure
 * @param end first identifier within idtable not belonging to this lamination
 * @return sac code representing the shape vector
 */
node * SEUTgenShape (idtable *table)
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
                CTIerror (LINE_TO_LOC (global.linenum),
                          "No shape information found for index scalar '%s'.",
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
            CTIerror (LINE_TO_LOC (global.linenum),
                      "No shape information found for index vector '%s'.",
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
            DBUG_PRINT ("info for \"%s\"...", table->id);
            if (table->shapes == NULL) {
                result = FALSE;
                DBUG_PRINT ("   ... missing");
            } else {
                DBUG_PRINT ("   ... found");
            }
            table = table->next;
        }

    } else if (table->type == ID_vector) {
        DBUG_PRINT ("info for \"%s\"...", table->id);
        if (table->shapes == NULL) {
            result = FALSE;
            DBUG_PRINT ("   ... missing");
        } else {
            DBUG_PRINT ("   ... found");
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
void SEUTscanSelectionForShapeInfo (node *idxvec, node *array, idtable *scope)
{
    int poscnt = 0;
    int tripledotflag = 0;
    size_t exprslen;
    idtable *entry;
    node *position;
    shpchain *chain;

    DBUG_ENTER ();

    DBUG_PRINT ("scanning for shape info in selection");
    if ((scope != NULL) && !SEUTcontainsIdFromTable (array, scope, scope->nextframe)) {
        if (NODE_TYPE (idxvec) == N_array) {
            idxvec = ARRAY_AELEMS (idxvec);
            exprslen = TCcountExprs (idxvec);
        
            while (idxvec != NULL) {
                if (NODE_TYPE (EXPRS_EXPR (idxvec)) == N_spid) {
                    entry = SEUTsearchIdInTable (SPID_NAME (EXPRS_EXPR (idxvec)), scope, NULL);

                    if (entry != NULL) {
                        DBUG_PRINT ("id \"%s\" found", entry->id);

                        if (entry->type == ID_scalar) {

                            if (tripledotflag) {
                                position
                                  = TCmakePrf2 (F_sub_SxS,
                                                TCmakePrf1 (F_dim_A,
                                                            DUPdoDupTree (array)),
                                                TBmakeNum (ConvSI (exprslen) - poscnt));
                            } else {
                                position = TBmakeNum (poscnt);
                            }
                            chain = (shpchain *)MEMmalloc (sizeof (shpchain));
                            chain->shape
                              = TCmakePrf2 (F_sel_VxA,
                                            TCmakeIntVector (TBmakeExprs (position, NULL)),
                                            TCmakePrf1 (F_shape_A,
                                                        DUPdoDupTree (array)));
                            chain->next = entry->shapes;
                            entry->shapes = chain;
                            DBUG_PRINT ("shape added:");
                            DBUG_EXECUTE (PRTdoPrintFile (stderr, chain->shape));

                        } else {
                            CTIerror (NODE_LOCATION (idxvec),
                                      "Set notation index vector '%s' is used in a scalar context.",
                                      entry->id);
                        }
                    }
                } else if ((NODE_TYPE (EXPRS_EXPR (idxvec)) == N_dot)
                            && (DOT_NUM (EXPRS_EXPR (idxvec)) == 3)) {
                    tripledotflag = 1;
                }

                poscnt++;
                idxvec = EXPRS_NEXT (idxvec);
            }
        } else if (NODE_TYPE (idxvec) == N_spid) {

            entry = SEUTsearchIdInTable (SPID_NAME (idxvec), scope, NULL);
            if (entry != NULL) {
                chain = (shpchain *)MEMmalloc (sizeof (shpchain));
                chain->next = entry->shapes;
                entry->shapes = chain;
                DBUG_PRINT ("id \"%s\" found", entry->id);

                if (entry->type == ID_vector) {
                    chain->shape = TCmakePrf1 (F_shape_A, DUPdoDupTree (array));
                } else {
                    //scalar used in vector position, i.e., a[0] case!
                    chain->shape = TCmakePrf2 (F_sel_VxA,
                                               TCcreateIntVector (1, 0, 1),
                                               TCmakePrf1 (F_shape_A, DUPdoDupTree (array)));
                }
                DBUG_PRINT ("shape added:");
                DBUG_EXECUTE (PRTdoPrintFile (stderr, chain->shape));
            }
        }
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Below is the substitution mechanism which is needed to replace generator
 * variables either by zeros or vectors of zeros.
 */

#define INFO_RBZ_SPID(n) ((n)->rbz.spid)
#define INFO_RBZ_SUBST(n) ((n)->rbz.subst)
#define INFO_RBZ_FOUND(n) ((n)->rbz.found)

static node *
ATravRBZspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (STReq (SPID_NAME (INFO_RBZ_SPID (arg_info)), SPID_NAME (arg_node))) {
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = DUPdoDupTree (INFO_RBZ_SUBST (arg_info));
    }

    DBUG_RETURN (arg_node);
}

static bool
SpidInIdxs (node *spid, node *idxs)
{
    bool res = FALSE;
    node * idx;

    DBUG_ENTER ();

    if (NODE_TYPE (idxs) == N_spid) {
        res = STReq (SPID_NAME (spid), SPID_NAME (idxs));
    } else {
        do {
            idx = EXPRS_EXPR (idxs);
            res = res || STReq (SPID_NAME (spid), SPID_NAME (idx));
            idxs = EXPRS_NEXT (idxs);
        } while ((res == FALSE) && (idxs != NULL));
    }

    DBUG_RETURN (res);
}

static node *
ATravRBZsetwl (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Make sure we do not replace locally bound variables!
     */
    if (!SpidInIdxs (INFO_RBZ_SPID (arg_info), SETWL_VEC (arg_node))) {
        SETWL_EXPR (arg_node) = TRAVdo (SETWL_EXPR (arg_node), arg_info);
    }
    SETWL_GENERATOR (arg_node) = TRAVopt (SETWL_GENERATOR (arg_node), arg_info);
    SETWL_NEXT (arg_node) = TRAVopt (SETWL_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
SubstituteSpid (node *expr, node *spid, node *subst)
{
    anontrav_t rbz_trav[3] = {{N_spid, &ATravRBZspid},
                              {N_setwl, &ATravRBZsetwl},
                              {(nodetype)0, NULL}};
    info arg_info = {{FALSE,NULL,NULL},{spid,subst,FALSE}};

    DBUG_ENTER ();

    TRAVpushAnonymous (rbz_trav, &TRAVsons);

    expr = TRAVopt (expr, &arg_info);

    TRAVpop ();

    DBUG_RETURN (expr);
}


node *
SEUTsubstituteIdxs (node *expr, node *idxs, node *subst)
{
    node *idx;
    DBUG_ENTER ();

    if (NODE_TYPE (idxs) == N_spid) {
        expr = SubstituteSpid (expr, idxs, subst);
    } else {
        do {
            idx = EXPRS_EXPR (idxs);
            if (NODE_TYPE (idx) == N_spid) {
                expr = SubstituteSpid (expr, idx, subst);
            }
            // skip N_dot expressions
            idxs = EXPRS_NEXT (idxs);
        } while (idxs != NULL);
    }

    DBUG_RETURN (expr);
}

/******************************************************************************
 *
 * Below is the search mechanism which is needed to find relatively free
 * variables in expressions. It piggy-backs on the substitution mechanism
 * above and shares the anti-shadowing mechanism (ATravRBZsetwl)
 */
static node *
ATravRBZspid2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (STReq (SPID_NAME (INFO_RBZ_SPID (arg_info)), SPID_NAME (arg_node))) {
        INFO_RBZ_FOUND (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

static bool
FindSpid (node *expr, node *spid)
{
    anontrav_t rbz_trav[3] = {{N_spid, &ATravRBZspid2},
                              {N_setwl, &ATravRBZsetwl},
                              {(nodetype)0, NULL}};
    info arg_info = {{FALSE,NULL,NULL},{spid,NULL,FALSE}};

    DBUG_ENTER ();

    DBUG_PRINT ("searching for free occurrence of variable \"%s\" in",
                SPID_NAME (spid));
    DBUG_EXECUTE (PRTdoPrint (expr));

    TRAVpushAnonymous (rbz_trav, &TRAVsons);

    expr = TRAVopt (expr, &arg_info);

    TRAVpop ();

    DBUG_RETURN (INFO_RBZ_FOUND (&arg_info));
}

bool
SEUTcontainsIdxs (node *expr, node *idxs)
{
    node *idx;
    bool found=FALSE;
    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (idxs) == N_spid) || (NODE_TYPE (idxs) == N_exprs)),
                 "idxs arg of SEUTcontainsIdxs not legel SETWL_VEC entry!");
    if (NODE_TYPE (idxs) == N_spid) {
        found = FindSpid (expr, idxs);
    } else {
        do {
            idx = EXPRS_EXPR (idxs);
            if (NODE_TYPE (idx) == N_spid) {
                found = found || FindSpid (expr, idx);
            }
            // skip N_dot expressions!
            idxs = EXPRS_NEXT (idxs);
        } while (idxs != NULL);
    }

    DBUG_RETURN (found);
}

node *
SEUTbuildSimpleWl (node *shape, node *def)
{
    node *result = NULL;
    DBUG_ENTER ();

    result
      = TBmakeWith (TBmakePart (NULL,
                                TBmakeWithid (TBmakeSpids (TRAVtmpVar (), NULL), NULL),
                                TBmakeGenerator (F_wl_le, F_wl_le, TBmakeDot (1),
                                                 TBmakeDot (1), NULL, NULL)),
                    TBmakeCode (MAKE_EMPTY_BLOCK (),
                                TBmakeExprs (def, NULL)),
                    TBmakeGenarray (shape, NULL));

    GENARRAY_DEFAULT (WITH_WITHOP (result)) = DUPdoDupTree (def);

    CODE_USED (WITH_CODE (result))++;
    PART_CODE (WITH_PART (result)) = WITH_CODE (result);

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
