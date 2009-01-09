/*****************************************************************************
 *
 * $Id$
 *
 * file: lw3.c
 *
 * prefix: lw3
 *
 * Description:
 *
 *   This traversal lifts with3 bodies into functions
 *
 * usage of arg_info (INFO_LW3_...):
 *
 *    ...FUNDEFS    pointer to a chain of fundefs to be added to the syntax
 *                  tree
 *    ...RUNS       number of functions created
 *    ...MASK_IN    DFMask for in variables
 *    ...MASK_OUT   DFMask for in variables
 *    ...MASK_LOCAL DFMask for in variables
 *
 *****************************************************************************/

#include "lw3.h"
#include "dbug.h"
#include "memory.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "tree_basic.h"
#include "str.h"
#include "namespaces.h"
#include "node_basic.h"
#include "traverse.h"
#include "cleanup_decls.h"
#include "remove_dfms.h"
#include "tree_compound.h"
#include "InferDFMs.h"
/*
 * INFO structure
 */
struct INFO {
    node *fundefs;
    int funs;
    dfmask_t *mask_in;
    dfmask_t *mask_out;
    dfmask_t *mask_local;
    node *assign;
    namespace_t *ns;
};

/*
 * INFO macros
 */
#define INFO_FUNDEFS(n) (n->fundefs)
#define INFO_FUNS(n) (n->funs)
#define INFO_MASK_IN(n) (n->mask_in)
#define INFO_MASK_OUT(n) (n->mask_out)
#define INFO_MASK_LOCAL(n) (n->mask_local)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_NS(n) (n->ns)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEFS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   char *CreateThreadFunName(info *arg_info)
 *
 * description:
 *   Create a name for a new thread function
 *
 *****************************************************************************/

static char *
CreateThreadFunName (info *arg_info)
{
    static int num = 0;
    char *name;

    DBUG_ENTER ("CreateThreadFunName");

    name = (char *)MEMmalloc ((1 + 10 + 1 + (sizeof (int) / 3)) * sizeof (char));
    //                          ^ ^ ^  ^< Int in base ten over approximation
    //                          ^ ^< "threadFun_"
    //                          ^< NULL

    sprintf (name, "threadFun_%i", num++);

    DBUG_RETURN (name);
}

/******************************************************************************
 *
 * function:
 *   node *CreateThreadFunction()
 *
 * description:
 *   Create a thread function
 *
 *****************************************************************************/

static node *
CreateThreadFunction (node *block, info *arg_info)
{
    lut_t *lut;
    node *args, *ret, *fundef, *ap;
    char *funName;
    dfmask_t *tmp_mask;
    node *vardecs;

    DBUG_ENTER ("CreateThreadFunction");

    lut = LUTgenerateLut ();
    args = DFMUdfm2Args (INFO_MASK_IN (arg_info), lut);
    tmp_mask = DFMgenMaskMinus (INFO_MASK_OUT (arg_info), INFO_MASK_IN (arg_info));
    vardecs = DFMUdfm2Vardecs (tmp_mask, lut);
    tmp_mask = DFMremoveMask (tmp_mask);

    ret
      = TBmakeAssign (TBmakeReturn (DFMUdfm2ReturnExprs (INFO_MASK_OUT (arg_info), lut)),
                      NULL);

    if (vardecs != NULL) {
        BLOCK_VARDEC (block) = TCappendVardec (vardecs, BLOCK_VARDEC (vardecs));
    }

    funName = CreateThreadFunName (arg_info);
    fundef
      = TBmakeFundef (STRcpy (funName), NSdupNamespace (INFO_NS (arg_info)),
                      DFMUdfm2Rets (INFO_MASK_OUT (arg_info)), args, block, /* block */
                      INFO_FUNDEFS (arg_info)); /*point to existing fundefs*/
    FUNDEF_RETURN (fundef) = ASSIGN_INSTR (ret);
    FUNDEF_ISWITH3FUN (fundef) = TRUE;
    lut = LUTremoveLut (lut);

    INFO_FUNDEFS (arg_info) = fundef; /* save fundef */

    ap = TBmakeAp (fundef, DFMUdfm2ApArgs (INFO_MASK_IN (arg_info), lut));

    DBUG_RETURN (ap);
}
/******************************************************************************
 *
 * function:
 *   node *LW3module( node *syntax_tree)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
LW3module (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LW3module");

    INFO_NS (arg_info) = MODULE_NAMESPACE (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_ASSERT ((MODULE_FUNTHREADS (arg_node) == NULL),
                 "Thread functions are already in this module");

    MODULE_FUNTHREADS (arg_node) = INFO_FUNDEFS (arg_info);

    INFO_FUNDEFS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LW3with3( node *arg_node, info *arg_info)
 *
 * description:
 *   Store mask information about variables
 *
 *****************************************************************************/
node *
LW3with3 (node *arg_node, info *arg_info)
{
    info *new_info;

    DBUG_ENTER ("LW3with3");

    /* Create new info to correctly handle nested with3 loops*/
    new_info = MakeInfo ();
    INFO_NS (new_info) = INFO_NS (arg_info);

    DBUG_ASSERT ((WITH3_IN_MASK (arg_node) != NULL),
                 "No input DFM for current with3 loop");
    DBUG_ASSERT ((WITH3_OUT_MASK (arg_node) != NULL),
                 "No output DFM for current with3 loop");
    DBUG_ASSERT ((WITH3_LOCAL_MASK (arg_node) != NULL),
                 "No local DFM for current with3 loop");

    INFO_MASK_IN (new_info) = WITH3_IN_MASK (arg_node);
    INFO_MASK_OUT (new_info) = WITH3_OUT_MASK (arg_node);
    INFO_MASK_LOCAL (new_info) = WITH3_LOCAL_MASK (arg_node);

    arg_node = TRAVcont (arg_node, new_info);

    INFO_FUNDEFS (arg_info)
      = TCappendFundef (INFO_FUNDEFS (new_info), INFO_FUNDEFS (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LW3range( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
LW3range (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LW3range");

    TRAVdo (RANGE_BODY (arg_node), arg_info);

    RANGE_RESULTS (arg_node) = CreateThreadFunction (RANGE_BODY (arg_node), arg_info);
    RANGE_BODY (arg_node) = TBmakeBlock (TBmakeEmpty (), NULL);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *LW3doLW3( node *syntax_tree)
 *
 * description:
 *   Lifts out the bodies of with loop3s into functions
 *
 *****************************************************************************/

node *
LW3doLiftWith3 (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("LW3");

    /*
     * Locate variable usage
     */

    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_WITH3);

    info = MakeInfo ();

    TRAVpush (TR_lw3);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    /*  syntax_tree = CUDdoCleanupDecls( syntax_tree); */
    syntax_tree = RDFMSdoRemoveDfms (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
