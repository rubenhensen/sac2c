/*
 *
 * $Log$
 * Revision 1.57  1995/07/13 16:26:13  hw
 * - changed compilation of N_foldprf & N_foldfun
 * - changed secound argument of GET_LENGTH
 * - moved compilation of a with-loop-N_return to new function
 *   CompWithReturn
 * - changed CompFundec & CompArg to compile function declarations
 *   without formal paramter names( only imported functions)
 *
 * Revision 1.56  1995/07/04  09:24:06  hw
 * compilation of primitive functions itod, ftod, dtoi & dtof inserted
 *
 * Revision 1.55  1995/06/30  12:14:14  hw
 * - renamed macro GET_BASIC_TYPE to GET_BASIC_SIMPLETYPE
 * - changed function CompVardec
 * - compilation of primitive functions itof & ftoi inserted
 *
 * Revision 1.54  1995/06/26  14:23:47  hw
 * - deleted some unused variables
 * - remove #if 0 .. #endif parts
 *
 * Revision 1.53  1995/06/26  14:05:51  hw
 * moved some macros to compile.h and tree.h
 *
 * Revision 1.52  1995/06/26  12:01:37  hw
 * compilation of idx_psi added
 *
 * Revision 1.51  1995/06/26  09:27:07  sbs
 * insertion of idx_psi initiated :->
 *
 * Revision 1.50  1995/06/23  13:13:53  hw
 * - functions will be renamed now, so overloading of userdefined
 *   functions can be done.
 *   ( new function RenameFunName inserted)
 * -  added argument to call of 'DuplicateTypes'
 *
 * Revision 1.49  1995/06/16  15:29:11  hw
 * bug fixed in CompVardec( rmoveing of vardec-nodes)
 *
 * Revision 1.48  1995/06/14  15:32:25  hw
 * changed Compile( "extern"-declaration for each not imported function inserted)
 *
 * Revision 1.47  1995/06/14  14:24:48  hw
 * bug fixed in CompPrf ( new variable declaration will be inserted
 *  now in all cases )
 *
 * Revision 1.46  1995/06/14  12:41:55  hw
 * - N_cast nodes in argument position of primitive functions
 *   will be deleted
 * - bug fixed in creation of ND_KD_PSI_VxA_S & ND_KD_PSI_CxA_S
 * - renamed macro CONST_ARRAY to CREATE_TMP_CONST_ARRAY and
 *   changed it (removed creation of ND_REUSE)
 *
 * Revision 1.45  1995/06/13  15:11:08  hw
 * - changed RenameVar (added new parameter)
 * - added function RenameReturn
 * inserted renameing of varaibles of a return_statement  and things
 * beloning to it
 * - cast in a return_statement will be deleted
 *
 * Revision 1.44  1995/06/12  09:19:17  hw
 * bug fixed in CompVardec ( arg_node->nnode is set correctly)
 *
 * Revision 1.43  1995/06/09  17:35:53  hw
 * -bug fixed in CompLoop (look whether next there is a assignment ater loop)
 *
 * Revision 1.42  1995/06/09  15:30:42  hw
 * changed N_icms ND_KD_PSI_... (linenumber inserted)
 *
 * Revision 1.41  1995/06/08  17:48:57  hw
 * CompTypedef inserted
 *
 * Revision 1.40  1995/06/07  13:33:32  hw
 * exchanges N_icm ND_CREATE_CONST_ARRAY with ND_CREATE_CONST_ARRAY_S
 * N_icm ND_CREATE_CONST_ARRAY_A (array out of arrays) inserted
 *
 * Revision 1.39  1995/06/06  15:54:30  hw
 * changed CompVardec ( a declaration of an array with unknown shape
 *   will be removed )
 *
 * Revision 1.38  1995/06/06  11:39:02  hw
 * changed CompPrf (F_take/F_drop) (first argument can be a scalar too, now)
 *
 * Revision 1.37  1995/06/02  08:43:34  hw
 * - renamed N_icm ND_END_FOLDPRF to ND_END_FOLD
 * - compilation of N_foldfun inserted
 *
 * Revision 1.36  1995/05/24  15:28:37  hw
 * bug fixed in creation of N_icm ND_END_MODARRAY_S\A ( last arg is
 *  a N_exprs node now ;-)) )
 *
 * Revision 1.35  1995/05/24  13:45:24  hw
 * - changed args of N_icm FUN_DEC ( back to rev 1.33 )
 *   and added N_icm ND_KS_DECL_ARRAY_ARG instead ( this N_icm will be put
 *    at beginning of the block of a function)
 *    ( changes are done in CompArg only)
 * - added compilation of "fold_prf", but it does not work correctly :-((
 *   there has to be done some work on it
 *
 * Revision 1.34  1995/05/22  16:51:27  hw
 * - deleted some Check_reuse N_icms
 * - changed args of FUN_DEC N_icm
 *
 * Revision 1.33  1995/05/22  10:10:46  hw
 * - added function "CompCast" to delete N_cast nodes
 *  - remove N_cast nodes while compilation in functions ( CompReturn,
 *    CompPrf, CompAp )
 *
 * Revision 1.32  1995/05/19  13:33:30  hw
 * - bug fixed in CompPrf ( added ND_DEC_... while compilation of F_psi (case:
 *     ND_KD_PSI_VxA_S))
 * - bug fixed in CompAp (refcounts of arrays which are returned from a function
 *     will be set and inserted correctly)
 * - changed CompReturn ( if return contains to a with_loop, following refcounts
 *    will be decremented: -index_vector, left and right border,
 *                           array of modarray-part, all used arrays
 * - bug fixed in CompLoop ( - create only ND_DEC_RC_FREE macros
 *                           - put ND_LABEL(..) at the right place (after DEC's
 *                              but before INC's )
 *
 * Revision 1.31  1995/05/11  08:25:16  hw
 * changed setting of refcount at beginning of userdefined functions
 *
 * Revision 1.30  1995/05/10  13:54:38  hw
 * changed increasing of refcounts of used variables in loops
 *
 * Revision 1.29  1995/05/09  16:39:35  hw
 * bug fixed in CompAssign ( arg_info->node[0] will be set correctly now )
 *
 * Revision 1.28  1995/05/08  15:45:53  hw
 * added CompBlock
 * compilation (renaming) of expressions like `a=a+a` added
 *
 * Revision 1.27  1995/05/04  14:59:46  hw
 * -changed compilation of 'dim' & 'shape' (refcount will be decremented now)
 * - changed parameters of N_icm ND_REUSE
 * - changed setting of refcount behind a loop
 * - changed setting of refcount at beginning of a function
 *
 * Revision 1.26  1995/05/03  13:20:52  hw
 * - deleted ND_KD_ROT_SxSxA_A
 * - bug fixed in compilation of primitive function 'rotate'
 *   ( added N_icm ND_ALLOC_ARRAY for result of 'rotate' )
 * - refcounts of results of userdefined functions will be increased
 *
 * Revision 1.25  1995/05/02  10:11:52  hw
 *  bug fixed in CompReturn ( ND_END_MODARRAY and  ND_END_GENARRAY have correct
 *   parameters now)
 *
 * Revision 1.24  1995/05/02  09:32:58  hw
 * bug fixed in CompReturn
 *
 * Revision 1.23  1995/04/28  17:29:30  hw
 * - changed compilation of with_loop:
 *   - N_icm for allocation of index_vector added
 *   - refcounts of  used before defined variables (only arrays) of a
 *     with-loop will be incremented
 *   - refcount of index_vector will be decremented after with_loop
 *
 * Revision 1.22  1995/04/28  09:02:38  hw
 * bug fixed in creation of N_icm ND_BEGIN_MODARRAY ( now 3. argument is set
 *  correctly)
 *
 * Revision 1.21  1995/04/27  14:44:10  hw
 * bug fixed in CompWith (renamed N_icm for genarray)
 *
 * Revision 1.20  1995/04/27  14:24:48  hw
 * added compilation of with-loop (modarray and genarray)
 *
 * Revision 1.19  1995/04/24  18:08:06  hw
 * - renamed CompWhile to CompLoop
 * -compilation of N_do done
 * - refcount after KS_ASSIGN_ARRAY is set corectly
 *
 * Revision 1.18  1995/04/24  14:18:53  hw
 * - CompWhile & CompCond inserted
 * - changed refcount-parameter of ND_ALLOC_ARRAY if it is used with
 *   ND_CHECK_REUSE
 *
 * Revision 1.17  1995/04/19  14:30:32  hw
 * bug fixed in compilation of 'rotate'
 *
 * Revision 1.16  1995/04/19  14:13:18  hw
 * changed compilation of primitive function 'rotate'
 *  ( 3. argument of `rotate' will not be reused )
 *
 * Revision 1.15  1995/04/19  13:50:47  hw
 * changed arguments of N_icm ND_KD_PSI_CxA_S &  ND_KD_PSI_VxA_S
 *
 * Revision 1.14  1995/04/19  13:08:28  hw
 * bug fixed in compilation of primitive function 'cat'
 *
 * Revision 1.13  1995/04/19  12:51:59  hw
 * changed parameters of N_icm ND_ALLOC_ARRAY & ND_CREATE_CONST_ARRAY
 *
 * Revision 1.12  1995/04/13  17:21:28  hw
 * compilation of primitive function 'cat' completed
 * compilation of primitive function 'rotate' inserted
 *
 * Revision 1.11  1995/04/13  10:04:09  hw
 * compilation of primitive function 'cat' inserted, but not completly
 *
 * Revision 1.10  1995/04/11  15:11:55  hw
 * compilation of N_fundef, N_ap, N_return done
 *
 * Revision 1.9  1995/04/07  11:30:18  hw
 * changed N_icm name ND_KS_ASSIGN -> ND_KS_ASSIGN_ARRAY
 *
 * Revision 1.8  1995/04/07  10:01:29  hw
 * - added one argument to N_icm of primitive functions take, drop and psi
 * - changed compilation of primitive function reshape
 *
 * Revision 1.7  1995/04/06  08:48:58  hw
 * compilation of F_dim and F_shape added
 *
 * Revision 1.6  1995/04/05  15:26:33  hw
 * F_..AxA_A will be compiled now
 *
 * Revision 1.5  1995/04/04  08:31:30  hw
 * changed CompFundef
 *
 * Revision 1.4  1995/04/03  16:35:29  hw
 * - CompFundef inserted
 * - changed CompArg (ICM for increasing refcounts inserted)
 * - F_psi will work on constant arrays
 *
 * Revision 1.3  1995/04/03  13:27:29  hw
 * bug fixed in creation of N_icm ND_CREATE_CONST_ARRAY.
 * primitive functions F_...SxA, F_..AxS, F_take can have constant
 *    arrays as argument
 *
 * Revision 1.2  1995/03/31  15:49:16  hw
 * added macros to create and insert 'N_icm'
 * added CompArray, CompId, CompReturn
 * changed CompPrf (now more functions will be compiled)
 *
 * Revision 1.1  1995/03/29  12:38:10  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <string.h>

#include "print.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "access_macros.h"
#include "Error.h"
#include "traverse.h"
#include "compile.h"
#include "convert.h"
#include "refcount.h"  /* to use IsArray */
#include "typecheck.h" /* to use LookupType */
#include "free.h"

extern int malloc_verify ();
extern int malloc_debug (int level);
extern char filename[]; /* imported from main.c */

#define DUMMY_NAME "__OUT_"
#define LABEL_NAME "__Label" /* basic-name for goto label */
#define FUN_NAME_LENGTH 256

#if 0
/* following macros are only temporay (they are copied from typecheck.c) */
#define MOD(a) (NULL == a) ? "" : a
#define MOD_CON(a) (NULL == a) ? "" : MOD_NAME_CON
#define MOD_NAME(a) MOD (a), MOD_CON (a)
#endif

#ifndef DBUG_OFF

#define MDB_VAR(var) var
#define MDB_DECLARE(type, var) type var
#define MDB_ASSIGN(var, value) var = value

#else

#define MDB_VAR(var)
#define MDB_DECLARE(type, var)
#define MDB_ASSIGN(var, value)

#endif /* DBUG_OFF */

/* the following macros are while generation of N_icms */

#define SET_VARS_FOR_MORE_ICMS                                                           \
    first_assign = LAST_ASSIGN (arg_info);                                               \
    old_arg_node = arg_node;                                                             \
    last_assign = NEXT_ASSIGN (arg_info);                                                \
    arg_node = arg_info->node[1]->node[0]

#define INSERT_ASSIGN                                                                    \
    if (NULL != last_assign) {                                                           \
        DBUG_PRINT ("COMP", ("last :" P_FORMAT "(%s " P_FORMAT ")", last_assign,         \
                             mdb_nodetype[last_assign->node[0]->nodetype],               \
                             last_assign->node[0]));                                     \
        DBUG_PRINT ("COMP", ("first: " P_FORMAT "(%s " P_FORMAT ")", first_assign,       \
                             mdb_nodetype[first_assign->node[0]->nodetype],              \
                             first_assign->node[0]));                                    \
        APPEND_ASSIGNS (first_assign, last_assign);                                      \
    }

#define CHECK_REUSE(test)                                                                \
    BIN_ICM_REUSE (arg_info->node[1], "ND_CHECK_REUSE", test, res);                      \
    SET_VARS_FOR_MORE_ICMS

#define CHECK_REUSE__ALLOC_ARRAY_ND(test, rc)                                            \
    if (1 >= test->refcnt) { /* create ND_CHECK_REUSE  */                                \
        node *num;                                                                       \
        BIN_ICM_REUSE (arg_info->node[1], "ND_CHECK_REUSE", test, res);                  \
        first_assign = LAST_ASSIGN (arg_info);                                           \
        DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                          \
        old_arg_node = arg_node;                                                         \
        arg_node = arg_info->node[1]->node[0];                                           \
        /* create ND_ALLOC_ARRAY */                                                      \
        MAKENODE_NUM (num, 0);                                                           \
        CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res, num);        \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
    } else { /* create ND_ALLOC_ARRAY */                                                 \
        node *num;                                                                       \
        MAKENODE_NUM (num, 0);                                                           \
        BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);          \
        MAKE_NEXT_ICM_ARG (icm_arg, num);                                                \
        first_assign = LAST_ASSIGN (arg_info);                                           \
        DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                          \
        old_arg_node = arg_node;                                                         \
        arg_node = arg_info->node[1]->node[0];                                           \
    }

#define DEC_OR_FREE_RC_ND(array, num_node)                                               \
    if (1 < array->refcnt) { /* create ND_DEC_RC */                                      \
        CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", array, num_node);                    \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
    } else {                                                                             \
        DEC_RC_FREE_ND (array, num_node);                                                \
    }

#define DEC_RC_ND(array, num_node)                                                       \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define DEC_RC_FREE_ND(array, num_node) /* create ND_DEC_RC_FREE */                      \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE", array, num_node);                   \
    APPEND_ASSIGNS (first_assign, next_assign)

#define INC_RC_ND(array, num_node) /* create ND_INC_RC */                                \
    CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define CREATE_TMP_CONST_ARRAY(array, rc)                                                \
    array_is_const = 1;                                                                  \
    old_arg_node = arg_node;                                                             \
    tmp = array->node[0];                                                                \
    n_elems = 0;                                                                         \
    do {                                                                                 \
        n_elems += 1;                                                                    \
        tmp = tmp->node[1];                                                              \
    } while (NULL != tmp);                                                               \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    MAKENODE_NUM (n_node1, 1);                                                           \
    MAKENODE_ID (tmp_array1, "__TMP");                                                   \
    arg_info->node[1]->nodetype = N_block; /* reuse previous N_let*/                     \
    CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node, tmp_array1,        \
                      n_node1, n_node);                                                  \
    arg_node = first_assign;                                                             \
    CREATE_CONST_ARRAY (array, tmp_array1, type_id_node, rc);                            \
    array = tmp_array1 /* set array to __TMP */

#define CREATE_CONST_ARRAY(array, name, type, rc)                                        \
    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type, name, rc);                    \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    COUNT_ELEMS (n_elems, array->node[0]);                                               \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", name, n_node);             \
    icm_arg->node[1] = array->node[0];                                                   \
    icm_arg->nnode = 2;                                                                  \
    APPEND_ASSIGNS (first_assign, next_assign)

#define DECL_ARRAY(assign, Node, var_str, var_str_node)                                  \
    COUNT_ELEMS (n_elems, Node);                                                         \
    MAKENODE_NUM (n_elems_node, n_elems);                                                \
    MAKENODE_NUM (n_node1, 1);                                                           \
    MAKENODE_ID (var_str_node, var_str);                                                 \
    CREATE_4_ARY_ICM (assign, "ND_KS_DECL_ARRAY", type_id_node, var_str_node, n_node1,   \
                      n_elems_node)

#define INSERT_ID_NODE(no, last, str)                                                    \
    tmp = MakeNode (N_exprs);                                                            \
    MAKENODE_ID (tmp->node[0], str);                                                     \
    tmp->node[1] = no;                                                                   \
    tmp->nnode = 2;                                                                      \
    last->node[1] = tmp;                                                                 \
    last->nnode = 2

/* following macros are used to compute last but one or next N_assign form
 * a 'arg_info' node
 */
#define NEXT_ASSIGN(arg_info)                                                            \
    (N_block == arg_info->node[0]->nodetype) ? arg_info->node[0]->node[0]->node[1]       \
                                             : arg_info->node[0]->node[1]->node[1]

#define LAST_ASSIGN(arg_info)                                                            \
    (N_block == arg_info->node[0]->nodetype) ? arg_info->node[0]->node[0]                \
                                             : arg_info->node[0]->node[1]

#define INSERT_BEFORE(arg_info, Node)                                                    \
    if (N_block == arg_info->node[0]->nodetype) {                                        \
        DBUG_PRINT ("COMP",                                                              \
                    ("insert node:" P_FORMAT " before:" P_FORMAT " after:" P_FORMAT,     \
                     Node, arg_info->node[0]->node[0], arg_info->node[0]));              \
        arg_info->node[0]->node[0] = Node;                                               \
    } else {                                                                             \
        DBUG_PRINT ("COMP",                                                              \
                    ("insert node:" P_FORMAT " before:" P_FORMAT " after:" P_FORMAT,     \
                     Node, arg_info->node[0]->node[1], arg_info->node[0]));              \
        arg_info->node[0]->node[1] = Node;                                               \
    }

#define ELIMINATE_CAST(exprs)                                                            \
    if (N_cast == exprs->node[0]->nodetype) {                                            \
        tmp = exprs->node[0]->node[0];                                                   \
        FREE_TYPE (exprs->node[0]->TYPES);                                               \
        FREE (exprs->node[0]);                                                           \
        exprs->node[0] = tmp;                                                            \
    }

#define GOTO_LAST_N_EXPRS(exprs, last_exprs)                                             \
    last_exprs = exprs;                                                                  \
    while (2 == last_exprs->nnode) {                                                     \
        DBUG_ASSERT (N_exprs == last_exprs->nodetype, " nodetype  != N_expr");           \
        last_exprs = last_exprs->node[1];                                                \
    }

#define GOTO_LAST_BUT_LEAST_N_EXPRS(exprs, lbl_exprs)                                    \
    {                                                                                    \
        node *tmp;                                                                       \
        lbl_exprs = exprs;                                                               \
        DBUG_ASSERT (N_exprs == lbl_exprs->nodetype, " nodetype  != N_expr");            \
        DBUG_ASSERT (2 == lbl_exprs->nnode, "there is NO N_exprs-chain contains only"    \
                                            " one element");                             \
        tmp = lbl_exprs->node[1];                                                        \
        while (tmp->nnode == 2) {                                                        \
            DBUG_ASSERT (N_exprs == tmp->nodetype, " nodetype  != N_expr");              \
            lbl_exprs = tmp;                                                             \
            tmp = tmp->node[1];                                                          \
        }                                                                                \
    }

#define FREE(a)                                                                          \
    {                                                                                    \
        DBUG_PRINT ("FREE", (P_FORMAT, a));                                              \
        free (a);                                                                        \
    }

#define FREE_TYPE(a)                                                                     \
    if (NULL != a->shpseg)                                                               \
        FREE (a->shpseg)                                                                 \
    FREE (a)

#define FREE_VARDEC(a)                                                                   \
    FREE_TYPE (a->TYPES)                                                                 \
    FREE (a)

#define FREE_TREE(a) FreeTree (a)

#define FREE_IDS(a)                                                                      \
    {                                                                                    \
        DBUG_PRINT ("FREE", (P_FORMAT, a));                                              \
        FreeIds (a);                                                                     \
    }

#if 0
#define FREE(a)
#define FREE_TYPE(a)
#define FREE_VARDEC(a)
#define FREE_TREE(a)
#define FREE_IDS(a)
#endif

static int label_nr = 0;

/*
 *
 *  functionname  : RenameVar
 *  arguments     : 1) name of variable
 *  description   : puts '__' in fornt of 1)
 *  global vars   :
 *  internal funs :
 *  external funs : Malloc, strlen, sprintf, sizeof
 *  macros        : DBUG...,
 *
 *  remarks       :
 *
 */
char *
RenameVar (char *string, int i)
{
    char *new_name;
    DBUG_ENTER ("RenameVar");
    if (0 == i) {
        new_name = (char *)Malloc (sizeof (char) * (strlen (string) + 3));
        sprintf (new_name, "__%s", string);
    } else {
        new_name = (char *)Malloc (sizeof (char) * (strlen (string) + 5));
        sprintf (new_name, "__%s_%d", string, i);
    }

    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : RenameFunName
 *  arguments     : 1) N_fundef node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : Malloc, strlen, sprintf, sizeof, Type2String
 *  macros        : DBUG..., NULL, ID_MOD, ID, FUN_NAME_LENGTH,
 *                  MDB_DECLARE, MDB_ASSIGN, FREE
 *
 *  remarks       : imported "non sac" functions and functions without
 *                  arguments and modul-name will NOT be renamed
 *
 */
node *
RenameFunName (node *fun_node)
{
    node *tmp_fun_node, *args;
    char *arg_type, *new_name;
    MDB_DECLARE (int, length);

    DBUG_ENTER ("RenameFunName");
    tmp_fun_node = fun_node;
    do {
        if (((NULL != tmp_fun_node->node[0])
             || ((NULL != tmp_fun_node->ID_MOD) && (NULL == tmp_fun_node->node[0])))
            && ((NULL != tmp_fun_node->node[2]) || (NULL != tmp_fun_node->ID_MOD))) {
            args = tmp_fun_node->node[2];
            new_name = (char *)Malloc (sizeof (char) * FUN_NAME_LENGTH);
            sprintf (new_name, "%s%s%s_", MOD_NAME (tmp_fun_node->ID_MOD),
                     tmp_fun_node->ID);
            MDB_ASSIGN (length, strlen (new_name));
            while (NULL != args) {
                arg_type = Type2String (args->TYPES, 2);
                MDB_ASSIGN (length, length + strlen (arg_type));
                DBUG_ASSERT (length < FUN_NAME_LENGTH, "new fun_name is to long");
                strcat (new_name, arg_type);
                args = args->node[0];
            }
            FREE (tmp_fun_node->ID);
            /* don't free tmp_fun_node->ID_MOD, because it is shared !! */
            tmp_fun_node->ID = new_name;
            tmp_fun_node->ID_MOD = NULL;
        }
        tmp_fun_node = tmp_fun_node->node[1];
    } while (NULL != tmp_fun_node);

    DBUG_RETURN (fun_node);
}

/*
 *
 *  functionname  : RenameReturn
 *  arguments     : 1) N_return node
 *                  2) arg_info
 *  description   : - renames the variables in a return_sttatement,
 *                  - adds variable declaration
 *                  - inserts new assignments (after LAST_ASSIGN(arg_info))
 *  global vars   :
 *  internal funs : RenameVar,
 *  external funs : MakeNode, StringCopy, strcmp, DuplicateTypes,
 *  macros        : DBUG..., ELIMINATE_CAST, IDS_ID, ISD_NODE, ID, TYPES,
 *                  FREE, MAKENODE_ID, LAST_ASSIGN, NULL
 *
 *  remarks       : - pointer to variable declaration is stored in
 *                    arg_info->node[3]
 *                  - returns N_let of last inserted new assign if
 *                     there renameing had to be done
 *                    returns N_return if no renaming had to be done
 *                  - puts new assignments after LAST_ASSIGN(arg_info).
 *                    node[0] of LAST_ASSIGN(arg_info) will be set in
 *                    last CompAssign (return value of CompReturn)) again
 *
 */
node *
RenameReturn (node *return_node, node *arg_info)
{
    node *exprs, *tmp_exprs, *assign, *let, *next_assign, *new_vardec, *vardec, *tmp;
    int i;
    char *old_id, *new_id;

    DBUG_ENTER ("RenameReturn");

    exprs = return_node->node[0];
    next_assign = MakeNode (N_assign);
    next_assign->node[0] = return_node;
    ;
    next_assign->nnode = 1;
    vardec = arg_info->node[3];

    while (NULL != exprs) {
        tmp_exprs = exprs->node[1];
        i = 1;
        ELIMINATE_CAST (exprs);
        old_id = exprs->node[0]->IDS_ID;
        while (NULL != tmp_exprs) {
            ELIMINATE_CAST (tmp_exprs);
            if (0 == strcmp (tmp_exprs->node[0]->IDS_ID, old_id)) {
                /* generates new nodes */
                assign = MakeNode (N_assign);
                let = MakeNode (N_let);
                assign->node[0] = let;
                assign->node[1] = next_assign;
                assign->nnode = 2;
                next_assign = assign;
                let->IDS = MakeIds (exprs->node[0]->IDS_ID);
                let->IDS_NODE = exprs->node[0]->IDS_NODE;
                MAKENODE_ID (let->node[0], RenameVar (old_id, i));
                let->nnode = 1;
                new_id = let->node[0]->IDS_ID;
                new_vardec = MakeNode (N_vardec);
                new_vardec->TYPES
                  = DuplicateTypes (tmp_exprs->node[0]->IDS_NODE->TYPES, 0);
                new_vardec->ID = StringCopy (new_id); /* set new variable name */
                let->node[0]->IDS_NODE = new_vardec;  /* set pointer to vardec */

                /* rename current ret_value */
                FREE (tmp_exprs->node[0]->IDS_ID);
                tmp_exprs->node[0]->IDS_ID = StringCopy (new_id);
                /* insert vardec */
                if (NULL == vardec)
                    vardec = new_vardec;
                else {
                    if (1 == vardec->nnode) {
                        new_vardec->node[0] = vardec->node[0];
                        new_vardec->nnode = 1;
                        vardec = new_vardec;
                    } else {
                        vardec->node[0] = new_vardec;
                        vardec->nnode = 1;
                    }
                }
            }
            tmp_exprs = tmp_exprs->node[1];
            i += 1;
        }
        exprs = exprs->node[1];
    }
    if (next_assign->node[0] != return_node) {
        /* new nodes have been inserted */
        node *last_assign = LAST_ASSIGN (arg_info);
        last_assign->node[0] = assign->node[0];
        last_assign->node[1] = assign->node[1];
        last_assign->nnode = 2;
        FREE (assign);
        arg_info->node[3] = vardec;

        return_node = assign->node[0];
    }

    DBUG_RETURN (return_node);
}

/*
 *
 *  functionname  : GenName
 *  arguments     : 1) number of variable
 *                  2) name
 *  description   : allocates memory for a new string , copies name to this
 *                  string and appends a number 1) to it
 *  global vars   :
 *  internal funs :
 *  external funs : strlen, sizeof, Malloc, sprintf
 *  macros        : DBUG..., NULL
 *  remarks       : 1) <= 99
 *
 */
char *
GenName (int i, char *name)
{
    char *new_name;

    DBUG_ENTER ("GenName");

    new_name = (char *)Malloc (sizeof (char) * (strlen (name) + 3));
    sprintf (new_name, "%s%d", name, i);

    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : GenFunName
 *  arguments     : 1) fun_name
 *                  2) mod_name
 *  description   : allocates memory for a new string and makes a new function
 *                  name out of 1), MOD_NAME_CON and 2)
 *
 *  global vars   :
 *  internal funs :
 *  external funs : strlen, sizeof, Malloc
 *  macros        : DBUG..., NULL, MOD_NAME_CON
 *  remarks       : ----
 *
 */
char *
GenFunName (char *fun_name, char *mod_name)
{
    int str_length;
    char *new_name;

    DBUG_ENTER ("GenFunName");

    if (NULL == mod_name)
        new_name = fun_name;
    else {
        str_length = strlen (fun_name) + strlen (mod_name);
        new_name = (char *)Malloc (sizeof (char) * (str_length + 3));
        strcat (new_name, mod_name);
        strcat (new_name, MOD_NAME_CON);
        strcat (new_name, fun_name);
    }
    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : ShapeToArray
 *  arguments     : 1) N_vardec node
 *  description   : computes the shape of corresponding type and stores it
 *                  as N_exprs - chain
 *
 *  global vars   :
 *  internal funs :
 *  external funs : LookupType, MakeNode
 *  macros        : DBUG..., NULL, SIMPLETYPE, DIM, SHP, MAKENODE_NUM, NAME,
 *                  NAME_MOD
 *  remarks       : ----
 *
 */
node *
ShapeToArray (node *vardec_node)
{
    node *ret_node = NULL, *tmp, *basic_type_node;
    int i;

    DBUG_ENTER ("ShapeToArray");
    if (T_user != vardec_node->SIMPLETYPE) {
        ret_node = MakeNode (N_exprs);
        MAKENODE_NUM (ret_node->node[0], vardec_node->SHP[0]);
        tmp = ret_node;
        for (i = 1; i < vardec_node->DIM; i++) {
            tmp->nnode = 2;
            tmp->node[1] = MakeNode (N_exprs);
            MAKENODE_NUM (tmp->node[1]->node[0], vardec_node->SHP[i]);
            tmp->node[1]->nnode = 1;
            tmp = tmp->node[1];
        }
    } else {
        basic_type_node = LookupType (vardec_node->NAME, vardec_node->NAME_MOD,
                                      042); /* 042 is dummy argument */
        if (1 <= vardec_node->DIM) {
            ret_node = MakeNode (N_exprs);
            MAKENODE_NUM (ret_node->node[0], vardec_node->SHP[0]);
            tmp = ret_node;
            for (i = 1; i < vardec_node->DIM; i++) {
                tmp->nnode = 2;
                tmp->node[1] = MakeNode (N_exprs);
                MAKENODE_NUM (tmp->node[1]->node[0], vardec_node->SHP[i]);
                tmp->node[1]->nnode = 1;
                tmp = tmp->node[1];
            }
            for (i = 0; i < basic_type_node->DIM; i++) {
                tmp->nnode = 2;
                tmp->node[1] = MakeNode (N_exprs);
                MAKENODE_NUM (tmp->node[1]->node[0], basic_type_node->SHP[i]);
                tmp->node[1]->nnode = 1;
                tmp = tmp->node[1];
            }
        } else {
            ret_node = MakeNode (N_exprs);
            MAKENODE_NUM (ret_node->node[0], basic_type_node->SHP[0]);
            tmp = ret_node;
            for (i = 1; i < basic_type_node->DIM; i++) {
                tmp->nnode = 2;
                tmp->node[1] = MakeNode (N_exprs);
                MAKENODE_NUM (tmp->node[1]->node[0], basic_type_node->SHP[i]);
                tmp->node[1]->nnode = 1;
                tmp = tmp->node[1];
            }
        }
    }

    DBUG_RETURN (ret_node);
}

/*
 *
 *  functionname  : Compile
 *  arguments     : 1) syntax tree
 *  description   : starts compilation  and initializes act_tab
 *
 *  global vars   : act_tab
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,
 *  remarks       : ----
 *
 */
node *
Compile (node *arg_node)
{
    node *info, *fundef, *tmp_fundef, *new_fundef;
    DBUG_ENTER ("Compile");

#ifdef MALLOC_TOOL
    malloc_debug (2);
#endif /* MALLOC_TOOL */

    act_tab = comp_tab; /* set new function-table for traverse */
    info = MakeNode (N_info);
    if (N_modul == arg_node->nodetype) {
        if (NULL != arg_node->node[2]) {
            /* first rename function names */
            arg_node->node[2] = RenameFunName (arg_node->node[2]);
            /* traverse functions */
            arg_node->node[2] = Trav (arg_node->node[2], info);
        }
        if (NULL != arg_node->node[1])
            /* traverse typedefs */
            arg_node->node[1] = Trav (arg_node->node[1], info);
    } else {
        DBUG_ASSERT ((N_fundef == arg_node->nodetype), "wrong node");
        arg_node = Trav (arg_node, info);
    }
    /* now create a extern declaration for each non imported function, except for
     * main
     */
    if (N_modul == arg_node->nodetype)
        fundef = arg_node->node[2];
    else
        fundef = arg_node->node[1];
    tmp_fundef = fundef;
    /* first skip imported functions */
    while (NULL == tmp_fundef->node[0])
        tmp_fundef = tmp_fundef->node[1];
    /* now add "extern" declarations */
    while (NULL != tmp_fundef)
        if (NULL != tmp_fundef->node[1]) {
            /* this isn't the main function */
            new_fundef = MakeNode (N_fundef);
            new_fundef->node[3] = tmp_fundef->node[3]; /* share N_icm ND_FUN_DEC */
            new_fundef->node[1] = fundef; /* put it in front of N_fundef nodes */
            fundef = new_fundef;
            tmp_fundef = tmp_fundef->node[1];
        } else
            tmp_fundef = tmp_fundef->node[1];

    /* insert expanded tree */
    if (N_modul == arg_node->nodetype)
        arg_node->node[2] = fundef;
    else
        arg_node->node[1] = fundef;

#ifdef MALLOC_TOOL
    malloc_verify ();
    malloc_debug (0);
#endif /* MALLOC_TOOL */

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompVardec
 *  arguments     : 1) N_vardec node
 *                  2) NULL
 *  description   : transforms N_vardec to N_icm node if it is the declaration
 *                  of an array
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,GET_BASIC_TYPES, TYPES, DIM,
 *  remarks       : if it is a declaration of an array, a N_assign
 *                  node will be inserted to get the new N_icm node into the
 *                  chain of N_vardecs
 *
 */
node *
CompVardec (node *arg_node, node *arg_info)
{
    node *assign, *id_node, *n_node, *n_dim, *id_type, *icm_arg;
    int i;

    DBUG_ENTER ("CompVardec");

    if ((1 == IsArray (arg_node->TYPES)) && (arg_node->DIM >= 0)) {
        types *b_type;

        if (T_user == arg_node->SIMPLETYPE) {
            GET_BASIC_TYPE (b_type, arg_node->TYPES, 0);
        } else
            b_type = arg_node->TYPES;

        MAKENODE_ID (id_type, type_string[b_type->simpletype]); /* declared type */
        MAKENODE_ID (id_node, arg_node->ID);                    /* name of variable */
        MAKENODE_NUM (n_dim, b_type->dim);

        /* now create N_icm */
        MAKE_ICM (assign);
        MAKE_ICM_NAME (assign->node[0], "ND_KS_DECL_ARRAY");
        if (NULL != arg_node->node[0])
            assign->node[1] = arg_node->node[0];
        MAKE_ICM_ARG (assign->node[0]->node[0], id_type);
        icm_arg = assign->node[0]->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);
        MAKE_NEXT_ICM_ARG (icm_arg, n_dim);
        for (i = 0; i < b_type->dim; i++) {
            /* the shape information will be converted & added */
            MAKENODE_NUM (n_node, b_type->shpseg->shp[i]);
            MAKE_NEXT_ICM_ARG (icm_arg, n_node);
        }
        /* now free some nodes */
        if (T_user == arg_node->SIMPLETYPE) {
            FREE_TYPE (b_type);
        }
        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */
        if (NULL != arg_node->node[1]) {
            arg_node->node[1] = Trav (arg_node->node[1], NULL);
            if (NULL == arg_node->node[1])
                arg_node->nnode = 1;
            else
                arg_node->nnode = 2;
        }
    } else if (arg_node->DIM < 0) {
        /* current vardec-node has unknown shape and will be removed */
        node *tmp;
        tmp = arg_node;
        if (1 == arg_node->nnode)
            arg_node = Trav (arg_node->node[0], NULL);
        else
            arg_node = NULL;
        FREE_VARDEC (tmp);
    } else
      /* traverse next N_vardec node if any */
      if (1 == arg_node->nnode) {
        arg_node->node[0] = Trav (arg_node->node[0], NULL);
        if (NULL == arg_node->node[0])
            arg_node->nnode = 0;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompPrf
 *  arguments     : 1) N_prf node
 *                  2) NULL
 *  description   : transforms N_prf node to N_icm nodes if prf works on
 *                  arrays
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., ELIMINATE_CAST
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN )
 *                  arg_info->node[0] is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  arg_info->node[1] contains pointer to last N_let
 */
node *
CompPrf (node *arg_node, node *arg_info)
{
    node *array, *scalar, *tmp, *res, *res_ref, *n_node, *icm_arg, *prf_id_node,
      *type_id_node, *arg1, *arg2, *arg3, *n_node1, *n_elems_node, *first_assign,
      *next_assign, *last_assign, *old_arg_node, *length_node, *tmp_array1, *tmp_array2,
      *dim_node, *tmp_rc, *exprs;
    simpletype s_type;
    int dim, is_SxA = 0, n_elems = 0, is_drop = 0, array_is_const = 0, convert = 0;

    DBUG_ENTER ("CompPrf");

    DBUG_PRINT ("COMP", ("%s line: %d", mdb_prf[arg_node->info.prf], arg_node->lineno));

    /* first eliminate N_cast node form arguments of primitive function */
    exprs = arg_node->node[0];

    do {
        ELIMINATE_CAST (exprs);
        exprs = exprs->node[1];
    } while (NULL != exprs);

    /* NOTE :  F_neq should be the last "function enumerator" that hasn't
     *         arrays as arguments.
     */
    if (arg_node->info.prf > F_neq) {
        ids *let_ids = arg_info->IDS;
        node *new_name, *vardec_p, *new_assign, *new_vardec, *old_name;
        int insert_vardec = 0, insert_assign = 0;

        exprs = arg_node->node[0];
        while (NULL != exprs) {
            if (N_id == exprs->node[0]->nodetype)
                if (0 == strcmp (let_ids->id, exprs->node[0]->IDS_ID)) {
                    if (0 == insert_assign) {
                        MAKENODE_ID (new_name, RenameVar (let_ids->id, 0));
                        MAKENODE_ID_REUSE_IDS (old_name, let_ids);
                        CREATE_2_ARY_ICM (new_assign, "ND_KS_ASSIGN_ARRAY", old_name,
                                          new_name);
                        new_assign->node[1] = LAST_ASSIGN (arg_info);
                        new_assign->nnode = 2;
                        INSERT_BEFORE (arg_info, new_assign);

                        /* set info_node to right node (update info_node )*/
                        arg_info->node[0] = new_assign;

                        insert_assign = 1;

                        /* now insert vardec if necessary */
                        vardec_p = let_ids->node;
                        if (NULL != vardec_p->node[0])
                            if (0 != strcmp (new_name->IDS_ID, vardec_p->node[0]->ID))
                                insert_vardec = 1;
                            else
                                insert_vardec = 0;
                        else
                            insert_vardec = 1;
                        if (1 == insert_vardec) {
                            new_vardec = MakeNode (N_vardec);
                            new_vardec->TYPES = DuplicateTypes (vardec_p->TYPES, 0);
                            new_vardec->ID = RenameVar (let_ids->id, 0);
                            new_vardec->node[0] = vardec_p->node[0];
                            vardec_p->node[0] = new_vardec;
                        }
                    }

                    /* now rename N_id */
                    FREE (exprs->node[0]->IDS_ID);
                    exprs->node[0]->IDS_ID = RenameVar (let_ids->id, 0);
                }

            exprs = exprs->node[1];
        }
        switch (arg_node->info.prf) {
        case F_add_SxA:
        case F_div_SxA:
        case F_sub_SxA:
        case F_mul_SxA: {
            is_SxA = 1;
            /* here is NO break missing */
        }
        case F_add_AxS:
        case F_div_AxS:
        case F_sub_AxS:
        case F_mul_AxS: {
            /* store arguments and result (as N_id)  */
            if (0 == is_SxA) {
                array = arg_node->node[0]->node[0];
                scalar = arg_node->node[0]->node[1]->node[0];
            } else {
                array = arg_node->node[0]->node[1]->node[0];
                scalar = arg_node->node[0]->node[0];
            }
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* store prf as N_id */
            MAKENODE_ID (prf_id_node, prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

            if (N_id == array->nodetype) {
                last_assign = NEXT_ASSIGN (arg_info);

                CHECK_REUSE__ALLOC_ARRAY_ND (array, res_ref);
            } else {
                /* array is constant, so make a block , declare a temporary
                 * variable __TMP and create a constant array
                 */

                array_is_const = 1;
                old_arg_node = arg_node;

                /* count number of elements */
                tmp = array->node[0];
                while (NULL != tmp) {
                    n_elems += 1;
                    tmp = tmp->node[1];
                }

                MAKENODE_NUM (n_node, n_elems);
                MAKENODE_NUM (n_node1, 1);
                MAKENODE_ID (tmp_array1, "__TMP");

                arg_info->node[1]->nodetype = N_block; /* reuse previous N_let*/
                CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node,
                                  tmp_array1, n_node1, n_node);
                arg_node = first_assign;

                /* create const array */
                CREATE_CONST_ARRAY (array, tmp_array1, type_id_node, res_ref);

                /* reuse temporary array __TMP */
                CREATE_2_ARY_ICM (next_assign, "ND_REUSE", tmp_array1, res);
                APPEND_ASSIGNS (first_assign, next_assign);

                array = tmp_array1; /* set array to __TMP */
            }

            if (0 == is_SxA) {
                /* create ND_BINOP_AxS_A */
                CREATE_4_ARY_ICM (next_assign, "ND_BINOP_AxS_A", prf_id_node, array,
                                  scalar, res);
            } else {
                /* create ND_BINOP_SxA_A */
                CREATE_4_ARY_ICM (next_assign, "ND_BINOP_SxA_A", prf_id_node, scalar,
                                  array, res);
            }
            APPEND_ASSIGNS (first_assign, next_assign);

            if (0 == array_is_const) {
                if (0 < res_ref->info.cint) {
                    /* create ND_INC_RC */
                    CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* create ND_DEC_RC */
                    MAKENODE_NUM (n_node, 1);
                    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", array, n_node);
                    APPEND_ASSIGNS (first_assign, next_assign);
                } else {
                    /* create ND_DEC_RC_FREE */
                    MAKENODE_NUM (n_node, 1);
                    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE", array, n_node);
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
            }

            if (0 == array_is_const) {
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);
            break;
        }
        case F_add_AxA:
        case F_sub_AxA:
        case F_mul_AxA:
        case F_div_AxA: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* store prf as N_id */
            MAKENODE_ID (prf_id_node, prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

            if ((N_id == arg1->nodetype) && (N_id == arg2->nodetype)) {
                last_assign = NEXT_ASSIGN (arg_info);
                if ((1 >= arg1->refcnt) && (1 >= arg2->refcnt)) {
                    node *num;
                    BIN_ICM_REUSE (arg_info->node[1], "ND_CHECK_REUSE", arg1, res);
                    SET_VARS_FOR_MORE_ICMS;
                    CREATE_2_ARY_ICM (next_assign, "ND_CHECK_REUSE", arg2, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    MAKENODE_NUM (num, 0);
                    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                      num);
                    APPEND_ASSIGNS (first_assign, next_assign);
                } else if (1 >= arg1->refcnt) {
                    CHECK_REUSE__ALLOC_ARRAY_ND (arg1, res_ref);
                } else {
                    CHECK_REUSE__ALLOC_ARRAY_ND (arg2, res_ref);
                }
            } else {
                arg_info->node[1]->nodetype = N_block; /*  reuse previous N_let*/
                old_arg_node = arg_node;
                if ((N_array == arg1->nodetype) && (N_array == arg2->nodetype)) {
                    array_is_const = 3;
                    DECL_ARRAY (first_assign, arg1->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign; /* set new arg_node */
                    DECL_ARRAY (next_assign, arg2->node[0], "__TMP2", tmp_array2);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    CREATE_CONST_ARRAY (arg1, tmp_array1, type_id_node, res_ref);
                    MAKENODE_NUM (n_node1, 0);
                    CREATE_CONST_ARRAY (arg2, tmp_array2, type_id_node, n_node1);
                    CREATE_2_ARY_ICM (next_assign, "ND_REUSE", tmp_array1, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg1 and arg2 for later use as parameters of BIN_OP */
                    arg1 = tmp_array1;
                    arg2 = tmp_array2;
                } else if (N_array == arg1->nodetype) {
                    array_is_const = 1;
                    DECL_ARRAY (first_assign, arg1->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg1, tmp_array1, type_id_node, res_ref);
                    CREATE_2_ARY_ICM (next_assign, "ND_REUSE", tmp_array1, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg1 for later use as parameters of BIN_OP */
                    arg1 = tmp_array1;
                } else {
                    array_is_const = 2;
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP2", tmp_array2);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg2, tmp_array2, type_id_node, res_ref);
                    CREATE_2_ARY_ICM (next_assign, "ND_REUSE", tmp_array2, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg2 for later use as parameters of BIN_OP */
                    arg2 = tmp_array2;
                }
            }
            CREATE_4_ARY_ICM (next_assign, "ND_BINOP_AxA_A", prf_id_node, arg1, arg2,
                              res);
            APPEND_ASSIGNS (first_assign, next_assign);

            MAKENODE_NUM (n_node, 1);
            switch (array_is_const) {
            case 0:
                CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
                DEC_OR_FREE_RC_ND (arg2, n_node);
                DEC_OR_FREE_RC_ND (arg1, n_node);
                break;
            case 1:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE", arg2, n_node);
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            case 2:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE", arg1, n_node);
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            case 3:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE", arg2, n_node);
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            default:
                DBUG_ASSERT (0, "array_is_const is out of range");
                break;
            }

            if (0 == array_is_const) {
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);

            break;
        }
        case F_drop:
            is_drop = 1;
            /* here is NO break missing */

        case F_take: {
            /* store arguments and result (res contains refcount and pointer to
             * vardec ( don't free arg_info->IDS !!! )
             *
             * if first argument of prf is a scalar (N_um), it will be compiled
             * like an vector (array) with one element
             */
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            DBUG_ASSERT (((N_array == arg1->nodetype) || (N_num == arg1->nodetype)),
                         "first argument of take/drop isn't an array or scalar");

            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

            if (N_id == arg2->nodetype) {
                node *num;

                GET_DIM (dim, arg2->IDS_NODE->TYPES);
                MAKENODE_NUM (dim_node, dim); /* store dimension of argument-array */
                MAKENODE_NUM (num, 0);
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, num);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                DBUG_ASSERT ((N_array == arg2->nodetype), "wrong nodetype");

                MAKENODE_NUM (dim_node, 1); /* store dimension of argument-array */
                CREATE_TMP_CONST_ARRAY (arg2, res_ref);
            }

            if (N_array == arg1->nodetype) {
                exprs = arg1->node[0];
                n_elems = 0;
                do {
                    n_elems += 1;
                    exprs = exprs->node[1];
                } while (NULL != exprs);
            } else
                n_elems = 1;

            MAKENODE_NUM (n_node, n_elems);
            if (1 == is_drop) {
                CREATE_4_ARY_ICM (next_assign, "ND_KD_DROP_CxA_A", dim_node, arg2, res,
                                  n_node);
            } else {
                CREATE_4_ARY_ICM (next_assign, "ND_KD_TAKE_CxA_A", dim_node, arg2, res,
                                  n_node);
            }
            if (N_num == arg1->nodetype) {
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
            } else {
                icm_arg->node[1] = arg1->node[0];
                icm_arg->nnode = 2;
            }
            APPEND_ASSIGNS (first_assign, next_assign);

            MAKENODE_NUM (n_node, 1);
            if (0 == array_is_const) {
                DEC_OR_FREE_RC_ND (arg2, n_node);
                INC_RC_ND (res, res_ref);
            }
            if (0 == array_is_const) {
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);
            break;
        }
        case F_reshape: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            FREE_TREE (arg1);
            FREE (arg_node->node[0]);
            if (N_array == arg2->nodetype) {
                arg_node = CompArray (arg2, arg_info);
            } else {
                DBUG_ASSERT ((N_id == arg2->nodetype), "wrong nodetype");
                if (0 == strcmp (arg2->IDS_ID, arg_info->IDS_ID)) {
                    FREE_IDS (arg2->IDS);
                    FREE (arg2);
                    FREE (arg_node->node[0]->node[1]);
                    arg_node = NULL;
                    arg_info->node[1]->nodetype = N_icm;
                    arg_info->node[1]->nnode = 0;
                    /* don't free  arg_info->node[1]->IDS, because ..->IDS_ID
                       is shared with vardec  FREE_IDS(arg_info->node[1]->IDS);
                       */
                    MAKE_ICM_NAME (arg_info->node[1], "NOOP");
                } else {
                    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
                    BIN_ICM_REUSE (arg_info->node[1], "ND_KS_ASSIGN_ARRAY", arg2, res);
                    SET_VARS_FOR_MORE_ICMS;
                    if (res->IDS_REFCNT > 1) {
                        MAKENODE_NUM (n_node, res->IDS_REFCNT - 1);
                        INC_RC_ND (res, n_node);
                        INSERT_ASSIGN;
                    } else if (0 == res->IDS_REFCNT) {
                        MAKENODE_NUM (n_node, 1);
                        DEC_OR_FREE_RC_ND (res, n_node);
                        INSERT_ASSIGN;
                    }
                    FREE (old_arg_node);
                }
            }
            break;
        }
        case F_psi: {
            /* store arguments and result (res contains refcount and pointer to
             * vardec ( don't free arg_info->IDS !!! )
             */
            node *line, *arg1_length;

            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            MAKENODE_NUM (line, arg_node->lineno);

            last_assign = NEXT_ASSIGN (arg_info);

            /* compute basic type */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* compute length of arg1 */
            if (N_id == arg1->nodetype) {
                GET_LENGTH (n_elems, arg1->IDS_NODE->TYPES);
                MAKENODE_NUM (arg1_length, n_elems);
            } else {
                n_elems = 0;
                tmp = arg1->node[0];
                do {
                    n_elems += 1;
                    tmp = tmp->node[1];
                } while (NULL != tmp);
                MAKENODE_NUM (arg1_length, n_elems);
            }

            if (0 == IsArray (res->IDS_NODE->TYPES)) {
                if (N_id == arg2->nodetype) {
                    if (N_id == arg1->nodetype) {
                        BIN_ICM_REUSE (arg_info->node[1], "ND_KD_PSI_VxA_S", line, arg2);
                        MAKE_NEXT_ICM_ARG (icm_arg, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1_length);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                    } else {
                        BIN_ICM_REUSE (arg_info->node[1], "ND_KD_PSI_CxA_S", line, arg2);
                        MAKE_NEXT_ICM_ARG (icm_arg, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1_length);
                        icm_arg->node[1] = arg1->node[0];
                        icm_arg->nnode = 2;
                        FREE (arg1);
                    }
                    first_assign = LAST_ASSIGN (arg_info);
                    old_arg_node = arg_node;
                    arg_node = arg_info->node[1]->node[0];
                    MAKENODE_NUM (n_node, 1);
                    DEC_RC_FREE_ND (arg2, n_node);
                    if (N_id == arg1->nodetype) {
                        DEC_OR_FREE_RC_ND (arg1, n_node);
                    }

                    INSERT_ASSIGN;

                    FREE (old_arg_node);
                } else {
                    /* arg2 is a constant array */
                    DBUG_ASSERT ((N_array == arg2->nodetype), "nodetype != N_array");
                    MAKENODE_NUM (tmp_rc, 0);
                    CREATE_TMP_CONST_ARRAY (arg2, tmp_rc);
                    if (N_id == arg1->nodetype) {
                        CREATE_5_ARY_ICM (next_assign, "ND_KD_PSI_VxA_S", line, arg2, res,
                                          arg1_length, arg1);
                    } else {
                        CREATE_4_ARY_ICM (next_assign, "ND_KD_PSI_CxA_S", line, arg2, res,
                                          arg1_length);
                        icm_arg->node[1] = arg1->node[0];
                        icm_arg->nnode = 2;
                        FREE (arg1);
                    }
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
            } else {
                node *num;

                DBUG_ASSERT ((N_id == arg2->nodetype), "arg2 != N_id");
                GET_DIM (dim, arg2->IDS_NODE->TYPES);
                MAKENODE_NUM (dim_node, dim);
                /* store refcount of res as N_num */
                MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

                MAKENODE_NUM (num, 0);
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, num);
                SET_VARS_FOR_MORE_ICMS;

                if (N_id == arg1->nodetype) {
                    CREATE_6_ARY_ICM (next_assign, "ND_KD_PSI_VxA_A", line, dim_node,
                                      arg2, res, arg1_length, arg1);
                } else {
                    CREATE_5_ARY_ICM (next_assign, "ND_KD_PSI_CxA_A", line, dim_node,
                                      arg2, res, arg1_length);
                    icm_arg->node[1] = arg1->node[0];
                    icm_arg->nnode = 2;
                    FREE (arg1);
                }
                APPEND_ASSIGNS (first_assign, next_assign);

                MAKENODE_NUM (n_node, 1);
                DEC_RC_FREE_ND (arg2, n_node);
                INC_RC_ND (res, res_ref);
                INSERT_ASSIGN;
                FREE (old_arg_node);
            }
            break;
        }
        case F_idx_psi: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            DBUG_ASSERT (N_id == arg1->nodetype, "wrong first arg of idx_psi");
            DBUG_ASSERT (N_id == arg2->nodetype, "wrong second arg of idx_psi");

            /* reuse last N_let node */
            arg_info->node[1]->nodetype = N_icm;
            MAKE_ICM_NAME (arg_info->node[1], "ND_IDX_PSI");

            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            MAKE_ICM_ARG (icm_arg, res);
            /* append res to arguments of current node  */
            arg_node->node[0]->node[1]->node[1] = icm_arg;
            arg_node->node[0]->node[1]->nnode = 2;
            /* set arg_node, because arg_node will be returned */
            old_arg_node = arg_node;
            arg_node = arg_node->node[0];
            FREE (old_arg_node);
            break;
        }
        case F_dim: {
            arg1 = arg_node->node[0]->node[0];
            arg_node->nodetype = N_num;
            if (N_array == arg1->nodetype)
                arg_node->info.cint = 1;
            else {
                node *id_node;

                GET_DIM (arg_node->info.cint, arg1->IDS_NODE->TYPES);
                first_assign = LAST_ASSIGN (arg_info);
                last_assign = NEXT_ASSIGN (arg_info);
                MAKENODE_NUM (n_node, 1);
                MAKENODE_ID (id_node, arg1->IDS_ID);
                DEC_OR_FREE_RC_ND (id_node, n_node);
                INSERT_ASSIGN;
            }
            arg_node->nnode = 0;
            FREE_TREE (arg_node->node[0]);
            break;
        }
        case F_shape: {
            int dim;
            arg1 = arg_node->node[0]->node[0];
            MAKENODE_ID (type_id_node, "int");          /* store type of new array */
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS); /* store name of new array */
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);
            BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
            MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
            SET_VARS_FOR_MORE_ICMS;
            if (N_id == arg1->nodetype) {
                GET_DIM (dim, arg1->IDS_NODE->TYPES);
                tmp_array1 = ShapeToArray (arg1->IDS_NODE);
            } else {
                DBUG_ASSERT ((N_array == arg1->nodetype), "wrong nodetype");
                COUNT_ELEMS (n_elems, arg1->node[0]);
                tmp_array1 = MakeNode (N_exprs);
                MAKENODE_NUM (tmp_array1->node[0], n_elems);
                tmp_array1->nnode = 1;
                dim = 1;
            }
            MAKENODE_NUM (length_node, dim); /* store length of shape_vector */
            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res, length_node);
            icm_arg->node[1] = tmp_array1; /* append shape_vector */
            icm_arg->nnode = 2;
            APPEND_ASSIGNS (first_assign, next_assign);
            if (N_id == arg1->nodetype) {
                node *id_node;
                MAKENODE_NUM (n_node, 1);
                MAKENODE_ID (id_node, arg1->IDS_ID);
                DEC_OR_FREE_RC_ND (id_node, n_node);
            }
            FREE_TREE (old_arg_node);
            INSERT_ASSIGN;
            break;
        }
        case F_cat: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

            if ((N_id == arg2->nodetype) && (N_id == arg3->nodetype)) {
                GET_DIM (dim, arg2->IDS_NODE->TYPES);
                MAKENODE_NUM (dim_node, dim);
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                SET_VARS_FOR_MORE_ICMS;
                CREATE_5_ARY_ICM (next_assign, "ND_KD_CAT_SxAxA_A", dim_node, arg2, arg3,
                                  res, arg1);
                APPEND_ASSIGNS (first_assign, next_assign);
                MAKENODE_NUM (n_node, 1);
                if (1 >= arg2->IDS_REFCNT) {
                    DEC_RC_FREE_ND (arg2, n_node);
                } else {
                    DEC_RC_ND (arg2, n_node);
                }
                if (1 >= arg3->IDS_REFCNT) {
                    DEC_RC_FREE_ND (arg3, n_node);
                } else {
                    DEC_RC_ND (arg3, n_node);
                }
                INC_RC_ND (res, res_ref);
                INSERT_ASSIGN;

            } else {
                arg_info->node[1]->nodetype = N_block; /*  reuse previous N_let*/
                old_arg_node = arg_node;
                MAKENODE_NUM (tmp_rc, 0);
                if ((N_array == arg2->nodetype) && (N_array == arg3->nodetype)) {
                    array_is_const = 3;
                    GET_DIM (dim, arg2->TYPES);
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign; /* set new arg_node */
                    DECL_ARRAY (next_assign, arg3->node[0], "__TMP2", tmp_array2);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    CREATE_CONST_ARRAY (arg2, tmp_array1, type_id_node, tmp_rc);
                    CREATE_CONST_ARRAY (arg3, tmp_array2, type_id_node, tmp_rc);
                    arg2 = tmp_array1;
                    arg3 = tmp_array2;
                } else if (N_array == arg2->nodetype) {
                    array_is_const = 1;
                    GET_DIM (dim, arg2->TYPES);
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg2, tmp_array1, type_id_node, tmp_rc);
                    /* set arg2 for later use as parameters of ND_KD_CAT*/
                    arg2 = tmp_array1;
                } else {
                    array_is_const = 2;
                    GET_DIM (dim, arg3->TYPES);
                    DECL_ARRAY (first_assign, arg3->node[0], "__TMP2", tmp_array2);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg3, tmp_array2, type_id_node, tmp_rc);
                    /* set arg3 for later use as parameters of ND_KD_CAT*/
                    arg3 = tmp_array2;
                }
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                  res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
                MAKENODE_NUM (dim_node, dim);
                CREATE_5_ARY_ICM (next_assign, "ND_KD_CAT_SxAxA_A", dim_node, arg2, arg3,
                                  res, arg1);
                APPEND_ASSIGNS (first_assign, next_assign);
                MAKENODE_NUM (n_node, 1);
                switch (array_is_const) {
                case 1: {
                    if (1 >= arg3->IDS_REFCNT) {
                        DEC_RC_FREE_ND (arg3, n_node);
                    } else {
                        DEC_RC_ND (arg3, n_node);
                    }
                    DEC_RC_FREE_ND (arg2, n_node);
                    break;
                }
                case 2: {
                    if (1 >= arg2->IDS_REFCNT) {
                        DEC_RC_FREE_ND (arg2, n_node);
                    } else {
                        DEC_RC_ND (arg2, n_node);
                    }
                    DEC_RC_FREE_ND (arg3, n_node);
                    break;
                }
                case 3: {
                    DEC_RC_FREE_ND (arg2, n_node);
                    DEC_RC_FREE_ND (arg3, n_node);
                    break;
                }
                default:
                    DBUG_ASSERT (0, "array_is_const is out of range");
                    break;
                }
                CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
                FREE (old_arg_node);
            }
            break;
        }
        case F_rotate: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);
            MAKENODE_NUM (n_node, 1);

            if (N_id == arg3->nodetype) {
                GET_DIM (dim, arg3->IDS_NODE->TYPES); /* dim will be used later */
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                array_is_const = 1;
                arg_info->node[1]->nodetype = N_block; /*  reuse previous N_let*/
                old_arg_node = arg_node;
                GET_DIM (dim, arg3->TYPES); /* dim will be used later */
                DECL_ARRAY (first_assign, arg3->node[0], "__TMP1", tmp_array1);
                arg_node = first_assign;
                CREATE_CONST_ARRAY (arg3, tmp_array1, type_id_node, n_node);
                arg3 = tmp_array1;
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                  res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
            }
            /* store dimension of arg3 */
            MAKENODE_NUM (dim_node, dim);
            DBUG_ASSERT ((N_num == arg1->nodetype), "wrong 1.arg of 'rotate'");
            CREATE_5_ARY_ICM (next_assign, "ND_KD_ROT_CxSxA_A", arg1, arg2, dim_node,
                              arg3, res);
            APPEND_ASSIGNS (first_assign, next_assign);
            if (0 == array_is_const) {

                DEC_OR_FREE_RC_ND (arg3, n_node);
                INSERT_ASSIGN;
            } else {
                DEC_RC_FREE_ND (arg3, n_node);
                FREE (old_arg_node);
            }
            break;
        }
        case F_itof_A:
            convert = 2;
            /* here is NO break missing !! */
        case F_itod_A:
            convert = 3;
            /* here is NO break missing !! */
        case F_ftod_A:
            convert = 2;
            /* here is NO break missing !! */
        case F_dtoi_A:
            convert = 4;
            /* here is NO break missing !! */
        case F_dtof_A:
            convert = 5;
            /* here is NO break missing !! */
        case F_ftoi_A: {
            int length;
            node *res_rc, *n_length;

            arg1 = arg_node->node[0]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            /* compute basic type */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);
            MAKENODE_NUM (res_rc, arg_info->IDS_REFCNT);
            BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
            MAKE_NEXT_ICM_ARG (icm_arg, res_rc);
            SET_VARS_FOR_MORE_ICMS;

            if (N_id == arg1->nodetype) {
                switch (convert) {
                case 0:
                    CREATE_2_ARY_ICM (next_assign, "ND_F2I_A", arg1, res);
                    break;
                case 1:
                    CREATE_2_ARY_ICM (next_assign, "ND_F2D_A", arg1, res);
                    break;
                case 2:
                    CREATE_2_ARY_ICM (next_assign, "ND_I2F_A", arg1, res);
                    break;
                case 3:
                    CREATE_2_ARY_ICM (next_assign, "ND_I2D_A", arg1, res);
                    break;
                case 4:
                    CREATE_2_ARY_ICM (next_assign, "ND_D2I_A", arg1, res);
                    break;
                case 5:
                    CREATE_2_ARY_ICM (next_assign, "ND_D2F_A", arg1, res);
                    break;
                default:
                    DBUG_ASSERT (0, "wrong tag (convert)");
                    break;
                }
                APPEND_ASSIGNS (first_assign, next_assign);

                MAKENODE_NUM (n_node, 1);
                DEC_OR_FREE_RC_ND (arg1, n_node);
                INSERT_ASSIGN;
            } else {
                DBUG_ASSERT (N_array == arg1->nodetype, "wrong node != N_array");
                DBUG_ASSERT (NULL != arg1->TYPES, " info.types is NULL");
                COUNT_ELEMS (length, arg1->node[0]);
                MAKENODE_NUM (n_node, length);
                if (1 < arg1->DIM) {
                    node *dummy;
                    /* it is an array of arrays, so we have to use
                     * ND_CREATE_CONST_ARRAY_A
                     */
                    DBUG_ASSERT (N_id == arg1->node[0]->node[0]->nodetype,
                                 "wrong node != N_id");
                    GET_LENGTH (length, arg1->node[0]->node[0]->IDS_NODE->TYPES);
                    MAKENODE_NUM (n_length, length);

                    CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res,
                                      n_length, n_node);
                    icm_arg->node[1] = arg1->node[0];
                    icm_arg->nnode = 2;
                    APPEND_ASSIGNS (first_assign, next_assign);

                    /* now decrement refcount of the arrays */
                    dummy = arg1->node[0];
                    MAKENODE_NUM (n_node, 1);
                    while (NULL != dummy) {
                        DBUG_ASSERT (N_id == dummy->node[0]->nodetype,
                                     "wrong nodetype != N_id");
                        DEC_OR_FREE_RC_ND (dummy->node[0], n_node);
                        dummy = dummy->node[1];
                    }
                } else {
                    /* it is an array out of scalar values */
                    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res,
                                      n_node);
                    icm_arg->node[1] = arg1->node[0];
                    icm_arg->nnode = 2;
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
                INSERT_ASSIGN;
            }
            break;
        }
        default:
            /*   DBUG_ASSERT(0,"wrong prf"); */
            break;
        }
    } else /* (arg_node->info.prf > F_neq) */
      if ((arg_node->info.prf == F_ftoi) || (arg_node->info.prf == F_ftod)
          || (arg_node->info.prf == F_itof) || (arg_node->info.prf == F_itod)
          || (arg_node->info.prf == F_dtof) || (arg_node->info.prf == F_dtoi)) {
        node *dummy = arg_node;
        /* return argument of ftoi */
        arg_node = arg_node->node[0]->node[0];
        FREE (dummy->node[0]); /* free N_exprs node */
        FREE (dummy);          /* free N_prf node */
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompAssign
 *  arguments     : 1) N_assign node
 *                  2) NULL
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to previous assign_node
 *                  or N_block node
 *
 */
node *
CompAssign (node *arg_node, node *arg_info)
{
    node *old_next_assign;

    DBUG_ENTER ("CompAssign");

    DBUG_PRINT ("COMP",
                ("current:" P_FORMAT " next:" P_FORMAT, arg_node, arg_node->node[1]));
    DBUG_PRINT ("COMP", ("last:" P_FORMAT "next:" P_FORMAT, LAST_ASSIGN (arg_info),
                         NEXT_ASSIGN (arg_info)));
    if (2 == arg_node->nnode) {
        old_next_assign = arg_node->node[1];
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        arg_info->node[0] = arg_node;
        while (old_next_assign != arg_info->node[0]->node[1])
            arg_info->node[0] = arg_info->node[0]->node[1];

        DBUG_PRINT ("COMP", ("set info->node[0] to :" P_FORMAT " (node[0]:" P_FORMAT,
                             arg_info->node[0], arg_info->node[0]->node[0]));
        DBUG_ASSERT (N_icm != old_next_assign->node[0]->nodetype, "wrong nodetype"
                                                                  " N_icm");
        Trav (old_next_assign, arg_info);
    } else
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompLet
 *  arguments     : 1) N_Let node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer node before
 *                   last assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CompLet");

    arg_info->node[1] = arg_node;
    arg_info->IDS = arg_node->IDS;
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    arg_info->node[1] = NULL;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompArray
 *  arguments     : 1) N_arrray node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to node before
 *                    last assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompArray (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *exprs, *res, *type_id_node, *old_arg_node,
      *icm_arg, *n_node, *res_ref, *last_assign;
    simpletype s_type;
    int n_elems = 0, icm_created = 0;

    DBUG_ENTER ("CompArray");

    /* store next assign */
    last_assign = NEXT_ASSIGN (arg_info);

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* store result as N_id */
    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

    /* create ND_ALLOC_ARRAY */
    BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
    MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
    first_assign = LAST_ASSIGN (arg_info);
    old_arg_node = arg_node;
    arg_node = arg_info->node[1]->node[0];

    /* create ND_CREATE_CONST_ARRAY */
    exprs = old_arg_node->node[0];
    do {
        n_elems += 1;
        exprs = exprs->node[1];
    } while (NULL != exprs);
    MAKENODE_NUM (n_node, n_elems);
    DBUG_ASSERT (NULL != old_arg_node->node[0], " NULL pointer ");
    if (N_id == old_arg_node->node[0]->node[0]->nodetype)
        if (1 == IsArray (old_arg_node->node[0]->node[0]->IDS_NODE->TYPES)) {
            node *length;
            int len;
            GET_LENGTH (len, old_arg_node->node[0]->node[0]->IDS_NODE->TYPES);
            MAKENODE_NUM (length, len);
            CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res, length,
                              n_node);
            icm_created = 1;
        }

    if (0 == icm_created) {
        CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res, n_node);
    }
    /* now append the elements of the array to the last N_icm */
    icm_arg->node[1] = old_arg_node->node[0];
    icm_arg->nnode = 2;
    APPEND_ASSIGNS (first_assign, next_assign);

    INSERT_ASSIGN;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompId
 *  arguments     : 1) N_id node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to to node before
 *                   last assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompId (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *last_assign, *old_arg_node, *icm_arg, *res,
      *n_node, *icm_node;

    DBUG_ENTER ("CompId");
    if (1 == IsArray (arg_node->IDS_NODE->TYPES)) {
        if (NULL != arg_info) {
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            BIN_ICM_REUSE (arg_info->node[1], "ND_KS_ASSIGN_ARRAY", arg_node, res);
            SET_VARS_FOR_MORE_ICMS;
            if (0 == arg_info->IDS_REFCNT) {
                MAKENODE_NUM (n_node, 1);
                DEC_RC_FREE_ND (res, n_node);
            } else if (1 < arg_info->IDS_REFCNT) {
                MAKENODE_NUM (n_node, arg_info->IDS_REFCNT - 1);
                INC_RC_ND (res, n_node);
            }
            INSERT_ASSIGN;
        } else {
            icm_node = MakeNode (N_icm);
            MAKE_ICM_NAME (icm_node, "ND_KS_RET_ARRAY");
            MAKE_ICM_ARG (icm_node->node[0], arg_node);
            icm_node->nnode = 1;
            arg_node = icm_node;
        }
    }

    DBUG_RETURN (arg_node);
}
/*
 *
 *  functionname  : CompAp
 *  arguments     : 1) N_Ap node
 *                  2) info node
 *  description   : - creates N_icm for function application
 *                  - insert N_icm to decrement the refcount of functions
 *                    arguments
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompAp (node *arg_node, node *arg_info)
{
    int n = 0;
    node *tmp, *last, *next, *exprs, *icm_arg, *id_node, *tag_node, *outs, *fun_args,
      *next_assign, *first_assign, *refs_node, *n_node, *ap_icm_arg;
    ids *ids;

    DBUG_ENTER ("CompAp");

    fun_args = arg_node->node[0];
    refs_node = MakeNode (N_assign); /* refs_node will be used to store N_icms
                                      * for incrementation and decremantation
                                      * of refcounts
                                      */
    first_assign = refs_node;
    /* first take Let ids (variables assigned to) */
    ids = arg_info->node[1]->IDS;
    MAKENODE_ID_REUSE_IDS (id_node, ids);
    if (1 == IsArray (id_node->IDS_NODE->TYPES)) {
        MAKENODE_ID (tag_node, "out_a");
        if (1 < ids->refcnt) {
            MAKENODE_NUM (n_node, (ids->refcnt - 1));
            INC_RC_ND (id_node, n_node); /* create N_icm to increment refcount of
                                          * function result. It will be stored in
                                          * refs_node->node[1]->.. and will be
                                          * inserted later.
                                          */
        }
    } else {
        MAKENODE_ID (tag_node, "out");
    }
    MAKE_ICM_ARG (ap_icm_arg, tag_node);
    outs = ap_icm_arg;
    MAKE_NEXT_ICM_ARG (ap_icm_arg, id_node);
    n += 1;
    ids = ids->next;
    while (NULL != ids) {
        MAKENODE_ID_REUSE_IDS (id_node, ids);
        if (1 == IsArray (id_node->IDS_NODE->TYPES)) {
            if (1 < ids->refcnt) {
                /* create N_icm to increment refcount of function result.
                 * It will be stored in refs_node->node[1]->.. and will be
                 * inserted later.
                 */
                MAKENODE_NUM (n_node, ids->refcnt);
                INC_RC_ND (id_node, n_node);
            }

            MAKENODE_ID (tag_node, "out_a");

        } else {
            MAKENODE_ID (tag_node, "out");
        }
        MAKE_NEXT_ICM_ARG (ap_icm_arg, tag_node);
        MAKE_NEXT_ICM_ARG (ap_icm_arg, id_node);
        n += 1;
        ids = ids->next;
    }

    /* now do the arguments of the function */
    exprs = arg_node->node[0];
    last = ap_icm_arg;
    MAKENODE_NUM (n_node, 1);
    while (NULL != exprs) {
        next = exprs->node[1];
        if (N_id == exprs->node[0]->nodetype)
            if (1 == IsArray (exprs->node[0]->IDS_NODE->TYPES)) {
                INSERT_ID_NODE (exprs, last, "in_a");
                last = exprs;
                exprs = next;
                n += 1;
            } else {
                INSERT_ID_NODE (exprs, last, "in");
                last = exprs;
                exprs = next;
                n += 1;
            }
        else if (N_cast == exprs->node[0]->nodetype) {
            tmp = exprs->node[0];
            exprs->node[0] = exprs->node[0]->node[0];
            FREE_TYPE (tmp->TYPES);
            FREE (tmp);
            /* do loop again with new exprs->node[0] */
        } else {

            INSERT_ID_NODE (exprs, last, "in");
            last = exprs;
            exprs = next;
            n += 1;
        }
    }

    /* put number of arguments and return value in front of 'outs' */
    exprs = MakeNode (N_exprs);
    MAKENODE_NUM (exprs->node[0], n);
    exprs->node[1] = outs;
    exprs->nnode = 2;
    outs = exprs;

    /* put function_name in front */
    exprs = MakeNode (N_exprs);
    MAKENODE_ID (id_node, arg_node->node[1]->ID);
    exprs->node[0] = id_node;
    exprs->node[1] = outs;
    exprs->nnode = 2;
    arg_node = exprs;

    /* make last N_let node to N_icm node */
    arg_info->node[1]->nodetype = N_icm;
    MAKE_ICM_NAME (arg_info->node[1], "ND_FUN_AP");
    /* now insert N_icms for incrementation of refcounts of function results
     * if any
     */
    if (NULL != refs_node->node[1]) {
        node *last_assign;

        last_assign = LAST_ASSIGN (arg_info);
        first_assign->node[1] = NEXT_ASSIGN (arg_info);
        if (NULL != first_assign->node[1])
            first_assign->nnode = 2;
        last_assign->node[1] = refs_node->node[1];
        last_assign->nnode = 2;
    }
    FREE (refs_node); /* refs_node will not be used anymore */

    DBUG_RETURN (arg_node);
}
/*
 *
 *  functionname  : CompWithReturn
 *  arguments     : 1) N_return node
 *                  2) info node
 *  description   : generates N_icms of a with-loop
 *  global vars   :
 *  internal funs : RenameReturn
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : if N_return node contains to a with_loop, then
 *                   arg_info->node[2] will point to the first argument(N_exprs)
 *                   of corresponding N_icm for start of with_loop
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node
 *
 */
node *
CompWithReturn (node *arg_node, node *arg_info)
{
    node *ret_val, *with_icm_arg, *icm_arg, *index_length, *tmp_with_icm_arg, *dec_rc,
      *res, *res_dim, *exprs, *from, *to, *index_node, *next_assign, *first_assign,
      *n_node, *mod_array = NULL;
    int is_array, con_type = 0;

#define MOD_A 1
#define GEN 2
#define FOLD 3

    /* arg_node is a N_return of a with_loop.
     * arg_info->node[2] points to the N_icm that desribes the begin of
     * a with_loop.
     * The arguments of this N_icm are usefull to get the name of
     * the resulting array, the dimension of this array, and the
     * length of the index_vector.
     * The name of the N_icm is used to detect whether N_return node is part
     * of a 'genarray' or 'modarray' with-loop.
     */

    DBUG_ENTER ("CompWithReturn");

    exprs = arg_node->node[0];
    is_array = IsArray (exprs->node[0]->IDS_NODE->TYPES);
    switch (arg_info->node[2]->info.id[9]) {
    case 'F':
        con_type = FOLD;
        break;
    case 'M':
        con_type = MOD_A;
        break;
    case 'G':
        con_type = GEN;
        break;
    default:
        DBUG_ASSERT (0, "unknown Con-expr ");
    }

    /* N_return will be converted to N_icm node */
    arg_node->nodetype = N_icm;
    ret_val = arg_node->node[0]->node[0]; /* store return_value node */
    with_icm_arg = arg_info->node[2]->node[0];
    /* 'dec_rc' points to a list of variables whose refcount has to be
     * decremented
     */
    dec_rc = arg_info->node[2]->node[3]->node[0];

    /* store name of resulting array */
    res = with_icm_arg->node[0];
    with_icm_arg = with_icm_arg->node[1];
    /* store dimension of resulting array */
    res_dim = with_icm_arg->node[0];
    tmp_with_icm_arg = with_icm_arg->node[1];

    if (MOD_A == con_type) {
        mod_array = tmp_with_icm_arg->node[0];
        tmp_with_icm_arg = tmp_with_icm_arg->node[1];
    }

    /* now we store some information:
     * from        : N_id ( the left border of a mod/genarray)
     * to          : N_id ( the right  border of a mod/genarray)
     * index_node  : N_id ( name of index-vector of mod/genarray)
     * index_length: N_num ( length of index-vector )
     *
     * all informations are shared with belonging mod/genarray N_icm
     */
    from = tmp_with_icm_arg->node[0];
    DBUG_ASSERT (N_id == from->nodetype, " wrong nodetype for 'from'");
    tmp_with_icm_arg = tmp_with_icm_arg->node[1];
    to = tmp_with_icm_arg->node[0];
    DBUG_ASSERT (N_id == to->nodetype, " wrong nodetype for 'to'");
    tmp_with_icm_arg = tmp_with_icm_arg->node[1];
    index_node = tmp_with_icm_arg->node[0];
    tmp_with_icm_arg = tmp_with_icm_arg->node[1];
    DBUG_ASSERT (N_id == index_node->nodetype, " wrong nodetype for 'index_node'");
    index_length = tmp_with_icm_arg->node[0];
    DBUG_ASSERT (N_num == index_length->nodetype, " wrong nodetype for 'index_length'");

    if (MOD_A == con_type) {
        MAKE_ICM_ARG (arg_node->node[0], res);
        icm_arg = arg_node->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, res_dim);
        /* N_return belongs to a 'modarray' with-loop */
        /* add name of modified array */
        MAKE_NEXT_ICM_ARG (icm_arg, mod_array);
        /* add name of return value */
        MAKE_NEXT_ICM_ARG (icm_arg, ret_val);
        if (0 == is_array) {
            MAKE_ICM_NAME (arg_node, "ND_END_MODARRAY_S");
        } else {
            MAKE_ICM_NAME (arg_node, "ND_END_MODARRAY_A");

            /* add length of index_vector */
            MAKE_NEXT_ICM_ARG (icm_arg, index_length);
        }
    } else if (GEN == con_type) {
        MAKE_ICM_ARG (arg_node->node[0], res);
        icm_arg = arg_node->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, res_dim);
        /* N_return belongs to a 'genarray' with-loop */
        /* add name of return value */
        MAKE_NEXT_ICM_ARG (icm_arg, ret_val);
        if (0 == is_array) {
            MAKE_ICM_NAME (arg_node, "ND_END_GENARRAY_S");
        } else {
            MAKE_ICM_NAME (arg_node, "ND_END_GENARRAY_A");
            MAKE_NEXT_ICM_ARG (icm_arg, index_length);
        }
    } else if (FOLD == con_type) {
        node *let, *exprs1, *exprs2, *last_assign, *tmp_res;

        arg_node->nodetype = N_let;
        let = arg_node;
        /* in arg_info->node[2]->node[2] the kind of the fold-function
         * is stored (as N_prf or as N_ap)
         */
        let->node[0] = arg_info->node[2]->node[2];
        let->IDS = MakeIds (res->IDS_ID);
        let->IDS_NODE = res->IDS_NODE;
        let->IDS_REFCNT = 1;
        let->nnode = 1;
        exprs1 = MakeNode (N_exprs);
        MAKENODE_ID (tmp_res, StringCopy (res->IDS_ID));
        tmp_res->IDS_NODE = res->IDS_NODE;
        exprs1->node[0] = tmp_res;
        exprs2 = MakeNode (N_exprs);
        exprs2->node[0] = ret_val;
        exprs2->nnode = 1;
        exprs1->node[1] = exprs2;
        exprs1->nnode = 2;
        let->node[0]->node[0] = exprs1;
        let->node[0]->node[0]->nnode = 1;
        arg_info->IDS = let->IDS;
        arg_info->node[1] = let;
        let->node[0] = Trav (let->node[0], arg_info);

        last_assign = LAST_ASSIGN (arg_info);
        while (2 == last_assign->nnode)
            last_assign = last_assign->node[1];

        MAKE_ICM (next_assign);
        MAKE_ICM_NAME (next_assign->node[0], "ND_END_FOLD");
        MAKE_ICM_ARG (next_assign->node[0]->node[0], index_length);

        /* now insert next_assign */
        last_assign->node[1] = next_assign;
        last_assign->nnode = 2;
        first_assign = next_assign;
    }

    /* now create N_icm to decrement refcount of index_vector ,
     * left(from) and right(to) border of mod/genarray
     */
    MAKENODE_NUM (n_node, 1);
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE", index_node, n_node);
    if (FOLD != con_type) {
        first_assign = next_assign;
        /* now insert next_assign */
        arg_info->node[0]->node[1]->node[1] = next_assign;
        arg_info->node[0]->node[1]->nnode = 2;
    } else {
        APPEND_ASSIGNS (first_assign, next_assign);
    }
    DEC_OR_FREE_RC_ND (from, n_node);
    DEC_OR_FREE_RC_ND (to, n_node);
    if (NULL != mod_array) {
        DEC_RC_FREE_ND (mod_array, n_node);
    }
    while (NULL != dec_rc) {
        if ((0 != strcmp (index_node->IDS_ID, dec_rc->IDS_ID))
            && ((NULL == mod_array)
                  ? 1
                  : (0 != strcmp (mod_array->IDS_ID, dec_rc->IDS_ID)))) {
            DEC_RC_FREE_ND (dec_rc, n_node);
        }
        dec_rc = dec_rc->node[0];
    }
    if (FOLD == con_type) {
        /* return contains to N_foldfun or N_foldprf, so the refcount of the
         * result is stored in the last_argument of the BEGIN N_icm.
         * It will be used for the END N_icm and removed form BEGIN N_icm
         */
        node *last_but_least_icm_arg;
        GOTO_LAST_BUT_LEAST_N_EXPRS (tmp_with_icm_arg, last_but_least_icm_arg);
        if (0 < last_but_least_icm_arg->node[1]->node[0]->info.cint) {
            INC_RC_ND (res, last_but_least_icm_arg->node[1]->node[0]);
        } else {
            FREE (last_but_least_icm_arg->node[1]->node[0]);
        }
        last_but_least_icm_arg->nnode = 1;
        FREE (last_but_least_icm_arg->node[1]);
    }
#undef MOD_A
#undef GEN
#undef FOLD

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompReturn
 *  arguments     : 1) N_return node
 *                  2) info node
 *  description   : generates N_icms for N_return of a function or of a
 *                  with-loop
 *  global vars   :
 *  internal funs : RenameReturn
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : if N_return node contains to a with_loop, then
 *                   arg_info->node[2] will point to the first argument(N_exprs)
 *                   of corresponding N_icm for start of with_loop
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node
 *
 */
node *
CompReturn (node *arg_node, node *arg_info)
{
    int n = 0;
    node *tmp, *next, *exprs, *last;

    DBUG_ENTER ("CompReturn");

    if (NULL == arg_info->node[2]) {
        node *ret;

        /* this is a N_return of a function-body */
        ret = RenameReturn (arg_node, arg_info);
        exprs = arg_node->node[0];
        last = arg_node;
        /* the new N_exprs chain will be stored in arg_node->node[1]
         * temporaryly
         */
        do {
            if (N_cast == exprs->node[0]->nodetype) {
                next = exprs->node[0]->node[0];
                FREE_TYPE (exprs->node[0]->TYPES);
                FREE (exprs->node[0]);
                exprs->node[0] = next;
            } else {
                DBUG_ASSERT ((N_id == exprs->node[0]->nodetype), " wrong node (!= N_id)");
                next = exprs->node[1];
                if (1 == IsArray (exprs->node[0]->IDS_NODE->TYPES)) {
                    INSERT_ID_NODE (exprs, last, "out_a");
                } else {
                    INSERT_ID_NODE (exprs, last, "out");
                }
                last = exprs;
                exprs = next;
                n += 1;
            }
        } while (NULL != exprs);

        arg_node->nodetype = N_icm;
        MAKE_ICM_NAME (arg_node, "ND_FUN_RET");
        exprs = MakeNode (N_exprs);
        MAKENODE_NUM (exprs->node[0], n);
        exprs->node[1] = arg_node->node[1]; /* put number of ret-values in front */
        exprs->nnode = 2;
        arg_node->node[0] = exprs;
        arg_node->node[1] = NULL; /* was only used temporaryly */

        arg_node = ret; /* set new return_value of current function
                         * (N_let node if at least one variable in the "return"
                         * statement has been renamed, or  N_return otherwise)
                         */
    } else
        arg_node = CompWithReturn (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompWith
 *  arguments     : 1) N_with node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to last but one
 *                    assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *                  arg_info->node[2] will be set to  pointer to N_icm
 *                  of with_loop begin
 *
 */
node *
CompWith (node *arg_node, node *arg_info)
{
    node *old_info2, *first_assign, *next_assign, *n_node, *inc_rc, *icm_arg, *from, *to,
      *type_id_node, *arg, *res, *res_ref, *res_dim_node, *index, *indexlen,
      *old_arg_node, *last_assign, *fun_node, *res_size_node;
    int res_dim, is_foldprf, res_size;
    simpletype s_type;

    DBUG_ENTER ("CompWith");

    /* store arg_info->node[2] */
    old_info2 = arg_info->node[2];

    /* store res as N_id */
    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* compute dimension of res */
    GET_DIM (res_dim, arg_info->IDS_NODE->TYPES);
    MAKENODE_NUM (res_dim_node, res_dim);

    /* compute size of res */
    GET_LENGTH (res_size, arg_info->IDS_NODE->TYPES);
    MAKENODE_NUM (res_size_node, res_size);

    /* store index_vector as N_id */
    MAKENODE_ID_REUSE_IDS (index, arg_node->node[0]->IDS);

    /* store length of index-vector */
    MAKENODE_NUM (indexlen, arg_node->node[0]->IDS_NODE->SHP[0]);

    /* set 'from' to left range of index-vector */
    from = arg_node->node[0]->node[0];

    /* set 'to' to left range of index-vector */
    to = arg_node->node[0]->node[1];
    if (res_dim > 0) {
        /* first create N_icm to allocate memeory */
        BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
        SET_VARS_FOR_MORE_ICMS;
        if ((N_modarray == old_arg_node->node[1]->nodetype)
            || (N_genarray == old_arg_node->node[1]->nodetype)) {
            MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
        } else {
            /* in this case (N_foldfun , N_foldprf) the refcount will be set
             * behind the with_loop
             */
            MAKENODE_NUM (n_node, 0);
            MAKE_NEXT_ICM_ARG (icm_arg, n_node);
        }
        MAKENODE_NUM (n_node, 1);
        MAKENODE_ID (type_id_node, "int");
        CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, index, n_node);
        APPEND_ASSIGNS (first_assign, next_assign);
    } else {
        MAKENODE_NUM (n_node, 1);
        MAKENODE_ID (type_id_node, "int");
        BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, index);
        MAKE_NEXT_ICM_ARG (icm_arg, n_node);
        SET_VARS_FOR_MORE_ICMS;
    }

    if (N_modarray == old_arg_node->node[1]->nodetype) {
        arg = old_arg_node->node[1]->node[0];
        DBUG_ASSERT (N_id == arg->nodetype, "wrong nodetype != N_id");
        CREATE_7_ARY_ICM (next_assign, "ND_BEGIN_MODARRAY", res, res_dim_node, arg, from,
                          to, index, indexlen);
        /* store pointer to N_icm ND_BEGIN.. in arg_info->node[2] */
        arg_info->node[2] = next_assign->node[0];
        /* store pointer to variables that have to be increased in
         * in arg_info->node[2]->node[3] ( it will be used in CompReturn )
         */
        arg_info->node[2]->node[3] = old_arg_node->node[2];

        APPEND_ASSIGNS (first_assign, next_assign);
    } else if (N_genarray == old_arg_node->node[1]->nodetype) {
        CREATE_6_ARY_ICM (next_assign, "ND_BEGIN_GENARRAY", res, res_dim_node, from, to,
                          index, indexlen);
        /* store pointer to N_icm ND_BEGIN.. in arg_info->node[2] */
        arg_info->node[2] = next_assign->node[0];
        /* store pointer to variables that have to be increased in
         * in arg_info->node[2]->node[3] ( it will be used in CompReturn )
         */
        arg_info->node[2]->node[3] = old_arg_node->node[2];
        APPEND_ASSIGNS (first_assign, next_assign);
    } else if (N_foldprf == old_arg_node->node[1]->nodetype) {
        fun_node = MakeNode (N_prf);
        fun_node->info.prf = old_arg_node->node[1]->info.prf;
        is_foldprf = 1;
    } else if (N_foldfun == old_arg_node->node[1]->nodetype) {
        fun_node = MakeNode (N_ap);
        fun_node->FUN_NAME = old_arg_node->node[1]->FUN_NAME;
        fun_node->FUN_MOD_NAME = old_arg_node->node[1]->FUN_MOD_NAME;
        fun_node->node[1] = old_arg_node->node[1]->node[2];
        DBUG_ASSERT (N_fundef == fun_node->node[1]->nodetype,
                     "wrong nodetype != N_fundef ");
        is_foldprf = 0;
    } else {
        DBUG_ASSERT (0, "wrong nodetype != N_foldfun, ... ");
    }
    if ((N_foldfun == old_arg_node->node[1]->nodetype)
        || (N_foldprf == old_arg_node->node[1]->nodetype)) {
        node *neutral_node, *n_neutral;
        int length;

        /* old_arg_node->node[1]->node[1] is the neutral element of
         * fold-function
         */
        neutral_node = old_arg_node->node[1]->node[1];
        DBUG_ASSERT (NULL != neutral_node, " neutral element is missing");
        DBUG_ASSERT (2 == old_arg_node->node[1]->nnode, " nnode != 2 ");

        if (N_array == neutral_node->nodetype) {
            COUNT_ELEMS (length, neutral_node->node[0]);
            MAKENODE_NUM (n_neutral, length);
            CREATE_7_ARY_ICM (next_assign,
                              (is_foldprf) ? "ND_BEGIN_FOLDPRF" : "ND_BEGIN_FOLDFUN", res,
                              res_size_node, from, to, index, indexlen, n_neutral);
            icm_arg->node[1] = neutral_node->node[0];
            icm_arg->nnode = 2;
            GOTO_LAST_N_EXPRS (neutral_node->node[0], icm_arg);
        } else {
            length = 1;
            MAKENODE_NUM (n_neutral, length);
            CREATE_7_ARY_ICM (next_assign,
                              (is_foldprf) ? "ND_BEGIN_FOLDPRF" : "ND_BEGIN_FOLDFUN", res,
                              res_size_node, from, to, index, indexlen, n_neutral);
            MAKE_NEXT_ICM_ARG (icm_arg, neutral_node);
        }
        /* append res_ref to N_icm temporarily. It will be used and
         * removed in CompReturn.
         */
        res_ref->info.cint -= 1;
        MAKE_NEXT_ICM_ARG (icm_arg, res_ref);

        /* store pointer to N_icm ND_BEGIN.. in arg_info->node[2] */
        arg_info->node[2] = next_assign->node[0];

        /* store pointer to variables that have to be increased in
         * in arg_info->node[2]->node[3] ( it will be used in CompReturn )
         */
        arg_info->node[2]->node[3] = old_arg_node->node[2];

        /* Store  N_prf or N_ap  in node[2] of current N_icm.
         * It will be used in CompReturn and than eliminated.
         */
        arg_info->node[2]->node[2] = fun_node;
        APPEND_ASSIGNS (first_assign, next_assign);
    }

    /* now add some INC_RC's */
    inc_rc = old_arg_node->node[2]->node[0];
    while (NULL != inc_rc) {
        MAKENODE_NUM (n_node, inc_rc->refcnt);
        INC_RC_ND (inc_rc, n_node);
        inc_rc = inc_rc->node[0];
    }
    /* update arg_info, after inserting new N_assign nodes */
    if ((N_foldprf == old_arg_node->node[1]->nodetype)
        || (N_foldfun == old_arg_node->node[1]->nodetype))
        next_assign->node[1] = old_arg_node->node[1]->node[0]->node[0];
    else
        next_assign->node[1] = old_arg_node->node[1]->node[1]->node[0];
    next_assign->nnode = 2;
    arg_info->node[0] = next_assign;
    DBUG_PRINT ("COMP", ("set info->node[0] to :" P_FORMAT " (node[0]:" P_FORMAT,
                         arg_info->node[0], arg_info->node[0]->node[0]));
    if ((N_foldprf == old_arg_node->node[1]->nodetype)
        || (N_foldfun == old_arg_node->node[1]->nodetype))
        next_assign = Trav (old_arg_node->node[1]->node[0]->node[0], arg_info);
    else
        next_assign = Trav (old_arg_node->node[1]->node[1]->node[0], arg_info);
    arg_info->node[2] = old_info2;
    APPEND_ASSIGNS (first_assign, next_assign);

    /* set first_assign to the last N_assign node, that is generated by Trav() */
    first_assign = LAST_ASSIGN (arg_info);
    while (2 == first_assign->nnode)
        first_assign = first_assign->node[1];

    INSERT_ASSIGN;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompArg
 *  arguments     : 1) N_arg node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->node[0] contains pointer to N_block
 *
 */
node *
CompArg (node *arg_node, node *arg_info)
{
    node *icm_arg, *id_node, *type_id_node, *refcnt_node, *new_assign, *next_arg, *exprs,
      *type_node;
    simpletype s_type;
    int dim = 0;

    DBUG_ENTER ("CompArg");

    next_arg = arg_node->node[0];
    if (T_user != arg_node->SIMPLETYPE) {
        s_type = arg_node->SIMPLETYPE;
        dim = arg_node->DIM;
    } else {
        type_node = LookupType (arg_node->NAME, arg_node->NAME_MOD, 042);
        /* 042 is only a dummy argument */
        /* store basic_type */
        s_type = type_node->SIMPLETYPE;
        dim = type_node->DIM + arg_node->DIM;
    }
    MAKENODE_ID (type_id_node, type_string[s_type]); /* store type of paramter */

    /* store name of formal parrameter */
    if (NULL != arg_node->ID) {
        MAKENODE_ID (id_node, arg_node->ID);
    } else {
        MAKENODE_ID (id_node, (NULL != arg_node->ID) ? arg_node->ID : "");
    }

    arg_node->nodetype = N_exprs;
    exprs = arg_node;
    if (1 == IsArray (arg_node->TYPES)) {
        MAKENODE_ID (exprs->node[0], "in_a");
        /* put ND_ICM_RC at beginning of function block */
        if (1 < arg_node->refcnt) {
            MAKENODE_NUM (refcnt_node, arg_node->refcnt - 1);
            CREATE_2_ARY_ICM (new_assign, "ND_INC_RC", id_node, refcnt_node);
            new_assign->node[1] = arg_info->node[0];
            new_assign->nnode += 1;
            arg_info->node[0] = new_assign;
        } else if (0 == arg_node->refcnt) {
            MAKENODE_NUM (refcnt_node, 1);
            CREATE_2_ARY_ICM (new_assign, "ND_DEC_RC_FREE", id_node, refcnt_node);
            new_assign->node[1] = arg_info->node[0];
            new_assign->nnode += 1;
            arg_info->node[0] = new_assign;
        }
    } else {
        MAKENODE_ID (exprs->node[0], "in");
    }
    MAKE_NEXT_ICM_ARG (exprs, type_id_node);

    MAKE_NEXT_ICM_ARG (exprs, id_node);
    if (NULL != next_arg) {
        exprs->node[1] = Trav (next_arg, arg_info);
        exprs->nnode = 2;
    } else
        exprs->nnode = 1;

    if (dim > 0) { /* put N_icm "ND_KS_DECL_ARRAY_ARG" at beginning of function block */

        node *dim_node, *shape;
        int i;
        /* store dim and shape */
        MAKENODE_NUM (dim_node, dim);
        CREATE_2_ARY_ICM (new_assign, "ND_KS_DECL_ARRAY_ARG", id_node, dim_node);
        for (i = 0; i < arg_node->DIM; i++) {
            MAKENODE_NUM (shape, arg_node->SHP[i]);
            MAKE_NEXT_ICM_ARG (icm_arg, shape);
        }
        if (T_user == arg_node->SIMPLETYPE)
            for (i = 0; i < type_node->DIM; i++) {
                MAKENODE_NUM (shape, type_node->SHP[i]);
                MAKE_NEXT_ICM_ARG (icm_arg, shape);
            }
        /* now put node in at beginning of block of function */
        new_assign->node[1] = arg_info->node[0];
        new_assign->nnode += 1;
        arg_info->node[0] = new_assign;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompFundef
 *  arguments     : 1) N_fundef node
 *                  2) info node
 *  description   : trasverses child-nodes,
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :-sets arg_info->node[0] to first N_assign of function before
 *                 - traversing the function's arguments
 *                 - sets arg_info->node[3] to variable declaration
 *                 - calls Trav to `compile` varbiable declarations
 *
 */
node *
CompFundef (node *arg_node, node *arg_info)
{
    node *return_node, *icm, *icm_arg, *type_id_node, *var_name_node, *n_node,
      *fun_name_node, *tag_node, *tmp, *return_id;
    types *types;
    simpletype s_type;
    int i = 0;

    DBUG_ENTER ("CompFundef");

    if (NULL != arg_node->node[0]) {
        arg_info->node[3] = arg_node->node[0]->node[1]; /* set pointer to vardec */
        arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* body */
        if (NULL != arg_info->node[3]) {
            /* compile vardecs */
            arg_node->node[0]->node[1] = Trav (arg_info->node[3], arg_info);
            arg_node->node[0]->nnode = 2;
        }
    }

    /* now create N_icm ND_FUN_DEC and put it to arg_node->node[3] */
    icm = MakeNode (N_icm);
    MAKE_ICM_NAME (icm, "ND_FUN_DEC");
    MAKENODE_ID (fun_name_node, arg_node->ID);
    MAKE_ICM_ARG (icm->node[0], fun_name_node);
    icm_arg = icm->node[0];
    MAKENODE_NUM (n_node, 0);
    MAKE_NEXT_ICM_ARG (icm_arg, n_node); /* number of following args of N_icm
                                          * the correct value will be set later
                                          */
    types = arg_node->TYPES;
    if (NULL != arg_node->node[3])
        /* arg_node->node[3] points to a N_icm (ND_FUN_RET)
         * return_node will point to the first N_exprs belonging to a return_value
         */
        return_node = arg_node->node[3]->node[0]->node[1]->node[1];

    do {
        if (1 == IsArray (types)) {
            MAKENODE_ID (tag_node, "out_a");
        } else {
            MAKENODE_ID (tag_node, "out");
        }
        MAKE_NEXT_ICM_ARG (icm_arg, tag_node);
        GET_BASIC_SIMPLETYPE (s_type, types);
        MAKENODE_ID (type_id_node, type_string[s_type]);
        MAKE_NEXT_ICM_ARG (icm_arg, type_id_node);
        if (NULL == arg_node->node[0]) {
            /* it is an extern declaration */
#ifdef IMPORTED_WITH_NAME
            MAKENODE_ID (var_name_node, GenName (i, DUMMY_NAME));
#else
            MAKENODE_ID (var_name_node, "");
#endif

        } else {
            return_id = return_node->node[0];
            DBUG_ASSERT (N_id == return_id->nodetype, "wrong nodetype != N_id");
            MAKENODE_ID_REUSE_IDS (var_name_node, return_id->IDS);
            if (2 == return_node->nnode)
                /* put return_node to next N_exprs where a function return_value
                 * is behinde
                 */
                return_node = return_node->node[1]->node[1];
        }
        MAKE_NEXT_ICM_ARG (icm_arg, var_name_node);
        i += 1;
        types = types->next;
    } while (NULL != types);
    arg_node->node[3] = icm; /* put N_icm ND_FUN_DEC to arg_node->node[3] */

    if (NULL != arg_node->node[2]) {
        /* count number of formal parameters */
        tmp = arg_node->node[2]; /* formal parameters */
        do {
            i += 1;
            tmp = tmp->node[0];
        } while (NULL != tmp);

        if (NULL != arg_node->node[0])
            /* first assign of body */
            arg_info->node[0] = arg_node->node[0]->node[0];
        /* traverse formal parameters (N_arg) */
        arg_node->node[2] = Trav (arg_node->node[2], arg_info);
        if (NULL != arg_node->node[0]) {
            /* new first assign of body */
            arg_node->node[0]->node[0] = arg_info->node[0];
            arg_info->node[0] = NULL;
        }

        /* append formal parameters  to N_icm args*/
        icm_arg->node[1] = arg_node->node[2];
        icm_arg->nnode = 2;
    }
    n_node->info.cint = i; /* set number of N_icm args to correct value */

    /* traverse next function if any */
    if (NULL != arg_node->node[1])
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompLoop
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node (use macro LAST_ASSIGN or NEXT_ASSIGN to
 *                     get these nodes)
 *
 *
 */
node *
CompLoop (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *icm_arg, *n_node, *v1, *v2, *label,
      *dummy_assign = NULL, *V1, *V2, *loop_assign;
    int found;

    DBUG_ENTER ("CompLoop");

    /* first compile termination condition and body of loop */
    loop_assign = LAST_ASSIGN (arg_info);
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    arg_node->node[1] = Trav (arg_node->node[1], arg_info);

    /* now add some  DEC_RC at begining of and after the loop */
    dummy_assign = MakeNode (N_assign);
    first_assign = dummy_assign;
    V1 = arg_node->node[2]->node[0];
    V2 = arg_node->node[2]->node[1];
    v1 = V1;
    v2 = V2;
    if ((NULL != v2) && (NULL == v1)) {
        MAKENODE_NUM (n_node, 1);
        while (NULL != v2) {
            DEC_RC_FREE_ND (v2, n_node); /* we don`t know the refcount of v2
                                          * in the current context, so we use
                                          * DEC_RC_FREE_ND
                                          */
            v2 = v2->node[0];
        }
    } else if ((NULL != v2) && (NULL != v1)) {
        node *v1_tmp;
        MAKENODE_NUM (n_node, 1);
        while (NULL != v2) {
            /* looking if v2 is in V2/V1 */
            v1_tmp = v1;
            found = 0;
            while ((0 == found) && (NULL != v1_tmp))
                if (0 == strcmp (v1_tmp->IDS_ID, v2->IDS_ID))
                    found = 1;
                else
                    v1_tmp = v1_tmp->node[0];

            if (0 == found) {
                DEC_RC_FREE_ND (v2, n_node);
            }

            v2 = v2->node[0];
        }
    }

    if (N_do == arg_node->nodetype) {
        label_nr++;
        MAKENODE_ID (label, GenName (label_nr, LABEL_NAME));
        CREATE_1_ARY_ICM (next_assign, "ND_LABEL", label);
        APPEND_ASSIGNS (first_assign, next_assign);
    }
    v1 = V1;
    if (NULL != v1) {
        /* now add some  INC_RC at begining of and after the loop */
        while (NULL != v1) {
            if (1 <= (v1->refcnt - 1)) {
                MAKENODE_NUM (n_node, v1->refcnt - 1);
                INC_RC_ND (v1, n_node);
            }
            v1 = v1->node[0];
        }
    }

    /* now insert INC's and DEC's at beginning of the loop */
    if (NULL != dummy_assign->node[1]) {
        first_assign->node[1] = arg_node->node[1]->node[0];
        first_assign->nnode = 2;
        arg_node->node[1]->node[0] = dummy_assign->node[1];
        dummy_assign->node[1] = NULL;
    }

    /* now create DEC_RC`s that have to be done after termination
     * of the loop
     */
    v1 = V1;
    v2 = V2;
    first_assign = dummy_assign; /* will be used in some macros */
    if (NULL != v1) {
        MAKENODE_NUM (n_node, 1);
        if (NULL != v2) {
            node *v2_tmp;
            while (NULL != v1) {
                /* looking if v1 is in V1/V2 */
                v2_tmp = v2;
                found = 0;
                while ((0 == found) && (NULL != v2_tmp))
                    if (0 == strcmp (v1->IDS_ID, v2_tmp->IDS_ID))
                        found = 1;
                    else
                        v2_tmp = v2_tmp->node[0];

                if (0 == found) {
                    DEC_RC_FREE_ND (v1, n_node);
                }
                v1 = v1->node[0];
            }
        } else
            while (NULL != v1) {
                DEC_OR_FREE_RC_ND (v1, n_node);
                v1 = v1->node[0];
            }
    }
    /* now increase RC of arrays that are defined in the  loop and are
     * used after it.
     */
    v2 = V2;
    if (NULL != v2)
        while (NULL != v2) {
            if (1 < v2->refcnt) {
                MAKENODE_NUM (n_node, v2->refcnt - 1);
                INC_RC_ND (v2, n_node);
            }
            v2 = v2->node[0];
        }

    if (NULL != dummy_assign->node[1]) {
        /* now put dummy_assign->node[1] behind while_loop */
        if (NULL != loop_assign->node[1]) {
            first_assign->node[1] = loop_assign->node[1];
            first_assign->nnode = 2;
        }
        loop_assign->node[1] = dummy_assign->node[1];
    }
    FREE (dummy_assign);

    if (N_do == arg_node->nodetype) {
        /* put N_icm 'ND_GOTO', in front of N_do node */
        CREATE_1_ARY_ICM (first_assign, "ND_GOTO", label);
        if (NULL != loop_assign->node[1]) {
            first_assign->node[1] = loop_assign->node[1]; /* next assign after do-loop */
            first_assign->nnode = 2;
        }
        first_assign->node[2] = loop_assign->node[0]; /* only temporary used (N_do) */
        loop_assign->node[0] = first_assign->node[0]; /* N_icm (ND_GOTO) node */
        loop_assign->node[1] = first_assign;
        loop_assign->nnode = 2;
        arg_node = first_assign->node[0];
        first_assign->node[0] = first_assign->node[2]; /* put N_do node */
        first_assign->node[2] = NULL;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompCond
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *
 *
 */
node *
CompCond (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *icm_arg, *n_node, *id_node, *dummy_assign = NULL;
    int i;

    DBUG_ENTER ("CompCond");

    /* compile condition, then and else part */
    for (i = 0; i < arg_node->nnode; i++)
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);

    /* insert N_icms to correct refcounts of then and else part */
    dummy_assign = MakeNode (N_assign);
    for (i = 0; i < arg_node->nnode; i++)
        if (NULL != arg_node->node[3]->node[i]) {
            id_node = arg_node->node[3]->node[i];
            first_assign = dummy_assign;
            do {
                MAKENODE_NUM (n_node, id_node->refcnt);
                DBUG_PRINT ("COMP", ("%d:create DEC_RC(%s, %d)", i, id_node->IDS_ID,
                                     id_node->refcnt));
                DEC_OR_FREE_RC_ND (id_node, n_node);
                id_node = id_node->node[0];
            } while (NULL != id_node);
            first_assign->node[1] = arg_node->node[i + 1]->node[0];
            first_assign->nnode = 2;
            arg_node->node[i + 1]->node[0] = dummy_assign->node[1];
        }
    FREE (dummy_assign);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompBlock
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   : sets only arg_info->node[0]
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node (use macro LAST_ASSIGN or NEXT_ASSIGN to
 *                     get these nodes)
 *
 */
node *
CompBlock (node *arg_node, node *arg_info)
{
    node *old_info;

    DBUG_ENTER ("CompBlock");
    old_info = arg_info->node[0];
    arg_info->node[0] = arg_node;
    Trav (arg_node->node[0], arg_info);
    arg_info->node[0] = old_info;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompCast
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   : deletes N_cast-node
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *
 */
node *
CompCast (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("CompCast");

    tmp = arg_node;
    arg_node = Trav (arg_node->node[0], arg_info);
    FREE_TYPE (tmp->TYPES);
    FREE (tmp);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompTypedef
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   : transforms N_typedef to N_icm if it is a definition of an
 *                  array
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *
 */
node *
CompTypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CompTypedef");

    if (1 <= arg_node->DIM) {
        char *typename, *new_typename;
        node *type1, *type2, *icm_arg;

        if (NULL != arg_node->NAME_MOD) {
            typename = (char *)Malloc (sizeof (char)
                                       * (strlen (arg_node->NAME) + strlen (MOD_NAME_CON)
                                          + strlen (arg_node->NAME_MOD)));
            sprintf (typename, "%s%s%s", arg_node->NAME_MOD, MOD_NAME_CON,
                     arg_node->NAME);
        } else
            typename = type_string[arg_node->SIMPLETYPE];

        new_typename = StringCopy (arg_node->ID);
        FREE_TYPE (arg_node->TYPES);
        arg_node->nodetype = N_icm;
        MAKE_ICM_NAME (arg_node, "ND_TYPEDEF_ARRAY");
        if (1 == arg_node->nnode)
            arg_node->node[1] = arg_node->node[0];
        MAKENODE_ID (type1, typename);
        MAKE_ICM_ARG (arg_node->node[0], type1);
        icm_arg = arg_node->node[0];
        MAKENODE_ID (type2, new_typename);
        MAKE_NEXT_ICM_ARG (icm_arg, type2);

        if (1 == arg_node->nnode) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
            arg_node->nnode = 2;
        }
    } else if (1 == arg_node->nnode)
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}
