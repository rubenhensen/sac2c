/*
 *
 * $Log$
 * Revision 1.74  1998/05/21 09:59:47  dkr
 * added missing 'break' in switch-statement (FltnNWithop)
 *
 * Revision 1.73  1998/05/20 20:17:23  sbs
 * completely revised version of flatten.c!!
 * All functions have been adjusted to the new macros,
 * new functions InsertRenaming, Abstract, FltnArray, and FltnBlock!
 * the pushes made for conds and loops have been changed so that
 * renamings that leed to typing errors are avoided ( references
 * to vars that have been defined in one branch of a cond only!)
 * Furthermore, the renaming of the LHS of WLs has been changed.
 * Instead of renaming the referenced vars on the RHS of the WL,
 * the WL is assigned to a tmp-var and AFTER the WL, the tmp-var
 * is renamed!
 *
 * Revision 1.72  1998/05/05 13:19:07  srs
 * new WL: constant array bounds are not flattened anymore.
 *
 * Revision 1.71  1998/04/29 13:44:40  srs
 * functions which are imported from moduls are not flattened anymore.
 * This has already been done while compiling the module.
 *
 * Revision 1.70  1998/03/17 14:19:32  cg
 * filename is now reset to sacfilename in order to produce correct
 * error messages
 *
 * Revision 1.69  1998/03/17 09:51:25  srs
 * fixed bug in FltnCon.
 * Traverse neutral element only if not NULL.
 *
 * Revision 1.68  1998/03/16 14:44:52  srs
 * fixed bug resulted from changes in 1.65
 *
 * Revision 1.67  1998/03/15 10:59:28  srs
 * fixed bug in FltnGen
 *
 * Revision 1.66  1998/03/13 18:03:50  srs
 * replaced GenTmpVar() by global function TmpVar()
 * and fixed a bug in FltnNCode
 *
 * Revision 1.65  1998/03/12 13:11:41  srs
 * fixed bug in FlatnNpart()
 *
 * Revision 1.64  1998/03/03 23:15:02  dkr
 * *** empty log message ***
 *
 * Revision 1.63  1998/02/25 13:20:16  srs
 * all index variables of new WL are renamed
 *
 * Revision 1.62  1998/02/19 11:45:22  srs
 * fixed bug in FltnNwith
 *
 * Revision 1.61  1998/02/18 11:34:10  srs
 * fixed bug in FltnNgenerator
 *
 * Revision 1.60  1998/02/15 21:15:02  srs
 * fixed bug in flattening of neutral element of WL-fold
 *
 * Revision 1.59  1998/02/11 17:14:14  srs
 * changed NPART_IDX to NPART_WITHID.
 * fixed bug in FltnNwithop.
 *
 * Revision 1.58  1998/02/06 16:45:57  srs
 * *** empty log message ***
 *
 * Revision 1.57  1998/01/31 13:44:53  srs
 * removed bug in FltnNCode
 *
 * Revision 1.56  1998/01/30 17:53:29  srs
 * adjusted flattening of new WL generator to new node structure.
 *
 * Revision 1.55  1998/01/21 12:52:50  srs
 * Fixed bug in function which flattens fold.
 * Neutral element may be optional.
 *
 * Revision 1.54  1998/01/02 10:57:47  srs
 * changes comments
 *
 * Revision 1.53  1997/12/19 16:08:30  srs
 * changed some functions headers to new style
 *
 * Revision 1.52  1997/12/10 18:37:24  srs
 * - fixed bug in foldprf of old WLs
 * - changed flattening of new WLs
 *
 * Revision 1.51  1997/12/05 16:26:44  srs
 * bugfix with flattening of WLs
 *
 * Revision 1.50  1997/12/04 21:19:02  srs
 * flattening of new WLs is working
 *
 * Revision 1.49  1997/12/02 18:56:33  srs
 * temporary checkin (don't try new_with)
 *
 * Revision 1.48  1997/11/25 12:37:24  srs
 * *** empty log message ***
 *
 * Revision 1.47  1997/11/02 13:57:37  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.46  1997/11/02 13:31:37  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.45  1997/05/14 08:17:59  sbs
 * error with_0001.sac solved:
 * a = with.... now PUSHES a on stack!
 *
 * Revision 1.44  1997/04/25  12:13:52  sbs
 * malloc replaced by Malloc
 *
 * Revision 1.43  1996/09/11  06:10:04  cg
 * Now, arrays as arguments to psi and modarray are abstracted.
 * This is necessary to overload these functiond with user-defined ones.
 *
 * Revision 1.42  1996/01/26  16:28:54  hw
 * bug fixed in FltnExprs (exprs that contain casts will be flattend in the
 * right way now
 *
 * Revision 1.41  1995/12/12  15:48:19  hw
 * changed DuplicateNode
 *
 * Revision 1.40  1995/10/12  14:15:47  cg
 * module implementations without any functions will now pass flatten
 *
 * Revision 1.39  1995/10/06  16:34:22  cg
 * calls to MakeIds adjusted to new signature (3 parameters)
 *
 * Revision 1.38  1995/09/05  09:50:15  hw
 * changed FltnExprs (constants of type N_double will be considered too)
 *
 * Revision 1.37  1995/08/15  12:36:55  hw
 * changed FltnLet (now the left part of a let can be empty)
 *
 * Revision 1.36  1995/07/13  15:21:56  hw
 * changed comments in FltnCon
 *
 * Revision 1.35  1995/07/13  15:16:33  hw
 * changed FltnCon( if neutral element is an array it will not be
 *                  extracted anymore)
 *
 * Revision 1.34  1995/06/30  11:53:54  hw
 * renamed MOD to MODARRAY
 *
 * Revision 1.33  1995/06/26  14:37:51  hw
 * now arrays in the modarray-part of a with-loop will be extracted
 *
 * Revision 1.32  1995/05/31  14:19:12  hw
 * bug fixed in FtlnCon (body after N_modarray will be flattened
 *   correctly now)
 *
 * Revision 1.31  1995/05/31  13:21:11  hw
 * changed flatten of with-loop and of N_foldfun
 *
 * Revision 1.30  1995/05/30  11:59:06  hw
 * bug fixed in FltnExprs
 *
 * Revision 1.29  1995/05/30  06:45:57  hw
 * - FltnMod deleted
 * - FltnCon inserted (node[1] of N_foldfun will be flattened too)
 *
 * Revision 1.28  1995/05/29  13:50:57  hw
 * calls of functions in condition of if-then-else and in
 *  termination condition of loops will be put out of them
 *
 * Revision 1.27  1995/05/29  12:53:05  hw
 * bug fixed in FltnExprs (leading casts will be ignored )
 *
 * Revision 1.26  1995/05/29  10:31:04  hw
 * - bug fixed in FltnWith
 * - changed renameing of variables in FltnLet
 *
 * Revision 1.25  1995/05/11  16:50:10  hw
 * - bug fixed in FltnWhile ( now traverse first the body of the loop and
 *     than the termination condition, because of renaming of variables
 *     in a with-loop)
 *
 * Revision 1.24  1995/05/11  16:18:48  hw
 * -bug fixed in FltnDo & FltnWhile ( empty loop-bodies will be treated
 *                                    correctly now )
 *
 * Revision 1.23  1995/05/09  13:45:41  hw
 * changed DuplicateNode ( node information (node.info) will be copied )
 *
 * Revision 1.22  1995/04/28  11:37:40  hw
 * - added FltnMod
 * - bug fixed in renameing of variables belonging the assignment of
 *   a with_loop
 *
 * Revision 1.21  1995/04/26  17:23:03  hw
 *  - arrays will be abstacted out of generator part of a with-loop
 *  - index_variable of a with_loop will be renamed, if they are modified
 *    in the body of the with-loop
 *  - the variable that a with-loop assignes to , will be renamed in the
 *    body of a with-loop
 *
 * Revision 1.20  1995/04/25  09:08:02  hw
 * index vector of with-loop will be renamed if necessary
 *
 * Revision 1.19  1995/04/21  12:44:18  hw
 * -added  FltnId, FltnLet & FltnArgs
 * -- removed FltnPrf
 * - now varibales in with_loops will be renamed and initialized
 *   if necessary
 *
 * Revision 1.18  1995/04/18  09:50:04  hw
 * bug fixed in FltnExprs (constant arrays will not be abstracted out of
 *  argumentposition of primitive functions)
 *
 * Revision 1.17  1995/04/07  15:33:34  hw
 * FltnExprs will now flatten its arguments depending on the context
 * (modified FltExprs, FltnWhile, FltnDo)
 * N_ap will be "abstracted" out of the termination condition of a loop
 *  (N_prf won't)
 *
 * Revision 1.16  1995/04/07  13:37:42  hw
 * FltnAp, FltnReturn inserted
 * modified FtnExprs to flatten N_exprs depending on the context
 *
 * Revision 1.15  1995/03/13  17:03:44  hw
 * changover from 'info.id' to 'info.ids' of node N_id,
 * N_post, N_pre done
 *
 * Revision 1.14  1995/03/08  17:01:41  hw
 * changed FltnExprs (if an N_array node is a child of N_exprs node
 *                    then N_array will be flattened too)
 *
 * Revision 1.13  1995/03/07  11:00:03  hw
 * added function FltnGen (flatten N_generator)
 *
 * Revision 1.12  1995/01/12  14:04:53  hw
 * initialized node of structure 'ids' with NULL
 *
 * Revision 1.11  1995/01/06  16:45:15  hw
 * added FltnFundef
 *
 * Revision 1.10  1994/12/15  11:47:06  hw
 * inserted FltnModul
 *
 * Revision 1.9  1994/12/12  16:03:59  asi
 * Error fixed in FltnDo
 *
 * Revision 1.8  1994/11/22  17:27:48  hw
 * added function DuplicateNode
 * call DuplicateNode in function FltnWhile to have only one referenze to
 * each node
 *
 * Revision 1.7  1994/11/22  16:40:08  hw
 * added FltnDo
 *
 * Revision 1.6  1994/11/18  13:13:10  hw
 * changed FltnWhile
 * now the flattened stop condition of the while loop is inserted infront of
 * the while statement and also at the end of the loop body
 *
 * Revision 1.5  1994/11/17  16:51:58  hw
 * added FltnWhile & FltnWith
 *
 * Revision 1.4  1994/11/15  14:49:29  hw
 * deleted FltFor
 * bug fixed in FltnPrf
 *
 * Revision 1.3  1994/11/14  17:49:23  hw
 * added FltnCond FltnFor
 * last one doesn`t work correkt at all
 *
 * Revision 1.2  1994/11/10  15:39:42  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "flatten.h"
#include "globals.h"

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
 * LASTASSIGN : every FltnAssign replaces node[0] with arg_node so that other
 * (node[0])    functions may place instructions IN FRONT of that assignment.
 *              FltnAssign returns this node[0]
 * node[1]: this node is only used in the context of WLs. It is necessary
 *          to put assignments (var initialisations) at the beginning of
 *          the WL-body. These assignments are stored here. See comment in
 *          FltnNcode.
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

void
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
 *  char *RenameWithVar(char *name, int level)
 *
 * description:
 *   - creates a new string with a prefix:
 *     name == a , level == 7      =>    result == <WITH_PREFIX>7_a
 *
 ******************************************************************************/

char *
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

node *
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
    if (NODE_TYPE (block_or_assign) == N_block)
        insert_at = &BLOCK_INSTR (block_or_assign);
    else
        insert_at = &ASSIGN_NEXT (block_or_assign);

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

local_stack *
FindId (char *name)
{
    local_stack *tmp;

    DBUG_ENTER ("FindId");

    tmp = tos - 1;
    while ((tmp >= stack) && (strcmp (tmp->id_old, name) != 0))
        tmp--;

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

local_stack *
FindIdInSeg (char *name, int seg_sz, local_stack *seg)
{
    local_stack *tmp;

    DBUG_ENTER ("FindIdInSeg");

    tmp = &seg[seg_sz - 1];
    while ((tmp >= seg) && (strcmp (tmp->id_old, name) != 0))
        tmp--;

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

local_stack *
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

node *
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
 *     a = f( a+b, c)            =>   __flat_<n> = a+b;
 *                                    a          = f( __flat_<n>, c);
 *
 *   renames vars that are defined by a with-loop AND have been defined before
 *   that with-loop:
 *     ...                            ...
 *     a = ...                        a = ...
 *     ...                       =>   ...
 *     a = with ... a ...             __flat_<n> = with ... a ...
 *                                    a = __flat_<n>;
 *
 *   and renames vars that have been defined before a with-loop AND
 *   are redefined in the with-loop:
 *     ...                            ...
 *     a = ...                        a = ...
 *     ...                            ...
 *     b = with(...) {                b = with(...) {
 *                                          __w<i>a = a;
 *           ... a ...           =>         ... __w<i>a ...
 *           a = ...                        __w<i>a = ...
 *           ... a ...                      ... __w<i>a ...
 *         } ...                          } ...
 *
 *   and gives the generator-variables of WLs unique names:
 *
 *     iv = ...                        iv = ...
 *     ...                             ...
 *     a = with(... iv ...)      =>    a = with(... __flat<n>iv ...)
 *           v = ... iv ...                  __w<i>iv = ... __flat<n>iv ...
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
    if (FUNDEF_BODY (arg_node) && ST_imported != FUNDEF_STATUS (arg_node)) {
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
    if (FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);

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

    if (ARG_NEXT (arg_node) != NULL)
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);

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
 *
 ******************************************************************************/

node *
FltnBlock (node *arg_node, node *arg_info)
{
    node *assigns, *mem_last_wlblock;

    DBUG_ENTER ("FltnBlock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        if (INFO_FLTN_CONTEXT (arg_info) == CT_wl) {
            mem_last_wlblock = INFO_FLTN_LASTWLBLOCK (arg_info);
            INFO_FLTN_LASTWLBLOCK (arg_info) = arg_node;
            DBUG_PRINT ("RENAME", ("LASTWLBLOCK set to %08x", arg_node));
            assigns = Trav (BLOCK_INSTR (arg_node), arg_info);
            if (NODE_TYPE (INFO_FLTN_LASTWLBLOCK (arg_info)) == N_block)
                BLOCK_INSTR (INFO_FLTN_LASTWLBLOCK (arg_info)) = assigns;
            else {
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
     * INFO_FLTN_LASTASSIGN( arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_FLTN_LASTASSIGN (arg_info);

    DBUG_EXECUTE ("FLATTEN", {
        if (return_node != arg_node)
            DBUG_PRINT ("FLATTEN", ("node %08x will be inserted instead of %08x",
                                    return_node, arg_node));
    });

    if (ASSIGN_NEXT (arg_node))
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    else {
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
 *   - for each id from the LHS do:
 *     - if we have   a = with ...   AND it is absolutely sure
 *       that a has been defined before, i.e.,
 *         if a has been defined within a conditional, it has been defined
 *         in both branches or if a has been defined in a while-loop, then
 *         it has been defined before as well
 *         <=>  there exists an entry [a, <x>, <wl>] on the stack,
 *       then we rename   a   into a temp-var   __flat<n>   and insert an
 *       assignment of the form    a = __flat<n>;   after the with-loop.
 *       NOTE HERE, that we neither have to push  __flat<n>  nor we
 *       have to take care of any required renamings of   a   as indicated
 *       by <x> and <wl> since that assignment will be traversed later and thus
 *       a required renaming will take place in that invocation of
 *       FltnLet !!
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
 *         and push [ a, __w<with_level>a, <with_level>] on the stack.
 *       - otherwise, this is  NOT the first assignment to    a   in the
 *         actual with-loop, i.e.,
 *           <wl> == with_level,
 *         then we simply rename it to   __w<with_level>a.
 *     - in all other cases we simply push [ a, a, with_level] on the stack
 *
 *   - NOTE, that the RHS has to be traversed first, since the vars on the
 *     LHS should not be on the stack during that traversal!!
 *
 ******************************************************************************/

node *
FltnLet (node *arg_node, node *arg_info)
{
    ids *ids;
    char *var_name, *tmp_var;
    local_stack *tmp;
    node *mem_last_assign;

    DBUG_ENTER ("FltnLet");

    mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);
    ids = LET_IDS (arg_node);

    DBUG_PRINT ("FLATTEN", ("flattening RHS of let-assignment to %s", IDS_NAME (ids)));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_PRINT ("RENAME",
                ("checking LHS of let-assignment to %s for renaming", IDS_NAME (ids)));

    while (ids != NULL) {
        var_name = IDS_NAME (ids);
        tmp = FindId (var_name);

        if (tmp == 0) {
            PUSH (var_name, var_name, with_level);
            /*
             * this PUSH operation covers the whole "third case" from the description
             * above. if var_name is found in the stack and non of the other cases
             * is given, then (with_level == 0) holds and [ var_name, var_name, 0]
             * has been pushed anyway!!
             */
        } else {
            if ((NODE_TYPE (LET_EXPR (arg_node)) == N_with)
                || (NODE_TYPE (LET_EXPR (arg_node))
                    == N_Nwith)) { /* the let expr is a WL */
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
 *  set the context-flag of arg_info to CT_normal, traverse the arguments,
 *  and finally restore the old context-flag.
 *
 ******************************************************************************/

node *
FltnArray (node *arg_node, node *arg_info)
{
    contextflag old_ctxt;

    DBUG_ENTER ("FltnArray");

    DBUG_ASSERT ((ARRAY_AELEMS (arg_node) != NULL), "N_array node where AELEMS is NULL!");

    old_ctxt = INFO_FLTN_CONTEXT (arg_info);
    INFO_FLTN_CONTEXT (arg_info) = CT_normal;
    ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    INFO_FLTN_CONTEXT (arg_info) = old_ctxt;

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
        if ((PRF_PRF (arg_node) == F_psi) || (PRF_PRF (arg_node) == F_modarray))
            INFO_FLTN_CONTEXT (arg_info) = CT_ap;
        else
            INFO_FLTN_CONTEXT (arg_info) = CT_normal;

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
 *
 ******************************************************************************/

node *
FltnExprs (node *arg_node, node *arg_info)
{
    int abstract;
    node *expr, *expr2;

    DBUG_ENTER ("FltnExprs");

    expr = EXPRS_EXPR (arg_node);

    /* skip leading casts */
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
                    || (NODE_TYPE (expr) == N_str) || (NODE_TYPE (expr) == N_array)
                    || (NODE_TYPE (expr) == N_ap) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_Nwith) || (NODE_TYPE (expr) == N_with));
        break;
    case CT_ap:
        abstract = ((NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_ap) || (NODE_TYPE (expr) == N_Nwith)
                    || (NODE_TYPE (expr) == N_with));
        break;
    case CT_normal:
        abstract = ((NODE_TYPE (expr) == N_ap) || (NODE_TYPE (expr) == N_prf)
                    || (NODE_TYPE (expr) == N_Nwith) || (NODE_TYPE (expr) == N_with));
        break;
    default:
        DBUG_ASSERT (0, "illegal context !");
    }

    DBUG_PRINT ("FLATTEN",
                ("context: %d, abstract: %d, expr: %s", INFO_FLTN_CONTEXT (arg_info),
                 abstract, mdb_nodetype[NODE_TYPE (expr)]));

    if (abstract) {
        EXPRS_EXPR (arg_node) = Abstract (EXPRS_EXPR (arg_node), arg_info);
    }
    expr2 = Trav (expr, arg_info);
    DBUG_ASSERT ((expr == expr2),
                 "return-node differs from arg_node while flattening an expr!");

    /*
     * Last, but not least remaining exprs have to be done:
     */
    if (EXPRS_NEXT (arg_node))
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
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
    node *mem_last_assign, *pred, *pred2, *new_assign, *mem_final_assign;
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
        mem_final_assign = INFO_FLTN_FINALASSIGN (arg_info);
        new_assign = Trav (new_assign, arg_info);
        ASSIGN_NEXT (mem_final_assign) = new_assign;
        DBUG_PRINT ("FLATTEN", ("appending %08x to %08x!", new_assign, mem_final_assign));
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
    node *mem_last_assign, *pred, *pred2;

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
    ASSIGN_NEXT (INFO_FLTN_FINALASSIGN (arg_info)) = INFO_FLTN_LASTASSIGN (arg_info);
    DBUG_PRINT ("FLATTEN", ("appending %08x tp %08x!", INFO_FLTN_LASTASSIGN (arg_info),
                            INFO_FLTN_FINALASSIGN (arg_info)));
    INFO_FLTN_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnWith(node *arg_node, node *arg_info)
 *
 * description:
 *   - increase with-level during traversal of the WL-generator and WL-body
 *     (the former is necessary since the generator pushes the generator-vars!)
 *   - pop all vars from stack which have been pushed during the traversal
 *     of the WL-body
 *
 ******************************************************************************/

node *
FltnWith (node *arg_node, node *arg_info)
{
    local_stack *tmp_tos;

    DBUG_ENTER ("FltnWith");

    with_level += 1;
    tmp_tos = tos; /* store tos */
    DBUG_PRINT ("RENAME", ("store tos " P_FORMAT, tos));

    /*
     * traverse the generator:
     */
    WITH_GEN (arg_node) = Trav (WITH_GEN (arg_node), arg_info);
    /*
     * traverse  N_genarray, N_modarray, N_foldfun or N_foldprf and body:
     */
    WITH_OPERATOR (arg_node) = Trav (WITH_OPERATOR (arg_node), arg_info);

    with_level -= 1;
    tos = tmp_tos; /* restore tos */
    DBUG_PRINT ("RENAME", ("restore tos " P_FORMAT, tos));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnGen(node *arg_node, node *arg_info)
 *
 * description:
 *   - abstract out exprs from the boundary-exprs
 *   - rename the generator-variable if necessary.
 *     For doing so, TmpVarName( <generator-variable>) is used since
 *     it might benecessary to rename <generator-variable> once more in
 *     the body of the WL (iff used on the LHS of an assignment!).
 *   - push the generator-variable on the stack with (with_level-1)
 *     Using (with_level-1) forces any assignments to the generator-variable
 *     within the body of the WL to be renamed. This does NOT affect the
 *     assignment in the level above since the stack will be cleared in FltnWith
 *     before leaving the actual WL!
 *
 ******************************************************************************/

node *
FltnGen (node *arg_node, node *arg_info)
{
    int i;
    char *old_name;
    node **bound, *bound_expr, *bound_expr2;
    local_stack *tmp;

    DBUG_ENTER ("FltnGen");

    for (i = 0; i < 2; i++) {
        if (i == 0)
            bound = &GEN_LEFT (arg_node);
        else
            bound = &GEN_RIGHT (arg_node);
        bound_expr = *bound;

        DBUG_ASSERT ((bound_expr != NULL), "NULL boundary of generator in WL!");

        if (NODE_TYPE (bound_expr) == N_ap || NODE_TYPE (bound_expr) == N_prf
            || NODE_TYPE (bound_expr) == N_array) {
            /*
             * This generator-bound has to be abstracted out:
             */
            *bound = Abstract (bound_expr, arg_info);
        }
        bound_expr2 = Trav (bound_expr, arg_info);

        DBUG_ASSERT ((bound_expr == bound_expr2),
                     "return-node differs from arg_node while flattening an expr!");
    }

    /* rename index-vector if necessary */
    old_name = GEN_ID (arg_node);
    tmp = FindId (old_name);
    if (NULL != tmp) {
        GEN_ID (arg_node) = TmpVarName (old_name);
    }
    PUSH (old_name, GEN_ID (arg_node), with_level - 1);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FltnCon(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
FltnCon (node *arg_node, node *arg_info)
{
    node *expr, *expr2, *mem_last_assign, **body;

    DBUG_ENTER ("FltnCon");

    switch (NODE_TYPE (arg_node)) {
    case N_modarray: {
        expr = MODARRAY_ARRAY (arg_node);
        if ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_ap)
            || (NODE_TYPE (expr) == N_array)) {
            MODARRAY_ARRAY (arg_node) = Abstract (expr, arg_info);
        }
        body = &MODARRAY_BODY (arg_node);
        break;
    }
    case N_foldfun: {
        expr = FOLDFUN_NEUTRAL (arg_node);
        if ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_ap)
            || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_with)
            || (NODE_TYPE (expr) == N_Nwith)) {
            FOLDFUN_NEUTRAL (arg_node) = Abstract (expr, arg_info);
        }
        body = &FOLDFUN_BODY (arg_node);
        break;
    }
    case N_foldprf: {
        expr = FOLDPRF_NEUTRAL (arg_node);
        if ((expr != NULL)
            && ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_ap)
                || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_with)
                || (NODE_TYPE (expr) == N_Nwith))) {
            FOLDPRF_NEUTRAL (arg_node) = Abstract (expr, arg_info);
        }
        body = &FOLDPRF_BODY (arg_node);
        break;
    }
    case N_genarray: {
        expr = NULL;
        body = &GENARRAY_BODY (arg_node);
        break;
    }
    default: {
        DBUG_ASSERT (0, "wrong nodetype in WL constructor!");
        break;
    }
    }
    if (expr != NULL) {
        tos = tos - 1; /* DIRTY TRICK: hide the generator-variable! */
        expr2 = Trav (expr, arg_info);
        DBUG_ASSERT ((expr == expr2),
                     "return-node differs from arg_node while flattening an expr!");
        tos = tos + 1; /* DIRTY TRICK: make the generator-variable visible again! */
    }

    mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);
    INFO_FLTN_CONTEXT (arg_info) = CT_wl;
    DBUG_PRINT ("RENAME", ("CONTEXT set to CT_wl"));
    *body = Trav (*body, arg_info);
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
        }
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
                || (NODE_TYPE (expr) == N_array) || (NODE_TYPE (expr) == N_with)
                || (NODE_TYPE (expr) == N_Nwith))) {
            NWITHOP_NEUTRAL (arg_node) = Abstract (expr, arg_info);
        }
        break;
    default:
        DBUG_ASSERT (0, "wrong withop tag in N_Nwithop node!");
        break;
    }

    if (expr != NULL) {
        expr2 = Trav (expr, arg_info);
        DBUG_ASSERT ((expr == expr2),
                     "return-node differs from arg_node while flattening an expr!");
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
        default:;
        }

        act_son_expr = *act_son;

        /* flatten evreything but Ids and constant arrays */
        if (act_son_expr != NULL) {
            if ((N_id != NODE_TYPE (act_son_expr))
                && !IsConstantArray (act_son_expr, N_num)) {
                *act_son = Abstract (act_son_expr, arg_info);
            }
            act_son_expr2 = Trav (act_son_expr, arg_info);

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
    node **insert_at, *expr, *expr2, *mem_last_assign;

    DBUG_ENTER ("FltnNcode");

    mem_last_assign = INFO_FLTN_LASTASSIGN (arg_info);

    /*
     * First, we traverse the body so that INFO_FLTN_FINALASSIGN will
     * be set correctly, and all renamings will be pushed already!
     * For inserting a flattened CEXPR later we memoize INFO_FLTN_FINALASSIGN
     * in *insert_at, since the block may be empty (->access macros!!!)
     */
    if (NCODE_CBLOCK (arg_node) == NULL) {
        NCODE_CBLOCK (arg_node) = MakeBlock (NULL, NULL);
        insert_at = &BLOCK_INSTR (NCODE_CBLOCK (arg_node));
    } else {
        if (NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (arg_node))) == N_empty) {
            insert_at = &BLOCK_INSTR (NCODE_CBLOCK (arg_node));
        } else {
            INFO_FLTN_CONTEXT (arg_info) = CT_wl;
            DBUG_PRINT ("RENAME", ("CONTEXT set to CT_wl"));
            NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
            insert_at = &ASSIGN_NEXT (INFO_FLTN_FINALASSIGN (arg_info));
        }
    }

    /*
     * After traversing the body, we finally flatten the CEXPR!
     */
    INFO_FLTN_LASTASSIGN (arg_info) = NULL;
    expr = NCODE_CEXPR (arg_node);
    if (0 == 0) {
        NCODE_CEXPR (arg_node) = Abstract (expr, arg_info);
    }
    expr2 = Trav (expr, arg_info);
    DBUG_ASSERT ((expr == expr2),
                 "return-node differs from arg_node while flattening an expr!");
    *insert_at = INFO_FLTN_LASTASSIGN (arg_info);

    INFO_FLTN_LASTASSIGN (arg_info) = mem_last_assign;

    DBUG_ASSERT ((NCODE_NEXT (arg_node) == NULL),
                 "there should be only one code block during flatten!");

    DBUG_RETURN (arg_node);
}
