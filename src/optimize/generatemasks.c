/*
 *
 * $Log$
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
 *
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
 * INFO_CF_VARNO( arg_info) carries the  number of variables in currenent function
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

#include "tree.h"
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
#include "ConstantFolding.h"
#include "DeadCodeRemoval.h"
#include "LoopInvariantRemoval.h"
#include "Inline.h"
#include "Unroll.h"
#include "Unswitch.h"
#include "ArrayElimination.h"
#include "CSE.h"
#include "WithloopFolding.h"

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

        FREE (mrdl_stack->stack);

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
    mrdl_stack->stack[++mrdl_stack->tos].varlist
      = (node **)Malloc (sizeof (node *) * (NumVar + 1));
    mrdl_stack->stack[mrdl_stack->tos].vl_len = NumVar;
    for (i = 0; i < NumVar; i++)
        mrdl_stack->stack[mrdl_stack->tos].varlist[i] = NULL;
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
    NumVar = mrdl_stack->stack[mrdl_stack->tos].vl_len;
    mrdl_stack->stack[++mrdl_stack->tos].varlist
      = (node **)Malloc (sizeof (node *) * (NumVar + 1));
    for (i = 0; i < NumVar; i++)
        mrdl_stack->stack[mrdl_stack->tos].varlist[i]
          = mrdl_stack->stack[mrdl_stack->tos - 1].varlist[i];
    mrdl_stack->stack[mrdl_stack->tos].vl_len = NumVar;
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
    FREE (mrdl_stack->stack[mrdl_stack->tos].varlist);
    mrdl_stack->tos--;
    DBUG_VOID_RETURN;
}

void
PopMRDL2 ()
{
    DBUG_ENTER ("PopMRDL2");
    DBUG_PRINT ("STACK",
                ("Pop second TOS = %d -> %d", mrdl_stack->tos, (mrdl_stack->tos) - 1));
    FREE (mrdl_stack->stack[(mrdl_stack->tos) - 1].varlist);
    mrdl_stack->stack[(mrdl_stack->tos) - 1].varlist
      = mrdl_stack->stack[mrdl_stack->tos].varlist;
    mrdl_stack->tos--;
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
    /*   int what, i; */
    int i;

    DBUG_ENTER ("CheckScope");

    /*   what = TRUE; */
    /*   for(i=0; i<varno; i++) */
    /*     { */
    /*       if (0 != ASSIGN_USEMASK(assign_node)[i]) */
    /*      if (act_mrdl[i] != ASSIGN_MRDMASK(assign_node)[i]) */
    /*        what = FALSE; */
    /*       if (checkdef && (0 != ASSIGN_DEFMASK(assign_node)[i])) */
    /*      if ((node *)act_mrdl[i] != assign_node) */
    /*        what = FALSE; */
    /*     } */

    /*   DBUG_RETURN(what); */

    for (i = 0; i < varno; i++) {
        /* check use mask. If any variable used in the expr of assign_node
           has been redefined between assign_node and the current node then
           we break.*/
        if (ASSIGN_USEMASK (assign_node)[i]
            && act_mrdl[i] != ASSIGN_MRDMASK (assign_node)[i])
            break;
        /* check def mask */
        if (checkdef && ASSIGN_DEFMASK (assign_node)[i]
            && (node *)act_mrdl[i] != assign_node)
            break;
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
                case N_with:
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
        FREE (mask);
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

/*
 *
 *  functionname  : ReadMask
 *  arguments     : 1) ptr to a mask
 *                  2) variable number
 *                  R) the current value for the variable
 *  description   : Reads value of 'mask' for variable 'number' and returns it
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

long
ReadMask (long *mask, long number)
{
    DBUG_ENTER ("ReadMask");
    DBUG_RETURN (mask[number]);
}

void
DbugMaskOut (node *n, int varno)
{
    char *s;
    s = PrintMask (n->mask[0], varno);
    printf ("DEF: %s\n", s);
    FREE (s);
    s = PrintMask (n->mask[1], varno);
    printf ("USE: %s\n", s);
    FREE (s);
    s = PrintMask (n->mask[2], varno);
    printf ("M3 : %s\n", s);
    FREE (s);
}

void
DbugMaskOut2 (long *mask, int varno)
{
    char *s;
    s = PrintMask (mask, varno);
    printf ("MASK: %s\n", s);
    FREE (s);
}

/*
 *
 *  functionname  : PrintMask
 *  arguments     : 1) the mask you whant to print in a string
 *                  2) number of variables
 *                  R) returns the generated string
 *  description   : generates a string with the textual description of the mask
 *  global vars   : --
 *  internal funs : Malloc
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

char *
PrintMask (long *mask, int varno)
{
    char *text, *temp;
    int i;

    DBUG_ENTER ("PrintMask");
    text = (char *)Malloc (sizeof (char) * 10 * varno);
    if (mask) {
        temp = text;
        for (i = 0; i < varno; i++)
            if (mask[i]) {
                sprintf (temp, "(%3d,%3d) ", i, (int)mask[i]);
                temp = temp + (sizeof (char) * 10);
            }
        sprintf (temp, "%c", '\0');
        if (text == temp)
            sprintf (text, " EMPTY");
    } else
        sprintf (text, " NULL");
    DBUG_RETURN (text);
}

/******************************************************************************
 *
 * function:
 *  char *PrintMRD(long *mask, int varno)
 *
 * description:
 *   generates a string with a textual description of the MRD-list
 *
 ******************************************************************************/

char *
PrintMRD (long *mask, int varno)
{
    char *text, *temp;
    node *nodeptr;
    int i;
    nodetype n_type;

    DBUG_ENTER ("PrintMRD");
    text = (char *)Malloc (sizeof (char) * 39 * varno);
    if (mask) {
        temp = text;
        for (i = 0; i < varno; i++)
            if (mask[i]) {
                n_type = NODE_TYPE ((node *)mask[i]);
                switch (n_type) {
                case N_assign: {
                    nodeptr = ASSIGN_INSTR (((node *)mask[i]));
                    sprintf (temp, "(%3d,%-5.5s(%6p))\n%19s", i,
                             mdb_nodetype[NODE_TYPE (nodeptr)], nodeptr, "");
                    temp = temp + (sizeof (char) * 39);
                    break;
                }
                case N_Npart: {
                    sprintf (temp, "(%3d,%-8.8s)\n%19s", i, mdb_nodetype[N_Npart], "");
                    temp = temp + (sizeof (char) * 34);
                    break;
                }
                default: {
                    if ((N_num <= n_type) && (n_type <= N_ok))
                        sprintf (temp, "(%3d,%-8.8s)\n%19s", i, mdb_nodetype[n_type], "");
                    else
                        sprintf (temp, "(%3d,%-8p)\n%19s", i, (node *)mask[i], "");
                    temp = temp + (sizeof (char) * 34);
                    break;
                }
                }
            }
        sprintf (temp, "%c", '\0');
        if (text == temp)
            sprintf (text, " EMPTY");
    } else
        sprintf (text, " NULL");
    DBUG_RETURN (text);
}

/*
 *  Prints all masks
 */

void
PrintMasks (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintMasks");
    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[0], VARNO);
                  fprintf (outfile, "**Global Def. Var: %s\n", text); FREE (text););
    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  fprintf (outfile, "**Global Used Var: %s\n", text); FREE (text););
    DBUG_EXECUTE ("MRD", char *text; text = PrintMRD (arg_node->mask[2], VARNO);
                  fprintf (outfile, "**MRD-list       : %s\n", text); FREE (text););
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
        PlusMask (arg_info->mask[0], chain->mask[0], VARNO);
        PlusMask (arg_info->mask[1], chain->mask[1], VARNO);
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
 *
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
    int i;
    ids *_ids;

    DBUG_ENTER ("OPTTrav");

    DBUG_ASSERT (arg_node, "Wrong OPTTrav call");

    if (trav_node) {
        switch (NODE_TYPE (arg_node)) {

        case N_fundef:
            switch (NODE_TYPE (trav_node)) {
            case N_assign: /* function body */
                assign_level = 0;
                INFO_CF_VARNO (arg_info) = FUNDEF_VARNO (arg_node);
                INFO_DEF = GenMask (INFO_CF_VARNO (arg_info));
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                if (MRD_TAB) {
                    MAKE_MRDL_STACK;
                    PushMRDL (INFO_CF_VARNO (arg_info));
                    trav_node = Trav (trav_node, arg_info);
                    PopMRDL ();
                    FREE_MRDL_STACK;
                } else {
                    trav_node = Trav (trav_node, arg_info);
                }

                PlusMask (FUNDEF_DEFMASK (arg_node), INFO_DEF, INFO_CF_VARNO (arg_info));
                PlusMask (FUNDEF_USEMASK (arg_node), INFO_USE, INFO_CF_VARNO (arg_info));
                FREE (INFO_DEF);
                FREE (INFO_USE);
                INFO_CF_VARNO (arg_info) = 0;
                DBUG_PRINT ("TRAV",
                            ("Travers function %s - body END", FUNDEF_NAME (arg_node)));
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
            /*************************************************************************************/

        case N_assign:
            ASSIGN_LEVEL (arg_node) = assign_level;
            if (N_assign == NODE_TYPE (trav_node))
                trav_node = Trav (trav_node, arg_info);
            else {
                if (MRD_TAB) {
                    old_mask[0] = INFO_DEF;
                    old_mask[1] = INFO_USE;
                    INFO_DEF = GenMask (INFO_CF_VARNO (arg_info));
                    INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                    switch (NODE_TYPE (trav_node)) {
                    case N_let: {
                        ids *ids_node;
                        node *comp_node;

                        /* in this N-let node we define a new var. The old MRDs are
                           stored in the N_assign node. */
                        if (ASSIGN_MRDMASK (arg_node))
                            FREE (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node)
                          = DupMask (MRD_LIST, INFO_CF_VARNO (arg_info));

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
                        if (comp_node && (N_with == comp_node->nodetype)) {
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
                            && (N_Nwith == NODE_TYPE (comp_node)
                                || N_with == NODE_TYPE (comp_node)))
                            PushDupMRDL ();

                        /* traverse into the N_let node */
                        trav_node = Trav (trav_node, arg_info);

                        if (comp_node
                            && (N_Nwith == NODE_TYPE (comp_node)
                                || N_with == NODE_TYPE (comp_node)))
                            PopMRDL ();

                        /* add new defines to MRD list */
                        ids_node = LET_IDS (trav_node);
                        while (ids_node) {
                            MRD (IDS_VARNO (ids_node)) = arg_node;
                            ids_node = IDS_NEXT (ids_node);
                        }
                    } break;
                    case N_cond:
                        if (ASSIGN_MRDMASK (arg_node))
                            FREE (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node)
                          = DupMask (MRD_LIST, INFO_CF_VARNO (arg_info));

                        assign_level++;
                        trav_node = Trav (trav_node, arg_info);
                        assign_level--;

                        /* all the defines from within the conditional point to the
                           N_cond node. No pointer to N_assign nodes inside the bodies
                           must exist. */
                        if (N_cond == NODE_TYPE (trav_node)) {
                            for (i = 0; i < MRD_TOS.vl_len; i++) {
                                if ((ReadMask (COND_THENDEFMASK (trav_node), i) != 0)
                                    || (ReadMask (COND_ELSEDEFMASK (trav_node), i) != 0))
                                    MRD (i) = arg_node;
                            }
                        }
                        break;
                    case N_do:
                        /* recreate MRD mask */
                        if (ASSIGN_MRDMASK (arg_node))
                            FREE (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node)
                          = DupMask (MRD_LIST, INFO_CF_VARNO (arg_info));
                        /* for every variable defined inside the body the MRD is set
                           to arg_node. */
                        for (i = 0; i < MRD_TOS.vl_len; i++) {
                            if (ReadMask (DO_DEFMASK (ASSIGN_INSTR (arg_node)), i) != 0)
                                MRD (i) = arg_node;
                        }

                        assign_level++;
                        trav_node = Trav (trav_node, arg_info);
                        assign_level--;

                        if ((N_empty == NODE_TYPE (trav_node))
                            || (N_assign == NODE_TYPE (trav_node))) {
                            FREE (MRD_TOS.varlist);
                            MRD_TOS.varlist = (node **)ASSIGN_MRDMASK (arg_node);
                            ASSIGN_MRDMASK (arg_node) = NULL;
                        }
                        break;
                    case N_while:
                        if (ASSIGN_MRDMASK (arg_node))
                            FREE (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node)
                          = DupMask (MRD_LIST, INFO_CF_VARNO (arg_info));
                        for (i = 0; i < MRD_TOS.vl_len; i++) {
                            if (ReadMask (WHILE_DEFMASK (ASSIGN_INSTR (arg_node)), i)
                                != 0)
                                MRD (i) = arg_node;
                        }

                        assign_level++;
                        trav_node = Trav (trav_node, arg_info);
                        assign_level--;

                        if (N_empty == NODE_TYPE (trav_node)) {
                            FREE (MRD_TOS.varlist);
                            MRD_TOS.varlist = (node **)ASSIGN_MRDMASK (arg_node);
                            ASSIGN_MRDMASK (arg_node) = NULL;
                        }
                        break;

                    default:
                        /* srs: the default case can be:
                           trav_node == N_return or trav_node == N_annotate */
                        if (ASSIGN_MRDMASK (arg_node))
                            FREE (ASSIGN_MRDMASK (arg_node));
                        ASSIGN_MRDMASK (arg_node)
                          = DupMask (MRD_LIST, INFO_CF_VARNO (arg_info));
                        trav_node = Trav (trav_node, arg_info);
                        break;
                    }

                    PlusMask (ASSIGN_DEFMASK (arg_node), INFO_DEF,
                              INFO_CF_VARNO (arg_info));
                    PlusMask (ASSIGN_USEMASK (arg_node), INFO_USE,
                              INFO_CF_VARNO (arg_info));
                    PlusMask (INFO_DEF, old_mask[0], INFO_CF_VARNO (arg_info));
                    PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                    FREE (old_mask[0]);
                    FREE (old_mask[1]);
                } else {
                    old_mask[0] = INFO_DEF;
                    old_mask[1] = INFO_USE;
                    INFO_DEF = GenMask (INFO_CF_VARNO (arg_info));
                    INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                    trav_node = Trav (trav_node, arg_info);

                    PlusMask (ASSIGN_DEFMASK (arg_node), INFO_DEF,
                              INFO_CF_VARNO (arg_info));
                    PlusMask (ASSIGN_USEMASK (arg_node), INFO_USE,
                              INFO_CF_VARNO (arg_info));
                    PlusMask (INFO_DEF, old_mask[0], INFO_CF_VARNO (arg_info));
                    PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                    FREE (old_mask[0]);
                    FREE (old_mask[1]);
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
                INFO_DEF = GenMask (INFO_CF_VARNO (arg_info));
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                if (MRD_TAB) {
                    PushDupMRDL ();
                    trav_node = Trav (trav_node, arg_info);
                    PopMRDL ();
                } else
                    trav_node = Trav (trav_node, arg_info);

                if (!trav_node)
                    trav_node = MakeEmpty ();

                if (is_then) {
                    PlusMask (COND_THENDEFMASK (arg_node), INFO_DEF,
                              INFO_CF_VARNO (arg_info));
                    PlusMask (COND_THENUSEMASK (arg_node), INFO_USE,
                              INFO_CF_VARNO (arg_info));
                    DBUG_PRINT ("TRAV", ("Travers cond-then-expr END"));
                } else {
                    PlusMask (COND_ELSEDEFMASK (arg_node), INFO_DEF,
                              INFO_CF_VARNO (arg_info));
                    PlusMask (COND_ELSEUSEMASK (arg_node), INFO_USE,
                              INFO_CF_VARNO (arg_info));
                    DBUG_PRINT ("TRAV", ("Travers cond-else-expr END"));
                }
                PlusMask (INFO_DEF, old_mask[0], INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[0]);
                FREE (old_mask[1]);
                break;
            default: /* condition */
                DBUG_PRINT ("TRAV", ("Travers cond - condition"));
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                trav_node = Trav (trav_node, arg_info);

                PlusMask (COND_CONDUSEMASK (arg_node), INFO_USE,
                          INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[1]);
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
                INFO_DEF = GenMask (INFO_CF_VARNO (arg_info));
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                trav_node = Trav (trav_node, arg_info);

                if (!trav_node)
                    trav_node = MakeEmpty ();

                PlusMask (DO_DEFMASK (arg_node), INFO_DEF, INFO_CF_VARNO (arg_info));
                PlusMask (DO_USEMASK (arg_node), INFO_USE, INFO_CF_VARNO (arg_info));
                PlusMask (INFO_DEF, old_mask[0], INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[0]);
                FREE (old_mask[1]);
                DBUG_PRINT ("TRAV", ("Travers do-body END"));
                break;
            default: /* break condition */
                DBUG_PRINT ("TRAV", ("Travers do - termination expression"));
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));
                trav_node = Trav (trav_node, arg_info);
                PlusMask (DO_TERMMASK (arg_node), INFO_USE, INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[1]);
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
                INFO_DEF = GenMask (INFO_CF_VARNO (arg_info));
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                if (MRD_TAB) {
                    PushDupMRDL ();
                    trav_node = Trav (trav_node, arg_info);
                    PopMRDL ();
                } else
                    trav_node = Trav (trav_node, arg_info);

                if (!trav_node)
                    trav_node = MakeEmpty ();

                PlusMask (WHILE_DEFMASK (arg_node), INFO_DEF, INFO_CF_VARNO (arg_info));
                PlusMask (WHILE_USEMASK (arg_node), INFO_USE, INFO_CF_VARNO (arg_info));
                PlusMask (INFO_DEF, old_mask[0], INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[0]);
                FREE (old_mask[1]);
                DBUG_PRINT ("TRAV", ("Travers while-body END"));
                break;

            default:
                DBUG_PRINT ("TRAV", ("Travers while - termination expression"));
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));
                trav_node = Trav (trav_node, arg_info);
                PlusMask (WHILE_TERMMASK (arg_node), INFO_USE, INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[1]);
                break;
            }
            break;
            /*************************************************************************************/

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
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                /*         if (WO_modarray == NWITHOP_TYPE(trav_node))  */
                /*           NWITHOP_ARRAY(trav_node) = Trav(NWITHOP_ARRAY(trav_node),
                 * arg_info); */
                /*         else if (WO_genarray == NWITHOP_TYPE(trav_node)) */
                /*           NWITHOP_SHAPE(trav_node) = Trav(NWITHOP_SHAPE(trav_node),
                 * arg_info); */
                /*         else if (NWITHOP_NEUTRAL(trav_node)) */
                /*           NWITHOP_NEUTRAL(trav_node) = Trav(NWITHOP_NEUTRAL(trav_node),
                 * arg_info); */
                trav_node = Trav (trav_node, arg_info);

                PlusMask (trav_node->mask[1], INFO_USE, INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[1]);
                break;
            case N_Ncode:
                /* see comment below (N_Npart) */
                old_mask[0] = INFO_DEF;
                old_mask[1] = INFO_USE;
                INFO_DEF = GenMask (INFO_CF_VARNO (arg_info));
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));
                /* defines within this code must not appear as MRDs while traversing
                   other N_Ncode nodes. So the MRDL has to be saved. */
                if (MRD_TAB)
                    PushDupMRDL ();
                trav_node = Trav (trav_node, arg_info);
                if (MRD_TAB)
                    PopMRDL ();
                PlusMask (trav_node->mask[0], INFO_DEF, INFO_CF_VARNO (arg_info));
                PlusMask (trav_node->mask[1], INFO_USE, INFO_CF_VARNO (arg_info));
                PlusMask (INFO_DEF, old_mask[0], INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[0]);
                FREE (old_mask[1]);
                break;
            case N_Npart:
                /* IMPORTANT:
                   This N_Npart case has always to be traversed before N_Ncode
                   (above). Else the MRD list would be set wrong. */
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

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
                PlusMask (trav_node->mask[1], INFO_USE, VARNO);
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[1]);
                break;
            default:
                DBUG_ASSERT ((FALSE), "Subtree not implemented for OPTTrav N_Nwith node");
                break;
            }
            break;
            /*************************************************************************************/

        case N_with:
            switch (NODE_TYPE (trav_node)) {
            case N_assign:
                DBUG_PRINT ("TRAV", ("Travers with-body START"));
                old_mask[0] = INFO_DEF;
                old_mask[1] = INFO_USE;
                INFO_DEF = GenMask (INFO_CF_VARNO (arg_info));
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                trav_node = Trav (trav_node, arg_info);

                PlusMask (WITH_BODYDEFMASK (arg_node), INFO_DEF,
                          INFO_CF_VARNO (arg_info));
                PlusMask (WITH_BODYUSEMASK (arg_node), INFO_USE,
                          INFO_CF_VARNO (arg_info));
                PlusMask (INFO_DEF, old_mask[0], INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[0]);
                FREE (old_mask[1]);
                DBUG_PRINT ("TRAV", ("Travers with-body END"));
                break;
            case N_generator:
                DBUG_PRINT ("TRAV", ("Travers with-genarator START"));
                old_mask[1] = INFO_USE;
                INFO_USE = GenMask (INFO_CF_VARNO (arg_info));

                /* Inserted MRD entry for index vector. This is only available
                   inside the WL body (PushMRD in assign node).
                   It points to the N_with node. */
                MRD (IDS_VARNO (GEN_IDS (trav_node))) = arg_node;

                trav_node = Trav (trav_node, arg_info);

                PlusMask (WITH_GENUSEMASK (arg_node), INFO_USE, INFO_CF_VARNO (arg_info));
                PlusMask (INFO_USE, old_mask[1], INFO_CF_VARNO (arg_info));
                FREE (old_mask[1]);
                DBUG_PRINT ("TRAV", ("Travers with-generator END"));
                break;
            default:
                DBUG_ASSERT ((FALSE), "Subtree not implemented for OPTTrav with_node");
                break;
            }
            break;
            /*************************************************************************************/

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
                arg_info->mask[0] = GenMask (VARNO);
                arg_info->mask[1] = GenMask (VARNO);
                FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
                PlusMask (FUNDEF_MASK (arg_node, 0), arg_info->mask[0], VARNO);
                PlusMask (FUNDEF_MASK (arg_node, 1), arg_info->mask[1], VARNO);
                FREE (arg_info->mask[0]);
                FREE (arg_info->mask[1]);
                /*        srs: FREE() does it */
                /*        arg_info->mask[0]=NULL; */
                /*        arg_info->mask[1]=NULL; */
                DBUG_PRINT ("TRAV",
                            ("Travers function %s - body END", arg_node->info.types->id));
            }
            break;

        case 1: /* Trav next function if any */

            if (FUNDEF_NEXT (arg_node))
                FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
            break;

        case 2: /* Trav arguments */

            DBUG_PRINT ("TRAV",
                        ("Travers function %s - arguments", arg_node->info.types->id));
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
            arg_info->mask[0] = GenMask (VARNO);
            arg_info->mask[1] = GenMask (VARNO);

            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

            PlusMask (arg_node->mask[0], arg_info->mask[0], VARNO);
            PlusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
            PlusMask (arg_info->mask[0], oldmask[0], VARNO);
            PlusMask (arg_info->mask[1], oldmask[1], VARNO);
            FREE (oldmask[0]);
            FREE (oldmask[1]);
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
            arg_info->mask[1] = GenMask (VARNO);
            DBUG_ASSERT ((NULL != arg_node->node[0]), "loop without conditional");
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
            PlusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
            PlusMask (arg_info->mask[1], oldmask[1], VARNO);
            FREE (oldmask[1]);
            break;

        case 1: /* Trav body */

            DBUG_PRINT ("TRAV", ("Travers loop - body"));
            oldmask[0] = arg_info->mask[0];
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[0] = GenMask (VARNO);
            arg_info->mask[1] = GenMask (VARNO);
            DBUG_ASSERT ((NULL != arg_node->node[1]), "loop without body");

            arg_node->node[1] = Trav (arg_node->node[1], arg_info);

            if ((arg_node->nodetype == N_do) || (arg_node->nodetype == N_while)) {
                PlusMask (arg_node->node[1]->mask[0], arg_info->mask[0], VARNO);
                PlusMask (arg_node->node[1]->mask[1], arg_info->mask[1], VARNO);
            }
            PlusMask (arg_info->mask[0], oldmask[0], VARNO);
            PlusMask (arg_info->mask[1], oldmask[1], VARNO);
            FREE (oldmask[0]);
            FREE (oldmask[1]);
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
            arg_info->mask[1] = GenMask (VARNO);
            DBUG_ASSERT ((NULL != arg_node->node[0]), "cond without conditional");
            COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
            PlusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
            PlusMask (arg_info->mask[1], oldmask[1], VARNO);
            FREE (oldmask[1]);
            break;

        case 1: /* Trav then  part */
        case 2: /* Trav else part */

            DBUG_PRINT ("TRAV", ("Travers cond - body %d", node_no));
            oldmask[0] = arg_info->mask[0];
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[0] = GenMask (VARNO);
            arg_info->mask[1] = GenMask (VARNO);

            DBUG_ASSERT ((NULL != arg_node->node[node_no]), "cond without body");
            arg_node->node[node_no] = Trav (arg_node->node[node_no], arg_info);

            if (arg_node->nodetype == N_cond) {
                PlusMask (arg_node->node[node_no]->mask[0], arg_info->mask[0], VARNO);
                PlusMask (arg_node->node[node_no]->mask[1], arg_info->mask[1], VARNO);
            } else
                DBUG_ASSERT (0, "srs: When does this happen? Pls report"); /* 13.2.98 */

            PlusMask (arg_info->mask[0], oldmask[0], VARNO);
            PlusMask (arg_info->mask[1], oldmask[1], VARNO);
            FREE (oldmask[0]);
            FREE (oldmask[1]);
            DBUG_PRINT ("TRAV", ("Travers cond - body %d END", node_no));
            break;

        default:
            break;
        }

        break;

    case N_with:
        switch (node_no) {
        case 0: /* Trav generator */

            DBUG_PRINT ("TRAV", ("Travers with - generator"));
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[1] = GenMask (VARNO);

            arg_node->node[0] = Trav (arg_node->node[0], arg_info);

            PlusMask (arg_node->node[0]->mask[1], arg_info->mask[1], VARNO);
            PlusMask (arg_info->mask[1], oldmask[1], VARNO);
            FREE (oldmask[1]);
            break;

        case 1: /* Trav genarray, modarray or foldfun */

            if ((N_genarray == arg_node->node[1]->nodetype)
                || (N_modarray == arg_node->node[1]->nodetype)) {
                DBUG_PRINT ("TRAV", ("Travers with - genarray or modarray"));
                oldmask[1] = arg_info->mask[1];
                arg_info->mask[1] = GenMask (VARNO);
                DBUG_ASSERT ((NULL != arg_node->node[1]->node[0]),
                             "genarray or modarray without array");
                arg_node->node[1]->node[0] = Trav (arg_node->node[1]->node[0], arg_info);
                PlusMask (arg_node->node[1]->mask[1], arg_info->mask[1], VARNO);
                PlusMask (arg_info->mask[1], oldmask[1], VARNO);
                FREE (oldmask[1]);
            }
            if (N_foldfun == arg_node->node[1]->nodetype) {
                DBUG_PRINT ("TRAV", ("Travers with - genarray or modarray"));
                oldmask[1] = arg_info->mask[1];
                arg_info->mask[1] = GenMask (VARNO);
                DBUG_ASSERT ((NULL != arg_node->node[1]->node[1]),
                             "N_foldfun without expression to compute neutral element");
                arg_node->node[1]->node[1] = Trav (arg_node->node[1]->node[1], arg_info);
                PlusMask (arg_node->node[1]->mask[1], arg_info->mask[1], VARNO);
                PlusMask (arg_info->mask[1], oldmask[1], VARNO);
                FREE (oldmask[1]);
            }
            break;

        case 2: /* Trav with body */

            DBUG_PRINT ("TRAV", ("Travers with - body"));
            oldmask[0] = arg_info->mask[0];
            oldmask[1] = arg_info->mask[1];
            arg_info->mask[0] = GenMask (VARNO);
            arg_info->mask[1] = GenMask (VARNO);

            if ((N_genarray == arg_node->node[1]->nodetype)
                || (N_modarray == arg_node->node[1]->nodetype)) {
                DBUG_ASSERT ((NULL != arg_node->node[1]->node[1]),
                             "with expr. without body");
                arg_node->node[1]->node[1] = Trav (arg_node->node[1]->node[1], arg_info);
            } else {
                DBUG_ASSERT ((NULL != arg_node->node[1]->node[0]),
                             "with expr. without body");
                arg_node->node[1]->node[0] = Trav (arg_node->node[1]->node[0], arg_info);
            }
            PlusMask (arg_node->mask[0], arg_info->mask[0], VARNO);
            PlusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
            PlusMask (arg_info->mask[0], oldmask[0], VARNO);
            PlusMask (arg_info->mask[1], oldmask[1], VARNO);
            FREE (oldmask[0]);
            FREE (oldmask[1]);
            DBUG_PRINT ("TRAV", ("Travers with - body END"));
            break;

        default:
            break;
        }
        break;

    case N_Nwithop: /* Trav withop */
        DBUG_PRINT ("TRAV", ("Travers with - withop"));
        oldmask[1] = arg_info->mask[1];
        arg_info->mask[1] = GenMask (VARNO);
        if (WO_genarray == NWITHOP_TYPE (arg_node))
            NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        else if (WO_modarray == NWITHOP_TYPE (arg_node))
            NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        else if (NWITHOP_NEUTRAL (arg_node))
            NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        PlusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
        PlusMask (arg_info->mask[1], oldmask[1], VARNO);
        FREE (oldmask[1]);
        break;

    case N_Ncode: /* Trav with body */
        DBUG_PRINT ("TRAV", ("Travers with - bodies"));
        oldmask[0] = arg_info->mask[0];
        oldmask[1] = arg_info->mask[1];
        arg_info->mask[0] = GenMask (VARNO);
        arg_info->mask[1] = GenMask (VARNO);
        /* traverse block (USE and DEF) and expr (USE) but not
           the next N_Ncode node.*/
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
        PlusMask (arg_node->mask[0], arg_info->mask[0], VARNO);
        PlusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
        PlusMask (arg_info->mask[0], oldmask[0], VARNO);
        PlusMask (arg_info->mask[1], oldmask[1], VARNO);
        FREE (oldmask[0]);
        FREE (oldmask[1]);
        break;

    case N_Npart: /* Trav generator */
        DBUG_PRINT ("TRAV", ("Travers with - part"));
        oldmask[1] = arg_info->mask[1];
        arg_info->mask[1] = GenMask (VARNO);
        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
        PlusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
        PlusMask (arg_info->mask[1], oldmask[1], VARNO);
        FREE (oldmask[1]);
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
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       : ---
 *
 */

node *
GNMmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMmodul");
    if (MODUL_FUNS (arg_node))
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
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
 *  global vars   : syntax_tree, info_node
 *  internal funs :
 *  external funs : Trav, GenMask
 *  macros        : DBUG..., FREE
 *
 *  remarks       :
 *
 */

node *
GNMfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMfundef");
    DBUG_PRINT ("GET", ("GetInfo function: %s", arg_node->info.types->id));
    VARNO = 0;

    arg_node = OptTrav (arg_node, arg_info, 2); /* enumberate arguments */

    if (FUNDEF_BODY (arg_node)) {
        if (FUNDEF_VARDEC (arg_node)) /* enumberate local variables */
            FUNDEF_BODY (arg_node) = OptTrav (FUNDEF_BODY (arg_node), arg_info, 1);
        FUNDEF_VARNO (arg_node) = VARNO;
        FUNDEF_BODY_VARNO (arg_node) = VARNO + optvar;
        NODE_TYPE (arg_info) = NODE_TYPE (arg_node);
        FUNDEF_MASK (arg_node, 0) = ReGenMask (FUNDEF_MASK (arg_node, 0), VARNO);
        FUNDEF_MASK (arg_node, 1) = ReGenMask (FUNDEF_MASK (arg_node, 1), VARNO);
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
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

node *
GNMarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMarg");
    DBUG_PRINT ("VAR", ("Arg. %s is number %d", arg_node->info.types->id, VARNO));
    arg_node->varno = VARNO;
    VARNO++;
    if (NULL != arg_node->node[0])
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMvardec
 *  arguments     : 1) ptr to vardec-node
 *                  2) ptr to info_node
 *                  R) vardec-node 1) with varno is set to arg_info->varno++
 *  description   : enumerates variable-declarations of a function
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

node *
GNMvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMvardec");
    DBUG_PRINT ("VAR", ("Variable %s is number %d", arg_node->info.types->id, VARNO));
    VARDEC_VARNO (arg_node) = VARNO;
    VARNO++;
    if (VARDEC_NEXT (arg_node))
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
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
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG..., INC_VAR
 *
 *  remarks       :
 *
 */

node *
GNMlet (node *arg_node, node *arg_info)
{
    ids *ids_node;

    DBUG_ENTER ("GNMlet");
    ids_node = LET_IDS (arg_node);
    while (ids_node) /* determine defined variables */
    {
        DBUG_PRINT ("VAR", ("Definition of variable %s", ids_node->id));

        DBUG_ASSERT ((ids_node->node != NULL), "N_let without pointer to declaration.");

        INC_VAR (arg_info->mask[0], IDS_VARNO (ids_node));

        ids_node = ids_node->next;
    }
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info); /* Trav expression */
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
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG..., INC_VAR
 *
 *  remarks       :
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
 *  functionname  : GNMpp
 *  arguments     : 1) ptr to post- or pre-node
 *                  2) ptr to info_node
 *                  R) not modified 1)
 *  description   : dertermines defined and used variables and stores information
 *                  in arg_info->mask[0] and arg_info->mask[1].
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG..., INC_VAR
 *
 *  remarks       :
 *
 */

node *
GNMpp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMpp");
    DBUG_ASSERT ((NULL != arg_node->info.ids->node),
                 "N_pre/post without pointer to declaration.");
    DBUG_PRINT ("VAR", ("Definition of Variable %d", arg_node->info.ids->node->varno));
    INC_VAR (arg_info->mask[0], arg_node->info.ids->node->varno);
    DBUG_PRINT ("VAR", ("Usage of Variable %d", arg_node->info.ids->node->varno));
    INC_VAR (arg_info->mask[1], arg_node->info.ids->node->varno);
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
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav, ReGenMask, PlusMask
 *  macros        : DBUG...
 *
 *  remarks       :
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
        tmp_node = AppendNodeChain (1, arg_node->node[0], arg_node->node[1]);
        ASSIGN_INSTR (arg_node) = NULL;
        ASSIGN_NEXT (arg_node) = NULL;
        FreeTree (arg_node);
        arg_node = Trav (tmp_node, arg_info);
        break;
    default:
        arg_node->mask[0] = ReGenMask (arg_node->mask[0], VARNO);
        arg_node->mask[1] = ReGenMask (arg_node->mask[1], VARNO);

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
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav, PlusMask, ReGenMask
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

node *
GNMloop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMloop");

    arg_node->mask[1] = ReGenMask (arg_node->mask[1], VARNO);
    arg_node = OptTrav (arg_node, arg_info, 0);

    DBUG_ASSERT ((NULL != arg_node->node[1]), "loop without body");
    arg_node->node[1]->mask[0] = ReGenMask (arg_node->node[1]->mask[0], VARNO);
    arg_node->node[1]->mask[1] = ReGenMask (arg_node->node[1]->mask[1], VARNO);
    arg_node = OptTrav (arg_node, arg_info, 1);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GNMwith
 *  arguments     : 1) ptr to whith-node
 *                  2) ptr to info_node
 *                  R)
 *  description   :
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav, PlusMask, ReGenMask
 *  macros        : DBUG...
 *
 *  remarks       :
 *   - USE mask of operator (array/shape/neutral elt) is stored in operator
 *     node (N_modarray,...)
 *   - USE and DEF masks of body are stored in N_with node.
 *   - USE mask of generator is stored in N_generator node.
 *   - Afterwards DEF mask in generator is modified for index variables.
 *
 */

node *
GNMwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMwith");

    DBUG_ASSERT ((NULL != arg_node->node[1]), "with-expr. without genarray or modarray");
    DBUG_ASSERT ((NULL != arg_node->node[1]->node[0]),
                 "genarray or modarray without array ");

    /* initiate N_"operator" masks */
    WITH_OPERATOR (arg_node)->mask[0]
      = ReGenMask (WITH_OPERATOR (arg_node)->mask[0], VARNO);
    WITH_OPERATOR (arg_node)->mask[1]
      = ReGenMask (WITH_OPERATOR (arg_node)->mask[1], VARNO);
    arg_node = OptTrav (arg_node, arg_info, 1);

    /* initiate N_with masks */
    arg_node->mask[0] = ReGenMask (arg_node->mask[0], VARNO);
    arg_node->mask[1] = ReGenMask (arg_node->mask[1], VARNO);
    arg_node = OptTrav (arg_node, arg_info, 2);

    DBUG_ASSERT (arg_node->node[0], "with-expr. without generator.");
    DBUG_ASSERT (arg_node->node[0]->info.ids->node,
                 "N_generator without pointer to declaration.");

    /* initiate N_generator masks */
    WITH_GEN (arg_node)->mask[0] = ReGenMask (arg_node->node[0]->mask[0], VARNO);
    WITH_GEN (arg_node)->mask[1] = GenMask (VARNO);
    /* srs: why not ReGenMask()? compare to GNMNwith.
       I guess we lose the pointer to the old mask here. But maybe we
       have two pointers and lose only one, so that the GenMask() is ok??? */
    arg_node = OptTrav (arg_node, arg_info, 0);

    INC_VAR (arg_node->node[0]->mask[0], arg_node->node[0]->info.ids->node->varno);
    INC_VAR (arg_info->mask[0], arg_node->node[0]->info.ids->node->varno);

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
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav, PlusMask, ReGenMask
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

node *
GNMcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMcond");

    arg_node->mask[1] = ReGenMask (arg_node->mask[1], VARNO);
    arg_node = OptTrav (arg_node, arg_info, 0);

    DBUG_ASSERT ((NULL != arg_node->node[1]), "cond without then-body");
    arg_node->node[1]->mask[0] = ReGenMask (arg_node->node[1]->mask[0], VARNO);
    arg_node->node[1]->mask[1] = ReGenMask (arg_node->node[1]->mask[1], VARNO);
    arg_node = OptTrav (arg_node, arg_info, 1);

    DBUG_ASSERT ((NULL != arg_node->node[2]), "cond without else-body");
    arg_node->node[2]->mask[0] = ReGenMask (arg_node->node[2]->mask[0], VARNO);
    arg_node->node[2]->mask[1] = ReGenMask (arg_node->node[2]->mask[1], VARNO);
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
 *  global vars   : syntax_tree, info_node
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */

node *
GNMblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("GNMblock");
    if (BLOCK_INSTR (arg_node))
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *GNMNwith(node *arg_node, node *arg_info)
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
GNMNwith (node *arg_node, node *arg_info)
{
    node *code, *part, *withop;
    ids *_ids;

    DBUG_ENTER ("GNMNwith");

    /* withop */
    withop = NWITH_WITHOP (arg_node);
    withop->mask[0] = ReGenMask (withop->mask[0], VARNO);
    withop->mask[1] = ReGenMask (withop->mask[1], VARNO);
    withop = OptTrav (withop, arg_info, 0);

    /* all bodies */
    code = NWITH_CODE (arg_node);
    while (code) {
        code->mask[0] = ReGenMask (code->mask[0], VARNO);
        code->mask[1] = ReGenMask (code->mask[1], VARNO);
        code = OptTrav (code, arg_info, 0);
        code = NCODE_NEXT (code);
    }

    /* all parts */
    part = NWITH_PART (arg_node);
    while (part) {
        part->mask[0] = ReGenMask (part->mask[0], VARNO);
        part->mask[1] = ReGenMask (part->mask[1], VARNO);
        /* traverse generator N_Ngenarator (only USE) */
        part = OptTrav (part, arg_info, 0);

        /* add the ids from N_Nwithid to DEFs */
        /*    DBUG_ASSERT(NPART_IDS(part), "NWITHID_IDS == NULL");  */
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
 * function:
 *   node *GNMicm(node *arg_node, node *arg_info)
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

    if (strcmp (ICM_NAME (arg_node), "ND_KS_VECT2OFFSET") == 0) {
        /*
         * ICM "ND_KS_VECT2OFFSET"
         */
        icm_arg = EXPRS_EXPR (ICM_ARGS (arg_node));
        INC_VAR (arg_info->mask[0], ID_VARNO (icm_arg));
    } else {
        /*
         * ICM "ND_KS_USE_GENVAR_OFFSET"
         */
        if (strcmp (ICM_NAME (arg_node), "ND_KS_USE_GENVAR_OFFSET") == 0) {
            icm_arg = EXPRS_EXPR (ICM_ARGS (arg_node));
            INC_VAR (arg_info->mask[0], ID_VARNO (icm_arg));
        } else {
            DBUG_ASSERT ((0), "unknown ICM found while mask-generation");
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
GenerateMasks (node *arg_node, node *arg_info)
{
    funptr *tmp_tab;

    DBUG_ENTER ("GenerateMasks");
    DBUG_PRINT ("OPT", ("GENERATEMASKS"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    tmp_tab = act_tab;
    act_tab = genmask_tab;
    if (NULL == arg_info) {
        arg_info = MakeInfo ();
        arg_node = Trav (arg_node, arg_info);
        FREE (arg_info);
    } else {
        /* arg_info is modified here so I guess it's better to avoid this branch. */
        arg_node = Trav (arg_node, arg_info);
    }

    act_tab = tmp_tab;
    DBUG_RETURN (arg_node);
}
