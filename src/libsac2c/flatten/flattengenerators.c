/*
 * $Id: flatten.c 15674 2008-02-28 12:08:58Z sah $
 */

#include <stdio.h>

#include "globals.h"
#include "dbug.h"

#include "new_types.h"
#include "types.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "handle_mops.h"
#include "while2do.h"
#include "handle_condexpr.h"
#include "namespaces.h"
#include "shape.h"

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
 * This phase flattens only WL generators.
 * It has to run in SAA mode.
 * Likely,  fixing flatten.c to do the job would
 * be a better approach, but I don't have time to
 * figure out how to make it handle types properly.
 *
 */

/**
 * INFO structure
 */
struct INFO {
    node *vardecs;
    node *preassigns;
    bool assignisnwith;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_ASSIGNISNWITH(n) (n->assignisnwith)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_ASSIGNISNWITH (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn static node *flattenBound( node *arg_node, info *arg_info)
 *
 *   @brief  Flattens the WL bound at arg_node.
 *           I.e., if the generator looks like this on entry:
 *            s0 = _idx_shape_sel(0,x);
 *            s1 = _idx_shape_sel(1,x);
 *            z = with {
 *             (. <= iv < [s0, s1]) ...
 *            }
 *
 *          it will look like this on the way out:
 *            int[2] TMP;
 *            ...
 *            s0 = _idx_shape_sel(0,x);
 *            s1 = _idx_shape_sel(1,x);
 *            TMP = [s0, s1];
 *            z = with {
 *             (. <= iv < TMP) ...
 *            }
 *
 *          The only rationale for this change is to ensure that
 *          WL bounds are named. This allows us to associate an
 *          N_avis node with each bound, which will be used to
 *          store AVIS_MINVAL and AVIS_MAXVAL for the bound.
 *          These fields, in turn, will be used by the constant
 *          folder to remove guards and do other swell optimizations.
 *
 *          Things are made a bit messier by the requirement
 *          of placing the new TMP assigns before the N_assign
 *          of the N_with, in a nested environment, such as this one
 *          (the second WLs are the S+V statements:
 *
 *          Compile this with these options to see what has
 *          to happen:
 *
 *          sac2c nestedwl.sac -doswlf -nowlur -doswlf -v1
 *                -b11:saacyc:flt -noprelude
 *
 *  use Array: {sel,shape,iota,+,*};
 *  use StdIO: {print};
 *
 *  int[*] id(int[*] y)
 *  { return(y);
 *  }
 *
 *  int main()
 *  {
 *   A = iota (id (25));
 *   B = 20 + iota( id(25));
 *   C = with {
 *        (. <= iv <= .)
 *           {
 *            e1 = A[iv] + [1,2,3,4,5];
 *            e2 = B[iv] + [1,2,3,4,50];
 *           } : e1 + e2;
 *       } : genarray([25]);
 *
 *   print(C);
 *  return(0);
 *  }
 *
 *
 *   @param  node *arg_node: a WL PART BOUND to be flattened.
 *           info *arg_info:
 *
 *   @return node *node:      N_id node for flattened bound
 ******************************************************************************/
static node *
flattenBound (node *arg_node, info *arg_info)
{
    node *avis;
    node *nas;
    node *res;
    shape *shp;
    int dim = 1;
    int xrho;

    DBUG_ENTER ("flattenBound");

    res = arg_node;
    if (NULL != arg_node) {
        switch (NODE_TYPE (arg_node)) {
        case N_array:
            xrho = TCcountExprs (ARRAY_AELEMS (arg_node));
            shp = SHcreateShape (dim, xrho);
            avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int), shp));
            INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));
            nas
              = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), DUPdoDupTree (arg_node)),
                              NULL);
            INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), nas);
            AVIS_SSAASSIGN (avis) = nas;
            AVIS_DIM (avis) = TBmakeNum (dim);
            AVIS_SHAPE (avis)
              = TCmakeIntVector (TBmakeExprs (DUPdoDupNode (AVIS_DIM (avis)), NULL));
            res = TBmakeId (avis);
            FREEdoFreeTree (arg_node);
            DBUG_PRINT ("FLATGflattenBound",
                        ("Generated avis for: %s", AVIS_NAME (avis)));
            break;
        case N_id:
            break;
        default:
            DBUG_ASSERT (FALSE, "FLATG flattenBound expected N_array or N_id");
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGdoFlatten(node *arg_node)
 *
 * description:
 *   eliminates nested function applications:
 *   flattens WL generator bounds, step, width.
 *
 *
 ******************************************************************************/

node *
FLATGdoFlatten (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("FLATGdoFlatten");

    info_node = MakeInfo ();

    TRAVpush (TR_flatg);
    arg_node = TRAVdo (arg_node, info_node);
    TRAVpop ();

    info_node = FreeInfo (info_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGmodule(node *arg_node, info *arg_info)
 *
 * description:
 *   this function is needed to limit the traversal to the FUNS-son of
 *   N_module!
 *
 ******************************************************************************/

node *
FLATGmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FLATGmodule");

    if (MODULE_FUNS (arg_node)) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FLATGfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   - calls TRAVdo to flatten the user defined functions if function body is not
 *   empty and resets tos after flatten of the function's body.
 *   - the formal parameters of a function will be traversed to put their names
 *   on the stack!
 *
 ******************************************************************************/

node *
FLATGfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("FLATGfundef");

    /*
     * Do not flatten imported functions. These functions have already been
     * flattened and if this is done again there may arise name clashes.
     * A new temp variable __flat42 may conflict with __flat42 which was
     * inserted in the first flatten phase (module compiliation).
     * Furthermore, imported code contains IDS nodes instead of SPIDS nodes!
     * This may lead to problems when this traversal is run.
     */
    if ((FUNDEF_BODY (arg_node) != NULL) && !FUNDEF_WASIMPORTED (arg_node)
        && FUNDEF_ISLOCAL (arg_node)) {
        INFO_VARDECS (arg_info) = NULL;
        DBUG_PRINT ("FLATG", ("flattening function %s:", FUNDEF_NAME (arg_node)));
        if (FUNDEF_ARGS (arg_node)) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /* Append new vardecs, if any were generated, to existing vardec chain. */
    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDEC (FUNDEF_BODY (arg_node))
          = TCappendVardec (BLOCK_VARDEC (FUNDEF_BODY (arg_node)),
                            INFO_VARDECS (arg_info));
        INFO_VARDECS (arg_info) = NULL;
    }

    /*
     * Proceed with the next function...
     */
    if (FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FLATGpart(node *arg_node, info *arg_info)
 *
 * @brief traverse the partition
 *        This is needed to handle nested WLs, such as:
 *
 *        use Array: {genarray,<=,+,iota,sel};
 *        use StdIO: {print};
 *
 *        int[*] id(int[*] y)
 *        { return(y);
 *        }
 *
 *        int main()
 *        {
 *         x = id(genarray([10,10],4));
 *
 *          z = with {
 *             (. <= iv <= .) : x[iv] + iota(3);
 *                } : genarray(_shape_A_(x), [1,2, 3]);
 *
 *                print(z);
 *                return(0);
 *                }
 *
 *
 *        We have to collect the new flattened variable assignments
 *        for all partitions, then insert them before the N_with.
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
FLATGpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FLATGpart");

    if (PART_CODE (arg_node) != NULL) {
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    }

    if (NULL != PART_NEXT (arg_node)) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    /* We have to traverse the generators last */
    if (PART_GENERATOR (arg_node) != NULL) {
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node FLATGwith( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        We need a fresh PREASSIGN chain, because we have to collect
 *        the preassigns for all partitions, then pass them back
 *        to our N_assign node.
 *
 *****************************************************************************/
node *
FLATGwith (node *arg_node, info *arg_info)
{
    info *new_info;

    DBUG_ENTER ("FLATGwith");

    new_info = MakeInfo ();
    INFO_VARDECS (new_info) = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    /* Traverse all partitions in this N_with. */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), new_info);
    INFO_VARDECS (arg_info) = INFO_VARDECS (new_info);
    INFO_PREASSIGNS (arg_info)
      = TCappendAssign (INFO_PREASSIGNS (new_info), INFO_PREASSIGNS (arg_info));
    new_info = FreeInfo (new_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node FLATGassign( node *arg_node, info *arg_info)
 *
 * @brief performs a depth-first traversal.
 *        Prepends new assign nodes ahead of this node.
 *        The check on node type is to ensure that we
 *        are in the right place for the prepends.
 *        The nested WLs are the problem here...
 *
 *****************************************************************************/
node *
FLATGassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("FLATGassign");
    if ((N_let == NODE_TYPE (ASSIGN_INSTR (arg_node)))
        && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))))) {
        INFO_ASSIGNISNWITH (arg_info) = TRUE;
        DBUG_PRINT ("FLATG", ("Traversing N_assign for %s",
                              AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_INSTR (arg_node))))));
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if ((INFO_ASSIGNISNWITH (arg_info)) && (NULL != INFO_PREASSIGNS (arg_info))) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
        INFO_ASSIGNISNWITH (arg_info) = FALSE;
    }

    /* Traverse remaining assigns in this block. */
    if (NULL != ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FLATGgenerator(node *arg_node, info *arg_info)
 *
 * description:
 *   flattens N_generator
 *   all non-N_ID-nodes are removed and the operators are changed
 *   to <= and < if possible (bounds != NULL).
 *
 ******************************************************************************/

node *
FLATGgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FLATGgenerator");

    GENERATOR_BOUND1 (arg_node) = flattenBound (GENERATOR_BOUND1 (arg_node), arg_info);
    GENERATOR_BOUND2 (arg_node) = flattenBound (GENERATOR_BOUND2 (arg_node), arg_info);
    GENERATOR_STEP (arg_node) = flattenBound (GENERATOR_STEP (arg_node), arg_info);
    GENERATOR_WIDTH (arg_node) = flattenBound (GENERATOR_WIDTH (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
