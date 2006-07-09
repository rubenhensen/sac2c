/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup isv Insert Shape Variables
 *
 * This traversal augments all AVIS nodes with expressions for dim and shape.
 * In contrast to the new_types, these expressions do not need to be constant.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file insert_shapevars.c
 *
 * Prefix: ISV
 *
 *****************************************************************************/
#include "insert_shapevars.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    enum { TS_module, TS_fundef } travscope;
    node *vardecs;
};

#define INFO_TRAVSCOPE(n) ((n)->travscope)
#define INFO_VARDECS(n) ((n)->vardecs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TRAVSCOPE (result) = TS_module;
    INFO_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *ISVdoInsertShapeVariables( node *syntax_tree)
 *
 *****************************************************************************/
node *
ISVdoInsertShapeVariables (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ISVdoInsertShapeVariables");

    info = MakeInfo ();

    INFO_TRAVSCOPE (info) = TS_module;
    TRAVpush (TR_isv);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVdoInsertShapeVariablesOneFundef( node *fundef)
 *
 *****************************************************************************/
node *
ISVdoInsertShapeVariablesOneFundef (node *fundef)
{
    info *info;

    DBUG_ENTER ("ISVdoInsertShapeVariablesOneFundef");

    info = MakeInfo ();

    INFO_TRAVSCOPE (info) = TS_fundef;
    TRAVpush (TR_isv);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ISVfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISVfundef");

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_VARDEC (arg_node) != NULL)) {
        FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), arg_info);
    }

    if ((INFO_TRAVSCOPE (arg_info) == TS_module) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVvardec(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ISVvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISVvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    VARDEC_AVIS (arg_node) = TRAVdo (VARDEC_AVIS (arg_node), arg_info);

    if (INFO_VARDECS (arg_info) != NULL) {
        VARDEC_NEXT (arg_node)
          = TCappendVardec (INFO_VARDECS (arg_info), VARDEC_NEXT (arg_node));

        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISVavis (node *arg_node, info *arg_info)
{
    node *assign = NULL;

    DBUG_ENTER ("ISVavis");

    if (AVIS_DIM (arg_node) == NULL) {
        if (TUdimKnown (AVIS_TYPE (arg_node))) {
            AVIS_DIM (arg_node) = TBmakeNum (TYgetDim (AVIS_TYPE (arg_node)));
        } else {
            if ((AVIS_SSAASSIGN (arg_node) != NULL)
                && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (arg_node))) != N_funcond)) {
                node *dimavis;

                dimavis
                  = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (arg_node)),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

                AVIS_DIM (dimavis) = TBmakeNum (0);
                AVIS_SHAPE (dimavis) = TCmakeIntVector (NULL);

                assign
                  = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL),
                                             TCmakePrf1 (F_dim, TBmakeId (arg_node))),
                                  NULL);
                AVIS_SSAASSIGN (dimavis) = assign;

                INFO_VARDECS (arg_info) = TBmakeVardec (dimavis, NULL);

                AVIS_DIM (arg_node) = TBmakeId (dimavis);
            }
        }
    }

    if (AVIS_SHAPE (arg_node) == NULL) {
        if (TUshapeKnown (AVIS_TYPE (arg_node))) {
            AVIS_SHAPE (arg_node) = SHshape2Array (TYgetShape (AVIS_TYPE (arg_node)));

        } else {
            if ((AVIS_SSAASSIGN (arg_node) != NULL)
                && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (arg_node))) != N_funcond)) {
                node *shpavis;
                node *newass;

                if (TUdimKnown (AVIS_TYPE (arg_node))) {
                    int dim = TYgetDim (AVIS_TYPE (arg_node));
                    shpavis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (arg_node)),
                                          TYmakeAKS (TYmakeSimpleType (T_int),
                                                     SHcreateShape (1, dim)));
                } else {
                    shpavis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (arg_node)),
                                          TYmakeAKD (TYmakeSimpleType (T_int), 1,
                                                     SHmakeShape (0)));
                }

                AVIS_DIM (shpavis) = TBmakeNum (1);
                AVIS_SHAPE (shpavis) = TCmakeIntVector (
                  TBmakeExprs (DUPdoDupNode (AVIS_DIM (arg_node)), NULL));

                newass
                  = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL),
                                             TCmakePrf1 (F_shape, TBmakeId (arg_node))),
                                  NULL);

                assign = TCappendAssign (assign, newass);

                AVIS_SSAASSIGN (shpavis) = ASSIGN_NEXT (assign);

                INFO_VARDECS (arg_info) = TBmakeVardec (shpavis, INFO_VARDECS (arg_info));

                AVIS_SHAPE (arg_node) = TBmakeId (shpavis);
            }
        }
    }

    if (assign != NULL) {
        ASSIGN_NEXT (AVIS_SSAASSIGN (arg_node))
          = TCappendAssign (assign, ASSIGN_NEXT (AVIS_SSAASSIGN (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Shape Variables -->
 *****************************************************************************/
