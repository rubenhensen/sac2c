/*
 *
 * $Log$
 * Revision 1.37  1995/06/02 08:43:34  hw
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
#include "traverse.h"
#include "compile.h"
#include "convert.h"
#include "refcount.h"  /* to use IsArray */
#include "typecheck.h" /* to use LookupType */
#include "free.h"

extern int malloc_verify ();
extern int malloc_debug (int level);

#define DUMMY_NAME "__OUT_"
#define LABEL_NAME "__Label" /* basic-name for goto label */

/* the following macros are used to generate N_icms */
#define MAKE_ICM_ARG(var, new_node)                                                      \
    var = MakeNode (N_exprs);                                                            \
    var->node[0] = new_node;                                                             \
    var->nnode = 1

#define MAKE_NEXT_ICM_ARG(prev, new_node)                                                \
    MAKE_ICM_ARG (tmp, new_node);                                                        \
    prev->node[1] = tmp;                                                                 \
    prev->nnode = 2;                                                                     \
    prev = tmp

#define MAKE_ICM_NAME(var, name)                                                         \
    var->info.fun_name.id = name;                                                        \
    var->info.fun_name.id_mod = NULL

#define MAKE_ICM(assign)                                                                 \
    assign = MakeNode (N_assign);                                                        \
    assign->node[0] = MakeNode (N_icm);                                                  \
    assign->nnode = 1

#define MAKE_PREV_ICM(assign, new_assign)                                                \
    new_assign = MakeNode (N_assign);                                                    \
    new_assign->node[0] = MakeNode (N_icm);                                              \
    new_assign->nnode = 1;                                                               \
    new_assign->node[1] = assign;                                                        \
    new_assign->nnode = 2;                                                               \
    assign = new_assign

#define CREATE_1_ARY_ICM(assign, str, arg)                                               \
    MAKE_ICM (assign);                                                                   \
    MAKE_ICM_NAME (assign->node[0], str);                                                \
    MAKE_ICM_ARG (assign->node[0]->node[0], arg);                                        \
    icm_arg = assign->node[0]->node[0];                                                  \
    assign->node[0]->nnode = 1

#define CREATE_2_ARY_ICM(assign, str, arg1, arg2)                                        \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define CREATE_3_ARY_ICM(assign, str, arg1, arg2, arg3)                                  \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3)

#define CREATE_4_ARY_ICM(assign, str, arg1, arg2, arg3, arg4)                            \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg4)

#define CREATE_5_ARY_ICM(assign, str, arg1, arg2, arg3, arg4, arg5)                      \
    CREATE_4_ARY_ICM (assign, str, arg1, arg2, arg3, arg4);                              \
    MAKE_NEXT_ICM_ARG (icm_arg, arg5)

#define CREATE_6_ARY_ICM(assign, str, arg1, arg2, arg3, arg4, arg5, arg6)                \
    CREATE_5_ARY_ICM (assign, str, arg1, arg2, arg3, arg4, arg5);                        \
    MAKE_NEXT_ICM_ARG (icm_arg, arg6)

#define CREATE_7_ARY_ICM(assign, str, arg1, arg2, arg3, arg4, arg5, arg6, arg7)          \
    CREATE_6_ARY_ICM (assign, str, arg1, arg2, arg3, arg4, arg5, arg6);                  \
    MAKE_NEXT_ICM_ARG (icm_arg, arg7)

#define BIN_ICM_REUSE(reuse, str, arg1, arg2)                                            \
    reuse->nodetype = N_icm;                                                             \
    MAKE_ICM_NAME (reuse, str);                                                          \
    MAKE_ICM_ARG (reuse->node[0], arg1);                                                 \
    icm_arg = reuse->node[0];                                                            \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define SET_VARS_FOR_MORE_ICMS                                                           \
    first_assign = LAST_ASSIGN (arg_info);                                               \
    old_arg_node = arg_node;                                                             \
    last_assign = NEXT_ASSIGN (arg_info);                                                \
    arg_node = arg_info->node[1]->node[0]

#define APPEND_ASSIGNS(first, next)                                                      \
    first->node[1] = next;                                                               \
    first->nnode = 2;                                                                    \
    first = next

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

#define CONST_ARRAY(array, rc)                                                           \
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
    CREATE_2_ARY_ICM (next_assign, "ND_REUSE", tmp_array1, res);                         \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    array = tmp_array1 /* set array to __TMP */

#define CREATE_CONST_ARRAY(array, name, type, rc)                                        \
    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type, name, rc);                    \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    COUNT_ELEMS (n_elems, array->node[0]);                                               \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY", name, n_node);               \
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

#define GET_DIM(result, type)                                                            \
    if (T_user == type->simpletype) {                                                    \
        result = LookupType (type->name, type->name_mod, 042)->DIM;                      \
        result += type->dim;                                                             \
    } else                                                                               \
        result = type->dim

#define GET_BASIC_TYPE(res, type)                                                        \
    if (T_user == type->simpletype)                                                      \
        res = LookupType (type->name, type->name_mod, 042)->SIMPLETYPE;                  \
    else                                                                                 \
        res = type->simpletype

#define GET_LENGTH(length, vardec_node)                                                  \
    if (T_user == vardec_node->SIMPLETYPE)                                               \
        length = LookupType (vardec_node->NAME, vardec_node->NAME_MOD, 042)->SHP[0];     \
    else                                                                                 \
        length = vardec_node->SHP[0]

#define COUNT_ELEMS(n, exprs)                                                            \
    n = 0;                                                                               \
    tmp = exprs;                                                                         \
    do {                                                                                 \
        n += 1;                                                                          \
        tmp = tmp->node[1];                                                              \
    } while (NULL != tmp)

#define MAKENODE_NUM(no, nr)                                                             \
    no = MakeNode (N_num);                                                               \
    no->info.cint = nr

#define MAKENODE_ID(no, str)                                                             \
    no = MakeNode (N_id);                                                                \
    no->IDS = MakeIds (str)

#define MAKENODE_ID_REUSE_IDS(no, Ids)                                                   \
    no = MakeNode (N_id);                                                                \
    no->IDS = Ids

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

#define FREE(a)                                                                          \
    DBUG_PRINT ("FREE", (P_FORMAT, a));                                                  \
    free (a)

#define FREE_TYPE(a)                                                                     \
    if (NULL != a->shpseg) {                                                             \
        FREE (a->shpseg);                                                                \
    }                                                                                    \
    FREE (a)

#define FREE_VARDEC(a)                                                                   \
    FREE_TYPE (a->TYPES);                                                                \
    FREE (a)

#define FREE_TREE(a) FreeTree (a)

#define FREE_IDS(a)                                                                      \
    DBUG_PRINT ("FREE", (P_FORMAT, a));                                                  \
    FreeIds (a)

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
RenameVar (char *string)
{
    char *new_name;
    DBUG_ENTER ("RenameVar");

    new_name = (char *)Malloc (sizeof (char) * (strlen (string) + 3));
    sprintf (new_name, "__%s", string);

    DBUG_RETURN (new_name);
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
    node *info;
    DBUG_ENTER ("Compile");

#ifdef MALLOC_TOOL
    malloc_debug (2);
#endif /* MALLOC_TOOL */

    act_tab = comp_tab; /* set new function-table for traverse */
    info = MakeNode (N_info);
    if (N_modul == arg_node->nodetype) {
        if (NULL != arg_node->node[1])
            /* traverse typedefs */
            arg_node->node[1] = Trav (arg_node->node[1], info);
        if (NULL != arg_node->node[2])
            /* traverse functions */
            arg_node->node[2] = Trav (arg_node->node[2], info);
    } else {
        DBUG_ASSERT ((N_fundef == arg_node->nodetype), "wrong node");
        arg_node = Trav (arg_node, info);
    }

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
 *  macros        : DBUG...,
 *  remarks       : ----
 *
 */
node *
CompVardec (node *arg_node, node *arg_info)
{
    node *exprs, *tmp, *type_node, *assign;
    int i, dim;

    DBUG_ENTER ("CompVardec");

    if (1 == IsArray (arg_node->TYPES)) {
        MAKE_ICM (assign);
        MAKE_ICM_NAME (assign->node[0], "ND_KS_DECL_ARRAY");

        /* store basic_type */
        MAKE_ICM_ARG (exprs, MakeNode (N_id));
        if (NULL != arg_node->node[0]) {
            assign->node[1] = arg_node->node[0];
            assign->node[0]->node[0] = exprs;
            assign->nnode = 2;
        } else
            assign->node[0]->node[0] = exprs;

        if (T_user != arg_node->SIMPLETYPE) {
            exprs->node[0]->IDS = MakeIds (type_string[arg_node->SIMPLETYPE]);
            arg_node->node[1] = exprs;

            dim = arg_node->DIM;
        } else {
            type_node = LookupType (arg_node->NAME, arg_node->NAME_MOD, 042);
            /* 042 is only a dummy argument */

            /* store basic_type */
            exprs->node[0]->IDS = MakeIds (type_string[type_node->SIMPLETYPE]);
            arg_node->node[1] = exprs;

            dim = type_node->DIM + arg_node->DIM;
        }

        /* store name of variable */
        MAKE_NEXT_ICM_ARG (exprs, MakeNode (N_id));
        tmp->node[0]->info.ids = MakeIds (arg_node->ID);

        /* store dimension */
        MAKE_NEXT_ICM_ARG (exprs, MakeNode (N_num));
        tmp->node[0]->info.cint = dim;

        /* store shape infomation */
        for (i = 0; i < arg_node->DIM; i++) {
            MAKE_NEXT_ICM_ARG (exprs, MakeNode (N_num));
            tmp->node[0]->info.cint = arg_node->SHP[i];
        }
        if (T_user == arg_node->SIMPLETYPE)
            for (i = 0; i < type_node->DIM; i++) {
                MAKE_NEXT_ICM_ARG (exprs, MakeNode (N_num));
                tmp->node[0]->info.cint = type_node->SHP[i];
            }

        /* now transform current node to one of type N_icm */
        FREE_VARDEC (arg_node);
        arg_node = assign;
        if (NULL != arg_node->node[1])
            arg_node->node[1] = Trav (arg_node->node[1], NULL);

    } else
      /* traverse next N_vardec node if any */
      if (NULL != arg_node->node[0])
        arg_node->node[0] = Trav (arg_node->node[0], NULL);

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
 *  macros        : DBUG...,
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
      *dim_node, *tmp_rc;
    simpletype s_type;
    int is_SxA = 0, n_elems = 0, is_drop = 0, array_is_const = 0, dim;

    DBUG_ENTER ("CompPrf");

    DBUG_PRINT ("COMP", ("%s line: %d", mdb_prf[arg_node->info.prf], arg_node->lineno));

    /* NOTE :  F_neq should be the last "function enumerator" that hasn't
     *         arrays as arguments.
     */
    if (arg_node->info.prf > F_neq) {
        node *exprs = arg_node->node[0];
        ids *let_ids = arg_info->IDS;
        node *new_name, *vardec_p, *new_assign, *new_vardec, *old_name;
        int insert_vardec = 0, insert_assign = 0;

        while (NULL != exprs) {
            if (N_id == exprs->node[0]->nodetype)
                if (0 == strcmp (let_ids->id, exprs->node[0]->IDS_ID)) {
                    if (0 == insert_assign) {
                        MAKENODE_ID (new_name, RenameVar (let_ids->id));
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
                        if (1 == insert_vardec) {
                            new_vardec = MakeNode (N_vardec);
                            new_vardec->TYPES = DuplicateTypes (vardec_p->TYPES);
                            new_vardec->ID = RenameVar (let_ids->id);
                            new_vardec->node[0] = vardec_p->node[0];
                            vardec_p->node[0] = new_vardec;
                        }
                    }

                    /* now rename N_id */
                    FREE (exprs->node[0]->IDS_ID);
                    exprs->node[0]->IDS_ID = RenameVar (let_ids->id);
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
            GET_BASIC_TYPE (s_type, arg_info->IDS_NODE->TYPES);
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
            GET_BASIC_TYPE (s_type, arg_info->IDS_NODE->TYPES);
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
#if 0
            CREATE_2_ARY_ICM(next_assign, "ND_DEC_RC", arg1, n_node);
            APPEND_ASSIGNS(first_assign, next_assign);
            CREATE_2_ARY_ICM(next_assign, "ND_DEC_RC", arg2, n_node);
            APPEND_ASSIGNS(first_assign, next_assign);
#endif
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
             */
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            DBUG_ASSERT ((N_array == arg1->nodetype),
                         "first argument of take/drop isn't an array");

            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* compute basic_type of result */
            GET_BASIC_TYPE (s_type, arg_info->IDS_NODE->TYPES);
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
                CONST_ARRAY (arg2, res_ref);
            }

            exprs = arg1->node[0];
            n_elems = 0;
            do {
                n_elems += 1;
                exprs = exprs->node[1];
            } while (NULL != exprs);

            MAKENODE_NUM (n_node, n_elems);
            if (1 == is_drop) {
                CREATE_4_ARY_ICM (next_assign, "ND_KD_DROP_CxA_A", dim_node, arg2, res,
                                  n_node);
            } else {
                CREATE_4_ARY_ICM (next_assign, "ND_KD_TAKE_CxA_A", dim_node, arg2, res,
                                  n_node);
            }
            icm_arg->node[1] = arg1->node[0];
            icm_arg->nnode = 2;
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
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            last_assign = NEXT_ASSIGN (arg_info);

            /* compute basic type */
            GET_BASIC_TYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* compute length of arg1 */
            if (N_id == arg1->nodetype) {
                GET_LENGTH (n_elems, arg1->IDS_NODE);
                MAKENODE_NUM (n_node, n_elems);
            } else {
                n_elems = 0;
                tmp = arg1->node[0];
                do {
                    n_elems += 1;
                    tmp = tmp->node[1];
                } while (NULL != tmp);
                MAKENODE_NUM (n_node, n_elems);
            }

            if (0 == IsArray (res->IDS_NODE->TYPES)) {
                if (N_id == arg2->nodetype) {
                    GET_DIM (dim, arg2->IDS_NODE->TYPES);
                    MAKENODE_NUM (dim_node, dim); /* store dimension of array */
                    if (N_id == arg1->nodetype) {
                        BIN_ICM_REUSE (arg_info->node[1], "ND_KD_PSI_VxA_S", arg2, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, n_node);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                    } else {
                        BIN_ICM_REUSE (arg_info->node[1], "ND_KD_PSI_CxA_S", arg2, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, n_node);
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
                    MAKENODE_NUM (dim_node, 1);
                    /* arg2 is a constant array */
                    DBUG_ASSERT ((N_array == arg2->nodetype), "nodetype != N_array");
                    MAKENODE_NUM (tmp_rc, 0);
                    CONST_ARRAY (arg2, tmp_rc);
                    if (N_id == arg1->nodetype) {
                        CREATE_4_ARY_ICM (next_assign, "ND_KD_PSI_VxA_S", arg2, res,
                                          n_node, arg1);
                    } else {
                        CREATE_3_ARY_ICM (next_assign, "ND_KD_PSI_CxA_S", arg2, res,
                                          n_node);
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
                    CREATE_5_ARY_ICM (next_assign, "ND_KD_PSI_VxA_A", dim_node, arg2, res,
                                      n_node, arg1);
                } else {
                    CREATE_4_ARY_ICM (next_assign, "ND_KD_PSI_CxA_A", dim_node, arg2, res,
                                      n_node);
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
            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY", res, length_node);
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
            GET_BASIC_TYPE (s_type, arg_info->IDS_NODE->TYPES);
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
            GET_BASIC_TYPE (s_type, arg_info->IDS_NODE->TYPES);
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
        default:
            /*   DBUG_ASSERT(0,"wrong prf"); */
            break;
        }
    } else {
        node *exprs = arg_node->node[0];
        do {
            if (N_cast == exprs->node[0]->nodetype) {
                tmp = exprs->node[0]->node[0];
                FREE_TYPE (exprs->node[0]->TYPES);
                FREE (exprs->node[0]);
                exprs->node[0] = tmp;
            }
            exprs = exprs->node[1];
        } while (NULL != exprs);
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
    node *tmp, *first_assign, *next_assign, *exprs, *res, *type_id_node, *old_arg_node,
      *icm_arg, *n_node, *res_ref, *last_assign;
    simpletype s_type;
    int n_elems = 0;

    DBUG_ENTER ("CompArray");

    /* store next assign */
    last_assign = NEXT_ASSIGN (arg_info);

    /* compute basic_type of result */
    GET_BASIC_TYPE (s_type, arg_info->IDS_NODE->TYPES);
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
    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY", res, n_node);
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
    node *first_assign, *next_assign, *tmp, *last_assign, *old_arg_node, *icm_arg, *res,
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
    MAKENODE_ID (id_node, GenFunName (arg_node->FUN_NAME, arg_node->FUN_MOD_NAME));
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
 *  functionname  : CompReturn
 *  arguments     : 1) N_return node
 *                  2) info node
 *  description   : generates N_icms for N_return of a function or of a
 *                  with-loop
 *  global vars   :
 *  internal funs :
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
    node *tmp, *next, *exprs, *last, *n_node, *next_assign, *index_node, *first_assign,
      *from, *to;

    DBUG_ENTER ("CompReturn");

    exprs = arg_node->node[0];
    if (NULL == arg_info->node[2]) {
        /* this is a N_return of a function-body */
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
        exprs->node[1] = arg_node->node[1];
        exprs->nnode = 2;
        arg_node->node[0] = exprs;
        arg_node->node[1] = NULL;
    } else {
        /* this is a N_return of a with_loop.
         * arg_info->node[2] points to the N_icm that desribes the begin of
         * a with_loop.
         * The arguments of this N_icm are usefull to get the name of
         * the resulting array, the dimension of this array, and the
         * length of the index_vector.
         * The name of the N_icm is used to detect whether N_return node is part
         * of a 'genarray' or 'modarray' with-loop.
         */
        node *ret_val, *with_icm_arg, *icm_arg, *index_length, *tmp_with_icm_arg, *dec_rc,
          *res, *res_dim, *mod_array = NULL;
        int is_array = IsArray (exprs->node[0]->IDS_NODE->TYPES);

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
        if ('F' == arg_info->node[2]->info.id[9])
            tmp_with_icm_arg = tmp_with_icm_arg->node[1];

        /* now we store some information:
         * from        : N_id ( the left border of a mod/genarray)
         * to          : N_id ( the right  border of a mod/genarray)
         * index_node  : N_id ( name of index-vector of mod/genarray)
         * index_length: N_num ( length of index-vector )
         *
         * all informations are shared with belonging mod/genarray N_icm
         */
        if ('M' == arg_info->node[2]->info.id[9]) {
            mod_array = tmp_with_icm_arg->node[0];
            tmp_with_icm_arg = tmp_with_icm_arg->node[1];
        }

        from = tmp_with_icm_arg->node[0];
        DBUG_ASSERT (N_id == from->nodetype, " wrong nodetype for 'form'");
        tmp_with_icm_arg = tmp_with_icm_arg->node[1];
        to = tmp_with_icm_arg->node[0];
        DBUG_ASSERT (N_id == to->nodetype, " wrong nodetype for 'to'");
        tmp_with_icm_arg = tmp_with_icm_arg->node[1];
        index_node = tmp_with_icm_arg->node[0];
        tmp_with_icm_arg = tmp_with_icm_arg->node[1];
        DBUG_ASSERT (N_id == index_node->nodetype, " wrong nodetype for 'index_node'");
        index_length = tmp_with_icm_arg->node[0];
        DBUG_ASSERT (N_num == index_length->nodetype,
                     " wrong nodetype for 'index_length'");

        if ('M' == arg_info->node[2]->info.id[9]) {

            MAKE_ICM_ARG (arg_node->node[0], res);
            icm_arg = arg_node->node[0];
            MAKE_NEXT_ICM_ARG (icm_arg, res_dim);
            /* N_return belongs to a 'modarray' with-loop */
            /* add name of modified array */
#if 0
         with_icm_arg=with_icm_arg->node[1];
         MAKE_NEXT_ICM_ARG(icm_arg, with_icm_arg->node[0]);
#endif
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
        } else if ('G' == arg_info->node[2]->info.id[9]) {

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
        } else if ('F' == arg_info->node[2]->info.id[9]) {
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
        if ('F' != arg_info->node[2]->info.id[9]) {
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
        if (NULL != tmp_with_icm_arg->node[1]) {
            /* return contains to N_foldfun or N_foldprf, so the refcount of the
             * result is stored in the last_argument of the BEGIN N_icm.
             * It will be used for the END N_icm and removed form BEGIN N_icm
             */
            INC_RC_ND (res, tmp_with_icm_arg->node[1]->node[0]);
            tmp_with_icm_arg->nnode = 1;
            FREE (tmp_with_icm_arg->node[1]);
        }
    }

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
    node *old_info2, *first_assign, *next_assign, *tmp, *n_node, *inc_rc, *icm_arg, *from,
      *to, *type_id_node, *arg, *res, *res_ref, *res_dim_node, *index, *indexlen,
      *old_arg_node, *last_assign;
    int res_dim;
    simpletype s_type;

    DBUG_ENTER ("CompWith");

    /* store arg_info->node[2] */
    old_info2 = arg_info->node[2];

    /* store res as N_id */
    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

    /* compute basic_type of result */
    GET_BASIC_TYPE (s_type, arg_info->IDS_NODE->TYPES);
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* compute dimension of res */
    GET_DIM (res_dim, arg_info->IDS_NODE->TYPES);
    MAKENODE_NUM (res_dim_node, res_dim);

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
        node *prf_node, *neutral_node;
        char *prf_str = prf_string[old_arg_node->node[1]->info.prf];

        /* store neutral element of fold-prf-function in neutral_node */
        if (('+' == prf_str[0]) || ('-' == prf_str[0])) {
            MAKENODE_NUM (neutral_node, 0);
        } else {
            MAKENODE_NUM (neutral_node, 1);
        }

        CREATE_7_ARY_ICM (next_assign, "ND_BEGIN_FOLDPRF", res, res_dim_node,
                          neutral_node, from, to, index, indexlen);
        res_ref->info.cint -= 1;
        if (0 < res_ref->info.cint) {
            /* append res_ref to N_icm temporarily. It will be used and
             * removed in CompReturn.
             */
            MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
        }

        /* store pointer to N_icm ND_BEGIN.. in arg_info->node[2] */
        arg_info->node[2] = next_assign->node[0];
        /* store pointer to variables that have to be increased in
         * in arg_info->node[2]->node[3] ( it will be used in CompReturn )
         */
        arg_info->node[2]->node[3] = old_arg_node->node[2];

        prf_node = MakeNode (N_prf);
        prf_node->info.prf = old_arg_node->node[1]->info.prf;
        /* Store  primitive function (as N_prf) in node[2] of current N_icm.
         * It will be used in CompReturn and than eliminated.
         */
        arg_info->node[2]->node[2] = prf_node;
        APPEND_ASSIGNS (first_assign, next_assign);
    } else if (N_foldfun == old_arg_node->node[1]->nodetype) {
        node *fun_node;

        /* old_arg_node->node[1]->node[1] is the neutral element of
         * fold-function
         */

        CREATE_7_ARY_ICM (next_assign, "ND_BEGIN_FOLDFUN", res, res_dim_node,
                          old_arg_node->node[1]->node[1], from, to, index, indexlen);
        res_ref->info.cint -= 1;
        if (0 < res_ref->info.cint) {
            /* append res_ref to N_icm temporarily. It will be used and
             * removed in CompReturn.
             */
            MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
        }

        /* store pointer to N_icm ND_BEGIN.. in arg_info->node[2] */
        arg_info->node[2] = next_assign->node[0];

        /* store pointer to variables that have to be increased in
         * in arg_info->node[2]->node[3] (it will be used in CompReturn)
         */
        arg_info->node[2]->node[3] = old_arg_node->node[2];

        /* Store  function  (as N_ap) in node[2] of current N_icm.
         * It will be used in CompReturn and than eliminated.
         */
        fun_node = MakeNode (N_ap);
        fun_node->FUN_NAME = old_arg_node->node[1]->FUN_NAME;
        fun_node->FUN_MOD_NAME = old_arg_node->node[1]->FUN_MOD_NAME;
        arg_info->node[2]->node[2] = fun_node;

        APPEND_ASSIGNS (first_assign, next_assign);
    } else {
        DBUG_ASSERT (0, "foldfun not implemented yet");
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
    node *icm_arg, *id_node, *tmp, *type_id_node, *refcnt_node, *new_assign, *next_arg,
      *exprs, *type_node;
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
    MAKENODE_ID (id_node, arg_node->ID);             /* store name of formal parrameter */
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
 *  remarks       : sets arg_info->node[0] to first N_assign of function before
 *                  traversing the function's arguments
 *
 *
 */
node *
CompFundef (node *arg_node, node *arg_info)
{
    node *return_node, *icm, *icm_arg, *type_id_node, *var_name_node, *n_node,
      *fun_name_node, *tag_node, *tmp;
    types *types;
    simpletype s_type;
    int i = 0;

    DBUG_ENTER ("CompFundef");

    /* now create N_icm ND_FUN_DEC and put it to arg_node->node[3] */
    icm = MakeNode (N_icm);
    MAKE_ICM_NAME (icm, "ND_FUN_DEC");
    MAKENODE_ID (fun_name_node, GenFunName (arg_node->ID, arg_node->ID_MOD));
    MAKE_ICM_ARG (icm->node[0], fun_name_node);
    icm_arg = icm->node[0];
    MAKENODE_NUM (n_node, 0);
    MAKE_NEXT_ICM_ARG (icm_arg, n_node); /* number of following args of N_icm
                                          * the correct value will be set later
                                          */
    types = arg_node->TYPES;
    if (NULL != arg_node->node[3])
        return_node = arg_node->node[3]->node[0];

    do {
        if (1 == IsArray (types)) {
            MAKENODE_ID (tag_node, "out_a");
        } else {
            MAKENODE_ID (tag_node, "out");
        }
        MAKE_NEXT_ICM_ARG (icm_arg, tag_node);
        GET_BASIC_TYPE (s_type, types);
        MAKENODE_ID (type_id_node, type_string[s_type]);
        MAKE_NEXT_ICM_ARG (icm_arg, type_id_node);
        if (NULL == arg_node->node[0]) {
            /* it is an extern declaration */
            MAKENODE_ID (var_name_node, GenName (i, DUMMY_NAME));
        } else {
            MAKENODE_ID_REUSE_IDS (var_name_node, return_node->node[0]->IDS);
            return_node = return_node->node[1];
        }
        MAKE_NEXT_ICM_ARG (icm_arg, var_name_node);
        i += 1;
        types = types->next;
    } while (NULL != types);
    arg_node->node[3] = icm; /* put N_icm ND_FUN_DEC to arg_node->node[3] */

    if (NULL != arg_node->node[0])
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    if (NULL != arg_node->node[2]) {
        /* count number of arguments */
        tmp = arg_node->node[2];
        do {
            i += 1;
            tmp = tmp->node[0];
        } while (NULL != tmp);

        if (NULL != arg_node->node[0])
            /* first assign of body */
            arg_info->node[0] = arg_node->node[0]->node[0];

        arg_node->node[2] = Trav (arg_node->node[2], arg_info);
        if (NULL != arg_node->node[0]) {
            /* new first assign of body */
            arg_node->node[0]->node[0] = arg_info->node[0];
            arg_info->node[0] = NULL;
        }
    }

    if (NULL != arg_node->node[2]) {
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
    node *first_assign, *next_assign, *icm_arg, *n_node, *tmp, *v1, *v2, *label,
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
        first_assign->node[1] = loop_assign->node[1];
        first_assign->nnode = 2;
        loop_assign->node[1] = dummy_assign->node[1];
    }
    FREE (dummy_assign);

    if (N_do == arg_node->nodetype) {
        /* put N_icm 'ND_GOTO', in front of N_do node */
        CREATE_1_ARY_ICM (first_assign, "ND_GOTO", label);
        first_assign->node[1] = loop_assign->node[1];
        first_assign->nnode = 2;
        first_assign->node[2] = loop_assign->node[0]; /* only temporary used */
        loop_assign->node[0] = first_assign->node[0];
        loop_assign->node[1] = first_assign;
        arg_node = first_assign->node[0];
        first_assign->node[0] = first_assign->node[2];
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
    node *first_assign, *next_assign, *icm_arg, *n_node, *tmp, *id_node,
      *dummy_assign = NULL;
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
 *                  arg_node->node[0] will be set to arg_node while traversing
 *                  the body of the block
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
    if (2 == arg_node->nnode)
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);

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
