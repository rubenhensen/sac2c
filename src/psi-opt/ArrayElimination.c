/*
 *
 * $Log$
 * Revision 2.15  2000/10/24 11:53:55  dkr
 * MakeType renamed into MakeTypes
 *
 * Revision 2.14  2000/10/23 18:39:47  dkr
 * AEfundef:
 * superfluous assignment to arg_node->refcnt removed
 *
 * Revision 2.13  2000/10/20 15:37:07  dkr
 * macro GET_DIM and GET_LENGTH replaced by functions GetDim and
 * GetTypesLength
 *
 * Revision 2.12  2000/07/14 11:34:41  dkr
 * FUNDEF_INLINE==0 replaced by !FUNDEF_INLINE
 *
 * Revision 2.11  2000/07/14 09:35:17  dkr
 * DupIds called with (arg_info = NULL) now
 *
 * Revision 2.10  2000/07/12 15:10:19  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 2.9  2000/06/13 12:32:14  dkr
 * function for old with-loop removed
 *
 * Revision 2.8  2000/01/26 17:25:46  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.7  1999/11/15 18:06:13  dkr
 * VARNO replaced, INFO_VARNO with changed signature
 *
 * Revision 2.6  1999/11/11 20:05:11  dkr
 * signature and name of function IsConstantArray changed
 *
 * Revision 2.5  1999/05/12 11:39:24  jhs
 * Adjusted macros to new access on constant vectors.
 *
 * Revision 2.4  1999/04/19 11:29:18  jhs
 * TRUE and FALSE from internal_lib.h ared used from now on.
 *
 * Revision 2.3  1999/03/31 15:09:00  bs
 * I did some code cosmetics with the MRD_GET... macros.
 *
 * Revision 2.2  1999/03/17 20:37:13  bs
 * Function GenPsi modified: arrays with compact vector propagation will be generated.
 *
 * Revision 2.1  1999/02/23 12:43:08  sacbase
 * new release made
 *
 * Revision 1.14  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.13  1998/08/20 12:06:14  srs
 * Fixed bug in AEPrf():
 * Too many psi-functions were replaced.
 *
 * Revision 1.12  1998/05/26 13:13:24  srs
 * fixed bug in GenPsi(): ARRAY_TYPE was not initialized
 *
 * Revision 1.11  1998/05/12 19:55:53  srs
 * fixed bug in GenPsi()
 *
 * Revision 1.10  1998/05/12 16:11:43  dkr
 * fixed a typo in GenPsi: MakeExpr -> MakeExprs
 *
 * Revision 1.9  1998/05/12 15:43:37  srs
 * added MRD support to enable substitution of
 *   i = [0];
 *   a = psi(i, A);
 *
 * Revision 1.8  1998/03/25 17:53:11  srs
 * added comment to a location of a possible bug
 *
 * Revision 1.7  1997/11/23 17:18:06  dkr
 * removed a bug in GenIds():
 * - malloc-size for new string corrected
 *
 * Revision 1.6  1997/11/04 13:25:20  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.5  1997/04/25 12:13:00  sbs
 * MAlloc replaced by Malloc from internal.lib
 *
 * Revision 1.4  1996/07/16  15:26:40  asi
 * macros from access_macros.h no longer used
 *
 * Revision 1.3  1996/01/17  14:17:52  asi
 * added globals.h
 *
 * Revision 1.2  1995/10/06  17:07:47  cg
 * adjusted calls to function MakeIds (now 3 parameters)
 *
 * Revision 1.1  1995/07/24  10:00:19  asi
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "typecheck.h" /* macro GET_BASIC_TYPE */
#include "free.h"
#include "DupTree.h"
#include "tree.h"
#include "internal_lib.h"

#include "optimize.h"
#include "generatemasks.h"
#include "LoopInvariantRemoval.h"
#include "ArrayElimination.h"

#define AE_PREFIX "__ae_"
#define AE_PREFIX_LENGTH 5

/*
 *
 *  functionname  : ArrayElimination
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr optimized 1)
 *  description   : initiates array elimination for the intermediate sac-code:
 *  global vars   : syntax_tree, act_tab, ae_tab
 *  internal funs : ---
 *  external funs : Trav, MakeNode
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 *
 */
node *
ArrayElimination (node *arg_node, node *info_node)
{
    funtab *tmp_tab;
    int mem_elim_arrays = elim_arrays;

    DBUG_ENTER ("ArrayElimination");
    DBUG_PRINT ("OPT", ("ARRAY ELIMINATION"));

    tmp_tab = act_tab;
    act_tab = ae_tab;
    info_node = MakeNode (N_info);

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    act_tab = tmp_tab;
    DBUG_PRINT ("OPT",
                ("                        result: %d", elim_arrays - mem_elim_arrays));
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CorrectArraySize
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
int
CorrectArraySize (ids *ids_node)
{
    int answer = FALSE;
    int length, dim;
    types *type;

    DBUG_ENTER ("CorrectArraySize");

    type = IDS_TYPE (ids_node);
    length = GetTypesLength (type);
    dim = GetDim (type);

    if ((length <= minarray) && (0 != length) && (1 == dim)) {
        DBUG_PRINT ("AE", ("array %s with length %d to eliminated found",
                           ids_node->node->info.types->id, length));
        answer = TRUE;
    }
    DBUG_RETURN (answer);
}

/*
 *
 *  functionname  : GetNumber
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
char *
GetNumber (node *vector)
{
    char *number, *tmp;
    node *expr_node;

    DBUG_ENTER ("GetNumber");
    number = (char *)Malloc (sizeof (char) * ((minarray / 10) + 2));
    number[0] = atoi ("\0");
    expr_node = vector->node[0];
    do {
        tmp = itoa (NUM_VAL (EXPRS_EXPR (expr_node)));
        strcat (number, tmp);
        expr_node = EXPRS_NEXT (expr_node);

        /* srs: we already have assured that the shape has only one element
                (CorrectArraySize). Else we allocated to little memory
                for 'number'. */
        DBUG_ASSERT (expr_node == NULL, ("to many arguments in GetNumber"));

    } while (expr_node);
    DBUG_RETURN (number);
}

/*
 *
 *  functionname  : GenIds
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
ids *
GenIds (node *arg[2])
{
    char *number, *new_name, *old_name;

    DBUG_ENTER ("GenIds");
    number = GetNumber (arg[0]);
    old_name = ID_NAME (arg[1]);
    new_name = (char *)Malloc (
      sizeof (char) * (strlen (old_name) + strlen (number) + AE_PREFIX_LENGTH + 1));
    sprintf (new_name, AE_PREFIX "%s%s", number, old_name);
    DBUG_RETURN (MakeIds (new_name, NULL, ST_regular));
}

/*
 *
 *  functionname  : GenPsi
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
GenPsi (ids *ids_node, node *arg_info)
{
    node *new_nodes = NULL, *new_let, *new_assign, *new_vardec;
    node *exprn;
    int length, i;
    types *type;
    node *arg[2];

    DBUG_ENTER ("GenPsi");
    type = IDS_TYPE (ids_node);
    ;
    length = GetTypesLength (type);
    for (i = 0; i < length; i++) {
        exprn = MakeExprs (MakeNum (i), NULL);
        arg[0] = MakeArray (exprn);
        ((int *)ARRAY_CONSTVEC (arg[0])) = Array2IntVec (exprn, NULL);
        /* srs: AE only works on arrays which have 1 dimension.
           type attribut was missing here. */
        ARRAY_VECTYPE (arg[0]) = T_int;
        ARRAY_VECLEN (arg[0]) = 1;
        ARRAY_TYPE (arg[0])
          = MakeTypes (T_int, 1, MakeShpseg (MakeNums (1, NULL)), NULL, NULL);

        arg[1] = MakeNode (N_id);
        arg[1]->info.ids = DupIds (ids_node, NULL);

        new_let = MakeNode (N_let);
        new_let->info.ids = GenIds (arg);
        DBUG_PRINT ("AE", ("Generating new value for %s", new_let->info.ids->id));
        IDS_VARDEC (LET_IDS (new_let))
          = SearchDecl (new_let->info.ids->id, INFO_AE_TYPES (arg_info));

        if (!IDS_VARDEC (LET_IDS (new_let))) {
            DBUG_PRINT ("AE", ("Generating new vardec for %s", new_let->info.ids->id));
            new_vardec = MakeNode (N_vardec);
            /* srs: something is wrong here and we lose memory.
               GET_BASIC_TYPES always calls DupTypes(). */
            GET_BASIC_TYPE (new_vardec->info.types, type, 0);
            /* srs: and here in the next line we yank it without setting it free.
               This is wrong since DupTypes ALWAYS allocetes new mem. */
            new_vardec->info.types = DupTypes (new_vardec->info.types);
            new_vardec->info.types->dim = 0;
            FREE (new_vardec->info.types->id);
            new_vardec->info.types->id = StringCopy (new_let->info.ids->id);
            INFO_AE_TYPES (arg_info)
              = AppendNodeChain (0, new_vardec, INFO_AE_TYPES (arg_info));
            new_let->info.ids->node = new_vardec;
        }
        LET_EXPR (new_let)
          = MakePrf (F_psi, MakeExprs (arg[0], MakeExprs (arg[1], NULL)));
        new_assign = MakeAssign (new_let, NULL);
        new_nodes = AppendNodeChain (1, new_nodes, new_assign);
    }
    DBUG_RETURN (new_nodes);
}

/*
 *
 *  functionname  : AEprf
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
AEprf (node *arg_node, node *arg_info)
{
    node *arg[2], *new_node, *tmpn;

    DBUG_ENTER ("AEprf");

    if (F_psi == PRF_PRF (arg_node)) {
        tmpn = NodeBehindCast (PRF_ARG1 (arg_node));
        if (N_id == NODE_TYPE (tmpn))
            tmpn = MRD_GETDATA (ID_VARNO (tmpn), INFO_VARNO (arg_info));

        arg[0] = tmpn;
        arg[1] = NodeBehindCast (PRF_ARG2 (arg_node));

        /* srs: added IsConstArray() so that psi([i],arr) is not replaced.
                This led to wrong programs. */
        if (N_id == NODE_TYPE (arg[1]) && tmpn && N_array == NODE_TYPE (tmpn)
            && IsConstArray (tmpn) && CorrectArraySize (ID_IDS (arg[1]))) {
            DBUG_PRINT ("AE", ("psi function with array %s to eliminated found",
                               arg[1]->info.ids->id));
            new_node = MakeNode (N_id);
            ID_IDS (new_node) = GenIds (arg);
            ID_VARDEC (new_node)
              = SearchDecl (ID_NAME (new_node), INFO_AE_TYPES (arg_info));
            if (ID_VARDEC (new_node)) {
                FreeTree (arg_node);
                arg_node = new_node;
            } else
                FreeTree (new_node);
        }
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
AEfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AEfundef");

    if (FUNDEF_BODY (arg_node) && (!FUNDEF_INLINE (arg_node))) {
        DBUG_PRINT ("AE", ("*** Trav function %s", FUNDEF_NAME (arg_node)));

        INFO_AE_TYPES (arg_info) = NULL;
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);
        FUNDEF_VARDEC (arg_node)
          = AppendNodeChain (0, INFO_AE_TYPES (arg_info), FUNDEF_VARDEC (arg_node));
        INFO_AE_TYPES (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
AEassign (node *arg_node, node *arg_info)
{
    node *new_nodes = NULL;
    ids *ids_node;

    DBUG_ENTER ("AEassign");
    /* create new assignments if array is smaler than threshold */
    if (N_let == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        ids_node = LET_IDS (ASSIGN_INSTR (arg_node));
        if (!IDS_NEXT (ids_node)) {
            if (CorrectArraySize (ids_node)) {
                VARDEC_FLAG (IDS_VARDEC (ids_node)) = TRUE;
                new_nodes = GenPsi (ids_node, arg_info);
            } else
                VARDEC_FLAG (IDS_VARDEC (ids_node)) = FALSE;
        }
    }

    /* traverse into compound nodes. */
    if (ASSIGN_INSTR (arg_node))
        ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
    if (ASSIGN_NEXT (arg_node))
        ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);

    if (new_nodes)
        ASSIGN_NEXT (arg_node) = AppendNodeChain (1, new_nodes, ASSIGN_NEXT (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEcond(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
AEcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AEcond");

    COND_COND (arg_node) = OPTTrav (COND_COND (arg_node), arg_info, arg_node);
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEdo(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
AEdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AEdo");

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
AEwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AEwhile");

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AENwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
AENwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AENwith");

    /* The Phase AE is not in the optimization loop and though
       WLT has not been done. Only one N_Npart and one N_Ncode exist. */
    NWITH_PART (arg_node) = OPTTrav (NWITH_PART (arg_node), arg_info, arg_node);
    NWITH_CODE (arg_node) = OPTTrav (NWITH_CODE (arg_node), arg_info, arg_node);
    NWITH_WITHOP (arg_node) = OPTTrav (NWITH_WITHOP (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}
