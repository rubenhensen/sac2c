/*
 *
 * $Log$
 * Revision 2.24  1999/11/10 09:10:18  cg
 * Bug fixed: folding of psi() with prior modarray() to the same index
 * location now works correctly even if the variable pointing to the
 * new (after folding) result of psi has been assigned a new value
 * between the applications of modarray() and psi()
 *      AND
 * if multiple applications of psi() refer to the same application
 * of modarray().
 *
 * Revision 2.23  1999/10/29 16:44:23  dkr
 * fixed a bug in ArrayPrf:
 *   when introducing a new var, the MRD list must be expanded (MAlloc!)
 * accordingly
 *
 * Revision 2.22  1999/10/27 11:37:17  dkr
 * bugs in CompareNumArrayElts() fixed
 *
 * Revision 2.21  1999/10/25 17:09:45  dkr
 * bug in CF for psi() fixed:
 *   for new assignments (ASSIGN_CF) correct MRDs, USE_MASKs and DEF_MASKs are created :))
 *
 * Revision 2.20  1999/10/17 18:12:07  sacbase
 * few minor changes
 *
 * Revision 2.19  1999/09/01 12:21:53  sbs
 * PRF_PRF added
 *
 * Revision 2.18  1999/07/29 07:32:02  cg
 * Bug fixed in folding of consecutive modarray() / psi()
 * operations; the actual value is now determined in the
 * correct scope.
 *
 * Revision 2.17  1999/07/15 20:39:07  sbs
 * CFarray added.
 *
 * Revision 2.16  1999/07/13 16:19:30  bs
 * Bug fixed in ArrayPrf.
 *
 * Revision 2.15  1999/07/08 14:53:59  sbs
 * Array2BoolVec used instead of Array2IntVec
 *
 * Revision 2.14  1999/05/14 09:25:13  jhs
 * Dbugged constvec annotations and their housekeeping in various compilation stages.
 *
 * Revision 2.13  1999/05/12 09:55:53  jhs
 * Adjusted macros to access constant vectors.
 *
 * Revision 2.12  1999/04/28 08:50:09  jhs
 * checkin in emergency comment to be done later
 *
 * Revision 2.11  1999/04/21 15:37:29  jhs
 * Deleted never valid DBUG_ASSERT in DupPartialArray.
 * Not r_expr but expr0 was meant there.
 *
 * Revision 2.10  1999/04/21 10:06:13  bs
 * Some access macros added.
 * Function ArrayPrf modified: F_shape
 *
 * Revision 2.9  1999/03/31 15:12:02  bs
 * CFid modified. Now we are using a new access macro MRD_GETCFID
 *
 * Revision 2.8  1999/03/31 12:19:14  srs
 * fixed bug in ArrayPrf(), F_mordarray
 *
 * Revision 2.7  1999/03/19 10:03:44  bs
 * Function IsConst modified.
 *
 * Revision 2.6  1999/03/15 17:51:05  bs
 * Bug fixed
 *
 * Revision 2.5  1999/03/15 17:25:50  bs
 * Bugs fixed!
 *
 * Revision 2.4  1999/03/15 15:43:14  bs
 * Comments added.
 *
 * Revision 2.3  1999/03/15 14:25:46  bs
 * Access macros renamed (take a look at tree_basic.h).
 * Function ArrayPrf modified.
 *
 * Revision 2.2  1999/03/09 11:30:45  bs
 * Now the compact propagation of constant integer vectors will remain
 * after constant folding. Therefore the following functions were modified:
 * CompareNumArrayElts, IsConst, DupPartialArray, FetchNum, CalcPsi,
 * ArrayPrf and CFprf.
 *
 * Revision 2.1  1999/02/23 12:41:05  sacbase
 * new release made
 *
 * Revision 1.81  1999/02/09 20:30:38  sbs
 * once again sweet love does now invite....
 * F_reshape left in ALL cases now!
 * To enable CF via PSI still, used MRD_GET_DATA which
 * reaches out behind reshape!
 *
 * Revision 1.80  1999/01/19 13:18:59  sbs
 * several BUGs eliminated [GRGRGRGR]
 *
 * Revision 1.79  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.78  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.77  1998/12/11 18:13:08  sbs
 * F_reshape patched again ++
 * BAD ERROR when swapping args in FoldPrfScalars....
 *
 * Revision 1.76  1998/12/11 17:41:03  sbs
 * changed N_xxxx into T_xxxx in FoldPrfScalars AND
 * inserted a correct default case....
 *
 * Revision 1.75  1998/12/10 17:26:03  sbs
 * ScalarPrf => FoldPrfScalars
 * CFprf streamlined (a bit)
 *
 * Revision 1.74  1998/10/29 16:58:27  cg
 * Bug fixed when constant folding is used during typechecking:
 * works now even if no concrete shapes can be determined.
 *
 * Revision 1.73  1998/08/31 16:04:38  sbs
 * expanded comment in Fold reshape-function
 *
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
 * ... [eliminated] ...
 *
 * Revision 1.1  1995/02/13  16:55:00  asi
 * Initial revision
 *
 *
 */

/******************************************************************************
 * srs: Usage of arg_info in the CF context
 *
 * INFO_CF_TYPE(n)   : type of primitive function (CFlet)
 *                   : type of generator (CFwith)
 * mask[0]/[1]       : DEF and USE information
 * INFO_CF_ASSIGN(n) : (node[0]) arg_node of last N_assign node (CFassign). Used in CFid.
 * INFO_CF_VARNO(n)  : no of elements used in masks
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
#include "generatemasks.h"
#include "DupTree.h"
#include "LoopInvariantRemoval.h"
#include "ConstantFolding.h"

#define MAXARG 3
#define FALSE 0
#define TRUE 1

extern int linenum;

typedef struct INSIDE_WL {
    char *wl_name;
    struct INSIDE_WL *next;
} inside_wl;

static inside_wl *inside_wl_root;

/*
 * ATTENTION: the macro CONST_VAL is intentionally defined locally here
 *  rather than in tree_compound.h since due to the type system of C
 *  it ALWAYS returns a double!!! Therefore, it is of limited use only!
 */

#define CONST_VAL(n)                                                                     \
    ((NODE_TYPE (n) == N_num)                                                            \
       ? NUM_VAL (n)                                                                     \
       : ((NODE_TYPE (n) == N_float)                                                     \
            ? FLOAT_VAL (n)                                                              \
            : ((NODE_TYPE (n) == N_double)                                               \
                 ? DOUBLE_VAL (n)                                                        \
                 : ((NODE_TYPE (n) == N_char) ? CHAR_VAL (n) : BOOL_VAL (n)))))

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
    int ok, i;

    DBUG_ENTER ("CompareNumArrayElts");

    ok = 1;

    /* compare elements */

    if ((ARRAY_VECLEN (array1) == ARRAY_VECLEN (array2))
        && (ARRAY_VECTYPE (array1) == ARRAY_VECTYPE (array2))) {
        switch (ARRAY_VECTYPE (array1)) {
        case T_int:
            for (i = 0; i < ARRAY_VECLEN (array1); i++) {
                if (((int *)ARRAY_CONSTVEC (array1))[i]
                    != ((int *)ARRAY_CONSTVEC (array2))[i]) {
                    ok = 0;
                    break;
                }
            }
            break;
        case T_float:
            for (i = 0; i < ARRAY_VECLEN (array1); i++) {
                if (((float *)ARRAY_CONSTVEC (array1))[i]
                    != ((float *)ARRAY_CONSTVEC (array2))[i]) {
                    ok = 0;
                    break;
                }
            }
            break;
        case T_double:
            for (i = 0; i < ARRAY_VECLEN (array1); i++) {
                if (((double *)ARRAY_CONSTVEC (array1))[i]
                    != ((double *)ARRAY_CONSTVEC (array2))[i]) {
                    ok = 0;
                    break;
                }
            }
            break;
        default:
            ok = 0;
        }
    } else {
        ok = 0;
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

    if (arg_node != NULL) {
        switch (NODE_TYPE (arg_node)) {
        case N_num:
        case N_float:
        case N_double:
        case N_str:
        case N_bool:
            isit = TRUE;
            break;
        case N_array:
            if (ARRAY_VECTYPE (arg_node) == T_unknown) {
                isit = TRUE;
                expr = ARRAY_AELEMS (arg_node);
                while ((expr != NULL) && (isit == TRUE)) {
                    if (NODE_TYPE (EXPRS_EXPR (expr)) == N_id)
                        isit = FALSE;
                    expr = EXPRS_NEXT (expr);
                }
            } else
                isit = TRUE;
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
    funptr *tmp_tab;
#ifndef DBUG_OFF
    int mem_cf_expr = cf_expr;
#endif

    DBUG_ENTER ("ConstantFolding");
    DBUG_PRINT ("OPT", ("CONSTANT FOLDING"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    tmp_tab = act_tab;
    act_tab = cf_tab;

    info_node = MakeNode (N_info);
    inside_wl_root = NULL;

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    act_tab = tmp_tab;

    DBUG_PRINT ("OPT", ("                        result: %d", cf_expr - mem_cf_expr));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));
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

    if (ASSIGN_CF (returnnode) != NULL) {
        ASSIGN_NEXT (ASSIGN_CF (returnnode)) = returnnode;
        returnnode = ASSIGN_CF (returnnode);
        ASSIGN_CF (ASSIGN_NEXT (returnnode)) = NULL;
    }

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

/******************************************************************************
 *
 * function:
 *  node * CFarray( node * arg_node, node * arg_info)
 *
 * description:
 *   traverses through its elements and after doing so checks whether the array
 *   has become constant. In that case, the required CONSTVEC annotation is
 *   created and attached to the N_array node.
 *
 ******************************************************************************/

node *
CFarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFarray");
    if (ARRAY_AELEMS (arg_node) != NULL)
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);

    if (IsConstantArray (arg_node, T_unknown)) {
        ARRAY_ISCONST (arg_node) = TRUE;
        ARRAY_VECTYPE (arg_node) = T_int;
        ARRAY_CONSTVEC (arg_node)
          = Array2IntVec (ARRAY_AELEMS (arg_node), &ARRAY_VECLEN (arg_node));
    }
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

    /*
     * This is old stuff:
     *
     * mrd = MRD_GETSUBST( ID_VARNO(arg_node), INFO_CF_VARNO( arg_info));
     */
    mrd = MRD_GETCFID (ID_VARNO (arg_node), INFO_CF_VARNO (arg_info));

    /*
     * check if this is an Id introduced by flatten (for WLs). This
     * Ids shall not be replaced by another Id. The CF for the old
     * WLs prevent this with a dirty trick described in optimize.c,
     * OPTTrav(), switch N_assign, N_let. The new WL does it better
     * with inside_wl struct.
     */
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
                 *
                 *   int[] f()
                 *   {
                 *     ret = ... external C-implementation ...
                 *     return (ret);
                 *   }
                 *
                 *   int main()
                 *   {
                 *     int[5] a;
                 *
                 *     a = f();
                 *     return ( ... a ... );
                 *   }
                 *
                 * function-inlining leads to
                 *
                 *   int main()
                 *   {
                 *     int[5] a;
                 *     int[] inl_tmp;
                 *
                 *     inl__tmp = ret;
                 *     a = inl__tmp;
                 *     return ( ... a ... );
                 *   }.
                 *
                 * When constant-folding now eliminates "a", the array-shape "[5]" must be
                 * handed over to "inl__tmp" (a = inl__tmp):
                 *
                 *   int main()
                 *   {
                 *     int[5] inl_tmp;
                 *
                 *     inl__tmp = ret;
                 *     return ( ... inl__tmp ... );
                 *   }
                 *
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
        case N_array:
            if (ARRAY_ISCONST (mrd) && (ID_CONSTVEC (arg_node) == NULL)) {
                AnnotateIdWithConstVec (mrd, arg_node);
            }
            break;
        case N_prf:
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
 *                  WHILE_DEFMASK, WHILE_USEMASK, WHILE_TERMMASK
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
    if (N_bool == NODE_TYPE (WHILE_COND (arg_node))) {
        if (!BOOL_VAL (WHILE_COND (arg_node)))
            trav_body = FALSE;
        else
            arg_node = While2Do (arg_node);
    }
    /*
     * is the condition a variable, which can be infered to false?
     */
    if (N_id == NODE_TYPE (WHILE_COND (arg_node))) {
        last_value
          = MRD_GETLAST (ID_VARNO (WHILE_COND (arg_node)), INFO_CF_VARNO (arg_info));
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
        MinusMask (INFO_DEF, WHILE_DEFMASK (arg_node), INFO_CF_VARNO (arg_info));
        MinusMask (INFO_USE, WHILE_USEMASK (arg_node), INFO_CF_VARNO (arg_info));
        MinusMask (INFO_USE, WHILE_TERMMASK (arg_node), INFO_CF_VARNO (arg_info));
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
 *                  DO_TERMMASK
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
            MinusMask (INFO_USE, DO_TERMMASK (arg_node), INFO_CF_VARNO (arg_info));
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
            MinusMask (INFO_DEF, COND_ELSEDEFMASK (arg_node), INFO_CF_VARNO (arg_info));
            MinusMask (INFO_USE, COND_ELSEUSEMASK (arg_node), INFO_CF_VARNO (arg_info));
            returnnode = COND_THENINSTR (arg_node);
            COND_THEN (arg_node) = NULL;
            FreeTree (arg_node);
            cf_expr++;
        } else {
            DBUG_PRINT ("CF", ("else-part of conditional eliminated in line %d",
                               NODE_LINE (arg_node)));
            MinusMask (INFO_DEF, COND_THENDEFMASK (arg_node), INFO_CF_VARNO (arg_info));
            MinusMask (INFO_USE, COND_THENUSEMASK (arg_node), INFO_CF_VARNO (arg_info));
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

    if (ARRAY_CONSTVEC (array) == NULL) {
        expr = ARRAY_AELEMS (array);
        for (i = 0; i < SHP_SEG_SIZE; i++) {
            if (expr != NULL) {
                vec_dim++;
                vec_shape[i] = NUM_VAL (ARRAY_AELEMS (expr));
                if (EXPRS_NEXT (expr) != NULL)
                    expr = EXPRS_NEXT (expr);
                else
                    expr = NULL;
            } else
                vec_shape[i] = 0;
        }
    } else {
        vec_dim = ARRAY_VECLEN (array);
        for (i = 0; i < vec_dim; i++)
            vec_shape[i] = ((int *)ARRAY_CONSTVEC (array))[i];
        for (i = vec_dim; i < SHP_SEG_SIZE; i++)
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

    size = ARRAY_VECLEN (array);
    if (size == SCALAR) {
        size = 1;
        array = ARRAY_AELEMS (array);
        while (EXPRS_NEXT (array) != NULL) {
            array = EXPRS_NEXT (array);
            size++;
        }
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
    node *new_node;
    node *expr0, *expr1;
    int i;
    int *tmp_ivec;
    float *tmp_fvec;
    double *tmp_dvec;

    DBUG_ENTER ("DupPartialArray");

    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[NODE_TYPE (array)]));

    if (length > 0) {
        new_node = DupArray (array, arg_info);
        expr0 = ARRAY_AELEMS (new_node);
        /*
         * Goto start position and erase the elements which were not wanted.
         */
        for (i = 0; i < start; i++) {

            DBUG_ASSERT ((expr0 != NULL), ("not a constant vector or vector too small"));

            expr1 = EXPRS_NEXT (expr0);
            EXPRS_NEXT (expr0) = NULL;
            if (expr0 != NULL)
                FreeTree (expr0);
            expr0 = expr1;
        }
        /*
         * Duplicate array till length reached
         */

        ARRAY_AELEMS (new_node) = expr0;

        for (i = 0; i < length; i++) {

            DBUG_ASSERT ((array != NULL), ("not a constant vector or vector too small"));

            if (NODE_TYPE (EXPRS_EXPR (expr0)) == N_id)
                DEC_VAR (arg_info->mask[1],
                         VARDEC_VARNO (ID_VARDEC (EXPRS_EXPR (expr0))));

            expr1 = expr0;
            expr0 = EXPRS_NEXT (expr0);
        }
        /*
         * Erase rest of array.
         */
        if (expr0 != NULL)
            FreeTree (expr0);
        EXPRS_NEXT (expr1) = NULL;

        switch (ARRAY_VECTYPE (array)) {
        case T_int:
            FREE (ARRAY_CONSTVEC (new_node));
            tmp_ivec = Array2IntVec (ARRAY_AELEMS (new_node), NULL);
            ((int *)ARRAY_CONSTVEC (new_node)) = tmp_ivec;
            ARRAY_VECLEN (new_node) = length;
            break;
        case T_float:
            FREE (ARRAY_CONSTVEC (new_node));
            tmp_fvec = Array2FloatVec (ARRAY_AELEMS (new_node), NULL);
            ((float *)ARRAY_CONSTVEC (new_node)) = tmp_fvec;
            ARRAY_VECLEN (new_node) = length;
            break;
        case T_double:
            FREE (ARRAY_CONSTVEC (new_node));
            tmp_dvec = Array2DblVec (ARRAY_AELEMS (new_node), NULL);
            ((double *)ARRAY_CONSTVEC (new_node)) = tmp_dvec;
            ARRAY_VECLEN (new_node) = length;
            break;
        default:
            ARRAY_VECLEN (new_node) = 0;
        }

    } else {
        new_node = MakeArray (NULL);
    }

    DBUG_RETURN (new_node);
}

/*
 *
 *  functionname  : FoundZero
 *  arguments     : 1) node to be examine
 *		    R) TRUE if node contains a 0, 0.0 , [0, ..., 0] or [0.0, ..., 0.0]
 *		       FALSE otherwise
 *  description   : determines if expression is a constant containing zero
 *  global vars   : syntax_tree, N_num, N_float, N_array
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., NULL, FALSE, TRUE
 *
 *  remarks       : --
 *
 */
int
FoundZero (node *arg_node)
{
    int FoundZero = FALSE;
    node *expr;

    DBUG_ENTER ("FoundZero");
    switch (NODE_TYPE (arg_node)) {
    case N_num:
    case N_float:
    case N_double:
        if (CONST_VAL (arg_node) == 0)
            FoundZero = TRUE;
        break;
    case N_array:
        expr = ARRAY_AELEMS (arg_node);
        while (expr != NULL) {
            if (CONST_VAL (EXPRS_EXPR (expr)) == 0)
                FoundZero = TRUE;
            expr = EXPRS_NEXT (expr);
        }
        break;
    default:
        FoundZero = FALSE;
        break;
    }
    DBUG_RETURN (FoundZero);
}

/******************************************************************************
 *
 * function:
 *   node *FoldPrfScalars( prf prf_name, node **arg, types *res_type, int swap)
 *
 * description:
 *   computes prf_name( arg[0], ...., arg[n]) in case (swap==FALSE),
 *            prf_name( arg[1], arg[0], arg[2], ..., arg[n]) otherwise;
 *   and returns either a new node of type "res_type" which carries the result
 *               or NULL (!) if the folding could not be performed.
 *
 ******************************************************************************/

node *
FoldPrfScalars (prf prf_name, node **arg, types *res_type, int swap)
{
    node *tmp, *res;

    /*
     * These macros compute all non array primitive functions
     */

#define SET_RESULT(res, rt, expr)                                                        \
    switch (TYPES_BASETYPE (rt)) {                                                       \
    case T_int:                                                                          \
        res = MakeNum (expr);                                                            \
        break;                                                                           \
    case T_float:                                                                        \
        res = MakeFloat (expr);                                                          \
        break;                                                                           \
    case T_double:                                                                       \
        res = MakeDouble (expr);                                                         \
        break;                                                                           \
    case T_char:                                                                         \
        res = MakeChar (expr);                                                           \
        break;                                                                           \
    case T_bool:                                                                         \
        res = MakeBool (expr);                                                           \
        break;                                                                           \
    default:                                                                             \
        res = NULL;                                                                      \
        break;                                                                           \
    }

#define MON_OP(op, a1, res, rt) SET_RESULT (res, rt, op (CONST_VAL (a1)))

#define ARI_OP(op, a1, a2, res, rt) SET_RESULT (res, rt, CONST_VAL (a1) op CONST_VAL (a2))

#define REL_OP(op, a1, a2, res, rt) res = MakeBool (CONST_VAL (a1) op CONST_VAL (a2))

#define MINMAX(op, a1, a2, res, rt)                                                      \
    SET_RESULT (res, rt,                                                                 \
                (CONST_VAL (a1) op CONST_VAL (a2) ? CONST_VAL (a1) : CONST_VAL (a2)))

    DBUG_ENTER ("FoldPrfScalars");

    if (swap) {
        tmp = arg[0];
        arg[0] = arg[1];
        arg[1] = tmp;
    }

    switch (prf_name) {
    case F_not:
        MON_OP (!, arg[0], res, res_type);
        break;
    case F_abs:
        MON_OP (abs, arg[0], res, res_type);
        break;

    case F_add:
    case F_add_AxA:
    case F_add_AxS:
    case F_add_SxA:
        ARI_OP (+, arg[0], arg[1], res, res_type);
        break;
    case F_sub:
    case F_sub_AxA:
    case F_sub_AxS:
    case F_sub_SxA:
        ARI_OP (-, arg[0], arg[1], res, res_type);
        break;
    case F_mul:
    case F_mul_AxA:
    case F_mul_AxS:
    case F_mul_SxA:
        ARI_OP (*, arg[0], arg[1], res, res_type);
        break;
    case F_div:
    case F_div_AxA:
    case F_div_AxS:
    case F_div_SxA:
        ARI_OP (/, arg[0], arg[1], res, res_type);
        break;
    case F_mod:
        res = MakeNum (NUM_VAL (arg[0]) % NUM_VAL (arg[1]));
        break;
    case F_gt:
        REL_OP (>, arg[0], arg[1], res, res_type);
        break;
    case F_lt:
        REL_OP (<, arg[0], arg[1], res, res_type);
        break;
    case F_ge:
        REL_OP (>=, arg[0], arg[1], res, res_type);
        break;
    case F_le:
        REL_OP (<=, arg[0], arg[1], res, res_type);
        break;
    case F_eq:
        REL_OP (==, arg[0], arg[1], res, res_type);
        break;
    case F_neq:
        REL_OP (!=, arg[0], arg[1], res, res_type);
        break;
    case F_and:
        ARI_OP (&&, arg[0], arg[1], res, res_type);
        break;
    case F_or:
        ARI_OP (||, arg[0], arg[1], res, res_type);
        break;

    case F_min:
        MINMAX (<, arg[0], arg[1], res, res_type);
        break;
    case F_max:
        MINMAX (>, arg[0], arg[1], res, res_type);
        break;

    default:
        NOTE2 (("CF not yet implemented for prf \"%s\"!", prf_string[prf_name]));
        res = NULL;
        break;
    }
    cf_expr++;
    DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_name]));

    if (swap) { /* we do not want to make any side-effects 8-(( */
        tmp = arg[0];
        arg[0] = arg[1];
        arg[1] = tmp;
    }

    DBUG_RETURN (res);
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
        && (test_pattern == CONST_VAL (arg[test_arg]))) {
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
        switch (PRF_PRF (arg_node)) {
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

    if (ARRAY_CONSTVEC (array) == NULL) {
        tmp = ARRAY_AELEMS (array);

        for (i = 0; i < pos; i++)
            tmp = EXPRS_NEXT (tmp);

        tmp = DupTree (EXPRS_EXPR (tmp), NULL);
    } else {
        i = ((int *)ARRAY_CONSTVEC (array))[pos];
        tmp = MakeNum (i);
    }

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

    /* Calculate length of result array */
    length = 1;
    if (0 < result_dim) {
        result_shape = TYPES_SHPSEG (INFO_CF_TYPE (arg_info));
        for (i = 0; i < result_dim; i++)
            length *= SHPSEG_SHAPE (result_shape, i);
    }

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
        if (vec_dim == array_dim)
            res_node = FetchNum (start, array);
        else
            res_node = DupPartialArray (start, length, array, arg_info);
    } else {
        WARN (linenum, ("Illegal vector for primitive function psi"));
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
    int tmp_len;
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
    switch (PRF_PRF (arg_node)) {
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
            value = MRD_GETDATA (arg[0]->info.ids->node->varno, INFO_CF_VARNO (arg_info));
            if (IsConst (value)) {
                DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
                arg[0] = DupTree (value, NULL);
                used_sofar = arg_info->mask[1];
                arg_info->mask[1] = GenMask (INFO_CF_VARNO (arg_info));
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
            value = MRD_GETDATA (arg[1]->info.ids->node->varno, INFO_CF_VARNO (arg_info));
            if (IsConst (value)) {
                DEC_VAR (arg_info->mask[1], arg[1]->info.ids->node->varno);
                arg[1] = DupTree (value, NULL);
                used_sofar = arg_info->mask[1];
                arg_info->mask[1] = GenMask (INFO_CF_VARNO (arg_info));
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
            while ((expr[0] != NULL) && (expr[1] != NULL)) {
                expr_arg[0] = expr[0]->node[0];
                expr_arg[1] = expr[1]->node[0];
                tmp = FoldPrfScalars (PRF_PRF (arg_node), expr_arg,
                                      INFO_CF_TYPE (arg_info), swap);
                if (tmp != NULL) {
                    expr[0]->node[0] = tmp;
                }
                expr[0] = expr[0]->node[1];
                expr[1] = expr[1]->node[1];
            }
        } else {
            /*
             * Calculate prim-function with one array
             */
            expr[0] = ARRAY_AELEMS (arg[0]);
            expr_arg[1] = arg[1];
            while (expr[0] != NULL) {
                expr_arg[0] = EXPRS_EXPR (expr[0]);
                tmp = FoldPrfScalars (PRF_PRF (arg_node), expr_arg,
                                      INFO_CF_TYPE (arg_info), swap);
                if (tmp != NULL) {
                    EXPRS_EXPR (expr[0]) = tmp;
                }
                expr[0] = EXPRS_NEXT (expr[0]);
            }
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
            tmp = ARRAY_AELEMS (arg[0]);
            do {
                i++;
                tmp = EXPRS_NEXT (tmp);
            } while (tmp != NULL);

            /*
             * Store result in this array
             */
            NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (arg[0]))) = i;
            NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (arg[0]))) = N_num;

            /*
             * Free rest of array and prf-node
             */
            if (EXPRS_NEXT (ARRAY_AELEMS (arg[0])) != NULL)
                FreeTree (EXPRS_NEXT (ARRAY_AELEMS (arg[0])));
            EXPRS_NEXT (ARRAY_AELEMS (arg[0])) = NULL; /* ??? */
                                                       /*
                                                        * Gives Array the correct type
                                                        */
            ARRAY_TYPE (arg[0]) = FreeOneTypes (ARRAY_TYPE (arg[0]));
            ARRAY_TYPE (arg[0]) = DuplicateTypes (INFO_CF_TYPE (arg_info), 0);
            ARRAY_VECLEN (arg[0]) = 1;
            ((int *)ARRAY_CONSTVEC (arg[0])) = Array2IntVec (ARRAY_AELEMS (arg[0]), NULL);
            ARRAY_VECTYPE (arg[0]) = T_int;
            /*
             * Store result
             */
            FREE (arg_node);
            arg_node = arg[0];
            cf_expr++;
            break;

        case N_id:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            SHAPE_2_ARRAY (tmp, arg[0]->info.ids->node->info.types,
                           INFO_CF_TYPE (arg_info));

            if (tmp != NULL) {
                /*
                 * The macro SHAPE_2_ARRAY "returns" NULL if no complete array shape can
                 * be inferred. This happens in particular when constant folding is done
                 * during typechecking. Especially, in module implementations the concrete
                 * shape of formal function parameters can often not be determined.
                 */
                /*
                 * update masks!
                 * When called from TC no masks present!
                 */
                if (arg_info->mask[1])
                    DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
                ((int *)ARRAY_CONSTVEC (tmp))
                  = Array2IntVec (ARRAY_AELEMS (tmp), &ARRAY_VECLEN (tmp));
                ARRAY_VECTYPE (tmp) = T_int;

                FreeTree (arg_node);
                arg_node = tmp;
                cf_expr++;
            }
            break;

        default:
            break;
        }
        break; /* shape */

    /***********************/
    /* Fold reshape-function */
    /***********************/
    case F_reshape: {

        /*
         * we want to eliminate reshape-calls here since they hinder CF
         * in many situations, e.g. when accessing constant arrays
         * that are defined by reshape....
         */
        /* arg_node = arg[1]; */

        /*
         * srs: NoNo, we really shouldn't do that. Imagine the following case:
         *   A = WL () genarray([8]);
         *   B = reshape([2,2,2],A);
         *   C = WL () {...references to B...} modarray(B)
         * If the second line is replaced by B = A then while WLF the second
         * WL operates on an array of the *wrong* shape (if it's possible in
         * this situation.
         * But WLF must not become active here because no generators are known
         * for the array B.
         */

        /*
         * sbs: could this be a solution???
         *      iff the second arg is a constant, then we do throw the
         *      reshape away?!
         */
        /*
         *      if (N_id == NODE_TYPE(arg[1])) {
         *        value = MRD_GETDATA(arg[1]->info.ids->node->varno,
         * INFO_CF_VARNO(arg_info));
         *      }
         *      else {
         *        value = arg[1];
         *      }
         *
         *      if (IsConst(value)) {
         *        if (N_id == NODE_TYPE(arg[0])) {
         *          DEC_VAR(arg_info->mask[1], arg[0]->info.ids->node->varno);
         *        }
         *        arg_node = arg[1];
         *      }
         */
        /*
         * Now, we should (!!!) free the N_prf-node and its first arg...
         * Unfortunately, we did not yet implement it 8-(
         */
        /*
         * sbs: NoNo, we should neither do that since we run into troubles
         *      when actually folding then. "errors/fixed/constfold_0002.sac"
         *      contains an example for that.
         *
         * A better solution for that problem is to use MRD_GET_DATA first,
         * since MrdGet looks behind F_reshapes. Therefore, it does find
         * the constant array, if any, anyway!
         */
        break;
    }

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

            DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
            /*
             * Free argument of prim function
             */
            FreeTree (arg_node->node[0]);
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
        node *shape, *array, *res_array, *first_elem, *new_assign;
        node *tmpn, *modindex, *assign;
        types *array_type;
        node **mrdmask;

        res_array = NULL;

        /*
         * Substitute shape-vector
         */
        if (N_id == NODE_TYPE (arg[0])) {
            shape = MRD_GETDATA (ID_VARNO (arg[0]), INFO_CF_VARNO (arg_info));
        } else
            shape = arg[0];

        /*
         * preserve the array_type!!
         * this is essential, since the "array-substitution" will
         * get const-arrays which are "behind" reshape operations!
         * Therefore, this is the only chance to know about the shape
         * of arg[1]!!
         */
        if (NODE_TYPE (arg[1]) == N_id) {
            array_type = ID_TYPE (arg[1]);
        } else {
            DBUG_ASSERT ((NODE_TYPE (arg[1]) == N_array), "wrong argument node found!");
            array_type = ARRAY_TYPE (arg[1]);
        }
        /*
         * Substitute array iff it refers to a constant!
         *  sbs: added this in order to reach out behind reshape-ops!
         *  Otherwise prgs like "errors/fixed/constfold_0002.sac"
         *  cannot be folded !
         */
        if (N_id == NODE_TYPE (arg[1])) {
            tmpn = MRD_GETDATA (ID_VARNO (arg[1]), INFO_CF_VARNO (arg_info));
            if (IsConst (tmpn))
                arg[1] = tmpn;
        }

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

            /* let's have a closer look at the id. If it is a prf modarray
               and we index the element which is modified by this prf,
               the second arg of the prf can be used.
               Else we search for an N_array node.*/
            ok = 0;
            tmpn = arg[1];
            mrdmask = MRD_TOS.varlist;
            do {
                assign = mrdmask[ID_VARNO (tmpn)];
                ok = 1;
                if (assign && (N_assign == NODE_TYPE (assign))
                    && (N_let == NODE_TYPE (ASSIGN_INSTR (assign)))) {
                    mrdmask = (node **)ASSIGN_MRDMASK (assign);
                    array = LET_EXPR (ASSIGN_INSTR (assign));
                } else
                    array = NULL;

                if (array && (N_array == NODE_TYPE (array)))
                    ; /* leave do-loop */
                else if (array && (N_prf == NODE_TYPE (array))
                         && (F_modarray == PRF_PRF (array))) {
                    /* check index of prf modarray */
                    modindex = PRF_ARG2 (array);
                    if (N_id == NODE_TYPE (modindex))
                        modindex
                          = MRD_GETDATA (ID_VARNO (modindex), INFO_CF_VARNO (arg_info));
                    if (IsConstantArray (modindex, N_num)
                        && CompareNumArrayType (shape, modindex)) {
                        if (CompareNumArrayElts (shape, modindex)) {
                            node *val = PRF_ARG3 (array);
                            if (NODE_TYPE (val) == N_id) {
                                if (mrdmask[ID_VARNO (val)]
                                    == ((node **)ASSIGN_MRDMASK (
                                         INFO_CF_ASSIGN (arg_info)))[ID_VARNO (val)]) {
                                    res_array = DupTree (val, NULL);
                                } else {
                                    /*
                                     * A variable with the same name has been defined
                                     * between the calls to modarray() and psi(), i.e. we
                                     * have to deal with the following situation:
                                     *
                                     * A = modarray( A, [0], val);
                                     * ....
                                     * val = .... ;
                                     *
                                     * ... = psi( [0], A);
                                     *
                                     * As a consequence, the call to psi obviously cannot
                                     * be replaced by the variable 'val'. However, we can
                                     * use the following trick when inserting a new, fresh
                                     * identifier:
                                     *
                                     * A = modarray( A, [0], val);
                                     * fresh_var = val;
                                     * ....
                                     * val = .... ;
                                     *
                                     * ... = psi( [0], A);
                                     *
                                     * Now, the call to psi() may safely be replaced by
                                     * the variable 'fresh_var'.
                                     *
                                     * The following lines of code implement exactly this
                                     * transformation scheme.
                                     */
                                    char *fresh_var;
                                    ids *new_ids;
                                    node *vardecs, *new_vardec;

                                    if (IDS_VARDEC (LET_IDS (ASSIGN_INSTR (assign)))
                                        != ID_VARDEC (val)) {
                                        /*
                                         * The above condition may not hold only in very
                                         * rare and dubious circumstance.
                                         */

                                        if (ASSIGN_CF (assign) == NULL) {
                                            /*
                                             * No fresh variable has yet been introduced
                                             * for this application of prf modarray().
                                             */

                                            /*
                                             * create a new variable
                                             */
                                            fresh_var = TmpVar ();
                                            new_ids
                                              = MakeIds (fresh_var, NULL, ST_regular);

                                            /*
                                             * create a new vardec for this variable,
                                             * increment VARNO
                                             */
                                            vardecs = IDS_VARDEC (
                                              LET_IDS (ASSIGN_INSTR (assign)));
                                            new_vardec
                                              = MakeVardec (StringCopy (fresh_var),
                                                            DuplicateTypes (ID_TYPE (val),
                                                                            0),
                                                            VARDEC_NEXT (vardecs));
                                            VARDEC_NEXT (vardecs) = new_vardec;
                                            VARDEC_VARNO (new_vardec)
                                              = (INFO_CF_VARNO (arg_info))++;
                                            IDS_VARDEC (new_ids) = new_vardec;

                                            /*
                                             * create the new assignment,
                                             * build masks for this assignment
                                             */
                                            new_assign
                                              = MakeAssign (MakeLet (DupTree (val, NULL),
                                                                     new_ids),
                                                            NULL);
                                            ASSIGN_MRDMASK (new_assign)
                                              = GenMask (INFO_CF_VARNO (arg_info));
                                            ASSIGN_MRDMASK (new_assign)
                                              = CopyMask (ASSIGN_MRDMASK (assign),
                                                          INFO_CF_VARNO (arg_info) - 1,
                                                          ASSIGN_MRDMASK (new_assign),
                                                          INFO_CF_VARNO (arg_info));
                                            ASSIGN_DEFMASK (new_assign)
                                              = ReGenMask (ASSIGN_DEFMASK (new_assign),
                                                           INFO_CF_VARNO (arg_info));
                                            INC_VAR (ASSIGN_DEFMASK (new_assign),
                                                     IDS_VARNO (new_ids));
                                            ASSIGN_USEMASK (new_assign)
                                              = ReGenMask (ASSIGN_USEMASK (new_assign),
                                                           INFO_CF_VARNO (arg_info));
                                            INC_VAR (ASSIGN_USEMASK (new_assign),
                                                     ID_VARNO (val));

                                            /*
                                             * store new assignment in ASSIGN_CF,
                                             * correct current MRD-masks
                                             */
                                            ASSIGN_CF (assign) = new_assign;
                                            ExpandMRDL (1);
                                            MRD (IDS_VARNO (new_ids)) = new_assign;
                                            FREE (
                                              ASSIGN_MRDMASK (INFO_CF_ASSIGN (arg_info)));
                                            ASSIGN_MRDMASK (INFO_CF_ASSIGN (arg_info))
                                              = DupMask (MRD_LIST,
                                                         INFO_CF_VARNO (arg_info));
                                        } else {
                                            /*
                                             * A fresh variable has already been inserted
                                             * behind this application of prf modarray()
                                             * within this run of constant folding, i.e.
                                             * the assignment is not yet really inserted
                                             * but temporarily stored in the ASSIGN_CF
                                             * field. As a consequence, this variable must
                                             * still be a fresh one and may therefore be
                                             * reused for further applications of prf
                                             * psi() referring to this application of prf
                                             * modarray().
                                             */
                                            ids *still_fresh_var = LET_IDS (
                                              ASSIGN_INSTR (ASSIGN_CF (assign)));
                                            fresh_var = IDS_NAME (still_fresh_var);
                                            new_vardec = IDS_VARDEC (still_fresh_var);
                                        }

                                        res_array = MakeId (StringCopy (fresh_var), NULL,
                                                            ST_regular);
                                        ID_VARDEC (res_array) = new_vardec;
                                    }
                                }
                            } else {
                                res_array = DupTree (val, NULL);
                            }
                        } else {
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
            MinusMask (INFO_USE, ASSIGN_USEMASK (INFO_CF_ASSIGN (arg_info)),
                       INFO_CF_VARNO (arg_info));
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
                /* access current MRD-masks */
                base_array = MRD (ID_VARNO (base_array));
            else {
                DBUG_ASSERT (ASSIGN_MRDMASK (tmpn),
                             ("MRDMASKs are NULL, ArrayPrf, modarray"));
                /* access older MRD-masks */
                base_array = (node *)ASSIGN_MRDMASK (tmpn)[ID_VARNO (base_array)];
            }

            tmpn = base_array;

            if (base_array && /* mrd exists */
                N_assign == NODE_TYPE (base_array)
                && N_let == NODE_TYPE (ASSIGN_INSTR (base_array))) {
                base_array = LET_EXPR (ASSIGN_INSTR (base_array));
            }
        }

        /* search for mrd index vector and val */
        if (N_id == NODE_TYPE (vectorn))
            vectorn = MRD_GETDATA (ID_VARNO (vectorn), INFO_CF_VARNO (arg_info));
        if (N_id == NODE_TYPE (valn))
            valn = MRD_GETDATA (ID_VARNO (valn), INFO_CF_VARNO (arg_info));

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
    /*
     *  After CF constant arrays may have lost their special constant propagation.
     *  Therefore it's neccessary to rebuild it.
     */
    if (NODE_TYPE (arg_node) == N_array) {
        switch (ARRAY_VECTYPE (arg_node)) {
        case T_int:
            FREE (ARRAY_CONSTVEC (arg_node));
            ((int *)ARRAY_CONSTVEC (arg_node))
              = Array2IntVec (ARRAY_AELEMS (arg_node), &tmp_len);
            ARRAY_VECLEN (arg_node) = tmp_len;
            ARRAY_ISCONST (arg_node) = TRUE;
            break;
        case T_bool:
            FREE (ARRAY_CONSTVEC (arg_node));
            ((int *)ARRAY_CONSTVEC (arg_node))
              = Array2BoolVec (ARRAY_AELEMS (arg_node), &tmp_len);
            ARRAY_VECLEN (arg_node) = tmp_len;
            ARRAY_ISCONST (arg_node) = TRUE;
            break;
        case T_char:
            FREE (ARRAY_CONSTVEC (arg_node));
            ((char *)ARRAY_CONSTVEC (arg_node))
              = Array2CharVec (ARRAY_AELEMS (arg_node), &tmp_len);
            ARRAY_VECLEN (arg_node) = tmp_len;
            ARRAY_ISCONST (arg_node) = TRUE;
            break;
        case T_float:
            FREE (ARRAY_CONSTVEC (arg_node));
            ((float *)ARRAY_CONSTVEC (arg_node))
              = Array2FloatVec (ARRAY_AELEMS (arg_node), &tmp_len);
            ARRAY_VECLEN (arg_node) = tmp_len;
            ARRAY_ISCONST (arg_node) = TRUE;
            break;
        case T_double:
            FREE (ARRAY_CONSTVEC (arg_node));
            ((double *)ARRAY_CONSTVEC (arg_node))
              = Array2DblVec (ARRAY_AELEMS (arg_node), &tmp_len);
            ARRAY_VECLEN (arg_node) = tmp_len;
            ARRAY_ISCONST (arg_node) = TRUE;
            break;
        default:
            /* Nothing to do ! */
            break;
        }
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
 *  internal funs : FoldPrfScalars, ArrayPrf
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

    /*
     * First we substitute all arguments (arrays are not propagated!),
     * i.e., we do propagate scalars!
     */
    if ((PRF_PRF (arg_node) != F_shape) && (PRF_PRF (arg_node) != F_dim))
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);

    /*
     * Copy pointers to the args of the Prf into the array "arg":
     */
    tmp = PRF_ARGS (arg_node);
    for (i = 0; i < MAXARG; i++) {
        if (tmp) {
            arg[i] = NodeBehindCast (EXPRS_EXPR (tmp));
            tmp = EXPRS_NEXT (tmp);
        } else {
            arg[i] = NULL;
        }
    }

    if (PRF_PRF (arg_node) <= F_neq) { /* prfs on scalars only !! */
        if (IsConst (arg[0]) && ((PRF_PRF (arg_node) <= F_not) || IsConst (arg[1]))) {
            /*
             * this is a constants only application!
             */
            if ((PRF_PRF (arg_node) == F_div) && FoundZero (arg[1])) {
                WARN (NODE_LINE (arg_node), ("Division by zero expected"));
            } else {
                tmp = FoldPrfScalars (PRF_PRF (arg_node), arg, INFO_CF_TYPE (arg_info),
                                      FALSE);
                if (tmp != NULL) {
                    FreeTree (arg_node);
                    arg_node = tmp;
                }
            }
        }
    } else { /* prfs that require at least one array as argument! */
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
