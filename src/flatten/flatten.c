/*
 *
 * $Log$
 * Revision 2.25  2000/10/09 19:19:21  dkr
 * a = prf( a) is flattened now, too
 *
 * Revision 2.24  2000/09/27 16:47:52  dkr
 * a = fun( a) is flattened now
 *
 * Revision 2.23  2000/06/23 13:53:39  dkr
 * nodetype N_with removed
 *
 * Revision 2.22  2000/05/30 12:35:26  dkr
 * functions for old with-loop removed
 *
 * Revision 2.21  2000/05/17 15:14:23  dkr
 * Use of INFO_FLTN_LASTASSIGN and INFO_FLTN_FINALASSIGN in loops
 * corrected. (Especially in case of empty loop bodies.)
 *
 * Function FltnCon removed (old with-loop only).
 *
 * Revision 2.20  2000/02/23 20:16:34  cg
 * Node status ST_imported replaced by ST_imported_mod and
 * ST_imported_class in order to allow distinction between enteties
 * that are imported from a module and those that are imported from a
 * class.
 *
 * Revision 2.19  1999/06/02 09:46:31  jhs
 * Fixed a bug in FltnArray occuring when a constant array was nested
 * somehow in a non-constant one (e.g. in a primitive function) and the
 * values needed for infering were overwritten.
 *
 * Revision 2.18  1999/05/14 10:36:53  jhs
 * Corrected little Bug in FltnNwithop, Bug was inserted while changeing
 * Annotation to AnnotateIdWithConstVec.
 *
 * Revision 2.17  1999/05/14 09:25:13  jhs
 * Dbugged constvec annotations and their housekeeping in various compilation stages.
 *
 * Revision 2.16  1999/05/12 15:30:16  jhs
 * Annotate... added; const-annotation dbugged.
 *
 * Revision 2.15  1999/05/12 08:42:26  sbs
 * brushed handling of CONSTVECs; adjusted to new macros.
 *
 * Revision 2.14  1999/05/11 16:18:27  jhs
 * Done4 some cosmetics to learn about the code.
 *
 * Revision 2.13  1999/05/06 12:08:50  sbs
 * includes brushed!.
 *
 * Revision 2.12  1999/04/20 12:58:24  jhs
 * Changes made for emty arrays.
 *
 * Revision 2.11  1999/04/08 17:10:21  jhs
 * FltnArray expanded for EmptyArrays.
 *
 * Revision 2.10  1999/03/19 09:40:19  bs
 * PreTypecheck mutated to the global function FltnPreTypecheck
 *
 * Revision 2.9  1999/03/17 21:44:22  bs
 * Flattening of costant boolean arrays and constant character arrays!
 *
 * Revision 2.8  1999/03/17 15:35:04  bs
 * Bug fixed in FltnArray and FltnExprs. Macro EXPR_VAL added.
 *
 * Revision 2.7  1999/03/15 14:13:37  bs
 * Access macros renamed (take a look at tree_basic.h).
 * FltnArray and FltnExprs modified: Now compact propagation of float and double
 * vectors were stored additionally.
 *
 * Revision 2.6  1999/03/09 10:44:07  bs
 * DbugPrintArray removed. This debugging-information will be printed from print.c .
 *
 * Revision 2.5  1999/03/05 17:35:13  bs
 * Another bug fixed in DbugPrintArray.
 *
 * Revision 2.4  1999/03/05 17:20:36  bs
 * Bug fixed in DbugPrintArray.
 *
 * Revision 2.3  1999/02/26 10:53:04  bs
 * FltnPrf() modified: the primitive functions take, drop, reshape and genarray
 * will be flattened like the other primitive functions.
 *
 * Revision 2.2  1999/02/24 20:24:09  bs
 * Function CopyIntArray moved to internal_lib.c .
 *
 * Revision 2.1  1999/02/23 12:39:08  sacbase
 * new release made
 *
 * ... [eliminated] ...
 *
 * Revision 1.2  1994/11/10  15:39:42  sbs
 * RCS-header inserted
 *
 */

#include <stdio.h>

#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "types.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"

#include "flatten.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 */

/*
 * arg_info in this file:
 * CONTEXT    : context of this arg_node, e.g. within a condition. FltnExprs
 *              decides to abstract or not abstract the node.
 *              legal values are (enum type contextflag):
 *                CT_normal,
 *                CT_ap,
 *                CT_return,
 *
 * LASTASSIGN:  Every FltnAssign replaces node[0] with arg_node so that other
 * (node[0])    functions may place instructions IN FRONT of that assignment.
 *              FltnAssign returns this node[0]
 *
 * LASTWLBLOCK: This node is only used in the context of WLs. It is necessary
 * (node[1])    to put assignments (var initialisations) at the beginning of
 *              the WL-body. These assignments are stored here. See comment in
 *              FltnNcode.
 *
 * FINALASSIGN: Every FltnBlock resets node[2] to NULL.
 * (node[2])    Every FltnAssign replaces node[2] with arg_node if
 *              ASSIGN_NEXT(arg_node) equals NULL.
 */

#define P_FORMAT "(%06x)" /* formatstring for pointer address */
#define WITH_PREFIX "__w" /* name of new variable in with-statement */
#define WITH_PREFIX_LENGTH 3

#define STACK_SIZE 1000

#define PUSH(old, new, n)                                                                \
    if (tos < stack_limit) {                                                             \
        tos->id_old = old;                                                               \
        tos->id_new = new;                                                               \
        tos->w_level = n;                                                                \
        tos++;                                                                           \
        DBUG_EXECUTE ("RENAME", DbugPrintStack (););                                     \
    } else                                                                               \
        ERROR2 (1, (" stack overflow (local)"))

#define PUSH_ENTRY(ptr) PUSH (ptr.id_old, ptr.id_new, ptr.w_level)

#define EXPR_VAL(node)                                                                   \
    (NODE_TYPE (node) == N_double)                                                       \
      ? (DOUBLE_VAL (node))                                                              \
      : ((NODE_TYPE (node) == N_float)                                                   \
           ? (FLOAT_VAL (node))                                                          \
           : ((NODE_TYPE (node) == N_num)                                                \
                ? (NUM_VAL (node))                                                       \
                : ((NODE_TYPE (node) == N_bool) ? (BOOL_VAL (node))                      \
                                                : (CHAR_VAL (node)))))

typedef struct LOCAL_STACK {
    char *id_old;
    char *id_new;
    int w_level;
} local_stack;

static int with_level = 0;
static local_stack *tos, *stack, *stack_limit;

/******************************************************************************
 *
 * function:
 *   void DbugPrintStack(void)
 *
 * description:
 *   prints stack top-down from tos.
 *
 *
 ******************************************************************************/

static void
DbugPrintStack (void)
{
    local_stack *tmp;

    tmp = tos;
    while (tmp-- > stack)
        printf ("%p : %s -> %s on level %d\n", tmp, tmp->id_old, tmp->id_new,
                tmp->w_level);
}

/******************************************************************************
 *
 * function:
 *   simpletype PreTypecheck(nodetype act_node_t, simpletype elem_type)
 *
 * description:
 *   Checks whether the type of act_node_t is compatible (or can be made
 *   compatible) to the type of the other elements (elem_type).
 *   If so, the type is returned, otherwise T_unknown will be returned.
 *   Note here, that T_nothing indicates that this is the first element
 *   of the array to be checked (cf. FltnArray).
 *
 ******************************************************************************/

static simpletype
FltnPreTypecheck (nodetype act_node_t, simpletype elem_type)
{
    DBUG_ENTER ("PreTypecheck");

    switch (act_node_t) {
    case N_num:
        if ((elem_type == T_int) || (elem_type == T_nothing)) {
            elem_type = T_int;
            break;
        }
    case N_float:
        if ((elem_type == T_float) || (elem_type == T_int) || (elem_type == T_nothing)) {
            elem_type = T_float;
            break;
        }
    case N_double:
        if ((elem_type == T_double) || (elem_type == T_float) || (elem_type == T_int)
            || (elem_type == T_nothing)) {
            elem_type = T_double;
        } else {
            elem_type = T_unknown;
        }
        break;

    case N_bool:
        if ((elem_type == T_bool) || (elem_type == T_nothing)) {
            elem_type = T_bool;
        } else {
            elem_type = T_unknown;
        }
        break;
    case N_char:
        if ((elem_type == T_char) || (elem_type == T_nothing)) {
            elem_type = T_char;
        } else {
            elem_type = T_unknown;
        }
        break;
    default:
        elem_type = T_unknown;
    }

    DBUG_RETURN (elem_type);
}

/******************************************************************************
 *
 * function:
 *  char *RenameWithVar(char *name, int level)
 *
 * description:
 *   - creates a new string with a prefix:
 *     name == a , level == 7      =>    result == <WITH_PREFIX>7_a
 *
 ******************************************************************************/

static char *
RenameWithVar (char *name, int level)
{
    char *string;
    DBUG_ENTER ("RenameWithVar");

    string = (char *)Malloc (sizeof (char) * (strlen (name) + WITH_PREFIX_LENGTH + 6));
    /*
     * 6 will be added, because 'level' and `_` will be part of
     * new name
     */
    sprintf (string, WITH_PREFIX "%d_%s", level, name);

    DBUG_RETURN (string);
}

/******************************************************************************
 *
 * function:
 *  void InsertRenaming(node *block_or_assign, char *new_name, char *old_name)
 *
 * description:
 *   - inserts an assigment of the kind:
 *       new_name = old_name;
 *     after the node pointed at by block_or_assign.
 *     As the parameter name suggests, this node may either be a block
 *     node (needed for the renamings at the beginning of the WL bodies)
 *     or an assign node (needed for renaming the LHS of a WL).
 *   - returns a pointer to the freshly generated N_assign node.
 *
 ******************************************************************************/

static node *
InsertRenaming (node *block_or_assign, char *new_name, char *old_name)
{
    node **insert_at;

    DBUG_ENTER ("InsertRenaming");

    DBUG_ASSERT ((block_or_assign != NULL), "1st arg of InsertRenaming is NULL!");
    DBUG_ASSERT (((NODE_TYPE (block_or_assign) == N_block)
                  || (NODE_TYPE (block_or_assign) == N_assign)),
                 "1st arg of InsertRenaming is neither an N_block nor"
                 "an N_assign node!!");

    /*
     * First insert_at is set to the address of the son node of block_or_assign
     * where the new assignment will be inserted. This has to be done since
     * we otherwise would have to use differen Access-Macros for creating and
     * inserting the new assignment!
     */
    if (NODE_TYPE (block_or_assign) == N_block) {
        insert_at = &BLOCK_INSTR (block_or_assign);
    } else {
        insert_at = &ASSIGN_NEXT (block_or_assign);
    }

    *insert_at = MakeAssign (MakeLet (MakeId (StringCopy (old_name), NULL, ST_regular),
                                      MakeIds (StringCopy (new_name), NULL, ST_regular)),
                             *insert_at);

    DBUG_PRINT ("RENAME", ("inserted %0x between %0x and %0x", *insert_at,
                           block_or_assign, ASSIGN_NEXT ((*insert_at))));

    DBUG_RETURN (*insert_at);
}

/******************************************************************************
 *
 * function:
 *  local_stack *FindId(char *name)
 *
 * description:
 *   - searches name on the stack
 *   - returns the repective pointer iff succesfull otherwise it returns NULL
 *
 ******************************************************************************/

static local_stack *
FindId (char *name)
{
    local_stack *tmp;

    DBUG_ENTER ("FindId");

    tmp = tos - 1;
    while ((tmp >= stack) && (strcmp (tmp->id_old, name) != 0)) {
        tmp--;
    }

    if (tmp < stack) {
        tmp = NULL;
        DBUG_PRINT ("RENAME", ("var %s not found!", name));
    } else {
        DBUG_PRINT ("RENAME", ("found:" P_FORMAT "old: %s, new: %s, id_level: %d ,"
                               " with_level: %d",
                               tmp, tmp->id_old, tmp->id_new, tmp->w_level, with_level));
    }

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *  local_stack *FindIdInSeg(char *name, int seg_sz, local_stack *seg)
 *
 * description:
 *   - searches name in the given stack segment
 *   - returns the repective pointer iff succesfull otherwise it returns NULL
 *
 ******************************************************************************/

static local_stack *
FindIdInSeg (char *name, int seg_sz, local_stack *seg)
{
    local_stack *tmp;

    DBUG_ENTER ("FindIdInSeg");

    tmp = &seg[seg_sz - 1];
    while ((tmp >= seg) && (strcmp (tmp->id_old, name) != 0)) {
        tmp--;
    }

    if (tmp < seg) {
        tmp = NULL;
        DBUG_PRINT ("RENAME", ("var %s not found!", name));
    } else {
        DBUG_PRINT ("RENAME", ("found:" P_FORMAT "old: %s, new: %s, id_level: %d ,"
                               " with_level: %d",
                               tmp, tmp->id_old, tmp->id_new, tmp->w_level, with_level));
    }

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *  local_stack *CopyStackSeg( local_stack *first_elem, local_stack *last_elem)
 *
 * description:
 *   - allocates a local_stack of size (last_elem-first_elem) and copies
 *     all entries from first_elem to (last_elem-1) into that segment.
 *   - returns a pointer to the allocated segment.
 *
 ******************************************************************************/

static local_stack *
CopyStackSeg (local_stack *first_elem, local_stack *last_elem)
{
    local_stack *seg;
    int i, sz;

    DBUG_ENTER ("CopyStackSeg");

    sz = last_elem - first_elem;
    seg = (local_stack *)Malloc (sizeof (local_stack) * sz);

    for (i = 0; i < sz; i++) {
        seg[i].id_old = first_elem[i].id_old;
        seg[i].id_new = first_elem[i].id_new;
        seg[i].w_level = first_elem[i].w_level;
    }

    DBUG_RETURN (seg);
}

/******************************************************************************
 *
 * function:
 *  node *Abstract(node *arg_node, *arg_info)
 *
 * description:
 *   - gets an expression <expr> to be abstracted out as argument
 *   - creates an assignment of the form:
 *        __flat_<n> = <expr>;
 *     end prepands it to LASTASSIGN from arg_info
 *   - returns a freshly created N_id node holding  __flat_<n>
 *
 ******************************************************************************/

static node *
Abstract (node *arg_node, node *arg_info)
{
    char *tmp;
    node *res;

    DBUG_ENTER ("Abstract");

    tmp = TmpVar ();
    INFO_FLTN_LASTASSIGN (arg_info)
      = MakeAssign (MakeLet (arg_node, MakeIds (tmp, NULL, ST_regular)),
                    INFO_FLTN_LASTASSIGN (arg_info));

    DBUG_PRINT ("FLATTEN",
                ("node %08x inserted before %08x", INFO_FLTN_LASTASSIGN (arg_info),
                 ASSIGN_NEXT (INFO_FLTN_LASTASSIGN (arg_info))));

    res = MakeId (StringCopy (tmp), NULL, ST_regular);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node *Flatten(node *arg_node)
 *
 * description:
 *   eliminates nested function applications:
 *
 *     a = f( a+b, c);         =>   __flat_<n> = a+b;
 *                                  a          = f( __flat_<n>, c);
 *
 *   and renames vars that occur on the LHS as well as in argument position
 *   of a function application:
 *
 *     a, b, a = f( a, b);     =>   __flat_<n>, __flat_<m>, __flat_<n> = f( a, b);
 *                                  a = __flat_<n>;
 *                                  b = __flat_<m>;
 *
 *   and renames vars that are defined by a with-loop AND have been defined
 *   before that with-loop:
 *     ...                          ...
 *     a = ...                      a = ...
 *     ...                     =>   ...
 *     a = with ... a ...           __flat_<n> = with ... a ...
 *                                  a = __flat_<n>;
 *
 *   and renames vars that have been defined before a with-loop AND
 *   are redefined in the with-loop:
 *     ...                          ...
 *     a = ...                      a = ...
 *     ...                          ...
 *     b = with(...) {              b = with(...) {
 *                                        __w<i>a = a;
 *           ... a ...         =>         ... __w<i>a ...
 *           a = ...                      __w<i>a = ...
 *           ... a ...                    ... __w<i>a ...
 *         } ...                        } ...
 *
 *   and gives the generator-variables of WLs unique names:
 *
 *     iv = ...                      iv = ...
 *     ...                           ...
 *     a = with(... iv ...)    =>    a = with(... __flat<n>iv ...)
 *           v = ... iv ...                __w<i>iv = ... __flat<n>iv ...
 *
 *  NOTE: For the old WL, this renaming only occurs, if iv has been previously
 *  defined. In the new WL, this is done always!
 *
 ******************************************************************************/

node *
Flatten (node *arg_node)
{
    node *info_node;

    DBUG_ENTER ("Flatten");

    /*
     * initialize the static variables :
     */
    stack = (local_stack *)Malloc (sizeof (local_stack) * STACK_SIZE);
    stack_limit = STACK_SIZE + stack;
    tos = stack;

    with_level = 0;

    /*
     * traverse the syntax tree:
     */
    act_tab = flat_tab;
    info_node = MakeInfo ();
    INFO_FLTN_CONTEXT (info_node) = CT_normal;

    arg_node = Trav (arg_node, info_node);

    FreeInfo (info_node, NULL);

    /*
     * de-initialize the static variables :
     */
    FREE (stack);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnModul(node *arg_node, node *arg_info)
 *
 * description:
 *   this function is needed to limit the traversal to the FUNS-son of
 *   N_modul!
 *
 ******************************************************************************/

node *
FltnModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FltnModul");

    if (MODUL_FUNS (arg_node)) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   - calls Trav to flatten the user defined functions if function body is not
 *   empty and resets tos after flatten of the function's body.
 *   - the formal parameters of a function will be traversed to put their names
 *   on the stack!
 *
 ******************************************************************************/

node *
FltnFundef (node *arg_node, node *arg_info)
{
    local_stack *tmp_tos;

    DBUG_ENTER ("FltnFundef");

    tmp_tos = tos; /* store tos */

    /*
     * Do not flatten imported functions. These functions have already been
     * flattened and if this is done again there may arise name clashes.
     * A new temp variable __flat42 may conflict with __flat42 which was
     * inserted in the first flatten phase (module compiliation).
     */
    if ((FUNDEF_BODY (arg_node) != NULL) && (ST_imported_mod != FUNDEF_STATUS (arg_node))
        && (ST_imported_class != FUNDEF_STATUS (arg_node))) {
        DBUG_PRINT ("FLATTEN", ("flattening function %s:", FUNDEF_NAME (arg_node)));
        if (FUNDEF_ARGS (arg_node)) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }
    tos = tmp_tos; /* restore tos */

    /*
     * Proceed with the next function...
     */
    if (FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnArgs(node *arg_node, node *arg_info)
 *
 * description:
 *   - adds names of formal parameters to the stack
 *
 ******************************************************************************/

node *
FltnArgs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FltnArgs");

    PUSH (ARG_NAME (arg_node), ARG_NAME (arg_node), with_level);

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnBlock(node *arg_node, node *arg_info)
 *
 * description:
 *   - if CONTEXT is CT_wl arg_node is inserted in LASTWLBLOCK and after
 *     traversing the body LASTWLBLOCK -> NEXT is set to the result of the
 *     traversal. This ensures that flattening and renaming do not interfere!
 *   - resets FINALASSIGN to NULL.
 *
 ******************************************************************************/

node *
FltnBlock (node *arg_node, node *arg_info)
{
    node *assigns, *mem_last_wlblock;

    DBUG_ENTER ("FltnBlock");

    INFO_FLTN_FINALASSIGN (arg_info) = NULL;
    if (BLOCK_INSTR (arg_node) != NULL) {
        if (INFO_FLTN_CONTEXT (arg_info) == CT_wl) {
            /*
             * First, we reset INFO_FLTN_CONTEXT( arg_info) to CT_normal!
             * This is essential since otherwise non-WL-blocks within a
             * WL would penetrate INFO_FLTN_LASTWLBLOCK( arg_info)!!
             */
            INFO_FLTN_CONTEXT (arg_info) = CT_normal;
            mem_last_wlblock = INFO_FLTN_LASTWLBLOCK (arg_info);
            INFO_FLTN_LASTWLBLOCK (arg_info) = arg_node;
            DBUG_PRINT ("RENAME", ("LASTWLBLOCK set to %08x", arg_node));
            assigns = Trav (BLOCK_INSTR (arg_node), arg_info);
            if (NODE_TYPE (INFO_FLTN_LASTWLBLOCK (arg_info)) == N_block) {
                BLOCK_INSTR (INFO_FLTN_LASTWLBLOCK (arg_info)) = assigns;
            } else {
                DBUG_ASSERT ((NODE_TYPE (INFO_FLTN_LASTWLBLOCK (arg_info)) == N_assign),
                             ("LASTWLBLOCK in flatten does neither point to"
                              " an N_block nor to an N_assign node !"));
                ASSIGN_NEXT (INFO_FLTN_LASTWLBLOCK (arg_info)) = assigns;
            }
            DBUG_PRINT ("RENAME", ("connecting %08x to %08x!",
                                   INFO_FLTN_LASTWLBLOCK (arg_info), assigns));
            INFO_FLTN_LASTWLBLOCK (arg_info) = mem_last_wlblock;
        } else {
            BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnAssign(node *arg_node, node *arg_info)
 *
 * description:
 *   - stores arg_node in INFO_FLTN_LASTASSIGN( arg_info)
 *   - stores arg_node in INFO_FLTN_FINALASSIGN( arg_info)
 *   iff (ASSIGN_NEXT == NULL)
 *   - returns the modified INFO_FLTN_LASTASSIGN( arg_info) yielded
 *   by traversing the instruction so that newly created abstractions
 *   will automatically be inserted by the calling function!
 *
 ******************************************************************************/

node *
FltnAssign (node *arg_node, node *arg_info)
{
    node *return_node;

    DBUG_ENTER ("FltnAssign");

    INFO_FLTN_LASTASSIGN (arg_info) = arg_node;
    DBUG_PRINT ("FLATTEN", ("LASTASSIGN set to %08x!", arg_node));

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_FLTN_LASTASSIGN(arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_FLTN_LASTASSIGN (arg_info);

    if (return_node != arg_node) {
        DBUG_PRINT ("FLATTEN", ("node %08x will be inserted instead of %08x", return_node,
                                arg_node));
    }

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        INFO_FLTN_FINALASSIGN (arg_info) = arg_node;
        DBUG_PRINT ("FLATTEN", ("FINALASSIGN set to %08x!", arg_node));
    }

    DBUG_RETURN (return_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnLet(node *arg_node, node *arg_info)
 *
 * description:
 *   BEFORE the traversal of the RHS (since the vars on the RHS should still
 *   have the original names)
 *   for each id from the LHS we do:
 *     - if we have   a ... a = fun/prf( ... a ... )
 *       then we rename each LHS   a   into a temp-var   __flat<n>   and insert
 *       an assignment of the form   a = __flat<n>;   after the function
 *       application.
 *       NOTE HERE, that we neither have to push  __flat<n>  nor we have to
 *       take care of any required renamings of   a   as indicated by <x> and
 *       <wl> since that new assignment will be traversed later and thus a
 *       required renaming will take place in that invocation of FltnLet !!
 *
 *   AFTER the traversal of the RHS (since the vars on the LHS should not be on
 *   the stack during that traversal)
 *   for each id from the LHS we do:
 *     - if we have   a = with ...   AND it is absolutely sure
 *       that a has been defined before, i.e.,
 *         if a has been defined within a conditional, it has been defined
 *         in both branches or if a has been defined in a while-loop, then
 *         it has been defined before as well
 *         <=>  there exists an entry [a, <x>, <wl>] on the stack,
 *       then we rename   a   into a temp-var   __flat<n>   and insert an
 *       assignment of the form   a = __flat<n>;   after the with-loop.
 *       NOTE HERE, that we neither have to push  __flat<n>  nor we have to
 *       take care of any required renamings of   a   as indicated by <x> and
 *       <wl> since that new assignment will be traversed later and thus a
 *       required renaming will take place in that invocation of FltnLet !!
 *     - if this LET-node is within a with-loop AND it is absolutely
 *       sure that   a   has been defined before, i.e.,
 *         there exists an entry [a, <x>, <wl>] on the stack,
 *       then we have to make sure, that   a   will be renamed!
 *       - if this is the first assignment to   a   in the actual with-loop,
 *           i.e., <wl> < with_level
 *         then we have to rename it to   __w<with_level>a,
 *         insert an assignment of the form:
 *           __w<with_level>a = <x>;
 *         at the beginning of the with-loop (this can be done via
 *         INFO_FLTN_WL_BLOCK( arg_info)),
 *         and push [a, __w<with_level>a, <with_level>] on the stack.
 *       - otherwise, this is  NOT the first assignment to   a   in the
 *         actual with-loop, i.e.,
 *           <wl> == with_level,
 *         then we simply rename it to   __w<with_level>a.
 *     - in all other cases we simply push [a, a, with_level] on the stack
 *
 ******************************************************************************/

node *
FltnLet (node *arg_node, node *arg_info)
{
    ids *ids, *ids2;
    char *var_name, *tmp_var;
    local_stack *tmp;
    node *mem_last_assign, *arg, *id;

    DBUG_ENTER ("FltnLet");

    mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);
    ids = LET_IDS (arg_node);

    if (ids != NULL) {
        DBUG_PRINT ("RENAME",
                    ("checking LHS of let-assignment to %s for renaming (part 1)",
                     IDS_NAME (ids)));
    }

    while (ids != NULL) {
        var_name = IDS_NAME (ids);

        /*
         * Note here, that it is strongly recommended to flat BOTH primitive and
         * user-defined functions!
         * Flattening one class of functions only, e.g. flattening user-defined
         * functions but not-flattening prfs, will probably lead to nice errors
         * after type-checking  =:-<
         *
         * Example:
         *   mytype[] modarray( mytype[] a, int[] iv, mytype val) { ... }
         *   ...
         *   mytype[] a;
         *   ...
         *   a = modarray( a, iv, val)
         * Here the modarray is parsed as a prf but in fact it is a user-defined
         * function!!! That means, the application is a N_prf node after scan/parse
         * and will therefore be left untouched during the flattening phase.
         * The type-checker infers that this application has to be a N_ap node
         * and transform it accordingly.
         * Now, we have a N_ap node that NOT has been flattened correctly ....
         */
        if ((NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
            || (NODE_TYPE (LET_EXPR (arg_node)) == N_ap)) {
            /*
             * does 'var_name' occur as an argument of the function application??
             */
            arg = AP_OR_PRF_ARGS (LET_EXPR (arg_node));
            while (arg != NULL) {
                id = EXPRS_EXPR (arg);
                if (NODE_TYPE (id) == N_id) {
                    if (!strcmp (ID_NAME (id), var_name)) {
                        DBUG_PRINT ("RENAME",
                                    ("renaming LHS (%s) of function application",
                                     var_name));
                        tmp_var = TmpVar ();
                        /*
                         * Now, we insert an assignment    var_name = tmp_var;   AFTER
                         * the actual assignment! This id done
                         * by using InsertRenaming with a pointer to the actual
                         * assignment-node which can be found in mem_last_assign!!!
                         */
                        mem_last_assign
                          = InsertRenaming (mem_last_assign, var_name, tmp_var);

                        /*
                         * rename all other occurences of 'var_name' on the LHS
                         */
                        ids2 = IDS_NEXT (ids);
                        while (ids2 != NULL) {
                            if (!strcmp (IDS_NAME (ids2), var_name)) {
                                FREE (IDS_NAME (ids2));
                                IDS_NAME (ids2) = StringCopy (tmp_var);
                            }
                            ids2 = IDS_NEXT (ids2);
                        }

                        /*
                         * and we rename the actual LHS
                         */
                        FREE (var_name);
                        IDS_NAME (ids) = tmp_var;
                        break;
                    }
                }
                arg = EXPRS_NEXT (arg);
            }
        }
        ids = IDS_NEXT (ids);
    }

    ids = LET_IDS (arg_node);

    if (ids != NULL) {
        DBUG_PRINT ("FLATTEN",
                    ("flattening RHS of let-assignment to %s", IDS_NAME (ids)));
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (ids != NULL) {
        DBUG_PRINT ("RENAME",
                    ("checking LHS of let-assignment to %s for renaming (part 2)",
                     IDS_NAME (ids)));
    }

    while (ids != NULL) {
        var_name = IDS_NAME (ids);
        tmp = FindId (var_name);

        if (tmp == 0) {
            PUSH (var_name, var_name, with_level);
            /*
             * this PUSH operation covers the whole "third case" from the description
             * above. if var_name is found in the stack and non of the other cases
             * is given, then (with_level == 0) holds and [var_name, var_name, 0]
             * has been pushed anyway!!
             */
        } else {
            if (NODE_TYPE (LET_EXPR (arg_node)) == N_Nwith) { /* the let expr is a WL */
                DBUG_PRINT ("RENAME", ("renaming LHS of WL-assignment"));
                tmp_var = TmpVar ();
                /*
                 * Now, we insert an assignment    var_name = tmp_var;   AFTER
                 * the actual assignment! This id done
                 * by using InsertRenaming with a pointer to the actual
                 * assignment-node which can be found in mem_last_assign!!!
                 */
                mem_last_assign = InsertRenaming (mem_last_assign, var_name, tmp_var);
                /*
                 * and we rename the actual LHS:
                 */
                FREE (var_name);
                IDS_NAME (ids) = tmp_var;
            } else {                  /* the let expr is not a WL */
                if (with_level > 0) { /* we are in the body of a WL */
                    if (tmp->w_level
                        < with_level) { /* var_name has not yet been renamed */
                        DBUG_PRINT ("RENAME", ("inserting new renaming - assignment at "
                                               "the beginning of the WL"));
                        tmp_var = RenameWithVar (var_name, with_level);
                        PUSH (tmp->id_old, tmp_var, with_level);
                        INFO_FLTN_LASTWLBLOCK (arg_info)
                          = InsertRenaming (INFO_FLTN_LASTWLBLOCK (arg_info), tmp_var,
                                            tmp->id_new);
                    } else {
                        tmp_var = StringCopy (tmp->id_new);
                    }
                    FREE (var_name);
                    IDS_NAME (ids) = tmp_var;
                }
            }
        }
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnArray(node *arg_node, node *arg_info)
 *
 * description:
 *  set the context-flag of arg_info to CT_array, traverse the arguments,
 *  in the case of constant integers store the array in compact form
 *  additionally, and finally restore the old context-flag.
 *
 ******************************************************************************/

node *
FltnArray (node *arg_node, node *arg_info)
{
    contextflag old_context;
    int old_isconst;
    simpletype old_vectype;
    int old_veclen;
    void *old_constvec;

    DBUG_ENTER ("FltnArray");

    /*
     * if we are dealing with an empty array, things are easy: we do not have
     * to use the info-node mechanism to find out whether it is constant....
     * Instead, we can directly set the attributes correctly.
     * Only ARRAY_VECTYPE is not known here; therefore, it is set to the
     * dummy value T_nothing. This will be straightened during type checking.
     * T_nothin on the right hand side has to be casted or an arror will occur.
     */
    if (ARRAY_AELEMS (arg_node) == NULL) {
        ARRAY_ISCONST (arg_node) = TRUE;
        ARRAY_VECTYPE (arg_node) = T_nothing;
        ARRAY_VECLEN (arg_node) = 0;
        ARRAY_CONSTVEC (arg_node) = NULL;
    } else {
        /*
         *  The array is not empty; so we have to traverse it and - while
         *  doing so - collect infos whether the array is constant in the info_node.
         *
         *  During the following traversal some values of arg_info will be changed,
         *  so we need to save the actual values here, so they can be restored later.
         */
        old_context = INFO_FLTN_CONTEXT (arg_info);
        old_isconst = INFO_FLTN_ISCONST (arg_info);
        old_vectype = INFO_FLTN_VECTYPE (arg_info);
        old_veclen = INFO_FLTN_VECLEN (arg_info);
        old_constvec = INFO_FLTN_CONSTVEC (arg_info);

        INFO_FLTN_CONTEXT (arg_info) = CT_array;
        INFO_FLTN_ISCONST (arg_info) = FALSE;
        INFO_FLTN_VECTYPE (arg_info) = T_nothing; /* T_unknown would indicate
                                                   * non-constant components!
                                                   */
        INFO_FLTN_VECLEN (arg_info) = 0;
        INFO_FLTN_CONSTVEC (arg_info) = NULL;

        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);

        ARRAY_ISCONST (arg_node) = INFO_FLTN_ISCONST (arg_info);
        ARRAY_VECLEN (arg_node) = INFO_FLTN_VECLEN (arg_info);
        ARRAY_VECTYPE (arg_node) = INFO_FLTN_VECTYPE (arg_info);
        ARRAY_CONSTVEC (arg_node) = INFO_FLTN_CONSTVEC (arg_info);

        /*
         *  As mentioned above, the values are restored now.
         */
        INFO_FLTN_CONTEXT (arg_info) = old_context;
        INFO_FLTN_ISCONST (arg_info) = old_isconst;
        INFO_FLTN_VECTYPE (arg_info) = old_vectype;
        INFO_FLTN_VECLEN (arg_info) = old_veclen;
        INFO_FLTN_CONSTVEC (arg_info) = old_constvec;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnAp(node *arg_node, node *arg_info)
 *
 * description:
 *  if the application has some arguments, set the context-flag of
 *  arg_info to CT_ap, traverse the arguments, and finally restore
 *  the old context-flag.
 *
 ******************************************************************************/

node *
FltnAp (node *arg_node, node *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ("FltnAp");

    DBUG_PRINT ("FLATTEN", ("flattening application of %s:", AP_NAME (arg_node)));

    if (AP_ARGS (arg_node) != NULL) {
        old_ctxt = INFO_FLTN_CONTEXT (arg_info);
        INFO_FLTN_CONTEXT (arg_info) = CT_ap;
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
        INFO_FLTN_CONTEXT (arg_info) = old_ctxt;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnPrf(node *arg_node, node *arg_info)
 *
 * description:
 *  - If the application has some arguments, set the context-flag of
 *  arg_info either to CT_ap or to CT_normal, traverse the arguments, and
 *  finally restore the old context-flag.
 *  - The context-flag is set to CT_normal iff we want arrays NOT to be abstracted
 *  out!!! This is needed only for the typechecker when he wants to infer the
 *  exact shapes for applications of functions such as TAKE, etc.
 *
 ******************************************************************************/

node *
FltnPrf (node *arg_node, node *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ("FltnPrf");

    DBUG_PRINT ("FLATTEN",
                ("flattening application of %s:", mdb_prf[PRF_PRF (arg_node)]));

    if (PRF_ARGS (arg_node) != NULL) {
        old_ctxt = INFO_FLTN_CONTEXT (arg_info);
        INFO_FLTN_CONTEXT (arg_info) = CT_ap;
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        INFO_FLTN_CONTEXT (arg_info) = old_ctxt;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnReturn(node *arg_node, node *arg_info)
 *
 * description:
 *  if the function returns values, set the context-flag of
 *  arg_info to CT_return, traverse the results, and finally restore
 *  the old context-flag.
 *
 ******************************************************************************/

node *
FltnReturn (node *arg_node, node *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ("FltnReturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        old_ctxt = INFO_FLTN_CONTEXT (arg_info);
        INFO_FLTN_CONTEXT (arg_info) = CT_return;
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
        INFO_FLTN_CONTEXT (arg_info) = old_ctxt;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnExprs(node *arg_node, node *arg_info)
 *
 * description:
 *  - flattens all the exprs depending on the context INFO_FLTN_CONTEXT( arg_info)
 *  - collecting constant integer elements
 *
 ******************************************************************************/

node *
FltnExprs (node *arg_node, node *arg_info)
{
    int info_fltn_array_index, abstract;
    node *expr, *expr2, *casts_expr;

    DBUG_ENTER ("FltnExprs");

    info_fltn_array_index = INFO_FLTN_VECLEN (arg_info);

    /*  an expression can be hidden behind some cast, for programmer's convenience
     *  we hold both: the expression with the casts (casts_expr) and the real one
     *  behind the casts without the casts (expr).
     *  So there will be no traversal on the N_cast nodes.
     *
     *  if there is no cast => (casts_expr == expr)
     */
    casts_expr = EXPRS_EXPR (arg_node);
    expr = casts_expr;
    while (NODE_TYPE (expr) == N_cast)
        expr = CAST_EXPR (expr);

    /*
     * compute whether to abstract <expr> or not , depending on the
     * context of the expression, given by INFO_FLTN_CONTEXT( arg_info)
     */
    switch (INFO_FLTN_CONTEXT (arg_info)) {
    case CT_return:
        abstract = ((NODE_TYPE (expr) == N_num) || (NODE_TYPE (expr) == N_float)
                    || (NODE_TYPE (expr) == N_double) || (NODE_TYPE (expr) == N_bool)
                    || (NODE_TYPE (expr) == N_char) || (NODE_TYPE (expr) == N_str)
                    || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_ap)
                    || (NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_Nwith));
        break;
    case CT_ap:
        abstract = ((NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_ap) || (NODE_TYPE (expr) == N_Nwith));
        break;
    case CT_normal:
        abstract = ((NODE_TYPE (expr) == N_ap) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_Nwith));
        break;
    case CT_array:
        abstract = ((NODE_TYPE (expr) == N_ap) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_Nwith));
        INFO_FLTN_VECTYPE (arg_info)
          = FltnPreTypecheck (NODE_TYPE (expr), INFO_FLTN_VECTYPE (arg_info));
        INFO_FLTN_VECLEN (arg_info) = info_fltn_array_index + 1;
        break;
    default:
        DBUG_ASSERT (0, "illegal context !");
        /* the following assignment is used only for convincing the C compiler
         * that abstract will be initialized in any case!
         */
        abstract = 0;
    }

    DBUG_PRINT ("FLATTEN",
                ("context: %d, abstract: %d, expr: %s", INFO_FLTN_CONTEXT (arg_info),
                 abstract, mdb_nodetype[NODE_TYPE (expr)]));

    /*
     * if this is to be abstracted, we abstract and potentially annotate constant
     * arrays in the freshly generated N_id node.
     */
    if (abstract) {
        /*
         *  if there are type casts we need to abstract them too.
         *  if we leave them, empty arrays will be left uncasted and thus untyped.
         */
        EXPRS_EXPR (arg_node) = Abstract (casts_expr, arg_info);
        expr2 = Trav (expr, arg_info);
        AnnotateIdWithConstVec (expr, EXPRS_EXPR (arg_node));

    } else {
        expr2 = Trav (expr, arg_info);
    }

    DBUG_ASSERT ((expr == expr2),
                 "return-node differs from arg_node while flattening an expr!");

    /*
     * Last but not least remaining exprs have to be done:
     */
    if (EXPRS_NEXT (arg_node) == NULL) {
        /*
         * iff we are within an array, we can now decide whether the array
         * is constant; if so, we allocate a constvec for collecting
         * the constant values and insert the last one.
         */
        if ((INFO_FLTN_CONTEXT (arg_info) == CT_array)
            && (INFO_FLTN_VECTYPE (arg_info) != T_unknown)) {
            INFO_FLTN_ISCONST (arg_info) = TRUE;
            INFO_FLTN_CONSTVEC (arg_info)
              = AllocConstVec (INFO_FLTN_VECTYPE (arg_info), INFO_FLTN_VECLEN (arg_info));
            INFO_FLTN_CONSTVEC (arg_info)
              = ModConstVec (INFO_FLTN_VECTYPE (arg_info), INFO_FLTN_CONSTVEC (arg_info),
                             info_fltn_array_index, expr);
        }
    } else {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);

        /*
         * iff we are within a constant array, insert the actual value in
         * the constvec.
         */
        if ((INFO_FLTN_CONTEXT (arg_info) == CT_array)
            && (INFO_FLTN_VECTYPE (arg_info) != T_unknown)) {
            INFO_FLTN_CONSTVEC (arg_info)
              = ModConstVec (INFO_FLTN_VECTYPE (arg_info), INFO_FLTN_CONSTVEC (arg_info),
                             info_fltn_array_index, expr);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnId(node *arg_node, node *arg_info)
 *
 * description:
 *   - this function is only used for renaming of variables within a WL
 *   - it does so, iff the identifier is found on the stack AND the
 *     variable has to be renamed, i.e. (tmp->id_new != tmp->id_old)
 *
 ******************************************************************************/

node *
FltnId (node *arg_node, node *arg_info)
{
    char *old_name;
    local_stack *tmp;

    DBUG_ENTER ("FltnId");

    if (0 < with_level) {
        tmp = FindId (ID_NAME (arg_node));
        if (tmp) {
            DBUG_ASSERT ((with_level >= tmp->w_level), "actual with-level is smaller "
                                                       "than with-level pushed on the "
                                                       "stack!");
            if (tmp->id_new != tmp->id_old) {
                old_name = ID_NAME (arg_node);
                ID_NAME (arg_node) = StringCopy (tmp->id_new);

                FREE (old_name);
            }
        } else
            DBUG_PRINT ("RENAME", ("not found: %s", ID_NAME (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnCond(node *arg_node, node *arg_info)
 *
 * description:
 *   - flattens the predicate and both alternatives.
 *   - after doing the first alternative all those vars that have been
 *     pushed on the stack during that traversal are copied into a freshly
 *     allocated stack segment (-> then_stack_seg). Only those elems which
 *     where pushed due to a renaming are pushed back to the global stack
 *     since they are inserted before the conditional.
 *     In a similar way, the vars pushed during the traversal of the second
 *     alternative are handled. The only difference being that vars that are
 *     local in both branches, i.e., they are in then_stack_seg and in
 *     else_stack_seg, are pushed as well!
 *   - This approach makes sure that only those vars will remain on the
 *     stack that are either defined before the conditional or in both
 *     branches of the conditional. As a consequence, there will be no
 *     more renamings that lead to errors during type-checking!
 *
 ******************************************************************************/

node *
FltnCond (node *arg_node, node *arg_info)
{
    int then_stack_sz, else_stack_sz, i;
    local_stack *mem_tos, *then_stack_seg, *else_stack_seg;
    node *pred, *pred2, *mem_last_assign;

    DBUG_ENTER ("FltnCond");

    pred = COND_COND (arg_node);
    if ((NODE_TYPE (pred) == N_ap) || (NODE_TYPE (pred) == N_prf)) {
        COND_COND (arg_node) = Abstract (pred, arg_info);
    }

    pred2 = Trav (pred, arg_info);
    DBUG_ASSERT ((pred == pred2),
                 "return-node differs from arg_node while flattening an expr!");

    mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);

    mem_tos = tos;
    if (COND_THEN (arg_node)) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }
    then_stack_sz = tos - mem_tos;
    then_stack_seg = CopyStackSeg (mem_tos, tos);
    tos = mem_tos;

    for (i = 0; i < then_stack_sz; i++) {
        if (then_stack_seg[i].id_new != then_stack_seg[i].id_old) {
            /*
             * if those two pointers (!) are different, we do have a
             * variable renaming here which has happend in this with-level
             * and thus has been inserted BEFORE the conditional.
             * Hence, the renamed variable has to be available in the
             * else-part!
             */
            PUSH_ENTRY (then_stack_seg[i]);
        }
    }

    mem_tos = tos;
    if (COND_ELSE (arg_node)) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }
    else_stack_sz = tos - mem_tos;
    else_stack_seg = CopyStackSeg (mem_tos, tos);
    tos = mem_tos;

    for (i = 0; i < else_stack_sz; i++) {
        if ((FindIdInSeg (else_stack_seg[i].id_old, then_stack_sz, then_stack_seg)
             != NULL)
            || (else_stack_seg[i].id_new != else_stack_seg[i].id_old)) {
            /*
             * for the same reason as for the then-part, renamed vars have to be
             * pushed AND those vars that have been defined locally in BOTH
             * branches of the conditional have to be pushed as well!
             */
            PUSH_ENTRY (else_stack_seg[i]);
        }
    }

    FREE (then_stack_seg);
    FREE (else_stack_seg);

    INFO_FLTN_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnWhile(node *arg_node, node *arg_info)
 *
 * description:
 *  - traverse the body
 *  - leave only those new entries on the stack that have been
 *    renamings since the renamings are inserted before the while-loop!!
 *  - if the predicate has to be flattened out, duplicate the condition
 *    and insert the new assignment at the end of the while-loop body and
 *    before the while-loop.
 *  - Anyway, invoke flatten on the condition(s) => renaming!
 *
 ******************************************************************************/

node *
FltnWhile (node *arg_node, node *arg_info)
{
    node *mem_last_assign, *pred, *pred2, *new_assign, *final_assign;
    local_stack *mem_tos, *body_stack_seg;
    int i, body_stack_sz;

    DBUG_ENTER ("FltnWhile");

    mem_tos = tos;
    mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);

    /*
     * First, we traverse the body of the while-loop.
     * This guarantees that INFO_FLTN_FINALASSIGN( arg_info)
     * will be set to the last N_assign in the body of the loop
     * which may be required for inserting assignments that
     * flatten the break condition!
     */
    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);
    final_assign = INFO_FLTN_FINALASSIGN (arg_info);

    body_stack_sz = tos - mem_tos;
    body_stack_seg = CopyStackSeg (mem_tos, tos);
    tos = mem_tos;

    for (i = 0; i < body_stack_sz; i++) {
        if (body_stack_seg[i].id_new != body_stack_seg[i].id_old) {
            /*
             * if those two pointers (!) are different, we do have a
             * variable renaming here which has happend in this with-level
             * and thus has been inserted BEFORE the while-loop.
             * Hence, the renamed variable has to be available after
             * the loop irrespective of the result of the first evaluation
             * of the break-condition!
             */
            PUSH_ENTRY (body_stack_seg[i]);
        }
    }

    FREE (body_stack_seg);
    INFO_FLTN_LASTASSIGN (arg_info) = mem_last_assign;

    /*
     * Now, we take care of the predicate of the loop:
     *   INFO_FLTN_LASTASSIGN( arg_info) points to the actual N_assign,
     *   INFO_FLTN_FINALASSIGN( arg_info) points to the last N_assign in
     *     the while-loop body!
     */

    pred = WHILE_COND (arg_node);
    if ((NODE_TYPE (pred) == N_ap) || (NODE_TYPE (pred) == N_prf)) {
        /*
         * abstract the condition out and insert it before the while-loop:
         */
        WHILE_COND (arg_node) = Abstract (pred, arg_info);

        /*
         * Duplicate the new N_assign node, flatten it, and insert the
         * chain of assignments after INFO_FLTN_FINALASSIGN( arg_info):
         */
        new_assign = DupNode (INFO_FLTN_LASTASSIGN (arg_info));
        DBUG_PRINT ("FLATTEN", ("duplicated %08x to %08x!",
                                INFO_FLTN_LASTASSIGN (arg_info), new_assign));
        ASSIGN_NEXT (new_assign) = NULL;

        mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);
        new_assign = Trav (new_assign, arg_info);

        if (final_assign == NULL) {
            DBUG_ASSERT ((NODE_TYPE (WHILE_INSTR (arg_node)) == N_empty),
                         "INFO_FLTN_FINALASSIGN is NULL although while-body is "
                         "non-empty");
            /*
             * loop-body ist empty so far!
             */
            WHILE_INSTR (arg_node) = FreeTree (WHILE_INSTR (arg_node));
            WHILE_INSTR (arg_node) = new_assign;
        } else {
            ASSIGN_NEXT (final_assign) = new_assign;
        }
        DBUG_PRINT ("FLATTEN", ("appending %08x to %08x!", new_assign, final_assign));
        INFO_FLTN_LASTASSIGN (arg_info) = mem_last_assign;
    }

    /*
     * Whether abstracted out or not, do flatten the condition:
     */
    pred2 = Trav (pred, arg_info);
    DBUG_ASSERT ((pred == pred2),
                 "return-node differs from arg_node while flattening an expr!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnDo(node *arg_node, node *arg_info)
 *
 * description:
 *  - traverse the body
 *  - leave all entries on the stack since the loop will be executed
 *    at least once!
 *  - if the predicate has to be flattened out, insert the new assignment
 *    at the end of the do-loop body.
 *  - Anyway, invoke flatten on the condition => renaming!
 *
 ******************************************************************************/

node *
FltnDo (node *arg_node, node *arg_info)
{
    node *mem_last_assign, *final_assign, *pred, *pred2;

    DBUG_ENTER ("FltnDo");

    mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);

    /*
     * First, we traverse the body of the while-loop.
     * This guarantees that INFO_FLTN_FINALASSIGN( arg_info)
     * will be set to the last N_assign in the body of the loop
     * which may be required for inserting assignments that
     * flatten the break condition!
     */
    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);
    final_assign = INFO_FLTN_FINALASSIGN (arg_info);

    pred = DO_COND (arg_node);
    if ((NODE_TYPE (pred) == N_ap) || (NODE_TYPE (pred) == N_prf)) {
        /*
         * abstract the condition out and insert it at the end of the do-loop:
         */
        INFO_FLTN_LASTASSIGN (arg_info) = NULL;
        DO_COND (arg_node) = Abstract (pred, arg_info);
    } else {
        INFO_FLTN_LASTASSIGN (arg_info) = NULL;
    }
    pred2 = Trav (pred, arg_info);
    DBUG_ASSERT ((pred == pred2),
                 "return-node differs from arg_node while flattening an expr!");
    if (final_assign == NULL) {
        DBUG_ASSERT ((NODE_TYPE (DO_INSTR (arg_node)) == N_empty),
                     "INFO_FLTN_FINALASSIGN is NULL although do-body is non-empty");
        /*
         * loop-body ist empty so far!
         */
        if (INFO_FLTN_LASTASSIGN (arg_info) != NULL) {
            DO_INSTR (arg_node) = FreeTree (DO_INSTR (arg_node));
            DO_INSTR (arg_node) = INFO_FLTN_LASTASSIGN (arg_info);
        }
    } else {
        ASSIGN_NEXT (final_assign) = INFO_FLTN_LASTASSIGN (arg_info);
    }
    DBUG_PRINT ("FLATTEN", ("appending %08x tp %08x!", INFO_FLTN_LASTASSIGN (arg_info),
                            final_assign));
    INFO_FLTN_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FltnNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   flattens node N_Nwith
 *   increments with_level and saves local stack only.
 *
 ******************************************************************************/

node *
FltnNwith (node *arg_node, node *arg_info)
{
    local_stack *tmp_tos;

    DBUG_ENTER ("FltnNWith");

    with_level++;
    tmp_tos = tos; /* store tos */
    DBUG_PRINT ("RENAME", ("store tos " P_FORMAT, tos));

    /*
     * for traversing the operation, the generator var(s) should
     * not yet be pushed!
     */
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /*
     * for traversing the body, the generator var(s) should BE pushed!
     */
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    with_level--;
    tos = tmp_tos;
    DBUG_PRINT ("RENAME", ("restore tos " P_FORMAT, tos));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FltnNwithop(node *arg_node, node *arg_info)
 *
 * description:
 *   flattens N_Nwithop
 *   - genarray: the shape is NOT flattened!
 *   - modarray: the array has to be an id or is flattened otherwise.
 *   - fold: the neutral element has to be an id or a constant scalar
 *           or is flattened otherwise. It is optional.
 *
 ******************************************************************************/

node *
FltnNwithop (node *arg_node, node *arg_info)
{
    node *expr, *expr2;
    DBUG_ENTER ("FltnNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_modarray:
        expr = NWITHOP_ARRAY (arg_node);
        if ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_ap)
            || (NODE_TYPE (expr) == N_array)) {
            NWITHOP_ARRAY (arg_node) = Abstract (expr, arg_info);
            expr2 = Trav (expr, arg_info);
            AnnotateIdWithConstVec (expr, NWITHOP_ARRAY (arg_node));
        } else {
            expr2 = Trav (expr, arg_info);
        }
        DBUG_ASSERT ((expr == expr2),
                     "return-node differs from arg_node while flattening an expr!");
        break;

    case WO_genarray:
        expr = NULL;
        break;

    case WO_foldfun:
        /* here is no break missing! */
    case WO_foldprf:
        expr = NWITHOP_NEUTRAL (arg_node);
        if ((expr != NULL)
            && ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_ap)
                || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_Nwith))) {
            NWITHOP_NEUTRAL (arg_node) = Abstract (expr, arg_info);
            expr2 = Trav (expr, arg_info);
            AnnotateIdWithConstVec (expr, NWITHOP_NEUTRAL (arg_node));

            DBUG_ASSERT ((expr == expr2),
                         "return-node differs from arg_node while flattening an expr!");
        }
        break;

    default:
        DBUG_ASSERT (0, "wrong withop tag in N_Nwithop node!");
        /*
         * the following assignment is used only for convincing the C compiler
         * that expr will be initialized in any case!
         */
        expr = 0;
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FltnNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   flattens all N_Npart nodes
 *
 * remark:
 *   the index variables are always renamed to unique names (this differs
 *   from the old WLs). This is done in respect of later Withloop folding.
 *   During WL-folding WL bodies are merged and with unique generators name clashes
 *   can beavoided. Example:
 *   A: with (...i...) { ...B... }
 *   B: with((...j...) { ...tmp = i... }  i bound outside withloop.
 *   substitute B in A and i is bound to index variable of A.
 *
 ******************************************************************************/

node *
FltnNpart (node *arg_node, node *arg_info)
{
    ids *_ids;
    char *old_name;

    DBUG_ENTER ("FltnNpart");

    /* flatten the generator */
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    /*
     * rename index scalars:
     * in the old WL renaming was only done if a variable of the same name
     * was found before. Here we rename all index variables.
     */
    _ids = NPART_IDS (arg_node);
    while (_ids) {
        old_name = IDS_NAME (_ids);
        IDS_NAME (_ids) = TmpVarName (old_name);
        PUSH (old_name, IDS_NAME (_ids), with_level - 1);
        _ids = IDS_NEXT (_ids);
    }

    /*
     * rename index-vector:
     */
    _ids = NPART_VEC (arg_node);
    if (_ids) {
        old_name = IDS_NAME (_ids);
        IDS_NAME (_ids) = TmpVarName (old_name);
        PUSH (old_name, IDS_NAME (_ids), with_level - 1);
    }

    /* at this early point there are no other N_Npart nodes */
    DBUG_ASSERT (!NPART_NEXT (arg_node), "NPART_NEXT() should not yet exist.");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FltnNgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   flattens N_Ngenerator
 *   all non-N_ID-nodes are removed and the operators are changed
 *   to <= and < if possible (bounds != NULL).
 *
 ******************************************************************************/

node *
FltnNgenerator (node *arg_node, node *arg_info)
{
    node **act_son, *act_son_expr, *act_son_expr2;
    int i;
    DBUG_ENTER ("FltnNgenerator");

    /*
     * First, the bounds are adjusted so that the operator can be
     * "normalized" to:   bound1 <= iv = [...] < bound2
     * Howeveer, this is done only for those bounds given explicitly,
     * not for the "." bounds. For those bounds, the "normalization"
     * takes place during typechecking!
     */
    if (NGEN_BOUND1 (arg_node) && F_lt == NGEN_OP1 (arg_node)) {
        /* make <= from < and add 1 to bound */
        NGEN_OP1 (arg_node) = F_le;
        NGEN_BOUND1 (arg_node)
          = MakePrf (F_add,
                     MakeExprs (NGEN_BOUND1 (arg_node), MakeExprs (MakeNum (1), NULL)));
    }
    if (NGEN_BOUND2 (arg_node) && F_le == NGEN_OP2 (arg_node)) {
        /* make < from <= and add 1 to bound */
        NGEN_OP2 (arg_node) = F_lt;
        NGEN_BOUND2 (arg_node)
          = MakePrf (F_add,
                     MakeExprs (NGEN_BOUND2 (arg_node), MakeExprs (MakeNum (1), NULL)));
    }

    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            act_son = &NGEN_BOUND1 (arg_node);
            DBUG_PRINT ("FLATTEN", ("flattening left boundary!"));
            break;
        case 1:
            act_son = &NGEN_BOUND2 (arg_node);
            DBUG_PRINT ("FLATTEN", ("flattening right boundary!"));
            break;
        case 2:
            act_son = &NGEN_STEP (arg_node);
            DBUG_PRINT ("FLATTEN", ("flattening step parameter!"));
            break;
        case 3:
            act_son = &NGEN_WIDTH (arg_node);
            DBUG_PRINT ("FLATTEN", ("flattening width parameter!"));
            break;
        default:
            /*
             * the following assignment is used only for convincing the C compiler
             * that act_son will be initialized in any case!
             */
            act_son = NULL;
        }

        act_son_expr = *act_son;

#if 0
    /* flatten evreything but Ids and constant arrays */
    if( act_son_expr != NULL) {
      if( (N_id != NODE_TYPE(act_son_expr)) && 
          !IsConstantArray(act_son_expr,N_num) ) {
        *act_son = Abstract( act_son_expr, arg_info);
      }
      act_son_expr2 = Trav( act_son_expr, arg_info);
#else
        if (act_son_expr != NULL) {
            if (N_id != NODE_TYPE (act_son_expr)) {
                *act_son = Abstract (act_son_expr, arg_info);
                act_son_expr2 = Trav (act_son_expr, arg_info);
                AnnotateIdWithConstVec (act_son_expr, *act_son);
            } else {
                act_son_expr2 = Trav (act_son_expr, arg_info);
            }
#endif

        DBUG_ASSERT ((act_son_expr == act_son_expr2),
                     "return-node differs from arg_node while flattening an expr!");
    }
}

DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FltnNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   flattens the Ncode nodes.
 *   it's important to have Npart flattened before to avoid name clashes.
 *
 ******************************************************************************/

node *
FltnNcode (node *arg_node, node *arg_info)
{
    node **insert_at, *expr, *expr2, *mem_last_assign, *empty_block;

    DBUG_ENTER ("FltnNcode");

    mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);

    /*
     * First, we traverse the body so that INFO_FLTN_FINALASSIGN will
     * be set correctly, and all renamings will be pushed already!
     * For inserting assignments that (may) result from flattening CEXPR later,
     * we memoize the address which WOULD point to the next assignment
     * IFF one more WOULD exist, i.e. ASSIGNMENT_NEXT if the block is
     * non-empty, BLOCK_INSTR if the block is empty!
     * That allows us, to insert the result of flattening CEXPR
     * by simply assigning to (*insert_at) rather than using 2 different
     * ACCESS-Macros.
     * empty_block holds the pointer to the N_empty node iff one exists!
     */
    DBUG_ASSERT ((NCODE_CBLOCK (arg_node) != NULL), "no code block found");
    if (NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (arg_node))) == N_empty) {
        /*
         * The body is empty; hence we do not need to traverse it!!
         */
        insert_at = &BLOCK_INSTR (NCODE_CBLOCK (arg_node));
        empty_block = BLOCK_INSTR (NCODE_CBLOCK (arg_node));
    } else {
        INFO_FLTN_CONTEXT (arg_info) = CT_wl;
        DBUG_PRINT ("RENAME", ("CONTEXT set to CT_wl"));
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        insert_at = &ASSIGN_NEXT (INFO_FLTN_FINALASSIGN (arg_info));
        empty_block = NULL;
    }

    /*
     * After traversing the body, we finally flatten the CEXPR!
     */
    INFO_FLTN_LASTASSIGN (arg_info) = NULL;
    expr = NCODE_CEXPR (arg_node);
    if (NODE_TYPE (expr) != N_id) {
        NCODE_CEXPR (arg_node) = Abstract (expr, arg_info);
        expr2 = Trav (expr, arg_info);
        AnnotateIdWithConstVec (expr, NCODE_CEXPR (arg_node));
    } else {
        expr2 = Trav (expr, arg_info);
    }
    DBUG_ASSERT ((expr == expr2),
                 "return-node differs from arg_node while flattening an expr!");
    /*
     * Here, INFO_FLTN_LASTASSIGN( arg_info) either points to the freshly
     * generated flatten-assignments or is NULL (if nothing had to be abstracted out)!!
     */
    *insert_at = INFO_FLTN_LASTASSIGN (arg_info);

    /*
     * Now, we take care of the fu....g N_empty node...
     */
    if (BLOCK_INSTR (NCODE_CBLOCK (arg_node)) == NULL) {
        /*
         * Block must have been empty & there is nothing to be flatted out
         * from CEXPR! => re-use empty_block !!
         */
        DBUG_ASSERT ((empty_block != NULL),
                     "flattened body is empty although un-flattened body isn't!!");
        BLOCK_INSTR (NCODE_CBLOCK (arg_node)) = empty_block;
    } else {
        if (empty_block != NULL)
            FreeTree (empty_block);
    }

    INFO_FLTN_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_ASSERT ((NCODE_NEXT (arg_node) == NULL),
                 "there should be only one code block during flatten!");

    DBUG_RETURN (arg_node);
}
