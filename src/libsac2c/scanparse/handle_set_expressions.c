#include "handle_set_expressions.h"
#include "traverse.h"

#define DBUG_PREFIX "HSE"
#include "debug.h"

#include "print.h"
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
    bool fullpart;
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
#define INFO_HSE_FULLPART(n) ((n)->fullpart)
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
    INFO_HSE_FULLPART (result) = FALSE;
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
BuildSimpleWl( node *shape, node *def)
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


static node *
ATravRBZspid( node *arg_node, info *arg_info)
{
    node *handle;
    DBUG_ENTER ();

    handle = INFO_HSE_VEC (arg_info);

    if (handle != NULL) {
        if (NODE_TYPE (handle) == N_spid) {
            /* we are dealing with the vector variable case */
            if (STReq (SPID_NAME (handle), SPID_NAME (arg_node))) {
                node *gen_shape, *gen_default;

                gen_shape = TCmakePrf1(
                                F_shape_A,
                                DUPdoDupTree (GENERATOR_BOUND2 (PART_GENERATOR (INFO_HSE_PART (arg_info)))));
                gen_default = TBmakeNum (0);
                arg_node = FREEdoFreeTree( arg_node);
                arg_node = BuildSimpleWl( gen_shape, gen_default);
            }
            
        } else {
            /* we are dealing with the scalar variables case */
            while (handle != NULL) {
                if (STReq (SPID_NAME (EXPRS_EXPR (handle)), SPID_NAME (arg_node))) {
                    arg_node = FREEdoFreeTree( arg_node);
                    arg_node = TBmakeNum( 0);
                    handle = NULL;
                } else {
                    handle = EXPRS_NEXT (handle);
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}


static node *
ReplaceByZeros( node *arg_node, info *arg_info)
{
    anontrav_t rbz_trav[2]
      = {{N_spid, &ATravRBZspid}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    TRAVpushAnonymous (rbz_trav, &TRAVsons);

    arg_node = TRAVopt (arg_node, arg_info);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/**
 * builds a withloop generating an array of the same shape
 * as the shape of "expr" whose elements are all "zero( expr)" .
 * It is assumed that expr does NOT contain any references to
 * the generator variables!
 *
 * @param expr AST copy of the expression
 */

static node *
BuildDefault (node *expr)
{
    node *shape, *def;
    DBUG_ENTER ();

    DBUG_PRINT ("Building Default Element WL from expression:");
    DBUG_EXECUTE (PRTdoPrint( expr));

    shape = TCmakePrf1( F_shape_A, expr);
    def = TCmakeSpap1 (NSgetNamespace (global.preludename),
                       STRcpy ("zero"),
                       DUPdoDupTree (expr));
    DBUG_RETURN (BuildSimpleWl (shape, def));
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

    arg_info = MakeInfo (NULL);

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
    node *handle, *idx;
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    handle = INFO_HSE_VEC (arg_info);
    if (handle != NULL) {
        INFO_HSE_COPY_FROM (arg_info) = NULL;

        if (STReq (SPID_NAME (SPAP_ID (arg_node)), "sel")
            && (NODE_TYPE (EXPRS_EXPR2 (SPAP_ARGS (arg_node))) == N_spid) ) {

            idx = EXPRS_EXPR1 (SPAP_ARGS (arg_node));

            if ((NODE_TYPE (handle) == N_spid) && (NODE_TYPE (idx) == N_spid)
                && STReq (SPID_NAME (handle), SPID_NAME (idx))) {
                INFO_HSE_COPY_FROM (arg_info) = EXPRS_EXPR2 (SPAP_ARGS (arg_node));

            } else if (NODE_TYPE (idx) == N_array) {
                idx = ARRAY_AELEMS (idx);
                INFO_HSE_COPY_FROM (arg_info) = EXPRS_EXPR2 (SPAP_ARGS (arg_node));
                while ((handle != NULL) && (idx != NULL)) {
                    if ((NODE_TYPE (EXPRS_EXPR( idx)) == N_spid)
                        && STReq (SPID_NAME (EXPRS_EXPR (handle)), SPID_NAME ( EXPRS_EXPR (idx)))) {
                        handle = EXPRS_NEXT (handle);
                        idx = EXPRS_NEXT (idx);
                    } else {
                        INFO_HSE_COPY_FROM (arg_info) = NULL;
                        handle = NULL;
                    }
                }
            }
        }
    }

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
 * Used to scan selections for ids found in a prior lamination.
 * Depending on the type of the selection vector, ScanId or ScanVector
 * is called.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HSEgenerator (node *arg_node, info *arg_info)
{
    node *handle;
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);
    
    INFO_HSE_FULLPART (arg_info) = FALSE;
    if ((GENERATOR_STEP (arg_node) == NULL)
        && (GENERATOR_WIDTH (arg_node) == NULL)
        && (GENERATOR_OP1 (arg_node) == F_wl_le)) {
        if (NODE_TYPE( GENERATOR_BOUND1 (arg_node)) == N_dot) {
            INFO_HSE_FULLPART (arg_info) = TRUE;
        } else if (NODE_TYPE( GENERATOR_BOUND1 (arg_node)) == N_array) {
            handle = ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node));
            INFO_HSE_FULLPART (arg_info) = TRUE;
            while (handle != NULL) {
                if ((NODE_TYPE (EXPRS_EXPR (handle)) == N_num)
                    && (NUM_VAL (EXPRS_EXPR (handle)) == 0)) {
                    handle = EXPRS_NEXT (handle);
                } else {
                    INFO_HSE_FULLPART (arg_info) = FALSE;
                    handle = NULL;
                }
            }
        }
    }
    

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
        DBUG_PRINT ("looking at Set-Expression in line %zu:", global.linenum);
        oldinfo = arg_info;
        arg_info = MakeInfo (oldinfo);
    } else {
        DBUG_PRINT ("next partition of Set-Expression in line %zu:", global.linenum);
        INFO_HSE_OUTSIDE (arg_info) = TRUE;
    }

    INFO_HSE_LASTPART (arg_info) = (SETWL_NEXT (arg_node) == NULL);
    INFO_HSE_VEC (arg_info) = SETWL_VEC (arg_node);

    /* traverse the generator (may contain setwls!) 
     * This traversal also infers INFO_HSE_FULLPART
     * in case INFO_HSE_LASTPART is TRUE!
     */
    DBUG_PRINT ("traversing generator");
    SETWL_GENERATOR (arg_node) = TRAVdo (SETWL_GENERATOR (arg_node), arg_info);
    DBUG_PRINT ("generator is %s", (INFO_HSE_FULLPART (arg_info) ? "full" : "not full"));

    /* traverse the expression (may contain setwls!)
     * This traversal also infers INFO_HSE_GENREF and INFO_HSE_COPY_FROM
     * in case INFO_HSE_LASTPART is TRUE!
     */
    DBUG_PRINT ("traversing expression");
    INFO_HSE_GENREF (arg_info) = FALSE;
    SETWL_EXPR (arg_node) = TRAVdo (SETWL_EXPR (arg_node), arg_info);
    DBUG_PRINT( "generator variable %s in expression!",
                (INFO_HSE_GENREF (arg_info)? "found" : "not found"));
    DBUG_PRINT( "expression is %s%s!",
                (INFO_HSE_COPY_FROM (arg_info) == NULL ? "not a copy partition" : "copy of "),
                (INFO_HSE_COPY_FROM (arg_info) == NULL ? "" : SPID_NAME (INFO_HSE_COPY_FROM (arg_info))));
    
    /* We build the WLs bottom up! */
    if (SETWL_NEXT (arg_node) != NULL) {
        INFO_HSE_OUTSIDE (arg_info) = FALSE;
        SETWL_NEXT (arg_node) = TRAVdo (SETWL_NEXT (arg_node), arg_info);
        INFO_HSE_LASTPART (arg_info) = FALSE;
    }

    /* Now, we add a new partition to arg_info, iff we are 
     *   not the last partition
     *   or, in case we are the last part, we refer to the generator variable(s)
     *   and do so in a way different from a copying expression (ie a[iv])
     */
    if (!INFO_HSE_LASTPART (arg_info) || !INFO_HSE_FULLPART (arg_info)
         || (INFO_HSE_GENREF (arg_info) && (INFO_HSE_COPY_FROM (arg_info) == NULL))) {
        DBUG_PRINT ("building code and partition");

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
        DBUG_PRINT ("partition inserted:");
        DBUG_EXECUTE (PRTdoPrint( part));

    }

    /* if we are the last partition, we generate the operator part now: */
    if (INFO_HSE_LASTPART (arg_info)) {
        DBUG_PRINT ("building withop");
        if (INFO_HSE_FULLPART (arg_info) && (INFO_HSE_COPY_FROM (arg_info) != NULL)) {
            /* we can generate a modarray WL (the last partition has not been generated!) */
            INFO_HSE_WITHOP (arg_info) = TBmakeModarray (DUPdoDupTree (INFO_HSE_COPY_FROM (arg_info)));
        } else if (INFO_HSE_FULLPART (arg_info) && !INFO_HSE_GENREF (arg_info)) {
            /* As there is no reference to the generator, we use the expression as default */
            INFO_HSE_WITHOP (arg_info) = TBmakeGenarray (DUPdoDupTree (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))),
                                                         DUPdoDupTree (SETWL_EXPR (arg_node)) );
        } else {
            INFO_HSE_WITHOP (arg_info)
                = TBmakeGenarray (DUPdoDupTree (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))),
                                  BuildDefault (ReplaceByZeros (DUPdoDupTree (SETWL_EXPR (arg_node)), arg_info)) );
        }
        DBUG_PRINT ("withop inserted:");
        DBUG_EXECUTE (PRTdoPrint (INFO_HSE_WITHOP (arg_info)));
    }

    arg_node = FREEdoFreeTree (arg_node);

    if (oldinfo != NULL) {
        arg_node = TBmakeWith (INFO_HSE_PART( arg_info), 
                               INFO_HSE_CODE( arg_info),
                               INFO_HSE_WITHOP( arg_info));
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
    node *handle;
    bool found;
    DBUG_ENTER ();

    handle = INFO_HSE_VEC (arg_info);

    if (handle != NULL) {
        if (NODE_TYPE (handle) == N_spid) {
            /* we are dealing with the vector variable case */
            found = STReq (SPID_NAME (handle), SPID_NAME (arg_node));
            INFO_HSE_GENREF (arg_info) = INFO_HSE_GENREF (arg_info) || found;
        } else {
            /* we are dealing with the scalar variables case */
            while (handle != NULL) {
                found = STReq (SPID_NAME (EXPRS_EXPR (handle)), SPID_NAME (arg_node));
                INFO_HSE_GENREF (arg_info) = INFO_HSE_GENREF (arg_info) || found;
                handle = EXPRS_NEXT (handle);
            }
        }
    }
    
    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
