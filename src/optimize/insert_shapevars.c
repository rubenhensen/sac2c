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
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "constants.h"
#include "makedimexpr.h"
#include "makeshapeexpr.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    enum { TS_module, TS_fundef } travscope;
    node *vardecs;
    node *preassign;
    node *fundef;
    node *lhs;
    node *rhs;
};

#define INFO_TRAVSCOPE(n) ((n)->travscope)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_RHS(n) ((n)->rhs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TRAVSCOPE (result) = TS_module;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGN (result) = NULL;

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
static node *
GenIntVector (node *element)
{
    node *res = NULL;

    DBUG_ENTER ("GenIntVector");

    res = TCmakeIntVector (TBmakeExprs (element, NULL));

    DBUG_RETURN (res);
}

static node *
MakeScalarAvis (node *orig_avis, char *name)
{
    node *res;

    DBUG_ENTER ("MakeScalarAvis");

    res = TBmakeAvis (name, TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    AVIS_DIM (res) = TBmakeNum (0);
    AVIS_SHAPE (res) = TCmakeIntVector (NULL);
    AVIS_SHAPEVAROF (res) = orig_avis;

    DBUG_RETURN (res);
}

static bool
DimIsUndefinedId (node *avis)
{
    bool res;

    DBUG_ENTER ("DimIsUndefinedId");

    res = ((NODE_TYPE (AVIS_DIM (avis)) == N_id)
           && (AVIS_SHAPEVAROF (ID_AVIS (AVIS_DIM (avis))) == avis));

    DBUG_RETURN (res);
}

static bool
ShapeIsUndefinedId (node *avis)
{
    bool res;

    DBUG_ENTER ("ShapeIsUndefinedId");

    res = ((NODE_TYPE (AVIS_SHAPE (avis)) == N_id)
           && (AVIS_SHAPEVAROF (ID_AVIS (AVIS_SHAPE (avis))) == avis));

    DBUG_RETURN (res);
}

static bool
ShapeIsUndefinedIdArray (node *avis)
{
    bool res = FALSE;

    DBUG_ENTER ("ShapeIsUndefinedIdArray");

    if ((NODE_TYPE (AVIS_SHAPE (avis)) == N_array)
        && (ARRAY_AELEMS (AVIS_SHAPE (avis)) != NULL)) {
        node *elem1 = EXPRS_EXPR (ARRAY_AELEMS (AVIS_SHAPE (avis)));
        res
          = ((NODE_TYPE (elem1) == N_id) && (AVIS_SHAPEVAROF (ID_AVIS (elem1)) == avis));
    }

    DBUG_RETURN (res);
}

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

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), arg_info);

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
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
    DBUG_ENTER ("ISVavis");

    if (AVIS_DIM (arg_node) == NULL) {
        if (TUdimKnown (AVIS_TYPE (arg_node))) {
            AVIS_DIM (arg_node) = TBmakeNum (TYgetDim (AVIS_TYPE (arg_node)));
        } else {
            node *dimavis
              = MakeScalarAvis (arg_node,
                                ILIBstringConcat (AVIS_NAME (arg_node), "__dim"));

            INFO_VARDECS (arg_info) = TBmakeVardec (dimavis, INFO_VARDECS (arg_info));

            AVIS_DIM (arg_node) = TBmakeId (dimavis);
        }
    }

    if (AVIS_SHAPE (arg_node) == NULL) {
        if (TUshapeKnown (AVIS_TYPE (arg_node))) {
            /*
             * <= AKS
             */
            AVIS_SHAPE (arg_node) = SHshape2Array (TYgetShape (AVIS_TYPE (arg_node)));
        } else if (TUdimKnown (AVIS_TYPE (arg_node))) {
            /*
             * AKD
             */
            int i;
            node *aelems = NULL;
            str_buf *name;

            name = ILIBstrBufCreate (1024 * sizeof (char));

            for (i = TYgetDim (AVIS_TYPE (arg_node)) - 1; i >= 0; i--) {
                name = ILIBstrBufPrintf (name, "%s__shp%d", AVIS_NAME (arg_node), i);

                node *selavis = MakeScalarAvis (arg_node, ILIBstrBuf2String (name));

                ILIBstrBufFlush (name);

                INFO_VARDECS (arg_info) = TBmakeVardec (selavis, INFO_VARDECS (arg_info));

                aelems = TBmakeExprs (TBmakeId (selavis), aelems);
            }

            AVIS_SHAPE (arg_node) = TCmakeIntVector (aelems);

            name = ILIBstrBufFree (name);
        } else {
            /*
             * AUD
             */
            node *shpavis;

            shpavis
              = TBmakeAvis (ILIBstringConcat (AVIS_NAME (arg_node), "__shp"),
                            TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));

            AVIS_DIM (shpavis) = TBmakeNum (1);
            AVIS_SHAPE (shpavis) = GenIntVector (DUPdoDupNode (AVIS_DIM (arg_node)));
            AVIS_SHAPEVAROF (shpavis) = arg_node;

            INFO_VARDECS (arg_info) = TBmakeVardec (shpavis, INFO_VARDECS (arg_info));

            AVIS_SHAPE (arg_node) = TBmakeId (shpavis);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISVblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISVblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISVassign (node *arg_node, info *arg_info)
{
    node *preassign;

    DBUG_ENTER ("ISVassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    preassign = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (preassign != NULL) {
        arg_node = TCappendAssign (preassign, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISVlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        INFO_RHS (arg_info) = LET_EXPR (arg_node);

        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISVids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("ISVids");

    avis = IDS_AVIS (arg_node);

    if (DimIsUndefinedId (avis)) {
        INFO_PREASSIGN (arg_info)
          = TCappendAssign (INFO_PREASSIGN (arg_info),
                            MDEdoMakeDimExpression (INFO_RHS (arg_info), avis,
                                                    INFO_LHS (arg_info),
                                                    INFO_FUNDEF (arg_info)));
    }

    if ((!DimIsUndefinedId (avis)) && (ShapeIsUndefinedId (avis))) {
        INFO_PREASSIGN (arg_info)
          = TCappendAssign (INFO_PREASSIGN (arg_info),
                            MSEdoMakeShapeExpression (INFO_RHS (arg_info), avis,
                                                      INFO_LHS (arg_info),
                                                      INFO_FUNDEF (arg_info)));
    }

    if ((!DimIsUndefinedId (avis)) && (ShapeIsUndefinedIdArray (avis))) {
        int dim;
        node *shpavis;
        node *oldshape;
        node *newass = NULL;

        DBUG_ASSERT (NODE_TYPE (AVIS_DIM (avis)) == N_num,
                     "AKD array without constant AVIS_DIM found!");

        dim = SHgetExtent (ARRAY_SHAPE (AVIS_SHAPE (avis)), 0);

        shpavis
          = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (avis)),
                        TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim)));

        AVIS_DIM (shpavis) = TBmakeNum (1);
        AVIS_SHAPE (shpavis) = GenIntVector (TBmakeNum (dim));
        AVIS_SHAPEVAROF (shpavis) = avis;

        /*
         * Temporarily exchange Shapevector with Id
         */
        oldshape = AVIS_SHAPE (avis);
        AVIS_SHAPE (avis) = TBmakeId (shpavis);

        newass = MSEdoMakeShapeExpression (INFO_RHS (arg_info), avis, INFO_LHS (arg_info),
                                           INFO_FUNDEF (arg_info));

        AVIS_SHAPE (avis) = FREEdoFreeNode (AVIS_SHAPE (avis));
        AVIS_SHAPE (avis) = oldshape;

        if (newass == NULL) {
            shpavis = FREEdoFreeNode (shpavis);
        } else {
            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (shpavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            INFO_PREASSIGN (arg_info)
              = TCappendAssign (INFO_PREASSIGN (arg_info), newass);
            newass = NULL;

            for (dim = dim - 1; dim >= 0; dim--) {
                shape *shp;
                node *idxavis;
                node *shpelavis;

                shpelavis
                  = ID_AVIS (TCgetNthExpr (dim + 1, ARRAY_AELEMS (AVIS_SHAPE (avis))));

                shp = SHcreateShape (1, dim);
                idxavis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (avis)),
                                      TYmakeAKV (TYmakeSimpleType (T_int),
                                                 COmakeConstantFromShape (shp)));
                shp = SHfreeShape (shp);

                AVIS_DIM (idxavis) = TBmakeNum (1);
                AVIS_SHAPE (idxavis) = GenIntVector (TBmakeNum (1));

                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (idxavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                newass
                  = TBmakeAssign (TBmakeLet (TBmakeIds (idxavis, NULL),
                                             TCmakeIntVector (
                                               TBmakeExprs (TBmakeNum (dim), NULL))),
                                  TBmakeAssign (TBmakeLet (TBmakeIds (shpelavis, NULL),
                                                           TCmakePrf2 (F_sel,
                                                                       TBmakeId (idxavis),
                                                                       TBmakeId (
                                                                         shpavis))),
                                                newass));

                AVIS_SSAASSIGN (idxavis) = newass;
                AVIS_SSAASSIGN (shpelavis) = ASSIGN_NEXT (newass);
                AVIS_SHAPEVAROF (shpelavis) = NULL;
            }
        }

        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), newass);
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISVwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISVwith");

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static void
MakePreassign (node *avis, node *rhs, info *arg_info)
{
    DBUG_ENTER ("MakePreassign");

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), rhs), INFO_PREASSIGN (arg_info));
    AVIS_SHAPEVAROF (avis) = NULL;
    AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (arg_info);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *ISVpart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISVpart (node *arg_node, info *arg_info)
{
    node *ids;

    DBUG_ENTER ("ISVpart");

    if (NODE_TYPE (PART_GENERATOR (arg_node)) == N_generator) {
        node *ivavis = IDS_AVIS (PART_VEC (arg_node));

        if (NODE_TYPE (AVIS_DIM (ivavis)) == N_id) {
            node *dimavis = ID_AVIS (AVIS_DIM (ivavis));
            if (AVIS_SHAPEVAROF (dimavis) == ivavis) {
                MakePreassign (dimavis, TBmakeNum (1), arg_info);
            }
        }

        if (NODE_TYPE (AVIS_SHAPE (ivavis)) == N_id) {
            node *savis = ID_AVIS (AVIS_SHAPE (ivavis));

            if (AVIS_SHAPEVAROF (savis) == ivavis) {
                node *lb = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));

                if (NODE_TYPE (lb) == N_array) {
                    MakePreassign (savis, SHshape2Array (ARRAY_SHAPE (lb)), arg_info);
                } else {
                    MakePreassign (savis, DUPdoDupNode (AVIS_SHAPE (ID_AVIS (lb))),
                                   arg_info);
                }
            }
        }

        if (NODE_TYPE (AVIS_SHAPE (ivavis)) == N_array) {
            node *svar = EXPRS_EXPR (ARRAY_AELEMS (AVIS_SHAPE (ivavis)));

            if (NODE_TYPE (svar) == N_id) {
                node *svavis = ID_AVIS (svar);

                if (AVIS_SHAPEVAROF (svavis) == ivavis) {
                    node *lb = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));

                    if (NODE_TYPE (lb) == N_array) {
                        MakePreassign (svavis,
                                       TBmakeNum (SHgetExtent (ARRAY_SHAPE (lb), 0)),
                                       arg_info);
                    } else {
                        MakePreassign (svavis,
                                       TCmakePrf2 (F_idx_shape_sel, TBmakeNum (0),
                                                   DUPdoDupNode (lb)),
                                       arg_info);
                    }
                }
            }
        }

        ids = PART_IDS (arg_node);
        while (ids != NULL) {
            node *idsavis = IDS_AVIS (ids);

            if (NODE_TYPE (AVIS_DIM (idsavis)) == N_id) {
                node *dimavis = ID_AVIS (AVIS_DIM (idsavis));
                if (AVIS_SHAPEVAROF (dimavis) == idsavis) {
                    MakePreassign (dimavis, TBmakeNum (0), arg_info);
                }
            }

            if (NODE_TYPE (AVIS_SHAPE (idsavis)) == N_id) {
                node *shpavis = ID_AVIS (AVIS_SHAPE (idsavis));
                if (AVIS_SHAPEVAROF (shpavis) == idsavis) {
                    MakePreassign (shpavis, TCmakeIntVector (NULL), arg_info);
                }
            }

            ids = IDS_NEXT (ids);
        }
    }

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    } else {
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Shape Variables -->
 *****************************************************************************/
