/*
 *
 * $Log$
 * Revision 1.42  1996/02/13 15:09:28  asi
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
#define INFO_TYPE arg_info->info.types
#define FALSE 0
#define TRUE 1

/*
 *  This macros secects the right element out of the union info
 */
#define SELARG(n)                                                                        \
    ((n->nodetype == N_num)                                                              \
       ? NUM_VAL (n)                                                                     \
       : ((n->nodetype == N_float) ? FLOAT_VAL (n) : DOUBLE_VAL (n)))

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
    if (NULL != FUNDEF_BODY (arg_node))
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
    ntype = NODE_TYPE (ASSIGN_INSTR (arg_node));
    if (N_return != ntype) {
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
    } else {
        returnnode = arg_node;
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
    INFO_TYPE = GetType (VARDEC_TYPE (IDS_VARDEC (LET_IDS (arg_node))));

    /* Trav expression */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_TYPE = NULL;

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

    DBUG_ENTER ("CFid");
    MRD_GETSUBST (mrd, ID_VARNO (arg_node), INFO_VARNO);
    if (NULL != mrd) {
        switch (NODE_TYPE (mrd)) {
        case N_id:
            DEC_VAR (INFO_USE, ID_VARNO (arg_node));
            FreeTree (arg_node);
            arg_node = DupTree (mrd, NULL);
            INC_VAR (INFO_USE, ID_VARNO (arg_node));
            break;
        case N_num:
        case N_float:
        case N_double:
        case N_bool:
        case N_str:
            DEC_VAR (INFO_USE, ID_VARNO (arg_node));
            FreeTree (arg_node);
            arg_node = DupTree (mrd, NULL);
            break;
        case N_prf:
        case N_array:
        case N_ap:
        case N_with:
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
node *
CFcast (node *arg_node, node *arg_info)
{
    node *next_node;

    DBUG_ENTER ("CFcast");

    next_node = Trav (CAST_EXPR (arg_node), arg_info);

    CAST_EXPR (arg_node) = NULL;
    FreeTree (arg_node);
    DBUG_RETURN (next_node);
}

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

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);

    if (N_bool == NODE_TYPE (WHILE_COND (arg_node)))
        if (!BOOL_VAL (WHILE_COND (arg_node)))
            trav_body = FALSE;
        else
            arg_node = While2Do (arg_node);

    if (N_id == NODE_TYPE (WHILE_COND (arg_node))) {
        MRD_GETLAST (last_value, ID_VARNO (WHILE_COND (arg_node)), INFO_VARNO);
        if ((NULL != last_value) && (N_bool == NODE_TYPE (last_value))) {
            if (!BOOL_VAL (last_value))
                trav_body = FALSE;
            else
                arg_node = While2Do (arg_node);
        }
    }

    if (trav_body) {
        WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);
    } else {
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
 *  macros        : INFO_TYPE, WITH_GEN, GEN_VARDEC, VARDEC_TYPE, WITH_OPERATOR,
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
    oldtype = INFO_TYPE;
    INFO_TYPE = VARDEC_TYPE (GEN_VARDEC (WITH_GEN (arg_node)));

    WITH_GEN (arg_node) = OPTTrav (WITH_GEN (arg_node), arg_info, arg_node);

    INFO_TYPE = oldtype;

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
    while (2 == array->nnode) {
        array = array->node[1];
        size++;
    }
    DBUG_RETURN (size);
}

/*
 *
 *  functionname  : FetchNum
 *  arguments     : 1) position in array
 *                  2) ptr to the array
 *                  R) ptr to fetchted element
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : DupTree
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
FetchNum (int start, node *array)
{
    int i;
    node *returnexpr;

    DBUG_ENTER ("FetchNum");

    array = array->node[0];

    for (i = 0; i < start; i++)
        array = array->node[1];

    returnexpr = DupTree (array->node[0], NULL);

    DBUG_RETURN (returnexpr);
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
        if (N_id == expr->node[0]->nodetype)
            DEC_VAR (arg_info->mask[1], expr->node[0]->info.ids->node->varno);
        expr->nnode = 1;
        array = array->node[1];
        r_expr = expr;

        /*
         * Duplicate rest of array till length reached
         */
        for (i = 1; i < length; i++) {
            expr->nnode++;
            expr->node[1] = MakeNode (N_exprs);
            expr = expr->node[1];
            expr->node[0] = DupTree (array->node[0], NULL);
            if (N_id == expr->node[0]->nodetype)
                DEC_VAR (arg_info->mask[1], expr->node[0]->info.ids->node->varno);
            expr->nnode = 1;
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
    switch (arg_node->nodetype) {
    case N_num:
    case N_float:
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
 *  functionname  : SkalarPrf
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
SkalarPrf (node **arg, prf prf_type, types *res_type, int swap)
{

/*
 * This macro calculates all non array primitive functions
 */
#define ARI(op, a1, a2)                                                                  \
    {                                                                                    \
        switch (res_type->simpletype) {                                                  \
        case T_float:                                                                    \
            if (!swap) {                                                                 \
                a1->info.cfloat = SELARG (a1) op SELARG (a2);                            \
            } else {                                                                     \
                a1->info.cfloat = SELARG (a2) op SELARG (a1);                            \
            }                                                                            \
            a1->nodetype = N_float;                                                      \
            break;                                                                       \
        case T_double:                                                                   \
            if (!swap) {                                                                 \
                a1->info.cdbl = SELARG (a1) op SELARG (a2);                              \
            } else {                                                                     \
                a1->info.cdbl = SELARG (a2) op SELARG (a1);                              \
            }                                                                            \
            a1->nodetype = N_double;                                                     \
            break;                                                                       \
        case T_int:                                                                      \
            if (!swap) {                                                                 \
                a1->info.cint = a1->info.cint op a2->info.cint;                          \
            } else {                                                                     \
                a1->info.cint = a2->info.cint op a1->info.cint;                          \
            }                                                                            \
            break;                                                                       \
        case T_bool:                                                                     \
            if (!swap) {                                                                 \
                a1->info.cint = a1->info.cint op a2->info.cint;                          \
            } else {                                                                     \
                a1->info.cint = a2->info.cint op a1->info.cint;                          \
            }                                                                            \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT ((FALSE), "Type not implemented for Constant folding");          \
            break;                                                                       \
        }                                                                                \
        cf_expr++;                                                                       \
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_type]));       \
    }

    DBUG_ENTER ("SkalarPrf");

    /*
     * Calculate primitive Functions
     */
    switch (prf_type) {
    case F_not:
        arg[0]->info.cint = !arg[0]->info.cint;
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
    case F_gt:
        ARI (>, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_lt:
        ARI (<, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_ge:
        ARI (>=, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_le:
        ARI (<=, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_eq:
        ARI (==, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_neq:
        ARI (!=, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_and:
        ARI (&&, arg[0], arg[1]);
        break;
    case F_or:
        ARI (||, arg[0], arg[1]);
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
 *  macros        : DBUG..., DEC_VAR, SELARG, NULL
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

    arg[0] = arg_node->node[0]->node[0];
    arg[1] = arg_node->node[0]->node[1]->node[0];
    tmp = arg_node;
    node_t = arg[test_arg]->nodetype;
    if (((N_num == node_t) || (N_bool == node_t) || (N_float == node_t)
         || (N_double == node_t))
        && (test_pattern == SELARG (arg[test_arg]))) {
        arg_node = arg[res_arg];
        if ((N_id == arg[!res_arg]->nodetype) && (NULL != arg_info)) {
            DEC_VAR (arg_info->mask[1], arg[!res_arg]->info.ids->node->varno);
        }
        cf_expr++;
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[tmp->info.prf]));
        FreePrf2 (tmp, res_arg);
        arg_node = arg[res_arg];
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : NoConstSkalarPrf
 *  arguments     : 1) prf-node
 *		    2) result type
 *		    3) arg_info includes used mask to be updated and varno
 *		    R) result-node
 *  description   : Calculates some  prim-functions with one non constant arguments
 *  global vars   : N_float, ... , N_not, ..., prf_string
 *  internal funs : FreePrf2
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE
 *
 *  remarks       : if (NULL==arg_info) mask will not be updated
 *
 */
node *
NoConstSkalarPrf (node *arg_node, types *res_type, node *arg_info)
{
    DBUG_ENTER ("NoConstSkalarPrf");

    /*
     * Calculate prim-functions with non constant arguments
     */
    if (N_prf == arg_node->nodetype) {
        switch (arg_node->info.prf) {
        case F_and:
            arg_node
              = FoldExpr (arg_node, 0, 0, FALSE, arg_info); /* FALSE && x = FALSE */
            if (N_prf == arg_node->nodetype)
                arg_node
                  = FoldExpr (arg_node, 1, 1, FALSE, arg_info); /* x && FALSE = FALSE */
            if (N_prf == arg_node->nodetype)
                arg_node = FoldExpr (arg_node, 0, 1, TRUE, arg_info); /* x && TRUE = x */
            if (N_prf == arg_node->nodetype)
                arg_node = FoldExpr (arg_node, 1, 0, TRUE, arg_info); /* TRUE && x = x */
            break;
        case F_or:
            arg_node = FoldExpr (arg_node, 0, 0, TRUE, arg_info); /* TRUE || x = TRUE */
            if (N_prf == arg_node->nodetype)
                arg_node
                  = FoldExpr (arg_node, 1, 1, TRUE, arg_info); /* x || TRUE = TRUE */
            if (N_prf == arg_node->nodetype)
                arg_node
                  = FoldExpr (arg_node, 0, 1, FALSE, arg_info); /* x || FALSE = x */
            if (N_prf == arg_node->nodetype)
                arg_node
                  = FoldExpr (arg_node, 1, 0, FALSE, arg_info); /* FALSE || x = x */
            break;
        case F_mul:
        case F_mul_AxS:
        case F_mul_SxA:
            arg_node = FoldExpr (arg_node, 0, 0, 0, arg_info); /* 0 * x = 0 */
            if (N_prf == arg_node->nodetype)
                arg_node = FoldExpr (arg_node, 1, 1, 0, arg_info); /* x * 0 = 0 */
            if (N_prf == arg_node->nodetype)
                arg_node = FoldExpr (arg_node, 0, 1, 1, arg_info); /* 1 * x = x */
            if (N_prf == arg_node->nodetype)
                arg_node = FoldExpr (arg_node, 1, 0, 1, arg_info); /* x * 1 = x */
            break;
        case F_div:
        case F_div_SxA:
            if (TRUE == FoundZero (arg_node->node[0]->node[1]->node[0])) {
                WARN (arg_node->lineno, ("Division by zero expected"));
            }
            arg_node = FoldExpr (arg_node, 0, 0, 0, arg_info); /* 0 / x = 0 */
            if (N_prf == arg_node->nodetype)
                arg_node = FoldExpr (arg_node, 1, 0, 1, arg_info); /* x / 1 = x */
            break;
        case F_add:
        case F_add_AxS:
        case F_add_SxA:
            arg_node = FoldExpr (arg_node, 0, 1, 0, arg_info); /* 0 + x = x */
            if (N_prf == arg_node->nodetype)
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
 *  functionname  : ArrayPrf
 *  arguments     : 1) prf-node
 *		    2) result type
 *		    3) arg_info->mask[1] need for eliminating identifier
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
ArrayPrf (node *arg_node, types *res_type, node *arg_info)
{
    node *arg[MAXARG], *expr[MAXARG], *expr_arg[MAXARG], *tmp;
    int swap, i;
    long *used_sofar;

    DBUG_ENTER ("ArrayPrf");

    /*
     * Search Arguments for primitive Functions
     */
    tmp = arg_node->node[0];
    for (i = 0; i < MAXARG; i++) {
        if (tmp != NULL) {
            arg[i] = NodeBehindCast (tmp->node[0]);
            tmp = tmp->node[1];
        } else
            arg[i] = NULL;
    }

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

        if (N_id == arg[0]->nodetype) {
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

        if (N_id == arg[1]->nodetype) {
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
                if (N_id == old_arg[0]->nodetype) {
                    INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                    FreeTree (arg[0]);
                }
                break;
            }
        } else {
            arg[1] = Trav (arg[1], arg_info);
            if (!IsConst (arg[1])) {
                if (N_id == old_arg[0]->nodetype) {
                    INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                    FreeTree (arg[0]);
                }
                break;
            }
        }

        if (((F_div == arg_node->info.prf) || (F_div_SxA == arg_node->info.prf)
             || (F_div_AxS == arg_node->info.prf) || (F_div_AxA == arg_node->info.prf))
            && (TRUE == FoundZero (arg[1]))) {
            if (N_id == old_arg[0]->nodetype) {
                INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                FreeTree (arg[0]);
            }
            if (N_id == old_arg[1]->nodetype) {
                INC_VAR (arg_info->mask[1], old_arg[1]->info.ids->node->varno);
                FreeTree (arg[1]);
            }
            WARN (arg_node->lineno, ("Division by zero expected"));
            break;
        }

        /*
         * Swap arguments, to be sure that an array is first argument
         */
        if (N_array != arg[0]->nodetype) {
            tmp = arg[1];
            arg[1] = arg[0];
            arg[0] = tmp;
            swap = TRUE;
        } else {
            swap = FALSE;
        }

        if (N_array == arg[1]->nodetype) {
            /*
             * Calculate prim-function with two arrays
             */
            expr[0] = arg[0]->node[0];
            expr[1] = arg[1]->node[0];
            do {
                expr_arg[0] = expr[0]->node[0];
                expr_arg[1] = expr[1]->node[0];
                expr[0]->node[0]
                  = SkalarPrf (expr_arg, arg_node->info.prf, res_type, swap);
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
                expr[0]->node[0]
                  = SkalarPrf (expr_arg, arg_node->info.prf, res_type, swap);
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
        switch (arg[0]->nodetype) {
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
            arg[0]->node[0]->node[0]->nodetype = N_num;

            /*
             * Free rest of array and prf-node
             */
            if (NULL != arg[0]->node[0]->node[1])
                FreeTree (arg[0]->node[0]->node[1]);
            arg[0]->node[0]->nnode = 0;
            arg[0]->node[0]->node[1] = NULL;
            FREE (arg_node);

            /*
             * Gives Array the correct type
             */
            ARRAY_TYPE (arg[0]) = FreeOneTypes (ARRAY_TYPE (arg[0]));
            ARRAY_TYPE (arg[0]) = DuplicateTypes (res_type, 0);

            /*
             * Store result
             */
            arg_node = arg[0];
            cf_expr++;
            break;
        case N_id:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            SHAPE_2_ARRAY (tmp, arg[0]->info.ids->node->info.types, res_type);
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
    /* Fold dim-function   */
    /***********************/
    case F_dim:
        switch (arg[0]->nodetype) {
        case N_array:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * result is always 1
             */
            arg_node->nodetype = N_num;
            arg_node->info.cint = 1;

            /*
             * Free argument of prim function
             */
            FreeTree (arg_node->node[0]);
            arg_node->node[0] = NULL;
            arg_node->nnode = 0;

            cf_expr++;
            break;
        case N_id:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * get result
             */
            arg_node->nodetype = N_num;
            GET_DIM (arg_node->info.cint, arg[0]->info.ids->node->info.types);

            /*
             * Free argument of prim function
             */
            FreeTree (arg_node->node[0]);
            arg_node->node[0] = NULL;
            arg_node->nnode = 0;

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
    case F_psi:

        switch (NODE_TYPE (arg[1])) {
        case N_array: {
            int start;
            node *tmp;

            if ((0 <= NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (arg[0]))))
                && (NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (arg[0]))) < ArraySize (arg[1]))) {
                DBUG_PRINT ("CF", ("primitive function %s folded",
                                   prf_string[arg_node->info.prf]));
                cf_expr++;

                start = arg[0]->node[0]->node[0]->info.cint;
                tmp = DupTree (FetchNum (start, arg[1]), NULL);
                if (N_id == tmp->nodetype) {
                    DEC_VAR (arg_info->mask[1], tmp->info.ids->node->varno);
                }

                FreeTree (arg_node);
                arg_node = tmp;
            } else {
                WARN (arg_node->lineno, ("Illegal vector for primitive function psi"));
            }
        } break;
        case N_id: {
            int length, start, mult, i, j, arg_length;
            int vec_dim;
            int vec_shape[SHP_SEG_SIZE];
            node *res_node, *old_arg_0, *value, *tmp;

            old_arg_0 = arg[0];

            /*
             * Substitute shape-vector
             */
            if (N_id == arg[0]->nodetype) {
                MRD_GETDATA (value, arg[0]->info.ids->node->varno, INFO_VARNO);
                if (IsConst (value)) {
                    DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
                    arg[0] = DupTree (value, NULL);
                    used_sofar = arg_info->mask[1];
                    arg_info->mask[1] = GenMask (INFO_VARNO);
                    arg[0] = Trav (arg[0], arg_info);
                    FREE (arg_info->mask[1]);
                    arg_info->mask[1] = used_sofar;
                } else
                    break;
            } else {
                arg[0] = Trav (arg[0], arg_info);
            }

            /*
             * Substitution of shape-vector successful ?
             */
            MRD_GETDATA (tmp, arg[1]->info.ids->node->varno, INFO_VARNO);
            if ((NULL == tmp) || (N_array != tmp->nodetype)) {
                if (N_id == old_arg_0->nodetype) {
                    INC_VAR (arg_info->mask[1], old_arg_0->info.ids->node->varno);
                    FreeTree (arg[0]);
                }
                break;
            }

            /* Calculate dimension and shape vector of first argument */
            vec_dim = GetShapeVector (arg[0], vec_shape);

            /* Calculate length of result array */
            length = 1;
            for (i = 0; i < arg_info->info.types->dim; i++)
                length *= arg_info->info.types->shpseg->shp[i];

            /* Calculate startposition of result array in argument array */
            start = 0;
            for (i = 0; i < vec_dim; i++) {
                if (vec_shape[i] >= 0) {
                    mult = 1;
                    for (j = i + 1; j < arg[1]->info.ids->node->info.types->dim; j++)
                        mult *= arg[1]->info.ids->node->info.types->shpseg->shp[j];
                    start += vec_shape[i] * mult;
                } else {
                    start = -1;
                    i = vec_dim;
                }
            }

            arg_length = 1;
            for (i = 0; i < arg[1]->info.ids->node->info.types->dim; i++)
                arg_length *= arg[1]->info.ids->node->info.types->shpseg->shp[i];

            if ((start + length <= arg_length) && (start >= 0)) {
                DEC_VAR (arg_info->mask[1], arg[1]->info.ids->node->varno);
                if (vec_dim == arg[1]->info.ids->node->info.types->dim) {
                    MRD_GETDATA (tmp, arg[1]->info.ids->node->varno, INFO_VARNO);
                    res_node = FetchNum (start, tmp);
                    if (N_id == res_node->nodetype)
                        DEC_VAR (arg_info->mask[1], res_node->info.ids->node->varno);
                } else {
                    res_node = MakeNode (N_array);
                    res_node->nnode = 1;
                    MRD_GETDATA (tmp, arg[1]->info.ids->node->varno, INFO_VARNO);
                    res_node->node[0] = DupPartialArray (start, length, tmp, arg_info);
                }
                DBUG_PRINT ("CF", ("primitive function %s folded in line %d",
                                   prf_string[arg_node->info.prf], NODE_LINE (arg_info)));
                FreeTree (arg_node);
                arg_node = res_node;
                cf_expr++;
            } else {
                if (old_arg_0 != arg[0])
                    FreeTree (arg[0]);
                WARN (arg_node->lineno, ("Illegal vector for primitive function psi"));
            }
        } break;
        default:
            break;
        }
        break;
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
 *  internal funs : SkalarPrf, ArrayPrf
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

    if (PRF_PRF (arg_node) <= F_neq) {
        /*
         * substitute all arguments
         */
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);

        /*
         * Search Arguments for primitive Functions
         */
        tmp = PRF_ARGS (arg_node);
        for (i = 0; i < MAXARG; i++) {
            if (tmp != NULL) {
                arg[i] = EXPRS_EXPR (tmp);
                tmp = EXPRS_NEXT (tmp);
            } else
                arg[i] = NULL;
        }

        /*
         * Calculate non array primitive functions
         */
        if ((IsConst (arg[0])) && ((F_not == PRF_PRF (arg_node)) || (IsConst (arg[1])))) {
            if (!((F_div == PRF_PRF (arg_node)) && (TRUE == FoundZero (arg[1])))) {
                arg[0] = SkalarPrf (arg, PRF_PRF (arg_node), INFO_TYPE, FALSE);
                FreePrf2 (arg_node, 0);
                arg_node = arg[0];
            }
        }
    } else {
        /*
         * substitute all arguments
         */
        switch (PRF_PRF (arg_node)) {
        case F_shape:
        case F_dim:
            break;
        default:
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
            break;
        }

        /*
         * Calculate primitive functions with arrays
         */
        arg_node = ArrayPrf (arg_node, arg_info->info.types, arg_info);
    }

    /*
     * Do some foldings like 1 * x = x, etc.
     */
    arg_node = NoConstSkalarPrf (arg_node, arg_info->info.types, arg_info);

    DBUG_RETURN (arg_node);
}
