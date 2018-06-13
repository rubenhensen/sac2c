#include "set_expression_dots.h"
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
 * arg_info in this file:
 * IDS:    N_exprs list of lhs identifiers of the setwl.
 * LENTD:  an expression that computes the length of the triple-dot
 *         match (potentially at runtime)
 */

/* INFO structure */
struct INFO {
    node *ids;
    node *lentd;
};

/* access macros */
#define INFO_HSED_IDS(n) ((n)->ids)
#define INFO_HSED_LENTD(n) ((n)->lentd)

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

    INFO_HSED_IDS (result) = NULL;
    INFO_HSED_LENTD (result) = NULL;

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

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_setwl,
                "SERIdoInferRanges called on non-Nsetwl node!");

    arg_info = MakeInfo ();
    TRAVpush (TR_seri);

    arg_node = TRAVdo (arg_node, NULL);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

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
            ScanVector (ARRAY_AELEMS (PRF_ARG1 (arg_node)), PRF_ARG2 (arg_node),
                        arg_info);
        }
        else if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_spid) {
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

    /* if in HSE_scan mode, scan for shapes */

    if (STReq (SPAP_NAME (arg_node), "sel")
        && (SPAP_NS (arg_node) == NULL)) {
        if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_array) {
            ScanVector (ARRAY_AELEMS (SPAP_ARG1 (arg_node)), SPAP_ARG2 (arg_node),
                        arg_info);
        }
        else if (NODE_TYPE (SPAP_ARG1 (arg_node)) == N_spid) {
            ScanId (SPAP_ARG1 (arg_node), SPAP_ARG2 (arg_node), arg_info);
        }
    } else {

        /* Otherwise, look for info inside */

        arg_node = TRAVdo (arg_node, arg_info);
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
SERIsetwl (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("looking at Set-Expression in line %d:", global.linenum);
    arg_info = MakeInfo (arg_info);

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
        INFO_SERI_IDTABLE (arg_info) = BuildIdTable (ids, INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info)));
    } else {
        INFO_SERI_IDTABLE (arg_info) = INFO_SERI_OLDIDTABLE (INFO_SERI_NEXT (arg_info));
    }
    DBUG_PRINT( "traversing expression...");
    SETWL_EXPR (arg_node) = TRAV (SETWL_EXPR (arg_node), arg_info);
    if (SETWL_GENERATOR (arg_node) == NULL) {
        SETWL_GENERATOR (arg_node) = TBmakeGenerator (F_noop, F_noop, NULL, NULL, NULL, NULL);
    }
    INFO_SERI_ISLASTPART (arg_info) = (SETWL_NEXT (arg_node) == NULL);
    DBUG_PRINT( "traversing generator...");
    SETWL_GENERATOR (arg_node) = TRAV (SETWL_GENERATOR (arg_node), arg_info);

    SETWL_NEXT (arg_node) = TRAVopt (SETWL_NEXT (arg_node, arg_info));

    FreeIdTable (INFO_SERI_IDTABLE (arg_info), INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info)));
    INFO_SERI_IDTABLE (arg_info) = NULL;
    arg_info = FreeInfo (arg_info);

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

    DBUG_PRINT ("traversing generator...");
    if (INFO_SERI_LBMISSING (arg_info)) {
        if ( FALSE) {
            /* We now insert the information we found */
        } else {
            CTIwarning ("Unable to infer lower bound for a partition of"
                        " a set expression; using \".\" instead.");
            GENERATOR_BOUND1 (arg_node) = TBmakeDot (1);
            GENERATOR_OP1 (arg_node) = F_wl_le;
        }
        
    }
    if (INFO_SERI_LBMISSING (arg_info)) {
        if ( FALSE) {
            /* We now insert the information we found */
        } else {
            if (INFO_SERI_ISLASTPART (arg_info) {
                CTIerror ("Unable to infer upper bound for final partition of"
                          " set expression; please specify an upper bound.");
            } else {
                CTIwarning ("Unable to infer upper bound for a partition of"
                            " a set expression; using \".\" instead.");
                GENERATOR_BOUND1 (arg_node) = TBmakeDot (1);
                GENERATOR_OP1 (arg_node) = F_wl_le;
            }
        }
        
    }
    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
