#include "handle_set_expression_dots.h"
#include "traverse.h"

#define DBUG_PREFIX "HSED"
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


/** <!--********************************************************************-->
 * checks for any occurrences of a dot symbol within a set notation
 * selection vector.
 *
 * @param ids EXPRS node containing ids
 * @return number of dots found
 *****************************************************************************/
static int
CountDotsInVector (node *ids, int n)
{
    int result = 0;

    DBUG_ENTER ();

    if (NODE_TYPE (ids) != N_exprs) {
        result = 0;
    } else {
        while (ids != NULL) {
            if ((NODE_TYPE (EXPRS_EXPR (ids)) == N_dot)
                && (DOT_NUM (EXPRS_EXPR (ids)) == n))
                result++;
            ids = EXPRS_NEXT (ids);
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * counts the number of triple dots contained in
 * selection vector.
 *
 * @param ids EXPRS node containing ids
 * @return number of triple dots found
 *****************************************************************************/
static int
CountTripleDots (node *ids)
{
    DBUG_ENTER ();

    DBUG_RETURN (CountDotsInVector (ids, 3));
}

/** <!--********************************************************************-->
 * traverses spids vector and replaces dots by new spids and creates an 
 * N_exprs chain with these spids as return value
 *
 * @param exprs EXPR node containing ids
 * @return N_exprs chain of new spids
 *****************************************************************************/
static node *
HandleSingleDots (node *exprs)
{
    DBUG_ENTER ();
    node *array=NULL;
    node *spid;

    if (EXPRS_NEXT(exprs) != NULL)  {
        array = HandleSingleDots (EXPRS_NEXT(exprs));
    }
    if( NODE_TYPE( EXPRS_EXPR( exprs)) == N_dot) {
        EXPRS_EXPR( exprs) = FREEdoFreeNode (EXPRS_EXPR( exprs));
        spid = MakeTmpId( "dot");
        EXPRS_EXPR( exprs) = spid;

        array = TBmakeExprs( DUPdoDupNode( spid), array);
    }
    DBUG_RETURN (array);
}

static node *
HandleSingleDotsBeforeTriple (node *exprs, node *vec, int pos)
{
    DBUG_ENTER ();
    node *array=NULL;

    if (NODE_TYPE (EXPRS_EXPR (exprs)) != N_dot ) {
        array = HandleSingleDotsBeforeTriple (EXPRS_NEXT(exprs), vec, pos+1);
    } else if (DOT_NUM (EXPRS_EXPR (exprs)) != 3) {
        array = HandleSingleDotsBeforeTriple (EXPRS_NEXT(exprs), vec, pos+1);

        array = TBmakeExprs( TCmakePrf2 (F_sel_VxA,
                                         TCmakeIntVector (
                                             TBmakeExprs( TBmakeNum( pos), NULL)),
                                         DUPdoDupNode( vec)),
                             array);
    }
    DBUG_RETURN (array);
}

static node *
HandleSingleDotsBehindTriple( node *exprs, node *vec, int pos, node *len_td)
{
    DBUG_ENTER ();
    node *array=NULL;

    if (EXPRS_NEXT(exprs) != NULL)  {
        array = HandleSingleDotsBehindTriple (EXPRS_NEXT(exprs), vec, pos+1, len_td);
    }
    if( NODE_TYPE( EXPRS_EXPR( exprs)) == N_dot) {
        array = TBmakeExprs( TCmakePrf2 (F_sel_VxA,
                                         TCmakeIntVector (
                                             TBmakeExprs (
                                                 TCmakePrf2 (F_add_SxS,
                                                             DUPdoDupTree (len_td),
                                                             TBmakeNum( pos)),
                                                 NULL)),
                                         DUPdoDupNode( vec)),
                             array);
    }
    DBUG_RETURN (array);
}

static int
FindPos( char *name, node  *exprs)
{
  int pos;
  pos = 0;
  DBUG_ENTER();

  while ((exprs!= NULL) 
         && ((NODE_TYPE (EXPRS_EXPR (exprs)) != N_spid)
            || !STReq( SPID_NAME (EXPRS_EXPR (exprs)), name))) {
    pos++;
    exprs = EXPRS_NEXT (exprs);
  }
  if (exprs == NULL) {
    pos = -1;
  }
  DBUG_RETURN( pos);
}

static node *
ATravSetSpidZero (node *arg_node, info *arg_info)
{
  int pos;
  DBUG_ENTER ();
  pos = FindPos (SPID_NAME (arg_node), INFO_HSED_IDS (arg_info));
  if (pos>=0) {
    arg_node = FREEdoFreeNode( arg_node);
    arg_node = TBmakeNum( 0);
  }
  DBUG_RETURN (arg_node);
}


static node *
ReplaceIdsZero( node *expr, node *ids)
{
    info *arg_info;
    anontrav_t riz_trav[2] = {{N_spid, &ATravSetSpidZero}, {(nodetype)0, NULL}};
    DBUG_ENTER ();

    TRAVpushAnonymous (riz_trav, &TRAVsons);

    arg_info = MakeInfo ();
    INFO_HSED_IDS (arg_info) = ids;
    expr = TRAVopt (expr, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();
    DBUG_RETURN (expr);
}

static node *
ATravSetSpidSel (node *arg_node, info *arg_info)
{
  int pos;
  DBUG_ENTER ();
  pos = FindPos (SPID_NAME (arg_node), INFO_HSED_IDS (arg_info));
  if (pos>=0) {
    arg_node = FREEdoFreeNode( arg_node);
    arg_node = TBmakeNum( 0);
  }
  DBUG_RETURN (arg_node);
}

static node *
ReplaceIdsSel( node *expr, node *ids, node *len_td)
{
    info *arg_info;
    anontrav_t riz_trav[2] = {{N_spid, &ATravSetSpidSel}, {(nodetype)0, NULL}};
    DBUG_ENTER ();

    TRAVpushAnonymous (riz_trav, &TRAVsons);

    arg_info = MakeInfo ();
    INFO_HSED_IDS (arg_info) = ids;
    INFO_HSED_LENTD (arg_info) = len_td;
    expr = TRAVopt (expr, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();
    DBUG_RETURN (expr);
}


/** <!--********************************************************************-->
 * counts the number of single dots contained in
 * selection vector.
 *
 * @param ids EXPRS node containing ids
 * @return number of single dots found
 *****************************************************************************/
static int
CountSingleDots (node *ids)
{
    DBUG_ENTER ();

    DBUG_RETURN (CountDotsInVector (ids, 1));
}

/**
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
HSEDdoEliminateSetExpressionDots (node *arg_node)
{
    DBUG_ENTER ();

    TRAVpush (TR_hsed);

    arg_node = TRAVdo (arg_node, NULL);

    TRAVpop ();

    CTIabortOnError ();

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
HSEDsetwl (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    int num_sd;
    int num_td;
    int td_pos;
    node *ids;
    node *td_ids;
    node *array;
    node *len_td;
    node *spid;
    node *array1;
    node *array2;
    node *expr_p;
    node *expr_pp;
    node *index;

    DBUG_PRINT ("looking at Set-Expression in line %d:", global.linenum);
    /* Bottom up traversal! */
    DBUG_PRINT ("handling body ... ");
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_PRINT( "body done ... ");
    /* Now, we look at this setwl! */
    ids = SETWL_VEC (arg_node);
    num_sd = CountSingleDots (ids);
    num_td = CountTripleDots (ids);

    if (num_td == 1) {
        DBUG_PRINT (" triple dot with %d single dots found!", num_sd);
        spid = MakeTmpId( "td_vec");

        /*
         * traverse all ids before the "..." and return an array
         * of selections into "td_vec" according to their position
         */
        array1 = TCmakeIntVector
                     (HandleSingleDotsBeforeTriple (ids, spid, 0));

        /* find the "..." */
        td_pos = 0;
        td_ids = ids;
        while (( NODE_TYPE (EXPRS_EXPR (td_ids)) != N_dot)
               || (DOT_NUM (EXPRS_EXPR (td_ids)) == 1) ) {
            td_ids = EXPRS_NEXT (td_ids);
            td_pos++;
        }

       /*
        * Now traverse all ids behind the "..." and return an array
        * of selections into "td_vec" according to their position and
        * the number of dimensions (len_td) matched by the triple dots.
        * We have      len_td = dim( expr'') - num_sd
        * where expr'' is expr with all references to ids replaced
        * by scalar 0. NB: is this really safe to do?
        */
       expr_pp = ReplaceIdsZero (DUPdoDupTree (SETWL_EXPR (arg_node)), ids);
       len_td = TCmakePrf2 (F_sub_SxS,
                            TCmakePrf1 (F_dim_A, expr_pp),
                            TBmakeNum( num_sd));
       array2 = TCmakeIntVector
                    (HandleSingleDotsBehindTriple
                        (EXPRS_NEXT (td_ids), spid, td_pos, len_td));
        
       /*
        * Finally, we modify the original expression expr into expr' to use selections
        * into spid rather than scalar ids and we insert the overall selection:
        *     expr'[ _cat_VxV_( _cat_VxV_( array1,
        *                                  drop( td_pos+1-len_ids, drop( td_pos, spid))),
        *                                  array2) ]
        */
       expr_p = ReplaceIdsSel (SETWL_EXPR (arg_node), ids, len_td);
       index = TCmakePrf2
                   (F_cat_VxV,
                   TCmakePrf2
                       (F_cat_VxV,
                       array1,
                       TCmakePrf2
                           (F_drop_SxV,
                           TBmakeNum (td_pos+1-TCcountExprs(ids)),
                           TCmakePrf2
                               (F_drop_SxV,
                               TBmakeNum (td_pos),
                               spid)) ),
                   array2);
       SETWL_EXPR (arg_node) = TCmakeSpap2 (NULL, strdup ("sel"),
                                           index,
                                           SETWL_EXPR (arg_node));
       /*
        * and we replace the ids by spid:
        */
       SETWL_VEC (arg_node) = FREEdoFreeTree (SETWL_VEC (arg_node));
       SETWL_VEC (arg_node) = spid;
   
    } else if (num_td > 1) {
      CTIerrorLine( global.linenum,
                    " triple-dot notation used more than once in array comprehension;");

    } else if (num_sd >0) {
      /* We have at least one single dot but no triple dots! */
      DBUG_PRINT (" no triple dots but %d single dots found!", num_sd);

      /*
       * traverse the ids, replace dots with vars and create 
       * array of those variables.
       */

      array = HandleSingleDots( ids);
      
      SETWL_EXPR (arg_node) = TCmakeSpap2 (NULL, strdup ("sel"),
                                           TCmakeIntVector (array),
                                           SETWL_EXPR (arg_node));

    }  else {
      DBUG_PRINT (" no dots found!");
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
