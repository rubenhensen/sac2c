/*
 *
 * $Log$
 * Revision 1.1  2000/01/21 12:48:59  dkr
 * Initial revision
 *
 *
 */

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"

/******************************************************************************
 *
 * function:
 *   char *MakeDummyFunName( char *suffix)
 *
 * description:
 *   Creates a new name for a dummy function. 'suffix' should be one of the
 *   strings "Cond", "Do" or "While".
 *
 ******************************************************************************/

char *
MakeDummyFunName (char *suffix)
{
    static int number = 0;
    char *funname;

    DBUG_ENTER ("MakeDummyFunName");

    funname = (char *)MALLOC ((sizeof (char) * (strlen (suffix) + number / 10 + 4)));
    sprintf (funname, "__%s%i", suffix, number);

    DBUG_RETURN (funname);
}

/******************************************************************************
 *
 * function:
 *   node *MakeDummyFunArgs( DFMmask_t in)
 *
 * description:
 *   Creates the argument list (N_arg node chain) for a dummy function.
 *
 ******************************************************************************/

node *
MakeDummyFunArgs (DFMmask_t in)
{
    node *decl, *tmp;
    node *args = NULL;

    DBUG_ENTER ("MakeDummyFunArgs");

    decl = DFMGetMaskEntryDeclSet (in);
    while (decl != NULL) {
        if (NODE_TYPE (decl) == N_arg) {
            tmp = args;
            args = DupNode (decl);
            ARG_NEXT (args) = tmp;
        } else {
            DBUG_ASSERT ((NODE_TYPE (decl) == N_vardec),
                         "mask entry is neither an arg nor a vardec.");
            args = MakeArg (StringCopy (VARDEC_NAME (decl)), VARDEC_TYPE (decl),
                            VARDEC_STATUS (decl), VARDEC_ATTRIB (decl), args);
        }
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *MakeDummyFunVardecs( DFMmask_t local)
 *
 * description:
 *   Creates the vardec chain (local vars) for a dummy function.
 *
 ******************************************************************************/

node *
MakeDummyFunVardecs (DFMmask_t local)
{
    node *decl, *tmp;
    node *vardecs = NULL;

    DBUG_ENTER ("MakeDummyFunVardecs");

    decl = DFMGetMaskEntryDeclSet (local);
    while (decl != NULL) {
        DBUG_ASSERT ((NODE_TYPE (decl) != N_arg), "an arg can not be a local var.");
        DBUG_ASSERT ((NODE_TYPE (decl) == N_vardec), "vardec of local var not found.");
        tmp = DupNode (decl);
        VARDEC_NEXT (tmp) = vardecs;
        vardecs = tmp;
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *   node *MakeDummyFunReturn( DFMmask_t out)
 *
 * description:
 *   Creates the return node for a dummy function.
 *
 ******************************************************************************/

node *
MakeDummyFunReturn (DFMmask_t out)
{
    node *decl;
    node *ret = NULL;

    DBUG_ENTER ("MakeDummyFunReturn");

    decl = DFMGetMaskEntryDeclSet (out);
    while (decl != NULL) {
        ret = MakeExprs (MakeId1 (VARDEC_OR_ARG_NAME (decl)), ret);
        decl = DFMGetMaskEntryDeclSet (NULL);
    }
    ret = MakeReturn (ret);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *MakeDummyFunLet( char *funname, DFMmask_t in, DFMmask_t out)
 *
 * description:
 *   Creates the let node containing the call of a dummy function.
 *
 ******************************************************************************/

node *
MakeDummyFunLet (char *funname, DFMmask_t in, DFMmask_t out)
{
    node *decl, *let;
    ids *ids, *tmp;

    DBUG_ENTER ("MakeDummyFunLet");

    decl = DFMGetMaskEntryDeclSet (out);
    while (decl != NULL) {
        tmp = MakeIds1 (VARDEC_OR_ARG_NAME (decl));
        IDS_NEXT (tmp) = ids;
        ids = tmp;
        decl = DFMGetMaskEntryDeclSet (NULL);
    }
#if 0
  let = MakeLet( MakeAp( StringCopy( funname), NULL, ),
                 ids);
#endif

    DBUG_RETURN (let);
}

/******************************************************************************
 *
 * function:
 *   void DefinedVar( char *id, node *decl,
 *                    DFMmask_t needed,
 *                    DFMmask_t *in, DFMmask_t *out, DFMmask_t *local)
 *
 * description:
 *   Updates the given in-, out-, local-masks according to a defining occurence
 *   of a variable. The variable can either be specified directly (id) or as
 *   a pointer to the declaration (decl). The unused parameter has to be NULL.
 *
 ******************************************************************************/

void
DefinedVar (char *id, node *decl, DFMmask_t needed, DFMmask_t *in, DFMmask_t *out,
            DFMmask_t *local)
{
    DBUG_ENTER ("DefinedVar");

    DFMSetMaskEntryClear (*in, id, decl);
    if (DFMTestMaskEntry (needed, id, decl)) {
        DFMSetMaskEntrySet (*out, id, decl);
    } else {
        DFMSetMaskEntrySet (*local, id, decl);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void UsedVar( char *id, node *decl,
 *                 DFMmask_t *in, DFMmask_t *out, DFMmask_t *local)
 *
 * description:
 *   Updates the given in-, out-, local-masks according to a non-defining
 *   occurence of a variable. The variable can either be specified directly (id)
 *   or as a pointer to the declaration (decl). The unused parameter has to be
 *   NULL.
 *
 ******************************************************************************/

void
UsedVar (char *id, node *decl, DFMmask_t *in, DFMmask_t *local)
{
    DBUG_ENTER ("UsedVar");

    DFMSetMaskEntrySet (*in, id, decl);
    DFMSetMaskEntryClear (*local, id, decl);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void AdjustNeeded( DFMmask_t *needed,
 *                      DFMmask_t in, DFMmask_t out)
 *
 * description:
 *   Updates the needed-mask (contains all vars, that are needed in outer
 *   blocks) according to the in- and out-mask of the current block.
 *   This function is needed to calculate the new needed-mask before entering
 *   a new block.
 *
 ******************************************************************************/

void
AdjustNeeded (DFMmask_t *needed, DFMmask_t in, DFMmask_t out)
{
    DBUG_ENTER ("AdjustNeeded");

    DFMSetMaskMinus (needed, out);
    DFMSetMaskOr (needed, in);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   All DFM-masks needed during traversal of the fundef are build before
 *   and removed afterwards.
 *
 ******************************************************************************/

node *
LAC2FUNfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNfundef");

    INFO_LAC2FUN_DFMBASE (arg_info)
      = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

    INFO_LAC2FUN_IN (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_NEEDED (arg_info) = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_ISTRANS (arg_info) = 0;

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    INFO_LAC2FUN_IN (arg_info) = DFMRemoveMask (INFO_LAC2FUN_IN (arg_info));
    INFO_LAC2FUN_OUT (arg_info) = DFMRemoveMask (INFO_LAC2FUN_OUT (arg_info));
    INFO_LAC2FUN_LOCAL (arg_info) = DFMRemoveMask (INFO_LAC2FUN_LOCAL (arg_info));
    INFO_LAC2FUN_NEEDED (arg_info) = DFMRemoveMask (INFO_LAC2FUN_NEEDED (arg_info));

    INFO_LAC2FUN_DFMBASE (arg_info) = DFMRemoveMaskBase (INFO_LAC2FUN_DFMBASE (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNassign( node *arg_node, node *arg_info)
 *
 * description:
 *   bottom-up-traversal of the assignments.
 *   INFO_LAC2FUN_ISTRANS indicates whether an assignment has been transformed
 *   into a function call or not.
 *   If (INFO_LAC2FUN_ISTRANS > 0) is hold, the assign-node has been modificated
 *   and must be correctly inserted into the AST.
 *
 ******************************************************************************/

node *
LAC2FUNassign (node *arg_node, node *arg_info)
{
    node *assign_next;

    DBUG_ENTER ("LAC2FUNassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        assign_next = Trav (ASSIGN_NEXT (arg_node), arg_info);
        if (INFO_LAC2FUN_ISTRANS (arg_info)) {
            ASSIGN_NEXT (assign_next) = ASSIGN_NEXT (ASSIGN_NEXT (arg_node));
            ASSIGN_NEXT (arg_node) = assign_next;
            INFO_LAC2FUN_ISTRANS (arg_info) = 0;
        }
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNlet( node *arg_node, node *arg_info)
 *
 * description:
 *   every left hand side variable is marked as 'defined' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
LAC2FUNlet (node *arg_node, node *arg_info)
{
    ids *ids;

    DBUG_ENTER ("LAC2FUNlet");

    ids = LET_IDS (arg_node);
    while (ids != NULL) {
        DefinedVar (NULL, IDS_VARDEC (ids), INFO_LAC2FUN_NEEDED (arg_info),
                    &(INFO_LAC2FUN_IN (arg_info)), &(INFO_LAC2FUN_OUT (arg_info)),
                    &(INFO_LAC2FUN_LOCAL (arg_info)));
        ids = IDS_NEXT (ids);
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNid( node *arg_node, node *arg_info)
 *
 * description:
 *   every right hand side variable is marked as 'used' in the in-, out-,
 *   local-masks of the current block.
 *
 ******************************************************************************/

node *
LAC2FUNid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNid");

    UsedVar (NULL, ID_VARDEC (arg_node), &(INFO_LAC2FUN_IN (arg_info)),
             &(INFO_LAC2FUN_LOCAL (arg_info)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNcond( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
LAC2FUNcond (node *arg_node, node *arg_info)
{
    DFMmask_t needed, tmp;
    DFMmask_t old_in, old_out, old_local;
    DFMmask_t in_then, out_then, local_then;
    DFMmask_t in_else, out_else, local_else;
    char *funname;
    node *fundef;

    DBUG_ENTER ("LAC2FUNcond");

    needed = DFMGenMaskCopy (INFO_LAC2FUN_NEEDED (arg_info));
    AdjustNeeded (needed, INFO_LAC2FUN_IN (arg_info), INFO_LAC2FUN_OUT (arg_info));

    old_in = INFO_LAC2FUN_IN (arg_info);
    old_out = INFO_LAC2FUN_OUT (arg_info);
    old_local = INFO_LAC2FUN_LOCAL (arg_info);

    INFO_LAC2FUN_IN (arg_info) = in_then
      = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_OUT (arg_info) = out_then
      = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_LOCAL (arg_info) = local_then
      = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    INFO_LAC2FUN_IN (arg_info) = in_else
      = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_OUT (arg_info) = out_else
      = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));
    INFO_LAC2FUN_LOCAL (arg_info) = local_else
      = DFMGenMaskClear (INFO_LAC2FUN_DFMBASE (arg_info));

    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    /* in = in_then u in_else u (out_then \ out_else) u (out_else \ out_then) */
    INFO_LAC2FUN_IN (arg_info) = DFMGenMaskMinus (out_then, out_else);
    tmp = DFMGenMaskMinus (out_else, out_then);
    DFMSetMaskOr (INFO_LAC2FUN_IN (arg_info), tmp);
    DFMSetMaskOr (INFO_LAC2FUN_IN (arg_info), in_then);
    DFMSetMaskOr (INFO_LAC2FUN_IN (arg_info), in_else);

    /* out = out_then u out_else */
    INFO_LAC2FUN_OUT (arg_info) = DFMGenMaskOr (out_then, out_else);

    /* local = local_then n local_else */
    INFO_LAC2FUN_LOCAL (arg_info) = DFMGenMaskAnd (local_then, local_else);

    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    funname = MakeDummyFunName ("Cond");
#if 0
  fundef = MakeFundef(
             funname, NULL,
             types,
             MakeDummyFunArgs( INFO_LAC2FUN_IN( arg_info)),
             MakeBlock(
               MakeAssign( arg_node,
                           MakeDummyFunReturn( INFO_LAC2FUN_OUT( arg_info))),
               MakeDummyFunVardecs( INFO_LAC2FUN_LOCAL( arg_info))),
             NULL);
#endif
    FUNDEF_STATUS (fundef) = ST_condfun;
    arg_node = MakeDummyFunLet (funname, INFO_LAC2FUN_IN (arg_info),
                                INFO_LAC2FUN_OUT (arg_info));
    INFO_LAC2FUN_ISTRANS (arg_info) = 1;

    arg_node = Trav (arg_node, arg_info);

    needed = DFMRemoveMask (needed);
    tmp = DFMRemoveMask (tmp);
    in_then = DFMRemoveMask (in_then);
    out_then = DFMRemoveMask (out_then);
    local_then = DFMRemoveMask (local_then);
    in_else = DFMRemoveMask (in_else);
    out_else = DFMRemoveMask (out_else);
    local_else = DFMRemoveMask (local_else);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNdo( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
LAC2FUNdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNdo");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LAC2FUNwhile( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
LAC2FUNwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("LAC2FUNwhile");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *lac2fun( node *syntax_tree)
 *
 * description:
 *   converts all loops and conditions into (annotated) functions.
 *
 ******************************************************************************/

node *
LaC2Fun (node *syntax_tree)
{
    node *info_node;

    DBUG_ENTER ("LaC2Fun");

    act_tab = lac2fun_tab;
    info_node = MakeInfo ();

    syntax_tree = Trav (syntax_tree, info_node);

    DBUG_RETURN (syntax_tree);
}
