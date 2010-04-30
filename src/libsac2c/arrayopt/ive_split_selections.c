/*
 *
 * $Id$
 *
 */

#include "ive_split_selections.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "dbug.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "ctinfo.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/*
 * set WARN_MISSING_SAA to generate a warning when
 * splitting would not be possible due to missing SAA
 * shape information and explicit shape computations
 * are inserted instead.
 */
#define WARN_MISSING_SAA

/**
 *
 * @defgroup ive IVESPLIT
 * @ingroup opt
 *
 * @brief "index vector elimination - split selections" (IVESPLIT)
 *	splits all array accesses into an explicit offset computation
 *	and a direct array access to that offset. By doing so, offset
 *	computations are made available to classical SAC high-level
 *	optimisations as CSE and LIR.
 *
 *  The following transformations are performed:
 *
 *     x = sel(iv,B)            ->      iv_B = vect2offset($SHPEXPR$, iv);
 *	                                    x = _idx_sel(iv_B,B);
 *
 *
 *     x = modarray(B, iv, v);  ->      iv_B = vect2offset($SHPEXPR$, iv);
 *                                      X = idx_modarray(iv_B, B, v);
 *
 *     NB. Note different argument order in modarray vs idx_modarray!
 *
 *  $SHPEXPR$ thereby refers to the AVIS_SHAPE expression annotated by
 *  the SAA inference.
 *
 * @{
 */

/**
 * @{
 */

/**
 *
 * @file ive_split_selections.c
 *
 *  This file contains the implementation of the SPLIT transformation
 *  as described in the IVE paper.
 */

/**
 * INFO structure
 */
struct INFO {
    node *preassigns;
    node *vardecs;
};

/**
 * INFO macros
 */
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_VARDECS(n) ((n)->vardecs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PREASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * helper functions
 */

/** <!-- ****************************************************************** -->
 * @brief Inserts a vect2offset computation into the AST by modifying the
 *        corresponding fields of the info structure.
 *
 * @param iv      index vector to be used for the vect2offset
 * @param shpexpr shape expression to be used for the vect2offset (CONSUMED!)
 * @param info    info structure
 *
 * @return the N_avis of the computed offset
 ******************************************************************************/
node *
AddVect2Offset (node *iv, node *shpexpr, info *info)
{
    node *avis, *assign;

    DBUG_ENTER ("AddVect2Offset");

    DBUG_ASSERT ((shpexpr != NULL), "no shape information found!");

    avis
      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    INFO_VARDECS (info) = TBmakeVardec (avis, INFO_VARDECS (info));

    assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                 TCmakePrf2 (F_vect2offset, shpexpr, DUPdoDupTree (iv))),
                      NULL);

    AVIS_SSAASSIGN (avis) = assign;

    INFO_PREASSIGNS (info) = TCappendAssign (INFO_PREASSIGNS (info), assign);

    DBUG_RETURN (avis);
}

/** <!-- ****************************************************************** -->
 * @brief Inserts code into the AST to compute the shape of the id given as
 *        argument array.
 *
 * @param array    N_id node representing an array
 * @param arg_info info structure
 *
 * @return N_avis node representing the built shape expression
 ******************************************************************************/
node *
AddShapeComputation (node *array, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("AddShapeComputation");

    DBUG_ASSERT ((NODE_TYPE (array) == N_id), "non-flattened array found!");

    if (TUdimKnown (AVIS_TYPE (ID_AVIS (array)))) {
        /* AKD or AKD */
        int dim;
        int pos;
        node *sexprs = NULL;
        node *eassigns = NULL;
        node *assign = NULL;

        dim = TYgetDim (AVIS_TYPE (ID_AVIS (array)));
        avis = TBmakeAvis (TRAVtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim)));
        INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

        for (pos = dim - 1; pos >= 0; pos--) {
            node *eavis;

            eavis = TBmakeAvis (TRAVtmpVar (),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

            INFO_VARDECS (arg_info) = TBmakeVardec (eavis, INFO_VARDECS (arg_info));
            eassigns
              = TBmakeAssign (TBmakeLet (TBmakeIds (eavis, NULL),
                                         TCmakePrf2 (F_idx_shape_sel, TBmakeNum (pos),
                                                     DUPdoDupNode (array))),
                              eassigns);
            AVIS_SSAASSIGN (eavis) = eassigns;

            sexprs = TBmakeExprs (TBmakeId (eavis), sexprs);
        }

        assign
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), TCmakeIntVector (sexprs)),
                          NULL);
        AVIS_SSAASSIGN (avis) = assign;

        INFO_PREASSIGNS (arg_info)
          = TCappendAssign (INFO_PREASSIGNS (arg_info), eassigns);
        INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), assign);
    } else {
        /* AUD */
        node *assign;

        avis = TBmakeAvis (TRAVtmpVar (),
                           TYmakeAKD (TYmakeSimpleType (T_int), 1, SHcreateShape (0)));
        INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

        assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                          TCmakePrf1 (F_shape_A, DUPdoDupNode (array))),
                               NULL);
        AVIS_SSAASSIGN (avis) = assign;

        INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), assign);
    }

    DBUG_RETURN (avis);
}

/**
 * traversal functions
 */

/** <!-- ****************************************************************** -->
 * @brief Stores the vardec chain of the current function in INFO_VARDECS
 *        prior to traversing the body.
 *
 * @param arg_node N_fundef node
 * @param arg_info info structure
 *
 * @return N_fundef node
 ******************************************************************************/
node *
IVESPLITfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVESPLITfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDECS (arg_info) = BLOCK_VARDEC (FUNDEF_BODY (arg_node));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        BLOCK_VARDEC (FUNDEF_BODY (arg_node)) = INFO_VARDECS (arg_info);
        INFO_VARDECS (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Directs the traversal into the assignment instruction and inserts
 *        the INFO_PREASSIGNS chain before the current node prior to
 *        traversing further down.
 *
 * @param arg_node N_assign node
 * @param arg_info info structure
 *
 * @return unchanged N_assign node
 ******************************************************************************/
node *
IVESPLITassign (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("IVESPLITassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    new_node = arg_node;

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        new_node = TCappendAssign (INFO_PREASSIGNS (arg_info), new_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @brief Splits sel and modarray operations into the offset computation
 *        and array access parts.
 *
 * @param arg_node N_prf node
 * @param arg_info info structure
 *
 * @return unchanged N_prf node
 ******************************************************************************/
node *
IVESPLITprf (node *arg_node, info *arg_info)
{
    node *new_node;
    node *shpprf2 = NULL;
    node *avis;
    node *array = NULL;

    DBUG_ENTER ("IVESPLITprf");

    switch (PRF_PRF (arg_node)) {
    case F_sel_VxA:
    case F_modarray_AxVxS:
    case F_modarray_AxVxA:
        array
          = (PRF_PRF (arg_node) == F_sel_VxA) ? PRF_ARG2 (arg_node) : PRF_ARG1 (arg_node);

        if ((NULL != array) && (N_id == NODE_TYPE (array))) {
            if (AVIS_SHAPE (ID_AVIS (array)) != NULL) {
                shpprf2 = DUPdoDupTree (AVIS_SHAPE (ID_AVIS (array)));
            } else if ((TYisAKS (AVIS_TYPE (ID_AVIS (array))))
                       || (TYisAKV (AVIS_TYPE (ID_AVIS (array))))) {
                shpprf2 = SHshape2Array (TYgetShape (AVIS_TYPE (ID_AVIS (array))));
            }
        }
        if (NULL == shpprf2) {
#ifdef WARN_MISSING_SAA
            CTIwarn ("Insufficient symbolic shape information available. "
                     "Using explicit information to split index operation.");
#endif
            shpprf2 = TBmakeId (AddShapeComputation (array, arg_info));
        }
        break;

    default:
        shpprf2 = NULL;
        break;
    }

    if (NULL != shpprf2) {
        switch (PRF_PRF (arg_node)) {
        case F_sel_VxA:
            avis = AddVect2Offset (PRF_ARG1 (arg_node), shpprf2, arg_info);
            new_node = TCmakePrf2 (F_idx_sel, TBmakeId (avis), PRF_ARG2 (arg_node));
            PRF_ARG2 (arg_node) = NULL;

            arg_node = FREEdoFreeTree (arg_node);
            arg_node = new_node;
            break;

        case F_modarray_AxVxS:
        case F_modarray_AxVxA:
            avis = AddVect2Offset (PRF_ARG2 (arg_node), shpprf2, arg_info);

            new_node
              = TCmakePrf3 ((PRF_PRF (arg_node) == F_modarray_AxVxS)
                              ? F_idx_modarray_AxSxS
                              : F_idx_modarray_AxSxA,
                            PRF_ARG1 (arg_node), TBmakeId (avis), PRF_ARG3 (arg_node));

            PRF_ARG1 (arg_node) = NULL;
            PRF_ARG3 (arg_node) = NULL;
            arg_node = FREEdoFreeTree (arg_node);
            arg_node = new_node;
            break;

        default:
            /* nothing */
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief function triggering the index vector elimination on the
 *        syntax tree. the traversal relies on the information
 *        infered by index vector elimination inference.
 *
 * @param syntax_tree N_module node
 *
 * @return transformed syntax tree
 ******************************************************************************/
node *
IVESPLITdoSplitSelections (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IVEdoSplitSelections");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "IVESPLIT is intended to run on the entire tree");

    info = MakeInfo ();

    TRAVpush (TR_ivesplit);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/*@}*/ /* defgroup ive */
