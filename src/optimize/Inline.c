/*
 *
 * $Log$
 * Revision 3.38  2004/11/21 11:22:03  sah
 * removed some old ast infos
 *
 * Revision 3.37  2004/10/28 22:08:11  sah
 * fixed some typoes
 *
 * Revision 3.36  2004/10/28 18:40:59  sah
 * Inlining is not performed on used functions
 * from another module.
 *
 * Revision 3.35  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 3.34  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.33  2003/09/11 17:43:17  sbs
 * Inlining of wrappers made optional by INLINE_WRAPPERS.
 * Preset INLINE_WRAPPERS to 0, i.e., wrapper-inlining turned off!!
 *
 * Revision 3.32  2003/06/16 15:57:55  sbs
 * some DBUG reporting improved.
 *
 * Revision 3.31  2003/05/14 19:59:07  ktr
 * replaced createInlineName with TmpVarName
 *
 * Revision 3.30  2002/10/18 17:01:13  dkr
 * InlineArg() modified: support for ID_FLAGs added
 *
 * Revision 3.29  2002/10/18 14:07:49  dkr
 * InlineArg() modified: inlining of reference args corrected
 *
 * Revision 3.28  2002/09/11 23:19:03  dkr
 * CreateInlineName(): bug fixed
 *
 * Revision 3.27  2002/09/05 20:37:03  dkr
 * CreateInlineName() modified: PrefixForTmpVar() used
 *
 * Revision 3.26  2002/09/05 19:56:19  dkr
 * INLfundef(): comment about LaC limitation updated
 *
 * Revision 3.25  2002/09/05 16:54:53  dkr
 * wrapper functions always inlined now
 *
 * Revision 3.23  2002/08/31 04:58:27  dkr
 * FUNDEF_INLREC correctly initialized now :)
 *
 * Revision 3.22  2002/08/13 13:43:56  dkr
 * interface to LookUpTable.[ch] modified
 *
 * Revision 3.21  2001/05/17 12:46:31  nmw
 * MALLOC/FREE changed to Malloc/Free, result of Free() used
 *
 * Revision 3.20  2001/05/15 14:20:08  dkr
 * bug found: Inlining on LaC functions does not work correctly yet :-(
 *  -> comment and DBUG_ASSERT added in INLfundef()
 *
 * Revision 3.19  2001/05/08 13:12:29  nmw
 * set AVIS_SSAASSIGN online in ssa form
 *
 * Revision 3.18  2001/05/03 16:47:25  nmw
 * set correct AVIS_SSAASSIGN attribute for return copy assignments
 *
 * Revision 3.17  2001/04/24 17:14:30  dkr
 * call of InferDFMs() removed: after modification of DupTree() there is
 * no need to hold the DFMs consistent anymore ...
 *
 * Revision 3.16  2001/04/23 15:09:52  dkr
 * INLfundef(): calls InferDFMs() in order to get correct DFMs
 *
 * Revision 3.15  2001/04/18 10:06:45  dkr
 * signature of InlineSingleApplication modified
 *
 * Revision 3.13  2001/04/05 12:32:54  nmw
 * correct handling for global objects in Inline traversal added
 *
 * Revision 3.12  2001/04/04 09:54:20  nmw
 * AdjustAvisData added to modify fundef related attributes
 * in avis nodes when moving them out of a fundef
 *
 * Revision 3.11  2001/04/03 14:12:49  dkr
 * INL_NAIVE removed
 *
 * Revision 3.10  2001/03/29 09:46:11  dkr
 * InlineSingleApplication: DBUG_ASSERTs added
 *
 * Revision 3.9  2001/03/29 01:37:40  dkr
 * naive inlining added
 *
 * Revision 3.8  2001/03/27 13:48:08  dkr
 * signature of Inline() modified
 *
 * Revision 3.7  2001/03/22 18:05:30  dkr
 * include of tree.h eliminated
 *
 * Revision 3.6  2001/03/22 13:33:44  dkr
 * CreateInlineName is static now.
 * Renaming of identifiers via DupTree is triggered by LUT now :-)
 *
 * Revision 3.5  2001/03/21 18:09:33  dkr
 * bug fixed
 *
 * Revision 3.4  2001/03/21 17:50:17  dkr
 * Inlining recoded
 *
 * Revision 3.3  2001/02/13 17:16:19  dkr
 * MakeNode() eliminated
 *
 * Revision 3.2  2000/11/23 16:23:39  sbs
 * mem_inl_fun in Inline enclosed by ifndef DBUG_OFF to avoid compiler warning
 * in product version & node_behind initialized by NULL in INLassign to avoid
 * superfluous warning "might be used uninitialized in this function".
 *
 * Revision 3.1  2000/11/20 18:00:31  sacbase
 * new release made
 *
 * Revision 2.6  2000/07/14 11:34:47  dkr
 * FUNDEF_INLINE==0 replaced by !FUNDEF_INLINE
 *
 * Revision 2.5  2000/07/12 15:14:31  dkr
 * function DuplicateTypes renamed into DupTypes
 * function SearchDecl moved to tree_compound.c
 *
 * Revision 2.4  2000/06/23 15:19:11  dkr
 * function DupTree() with argument (arg_info != NULL) is replaced by
 * function DupTreeInfo()
 *
 * Revision 2.3  2000/01/26 17:29:49  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.2  1999/09/01 17:11:01  jhs
 * Fixed Duplicating of masks in DupAssign.
 *
 * Revision 2.1  1999/02/23 12:41:19  sacbase
 * new release made
 *
 * Revision 1.24  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.23  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.22  1998/08/07 21:46:22  dkr
 * comment added
 *
 * Revision 1.20  1998/07/16 17:20:58  sbs
 * InlineSingleApplication generated
 *
 * Revision 1.19  1998/05/08 15:46:03  srs
 * no semantic changes
 *
 * Revision 1.18  1998/04/16 16:07:34  srs
 * renamed macros which access arg_info
 *
 * Revision 1.17  1998/04/16 14:26:50  srs
 * removed NEWTREE
 *
 * Revision 1.16  1998/04/09 21:24:31  dkr
 * renamed macros:
 *   INLINE -> DUP_INLINE
 *   NORMAL -> DUP_NORMAL
 *   INVARIANT -> DUP_INVARIANT
 *
 * [...]
 *
 */

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "free.h"
#include "string.h"
#include "internal_lib.h"

#include "InferDFMs.h"
#include "optimize.h"
#include "DupTree.h"
#include "Inline.h"

/*
 * INFO structure
 */
struct INFO {
    int type;
    LUT_t lut;
    node *modul;
    node *vardecs;
    node *prolog;
    node *epilog;
    node *arg;
    ids *ids;
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_INL_TYPE(n) (n->type)
#define INFO_INL_LUT(n) (n->lut)
#define INFO_INL_MODUL(n) (n->modul)
#define INFO_INL_VARDECS(n) (n->vardecs)
#define INFO_INL_PROLOG(n) (n->prolog)
#define INFO_INL_EPILOG(n) (n->epilog)
#define INFO_INL_ARG(n) (n->arg)
#define INFO_INL_IDS(n) (n->ids)
#define INFO_INL_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_INL_TYPE (result) = 0;
    INFO_INL_LUT (result) = NULL;
    INFO_INL_MODUL (result) = NULL;
    INFO_INL_VARDECS (result) = NULL;
    INFO_INL_PROLOG (result) = NULL;
    INFO_INL_EPILOG (result) = NULL;
    INFO_INL_ARG (result) = NULL;
    INFO_INL_IDS (result) = NULL;
    INFO_INL_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/*
 * bit field (INFO_INL_TYPE)
 */
#define INL_COUNT 1

/*
 * Switch for turning inlining of wrapper functions on (1) or off (0):
 */
#define INLINE_WRAPPERS 0

/******************************************************************************
 *
 * Function:
 *   void ResetInlineNo( node *module)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
ResetInlineNo (node *module)
{
    node *fun_node;

    DBUG_ENTER ("ResetInlineNo");

    fun_node = MODUL_FUNS (module);
    while (fun_node != NULL) {
        if (FUNDEF_INLINE (fun_node)) {
            FUNDEF_INLREC (fun_node) = inlnum;
        } else {
            FUNDEF_INLREC (fun_node) = 0;
        }
        fun_node = FUNDEF_NEXT (fun_node);
    }

    DBUG_VOID_RETURN;
}

#if 0
/******************************************************************************
 *
 * Function:
 *   node *AdjustIntAssign( node *int_assign, node *ext_let)
 *
 * Description:
 *   
 *
 ******************************************************************************/

static
node *AdjustIntAssign( node *int_assign, node *ext_let)
{
  node *int_let;
  node *int_args, *ext_args;
  node *int_arg, *ext_arg;
  ids *int_ids, *ext_ids;
  node *prolog = NULL;
  node *epilog = NULL;

  DBUG_ENTER( "AdjustIntAssign");

  DBUG_ASSERT( ((int_assign != NULL) &&
                (NODE_TYPE( int_assign) == N_assign) &&
                (NODE_TYPE( ASSIGN_INSTR( int_assign)) == N_let) &&
                (NODE_TYPE( LET_EXPR( ASSIGN_INSTR( int_assign))) == N_ap)),
               "dummy fold-fun: FUNDEF_INT_ASSIGN corrupted!");

  DBUG_ASSERT( ((ext_let != NULL) && (NODE_TYPE( ext_let) == N_let)),
               "no N_let node found!");

  int_let = ASSIGN_INSTR( int_assign);

  int_ids = LET_IDS( int_let);
  ext_ids = LET_IDS( ext_let);
  while (int_ids != NULL) {
    if (strcmp( IDS_NAME( int_ids), IDS_NAME( ext_ids))) {
      epilog = MakeAssign( MakeLet( DupIds_Id( ext_ids),
                                    DupOneIds( int_ids)),
                           epilog);
    }
    int_ids = IDS_NEXT( int_ids);
    ext_ids = IDS_NEXT( ext_ids);
  }

  int_args = AP_ARGS( LET_EXPR( int_let));
  ext_args = AP_ARGS( LET_EXPR( ext_let));
  while (int_args != NULL) {
    int_arg = EXPRS_EXPR( int_args);
    ext_arg = EXPRS_EXPR( ext_args);
    DBUG_ASSERT( (NODE_TYPE( ext_arg) == N_id), "illegal argument found!");
    if ((NODE_TYPE( int_arg) != N_id) ||
        (strcmp( ID_NAME( int_arg), ID_NAME( ext_arg)))) {
      prolog = MakeAssign( MakeLet( DupNode( int_arg),
                                    DupId_Ids( ext_arg)),
                           prolog);
    }
    int_args = EXPRS_NEXT( int_args);
    ext_args = EXPRS_NEXT( ext_args);
  }

  if (prolog != NULL) {
    ASSIGN_INSTR( int_assign) = ASSIGN_INSTR( prolog);
    ASSIGN_INSTR( prolog) = NULL;
    prolog = FreeNode( prolog);
    prolog = AppendAssign( prolog, DupNode( ext_let));
  }
  else {
    ASSIGN_INSTR( int_assign) = DupNode( ext_let);
  }
  int_let = FreeTree( int_let);

  ASSIGN_NEXT( int_assign) = AppendAssign(
                                   prolog,
                                   AppendAssign(
                                         epilog,
                                         ASSIGN_NEXT( int_assign)));

  DBUG_RETURN( int_assign);
}
#endif

/******************************************************************************
 *
 * Function:
 *   node *InlineArg( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InlineArg (node *arg_node, info *arg_info)
{
    node *arg;
    node *new_ass;
    node *new_vardec;
    char *new_name;
    node *new_avis;
    node *tmp_id;
    ids *tmp_ids;

    DBUG_ENTER ("InlineArg");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_arg), "no N_arg node found!");

    DBUG_ASSERT ((INFO_INL_ARG (arg_info) != NULL), "INFO_INL_ARG not found!");
    DBUG_ASSERT ((NODE_TYPE (INFO_INL_ARG (arg_info)) == N_exprs),
                 "illegal INFO_INL_ARG found!");

    arg = EXPRS_EXPR (INFO_INL_ARG (arg_info));

    /*
     * build a new vardec based on 'arg_node' and rename it
     */
    new_vardec = MakeVardecFromArg (arg_node);
    new_name = TmpVarName (ARG_NAME (arg_node));
    VARDEC_NAME (new_vardec) = Free (VARDEC_NAME (new_vardec));
    VARDEC_NAME (new_vardec) = new_name;

    new_avis = AdjustAvisData (new_vardec, INFO_INL_FUNDEF (arg_info));
    /*
     * insert pointers ['old_avis', 'new_avis'] into INFO_INL_LUT
     */
    INFO_INL_LUT (arg_info)
      = InsertIntoLUT_P (INFO_INL_LUT (arg_info), ARG_AVIS (arg_node), new_avis);

    /*
     * insert new vardec into INFO_INL_VARDECS chain
     */
    VARDEC_NEXT (new_vardec) = INFO_INL_VARDECS (arg_info);
    INFO_INL_VARDECS (arg_info) = new_vardec;

    /*
     * insert assignment
     *   VARDEC_NAME( new_vardec) = arg;
     * into INFO_INL_PROLOG
     */
    tmp_ids = MakeIds_Copy (new_name);
    IDS_VARDEC (tmp_ids) = new_vardec;
    IDS_AVIS (tmp_ids) = VARDEC_OR_ARG_AVIS (new_vardec);

    new_ass = MakeAssign (MakeLet (DupNode (arg), tmp_ids), NULL);

    /* store definition assignment of this new vardec (only in ssaform) */
    if (valid_ssaform && (VARDEC_AVIS (new_vardec) != NULL)) {
        AVIS_SSAASSIGN (VARDEC_AVIS (new_vardec)) = new_ass;
    }
    ASSIGN_NEXT (new_ass) = INFO_INL_PROLOG (arg_info);
    INFO_INL_PROLOG (arg_info) = new_ass;

    /*
     * for reference parameters only!!!!!
     * insert assignment
     *   arg = VARDEC_NAME( new_vardec);
     * into INFO_INL_EPILOG
     */
    if (ARG_ATTRIB (arg_node) == ST_reference) {
        DBUG_ASSERT ((NODE_TYPE (arg) == N_id), "reference argument must be a N_id node");
        tmp_id = DupIds_Id (tmp_ids);
        SET_FLAG (ID, tmp_id, IS_GLOBAL, GET_FLAG (ID, arg, IS_GLOBAL));
        SET_FLAG (ID, tmp_id, IS_REFERENCE, FALSE);

        tmp_ids = MakeIds_Copy (ID_NAME (arg));
        IDS_VARDEC (tmp_ids) = ID_VARDEC (arg);
        IDS_AVIS (tmp_ids) = ID_AVIS (arg);

        new_ass = MakeAssign (MakeLet (tmp_id, tmp_ids), NULL);
        valid_ssaform = FALSE; /* no valid SSA form anymore ... */

        ASSIGN_NEXT (new_ass) = INFO_INL_EPILOG (arg_info);
        INFO_INL_EPILOG (arg_info) = new_ass;
    }

    /*
     * insert pointers ['arg_node', 'new_vardec'] into INFO_INL_LUT
     */
    INFO_INL_LUT (arg_info)
      = InsertIntoLUT_P (INFO_INL_LUT (arg_info), arg_node, new_vardec);

    /*
     * insert strings [ARG_NAME(arg_node), new_name] into INFO_INL_LUT
     */
    INFO_INL_LUT (arg_info)
      = InsertIntoLUT_S (INFO_INL_LUT (arg_info), ARG_NAME (arg_node), new_name);

    if (ARG_NEXT (arg_node) != NULL) {
        INFO_INL_ARG (arg_info) = EXPRS_NEXT (INFO_INL_ARG (arg_info));
        ARG_NEXT (arg_node) = InlineArg (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InlineVardec( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InlineVardec (node *arg_node, info *arg_info)
{
    node *new_vardec;
    char *new_name;
    node *new_avis;

    DBUG_ENTER ("InlineVardec");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_vardec), "no N_vardec node found!");

    /*
     * build a new vardec based on 'arg_node' and rename it
     */
    new_vardec = DupNode (arg_node);

    new_name = TmpVarName (VARDEC_NAME (arg_node));
    VARDEC_NAME (new_vardec) = Free (VARDEC_NAME (new_vardec));
    VARDEC_NAME (new_vardec) = new_name;

    /*
     * insert new vardec into INFO_INL_VARDECS chain
     */
    VARDEC_NEXT (new_vardec) = INFO_INL_VARDECS (arg_info);
    INFO_INL_VARDECS (arg_info) = new_vardec;

    new_avis = AdjustAvisData (new_vardec, INFO_INL_FUNDEF (arg_info));

    /*
     * insert pointers ['old_avis', 'new_avis'] into INFO_INL_LUT
     */
    INFO_INL_LUT (arg_info)
      = InsertIntoLUT_P (INFO_INL_LUT (arg_info), VARDEC_AVIS (arg_node), new_avis);

    /*
     * insert pointers ['arg_node', 'new_vardec'] into INFO_INL_LUT
     */
    INFO_INL_LUT (arg_info)
      = InsertIntoLUT_P (INFO_INL_LUT (arg_info), arg_node, new_vardec);

    /*
     * insert strings [VARDEC_NAME(arg_node), new_name] into INFO_INL_LUT
     */
    INFO_INL_LUT (arg_info)
      = InsertIntoLUT_S (INFO_INL_LUT (arg_info), VARDEC_NAME (arg_node), new_name);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = InlineVardec (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InlineRetExprs( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InlineRetExprs (node *arg_node, info *arg_info)
{
    node *new_expr, *avis;

    DBUG_ENTER ("InlineRetExprs");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_exprs), "no N_exprs node found!");
    DBUG_ASSERT ((INFO_INL_IDS (arg_info) != NULL), "INFO_INL_IDS not found!");

    new_expr
      = DupNodeLUT_Type (EXPRS_EXPR (arg_node), INFO_INL_LUT (arg_info), DUP_INLINE);

    if ((NODE_TYPE (new_expr) != N_id)
        || (strcmp (ID_NAME (new_expr), IDS_NAME (INFO_INL_IDS (arg_info))))) {
        /*
         * insert assignment
         *   INFO_INL_IDS( arg_info) = new_expr;
         * into INFO_INL_EPILOG
         */
        INFO_INL_EPILOG (arg_info)
          = MakeAssign (MakeLet (new_expr, DupOneIds (INFO_INL_IDS (arg_info))),
                        INFO_INL_EPILOG (arg_info));

        /* set the correct AVIS_SSAASSIGN() attribute for this new assignment */
        avis = IDS_AVIS (INFO_INL_IDS (arg_info));
        if (avis != NULL) {
            DBUG_PRINT ("INLAVIS", ("set correct SSAASSIGN attribute for %s",
                                    VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (avis))));
            AVIS_SSAASSIGN (avis) = INFO_INL_EPILOG (arg_info);
        }
    }

    if (EXPRS_NEXT (arg_node) != NULL) {
        INFO_INL_IDS (arg_info) = IDS_NEXT (INFO_INL_IDS (arg_info));
        EXPRS_NEXT (arg_node) = InlineRetExprs (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DoInline( node *let_node, info *arg_info)
 *
 * Description:
 *   Do function inlining. Returns a N_assign chain.
 *
 ******************************************************************************/

static node *
DoInline (node *let_node, info *arg_info)
{
    node *ap_node;
    node *inl_fundef;
    node *inl_nodes;

    DBUG_ENTER ("DoInline");

    DBUG_ASSERT ((arg_info != NULL), "DoInline called with (arg_info == NULL)!");
    DBUG_ASSERT ((NODE_TYPE (let_node) == N_let), "DoInline called with no N_let node!");

    ap_node = LET_EXPR (let_node);
    inl_fundef = AP_FUNDEF (ap_node);

    DBUG_PRINT ("INL", ("Inlining function %s", AP_NAME (ap_node)));

    INFO_INL_LUT (arg_info) = GenerateLUT ();
    INFO_INL_PROLOG (arg_info) = NULL;
    INFO_INL_EPILOG (arg_info) = NULL;
    if (INFO_INL_TYPE (arg_info) & INL_COUNT) {
        inl_fun++;
    }

    /*
     * generate new vardecs and fill INFO_INL_LUT
     */
    if (FUNDEF_VARDEC (inl_fundef) != NULL) {
        FUNDEF_VARDEC (inl_fundef) = InlineVardec (FUNDEF_VARDEC (inl_fundef), arg_info);
    }

    /*
     * generate new vardecs, fill INFO_INL_LUT and generate prolog assignments
     */
    if (FUNDEF_ARGS (inl_fundef) != NULL) {
        INFO_INL_ARG (arg_info) = AP_ARGS (ap_node);
        FUNDEF_ARGS (inl_fundef) = InlineArg (FUNDEF_ARGS (inl_fundef), arg_info);
    }

    /*
     * generate epilog assignments
     *
     * *** CAUTION ***
     * FUNDEF_RETURN must be traversed after FUNDEF_VARDEC and FUNDEF_ARGS !!
     */
    if (FUNDEF_RETURN (inl_fundef) != NULL) {
        INFO_INL_IDS (arg_info) = LET_IDS (let_node);
        if (RETURN_EXPRS (FUNDEF_RETURN (inl_fundef)) != NULL) {
            RETURN_EXPRS (FUNDEF_RETURN (inl_fundef))
              = InlineRetExprs (RETURN_EXPRS (FUNDEF_RETURN (inl_fundef)), arg_info);
        }
    }

    /*
     * duplicate function body (with LUT to get the right back-references!)
     */
    inl_nodes = DupTreeLUT_Type (BLOCK_INSTR (FUNDEF_BODY (inl_fundef)),
                                 INFO_INL_LUT (arg_info), DUP_INLINE);

    /*
     * insert INFO_INL_PROLOG, INFO_INL_EPILOG
     */
    inl_nodes = AppendAssign (INFO_INL_PROLOG (arg_info), inl_nodes);
    inl_nodes = AppendAssign (inl_nodes, INFO_INL_EPILOG (arg_info));

#if 0
  if ((FUNDEF_STATUS( inl_fundef) == ST_dofun) ||
      (FUNDEF_STATUS( inl_fundef) == ST_whilefun)) {
    node *int_assign = SearchInLUT_PP( INFO_INL_LUT( arg_info),
                                       FUNDEF_INT_ASSIGN( inl_fundef));

    DBUG_ASSERT( (int_assign != FUNDEF_INT_ASSIGN( inl_fundef)),
                 "duplicated FUNDEF_INT_ASSIGN not found in LUT!");
    int_assign = AdjustIntAssign( int_assign, let_node);
  }
#endif

    INFO_INL_LUT (arg_info) = RemoveLUT (INFO_INL_LUT (arg_info));

    DBUG_RETURN (inl_nodes);
}

/******************************************************************************
 *
 * Function:
 *   node *INLmodul( node *arg_node, info *arg_info)
 *
 * Description:
 *   Stores pointer to module in info-node and traverses function-chain.
 *
 ******************************************************************************/

node *
INLmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLmodul");

    if (MODUL_FUNS (arg_node)) {
        INFO_INL_MODUL (arg_info) = arg_node;
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *INLfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses instructons if function not inlined marked
 *
 ******************************************************************************/

node *
INLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLfundef");

    /*
     * CAUTION: inlining on LaC functions does not work correctly yet!!
     *
     * Example:
     * ========
     *
     *   before lac2fun           after lac2fun            after inlining
     *   --------------           -------------            --------------
     *
     *                            inline
     *                            int rec( x)
     *                            {
     *                              x = cond( x);
     *                              return( x);
     *                            }
     *   inline
     *   int rec( x)              int cond( int x)         int cond( int x)
     *   {                        {                        {
     *     if (x > 0) {             if (x > 0) {             if (x > 0) {
     *       x = rec( x-1);           x = rec( x-1);           x = cond( x-1);
     *     } else {                 } else {                 } else {
     *       x = 1;                   x = 1;                   x = 1;
     *     }                        }                        }
     *     return( x);              return( x);              return( x);
     *   }                        }                        }
     *
     *   int main()               int main()               int main()
     *   {                        {                        {
     *     x = rec( 10);            x = rec( 10);            x = cond( 10);
     *     return( x);              return( x);              return( x);
     *   }                        }                        }
     *
     * Now, the LaC function 'cond' is recursive and is used in more than a
     * single application :-((
     *
     * Instead, inlining must detect the recursive nature of the function
     * 'rec' and generate 'maxinl' new conditionals in 'main':
     *
     *   (maxinl == 1)            (maxinl == 2)
     *   -------------            -------------
     *
     *   inline                   inline
     *   int rec( x)              int rec( x)
     *   {                        {
     *     x = cond( x);            x = cond( x);
     *     return( x);              return( x);
     *   }                        }
     *
     *   int cond( int x)         int cond( int x)
     *   {                        {
     *     if (x > 0) {             if (x > 0) {
     *       x = rec( x-1);           x = rec( x-1);
     *     } else {                 } else {
     *       x = 1;                   x = 1;
     *     }                        }
     *     return( x);              return( x);
     *   }                        }
     *
     *                            int cond3( int x)
     *                            {
     *                              if (x > 0) {
     *                                x = rec( x-1);
     *                              } else {
     *                                x = 1;
     *                              }
     *                              return( x);
     *                            }
     *
     *   int cond2( int x)        int cond2( int x)
     *   {                        {
     *     if (x > 0) {             if (x > 0) {
     *       x = rec( x-1);           x = cond3( x-1);
     *     } else {                 } else {
     *       x = 1;                   x = 1;
     *     }                        }
     *     return( x);              return( x);
     *   }                        }
     *
     *   int main()               int main()
     *   {                        {
     *     x = cond2( 10);          x = cond2( 10);
     *     return( x);              return( x);
     *   }                        }
     */

    DBUG_ASSERT ((!FUNDEF_IS_LACFUN (arg_node)),
                 "inlining on LaC functions does not work correctly yet!");

#ifdef NEW_AST
    /*
     * We do not do inlining in used functions, as the bodies of those
     * functions will be thrown away anyways later on (they already
     * exist in a compiled version within the module), this inlining
     * would be of no use at all!
     */
    if ((FUNDEF_SYMBOLNAME (arg_node) == NULL) && (FUNDEF_BODY (arg_node) != NULL)
        && (!FUNDEF_INLINE (arg_node))) {
#else
    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_INLINE (arg_node))) {
#endif
        DBUG_PRINT ("INL", ("*** Trav function %s", FUNDEF_NAME (arg_node)));

        ResetInlineNo (INFO_INL_MODUL (arg_info));
        INFO_INL_VARDECS (arg_info) = NULL;
        INFO_INL_FUNDEF (arg_info) = arg_node;

        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);

        FUNDEF_VARDEC (arg_node)
          = AppendVardec (FUNDEF_VARDEC (arg_node), INFO_INL_VARDECS (arg_info));
    }

    if (FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *INLassign( node *arg_node, info *arg_info)
 *
 * Description:
 *   Initiates function inlining if substitution-counter not 0 or the given
 *   fundef is a wrapper function.
 *
 ******************************************************************************/

node *
INLassign (node *arg_node, info *arg_info)
{
    node *instr;
    node *inl_fundef = NULL;
    node *inlined_nodes = NULL;

    DBUG_ENTER ("INLassign");

    instr = ASSIGN_INSTR (arg_node);
    if ((NODE_TYPE (instr) == N_let) && (NODE_TYPE (LET_EXPR (instr)) == N_ap)) {
        /*
         * application -> try to inline
         */
        inl_fundef = AP_FUNDEF (LET_EXPR (instr));

        DBUG_PRINT ("INL", ("Ap of fun %s in line %d."
                            " Fundef status:"
                            " wrapper %d inline %d max-rec-inl left %d",
                            AP_NAME (LET_EXPR (instr)), NODE_LINE (arg_node),
                            (FUNDEF_STATUS (inl_fundef) == ST_wrapperfun),
                            FUNDEF_INLINE (inl_fundef), FUNDEF_INLREC (inl_fundef)));

        if (FUNDEF_INLINE (inl_fundef)) {
#if INLINE_WRAPPERS
            if ((FUNDEF_INLREC (inl_fundef) > 0)
                || (FUNDEF_STATUS (inl_fundef) == ST_wrapperfun)) {
#else
            if ((FUNDEF_INLREC (inl_fundef) > 0)
                && (FUNDEF_STATUS (inl_fundef) != ST_wrapperfun)) {
#endif
                inlined_nodes = DoInline (instr, arg_info);
            }
        }
    }

    if (inlined_nodes != NULL) {
        /*
         * inlining done
         *  -> traverse inline code
         *  -> insert inline code into assignment chain
         */

        /* 'inl_fundef' definitly is set, since (inlined_nodes != NULL) */
        if (FUNDEF_STATUS (inl_fundef) != ST_wrapperfun) {
            FUNDEF_INLREC (inl_fundef)--;
        }
        inlined_nodes = Trav (inlined_nodes, arg_info);
        if (FUNDEF_STATUS (inl_fundef) != ST_wrapperfun) {
            FUNDEF_INLREC (inl_fundef)++;
        }

        if (ASSIGN_NEXT (arg_node)) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }

        arg_node = AppendAssign (inlined_nodes, FreeNode (arg_node));
    } else {
        /*
         * no inlining done -> traverse sons
         */

        if (ASSIGN_INSTR (arg_node)) {
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        }

        if (ASSIGN_NEXT (arg_node)) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *InlineSingleApplication( node *let, node *fundef)
 *
 * description:
 *   This function allows a single function application to be inlined.
 *   It is ment for external calls only and preserves the actual fun_tab!
 *   <let_node> should point to the N_let node which RHS is to be unrolled,
 *   <fundef_node> should point to the N_fundef node of the function in which
 *      body the <let_node> is situated!! (needed for new vardecs only!)
 *   It returns an assignment-chain to be inserted for the N_assign node
 *   which has <let_node> as its body!
 *
 *   parameter 'type' is a bit field:
 *     INL_COUNT: increment 'inl_fun'
 *
 *   Example for inlining:
 *
 *     type, type, ... fun( type a_0, type a_1, ...)
 *     {
 *       type l_0;
 *       type l_1;
 *       ...
 *
 *       [...]                      <-- function body
 *
 *       return( r_0, r_1, ...);    <-- { r_? } subset of { a_?, l_? }
 *     }
 *
 *     ...
 *     R_0, R_1, ... = fun( A_0, A_1, ...);
 *     ...
 *
 *   After inlining:
 *
 *     type _inl_l_?;
 *
 *     _inl_a_? = A_?;     <-- epilog assignments
 *
 *     [...]               <-- renamed function body: a_? -> _inl_a_?
 *                                                    l_? -> _inl_l_?
 *
 *     R_? = _inl_r_?;     <-- prolog assignments
 *
 ******************************************************************************/

node *
InlineSingleApplication (node *let, node *fundef)
{
    info *arg_info;
    node *assigns;
    funtab *mem_tab;

    DBUG_ENTER ("InlineSingleApplication");

    DBUG_ASSERT ((NODE_TYPE (let) == N_let),
                 "InlineSingleApplication() needs a N_let node!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "InlineSingleApplication() needs a N_fundef node!");

    mem_tab = act_tab;
    act_tab = inline_tab;
    arg_info = MakeInfo ();
    INFO_INL_TYPE (arg_info) = 0;
    INFO_INL_VARDECS (arg_info) = FUNDEF_VARDEC (fundef);
    INFO_INL_FUNDEF (arg_info) = fundef;

    assigns = DoInline (let, arg_info);

    FUNDEF_VARDEC (fundef) = INFO_INL_VARDECS (arg_info);
    FreeInfo (arg_info);
    act_tab = mem_tab;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *Inline( node *arg_node)
 *
 * Description:
 *   Starts function inlining.
 *
 ******************************************************************************/

node *
Inline (node *arg_node)
{
    info *arg_info;
    funtab *mem_tab;
#ifndef DBUG_OFF
    int mem_inl_fun = inl_fun;
#endif

    DBUG_ENTER ("Inline");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "Inline() can be used for N_modul nodes only!");

    DBUG_PRINT ("OPT", ("FUNCTION INLINING"));

    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    mem_tab = act_tab;
    act_tab = inline_tab;
    arg_info = MakeInfo ();
    INFO_INL_TYPE (arg_info) = INL_COUNT;

    arg_node = Trav (arg_node, arg_info);

    DBUG_PRINT ("OPT", ("                        result: %d", inl_fun - mem_inl_fun));

    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    FreeInfo (arg_info);
    act_tab = mem_tab;

    DBUG_RETURN (arg_node);
}
