/*
 *
 * $Log$
 * Revision 1.72  1998/08/20 12:20:30  srs
 * added cf_expr++ in ArrayPrf() (case F_modarray)
 *
 * Revision 1.71  1998/08/06 18:35:12  srs
 * removed comments (cleanup)
 *
 * Revision 1.70  1998/07/23 10:02:46  srs
 * fixed bug from version 1.69 again
 *
 * Revision 1.69  1998/07/20 19:40:13  srs
 * fixed a bug in ArrayPrf(), case F_modarray.
 *
 * Revision 1.68  1998/07/14 12:57:18  srs
 * deactivated folding of reshape()
 *
 * Revision 1.67  1998/06/19 13:19:52  srs
 * added new case to ArrayPrf().
 * Prf F_modarray now folds sequently defined arrays:
 * modarray([1,2,3],[1],42) ==> [1,42,3]
 *
 * Revision 1.66  1998/06/02 17:08:03  sbs
 * CF now eliminates F-reshape-calls
 * (short & dirty hack in ArrayPrf!)
 *
 * Revision 1.65  1998/05/13 12:06:52  srs
 * enhanced Elemination of F_psi in ArrayPrf()
 *
 * Revision 1.64  1998/04/29 08:53:51  srs
 * patched ArrayPrf() to call it from typecheck.c
 *
 * Revision 1.63  1998/04/20 08:55:51  srs
 * added incrementations of cf_expr
 *
 * Revision 1.62  1998/04/01 07:37:56  srs
 * renames INFO_* macros,
 * added struct INSIDE_WL
 * changed CFId: no dirty/wrong trick (N_with) is used for N_Nwith
 *
 * Revision 1.61  1998/03/25 12:34:48  srs
 * changed CFNwith
 *
 * Revision 1.60  1998/03/17 12:21:29  cg
 * Bug fixed in DupPartialArray. Now, the type information is
 * copied as well.
 *
 * Revision 1.59  1998/03/10 17:30:55  srs
 * improved orthography
 *
 * Revision 1.58  1998/03/04 15:36:12  srs
 * added N_Npart node to switch in CFid
 *
 * Revision 1.57  1998/02/23 13:08:32  srs
 * changed CFid to support new WLs,
 * added comments
 *
 * Revision 1.56  1998/02/15 21:31:42  srs
 * added CF for new WL
 *
 * Revision 1.55  1998/02/12 11:09:53  srs
 * removed NEWTREE
 * replaced some direct accesses with access macros
 *
 * Revision 1.54  1997/11/07 10:24:13  dkr
 * with defined NEWTREE node.nnode is not used anymore
 *
 * Revision 1.53  1997/10/09 13:56:59  srs
 * modified SkalarPrf to fold F_min and F_max
 *
 * Revision 1.52  1997/10/05 15:25:40  dkr
 * added CF in function SkalarPrf() for % and abs()
 *
 * Revision 1.51  1997/09/05 18:31:38  dkr
 * removed a bug in function CFid.
 * constant-folding now does not lose type-shapes after function-inlining anymore
 *
 * Revision 1.50  1997/09/05 13:46:04  cg
 * All cast expressions are now removed by rmvoidfun.c. Therefore,
 * the respective attempts in precompile.c and ConstantFolding.c
 * are removed. Cast expressions are only used by the type checker.
 * Afterwards, they are useless, and they are not supported by
 * Constant Folding as well as code generation.
 *
 * Revision 1.49  1996/09/23 16:51:08  asi
 * bug fixed in CalcPsi
 *
 * Revision 1.48  1996/09/10  14:31:25  asi
 * bug fixed in ArrayPrf
 *
 * Revision 1.47  1996/08/08  13:51:42  asi
 * Bug Fixed in NoConstSkalarPrf
 *
 * Revision 1.46  1996/07/16  15:24:43  asi
 * Psi-expessions with user defined typed second argument will now be folded
 *
 * Revision 1.45  1996/05/31  13:51:24  asi
 * bug fixed in CFid
 *
 * Revision 1.44  1996/05/02  14:03:44  asi
 * bug fixed in CFassign
 *
 * Revision 1.43  1996/04/26  14:26:00  asi
 * Bug fixed for Psi Calculations
 *
 * Revision 1.42  1996/02/13  15:09:28  asi
 * bug fixed for calculating psi: now calculating psi for non constant arrays (again)
 *
 * Revision 1.41  1996/02/13  13:52:35  asi
 * bug fixed for calculating shape([...])
 *
 * Revision 1.40  1996/02/12  16:08:56  asi
 * argument res_type added to macro SHAPE_2_ARRAY
 *
 * Revision 1.39  1996/02/09  17:34:07  asi
 * Bug fixed in function IsConst
 *
 * Revision 1.38  1996/01/17  14:21:59  asi
 * new dataflow-information 'most-recently-defined-Listen' MRD used for substitution
 * added constant-folding for double-type
 * new trav-function OPTTrav used and some functions uses new access-macros for
 * virtuell syntax-tree
 *
 * Revision 1.37  1995/12/01  17:19:13  cg
 * All warnings converted to new Error macros
 *
 * Revision 1.36  1995/08/07  10:10:51  asi
 * now using macros defined in typecheck.h for folding of shape and dim
 *
 * Revision 1.35  1995/07/24  09:04:47  asi
 * bug fixed in function ArrayPrf case F_psi
 *
 * Revision 1.34  1995/07/21  13:16:52  asi
 * substitutes also within prim. functions
 *
 * Revision 1.33  1995/07/20  15:00:37  asi
 * substitutes y for x if x = y occures in program before
 *
 * Revision 1.32  1995/07/19  18:44:48  asi
 * changed CFprf
 * function call NoConstSkalarPrf moved to the end
 *
 * Revision 1.31  1995/07/06  16:13:26  asi
 * CFwhile and CFdo changed loop modification moved to Unroll.c
 *
 * Revision 1.30  1995/07/05  14:17:04  asi
 * bug fixed in ArrayPrf - F_psi
 *
 * Revision 1.29  1995/07/04  16:32:58  asi
 * IsConst defined global
 * CFwhile enhanced - consider cast-nodes
 *
 * Revision 1.28  1995/06/26  16:23:39  asi
 * some macros moved from .c file to .h file
 *
 * Revision 1.27  1995/06/26  15:39:16  asi
 * Unrolling handeled as independent optimization now
 *
 * Revision 1.26  1995/06/26  11:49:48  asi
 * logical prim-functions will generate boolean results now
 *
 * Revision 1.25  1995/06/21  13:23:58  asi
 * bug fix - same bug fixed for psi
 *
 * Revision 1.24  1995/06/21  13:10:02  asi
 * bug fix - ArrayPrf will check if it is a constant correctly
 *
 * Revision 1.23  1995/06/20  15:50:30  asi
 * Array fission modified
 *
 * Revision 1.22  1995/06/09  13:29:15  asi
 * now reallocating stack, if it sac-program has more than 50 levels
 *
 * Revision 1.21  1995/06/09  09:19:01  asi
 * *** empty log message ***
 *
 * Revision 1.20  1995/05/16  09:14:09  asi
 * usages of WARN1 adjust to changes in Error.h
 *
 * Revision 1.19  1995/05/05  09:06:17  asi
 * Constant arrays will no longer inserted in user defined or primitive functions,
 * if these functions could not be folded.
 *
 * Revision 1.18  1995/05/03  16:25:48  asi
 * CFap added
 *
 * Revision 1.17  1995/05/02  08:48:34  asi
 * DupConst and DupArray moved to DupTree.c
 *
 * Revision 1.16  1995/04/20  15:40:48  asi
 * mask handling in psi, dim and shape function added
 * bug fixed in psi: result isn't array, if vector and array dimension are equal
 *
 * Revision 1.15  1995/04/06  14:51:51  asi
 * constant folding in return-expessions disabled
 *
 * Revision 1.14  1995/04/06  11:35:20  asi
 * GetShapeVector now depends on SHP_SEG_SIZE
 *
 * Revision 1.13  1995/04/03  16:17:42  asi
 * Bug fixed in psi-calculation
 *
 * Revision 1.12  1995/03/24  15:54:25  asi
 * changed free() -> FREE()
 * changed Trav() -> OptTrav()
 * changed CFdo - replaced PopVL() with PopVL2()
 * changed CFwith - with-loops handeled like lokal funktions now
 *
 * Revision 1.11  1995/03/15  15:16:38  asi
 * modified mask handling in conditionals
 *
 * Revision 1.10  1995/03/13  17:58:12  asi
 * changover from 'info.id' to 'info.ids' of node N_id
 *
 * Revision 1.9  1995/03/13  16:59:05  asi
 * changed CFid
 *
 * Revision 1.8  1995/03/13  15:52:58  asi
 * changed DupConst
 *
 * Revision 1.7  1995/03/13  13:23:53  asi
 * bug fixed in CFwith : sets c.f. flag to false
 *
 * Revision 1.6  1995/03/08  17:28:09  asi
 * folding of primitive function psi() added
 * some changes for new prin. func. ..AxA, ..AxS and ..SxA
 *
 * Revision 1.5  1995/03/07  10:17:47  asi
 * added CFcast, constant folding within with loops
 * modified CFlet : determining basetype for user defined types
 *
 * Revision 1.4  1995/02/28  18:35:00  asi
 * added Cfwith
 * added counter for primfun application elimination
 * bug fixed in routines for shape, reshape and dim
 *
 * Revision 1.3  1995/02/22  18:16:34  asi
 * Fuctions CFfundef, CFassign, CFdo, CFwhile, CFcond, CFprf, CFid, CFlet
 * and PushVL, PushDupVL, PopVL for stack-management added
 *
 * Revision 1.2  1995/02/14  09:59:56  asi
 * added CFid
 *
 * Revision 1.1  1995/02/13  16:55:00  asi
 * Initial revision
 *
 *
 */

/******************************************************************************
 * srs: Usage of arg_info in the CF context
 *
 * info.types  : type of primitive function (CFlet)
 *             : type of generator (CFwith)
 * mask[0]/[1] : DEF and USE information
 * node[0]     : arg_node of last N_assign node (CFassign). Used in CFid.
 * varno       : no of elements used in masks
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "free.h"
#include "print.h"
#include "Error.h"
#include "dbug.h"
#include "globals.h"
#include "my_debug.h"
#include "traverse.h"
#include "typecheck.h"
#include "internal_lib.h"
#include "access_macros.h"

#include "optimize.h"
#include "DupTree.h"
#include "LoopInvariantRemoval.h"
#include "ConstantFolding.h"

#define MAXARG 3
#define FALSE 0
#define TRUE 1

/*
 *  This macros selects the right element out of the union info
 */
#define SELARG(n)                                                                        \
    (n->nodetype == N_num ? NUM_VAL (n)                                                  \
                          : (n->nodetype == N_float ? FLOAT_VAL (n) : DOUBLE_VAL (n)))

typedef struct INSIDE_WL {
    char *wl_name;
    struct INSIDE_WL *next;
} inside_wl;

static inside_wl *inside_wl_root;

/******************************************************************************
 *
 * function:
 *   int CalculateArrayOffset(types *array, node* index)
 *
 * description:
 *   calculates the data vector's element from an index vector.
 *   The array shape has to be given as a TYPES-struct and the index has
 *   to be a constant vector (N_array)
 *
 ******************************************************************************/

int
CalculateArrayOffset (types *array, node *index)
{
    int offset;
    int i, n, m;

    DBUG_ENTER ("CalculateArrayOffset");
    DBUG_ASSERT (IsConstantArray (index, N_num), ("not a constant index vector"));
    DBUG_ASSERT (1 == TYPES_DIM (ARRAY_TYPE (index)), ("wrong index vector dimension"));

    n = TYPES_SHAPE (ARRAY_TYPE (index), 0); /* length of index vector */
    m = TYPES_DIM (array);                   /* dimension of array */

    index = ARRAY_AELEMS (index);
    offset = NUM_VAL (EXPRS_EXPR (index));
    index = EXPRS_NEXT (index);

    for (i = 1; i < n; i++) {
        offset = offset * TYPES_SHAPE (array, i) + NUM_VAL (EXPRS_EXPR (index));
        index = EXPRS_NEXT (index);
    }

    for (i = n; i < m; i++)
        offset *= TYPES_SHAPE (array, i);

    DBUG_RETURN (offset);
}

/******************************************************************************
 *
 * function:
 *   int CompareNumArrayType(node *array1, node *array2)
 *
 * description:
 *   returnes 1 if both constant arrays have the same dimension and the same
 *   shape. Else returnes 0.
 *   Arrays must have type T_int.
 *
 ******************************************************************************/

int
CompareNumArrayType (node *array1, node *array2)
{
    int ok, i;

    DBUG_ENTER ("CompareNumArrayType");

    /* check misc */
    ok = !(N_array != NODE_TYPE (array1) || N_array != NODE_TYPE (array2)
           || T_int != ARRAY_BASETYPE (array1) || T_int != ARRAY_BASETYPE (array2)
           || ARRAY_DIM (array1) != ARRAY_DIM (array2));

    /* compare shape */
    for (i = 0; i < ARRAY_DIM (array1); i++)
        if (ARRAY_SHAPE (array1, i) != ARRAY_SHAPE (array2, i))
            ok = 0;

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *   int CompareNumArrayElts(node *array1, node *array2)
 *
 * description:
 *   returnes 1 if both constant arrays have the same elements. Else returnes 0.
 *   Arrays must have type T_int.
 *
 ******************************************************************************/

int
CompareNumArrayElts (node *array1, node *array2)
{
    int ok;

    DBUG_ENTER ("CompareNumArrayElts");

    ok = 1;

    /* compare elements */
    array1 = ARRAY_AELEMS (array1);
    array2 = ARRAY_AELEMS (array2);
    while (ok && array1 && array2) {
        ok = (N_num == NODE_TYPE (EXPRS_EXPR (array1))
              && N_num == NODE_TYPE (EXPRS_EXPR (array2))
              && NUM_VAL (EXPRS_EXPR (array1)) == NUM_VAL (EXPRS_EXPR (array2)));

        array1 = EXPRS_NEXT (array1);
        array2 = EXPRS_NEXT (array2);
    }

    DBUG_RETURN (ok);
}

/*
 *
 *  functionname  : IsConst
 *  arguments     : 1) node to be examine
 *		    R) TRUE  - if node is a constant
 *		       FALSE - if node isn't a constant, or if node is a array, but
 *                             containing non constant elements
 *  description   : determines if expression is a constant
 *  global vars   : syntax_tree, N_num, N_float, N_array, N_str, N_bool, N_id
 *  internal funs : --
 *  external funs : --
 *  macros        : TRUE, FALSE, NODE_TYPE, ARRAY_AELEMS, EXPRS_EXPR, EXPRS_NEXT
 *
 *  remarks       : --
 *
 */
int
IsConst (node *arg_node)
{
    int isit;
    node *expr;

    DBUG_ENTER ("IsConst");

    if (NULL != arg_node) {
        switch (NODE_TYPE (arg_node)) {
        case N_num:
        case N_float:
        case N_double:
        case N_str:
        case N_bool:
            isit = TRUE;
            break;
        case N_array:
            isit = TRUE;
            expr = ARRAY_AELEMS (arg_node);
            while ((NULL != expr) && (TRUE == isit)) {
                if (N_id == NODE_TYPE (EXPRS_EXPR (expr)))
                    isit = FALSE;
                expr = EXPRS_NEXT (expr);
            }
            break;
        default:
            isit = FALSE;
            break;
        }
    } else {
        isit = FALSE;
    }
    DBUG_RETURN (isit);
}

/*
 *
 *  functionname  : ConstantFolding
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr to root of the optimized syntax-tree
 *  description   : initiates constant folding for the intermediate sac-code:
 *		    - the most-recently-defined- stack (mrdl_stack) will be initialize
 *		    - call Trav to start constant-folding
 *  global vars   : syntax_tree, cf_tab, act_tab, mrdl_stack
 *  internal funs : ---
 *  external funs : Trav (traverse.h), MakeNode (tree_basic.h)
 *  macros        : FREE
 *
 *  remarks       : --
 *
 *
 */
node *
ConstantFolding (node *arg_node, node *info_node)
{
    DBUG_ENTER ("ConstantFolding");

    act_tab = cf_tab;

    info_node = MakeNode (N_info);
    inside_wl_root = NULL;

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFfundef
 *  arguments     : 1) N_fundef - node
 *		    2) N_info - node
 *		    R) N_fundef - node
 *  description   : calls OPTTrav to fold constants in current and following functions
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h)
 *  macros        : FUNDEF_BODY, FUNDEF_INSTR, FUNDEF_NEXT, FUNDEF_NAME
 *
 *  remarks       : ---
 *
 */
node *
CFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFfundef");

    DBUG_PRINT ("CF", ("Constant folding function: %s", FUNDEF_NAME (arg_node)));
    if (FUNDEF_BODY (arg_node))
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);
    FUNDEF_NEXT (arg_node) = OPTTrav (FUNDEF_NEXT (arg_node), arg_info, arg_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFassign
 *  arguments     : 1) N_assign - node
 *                  2) N_info - node
 *                  R) N_assign - node
 *  description   : - constant folding of the instructions
 *		    - construction of a new assign-list if nessessary
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs : CFassign
 *  external funs : OPTTrav (optimize.h), FreeNode (free.h),
 *                  AppendNodeChain (internal_lib.h)
 *  macros        : NODE_LINE, NODE_TYPE, ASSIGN_INSTR, ASSIGN_NEXT
 *
 *  remarks       : --
 *
 */
node *
CFassign (node *arg_node, node *arg_info)
{
    node *returnnode;
    nodetype ntype;

    DBUG_ENTER ("CFassign");
    DBUG_PRINT ("CF", ("Begin folding of line %d", NODE_LINE (arg_node)));
    NODE_LINE (arg_info) = NODE_LINE (arg_node);
    ntype = NODE_TYPE (ASSIGN_INSTR (arg_node));
    if (N_return != ntype) {
        INFO_CF_ASSIGN (arg_info) = arg_node;
        ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);

        switch (NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        case N_empty:
            returnnode = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
            FreeNode (arg_node);
            break;
        case N_assign:
            if (N_do == ntype) {
                ASSIGN_NEXT (arg_node)
                  = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
                returnnode
                  = AppendNodeChain (1, ASSIGN_INSTR (arg_node), ASSIGN_NEXT (arg_node));
                ASSIGN_INSTR (arg_node) = NULL;
                ASSIGN_NEXT (arg_node) = NULL;
                FreeTree (arg_node);
            } else {
                returnnode
                  = AppendNodeChain (1, ASSIGN_INSTR (arg_node), ASSIGN_NEXT (arg_node));
                ASSIGN_INSTR (arg_node) = NULL;
                ASSIGN_NEXT (arg_node) = NULL;
                FreeTree (arg_node);
                returnnode = CFassign (returnnode, arg_info);
            }
            break;
        default:
            ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);
            returnnode = arg_node;
            break;
        }
    } else
        returnnode = arg_node;

    DBUG_RETURN (returnnode);
}

/*
 *
 *  functionname  : GetType
 *  arguments     : 1) type
 *                  R) real type
 *  description   : determines real type
 *  global vars   : syntax_tree
 *  internal funs : --
 *  external funs : LookupType (typcheck.h)
 *  macros        : TYPES_BASETYPE, TYPES_NAME, TYPES_MOD, TYPEDEF_TYPE
 *
 *  remarks       : --
 *
 */
types *
GetType (types *type)
{
    node *tnode;

    DBUG_ENTER ("GetType");
    if (T_user == TYPES_BASETYPE (type)) {
        tnode = LookupType (TYPES_NAME (type), TYPES_MOD (type), 0);
        type = TYPEDEF_TYPE (tnode);
    }
    DBUG_RETURN (type);
}

/*
 *
 *  functionname  : CFlet
 *  arguments     : 1) N_let  - node
 *		    2) N_info - node
 *                  R) N_let  - node
 *  description   : - stores alink to the type of expression at the left hand side
 *		      of the let in the info_node
 *		    - initiates constant folding for the right hand side
 *  global vars   : syntax_tree, mrdl_stack
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
CFlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFlet");

    /* get result type for potential primitive function */
    INFO_CF_TYPE (arg_info) = GetType (IDS_TYPE (LET_IDS (arg_node)));

    /* Trav expression */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_CF_TYPE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFid
 *  arguments     : 1) id-node
 *		    2) info-node
 *		    R) id-node or modified node representing constant value:
 *		       num-, float-, bool- or array-node.
 *  description   : the id-node will be replaced with a new node reppresenting the
 *                  constant value of the identifikator in this context
 *  global vars   : syntax_tree, info_node, mrdl_stack
 *  internal funs :
 *  external funs : DupTree
 *  macros        : DBUG..., TOS
 *
 *  remarks       : --
 *
 */
node *
CFid (node *arg_node, node *arg_info)
{
    node *mrd;
    inside_wl *iw;

    DBUG_ENTER ("CFid");
    MRD_GETSUBST (mrd, ID_VARNO (arg_node), INFO_VARNO);

    /* check if this is an Id introduced by flatten (for WLs). This
       Ids shall not be replaced by another Id. The CF for the old
       WLs prevent this with a dirty trick described in optimize.c,
       OPTTrav(), switch N_assign, N_let. The new WL does it better
       with inside_wl struct. */
    if (mrd && N_id == NODE_TYPE (mrd) && inside_wl_root) {
        iw = inside_wl_root;
        while (mrd && iw) {
            if (!strcmp (ID_NAME (mrd), iw->wl_name))
                mrd = NULL;
            iw = iw->next;
        }
    }

    if (mrd) {
        switch (NODE_TYPE (mrd)) {
        case N_id:
            DEC_VAR (ASSIGN_USEMASK (INFO_CF_ASSIGN (arg_info)), ID_VARNO (arg_node));

            if (VARDEC_SHPSEG (ID_VARDEC (mrd)) == NULL
                && VARDEC_SHPSEG (ID_VARDEC (arg_node)) != NULL) {
                /* in the following fragment of a SAC-program

                   int[] f()
                   {
                   ret = ... external C-implementation ...
                   return (ret);
                   }

                   int main()
                   {
                   int[5] a;

                   a = f();

                   return ( ... a ... );
                   }

                   function-inlining leads to

                   int main()
                   {
                   int[5] a;
                   int[] inl_tmp;

                   inl__tmp = ret;
                   a = inl__tmp;

                   return ( ... a ... );
                   }.

                   When constant-folding now eliminates "a", the array-shape "[5]" must be
                   handed over to "inl__tmp" (a = inl__tmp):

                   int main()
                   {
                   int[5] inl_tmp;

                   inl__tmp = ret;

                   return ( ... inl__tmp ... );
                   }
                   */
                /*
                  if the type of mrd has not a shape yet, but the type of arg_node has,
                  then copy the dimension and the shape-segments from arg_node to mrd
                  */
                VARDEC_DIM (ID_VARDEC (mrd)) = VARDEC_DIM (ID_VARDEC (arg_node));
                VARDEC_SHPSEG (ID_VARDEC (mrd))
                  = DupShpSeg (VARDEC_SHPSEG (ID_VARDEC (arg_node)));
            }

            FreeTree (arg_node);
            arg_node = DupTree (mrd, NULL);
            INC_VAR (ASSIGN_USEMASK (INFO_CF_ASSIGN (arg_info)), ID_VARNO (arg_node));
            cf_expr++;
            break;
        case N_num:
        case N_float:
        case N_double:
        case N_bool:
        case N_str:
            DEC_VAR (INFO_USE, ID_VARNO (arg_node));
            FreeTree (arg_node);
            arg_node = DupTree (mrd, NULL);
            cf_expr++;
            break;
        case N_prf:
        case N_array:
        case N_ap:
        case N_with:
        case N_Nwith:
        case N_Npart: /* index vars point to this node. */
            break;
        default:
            DBUG_ASSERT ((FALSE), "Substitution not implemented for constant folding");
            break;
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFap
 *  arguments     : 1) N_ap   - node
 *                  2) N_info - node
 *                  R) N_ap   - node
 *  description   : Do not traverse arguments of user defined functions
 *  global vars   : syntax_tree
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       : ---
 *
 */
node *
CFap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFap");
    DBUG_RETURN (arg_node);
}

#if 0

All cast expressions are now removed by rmvoidfun.c !!

/* 
 * 
 *  functionname  : CFcast
 *  arguments     : 1) cast-node
 *                  2) info-node
 *                  R) cast-node
 *  description   : removes N_cast-node from syntax_tree
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h), FreeTree (free.h)
 *  macros        : CAST_EXPR
 * 
 *  remarks       : ---
 * 
 */
node *CFcast(node *arg_node, node *arg_info)
{
  node *next_node;
  
  DBUG_ENTER("CFcast");
  
  next_node = Trav(CAST_EXPR(arg_node), arg_info);
  
  CAST_EXPR(arg_node) = NULL;
  FreeTree(arg_node);
  DBUG_RETURN(next_node);
}

#endif

/*
 *
 *  functionname  : CFwhile
 *  arguments     : 1) N_while - node
 *		    2) N_info  - node
 *		    R) N_while - node
 *  description   : initiates constant folding inside while-loop
 *  global vars   : syntax_tree, mrdl_stack, cf_expr
 *  internal funs : ---
 *  external funs : OPTTrav, MinusMask (optimize.h), While2Do, MakeEmpty (tree_basic.h),
 *                  FreeTree (free.h)
 *  macros        : WHILE_INSTR, WHILE_COND, NODE_TYPE, BOOL_VAL, MRD_GETLAST, ID_VARNO,
 *                  WHILE_DEFMASK, WHILE_USEMASK, WHILE_TERMMASK, INFO_VARNO
 *
 *  remarks       : ---
 *
 */
node *
CFwhile (node *arg_node, node *arg_info)
{
    int trav_body = TRUE;
    node *last_value;

    DBUG_ENTER ("CFwhile");

    /* traverse condition. */
    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);

    /* is the condition FALSE? */
    if (N_bool == NODE_TYPE (WHILE_COND (arg_node)))
        if (!BOOL_VAL (WHILE_COND (arg_node)))
            trav_body = FALSE;
        else
            arg_node = While2Do (arg_node);

    /* is the condition a variable, which can be infered to false? */
    if (N_id == NODE_TYPE (WHILE_COND (arg_node))) {
        MRD_GETLAST (last_value, ID_VARNO (WHILE_COND (arg_node)), INFO_VARNO);
        if (last_value && (N_bool == NODE_TYPE (last_value))) {
            if (!BOOL_VAL (last_value))
                trav_body = FALSE;
            else
                arg_node = While2Do (arg_node);
        }
    }

    if (trav_body)
        WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);
    else {
        /* the body will never be entered - delete while loop. */
        DBUG_PRINT ("CF", ("while-loop eliminated in line %d", NODE_LINE (arg_node)));
        MinusMask (INFO_DEF, WHILE_DEFMASK (arg_node), INFO_VARNO);
        MinusMask (INFO_USE, WHILE_USEMASK (arg_node), INFO_VARNO);
        MinusMask (INFO_USE, WHILE_TERMMASK (arg_node), INFO_VARNO);
        FreeTree (arg_node);
        arg_node = MakeEmpty ();
        cf_expr++;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFdo
 *  arguments     : 1) N_do   - node
 *		    2) N_info - node
 *		    R) N_do   - node
 *  description   : initiates constant-folding inside do-loop and eliminates
 *                  do-loop if possible
 *  global vars   : syntax_tree, mrdl_stack, cf_expr
 *  internal funs : ---
 *  external funs : OPTTrav, MinusMask (optimize.h) FreeTree (free.h)
 *  macros        : DO_INSTR, DO_COND, NODE_TYPE, BOOL_VAL, NODE_LINE, INFO_USE,
 *                  DO_TERMMASK, INFO_VARNO,
 *
 *  remarks       : ---
 *
 */
node *
CFdo (node *arg_node, node *arg_info)
{
    node *returnnode;

    DBUG_ENTER ("CFdo");

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);

    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

    if (N_bool == NODE_TYPE (DO_COND (arg_node)))
        if (!BOOL_VAL (DO_COND (arg_node))) {
            DBUG_PRINT ("CF", ("do-loop eliminated in line %d", NODE_LINE (arg_node)));
            MinusMask (INFO_USE, DO_TERMMASK (arg_node), INFO_VARNO);
            returnnode = DO_INSTR (arg_node);
            DO_INSTR (arg_node) = NULL;
            FreeTree (arg_node);
            arg_node = returnnode;
            cf_expr++;
        }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFcond
 *  arguments     : 1) N_cond - node
 *		    2) N_info - node
 *		    R) N_cond - node , N_assign - node or N_empty - node
 *  description   : initiates constant folding for the conditional, if conditional
 *		    is true or false, return then or elseinstruction_chain,
 *                   otherwise constant-folding inside the conditional.
 *  global vars   : syntax_tree, mrdl_stack, cf_expr
 *  internal funs : ---
 *  external funs : OPTTrav, MinusMask (optimize.h), FreeTree (free.h)
 *  macros        : COND_COND, NODE_TYPE, BOOL_VAL, INFO_DEF, INFO_USE, COND_ELSEDEFMASK,
 *                  COND_ELSEUSEMASK, COND_THENDEFMASK, COND_THENUSEMASK, COND_ELSEINSTR,
 *                  COND_THENINSTR, COND_ELSE, COND_THEN
 *
 *  remarks       : ---
 *
 */
node *
CFcond (node *arg_node, node *arg_info)
{
    node *returnnode;

    DBUG_ENTER ("CFcond");
    returnnode = arg_node;

    COND_COND (arg_node) = OPTTrav (COND_COND (arg_node), arg_info, arg_node);

    if (N_bool == NODE_TYPE (COND_COND (arg_node))) {
        if (BOOL_VAL (COND_COND (arg_node))) {
            DBUG_PRINT ("CF", ("then-part of conditional eliminated in line %d",
                               NODE_LINE (arg_node)));
            MinusMask (INFO_DEF, COND_ELSEDEFMASK (arg_node), INFO_VARNO);
            MinusMask (INFO_USE, COND_ELSEUSEMASK (arg_node), INFO_VARNO);
            returnnode = COND_THENINSTR (arg_node);
            COND_THEN (arg_node) = NULL;
            FreeTree (arg_node);
            cf_expr++;
        } else {
            DBUG_PRINT ("CF", ("else-part of conditional eliminated in line %d",
                               NODE_LINE (arg_node)));
            MinusMask (INFO_DEF, COND_THENDEFMASK (arg_node), INFO_VARNO);
            MinusMask (INFO_USE, COND_THENUSEMASK (arg_node), INFO_VARNO);
            returnnode = COND_ELSEINSTR (arg_node);
            COND_ELSE (arg_node) = NULL;
            FreeTree (arg_node);
            cf_expr++;
        }
    } else {
        COND_THENINSTR (arg_node)
          = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
        COND_ELSEINSTR (arg_node)
          = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);
    }

    DBUG_RETURN (returnnode);
}

/*
 *
 *  functionname  : CFwith
 *  arguments     : 1) N_with - node
 *                  2) N_info - node
 *                  R) N_with - node
 *  description   : Travereses generator, then opertator
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : OPTTrav (optimize.h)
 *  macros        : INFO_CF_TYPE, WITH_GEN, GEN_VARDEC, VARDEC_TYPE, WITH_OPERATOR,
 *                  NODE_TYPE, GENARRAY_BODY, MODARRAY_BODY, FOLDPRF_BODY, FOLDFUN_BODY,
 *                  BLOCK_INSTR
 *
 *  remarks       : ---
 *
 */
node *
CFwith (node *arg_node, node *arg_info)
{
    types *oldtype;

    DBUG_ENTER ("CFwith");
    oldtype = INFO_CF_TYPE (arg_info);

    /* srs: this is the analogy to the storage of the return type of a prf in CFlet.
       But I wonder if this is necessary. A bound can only be an identifyer. And
       as far as I know the type information is only needed if we try to fold a
       prf (in CFprf). */
    INFO_CF_TYPE (arg_info) = VARDEC_TYPE (GEN_VARDEC (WITH_GEN (arg_node)));
    WITH_GEN (arg_node) = OPTTrav (WITH_GEN (arg_node), arg_info, arg_node);
    INFO_CF_TYPE (arg_info) = oldtype;

    switch (NODE_TYPE (WITH_OPERATOR (arg_node))) {
    case N_genarray:
        BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_modarray:
        BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldprf:
        BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldfun:
        BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    default:
        DBUG_ASSERT ((FALSE), "Operator not implemented for with_node");
        break;
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CFNwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
CFNwith (node *arg_node, node *arg_info)
{
    node *tmpn;
    inside_wl *iw;

    DBUG_ENTER ("CFNwith");

    /* add WL name to inside_wl struct */
    iw = Malloc (sizeof (inside_wl));
    iw->next = inside_wl_root;
    iw->wl_name = IDS_NAME (LET_IDS (ASSIGN_INSTR (INFO_CF_ASSIGN (arg_info))));
    inside_wl_root = iw;

    /* do NOT traverse withop in case of
       - genarray : TC assures constant array anyway
       - modarray : the Id has not to be substituted. This would destroy the
         work of flatten. */
    if (WO_foldprf == NWITH_TYPE (arg_node) || WO_foldfun == NWITH_TYPE (arg_node))
        NWITH_WITHOP (arg_node) = OPTTrav (NWITH_WITHOP (arg_node), arg_info, arg_node);

    /* traverse all generators */
    /* in CFwith type information of the index variable is stored in
       arg_info. I don't know why...*/
    tmpn = NWITH_PART (arg_node);
    while (tmpn) {
        /* we do not need to traverse N_Nwithid */
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NPART_NEXT (tmpn);
    }

    /* traverse bodies */
    tmpn = NWITH_CODE (arg_node);
    while (tmpn) {
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NCODE_NEXT (tmpn);
    }

    iw = inside_wl_root;
    inside_wl_root = iw->next;
    FREE (iw);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CFNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses into the generator.
 *   We do not need to care about the withid.
 *
 ******************************************************************************/

node *
CFNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER (" *CFNpart");
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CFNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse N_Ncode node. Only the CBLOCK.
 *
 *
 ******************************************************************************/

node *
CFNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER (" *CFNcode");
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GetShapeVector
 *  arguments     : 1) ptr to an array
 *		    2) ptr to an C-array, contains shape-vector after GetShapeVector
 *		    R) dimension of 1)
 *  description   : calulates dimension and shape-vector
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., SHP_SEG_SIZE
 *
 *  remarks       : --
 *
 */
int
GetShapeVector (node *array, int *vec_shape)
{
    int vec_dim = 0, i;
    node *expr;

    DBUG_ENTER ("GetShapeVector");

    expr = array->node[0];

    for (i = 0; i <= SHP_SEG_SIZE; i++) {

        if (NULL != expr) {
            vec_dim++;
            vec_shape[i] = expr->node[0]->info.cint;
            if (NULL != expr->node[1])
                expr = expr->node[1];
            else
                expr = NULL;
        } else
            vec_shape[i] = 0;
    }
    DBUG_RETURN (vec_dim);
}

/*
 *
 *  functionname  : ArraySize
 *  arguments     : 1) ptr to th array
 *		    R) size of the array
 *  description   : Counts the elements of the given array
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
int
ArraySize (node *array)
{
    int size;

    DBUG_ENTER ("ArraySize");
    size = 1;
    array = array->node[0];
    while (array->node[1]) {
        array = array->node[1];
        size++;
    }
    DBUG_RETURN (size);
}

/*
 *
 *  functionname  : DupPartialArray
 *  arguments     : 1) start position in array
 *                  2) lenght of result array
 *                  3) ptr to the array which should be partial duplicated
 *                  4) arg_info countains used-mask, which will be updated
 *                  R) ptr to result array
 *  description   : duplicates an array between start and (start+length)
 *  global vars   : N_exprs, N_id
 *  internal funs : --
 *  external funs : DupTree, MakeNode
 *  macros        : DBUG..., DEC_VAR
 *
 *  remarks       : --
 *
 */
node *
DupPartialArray (int start, int length, node *array, node *arg_info)
{
    int i;
    node *expr;
    node *r_expr = NULL;

    DBUG_ENTER ("DupPartialArray");

    if (0 < length) {
        array = array->node[0];

        /*
         * Goto start position
         */
        for (i = 0; i < start; i++)
            array = array->node[1];

        /*
         * Duplicate first elenment
         */
        expr = MakeNode (N_exprs);
        expr->node[0] = DupTree (array->node[0], NULL);
        if (N_id == NODE_TYPE (expr->node[0]))
            DEC_VAR (arg_info->mask[1], expr->node[0]->info.ids->node->varno);
        array = array->node[1];
        r_expr = expr;

        /*
         * Duplicate rest of array till length reached
         */
        for (i = 1; i < length; i++) {
            expr->node[1] = MakeNode (N_exprs);
            expr = expr->node[1];
            expr->node[0] = DupTree (array->node[0], NULL);
            if (N_id == NODE_TYPE (expr->node[0]))
                DEC_VAR (arg_info->mask[1], expr->node[0]->info.ids->node->varno);
            array = array->node[1];
        }
    }

    DBUG_RETURN (r_expr);
}

/*
 *
 *  functionname  : FoundZero
 *  arguments     : 1) node to be examine
 *		    R) TRUE if node contains a 0, 0.0 , [..., 0, ...] or [..., 0.0, ...]
 *		       FALSE otherwise
 *  description   : determines if expression is a constant containing zero
 *  global vars   : syntax_tree, N_num, N_float, N_array
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., SELARG, NULL, FALSE, TRUE
 *
 *  remarks       : --
 *
 */
int
FoundZero (node *arg_node)
{
    int FoundZero = FALSE;
    node *expr;

    DBUG_ENTER ("IsZero");
    switch (NODE_TYPE (arg_node)) {
    case N_num:
    case N_float:
    case N_double:
        if (0 == SELARG (arg_node))
            FoundZero = TRUE;
        break;
    case N_array:
        expr = arg_node->node[0];
        while (NULL != expr) {
            if (0 == SELARG (expr->node[0]))
                FoundZero = TRUE;
            expr = expr->node[1];
        }
        break;
    default:
        FoundZero = FALSE;
        break;
    }
    DBUG_RETURN (FoundZero);
}

/*
 *
 *  functionname  : ScalarPrf
 *  arguments     : 1) arguments for prim-function
 *		    2) type of prim-function
 *		    3) result type
 *		    4) calculate (arg1 op arg2) if swap==FALSE or
 *				 (arg2 op arg1) if swap==TRUE
 *		    R) result-node
 *  description   : Calculates non array prim-functions
 *  global vars   : N_float, ... , N_not, ..., prf_string
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE, SELARG
 *
 *  remarks       : arguments have to be constants before calling this function
 *
 */
node *
ScalarPrf (node **arg, prf prf_type, types *res_type, int swap)
{

    /*
     * This macro calculates all non array primitive functions
     */
#define ARI(op, a1, a2)                                                                  \
    {                                                                                    \
        switch (res_type->simpletype) {                                                  \
        case T_float:                                                                    \
            if (!swap)                                                                   \
                a1->info.cfloat = SELARG (a1) op SELARG (a2);                            \
            else                                                                         \
                a1->info.cfloat = SELARG (a2) op SELARG (a1);                            \
            NODE_TYPE (a1) = N_float;                                                    \
            break;                                                                       \
        case T_double:                                                                   \
            if (!swap)                                                                   \
                a1->info.cdbl = SELARG (a1) op SELARG (a2);                              \
            else                                                                         \
                a1->info.cdbl = SELARG (a2) op SELARG (a1);                              \
            NODE_TYPE (a1) = N_double;                                                   \
            break;                                                                       \
        case T_int:                                                                      \
            if (!swap)                                                                   \
                a1->info.cint = a1->info.cint op a2->info.cint;                          \
            else                                                                         \
                a1->info.cint = a2->info.cint op a1->info.cint;                          \
            break;                                                                       \
        case T_bool:                                                                     \
            if (!swap)                                                                   \
                a1->info.cint = a1->info.cint op a2->info.cint;                          \
            else                                                                         \
                a1->info.cint = a2->info.cint op a1->info.cint;                          \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT ((FALSE), "Type not implemented for Constant folding");          \
            break;                                                                       \
        }                                                                                \
        cf_expr++;                                                                       \
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_type]));       \
    }

/* srs: new define to handel min/max */
#define ARI2(op, a1, a2)                                                                 \
    {                                                                                    \
        /* swap is nonrelevant for min/max */                                            \
        switch (res_type->simpletype) {                                                  \
        case T_float:                                                                    \
            a1->info.cfloat = SELARG (a1) op SELARG (a2) ? SELARG (a1) : SELARG (a2);    \
            NODE_TYPE (a1) = N_float;                                                    \
            break;                                                                       \
        case T_double:                                                                   \
            a1->info.cdbl = SELARG (a1) op SELARG (a2) ? SELARG (a1) : SELARG (a2);      \
            NODE_TYPE (a1) = N_double;                                                   \
            break;                                                                       \
        case T_int:                                                                      \
            a1->info.cint = SELARG (a1) op SELARG (a2) ? SELARG (a1) : SELARG (a2);      \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT ((FALSE), "Type not implemented for Constant folding");          \
            break;                                                                       \
        }                                                                                \
        cf_expr++;                                                                       \
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_type]));       \
    }

    DBUG_ENTER ("ScalarPrf");

    /*
     * Calculate primitive Functions
     */
    switch (prf_type) {
    case F_not:
        arg[0]->info.cint = !arg[0]->info.cint;
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_type]));
        cf_expr++;
        break;
    case F_abs:
        if (arg[0]->info.cint < 0)
            arg[0]->info.cint *= -1;
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_type]));
        cf_expr++;
        break;
    case F_add:
    case F_add_AxA:
    case F_add_AxS:
    case F_add_SxA:
        ARI (+, arg[0], arg[1]);
        break;
    case F_sub:
    case F_sub_AxA:
    case F_sub_AxS:
    case F_sub_SxA:
        ARI (-, arg[0], arg[1]);
        break;
    case F_mul:
    case F_mul_AxA:
    case F_mul_AxS:
    case F_mul_SxA:
        ARI (*, arg[0], arg[1]);
        break;
    case F_div:
    case F_div_AxA:
    case F_div_AxS:
    case F_div_SxA:
        ARI (/, arg[0], arg[1]);
        break;
    case F_mod:
        if (!swap)
            arg[0]->info.cint = arg[0]->info.cint % arg[1]->info.cint;
        else
            arg[0]->info.cint = arg[1]->info.cint % arg[0]->info.cint;
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_type]));
        cf_expr++;
        break;
    case F_gt:
        ARI (>, arg[0], arg[1]);
        NODE_TYPE (arg[0]) = N_bool;
        break;
    case F_lt:
        ARI (<, arg[0], arg[1]);
        NODE_TYPE (arg[0]) = N_bool;
        break;
    case F_ge:
        ARI (>=, arg[0], arg[1]);
        NODE_TYPE (arg[0]) = N_bool;
        break;
    case F_le:
        ARI (<=, arg[0], arg[1]);
        NODE_TYPE (arg[0]) = N_bool;
        break;
    case F_eq:
        ARI (==, arg[0], arg[1]);
        NODE_TYPE (arg[0]) = N_bool;
        break;
    case F_neq:
        ARI (!=, arg[0], arg[1]);
        NODE_TYPE (arg[0]) = N_bool;
        break;
    case F_and:
        ARI (&&, arg[0], arg[1]);
        break;
    case F_or:
        ARI (||, arg[0], arg[1]);
        break;

    case F_min:
        ARI2 (<, arg[0], arg[1]);
        break;

    case F_max:
        ARI2 (>, arg[0], arg[1]);
        break;

    default:
        break;
    }
    DBUG_RETURN (arg[0]);
}

/*
 *
 *  functionname  : FoldExpr
 *  arguments     : 1) prf-node
 *		    2) argument number, which should be tested
 *		    3) argument number, which should be retured if test was successfully
 *		    4) test-pattern
 *		    5) arg_info-node needed for mask update
 *		    R) argument number res_arg or prf-node
 *  description   : folds the given prf to the argument number res_arg, if argument
 *		    number test_arg is an int, float or bool and this argument contains
 *		    the test_pattern
 *  global vars   : syntax_tree, N_num, N_bool, N_float, N_id
 *  internal funs : --
 *  external funs : FreePrf2
 *  macros        : DEC_VAR, SELARG, NULL, PRF_ARG1, PRF_ARG2, NODE_TYPE
 *
 *  remarks       :
 *
 */
node *
FoldExpr (node *arg_node, int test_arg, int res_arg, int test_pattern, node *arg_info)
{
    node *tmp, *arg[2];
    nodetype node_t;

    DBUG_ENTER ("FoldExpr");

    arg[0] = PRF_ARG1 (arg_node);
    arg[1] = PRF_ARG2 (arg_node);
    tmp = arg_node;
    node_t = NODE_TYPE (arg[test_arg]);
    if (((N_num == node_t) || (N_bool == node_t) || (N_float == node_t)
         || (N_double == node_t))
        && (test_pattern == SELARG (arg[test_arg]))) {
        arg_node = arg[res_arg];
        if ((N_id == NODE_TYPE (arg[!res_arg])) && (NULL != arg_info)) {
            DEC_VAR (INFO_USE, VARDEC_VARNO (ID_VARDEC (arg[!res_arg])));
        }
        cf_expr++;
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[PRF_PRF (tmp)]));
        FreePrf2 (tmp, res_arg);
        arg_node = arg[res_arg];
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : NoConstScalarPrf
 *  arguments     : 1) prf-node
 *		    2) arg_info includes used mask to be updated and varno
 *		    R) result-node
 *  description   : Calculates some  prim-functions with one non constant argument
 *  global vars   : N_float, ... , N_not, ..., prf_string
 *  internal funs : FreePrf2
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE
 *
 *  remarks       : if (NULL==arg_info) mask will not be updated
 *
 */
node *
NoConstScalarPrf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NoConstScalarPrf");

    /*
     * Calculate prim-functions with non constant arguments
     */
    if (N_prf == NODE_TYPE (arg_node)) {
        switch (arg_node->info.prf) {
        case F_and:
            arg_node
              = FoldExpr (arg_node, 0, 0, FALSE, arg_info); /* FALSE && x = FALSE */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node
                  = FoldExpr (arg_node, 1, 1, FALSE, arg_info); /* x && FALSE = FALSE */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 0, 1, TRUE, arg_info); /* x && TRUE = x */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 1, 0, TRUE, arg_info); /* TRUE && x = x */
            break;
        case F_or:
            arg_node = FoldExpr (arg_node, 0, 0, TRUE, arg_info); /* TRUE || x = TRUE */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node
                  = FoldExpr (arg_node, 1, 1, TRUE, arg_info); /* x || TRUE = TRUE */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node
                  = FoldExpr (arg_node, 0, 1, FALSE, arg_info); /* x || FALSE = x */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node
                  = FoldExpr (arg_node, 1, 0, FALSE, arg_info); /* FALSE || x = x */
            break;
        case F_mul:
            arg_node = FoldExpr (arg_node, 0, 0, 0, arg_info); /* 0 * x = 0 */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 1, 1, 0, arg_info); /* x * 0 = 0 */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 0, 1, 1, arg_info); /* 1 * x = x */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 1, 0, 1, arg_info); /* x * 1 = x */
            break;
        case F_mul_AxS:
        case F_mul_SxA:
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 0, 1, 1, arg_info); /* 1 * x = x */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 1, 0, 1, arg_info); /* x * 1 = x */

            break;
        case F_div:
            if (TRUE == FoundZero (arg_node->node[0]->node[1]->node[0])) {
                WARN (arg_node->lineno, ("Division by zero expected"));
            }
            arg_node = FoldExpr (arg_node, 0, 0, 0, arg_info); /* 0 / x = 0 */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 1, 0, 1, arg_info); /* x / 1 = x */
            break;
        case F_div_SxA:
            if (TRUE == FoundZero (arg_node->node[0]->node[1]->node[0])) {
                WARN (arg_node->lineno, ("Division by zero expected"));
            }
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 1, 0, 1, arg_info); /* x / 1 = x */
            break;
        case F_add:
        case F_add_AxS:
        case F_add_SxA:
            arg_node = FoldExpr (arg_node, 0, 1, 0, arg_info); /* 0 + x = x */
            if (N_prf == NODE_TYPE (arg_node))
                arg_node = FoldExpr (arg_node, 1, 0, 0, arg_info); /* x + 0 = x */
            break;
        case F_sub:
        case F_sub_AxS:
        case F_sub_SxA:
            arg_node = FoldExpr (arg_node, 1, 0, 0, arg_info); /* x - 0 = x */
            break;
        default:
            break;
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : FetchNum
 *  arguments     : 1) position in array
 *                  2) ptr to the array
 *                  R) ptr to fetchted element
 *  description   : picks up and duplicates the element at position pos
 *  global vars   : --
 *  internal funs : --
 *  external funs : DupTree
 *  macros        : ARRAY_AELEMS, EXPRS_NEXT, EXPRS_EXPR
 *
 *  remarks       : --
 *
 */
node *
FetchNum (int pos, node *array)
{
    int i;
    node *tmp;

    DBUG_ENTER ("FetchNum");

    tmp = ARRAY_AELEMS (array);

    for (i = 0; i < pos; i++)
        tmp = EXPRS_NEXT (tmp);

    tmp = DupTree (EXPRS_EXPR (tmp), NULL);

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : CalcPsi
 *  arguments     : 1) shape vector function psi is applied to
 *		    2) array the function psi is applied to
 *		    3) type of array the function psi is applied to
 *		    4) arg_info containing f.e. type of result
 *		    R) calculated result
 *  description   :
 *  global vars   :
 *  internal funs : --
 *  external funs : --
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
CalcPsi (node *shape, node *array, types *array_type, node *arg_info)
{
    int length, start, mult, i, j, arg_length;
    int vec_dim, array_dim, result_dim;
    int vec_shape[SHP_SEG_SIZE];
    shpseg *array_shape, *result_shape;
    node *res_node = NULL;

    DBUG_ENTER ("CalcPsi");
    GET_BASIC_TYPE (INFO_CF_TYPE (arg_info), INFO_CF_TYPE (arg_info), 47);
    GET_BASIC_TYPE (array_type, array_type, 47);

    /* Calculate dimension and shape vector of first argument */
    vec_dim = GetShapeVector (shape, vec_shape);
    array_dim = TYPES_DIM (array_type);
    array_shape = TYPES_SHPSEG (array_type);
    result_dim = TYPES_DIM (INFO_CF_TYPE (arg_info));
    if (0 < result_dim)
        result_shape = TYPES_SHPSEG (INFO_CF_TYPE (arg_info));

    /* Calculate length of result array */
    length = 1;
    for (i = 0; i < result_dim; i++)
        length *= SHPSEG_SHAPE (result_shape, i);

    /* Calculate startposition of result array in argument array */
    start = 0;
    for (i = 0; i < vec_dim; i++) {
        if (vec_shape[i] >= 0) {
            mult = 1;
            for (j = i + 1; j < array_dim; j++)
                mult *= SHPSEG_SHAPE (array_shape, j);
            start += vec_shape[i] * mult;
        } else {
            start = -1;
        }
    }

    arg_length = 1;
    for (i = 0; i < array_dim; i++)
        arg_length *= SHPSEG_SHAPE (array_shape, i);

    DBUG_PRINT ("CF",
                ("start = %d, lenght = %d, arg_length = %d", start, length, arg_length));
    if ((start + length <= arg_length) && (start >= 0)) {
        if (vec_dim == array_dim) {
            res_node = FetchNum (start, array);
        } else {
            res_node = MakeArray (DupPartialArray (start, length, array, arg_info));
            ARRAY_TYPE (res_node) = DuplicateTypes (ARRAY_TYPE (array), 1);
        }
    } else {
        WARN (NODE_LINE (INFO_CF_ASSIGN (arg_info)),
              ("Illegal vector for primitive function psi"));
    }
    DBUG_RETURN (res_node);
}

/*
 *
 *  functionname  : ArrayPrf
 *  arguments     : 1) prf-node
 *		    2) arg_info
 *		    R) result-node
 *  description   : Calculates array prim-functions
 *  global vars   : N_float, ... , N_not, ..., prf_string
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE
 *
 *  remarks       : most arguments have to be constants before calling this function
 *
 */
node *
ArrayPrf (node *arg_node, node *arg_info)
{
    node *arg[MAXARG], *expr[MAXARG], *expr_arg[MAXARG], *tmp;
    int swap, i;
    long *used_sofar;

    DBUG_ENTER ("ArrayPrf");
    DBUG_ASSERT (N_prf == NODE_TYPE (arg_node), "wrong argument");

    /*
     * Search Arguments for primitive Functions
     */
    tmp = PRF_ARGS (arg_node);
    for (i = 0; i < MAXARG; i++)
        if (tmp) {
            arg[i] = NodeBehindCast (EXPRS_EXPR (tmp));
            tmp = EXPRS_NEXT (tmp);
        } else
            arg[i] = NULL;

    /*
     * Calculate primitive Functions
     */
    switch (arg_node->info.prf) {
    case F_div_AxA:
    case F_div_AxS:
    case F_div_SxA:
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_ge:
    case F_gt:
    case F_neq:
    case F_add_AxA:
    case F_add_AxS:
    case F_add_SxA:
    case F_sub_AxA:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_mul_AxA:
    case F_mul_AxS:
    case F_mul_SxA: {
        node *old_arg[2], *value;

        DBUG_PRINT ("CF", ("Begin folding of %s", prf_string[arg_node->info.prf]));

        old_arg[0] = arg[0];
        old_arg[1] = arg[1];

        if (N_id == NODE_TYPE (arg[0])) {
            MRD_GETDATA (value, arg[0]->info.ids->node->varno, INFO_VARNO);
            if (IsConst (value)) {
                DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
                arg[0] = DupTree (value, NULL);
                used_sofar = arg_info->mask[1];
                arg_info->mask[1] = GenMask (INFO_VARNO);
                arg[0] = Trav (arg[0], arg_info);
                FREE (arg_info->mask[1]);
                arg_info->mask[1] = used_sofar;
            } else {
                break;
            }
        } else {
            arg[0] = Trav (arg[0], arg_info);
            if (!IsConst (arg[0]))
                break;
        }

        if (N_id == NODE_TYPE (arg[1])) {
            MRD_GETDATA (value, arg[1]->info.ids->node->varno, INFO_VARNO);
            if (IsConst (value)) {
                DEC_VAR (arg_info->mask[1], arg[1]->info.ids->node->varno);
                arg[1] = DupTree (value, NULL);
                used_sofar = arg_info->mask[1];
                arg_info->mask[1] = GenMask (INFO_VARNO);
                arg[1] = Trav (arg[1], arg_info);
                FREE (arg_info->mask[1]);
                arg_info->mask[1] = used_sofar;
            } else {
                if (N_id == NODE_TYPE (old_arg[0])) {
                    INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                    FreeTree (arg[0]);
                }
                break;
            }
        } else {
            arg[1] = Trav (arg[1], arg_info);
            if (!IsConst (arg[1])) {
                if (N_id == NODE_TYPE (old_arg[0])) {
                    INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                    FreeTree (arg[0]);
                }
                break;
            }
        }

        if (((F_div == arg_node->info.prf) || (F_div_SxA == arg_node->info.prf)
             || (F_div_AxS == arg_node->info.prf) || (F_div_AxA == arg_node->info.prf))
            && (TRUE == FoundZero (arg[1]))) {
            if (N_id == NODE_TYPE (old_arg[0])) {
                INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                FreeTree (arg[0]);
            }
            if (N_id == NODE_TYPE (old_arg[1])) {
                INC_VAR (arg_info->mask[1], old_arg[1]->info.ids->node->varno);
                FreeTree (arg[1]);
            }
            WARN (arg_node->lineno, ("Division by zero expected"));
            break;
        }

        /*
         * Swap arguments, to be sure that an array is first argument
         */
        if (N_array != NODE_TYPE (arg[0])) {
            tmp = arg[1];
            arg[1] = arg[0];
            arg[0] = tmp;
            swap = TRUE;
        } else {
            swap = FALSE;
        }

        if (N_array == NODE_TYPE (arg[1])) {
            /*
             * Calculate prim-function with two arrays
             */
            expr[0] = arg[0]->node[0];
            expr[1] = arg[1]->node[0];
            do {
                expr_arg[0] = expr[0]->node[0];
                expr_arg[1] = expr[1]->node[0];
                expr[0]->node[0] = ScalarPrf (expr_arg, arg_node->info.prf,
                                              INFO_CF_TYPE (arg_info), swap);
                expr[0] = expr[0]->node[1];
                expr[1] = expr[1]->node[1];
            } while ((NULL != expr[0]) && (NULL != expr[1]));
        } else {
            /*
             * Calculate prim-function with one array
             */
            expr[0] = arg[0]->node[0];
            expr_arg[1] = arg[1];
            do {
                expr_arg[0] = expr[0]->node[0];
                expr[0]->node[0] = ScalarPrf (expr_arg, arg_node->info.prf,
                                              INFO_CF_TYPE (arg_info), swap);
                expr[0] = expr[0]->node[1];
            } while ((NULL != expr[0]));
        }

        DBUG_PRINT ("CF", ("End folding of %s", prf_string[arg_node->info.prf]));

        /*
         * free useless nodes and store result
         */
        if (!swap) {
            if (old_arg[0] == arg[0])
                FreePrf2 (arg_node, 0);
            else
                FreeTree (arg_node);

            if (old_arg[1] != arg[1])
                FreeTree (arg[1]);

            arg_node = arg[0];
        } else {
            if (old_arg[1] == arg[0])
                FreePrf2 (arg_node, 1);
            else
                FreeTree (arg_node);

            if (old_arg[0] != arg[1])
                FreeTree (arg[1]);

            arg_node = arg[0];
            DBUG_PRINT ("MEM", ("Argument has address: %08x", arg_node));
        }
    } break;

    /***********************/
    /* Fold shape-function */
    /***********************/
    case F_shape:
        switch (NODE_TYPE (arg[0])) {
        case (N_array):
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * Count array length
             */
            i = 0;
            tmp = arg[0]->node[0];
            do {
                i++;
                tmp = tmp->node[1];
            } while (NULL != tmp);

            /*
             * Store result in this array
             */
            arg[0]->node[0]->node[0]->info.cint = i;
            NODE_TYPE (arg[0]->node[0]->node[0]) = N_num;

            /*
             * Free rest of array and prf-node
             */
            if (NULL != arg[0]->node[0]->node[1])
                FreeTree (arg[0]->node[0]->node[1]);
            arg[0]->node[0]->node[1] = NULL; /* ??? */
            FREE (arg_node);

            /*
             * Gives Array the correct type
             */
            ARRAY_TYPE (arg[0]) = FreeOneTypes (ARRAY_TYPE (arg[0]));
            ARRAY_TYPE (arg[0]) = DuplicateTypes (INFO_CF_TYPE (arg_info), 0);

            /*
             * Store result
             */
            arg_node = arg[0];
            cf_expr++;
            break;
        case N_id:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            SHAPE_2_ARRAY (tmp, arg[0]->info.ids->node->info.types,
                           INFO_CF_TYPE (arg_info));
            /* TC uses folding of shape() and has no masks present. */
            if (arg_info->mask[1])
                DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
            FreeTree (arg_node);
            arg_node = tmp;
            cf_expr++;
            break;
        default:
            break;
        }
        break; /* shape */

    /***********************/
    /* Fold reshape-function */
    /***********************/
    case F_reshape:
        /*
         * we want to eliminate reshape-calls here since they hinder CF
         * in many situations, e.g. when accessing constant arrays
         * that are defined by reshape....
         */

        /* srs: NoNo, we really shouldn't do that. Imagine the following case:
             A = WL () genarray([8]);
             B = reshape([2,2,2],A);
             C = WL () {...references to B...} modarray(B)
           If the second line is replaced by B = A then while WLF the second
           WL operates on an array of the *wrong* shape (if it's possible in
           this situation.
           But WLF must not become active here because no generators are known
           for the array B. */

        /*       arg_node = arg[1]; */

        /*
         * Now, we should (!!!) free the N_prf-node and its first arg...
         * Unfortunately, we did not yet implement it 8-(
         */
        break;

        /***********************/
        /* Fold dim-function   */
        /***********************/
    case F_dim:
        switch (NODE_TYPE (arg[0])) {
        case N_array:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * result is always 1
             */
            NODE_TYPE (arg_node) = N_num;
            arg_node->info.cint = 1;

            /*
             * Free argument of prim function
             */
            FreeTree (arg_node->node[0]);
            cf_expr++;
            break;
        case N_id:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * get result
             */
            NODE_TYPE (arg_node) = N_num;
            GET_DIM (arg_node->info.cint, arg[0]->info.ids->node->info.types);

            /*
             * Free argument of prim function
             */
            FreeTree (arg_node->node[0]);
            DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
            cf_expr++;
            break;
        default:
            break;
        }
        break; /* dim */

        /***********************/
        /* Fold psi-function   */
        /***********************/
    case F_psi: {
        int dim, ok;
        node *shape, *array, *res_array, *first_elem;
        node *tmpn, *modindex;
        types *array_type;
        node **mrdmask;

        res_array = NULL;

        /*
         * Substitute shape-vector
         */
        if (N_id == NODE_TYPE (arg[0])) {
            MRD_GETDATA (shape, ID_VARNO (arg[0]), INFO_VARNO);
        } else
            shape = arg[0];

        if (!IsConst (shape))
            break;
        DBUG_ASSERT (N_array == NODE_TYPE (shape), "Shape-vector for psi not an array");

        /*
         * Here, we know that the first arg of F_psi indeed is a
         * constant array and we know that shape points to that
         * N_array-node.
         */
        /* ths following procedure substitutes arrays that are modified with
         * prf modarray.
         *
         * NOTE here, that this is NOT identical to CF for F_modarray
         * itself!!
         * Example:
         *   A = unknown;
         *   A = modarray( A, c1, val);
         *      .... A[ c1] ....
         *
         *   is optimized to:
         *   A = unknown;
         *   A = modarray( A, c1, val);    <==  here we can not cf !!
         *      .... val ....
         *
         * Only a special case is implemented here to
         * support WL-unrolling.
         */
        res_array = NULL;

        if (N_id == NODE_TYPE (arg[1])) {
            array_type = ID_TYPE (arg[1]);

            /* let's have a closer look at the id. If it is a prf modarray
               and we index the element which is modified by this prf,
               the second arg of the prf can be used.
               Else we search for an N_array node.*/
            ok = 0;
            tmpn = arg[1];
            mrdmask = MRD_TOS.varlist;
            do {
                array = mrdmask[ID_VARNO (tmpn)];
                ok = 1;
                if (array && N_assign == NODE_TYPE (array)
                    && N_let == NODE_TYPE (ASSIGN_INSTR (array))) {
                    mrdmask = (node **)ASSIGN_MRDMASK (array);
                    array = LET_EXPR (ASSIGN_INSTR (array));
                } else
                    array = NULL;

                if (array && N_array == NODE_TYPE (array))
                    ; /* leave do-loop */
                else if (array && N_prf == NODE_TYPE (array)
                         && F_modarray == PRF_PRF (array)) {
                    /* check index of prf modarray */
                    modindex = PRF_ARG2 (array);
                    if (N_id == NODE_TYPE (modindex))
                        MRD_GETDATA (modindex, ID_VARNO (modindex), INFO_VARNO);
                    if (IsConstantArray (modindex, N_num)
                        && CompareNumArrayType (shape, modindex)) {
                        if (CompareNumArrayElts (shape, modindex))
                            res_array = DupTree (PRF_ARG3 (array), NULL);
                        else {
                            /* valid modindex, but not equal index. */
                            tmpn = PRF_ARG1 (array);
                            ok = 0; /* continue searching */
                        }
                    } else            /* F_modarray, but modindex nut supported */
                        array = NULL; /* not successful */
                } else                /* no N_array and no F_modarray */
                    array = NULL;     /* not successful */

            } while (!ok);
        } else {
            /* 2nd argument is constant array ... easy */
            DBUG_ASSERT (N_array == NODE_TYPE (arg[1]), "N_array expected");
            array_type = ARRAY_TYPE (arg[1]);
            array = arg[1];
        }

        if (!array)
            break;

        if (!res_array) {
            /* Arrays like [a,...] with a = [...] cannot be folded til now */
            first_elem = EXPRS_EXPR (ARRAY_AELEMS (array));
            if (N_id == NODE_TYPE (first_elem)) {
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (first_elem)));
                if (0 != dim)
                    break;
                /* make array flat here !!! */
            }

            res_array = CalcPsi (shape, array, array_type, arg_info);
        }

        if (res_array) {
            DBUG_PRINT ("CF", ("primitive function %s folded in line %d",
                               prf_string[arg_node->info.prf], NODE_LINE (arg_info)));
            MinusMask (INFO_USE, ASSIGN_USEMASK (INFO_CF_ASSIGN (arg_info)), INFO_VARNO);
            FreeTree (arg_node);
            /* srs: arg_info is modified within GenMasks. At least ->mask[0],
               mask[1], nodetype and node[2]. Is this intended to happen? */
            arg_node = GenerateMasks (res_array, arg_info);
            cf_expr++;
        }
    } break;

    /* modarray(A,iv,val)
       if - A is a reference to an N_array,
          - iv is a reference to a constant index vector and
          - val is a scalar or an N_array node,
       we can create a new N_array node which is based on A but contains
       val at position iv. */
    case F_modarray: {
        node *base_array, *vectorn, *valn, *newn, *tmpn;
        types *base_array_type;
        int offset;

        base_array = arg[0];
        vectorn = arg[1];
        valn = arg[2];

        if (N_array == NODE_TYPE (base_array))
            base_array_type = ARRAY_TYPE (base_array);
        else if (N_id == NODE_TYPE (base_array))
            base_array_type = ID_TYPE (base_array);
        else
            break;

        /* search for mrd constant array */
        tmpn = NULL;
        while (base_array && N_id == NODE_TYPE (base_array)) {
            if (!tmpn)
                base_array = MRD (ID_VARNO (base_array));
            else {
                DBUG_ASSERT (ASSIGN_MRDMASK (tmpn),
                             ("MRDMASKs are NULL, ArrayPrf, modarray"));
                base_array = (node *)ASSIGN_MRDMASK (tmpn)[ID_VARNO (base_array)];
            }

            tmpn = base_array;

            if (base_array && /* mrd exists */
                N_assign == NODE_TYPE (base_array)
                && N_let == NODE_TYPE (ASSIGN_INSTR (base_array))) {
                base_array = LET_EXPR (ASSIGN_INSTR (base_array));
                if (N_prf == NODE_TYPE (base_array) && F_modarray == PRF_PRF (base_array))
                    base_array = PRF_ARG1 (base_array);
            }
        }

        /* search for mrd index vector and val */
        if (N_id == NODE_TYPE (vectorn))
            MRD_GETDATA (vectorn, ID_VARNO (vectorn), INFO_VARNO);
        if (N_id == NODE_TYPE (valn))
            MRD_GETDATA (valn, ID_VARNO (valn), INFO_VARNO);

        if (base_array && vectorn && valn && N_array == NODE_TYPE (base_array)
            && IsConstantArray (vectorn, N_num) &&
            /* valn should be an N_array or a scalar value, not an id. */
            (N_array == NODE_TYPE (valn) || N_num == NODE_TYPE (valn)
             || N_char == NODE_TYPE (valn) || N_float == NODE_TYPE (valn)
             || N_double == NODE_TYPE (valn) || N_bool == NODE_TYPE (valn))) {
            offset = CalculateArrayOffset (base_array_type, vectorn);

            /* offset found, now change elements */
            newn = DupTree (base_array, NULL);
            tmpn = ARRAY_AELEMS (newn);
            while (offset--) {
                tmpn = EXPRS_NEXT (tmpn);
                if (!tmpn)
                    ABORT (NODE_LINE (arg_node), ("constant index vector of modarray() "
                                                  "out of bounds"));
            }

            if (N_array == NODE_TYPE (valn)) {
                valn = ARRAY_AELEMS (valn);
                while (valn) {
                    DBUG_ASSERT (tmpn, ("array data vector has wrong length"));
                    FreeTree (EXPRS_EXPR (tmpn));
                    EXPRS_EXPR (tmpn) = DupTree (EXPRS_EXPR (valn), NULL);
                    valn = EXPRS_NEXT (valn);
                    tmpn = EXPRS_NEXT (tmpn);
                }
            } else {
                FreeTree (EXPRS_EXPR (tmpn));
                EXPRS_EXPR (tmpn) = DupTree (valn, NULL);
            }

            /* replace F_modarray with newn */
            FreeTree (arg_node);
            arg_node = newn;
            cf_expr++;
        }

    } break;

    case F_take:
    case F_drop:
    case F_cat:
    case F_rotate:
    default:
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFprf
 *  arguments     : 1) prf-node
 *		    2) info_node
 *		    R) prf-node or calculated constant value
 *  description   : first replace constant values for variables in expression
 *		    then calculate primitive function.
 *  global vars   : syntax_tree, info_node, mrdl_stack
 *  internal funs : ScalarPrf, ArrayPrf
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
CFprf (node *arg_node, node *arg_info)
{
    node *arg[MAXARG];
    node *tmp;
    int i;

    DBUG_ENTER ("CFprf");

    if (PRF_PRF (arg_node) <= F_neq) /* prfs on scalars only !! */
    {
        /*
         * substitute all arguments
         */
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);

        /*
         * Search Arguments for primitive Functions
         */
        tmp = PRF_ARGS (arg_node);
        for (i = 0; i < MAXARG; i++) {
            if (tmp) {
                arg[i] = EXPRS_EXPR (tmp);
                tmp = EXPRS_NEXT (tmp);
            } else
                arg[i] = NULL;
        }

        /*
         * Calculate non array primitive functions
         */
        if (IsConst (arg[0])
            && (F_abs == PRF_PRF (arg_node) || F_not == PRF_PRF (arg_node)
                || IsConst (arg[1]))) {
            if (F_div != PRF_PRF (arg_node) || !FoundZero (arg[1])) {
                arg[0]
                  = ScalarPrf (arg, PRF_PRF (arg_node), INFO_CF_TYPE (arg_info), FALSE);
                FreePrf2 (arg_node, 0);
                arg_node = arg[0];
            }
        }
    } else /* prfs that require at least one array as argument! */
    {
        /*
         * substitute all arguments
         */
        switch (PRF_PRF (arg_node)) {
        case F_shape:
        case F_dim:
            break;
        default:
            PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
            break;
        }

        /*
         * Calculate primitive functions with arrays
         */
        arg_node = ArrayPrf (arg_node, arg_info);
    }

    /*
     * Do some foldings like 1 * x = x, etc.
     */
    arg_node = NoConstScalarPrf (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
