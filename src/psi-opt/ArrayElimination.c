/*
 *
 * $Log$
 * Revision 3.14  2001/12/11 15:55:31  dkr
 * GetDim() renamed into GetShapeDim()
 *
 * Revision 3.13  2001/07/18 12:57:45  cg
 * Applications of old tree construction function
 * AppendNodeChain eliminated.
 *
 * Revision 3.12  2001/07/16 08:23:11  cg
 * Old tree construction function MakeNode eliminated.
 *
 * Revision 3.11  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.10  2001/05/17 13:40:26  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.9  2001/05/15 08:03:59  nmw
 * remove call to OPTTrav when using ssa form
 *
 * Revision 3.8  2001/05/09 15:51:42  nmw
 * when using ssa form, ArrayElimination does not need masks anymore
 *
 * Revision 3.7  2001/05/02 06:56:25  nmw
 * init of constant arrays completed
 *
 * Revision 3.6  2001/04/30 12:16:59  nmw
 * integrate traversal of special fundefs in ArrayElimination traversal
 *
 * Revision 3.5  2001/04/18 13:42:30  dkr
 * bug in AEassign fixed: LHS might be void!
 *
 * [...]
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"

#include "optimize.h"
#include "generatemasks.h"
#include "LoopInvariantRemoval.h"
#include "ArrayElimination.h"

#define AE_PREFIX "__ae_"
#define AE_PREFIX_LENGTH 5

/*
 * macro to switch between OPTTrav and std. Trav function used in ssa form.
 * in future when only ssa form is used, the OPTTrav part can be removed.
 */
#define SELTRAV(ssa, subnode, info, node)                                                \
    (ssa ? Trav (subnode, info) : OPTTrav (subnode, info, node))

/*
 *
 *  functionname  : ArrayElimination
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr optimized 1)
 *  description   : initiates array elimination for the intermediate sac-code:
 *  global vars   : syntax_tree, act_tab, ae_tab
 *
 */
node *
ArrayElimination (node *arg_node, node *info_node)
{
    funtab *tmp_tab;
#ifndef DBUG_OFF
    int mem_elim_arrays = elim_arrays;
#endif

    DBUG_ENTER ("ArrayElimination");
    DBUG_PRINT ("OPT", ("ARRAY ELIMINATION"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "ArrayElimination() called for non-fundef node");

    if (!(FUNDEF_IS_LACFUN (arg_node))) {
        tmp_tab = act_tab;
        act_tab = ae_tab;
        info_node = MakeInfo ();

        arg_node = Trav (arg_node, info_node);

        info_node = FreeTree (info_node);
        act_tab = tmp_tab;
        DBUG_PRINT ("OPT", ("                        result: %d",
                            elim_arrays - mem_elim_arrays));
    }

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
    dim = GetShapeDim (type);

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
 *  functionname  : GenSel
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
GenSel (ids *ids_node, node *arg_info)
{
    node *new_nodes = NULL, *new_let, *new_assign, *new_vardec;
    node *exprn;
    int length, i;
    types *type;
    node *arg[2];

    DBUG_ENTER ("GenSel");
    type = IDS_TYPE (ids_node);
    ;
    length = GetTypesLength (type);
    for (i = 0; i < length; i++) {
        exprn = MakeExprs (MakeNum (i), NULL);
        arg[0] = MakeArray (exprn);
        ((int *)ARRAY_CONSTVEC (arg[0])) = Array2IntVec (exprn, NULL);
        /* srs: AE only works on arrays which have 1 dimension.
           type attribut was missing here. */
        ARRAY_ISCONST (arg[0]) = TRUE;
        ARRAY_VECTYPE (arg[0]) = T_int;
        ARRAY_VECLEN (arg[0]) = 1;
        ARRAY_TYPE (arg[0])
          = MakeTypes (T_int, 1, MakeShpseg (MakeNums (1, NULL)), NULL, NULL);
#if 0    
    arg[1] = MakeNode(N_id);
    arg[1]->info.ids = DupAllIds( ids_node);
#else
        arg[1] = MakeId (StringCopy (IDS_NAME (ids_node)), NULL, ST_regular);
        ID_VARDEC (arg[1]) = IDS_VARDEC (ids_node);
        ID_AVIS (arg[1]) = VARDEC_OR_ARG_AVIS (ID_VARDEC (arg[1]));
#endif
        new_let = MakeLet (NULL, GenIds (arg));

        DBUG_PRINT ("AE", ("Generating new value for %s", new_let->info.ids->id));
        IDS_VARDEC (LET_IDS (new_let))
          = SearchDecl (new_let->info.ids->id, INFO_AE_TYPES (arg_info));

        if (IDS_VARDEC (LET_IDS (new_let)) == NULL) {
            DBUG_PRINT ("AE", ("Generating new vardec for %s", new_let->info.ids->id));
            new_vardec = MakeVardec (StringCopy (IDS_NAME (LET_IDS (new_let))),
                                     DupTypes (GetTypes (type)), NULL);
            VARDEC_DIM (new_vardec) = 0;
            INFO_AE_TYPES (arg_info)
              = AppendVardec (new_vardec, INFO_AE_TYPES (arg_info));
            IDS_VARDEC (LET_IDS (new_let)) = new_vardec;
        }
        LET_EXPR (new_let)
          = MakePrf (F_sel, MakeExprs (arg[0], MakeExprs (arg[1], NULL)));
        new_assign = MakeAssign (new_let, NULL);
        new_nodes = AppendAssign (new_nodes, new_assign);
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

    if (F_sel == PRF_PRF (arg_node)) {
        tmpn = NodeBehindCast (PRF_ARG1 (arg_node));
        if (N_id == NODE_TYPE (tmpn)) {
            if (use_ssaform) {
                /* look up via ssa assign attribute */
                if (AVIS_SSAASSIGN (ID_AVIS (tmpn)) != NULL) {
                    DBUG_PRINT ("AE", ("defining assignment looked up via SSAASSIGN"));
                    tmpn = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (tmpn)));
                } else {
                    tmpn = NULL;
                }
            } else {
                /* old version - use MRD masks - can be removed in future... */
                tmpn = MRD_GETDATA (ID_VARNO (tmpn), INFO_VARNO (arg_info));
            }
        }

        arg[0] = tmpn;
        arg[1] = NodeBehindCast (PRF_ARG2 (arg_node));

        /* srs: added IsConstArray() so that sel([i],arr) is not replaced.
                This led to wrong programs. */
        if (N_id == NODE_TYPE (arg[1]) && tmpn && N_array == NODE_TYPE (tmpn)
            && IsConstArray (tmpn) && CorrectArraySize (ID_IDS (arg[1]))) {
            DBUG_PRINT ("AE", ("sel function with array %s to eliminated found",
                               arg[1]->info.ids->id));
            new_node = MakeId (NULL, NULL, ST_regular);
            ID_IDS (new_node) = GenIds (arg);
            ID_VARDEC (new_node)
              = SearchDecl (ID_NAME (new_node), INFO_AE_TYPES (arg_info));
            if (ID_VARDEC (new_node)) {
                FreeTree (arg_node);
                arg_node = new_node;
            } else {
                FreeTree (new_node);
            }
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

    INFO_AE_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) && (!FUNDEF_INLINE (arg_node))) {
        DBUG_PRINT ("AE", ("*** Trav function %s", FUNDEF_NAME (arg_node)));

        INFO_AE_TYPES (arg_info) = NULL;
        FUNDEF_INSTR (arg_node)
          = SELTRAV (use_ssaform, FUNDEF_INSTR (arg_node), arg_info, arg_node);
        FUNDEF_VARDEC (arg_node)
          = AppendVardec (INFO_AE_TYPES (arg_info), FUNDEF_VARDEC (arg_node));
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
 *
 ******************************************************************************/

node *
AEassign (node *arg_node, node *arg_info)
{
    node *new_nodes = NULL;
    ids *_ids;

    DBUG_ENTER ("AEassign");

    /* create new assignments if array is smaler than threshold */
    if (N_let == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        _ids = LET_IDS (ASSIGN_INSTR (arg_node));
        if ((_ids != NULL) && (IDS_NEXT (_ids) == NULL)) {
            if (CorrectArraySize (_ids)) {
                VARDEC_FLAG (IDS_VARDEC (_ids)) = TRUE;
                new_nodes = GenSel (_ids, arg_info);
            } else
                VARDEC_FLAG (IDS_VARDEC (_ids)) = FALSE;
        }
    }

    /* traverse into compound nodes. */
    if (ASSIGN_INSTR (arg_node))
        ASSIGN_INSTR (arg_node)
          = SELTRAV (use_ssaform, ASSIGN_INSTR (arg_node), arg_info, arg_node);
    if (ASSIGN_NEXT (arg_node))
        ASSIGN_NEXT (arg_node)
          = SELTRAV (use_ssaform, ASSIGN_NEXT (arg_node), arg_info, arg_node);

    if (new_nodes)
        ASSIGN_NEXT (arg_node) = AppendAssign (new_nodes, ASSIGN_NEXT (arg_node));

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

    COND_COND (arg_node)
      = SELTRAV (use_ssaform, COND_COND (arg_node), arg_info, arg_node);
    COND_THENINSTR (arg_node)
      = SELTRAV (use_ssaform, COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node)
      = SELTRAV (use_ssaform, COND_ELSEINSTR (arg_node), arg_info, arg_node);

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

    DO_INSTR (arg_node) = SELTRAV (use_ssaform, DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = SELTRAV (use_ssaform, DO_COND (arg_node), arg_info, arg_node);

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

    WHILE_COND (arg_node)
      = SELTRAV (use_ssaform, WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node)
      = SELTRAV (use_ssaform, WHILE_INSTR (arg_node), arg_info, arg_node);

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
    NWITH_PART (arg_node)
      = SELTRAV (use_ssaform, NWITH_PART (arg_node), arg_info, arg_node);
    NWITH_CODE (arg_node)
      = SELTRAV (use_ssaform, NWITH_CODE (arg_node), arg_info, arg_node);
    NWITH_WITHOP (arg_node)
      = SELTRAV (use_ssaform, NWITH_WITHOP (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AEap(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal of implicit inlined special fundef
 *
 ******************************************************************************/

node *
AEap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("AEap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* non-recursive call of special fundef */
    if ((AP_FUNDEF (arg_node) != NULL) && (FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (INFO_AE_FUNDEF (arg_info) != AP_FUNDEF (arg_node))) {

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        new_arg_info = FreeTree (new_arg_info);
    }
    DBUG_RETURN (arg_node);
}
