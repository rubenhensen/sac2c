/*
 *
 * $Log$
 * Revision 3.20  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.19  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.18  2004/02/20 08:18:59  mwe
 * now functions with (MODUL_FUNS) and without (MODUL_FUNDECS) body are separated
 * changed tree traversal according to that
 *
 * Revision 3.17  2002/09/06 09:37:41  dkr
 * ND_IDXS2OFFSET added
 *
 * Revision 3.16  2002/07/24 15:03:42  dkr
 * GNMicm() simplified
 *
 * Revision 3.15  2002/07/12 17:16:23  dkr
 * GNMicm(): modifications for new backend done
 *
 * Revision 3.14  2002/02/21 13:41:23  dkr
 * access macros used
 *
 * Revision 3.13  2001/07/18 12:57:45  cg
 * Applications of old tree construction function
 * AppendNodeChain eliminated.
 *
 * Revision 3.12  2001/06/25 13:04:37  dkr
 * fixed a bug in GNMap():
 * FreeTree() on arg_info replaced by Free()
 *
 * Revision 3.11  2001/05/17 16:45:06  sbs
 * NEVER CALL FreeTree on arg_info unless you really know what is happening!!!
 *
 * Revision 3.10  2001/05/17 12:46:31  nmw
 * MALLOC/FREE changed to Malloc/Free, result of Free() used
 *
 * Revision 3.9  2001/04/30 12:12:36  nmw
 * integrate traversal of special fundefs in generatemasks traversal
 *
 * Revision 3.8  2001/04/19 08:01:38  dkr
 * macro F_PTR used as format string for pointers
 *
 * Revision 3.7  2001/03/22 21:07:51  dkr
 * no changes done
 *
 * Revision 3.6  2000/12/14 18:00:15  dkr
 * warning message for Nwith2 added
 *
 * Revision 3.5  2000/12/12 11:42:22  dkr
 * nodes N_pre, N_post, N_inc, N_dec removed
 *
 * Revision 3.4  2000/12/07 14:37:21  dkr
 * DBUG_ASSERT for GNMwith2 removed
 *
 * Revision 3.2  2000/12/06 20:11:20  dkr
 * GNMwith2 added (dummy function)
 *
 * Revision 3.1  2000/11/20 18:00:42  sacbase
 * new release made
 *
 * Revision 2.21  2000/08/17 10:04:47  dkr
 * PrintDefUseMasks() split into PrintDefMask() and PrintUseMask()
 *
 * Revision 2.20  2000/06/23 14:09:47  dkr
 * nodes for old with-loop removed
 *
 * Revision 2.19  2000/05/30 12:35:16  dkr
 * functions for old with-loop removed
 *
 * Revision 2.18  2000/02/11 14:35:57  dkr
 * some hard wired tree accesses are replaced by access macros :)
 *
 * Revision 2.17  2000/01/26 18:09:19  dkr
 * casts for MRD_LIST corrected
 *
 * Revision 2.16  2000/01/26 17:26:44  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.14  1999/11/15 18:07:05  dkr
 * some macros changed, replaced or modified (VARNO, MRD, ...)
 *
 * Revision 2.13  1999/11/02 12:49:55  dkr
 * potential bug in function MrdGet() marked
 *
 * Revision 2.12  1999/10/29 16:44:44  dkr
 * ExpandMRDL() added
 *
 * Revision 2.10  1999/10/28 18:06:39  dkr
 * output of PrintMrdMask() changed
 *
 * Revision 2.9  1999/10/28 17:49:25  dkr
 * Output of PrintMrdMask() changed
 *
 * Revision 2.8  1999/10/28 17:11:44  dkr
 * functions Print...Mask() changed
 *
 * Revision 2.7  1999/10/26 13:51:52  dkr
 * PushDupMRDL simplified
 *
 * Revision 2.6  1999/10/25 17:10:48  dkr
 * some comments added
 *
 * Revision 2.5  1999/05/03 09:39:19  jhs
 * commented out DBUG_ASSERT in GNMNwith to allow empty generator sets
 * for index-vars ( iv = []).
 *
 * Revision 2.4  1999/04/19 12:55:03  jhs
 * TRUE and FALSE from internal_lib.h used from now on.
 *
 * Revision 2.3  1999/04/13 14:03:28  cg
 * Bug fixed in MrdGet(): function looks behind applications
 * of F_reshape only in modes 2 and 3.
 * MrdGet() now returns the left hand side expression when it used
 * to point to an N_assign node with an N_let node as instruction.
 * The functionality of former function GetExpr() is thus integrated
 * into MrdGet().
 *
 * Revision 2.2  1999/03/31 15:10:18  bs
 * MrdGet: a flag for CFid added. And
 * I did some code cosmetics with the MRD_GET... macros.
 *
 * Revision 2.1  1999/02/23 12:41:49  sacbase
 * new release made
 *
 * Revision 1.5  1999/02/16 21:55:42  sbs
 * made PrintMRD more robust..
 *
 * Revision 1.4  1999/02/11 12:54:52  srs
 * fixed bug in GNMassign()
 *
 * Revision 1.3  1999/02/10 09:52:06  srs
 * inserted access macros in GNMassign
 *
 * Revision 1.2  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.1  1999/01/07 17:37:18  sbs
 * Initial revision
 *
 */

/* General information:
 * The function GenerateMasks() creates DEF (mask[0]) and USE (mask[1])
 * masks for some special nodes. These nodes can be found in OptTrav
 * (note the capial letters, there exists another function OPTTrav()).
 * During later traversal (CF,...) the mechanism OPTTrav() is used to
 * - modify the already in GenerateMasks() created masks
 * - create MRD masks in some special phases (optional, see MRD_TAB).
 *   These MRD masks are stored in N_assign nodes EXCEPT one case:
 *   MRD of index variables introduced in the generator of the new WLs
 *   point to their N_Npart node.
 *
 * remark: storage of MRD mask
 * In OPTTrav (N_assign) the MRD mask ist stored in a node->mask for
 * later usage. Therefore the **node structure is cast to *long.
 *
 */

/*
 * Usage of arg_info during OPTTRAV:
 *
 * INFO_VARNO( arg_info) carries the  number of variables in currenent function
 */

/*
  srs: usage of arg_info in this file:
  mask and varno are used for mask generation, see macros in optimize.h.
  In OPTassign arg_info->node[2] is set, don't know why.
  In OPTfundef the NODE_TYPE of arg_info is changed, don't know why.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "print.h"

#include "optimize.h"
#include "generatemasks.h"
#include "Inline.h"
#include "ArrayElimination.h"

/*
 * Stack which implements the 'most recently defined' lists.
 */

stack *mrdl_stack;

/*
 * assign_level is a counter for the depth of assign nodes in a function.
 * The depth is increaed when entering a loop or conditional and is
 * stored in ASSIGN_LEVEL(assign_node). At the moment only used for
 * tabs in MRD_TAB.
 */

static int assign_level;

/*
 *
 *  functionname  : CheckStack
 *  arguments     :
 *  description   : checks whether stack has reached it's limit and reallocates
 *                  memory if necessary.
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
CheckStack (stack *my_stack)
{
    stelm *tmp;

    DBUG_ENTER ("CheckStack");

    if (mrdl_stack->tos == ((mrdl_stack->st_len) - 1)) {
        tmp = Malloc (sizeof (stelm) * (2 * mrdl_stack->st_len));

        memcpy (tmp, mrdl_stack->stack, sizeof (stelm) * mrdl_stack->st_len);

        mrdl_stack->stack = Free (mrdl_stack->stack);

        mrdl_stack->stack = tmp;
        mrdl_stack->st_len *= 2;
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PushMRDL
 *  arguments     : 1) number of variables in new list
 *  description   : a new entry will be pushed on the mrdl_stack. The entry is a pointer
 *                  to an array with size NumVar. The elements of the array are pointers
 *                  to the last expression the variable is set to in control-flow
 *                  direction.
 *  global vars   : mrdl_stack
 *  internal funs : --
 *  external funs : Error, Malloc
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */

void
PushMRDL (long NumVar)
{
    int i;

    DBUG_ENTER ("PushMRDL");
    DBUG_PRINT ("STACK",
                ("Push Stack TOS = %d -> %d", mrdl_stack->tos, (mrdl_stack->tos) + 1));
    mrdl_stack->tos++;
    MRD_LIST = (long *)Malloc (sizeof (node *) * (NumVar + 1));
    MRD_VLEN = NumVar;
    for (i = 0; i < NumVar; i++) {
        MRD (i) = NULL;
    }
    CheckStack (mrdl_stack);
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PushDupMRDL
 *  arguments     : --
 *  description   : Duplicates the top entry of the mrdl_stack
 *  global vars   : mrdl_stack
 *  internal funs : --
 *  external funs : Malloc
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */

void
PushDupMRDL ()
{
    int NumVar, i;

    DBUG_ENTER ("PushDupMRDL");
    DBUG_PRINT ("STACK",
                ("Dup Stack TOS = %d -> %d", mrdl_stack->tos, (mrdl_stack->tos) + 1));
    NumVar = MRD_VLEN;
    mrdl_stack->tos++;
    MRD_LIST = (long *)Malloc (sizeof (node *) * (NumVar + 1));
    for (i = 0; i < NumVar; i++) {
        MRD (i) = mrdl_stack->stack[mrdl_stack->tos - 1].varlist[i];
    }
    MRD_VLEN = NumVar;
    CheckStack (mrdl_stack);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PopMRDL
 *  arguments     : --
 *  description   : The top entry of the mrdl_stack will be removed
 *  global vars   : mrdl_stack
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */

void
PopMRDL ()
{
    DBUG_ENTER ("PopMRDL");
    DBUG_PRINT ("STACK", ("Pop TOS = %d -> %d", mrdl_stack->tos, (mrdl_stack->tos) - 1));
    MRD_LIST = Free (MRD_LIST);
    mrdl_stack->tos--;
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ExpandMRDL(int new_num)
 *
 * description:
 *   expands the top-most MRD-mask to the new # of entries 'new_num'.
 *   (if new_num is <= the current # of entries nothing happens,
 *    otherwise the old MRD-mask is copied into the expanded one und removed
 *    from memory.)
 *
 ******************************************************************************/

void
ExpandMRDL (int new_num)
{
    node **varlist;
    int NumVar, i;

    DBUG_ENTER ("ExpandMRDL");
    NumVar = MRD_VLEN;
    if (new_num > NumVar) {
        varlist = (node **)MRD_LIST;
        MRD_LIST = (long *)Malloc (sizeof (node *) * (new_num + 1));
        for (i = 0; i < NumVar; i++) {
            MRD (i) = varlist[i];
        }
        varlist = Free (varlist);
        for (i = NumVar; i < new_num; i++) {
            MRD (i) = NULL;
        }
        MRD_VLEN = new_num;
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : CheckScope
 *  arguments     : 1) MRD-list at psi-node
 *                  2) assign-node from the let-node who's right side should be
 * substituted 3) number of variables used and/or defined in current function 4) TRUE  =
 * check defined and used mask's FALSE = check only used mask R) TRUE  = substitution
 * possible FALSE = substitution inpossible description   : determines if substitution is
 * possible or not global vars   : syntax_tree, mrdl_stack internal funs : --- external
 * funs : ---
 *
 *  remarks       : ---
 *
 */

int
CheckScope (long *act_mrdl, node *assign_node, int varno, int checkdef)
{
    int i;

    DBUG_ENTER ("CheckScope");

    for (i = 0; i < varno; i++) {
        /* check use mask. If any variable used in the expr of assign_node
           has been redefined between assign_node and the current node then
           we break.*/
        if (ASSIGN_USEMASK (assign_node)[i]
            && act_mrdl[i] != ASSIGN_MRDMASK (assign_node)[i]) {
            break;
        }
        /* check def mask */
        if (checkdef && ASSIGN_DEFMASK (assign_node)[i]
            && (node *)act_mrdl[i] != assign_node) {
            break;
        }
    }

    DBUG_RETURN (i == varno);
}

/*
 *
 *  functionname  : MrdGet
 *  arguments     : 1) number of variable to search for
 *                  2) number of variables in function
 *                  3) 0 : MRD_GETSUBST
 *                     1 : MRD_GETLAST
 *                     2 : MRD_GETDATA
 *                     3 : MRD_GETCSE
 *                     4 : MRD_GETCFID
 *                  R) 0 : Get last node, without traversing through reshape
 *                     1 : Get last node, without traversing through reshape, but
 *                         through compound nodes that are lower in level as
 *                         current node
 *                     2 : Get data or NULL
 *                     3 : Get data or compound-node
 *                     4 : Get last node, without traversing through reshape.
 *                         If the let-expression is an array, it will be resulted.
 *                         It is necessary to do so to get a correct constant folding.
 *
 *  description   : traverses through most recently defined chain
 *  global vars   : mrd_stack, syntax_tree (with mrd-lists at assign-nodes)
 *  internal funs : CheckScope
 *  external funs : ---
 *
 *  remarks       : mrd_stack and lists at the assign_nodes will be created while
 *                  using OPTTrav and act_tab is included in MRD_TAB-macro (optimize.h).
 *
 */

node *
MrdGet (int i, int varno, int what)
{
    int mrd_change;
    node *new, *old, *let_expr;

    DBUG_ENTER ("MrdGet");

    DBUG_ASSERT (((i >= 0) || (i < varno)),
                 "illegal var index found (must be within 0..varno-1)");

    old = NULL;
    new = MRD (i);
    mrd_change = TRUE;

    while (mrd_change && new) {
#if 0
    /*
     * Unfortunately this property is not always hold 8-(((
     * (e.g. Nwithop ...)
     */
    if (NODE_TYPE( new) != N_assign) {
      int dummy = 0;         /* dummy expression for debugging */
    }
    DBUG_ASSERT( (NODE_TYPE( new) == N_assign), "MRD is not an assignment");
#endif
        mrd_change = FALSE;
        switch (ASSIGN_INSTRTYPE (new)) {
        case N_while:
        case N_do:
        case N_cond:
            if (what == 1) {
                new = (node *)ASSIGN_MRDMASK (new)[i];
                mrd_change = TRUE;
            } else if (what == 0 || what == 2 || what == 4)
                /* May be "what != 4" have to be erased! Im prooving... */
                new = old;
            break;
        case N_let:
            if (CheckScope (MRD_LIST, new, varno, FALSE) || (1 == what)) {
                let_expr = LET_EXPR (ASSIGN_INSTR (new));
                switch (NODE_TYPE (let_expr)) {
                case N_id:
                    if (what != 0 && what != 4) {
                        /* May be "what != 4" have to be erased! Im prooving... */
                        old = new;
                        new = (node *)ASSIGN_MRDMASK (new)[ID_VARNO (let_expr)];
                        mrd_change = TRUE;
                    }
                    break;
                case N_prf:
                    /* primitive functions which have not been eleminated yet
                       cannot be simplifyed and hence must not be substitued.*/
                    if (F_reshape == PRF_PRF (let_expr) && ((2 == what) || (3 == what))) {
                        let_expr = PRF_ARG2 (let_expr);
                        if (N_id == NODE_TYPE (let_expr)) {
                            old = new;
                            new = (node *)ASSIGN_MRDMASK (new)[ID_VARNO (let_expr)];
                            mrd_change = TRUE;
                        }
                    }
                    break;
                case N_array:
#if 0
        case N_with:
#endif
                case N_Nwith:
                    if (0 == what)
                        new = old;
                    break;
                default:
                    break;
                }
            } else
                new = old;
            break;
        default:
            break;
        } /* switch */
    }     /* while */

    if (NULL == new) {
        new = old;
    }

    if ((new != NULL) && (NODE_TYPE (new) == N_assign)) {
        if (NODE_TYPE (ASSIGN_INSTR (new)) == N_let) {
            new = LET_EXPR (ASSIGN_INSTR (new));
            if (N_prf == NODE_TYPE (new) && (F_reshape == PRF_PRF (new))) {
                if ((what == 2) || (what == 3)) {
                    new = PRF_ARG2 (new);
                }
            }
        }
    }

    DBUG_RETURN (new);
}

/*
 *
 *  functionname  : SetMask
 *  arguments     : 1) ptr to mask
 *                  2) new mask value
 *                  3) number of variables
 *  description   : initializes mask with value.
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
SetMask (long *mask, long value, int varno)
{
    int i;

    DBUG_ENTER ("SetMask");
    DBUG_PRINT ("GENMASK", ("Mask with lenght %d set to value %d", varno + 1, value));
    for (i = 0; i < varno; i++)
        mask[i] = value;
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : GenMask
 *  arguments     : 1) number of variables
 *                  R) ptr to a new mask
 *  description   : claims mem for a mask and initialized it with 0.
 *  global vars   : --
 *  internal funs : Malloc
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

long *
GenMask (int varno)
{
    long *mask;

    DBUG_ENTER ("GenMask");
    DBUG_PRINT ("GENMASK", ("Mask created with lenght = %d", varno + 1));
    mask = (long *)Malloc (sizeof (long) * (varno + optvar));
    SetMask (mask, 0, (varno + optvar));
    DBUG_RETURN (mask);
}

/*
 *
 *  functionname  : ReGenMask
 *  arguments     : 1) old mask pointer
 *                  2) number of variables
 *                  R) ptr to a new mask
 *  description   : claims mem for a mask and initialized it with 0.
 *  global vars   : --
 *  internal funs : GenMask
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

long *
ReGenMask (long *mask, int varno)
{
    DBUG_ENTER ("ReGenMask");
    if (mask)
        mask = Free (mask);
    DBUG_RETURN (GenMask (varno));
}

/*
 *
 *  functionname  : DupMask
 *  arguments     : 1) ptr to a mask which shall be duplicated
 *                  2) number of variables
 *                  R) the duplicted mask
 *  description   : duplicates mask and returns pointer to new one
 *  global vars   : --
 *  internal funs : --
 *  external funs : Malloc, sizeof
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

long *
DupMask (long *oldmask, int varno)
{
    int i;
    long *mask;

    DBUG_ENTER ("DupMask");
    DBUG_ASSERT ((oldmask != NULL), "DupMask without mask.");
    mask = GenMask (varno);
    for (i = 0; i < varno; i++)
        mask[i] = oldmask[i];
    DBUG_RETURN (mask);
}

long *
CopyMask (long *mask1, int varno1, long *mask2, int varno2)
{
    int i;

    DBUG_ENTER ("CopyMask");
    DBUG_ASSERT ((mask2 != NULL), "CopyMask without mask2.");

    for (i = 0; i < MIN (varno1, varno2); i++) {
        mask2[i] = mask1[i];
    }

    DBUG_RETURN (mask2);
}

/*
 *
 *  functionname  : ReadMask
 *  arguments     : 1) ptr to a mask
 *                  2) variable number
 *                  R) the current value for the variable
 *  description   : Reads value of 'mask' for variable 'number' and returns it
 *
 */

long
ReadMask (long *mask, long number)
{
    DBUG_ENTER ("ReadMask");
    DBUG_RETURN (mask[number]);
}

/******************************************************************************
 *
 * function:
 *   void PrintDefUseMask( FILE *handle, long *mask, int varno)
 *
 * description:
 *   Prints the given def/use mask into the given file.
 *   If (handle == NULL), 'stdout' is used. This is done for use in a debugging
 *   session.
 *
 ******************************************************************************/

void
PrintDefUseMask (FILE *handle, long *mask, int varno)
{
    int i;
    int empty = 1;

    DBUG_ENTER ("PrintDefUseMask");

    if (handle == NULL) {
        handle = stderr;
    }

    if (mask) {
        for (i = 0; i < varno; i++) {
            if (mask[i]) {
                empty = 0;
                fprintf (handle, "(%3d,%3d) ", i, (int)mask[i]);
            }
        }
        if (empty) {
            fprintf (handle, " EMPTY");
        }
    } else {
        fprintf (handle, " NULL");
    }
    fprintf (handle, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintDefMask( FILE *handle, long *defmask, int varno)
 *
 * description:
 *
 *
 ******************************************************************************/

void
PrintDefMask (FILE *handle, long *defmask, int varno)
{
    DBUG_ENTER ("PrintDefMask");

    fprintf (handle, "**Def. Vars: ");
    PrintDefUseMask (handle, defmask, varno);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintUseMask( FILE *handle, long *usemask, int varno)
 *
 * description:
 *
 *
 ******************************************************************************/

void
PrintUseMask (FILE *handle, long *usemask, int varno)
{
    DBUG_ENTER ("PrintUseMask");

    fprintf (handle, "**Used Vars: ");
    PrintDefUseMask (handle, usemask, varno);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *  void PrintMRDMask( FILE *handle, long *mrdmask, int varno)
 *
 * description:
 *   Generates an output in the given file with a textual description of the
 *   MRD-list.
 *   If (handle == NULL), 'stdout' is used. This is done for use in a debugging
 *   session.
 *
 ******************************************************************************/

void
PrintMrdMask (FILE *handle, long *mrdmask, int varno)
{
    node *nodeptr;
    nodetype n_type;
    char *typestr;
    int i;
    int empty = 1;

    DBUG_ENTER ("PrintMRD");

    if (handle == NULL) {
        handle = stderr;
    }

    fprintf (handle, "**MRD list: ");
    if (mrdmask) {
        for (i = 0; i < varno; i++) {
            if (mrdmask[i]) {
                if (!empty) {
                    fprintf (handle, ", ");
                }
                empty = 0;
                n_type = NODE_TYPE ((node *)mrdmask[i]);
                switch (n_type) {
                case N_assign:
                    nodeptr = ASSIGN_INSTR (((node *)mrdmask[i]));
                    typestr = mdb_nodetype[NODE_TYPE (nodeptr)];
                    break;
                default:
                    nodeptr = (node *)mrdmask[i];
                    if ((N_num <= n_type) && (n_type <= N_ok)) {
                        typestr = mdb_nodetype[n_type];
                    } else {
                        typestr = "N_unknown";
                    }
                    break;
                }
                fprintf (handle, "(%d, %s(" F_PTR "))", i, typestr, nodeptr);
            }
        }
        if (empty) {
            fprintf (handle, " EMPTY");
        }
    } else {
        fprintf (handle, " NULL");
    }
    fprintf (handle, "\n");

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : CheckMask
 *  arguments     : 1) ptr to first mask
 *                  2) ptr to second mask
 *                  3) number of variables
 *                  R) boolean
 *  description   : returns - 1 if all variables in first and second mask are both 0
 *                              or both not equal 0.
 *                          - 0 otherwise.
 *  global vars   : mask1, mask2
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       : Used for Dead-Code-Reamoval,
 *                  if Checkmask(variables defined in current statement,
 *                               variables used in further control flow) ==
 *                  0 -- you can't remove the current SAC-expression
 *                  1 -- Dead-Code-Removal is possible
 *
 */

short
CheckMask (long *mask1, long *mask2, int varno)
{
    int i;
    short b;

    DBUG_ENTER ("CheckMask");
    DBUG_ASSERT ((mask1 != NULL), "CheckMask without mask1.");
    DBUG_ASSERT ((mask2 != NULL), "CheckMask without mask2.");
    b = 0;
    for (i = 0; i < varno; i++)
        b = b | (mask1[i] && mask2[i]);
    DBUG_RETURN (!b);
}

/*
 *
 *  functionname  : CheckZero
 *  arguments     : 1) ptr to mask1
 *                  2) ptr to mask2
 *                  3) number of variables
 *                  R) 1 - if for all mask1[i]>0, all values of mask2[i] == 0
 *                     0 - otherwise
 *  description   :
 *  global vars   : mask1, mask2
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

short
CheckZero (long *mask1, long *mask2, int varno)
{
    short whatis = 0;
    int i;

    DBUG_ENTER ("CheckZero");
    for (i = 0; i < varno; i++)
        if ((mask1[i] != 0) && (mask2[i] == 0))
            whatis = 1;
    DBUG_RETURN (whatis);
}

/*
 *
 *  functionname  : CheckEqual
 *  arguments     : 1) ptr to mask1
 *                  2) ptr to mask2
 *                  3) number of variables
 *                  R) 1 - if for all mask1[i]>0 : mask1[i]==mask2[i]
 *                     0 - otherwise
 *  description   :
 *  global vars   : mask1, mask2
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

short
CheckEqual (long *mask1, long *mask2, int varno)
{
    short whatis = 1;
    int i;

    DBUG_ENTER ("CheckEqual");
    for (i = 0; i < varno; i++)
        if ((mask1[i] != 0) && (mask2[i] != mask1[i]))
            whatis = 0;
    DBUG_RETURN (whatis);
}

short
MaskIsNotZero (long *mask, int varno)
{
    int i;
    short b;

    DBUG_ENTER ("MaskIsNotZero");
    DBUG_ASSERT ((mask != NULL), "MaskIsZero without mask.");
    b = 0;
    for (i = 0; i < varno; i++)
        if (mask[i] != 0)
            b = 1;
    DBUG_RETURN (b);
}

/*
 *
 *  functionname  : MinusMask
 *  arguments     : 1) mask, which shall be modified
 *                  2) mask, which shall be subtracted from the other
 *                  3) number of variables
 *  description   : subtracts each element of mask2 from mask1
 *  global vars   : mask1, mask2
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
MinusMask (long *mask1, long *mask2, int varno)
{
    int i;

    DBUG_ENTER ("MinusMask");
    DBUG_ASSERT ((mask1 != NULL), "MinusMask without mask1.");
    DBUG_ASSERT ((mask2 != NULL), "MinusMask without mask2.");
    for (i = 0; i < varno; i++)
        mask1[i] -= mask2[i];
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PlusMask
 *  arguments     : 1) mask, which shall be modified
 *                  2) mask, which shall be added to the other
 *                  3) number of variables
 *  description   : adds each element of mask2 to mask1
 *  global vars   : mask1, mask2, max_var
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
PlusMask (long *mask1, long *mask2, int varno)
{
    int i;

    DBUG_ENTER ("PlusMask");
    DBUG_ASSERT ((mask1 != NULL), "PlusMask without mask1.");
    DBUG_ASSERT ((mask2 != NULL), "PlusMask without mask2.");
    for (i = 0; i < varno; i++)
        mask1[i] += mask2[i];
    DBUG_VOID_RETURN;
}

void
PlusChainMasks (int pos, node *chain, node *arg_info)
{
    DBUG_ENTER ("PlusChainMasks");
    while (NULL != chain) {
        PlusMask (arg_info->mask[0], chain->mask[0], INFO_VARNO (arg_info));
        PlusMask (arg_info->mask[1], chain->mask[1], INFO_VARNO (arg_info));
        chain = chain->node[pos];
    }
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : If3_2Mask
 *  arguments     : 1) mask to modify
 *                  2) controll mask
 *                  3) controll mask
 *                  4) number of variables
 *  description   : mask1[i] is set to
 *                  0 -- if mask2[i] == 0 == mask[3]
 *                  1 -- if mask2[i] != 0
 *                  X -- (no change) if mask2[i] == 0 and mask[3] != 0
 *  global vars   : mask1, mask2, mask3
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       : Used in Dead-Code Removal --
 *                  If3_2Mask( variables used in further control flow,
 *                             variables used in this SAC-expression
 *                             variables defined in this SAC-expression )
 *                  the first mask is modified and contains a new set of
 *                  variables used in further controll flow.
 *
 */

void
If3_2Mask (long *mask1, long *mask2, long *mask3, int varno)
{
    int i;

    DBUG_ENTER ("If3_2Mask");
    DBUG_ASSERT ((mask1 != NULL), "If3_2Mask without mask1.");
    DBUG_ASSERT ((mask2 != NULL), "If3_2Mask without mask2.");
    DBUG_ASSERT ((mask3 != NULL), "If3_2Mask without mask3.");
    for (i = 0; i < varno; i++)
        if (mask2[i] > 0)
            mask1[i] = 1;
        else if (mask3[i] > 0)
            mask1[i] = 0;
        else if (mask1[i] > 0)
            mask1[i] = 1;

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AndMask
 *  arguments     : 1) ptr to a mask
 *                  2) ptr to a mask
 *                  3) number of variables
 *  description   : mask1 = (mask1 and mask2)
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
AndMask (long *mask1, long *mask2, int varno)
{
    int i;

    DBUG_ENTER ("AndMask");
    DBUG_ASSERT ((mask1 != NULL), "AndMask without mask1.");
    DBUG_ASSERT ((mask2 != NULL), "AndMask without mask2.");
    for (i = 0; i < varno; i++)
        mask1[i] = mask1[i] && mask2[i];
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : OrMask
 *  arguments     : 1) ptr to a mask
 *                  2) ptr to a mask
 *                  3) number of variables
 *  description   : mask1 = (mask1 or mask2)
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

void
OrMask (long *mask1, long *mask2, int varno)
{
    int i;

    DBUG_ENTER ("OrMask");
    DBUG_ASSERT ((mask1 != NULL), "OrMask without mask1.");
    DBUG_ASSERT ((mask2 != NULL), "OrMask without mask2.");
    for (i = 0; i < varno; i++)
        mask1[i] = mask1[i] || mask2[i];
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : MaxMask
 *  arguments     : 1) ptr to a mask
 *                  2) ptr to a mask
 *                  3) number of variables
 *  description   : mask1[i] = MAX( mask1[i], mask2[i] )
 *  global vars   : max_var
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..
 *
 *  remarks       :
 *
 */

void
MaxMask (long *mask1, long *mask2, int varno)
{
    int i;

    DBUG_ENTER ("MaxMask");
    DBUG_ASSERT ((mask1 != NULL), "MaxMask without mask1.");
    DBUG_ASSERT ((mask2 != NULL), "MaxMask without mask2.");
    if ((mask1 != NULL) && (mask2 != NULL)) {
        for (i = 0; i < varno; i++)
            if (mask1[i] < mask2[i])
                mask1[i] = mask2[i];
    }
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *OPTTrav(node *trav_node, node *arg_info, node *arg_node)
 *
 * description:
 *   Already created DEF and USE masks are needed to do an OPTTrav() traversal.
 *   OPTTrav() create the MRD-list and modifies DEF and USE masks.
 *
 *   trav_node : node currently traversed (Trav() will be resumed with this node)
 *   arg_node  : node OPTTrav() was started from (only for reference)
 *
 ******************************************************************************/
node *
OPTTrav (node *trav_node, node *arg_info, node *arg_node)
{
    long *old_mask[2];
    int i, do_pop;
    ids *_ids;
    int old_assign_level;
    stack *old_mrdl_stack;

    DBUG_ENTER ("OPTTrav");

    DBUG_ASSERT (arg_node, "Wrong OPTTrav call");

    if (trav_node) {
        switch (NODE_TYPE (arg_node)) {

        case N_fundef:
            switch (NODE_TYPE (trav_node)) {
            case N_assign: /* function body */
                /*
                 * when entering a new fundef we have to store the old data so that we
                 * are able to restore them after traversing this fundef.
                 * this is needed when traversing special fundef in their order of
                 * calling.
                 */
                old_assign_level = assign_level;
                old_mrdl_stack = mrdl_stack;

                assign_level = 0;
                INFO_VARNO (arg_info) = FUNDEF_VARNO (arg_node);
                INFO_DEF = GenMask (INFO_VARNO (arg_info));
                INFO_USE = GenMask (INFO_VARNO (arg_info));

                if (MRD_TAB) {
                    MAKE_MRDL_STACK;
                    PushMRDL (INFO_VARNO (arg_info));
                    trav_node = Trav (trav_node, arg_info);
                    PopMRDL ();
                    FREE_MRDL_STACK;
                } else {
                    trav_node = Trav (trav_node, arg_info);
                }

                PlusMask (FUNDEF_DEFMASK (arg_node), INFO_DEF, INFO_VARNO (arg_info));
                PlusMask (FUNDEF_USEMASK (arg_node), INFO_USE, INFO_VARNO (arg_info));
                INFO_DEF = Free (INFO_DEF);
                INFO_USE = Free (INFO_USE);
                INFO_VARNO (arg_info) = 0;
                DBUG_PRINT ("TRAV",
                            ("Travers function %s - body END", FUNDEF_NAME (arg_node)));

                /* restore saved data */
                mrdl_stack = old_mrdl_stack;
                assign_level = old_assign_level;
                break;
            case N_vardec: /* vardecs of function */
                trav_node = Trav (trav_node, arg_info);
                break;
            case N_arg:
            case N_fundef: /* next fundef */
                trav_node = Trav (trav_node, arg_info);
                break;
            default:
                DBUG_ASSERT ((FALSE), "Subtree not implemented.");
            }
            break;
            /**************************************************************************/

        case N_assign:
            ASSIGN_LEVEL (arg_node) = assign_level;
            if (N_assign == NODE_TYPE (trav_node))
                trav_node = Trav (trav_node, arg_info);
            else {
                if (MRD_TAB) {
                    old_mask[0] = INFO_DEF;
                    old_mask[1] = INFO_USE;
                    INFO_DEF = GenMask (INFO_VARNO (arg_info));
                    INFO_USE = GenMask (INFO_VARNO (arg_info));

                    switch (NODE_TYPE (trav_node)) {
                    case N_let: {
                        ids *ids_node;
                        node *comp_node;

                        /* in this N-let node we define a new var. The old MRDs are
                           stored in the N_assign node. */
                        if (ASSIGN_MRDMASK (arg_node))
                            ASSIGN_MRDMASK (arg_node) = Free (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node) = DupMask (MRD_LIST, MRD_VLEN);

                        /* srs: I don't know, why the MRD for the new array is set here
                           and not only after the traversal of the withloop. This leads to
                           the following problem: 1- A = ... 2- A = with (...)
                           modarray(A,...) The A in modarray is bound the the wrong A in
                           line 2 instead of the A in line 1. In general: ALL A's in the
                           WL-body are bound to the wrong A. I better do not do this for
                           new_with. */
                        /* srs: Well, now I know why this is done. It's a dirty trick
                           CF depends on. The function CheckScope() returns 0 for vars
                           which are inserted in flatten (like __w0_A) instead of 1.
                           1 means to replace __w0_A with A which is not wanted in
                           this situation because it would revert the effect of flatten.
                           Example:
                              B = ...;
                              __w0_B = B;
                              B = with([1,1] <= iv <= [4,4]) {
                                c = __w0_B[0,0];
                              } modarray(__w0_B,iv,c);
                           Both occurences of __w0_B shall not be replaced with B.

                           CF for the new WLs does not need this trick and uses
                           inside_wl struct instrad (see ConstantFolding.c). I did not
                           change the behavior of the old WL to inside_wl because I
                           do not know if this trick is abused elsewhere. */
                        comp_node = GetCompoundNode (arg_node);
                        if (comp_node
#if 0
                && (N_with  == comp_node->nodetype)
#endif
                        ) {
                            ids_node = LET_IDS (trav_node);
                            while (ids_node) {
                                MRD (IDS_VARNO (ids_node)) = arg_node;
                                ids_node = IDS_NEXT (ids_node);
                            }
                        }

                        /* If this is a new WL we ignore all new definitions in the body.
                           Outside of the new WL this has the same effect as the old WL.
                           But inside we have the index variables defined.
                           See comment below at N_Nwith. */

                        if (comp_node
                            && ((N_Nwith == NODE_TYPE (comp_node))
#if 0
                              || (N_with  == NODE_TYPE(comp_node))
#endif
                                  )) {
                            /*
                             * For the current assignment a new MRD mask must be
                             * generated. For now we can simply duplicate the one for the
                             * previous assignment. This new MRD mask will be expanded
                             * during the following Trav(). Therefore this PushDupMRDL()
                             * and the following PopMRDL() are *not* redundant!!
                             */
                            PushDupMRDL ();
                            do_pop = 1;
                        } else {
                            do_pop = 0;
                        }

                        /* traverse into the N_let node */
                        trav_node = Trav (trav_node, arg_info);

                        if (do_pop) {
                            PopMRDL ();
                        }

                        /* add new defines to MRD list */
                        ids_node = LET_IDS (trav_node);
                        while (ids_node) {
                            MRD (IDS_VARNO (ids_node)) = arg_node;
                            ids_node = IDS_NEXT (ids_node);
                        }
                    } break;
                    case N_cond:
                        if (ASSIGN_MRDMASK (arg_node))
                            ASSIGN_MRDMASK (arg_node) = Free (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node) = DupMask (MRD_LIST, MRD_VLEN);

                        assign_level++;
                        trav_node = Trav (trav_node, arg_info);
                        assign_level--;

                        /* all the defines from within the conditional point to the
                           N_cond node. No pointer to N_assign nodes inside the bodies
                           must exist. */
                        if (N_cond == NODE_TYPE (trav_node)) {
                            for (i = 0; i < MRD_VLEN; i++) {
                                if ((ReadMask (COND_THENDEFMASK (trav_node), i) != 0)
                                    || (ReadMask (COND_ELSEDEFMASK (trav_node), i) != 0))
                                    MRD (i) = arg_node;
                            }
                        }
                        break;
                    case N_do:
                        /* recreate MRD mask */
                        if (ASSIGN_MRDMASK (arg_node))
                            ASSIGN_MRDMASK (arg_node) = Free (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node) = DupMask (MRD_LIST, MRD_VLEN);
                        /* for every variable defined inside the body the MRD is set
                           to arg_node. */
                        for (i = 0; i < MRD_VLEN; i++) {
                            if (ReadMask (DO_DEFMASK (ASSIGN_INSTR (arg_node)), i) != 0)
                                MRD (i) = arg_node;
                        }

                        assign_level++;
                        trav_node = Trav (trav_node, arg_info);
                        assign_level--;

                        if ((N_empty == NODE_TYPE (trav_node))
                            || (N_assign == NODE_TYPE (trav_node))) {
                            MRD_LIST = Free (MRD_LIST);
                            MRD_LIST = (long *)ASSIGN_MRDMASK (arg_node);
                            ASSIGN_MRDMASK (arg_node) = NULL;
                        }
                        break;
                    case N_while:
                        if (ASSIGN_MRDMASK (arg_node))
                            ASSIGN_MRDMASK (arg_node) = Free (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node) = DupMask (MRD_LIST, MRD_VLEN);
                        for (i = 0; i < MRD_VLEN; i++) {
                            if (ReadMask (WHILE_DEFMASK (ASSIGN_INSTR (arg_node)), i)
                                != 0)
                                MRD (i) = arg_node;
                        }

                        assign_level++;
                        trav_node = Trav (trav_node, arg_info);
                        assign_level--;

                        if (N_empty == NODE_TYPE (trav_node)) {
                            MRD_LIST = Free (MRD_LIST);
                            MRD_LIST = (long *)ASSIGN_MRDMASK (arg_node);
                            ASSIGN_MRDMASK (arg_node) = NULL;
                        }
                        break;

                    default:
                        /* srs: the default case can be:
                           trav_node == N_return or trav_node == N_annotate */
                        if (ASSIGN_MRDMASK (arg_node))
                            ASSIGN_MRDMASK (arg_node) = Free (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node) = DupMask (MRD_LIST, MRD_VLEN);
                        trav_node = Trav (trav_node, arg_info);
                        break;
                    }

                    PlusMask (ASSIGN_DEFMASK (arg_node), INFO_DEF, INFO_VARNO (arg_info));
                    PlusMask (ASSIGN_USEMASK (arg_node), INFO_USE, INFO_VARNO (arg_info));
                    PlusMask (INFO_DEF, old_mask[0], INFO_VARNO (arg_info));
                    PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                    old_mask[0] = Free (old_mask[0]);
                    old_mask[1] = Free (old_mask[1]);
                } else {
                    old_mask[0] = INFO_DEF;
                    old_mask[1] = INFO_USE;
                    INFO_DEF = GenMask (INFO_VARNO (arg_info));
                    INFO_USE = GenMask (INFO_VARNO (arg_info));

                    trav_node = Trav (trav_node, arg_info);

                    PlusMask (ASSIGN_DEFMASK (arg_node), INFO_DEF, INFO_VARNO (arg_info));
                    PlusMask (ASSIGN_USEMASK (arg_node), INFO_USE, INFO_VARNO (arg_info));
                    PlusMask (INFO_DEF, old_mask[0], INFO_VARNO (arg_info));
                    PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                    old_mask[0] = Free (old_mask[0]);
                    old_mask[1] = Free (old_mask[1]);
                }
            }
            break;
            /*************************************************************************************/

        case N_cond:
            switch (NODE_TYPE (trav_node)) {
                int is_then;
            case N_assign: /* inside then or else */
                is_then = trav_node == COND_THENINSTR (arg_node);

                old_mask[0] = INFO_DEF;
                old_mask[1] = INFO_USE;
                INFO_DEF = GenMask (INFO_VARNO (arg_info));
                INFO_USE = GenMask (INFO_VARNO (arg_info));

                if (MRD_TAB) {
                    /*
                     * This PushDupMRDL() and the following PopMRDL() are *not*
                     * redundant!! (see remark at first occurence of PushDupMRDL)
                     */
                    PushDupMRDL ();
                }

                trav_node = Trav (trav_node, arg_info);

                if (MRD_TAB) {
                    PopMRDL ();
                }

                if (!trav_node)
                    trav_node = MakeEmpty ();

                if (is_then) {
                    PlusMask (COND_THENDEFMASK (arg_node), INFO_DEF,
                              INFO_VARNO (arg_info));
                    PlusMask (COND_THENUSEMASK (arg_node), INFO_USE,
                              INFO_VARNO (arg_info));
                    DBUG_PRINT ("TRAV", ("Travers cond-then-expr END"));
                } else {
                    PlusMask (COND_ELSEDEFMASK (arg_node), INFO_DEF,
                              INFO_VARNO (arg_info));
                    PlusMask (COND_ELSEUSEMASK (arg_node), INFO_USE,
                              INFO_VARNO (arg_info));
                    DBUG_PRINT ("TRAV", ("Travers cond-else-expr END"));
                }
                PlusMask (INFO_DEF, old_mask[0], INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[0] = Free (old_mask[0]);
                old_mask[1] = Free (old_mask[1]);
                break;
            default: /* condition */
                DBUG_PRINT ("TRAV", ("Travers cond - condition"));
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_VARNO (arg_info));

                trav_node = Trav (trav_node, arg_info);

                PlusMask (COND_CONDUSEMASK (arg_node), INFO_USE, INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[1] = Free (old_mask[1]);
                break;
            }
            break;
            /*************************************************************************************/

        case N_do:
            switch (NODE_TYPE (trav_node)) {
            case N_assign: /* body */
                DBUG_PRINT ("TRAV", ("Travers do-body START"));
                old_mask[0] = INFO_DEF;
                old_mask[1] = INFO_USE;
                INFO_DEF = GenMask (INFO_VARNO (arg_info));
                INFO_USE = GenMask (INFO_VARNO (arg_info));

                trav_node = Trav (trav_node, arg_info);

                if (!trav_node)
                    trav_node = MakeEmpty ();

                PlusMask (DO_DEFMASK (arg_node), INFO_DEF, INFO_VARNO (arg_info));
                PlusMask (DO_USEMASK (arg_node), INFO_USE, INFO_VARNO (arg_info));
                PlusMask (INFO_DEF, old_mask[0], INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[0] = Free (old_mask[0]);
                old_mask[1] = Free (old_mask[1]);
                DBUG_PRINT ("TRAV", ("Travers do-body END"));
                break;
            default: /* break condition */
                DBUG_PRINT ("TRAV", ("Travers do - termination expression"));
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_VARNO (arg_info));
                trav_node = Trav (trav_node, arg_info);
                PlusMask (DO_TERMMASK (arg_node), INFO_USE, INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[1] = Free (old_mask[1]);
                break;
            }
            break;
            /*************************************************************************************/

        case N_while:
            switch (NODE_TYPE (trav_node)) {
            case N_assign:
                DBUG_PRINT ("TRAV", ("Travers while-body START"));
                old_mask[0] = INFO_DEF;
                old_mask[1] = INFO_USE;
                INFO_DEF = GenMask (INFO_VARNO (arg_info));
                INFO_USE = GenMask (INFO_VARNO (arg_info));

                if (MRD_TAB) {
                    /*
                     * This PushDupMRDL() and the following PopMRDL() are *not*
                     * redundant!! (see remark at first occurence of PushDupMRDL)
                     */
                    PushDupMRDL ();
                }

                trav_node = Trav (trav_node, arg_info);

                if (MRD_TAB) {
                    PopMRDL ();
                } else

                  if (!trav_node)
                    trav_node = MakeEmpty ();

                PlusMask (WHILE_DEFMASK (arg_node), INFO_DEF, INFO_VARNO (arg_info));
                PlusMask (WHILE_USEMASK (arg_node), INFO_USE, INFO_VARNO (arg_info));
                PlusMask (INFO_DEF, old_mask[0], INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[0] = Free (old_mask[0]);
                old_mask[1] = Free (old_mask[1]);
                DBUG_PRINT ("TRAV", ("Travers while-body END"));
                break;

            default:
                DBUG_PRINT ("TRAV", ("Travers while - termination expression"));
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_VARNO (arg_info));
                trav_node = Trav (trav_node, arg_info);
                PlusMask (WHILE_TERMMASK (arg_node), INFO_USE, INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[1] = Free (old_mask[1]);
                break;
            }
            break;
            /***********************************************************************/

        case N_Nwith:
            /* MRDs in WL:
               In the MRD for the old WL (N_with) the index variable is completely
               ignored. This may be practical because the MRD is easier to create
               and the information may not be necessary (e.g. CF does not need it -
               if there is no MRD entry CF is ignored for the index variable).
               ATTENTION: This was changed in 5.98 to avoid problems in SearchWL().
               But logically it is wrong. The MRD for the usage of an index variable
               in the WL body (before the first definition within the body) can
               always be found in the generator.
               Of course this information is important in the body ("is this an
               index variable?").
               The MRD for the new WL takes this into account, saves the old MRD
               before entering the WL and retores it afterwards. Within the WL body
               the MRD of an index variable points to the defining N_Npart node.
               Other MRDs of Ids in the body point to their defining N_assign nodes.
             */
            switch (NODE_TYPE (trav_node)) {
            case N_Nwithop: /* Trav withop */
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_VARNO (arg_info));

                trav_node = Trav (trav_node, arg_info);

                PlusMask (trav_node->mask[1], INFO_USE, INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[1] = Free (old_mask[1]);
                break;
            case N_Ncode:
                /* see comment below (N_Npart) */
                old_mask[0] = INFO_DEF;
                old_mask[1] = INFO_USE;
                INFO_DEF = GenMask (INFO_VARNO (arg_info));
                INFO_USE = GenMask (INFO_VARNO (arg_info));
                /* defines within this code must not appear as MRDs while traversing
                   other N_Ncode nodes. So the MRDL has to be saved. */

                if (MRD_TAB) {
                    /*
                     * This PushDupMRDL() and the following PopMRDL() are *not*
                     * redundant!! (see remark at first occurence of PushDupMRDL)
                     */
                    PushDupMRDL ();
                }

                trav_node = Trav (trav_node, arg_info);

                if (MRD_TAB) {
                    PopMRDL ();
                }

                PlusMask (trav_node->mask[0], INFO_DEF, INFO_VARNO (arg_info));
                PlusMask (trav_node->mask[1], INFO_USE, INFO_VARNO (arg_info));
                PlusMask (INFO_DEF, old_mask[0], INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[0] = Free (old_mask[0]);
                old_mask[1] = Free (old_mask[1]);
                break;
            case N_Npart:
                /* IMPORTANT:
                   This N_Npart case has always to be traversed before N_Ncode
                   (above). Else the MRD list would be set wrong. */
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_VARNO (arg_info));

                /* set MRD for index variables */
                if (MRD_TAB) {
                    MRD (IDS_VARNO (NPART_VEC (trav_node))) = trav_node;
                    _ids = NPART_IDS (trav_node);
                    while (_ids) {
                        MRD (IDS_VARNO (_ids)) = trav_node;
                        _ids = IDS_NEXT (_ids);
                    }
                }

                trav_node = Trav (trav_node, arg_info);
                PlusMask (trav_node->mask[1], INFO_USE, INFO_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_VARNO (arg_info));
                old_mask[1] = Free (old_mask[1]);
                break;
            default:
                DBUG_ASSERT ((FALSE), "Subtree not implemented for OPTTrav N_Nwith node");
                break;
            }
            break;
            /***********************************************************************/

#if 0
    case N_with:
      switch(NODE_TYPE(trav_node)) {
      case N_assign:
        DBUG_PRINT("TRAV",("Travers with-body START"));
        old_mask[0]=INFO_DEF; 
        old_mask[1]=INFO_USE;
        INFO_DEF=GenMask(INFO_VARNO( arg_info) );
        INFO_USE=GenMask(INFO_VARNO( arg_info) );
            
        trav_node=Trav(trav_node, arg_info);
            
        PlusMask(WITH_BODYDEFMASK(arg_node), INFO_DEF, INFO_VARNO( arg_info) ); 
        PlusMask(WITH_BODYUSEMASK(arg_node), INFO_USE, INFO_VARNO( arg_info) );
        PlusMask(INFO_DEF, old_mask[0], INFO_VARNO( arg_info) );
        PlusMask(INFO_USE, old_mask[1], INFO_VARNO( arg_info) );
        old_mask[0] = Free(old_mask[0]);
        old_mask[1] = Free(old_mask[1]);
        DBUG_PRINT("TRAV",("Travers with-body END"));
        break;
      case N_generator:
        DBUG_PRINT("TRAV",("Travers with-genarator START"));
        old_mask[1]=INFO_USE;
        INFO_USE=GenMask(INFO_VARNO( arg_info) );

        /* Inserted MRD entry for index vector. This is only available
           inside the WL body (PushMRD in assign node). 
           It points to the N_with node. */
        MRD(IDS_VARNO(GEN_IDS(trav_node))) = arg_node;

        trav_node=Trav(trav_node, arg_info);
            
        PlusMask(WITH_GENUSEMASK(arg_node), INFO_USE, INFO_VARNO( arg_info) );
        PlusMask(INFO_USE, old_mask[1], INFO_VARNO( arg_info) );
        old_mask[1] = Free(old_mask[1]);
        DBUG_PRINT("TRAV",("Travers with-generator END"));
        break;
      default:
        DBUG_ASSERT((FALSE),"Subtree not implemented for OPTTrav with_node");
        break;
      }
      break;
      /*************************************************************************************/
#endif

        default:
            DBUG_ASSERT ((FALSE), "Nodetype not implemented for OPTTrav");
        }
    }

    DBUG_RETURN (trav_node);
}

/*
 *
 *  functionname  : OptTrav
 *  arguments     : 1) ptr to node
 *                  2) ptr to info-node
 *                  3) number of node to traverse
 *                  R) 1) with updated informations for optimization
 *  description   : travers spezial node in current node and updates the
 *                  informations for optimization.
 *  global vars   : syntax_tree, arg_info
 *  internal funs : PlusMask, GenMask
 *  external funs : Trav
 *  macros        : DBUG..
 *
 *  remarks       :
 *
 */

node *
OptTrav (node *arg_node, node *arg_info, int node_no)
{
    long *oldmask[2];

    DBUG_ENTER ("OptTrav");

    switch (arg_node->nodetype) {
    case N_fundef:
        switch (node_no) {
        case 0: /* Trav body of function */

            DBUG_PRINT ("TRAV", ("Travers function %s - body", FUNDEF_NAME (arg_node)));
            if (FUNDEF_BODY (arg_node)) {
                arg_info->mask[0] = GenMask (INFO_VARNO (arg_info));
                arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));
                FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
                PlusMask (FUNDEF_MASK (arg_node, 0), arg_info->mask[0],
                          INFO_VARNO (arg_info));
                PlusMask (FUNDEF_MASK (arg_node, 1), arg_info->mask[1],
                          INFO_VARNO (arg_info));
                arg_info->mask[0] = Free (arg_info->mask[0]);
                arg_info->mask[1] = Free (arg_info->mask[1]);
                DBUG_PRINT ("TRAV",
                            ("Travers function %s - body END", FUNDEF_NAME (arg_node)));
            }
            break;

        case 1: /* Trav next function if any */

            if (FUNDEF_NEXT (arg_node))
                FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
            break;

        case 2: /* Trav arguments */

            DBUG_PRINT ("TRAV",
                        ("Travers function %s - arguments", FUNDEF_NAME (arg_node)));
            if (FUNDEF_ARGS (arg_node))
                arg_node->node[2] = Trav (arg_node->node[2], arg_info);
            break;

        default:
            break;
        }
        break;
    case N_assign:
        switch (node_no) {
        case 0: /* Trav instruction */

            DBUG_ASSERT ((NULL != arg_node->node[0]), "N_assign without instruction");
            DBUG_PRINT ("TRAV",
                        ("Travers assign - %s - line %d",
                         mdb_nodetype[arg_node->node[0]->nodetype], arg_node->lineno));
            oldmask[0] = arg_info->mask[0];
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[0] = GenMask (INFO_VARNO (arg_info));
            arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));

            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

            PlusMask (arg_node->mask[0], arg_info->mask[0], INFO_VARNO (arg_info));
            PlusMask (arg_node->mask[1], arg_info->mask[1], INFO_VARNO (arg_info));
            PlusMask (arg_info->mask[0], oldmask[0], INFO_VARNO (arg_info));
            PlusMask (arg_info->mask[1], oldmask[1], INFO_VARNO (arg_info));
            oldmask[0] = Free (oldmask[0]);
            oldmask[1] = Free (oldmask[1]);
            break;

        case 1: /* Trav next assign */

            if (ASSIGN_NEXT (arg_node))
                ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
            break;

        default:
            break;
        }
        break;

    case N_do:
    case N_while:
        switch (node_no) {
        case 0: /* Trav termination condition */

            DBUG_PRINT ("TRAV", ("Travers loop - termination condition"));
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));
            DBUG_ASSERT ((NULL != arg_node->node[0]), "loop without conditional");
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
            PlusMask (arg_node->mask[1], arg_info->mask[1], INFO_VARNO (arg_info));
            PlusMask (arg_info->mask[1], oldmask[1], INFO_VARNO (arg_info));
            oldmask[1] = Free (oldmask[1]);
            break;

        case 1: /* Trav body */

            DBUG_PRINT ("TRAV", ("Travers loop - body"));
            oldmask[0] = arg_info->mask[0];
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[0] = GenMask (INFO_VARNO (arg_info));
            arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));
            DBUG_ASSERT ((NULL != arg_node->node[1]), "loop without body");

            arg_node->node[1] = Trav (arg_node->node[1], arg_info);

            if ((arg_node->nodetype == N_do) || (arg_node->nodetype == N_while)) {
                PlusMask (arg_node->node[1]->mask[0], arg_info->mask[0],
                          INFO_VARNO (arg_info));
                PlusMask (arg_node->node[1]->mask[1], arg_info->mask[1],
                          INFO_VARNO (arg_info));
            }
            PlusMask (arg_info->mask[0], oldmask[0], INFO_VARNO (arg_info));
            PlusMask (arg_info->mask[1], oldmask[1], INFO_VARNO (arg_info));
            oldmask[0] = Free (oldmask[0]);
            oldmask[1] = Free (oldmask[1]);
            DBUG_PRINT ("TRAV", ("Travers loop - body END"));
            break;

        default:
            break;
        }
        break;

    case N_cond:
        switch (node_no) {
        case 0: /* Trav condition */

            DBUG_PRINT ("TRAV", ("Travers cond - condition"));
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));
            DBUG_ASSERT ((NULL != arg_node->node[0]), "cond without conditional");
            COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
            PlusMask (arg_node->mask[1], arg_info->mask[1], INFO_VARNO (arg_info));
            PlusMask (arg_info->mask[1], oldmask[1], INFO_VARNO (arg_info));
            oldmask[1] = Free (oldmask[1]);
            break;

        case 1: /* Trav then  part */
        case 2: /* Trav else part */

            DBUG_PRINT ("TRAV", ("Travers cond - body %d", node_no));
            oldmask[0] = arg_info->mask[0];
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[0] = GenMask (INFO_VARNO (arg_info));
            arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));

            DBUG_ASSERT ((NULL != arg_node->node[node_no]), "cond without body");
            arg_node->node[node_no] = Trav (arg_node->node[node_no], arg_info);

            if (arg_node->nodetype == N_cond) {
                PlusMask (arg_node->node[node_no]->mask[0], arg_info->mask[0],
                          INFO_VARNO (arg_info));
                PlusMask (arg_node->node[node_no]->mask[1], arg_info->mask[1],
                          INFO_VARNO (arg_info));
            } else
                DBUG_ASSERT (0, "srs: When does this happen? Pls report"); /* 13.2.98 */

            PlusMask (arg_info->mask[0], oldmask[0], INFO_VARNO (arg_info));
            PlusMask (arg_info->mask[1], oldmask[1], INFO_VARNO (arg_info));
            oldmask[0] = Free (oldmask[0]);
            oldmask[1] = Free (oldmask[1]);
            DBUG_PRINT ("TRAV", ("Travers cond - body %d END", node_no));
            break;

        default:
            break;
        }

        break;

#if 0
    case N_with:
      switch(node_no)
        {
        case 0: /* Trav generator */
          
          DBUG_PRINT("TRAV",("Travers with - generator"));
          oldmask[1]=arg_info->mask[1];
          arg_info->mask[1]=GenMask(INFO_VARNO(arg_info));
          
          arg_node->node[0]=Trav(arg_node->node[0], arg_info);
          
          PlusMask(arg_node->node[0]->mask[1], arg_info->mask[1], INFO_VARNO(arg_info));
          PlusMask(arg_info->mask[1], oldmask[1], INFO_VARNO(arg_info));
          oldmask[1] = Free(oldmask[1]);
          break;

        case 1: /* Trav genarray, modarray or foldfun */
          
          if ((N_genarray==arg_node->node[1]->nodetype) ||
              (N_modarray==arg_node->node[1]->nodetype))
            {
              DBUG_PRINT("TRAV",("Travers with - genarray or modarray"));
              oldmask[1]=arg_info->mask[1];
              arg_info->mask[1]=GenMask(INFO_VARNO(arg_info));
              DBUG_ASSERT((NULL!=arg_node->node[1]->node[0]),
                          "genarray or modarray without array");
              arg_node->node[1]->node[0]=Trav(arg_node->node[1]->node[0], arg_info);
              PlusMask(arg_node->node[1]->mask[1], arg_info->mask[1], INFO_VARNO(arg_info));
              PlusMask(arg_info->mask[1], oldmask[1], INFO_VARNO(arg_info));
              oldmask[1] = Free(oldmask[1]);
            }
          if (N_foldfun==arg_node->node[1]->nodetype)
            {
              DBUG_PRINT("TRAV",("Travers with - genarray or modarray"));
              oldmask[1]=arg_info->mask[1];
              arg_info->mask[1]=GenMask(INFO_VARNO(arg_info));
              DBUG_ASSERT((NULL!=arg_node->node[1]->node[1]),
                          "N_foldfun without expression to compute neutral element");
              arg_node->node[1]->node[1]=Trav(arg_node->node[1]->node[1], arg_info);
              PlusMask(arg_node->node[1]->mask[1], arg_info->mask[1], INFO_VARNO(arg_info));
              PlusMask(arg_info->mask[1], oldmask[1], INFO_VARNO(arg_info));
              oldmask[1] = Free(oldmask[1]);
            }
          break;
        
        case 2: /* Trav with body */
         
          DBUG_PRINT("TRAV",("Travers with - body"));
          oldmask[0]=arg_info->mask[0];
          oldmask[1]=arg_info->mask[1];
          arg_info->mask[0]=GenMask(INFO_VARNO(arg_info));
          arg_info->mask[1]=GenMask(INFO_VARNO(arg_info));
          
          if ((N_genarray==arg_node->node[1]->nodetype) ||
              (N_modarray==arg_node->node[1]->nodetype))
            {
              DBUG_ASSERT((NULL!=arg_node->node[1]->node[1]),
                          "with expr. without body");
              arg_node->node[1]->node[1]=Trav(arg_node->node[1]->node[1], arg_info);
            }
          else
            {
              DBUG_ASSERT((NULL!=arg_node->node[1]->node[0]),
                          "with expr. without body");
              arg_node->node[1]->node[0]=Trav(arg_node->node[1]->node[0], arg_info);
            }
          PlusMask(arg_node->mask[0], arg_info->mask[0], INFO_VARNO(arg_info));
          PlusMask(arg_node->mask[1], arg_info->mask[1], INFO_VARNO(arg_info));
          PlusMask(arg_info->mask[0], oldmask[0], INFO_VARNO(arg_info));
          PlusMask(arg_info->mask[1], oldmask[1], INFO_VARNO(arg_info));
          oldmask[0] = Free(oldmask[0]);
          oldmask[1] = Free(oldmask[1]);
          DBUG_PRINT("TRAV",("Travers with - body END"));
          break;
        
        default:
          break;
        }
      break;
#endif

    case N_Nwithop: /* Trav withop */
        DBUG_PRINT ("TRAV", ("Travers with - withop"));
        oldmask[1] = arg_info->mask[1];
        arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));
        if (WO_genarray == NWITHOP_TYPE (arg_node))
            NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        else if (WO_modarray == NWITHOP_TYPE (arg_node))
            NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        else if (NWITHOP_NEUTRAL (arg_node))
            NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        PlusMask (arg_node->mask[1], arg_info->mask[1], INFO_VARNO (arg_info));
        PlusMask (arg_info->mask[1], oldmask[1], INFO_VARNO (arg_info));
        oldmask[1] = Free (oldmask[1]);
        break;

    case N_Ncode: /* Trav with body */
        DBUG_PRINT ("TRAV", ("Travers with - bodies"));
        oldmask[0] = arg_info->mask[0];
        oldmask[1] = arg_info->mask[1];
        arg_info->mask[0] = GenMask (INFO_VARNO (arg_info));
        arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));
        /* traverse block (USE and DEF) and expr (USE) but not
           the next N_Ncode node.*/
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
        PlusMask (arg_node->mask[0], arg_info->mask[0], INFO_VARNO (arg_info));
        PlusMask (arg_node->mask[1], arg_info->mask[1], INFO_VARNO (arg_info));
        PlusMask (arg_info->mask[0], oldmask[0], INFO_VARNO (arg_info));
        PlusMask (arg_info->mask[1], oldmask[1], INFO_VARNO (arg_info));
        oldmask[0] = Free (oldmask[0]);
        oldmask[1] = Free (oldmask[1]);
        break;

    case N_Npart: /* Trav generator */
        DBUG_PRINT ("TRAV", ("Travers with - part"));
        oldmask[1] = arg_info->mask[1];
        arg_info->mask[1] = GenMask (INFO_VARNO (arg_info));
        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
        PlusMask (arg_node->mask[1], arg_info->mask[1], INFO_VARNO (arg_info));
        PlusMask (arg_info->mask[1], oldmask[1], INFO_VARNO (arg_info));
        oldmask[1] = Free (oldmask[1]);
        break;

    default:
        switch (node_no) {
        case 0:
        case 1:
        case 2:
            DBUG_PRINT ("TRAV", ("Travers %s node - %d", mdb_nodetype[arg_node->nodetype],
                                 node_no));
            arg_node->node[node_no] = Trav (arg_node->node[node_no], arg_info);
            break;
        default:
            DBUG_PRINT ("TRAV", ("Travers unknown node"));
            arg_node = Trav (arg_node, arg_info);
            break;
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMmodul
 *  arguments     : 1) ptr to syntax_tree
 *                  2) ptr to info_node
 *  description   : function for module traversation
 *
 */

node *
GNMmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMmodul");
    if (MODUL_FUNDECS (arg_node)) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }
    if (MODUL_FUNS (arg_node)) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMfundef
 *  arguments     : 1) ptr to fundef-node
 *                  2) ptr to info_node
 *                  R) ptr to fundef-node 1) with informations of used and defined
 *                     variables in this function.
 *  description   : determines used and defined variables of this function recursicley
 *                  and stores this information in this node.
 *
 */

node *
GNMfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMfundef");
    DBUG_PRINT ("GET", ("GetInfo function: %s", FUNDEF_NAME (arg_node)));
    INFO_VARNO (arg_info) = 0;

    INFO_GNM_FUNDEF (arg_info) = arg_node;

    arg_node = OptTrav (arg_node, arg_info, 2); /* enumberate arguments */

    if (FUNDEF_BODY (arg_node)) {
        if (FUNDEF_VARDEC (arg_node)) /* enumberate local variables */
            FUNDEF_BODY (arg_node) = OptTrav (FUNDEF_BODY (arg_node), arg_info, 1);
        FUNDEF_VARNO (arg_node) = INFO_VARNO (arg_info);
        FUNDEF_BODY_VARNO (arg_node) = INFO_VARNO (arg_info) + optvar;
        NODE_TYPE (arg_info) = NODE_TYPE (arg_node);
        FUNDEF_MASK (arg_node, 0)
          = ReGenMask (FUNDEF_MASK (arg_node, 0), INFO_VARNO (arg_info));
        FUNDEF_MASK (arg_node, 1)
          = ReGenMask (FUNDEF_MASK (arg_node, 1), INFO_VARNO (arg_info));
        arg_node = OptTrav (arg_node, arg_info, 0); /* Trav body of function */
    }

    /*
     * if we are optimizing we do not want to inspect more than one
     * function at a time!
     */
    if (compiler_phase != PH_sacopt)
        arg_node = OptTrav (arg_node, arg_info, 1); /* next fundef */

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMarg
 *  arguments     : 1) ptr to arg-node
 *                  2) ptr to info_node
 *                  R) arg-node 1) with varno is set to arg_info->varno++
 *  description   : enumberates arguments of a function
 *
 */

node *
GNMarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMarg");

    DBUG_PRINT ("VAR",
                ("Arg. %s is number %d", ARG_NAME (arg_node), INFO_VARNO (arg_info)));
    arg_node->varno = INFO_VARNO (arg_info);

    INFO_VARNO (arg_info)++;
    if (NULL != arg_node->node[0]) {
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMvardec
 *  arguments     : 1) ptr to vardec-node
 *                  2) ptr to info_node
 *                  R) vardec-node 1) with varno is set to arg_info->varno++
 *  description   : enumerates variable-declarations of a function
 *
 */

node *
GNMvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMvardec");

    DBUG_PRINT ("VAR", ("Variable %s is number %d", VARDEC_NAME (arg_node),
                        INFO_VARNO (arg_info)));

    VARDEC_VARNO (arg_node) = INFO_VARNO (arg_info);
    INFO_VARNO (arg_info)++;
    if (VARDEC_NEXT (arg_node)) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMlet
 *  arguments     : 1) ptr to let-node
 *                  2) ptr to info_node
 *                  R) not modified 1)
 *  description   : dertermines defined variable and stores information
 *                  in arg_info->mask[0].
 *
 */

node *
GNMlet (node *arg_node, node *arg_info)
{
    ids *ids_node;

    DBUG_ENTER ("GNMlet");

    ids_node = LET_IDS (arg_node);
    while (ids_node) { /* determine defined variables */
        DBUG_PRINT ("VAR", ("Definition of variable %s", IDS_NAME (ids_node)));

        DBUG_ASSERT ((IDS_VARDEC (ids_node) != NULL),
                     "N_let without pointer to declaration.");

        INC_VAR (arg_info->mask[0], IDS_VARNO (ids_node));

        ids_node = IDS_NEXT (ids_node);
    }
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info); /* Trav expression */

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMap
 *  arguments     : 1) ptr to let-node
 *                  2) ptr to info_node
 *                  R) not modified 1)
 *  description   : if application of special fundef, start traversal in it
 *
 */

node *
GNMap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("GNMap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* non-recursive call of special fundef */
    if ((AP_FUNDEF (arg_node) != NULL) && (compiler_phase == PH_sacopt)
        && (FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (INFO_GNM_FUNDEF (arg_info) != AP_FUNDEF (arg_node))) {

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        /*
         * dkr: do NOT use FreeTree() here, because new_arg_info
         *      might be no N_info node anymore ...
         */
        new_arg_info = Free (new_arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMid
 *  arguments     : 1) ptr to id-node
 *                  2) ptr to info_node
 *                  R) not modified 1)
 *  description   : dertermines used variable and stores information
 *                  in arg_info->mask[1].
 *
 */

node *
GNMid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMid");

    DBUG_ASSERT ((ID_VARDEC (arg_node) != NULL), "N_id without pointer to declaration.");

    DBUG_PRINT ("VAR", ("Usage of Variable %d", ID_VARNO (arg_node)));

    INC_VAR (arg_info->mask[1], ID_VARNO (arg_node));

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMassign
 *  arguments     : 1) ptr to assign-node
 *                  2) ptr to info_node
 *                  R) ptr to 1) with informations about defined and used vaiables
 *  description   : Determine defined and used variables for this subtree recursivley,
 *                  stores the information in this node, merges the old information
 *                  with the new information and goes on with next assign node.
 *
 */

node *
GNMassign (node *arg_node, node *arg_info)
{
    node *tmp_node;

    DBUG_ENTER ("GNMassign");

    switch (NODE_TYPE (ASSIGN_INSTR (arg_node))) {
    case N_empty:
        /* e.g. unswitching */
        DBUG_PRINT ("UNS", ("empty assign node removed from tree"));
        tmp_node = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;
        FreeTree (arg_node);
        if (tmp_node)
            arg_node = Trav (tmp_node, arg_info);
        else
            arg_node = NULL;
        break;
    case N_assign:
        /* e.g. unswitching */
        DBUG_PRINT ("UNS", ("double assign node moved into tree"));
        tmp_node = AppendAssign (arg_node->node[0], arg_node->node[1]);
        ASSIGN_INSTR (arg_node) = NULL;
        ASSIGN_NEXT (arg_node) = NULL;
        FreeTree (arg_node);
        arg_node = Trav (tmp_node, arg_info);
        break;
    default:
        arg_node->mask[0] = ReGenMask (arg_node->mask[0], INFO_VARNO (arg_info));
        arg_node->mask[1] = ReGenMask (arg_node->mask[1], INFO_VARNO (arg_info));

        /* srs: Hmmm, arg_info->node[2] may be the last assignment
           of a completely different function. What is this chain for?
           CSE? */
        ASSIGN_CSE (arg_node) = arg_info->node[2];
        arg_info->node[2] = arg_node;

        arg_node = OptTrav (arg_node, arg_info, 0); /* Trav instructions */
        arg_node = OptTrav (arg_node, arg_info, 1); /* go to next node */
        break;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMloop
 *  arguments     : 1) ptr to while- or do-node
 *                  2) ptr to info_node
 *                  R) 1) with  mask[1] is used variables in condition
 *  description   : stores used variables of condition in mask[1].
 *
 */

node *
GNMloop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMloop");

    arg_node->mask[1] = ReGenMask (arg_node->mask[1], INFO_VARNO (arg_info));
    arg_node = OptTrav (arg_node, arg_info, 0);

    DBUG_ASSERT ((NULL != arg_node->node[1]), "loop without body");
    arg_node->node[1]->mask[0]
      = ReGenMask (arg_node->node[1]->mask[0], INFO_VARNO (arg_info));
    arg_node->node[1]->mask[1]
      = ReGenMask (arg_node->node[1]->mask[1], INFO_VARNO (arg_info));
    arg_node = OptTrav (arg_node, arg_info, 1);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMcond
 *  arguments     : 1) ptr to cond-node
 *                  2) ptr to info_node
 *                  3) ptr to 1) with informations about defined and used variables.
 *  description   : Trav conditional, then and else part seperatly and
 *                  store informations in:
 *                  conditional : arg_node->mask
 *                  then        : arg_node->node[1]->mask
 *                  else        : arg_node->node[2]->mask
 *                  Unite the tree mask and store it in arg_info->mask
 *                  for the assignment node.
 *
 */

node *
GNMcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMcond");

    arg_node->mask[1] = ReGenMask (arg_node->mask[1], INFO_VARNO (arg_info));
    arg_node = OptTrav (arg_node, arg_info, 0);

    DBUG_ASSERT ((NULL != arg_node->node[1]), "cond without then-body");
    arg_node->node[1]->mask[0]
      = ReGenMask (arg_node->node[1]->mask[0], INFO_VARNO (arg_info));
    arg_node->node[1]->mask[1]
      = ReGenMask (arg_node->node[1]->mask[1], INFO_VARNO (arg_info));
    arg_node = OptTrav (arg_node, arg_info, 1);

    DBUG_ASSERT ((NULL != arg_node->node[2]), "cond without else-body");
    arg_node->node[2]->mask[0]
      = ReGenMask (arg_node->node[2]->mask[0], INFO_VARNO (arg_info));
    arg_node->node[2]->mask[1]
      = ReGenMask (arg_node->node[2]->mask[1], INFO_VARNO (arg_info));
    arg_node = OptTrav (arg_node, arg_info, 2);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMblock
 *  arguments     : 1) ptr to block-node
 *                  2) ptr to info-node
 *                  R) not modified 1)
 *  description   : traverse body of the block
 *
 */

node *
GNMblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMblock");

    if (BLOCK_INSTR (arg_node)) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *GNMwith(node *arg_node, node *arg_info)
 *
 * description:
 *   computes DEF and USE for a new WL.
 *   - USE mask of N_Nwithop is stored within itself.
 *   - USE and DEF masks of NCODE_CBLOCK and NCODE_CEXPR are stored in N_Ncode
 *   - masks of N_Ngenerator (USE) plus N_Nwithid (DEF) are stored in N_Npart.
 *   all other nodes of the new WL structure do not contain mask information.
 *
 * attention: we don't store the sum of all DEFs and USEs in all of the
 *   generators and bodies yet. Maybe this will be needed l8er.
 *
 ******************************************************************************/

node *
GNMwith (node *arg_node, node *arg_info)
{
    node *code, *part, *withop;
    ids *_ids;

    DBUG_ENTER ("GNMNwith");

    /* withop */
    withop = NWITH_WITHOP (arg_node);
    withop->mask[0] = ReGenMask (withop->mask[0], INFO_VARNO (arg_info));
    withop->mask[1] = ReGenMask (withop->mask[1], INFO_VARNO (arg_info));
    withop = OptTrav (withop, arg_info, 0);

    /* all bodies */
    code = NWITH_CODE (arg_node);
    while (code) {
        code->mask[0] = ReGenMask (code->mask[0], INFO_VARNO (arg_info));
        code->mask[1] = ReGenMask (code->mask[1], INFO_VARNO (arg_info));
        code = OptTrav (code, arg_info, 0);
        code = NCODE_NEXT (code);
    }

    /* all parts */
    part = NWITH_PART (arg_node);
    while (part) {
        part->mask[0] = ReGenMask (part->mask[0], INFO_VARNO (arg_info));
        part->mask[1] = ReGenMask (part->mask[1], INFO_VARNO (arg_info));
        /* traverse generator N_Ngenarator (only USE) */
        part = OptTrav (part, arg_info, 0);

        /* add the ids from N_Nwithid to DEFs */
        DBUG_ASSERT (NPART_VEC (part), "NWITHID_VEC == NULL");

        INC_VAR (part->mask[0], IDS_VARNO (NPART_VEC (part)));
        INC_VAR (arg_info->mask[0], IDS_VARNO (NPART_VEC (part)));

        _ids = NPART_IDS (part);
        while (_ids) {
            INC_VAR (part->mask[0], IDS_VARNO (_ids));
            INC_VAR (arg_info->mask[0], IDS_VARNO (_ids));
            _ids = IDS_NEXT (_ids);
        }

        part = NPART_NEXT (part);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *GNMwith2( node *arg_node, node *arg_info)
 *
 * Description:
 *   computes DEF and USE for a new WL.
 *   - USE mask of N_Nwithop is stored within itself.
 *   - USE and DEF masks of NCODE_CBLOCK and NCODE_CEXPR are stored in N_Ncode
 *   - masks of N_WLstride/grid (USE) plus N_Nwithid (DEF) are stored in
 *     N_Nwith2.
 *   all other nodes of the new WL structure do not contain mask information.
 *
 ******************************************************************************/

node *
GNMwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMNwith2");

#if 0
  /*
   * refcount still uses GenerateMasks() ...
   */
  DBUG_ASSERT( (0), "GenerateMask() not implemented for N_Nwith2 nodes!"
                    " Use InferDFMs() instead!");
#else
#ifndef DBUG_OFF
    SYSWARN (("GenerateMasks() not implemented for N_Nwith2 nodes"));
    CONT_WARN (("Use InferDFMs() instead!"));
#endif
#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *GNMicm( node *arg_node, node *arg_info)
 *
 * description:
 *   computes DEF and USE for IVE-ICMs.
 *
 ******************************************************************************/

node *
GNMicm (node *arg_node, node *arg_info)
{
    node *icm_arg;

    DBUG_ENTER ("GNMicm");

    if (strstr (ICM_NAME (arg_node), "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET icm
         */
        icm_arg = EXPRS_EXPR (ICM_ARGS (arg_node));
        INC_VAR (arg_info->mask[0], ID_VARNO (icm_arg));
    } else if (strstr (ICM_NAME (arg_node), "VECT2OFFSET") != NULL) {
        /*
         * VECT2OFFSET icm
         */
        icm_arg = EXPRS_EXPR (ICM_ARGS (arg_node));
        INC_VAR (arg_info->mask[0], ID_VARNO (icm_arg));

        ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);
    } else if (strstr (ICM_NAME (arg_node), "IDXS2OFFSET") != NULL) {
        /*
         * IDXS2OFFSET icm
         */
        icm_arg = EXPRS_EXPR (ICM_ARGS (arg_node));
        INC_VAR (arg_info->mask[0], ID_VARNO (icm_arg));

        ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((0), "unknown ICM found while mask-generation");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
GenerateMasks (node *arg_node, node *arg_info)
{
    funtab *tmp_tab;

    DBUG_ENTER ("GenerateMasks");
    DBUG_PRINT ("OPT", ("GENERATEMASKS"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    if ((NODE_TYPE (arg_node) == N_fundef) && (FUNDEF_IS_LACFUN (arg_node))) {
        /* do not start traversal in special fundef */
    } else {
        tmp_tab = act_tab;
        act_tab = genmask_tab;
        if (NULL == arg_info) {
            arg_info = MakeInfo ();
            arg_node = Trav (arg_node, arg_info);
            /*
             * dkr: do NOT use FreeTree() here, because new_arg_info
             *      might be no N_info node anymore ...
             */
            arg_info = Free (arg_info);
        } else {
            /*
             * 'arg_info' is modified here,
             * so I guess it's better to avoid this branch.
             */
            arg_node = Trav (arg_node, arg_info);
        }

        act_tab = tmp_tab;
    }

    DBUG_RETURN (arg_node);
}
