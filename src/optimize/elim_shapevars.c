/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup esv Eliminate Shape Variables
 *
 * This traversal removes all expressions for dim and shape from AVIS nodes.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file elim_shapevars.c
 *
 * Prefix: ESV
 *
 *****************************************************************************/
#include "elim_shapevars.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    enum { TM_all, TM_then, TM_else, TM_clearsubst } travmode;
    node *preblock;
    node *postass;
};

#define INFO_TRAVMODE(n) ((n)->travmode)
#define INFO_PREBLOCK(n) ((n)->preblock)
#define INFO_POSTASS(n) ((n)->postass)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TRAVMODE (result) = TM_all;
    INFO_PREBLOCK (result) = NULL;
    INFO_POSTASS (result) = NULL;

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
 * @fn node *ESVdoEliminateShapeVariables( node *syntax_tree)
 *
 *****************************************************************************/
node *
ESVdoEliminateShapeVariables (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ESVdoEliminateShapeVariables");

    info = MakeInfo ();

    TRAVpush (TR_esv);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
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
static void
ResetAvis (node *avis, node *assign)
{
    DBUG_ENTER ("ResetAvis");

    AVIS_SHAPEVAROF (avis) = NULL;
    AVIS_SSAASSIGN (avis) = assign;

    if (AVIS_DIM (avis) != NULL) {
        AVIS_DIM (avis) = FREEdoFreeNode (AVIS_DIM (avis));
    }

    if (AVIS_SHAPE (avis) != NULL) {
        AVIS_SHAPE (avis) = FREEdoFreeNode (AVIS_SHAPE (avis));
    }

    DBUG_VOID_RETURN;
}

static node *
MakeDimAssign (node *dimavis, node *avis, node *postass)
{
    DBUG_ENTER ("MakeDimAssign");

    postass = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL),
                                       TCmakePrf1 (F_dim, TBmakeId (avis))),
                            postass);

    ResetAvis (dimavis, postass);

    DBUG_RETURN (postass);
}

static node *
MakeShapeAssign (node *shapeavis, node *avis, node *postass)
{
    DBUG_ENTER ("MakeShapeAssign");

    postass = TBmakeAssign (TBmakeLet (TBmakeIds (shapeavis, NULL),
                                       TCmakePrf1 (F_shape, TBmakeId (avis))),
                            postass);

    ResetAvis (shapeavis, postass);

    DBUG_RETURN (postass);
}

static node *
MakeShapeSelAssign (node *shpelavis, int idx, node *avis, node *postass)
{
    DBUG_ENTER ("MakeShapeSelAssign");

    postass = TBmakeAssign (TBmakeLet (TBmakeIds (shpelavis, NULL),
                                       TCmakePrf2 (F_idx_shape_sel, TBmakeNum (idx),
                                                   TBmakeId (avis))),
                            postass);

    ResetAvis (shpelavis, postass);

    DBUG_RETURN (postass);
}

static node *
MakeShapeVar (node *avis, node *fundef, bool subst)
{
    node *res = avis;

    DBUG_ENTER ("MakeShapeVar");

    if (subst) {
        res = DUPdoDupNode (avis);
        AVIS_NAME (res) = ILIBfree (AVIS_NAME (res));
        AVIS_NAME (res) = ILIBtmpVarName (AVIS_NAME (avis));

        FUNDEF_VARDEC (fundef) = TBmakeVardec (res, FUNDEF_VARDEC (fundef));
        AVIS_SUBST (avis) = res;
    }

    DBUG_RETURN (res);
}

static node *
MakeShapeVarAssigns (node *arg_node, node *fundef, bool subst)
{
    node *ass = NULL;

    DBUG_ENTER ("MakeShapeVarAssigns");

    if (arg_node != NULL) {
        node *avis;

        ass = MakeShapeVarAssigns (ARG_NEXT (arg_node), fundef, subst);

        avis = ARG_AVIS (arg_node);

        if ((AVIS_SHAPE (avis) != NULL) && (NODE_TYPE (AVIS_SHAPE (avis)) == N_array)) {
            node *exprs = ARRAY_AELEMS (AVIS_SHAPE (avis));
            int i = 0;

            while (exprs != NULL) {
                node *shpel = EXPRS_EXPR (exprs);

                if (NODE_TYPE (shpel) == N_id) {
                    node *shpelavis = MakeShapeVar (ID_AVIS (shpel), fundef, subst);
                    ass = MakeShapeSelAssign (shpelavis, i, avis, ass);
                }

                exprs = EXPRS_NEXT (exprs);
                i += 1;
            }
        }

        if ((AVIS_SHAPE (avis) != NULL) && (NODE_TYPE (AVIS_SHAPE (avis)) == N_id)) {
            node *shapeavis = MakeShapeVar (ID_AVIS (AVIS_SHAPE (avis)), fundef, subst);
            ass = MakeShapeAssign (shapeavis, avis, ass);
        }

        if ((AVIS_DIM (avis) != NULL) && (NODE_TYPE (AVIS_DIM (avis)) == N_id)) {
            node *dimavis = MakeShapeVar (ID_AVIS (AVIS_DIM (avis)), fundef, subst);
            ass = MakeDimAssign (dimavis, avis, ass);
        }
    }

    DBUG_RETURN (ass);
}

static node *
PrependAssign (node *prefix, node *rest)
{
    DBUG_ENTER ("PrependAssign");

    if (prefix != NULL) {
        if (NODE_TYPE (rest) == N_empty) {
            rest = FREEdoFreeNode (rest);
            rest = prefix;
        } else {
            rest = TCappendAssign (prefix, rest);
        }
    }

    DBUG_RETURN (rest);
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
 * @fn node *ESVfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * Ensure no entries are left in AVIS_SUBST
         */
        INFO_TRAVMODE (arg_info) = TM_clearsubst;

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), arg_info);
        }

        /*
         * Resolve shape variables to dim, shape, idx_shape_sel
         * In case of cond functions, we need a copy of those variables
         */
        if (FUNDEF_ISCONDFUN (arg_node)) {
            INFO_PREBLOCK (arg_info)
              = MakeShapeVarAssigns (FUNDEF_ARGS (arg_node), arg_node, FALSE);

            INFO_TRAVMODE (arg_info) = TM_then;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            INFO_PREBLOCK (arg_info)
              = MakeShapeVarAssigns (FUNDEF_ARGS (arg_node), arg_node, TRUE);

            INFO_TRAVMODE (arg_info) = TM_else;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        } else {
            FUNDEF_INSTR (arg_node)
              = TCappendAssign (MakeShapeVarAssigns (FUNDEF_ARGS (arg_node), arg_node,
                                                     FALSE),
                                FUNDEF_INSTR (arg_node));

            if (FUNDEF_ISDOFUN (arg_node)) {
                INFO_TRAVMODE (arg_info) = TM_then;
            } else {
                INFO_TRAVMODE (arg_info) = TM_all;
            }
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        /*
         * Now remove the old shape variables
         */
        INFO_TRAVMODE (arg_info) = TM_all;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = TRAVdo (FUNDEF_VARDEC (arg_node), arg_info);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVavis");

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_clearsubst:
        AVIS_SUBST (arg_node) = NULL;
        break;

    case TM_all:
        AVIS_SUBST (arg_node) = NULL;

        if (AVIS_DIM (arg_node) != NULL) {
            AVIS_DIM (arg_node) = FREEdoFreeNode (AVIS_DIM (arg_node));
        }

        if (AVIS_SHAPE (arg_node) != NULL) {
            AVIS_SHAPE (arg_node) = FREEdoFreeNode (AVIS_SHAPE (arg_node));
        }
        break;

    default:
        DBUG_ASSERT ((0), "Illegal traversal mode");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    ASSIGN_NEXT (arg_node)
      = TCappendAssign (INFO_POSTASS (arg_info), ASSIGN_NEXT (arg_node));
    INFO_POSTASS (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (NODE_TYPE (LET_EXPR (arg_node)) != N_funcond) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("ESVids");

    avis = IDS_AVIS (arg_node);

    if (AVIS_SHAPE (avis) != NULL) {
        node *shape = AVIS_SHAPE (avis);

        if ((NODE_TYPE (shape) == N_id) && (AVIS_SHAPEVAROF (ID_AVIS (shape)) == avis)) {
            node *shapeavis = ID_AVIS (shape);

            INFO_POSTASS (arg_info)
              = MakeShapeAssign (shapeavis, avis, INFO_POSTASS (arg_info));
        }

        if (NODE_TYPE (shape) == N_array) {
            node *exprs = ARRAY_AELEMS (shape);
            while (exprs != NULL) {
                int i = 0;
                node *shpel = EXPRS_EXPR (exprs);

                if ((NODE_TYPE (shpel) == N_id)
                    && (AVIS_SHAPEVAROF (ID_AVIS (shpel)) == avis)) {
                    node *shpelavis = ID_AVIS (shpel);

                    INFO_POSTASS (arg_info)
                      = MakeShapeSelAssign (shpelavis, i, avis, INFO_POSTASS (arg_info));
                }

                exprs = EXPRS_NEXT (exprs);
                i += 1;
            }
        }
    }

    if (AVIS_DIM (avis) != NULL) {
        node *dim = AVIS_DIM (avis);

        if ((NODE_TYPE (dim) == N_id) && (AVIS_SHAPEVAROF (ID_AVIS (dim)) == avis)) {
            node *dimavis = ID_AVIS (dim);

            INFO_POSTASS (arg_info)
              = MakeDimAssign (dimavis, avis, INFO_POSTASS (arg_info));
        }
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVid");

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVcond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVcond");

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_then:
        BLOCK_INSTR (COND_THEN (arg_node))
          = PrependAssign (INFO_PREBLOCK (arg_info), BLOCK_INSTR (COND_THEN (arg_node)));
        INFO_PREBLOCK (arg_info) = NULL;

        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        break;

    case TM_else:
        BLOCK_INSTR (COND_ELSE (arg_node))
          = PrependAssign (INFO_PREBLOCK (arg_info), BLOCK_INSTR (COND_ELSE (arg_node)));
        INFO_PREBLOCK (arg_info) = NULL;

        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT ((0), "Illegal traversal mode");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVfuncond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVfuncond");

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_then:
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
        break;

    case TM_else:
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT ((0), "Illegal traversal mode");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Shape Variables -->
 *****************************************************************************/
