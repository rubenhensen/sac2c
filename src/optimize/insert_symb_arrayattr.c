/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup isaa Insert Symbolic Array Attributes
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
 * @file insert_symb_arrayattr.c
 *
 * Prefix: ISAA
 *
 *****************************************************************************/
#include "insert_symb_arrayattr.h"

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
    enum { TM_all, TM_then, TM_else } travmode;
    node *vardecs;
    node *preassign;
    node *postassign;
    node *fundef;
    node *preblock;
    node *lhs;
    node *rhs;
    node *withid;
};

#define INFO_TRAVSCOPE(n) ((n)->travscope)
#define INFO_TRAVMODE(n) ((n)->travmode)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_POSTASSIGN(n) ((n)->postassign)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREBLOCK(n) ((n)->preblock)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_RHS(n) ((n)->rhs)
#define INFO_WITHID(n) ((n)->withid)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TRAVSCOPE (result) = TS_module;
    INFO_TRAVMODE (result) = TM_all;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PREBLOCK (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_RHS (result) = NULL;
    INFO_WITHID (result) = NULL;

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
 * @fn node *ISAAdoInsertShapeVariables( node *syntax_tree)
 *
 *****************************************************************************/
node *
ISAAdoInsertShapeVariables (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ISAAdoInsertShapeVariables");

    info = MakeInfo ();

    INFO_TRAVSCOPE (info) = TS_module;
    TRAVpush (TR_isaa);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAdoInsertShapeVariablesOneFundef( node *fundef)
 *
 *****************************************************************************/
node *
ISAAdoInsertShapeVariablesOneFundef (node *fundef)
{
    info *info;

    DBUG_ENTER ("ISAAdoInsertShapeVariablesOneFundef");

    info = MakeInfo ();

    INFO_TRAVSCOPE (info) = TS_fundef;
    TRAVpush (TR_isaa);
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
MakeDTProxy (node *avis, node *postass, info *arg_info)
{
    bool makeproxy = FALSE;

    DBUG_ENTER ("MakeDTProxy");

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_then:
        makeproxy = (!AVIS_HASDTTHENPROXY (avis));
        break;

    case TM_else:
        makeproxy = (!AVIS_HASDTELSEPROXY (avis));
        break;

    case TM_all:
        makeproxy = ((!AVIS_HASDTTHENPROXY (avis)) || (!AVIS_HASDTELSEPROXY (avis)));
        break;
    }

    if (makeproxy) {
        node *dimavis;
        node *shpavis;
        node *proxyavis;
        node *fundef;

        fundef = INFO_FUNDEF (arg_info);

        dimavis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (avis)),
                              TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        AVIS_DIM (dimavis) = TBmakeNum (0);
        AVIS_SHAPE (dimavis) = TCmakeIntVector (NULL);

        shpavis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (avis)),
                              TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
        AVIS_DIM (shpavis) = TBmakeNum (1);
        AVIS_SHAPE (shpavis) = TCmakeIntVector (TBmakeExprs (TBmakeId (dimavis), NULL));

        proxyavis
          = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (avis)), TYcopyType (AVIS_TYPE (avis)));
        AVIS_DIM (proxyavis) = TBmakeId (dimavis);
        AVIS_SHAPE (proxyavis) = TBmakeId (shpavis);

        FUNDEF_VARDEC (fundef)
          = TBmakeVardec (dimavis,
                          TBmakeVardec (shpavis, TBmakeVardec (proxyavis,
                                                               FUNDEF_VARDEC (fundef))));

        postass
          = TBmakeAssign (TBmakeLet (TBmakeIds (proxyavis, NULL),
                                     TCmakePrf3 (F_dtype_conv, TBmakeId (dimavis),
                                                 TBmakeId (shpavis), TBmakeId (avis))),
                          postass);
        AVIS_SSAASSIGN (proxyavis) = postass;

        postass = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL),
                                           TCmakePrf1 (F_shape, TBmakeId (avis))),
                                postass);
        AVIS_SSAASSIGN (shpavis) = postass;

        postass = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL),
                                           TCmakePrf1 (F_dim, TBmakeId (avis))),
                                postass);
        AVIS_SSAASSIGN (dimavis) = postass;

        AVIS_SUBST (avis) = proxyavis;

        switch (INFO_TRAVMODE (arg_info)) {
        case TM_then:
            AVIS_HASDTTHENPROXY (avis) = TRUE;
            break;

        case TM_else:
            AVIS_HASDTELSEPROXY (avis) = TRUE;
            break;

        case TM_all:
            AVIS_HASDTTHENPROXY (avis) = TRUE;
            AVIS_HASDTELSEPROXY (avis) = TRUE;
            break;
        }
    }

    DBUG_RETURN (postass);
}

static node *
MakeArgProxies (node *arg_node, info *arg_info)
{
    node *ass = NULL;

    DBUG_ENTER ("MakeArgProxies");

    if (arg_node != NULL) {
        ass = MakeArgProxies (ARG_NEXT (arg_node), arg_info);
        ass = MakeDTProxy (ARG_AVIS (arg_node), ass, arg_info);
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

static node *
RemoveAvisSubst (node *fundef)
{
    DBUG_ENTER ("RemoveAvisSubst");

    if (FUNDEF_ARGS (fundef) != NULL) {
        FUNDEF_ARGS (fundef) = TRAVdo (FUNDEF_ARGS (fundef), NULL);
    }
    if (FUNDEF_VARDEC (fundef) != NULL) {
        FUNDEF_VARDEC (fundef) = TRAVdo (FUNDEF_VARDEC (fundef), NULL);
    }

    DBUG_RETURN (fundef);
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
 * @fn node *ISAAfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISAAfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;

        /*
         * Resolve shape variables to dim, shape, idx_shape_sel
         * In case of cond functions, we need a copy of those variables
         */
        if (FUNDEF_ISCONDFUN (arg_node)) {
            INFO_TRAVMODE (arg_info) = TM_then;
            arg_node = RemoveAvisSubst (arg_node);
            INFO_PREBLOCK (arg_info) = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            INFO_TRAVMODE (arg_info) = TM_else;
            arg_node = RemoveAvisSubst (arg_node);
            INFO_PREBLOCK (arg_info) = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        } else {
            INFO_TRAVMODE (arg_info) = TM_all;
            arg_node = RemoveAvisSubst (arg_node);

            INFO_PREBLOCK (arg_info) = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            FUNDEF_INSTR (arg_node)
              = TCappendAssign (INFO_PREBLOCK (arg_info), FUNDEF_INSTR (arg_node));
            INFO_PREBLOCK (arg_info) = NULL;
        }

        /*
         * Clean up the AVIS_SUBST mess
         */
        arg_node = RemoveAvisSubst (arg_node);
    }

    if ((INFO_TRAVSCOPE (arg_info) == TS_module) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISAAavis");

    AVIS_SUBST (arg_node) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISAAblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAassign (node *arg_node, info *arg_info)
{
    node *preassign;
    node *postassign;

    DBUG_ENTER ("ISAAassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    preassign = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;

    postassign = INFO_POSTASSIGN (arg_info);
    INFO_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (postassign != NULL) {
        ASSIGN_NEXT (arg_node) = TCappendAssign (postassign, ASSIGN_NEXT (arg_node));
    }

    if (preassign != NULL) {
        arg_node = TCappendAssign (preassign, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISAAlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
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
 * @fn node *ISAAids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("ISAAids");

    avis = IDS_AVIS (arg_node);

    if ((NODE_TYPE (INFO_RHS (arg_info)) != N_ap)
        && (!((NODE_TYPE (INFO_RHS (arg_info)) == N_prf)
              && (PRF_PRF (INFO_RHS (arg_info)) == F_type_conv)))) {
        if (AVIS_DIM (avis) == NULL) {
            if (TUdimKnown (AVIS_TYPE (avis))) {
                AVIS_DIM (avis) = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));
            } else {
                INFO_PREASSIGN (arg_info)
                  = TCappendAssign (INFO_PREASSIGN (arg_info),
                                    MDEdoMakeDimExpression (INFO_RHS (arg_info), avis,
                                                            INFO_LHS (arg_info),
                                                            INFO_FUNDEF (arg_info)));
            }
        }

        if ((AVIS_DIM (avis) != NULL) && (AVIS_SHAPE (avis) == NULL)) {
            if (TUshapeKnown (AVIS_TYPE (avis))) {
                AVIS_SHAPE (avis) = SHshape2Array (TYgetShape (AVIS_TYPE (avis)));
            } else {
                INFO_PREASSIGN (arg_info)
                  = TCappendAssign (INFO_PREASSIGN (arg_info),
                                    MSEdoMakeShapeExpression (INFO_RHS (arg_info), avis,
                                                              INFO_LHS (arg_info),
                                                              INFO_FUNDEF (arg_info)));
            }
        }
    } else {
        if (!((FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info)))
              && (NODE_TYPE (INFO_RHS (arg_info)) == N_ap)
              && (AP_FUNDEF (INFO_RHS (arg_info)) == INFO_FUNDEF (arg_info)))) {
            INFO_POSTASSIGN (arg_info)
              = MakeDTProxy (avis, INFO_POSTASSIGN (arg_info), arg_info);
        }
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAwith (node *arg_node, info *arg_info)
{
    node *oldwithid;

    DBUG_ENTER ("ISAAwith");

    oldwithid = INFO_WITHID (arg_info);
    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_WITHID (arg_info) = oldwithid;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAApart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAApart (node *arg_node, info *arg_info)
{
    node *ids;

    DBUG_ENTER ("ISAApart");

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (NODE_TYPE (PART_GENERATOR (arg_node)) == N_generator) {
        node *ivavis = IDS_AVIS (PART_VEC (arg_node));

        if (AVIS_DIM (ivavis) == NULL) {
            AVIS_DIM (ivavis) = TBmakeNum (1);
        }

        if (AVIS_SHAPE (ivavis) == NULL) {
            node *lb = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));

            if (NODE_TYPE (lb) == N_array) {
                AVIS_SHAPE (ivavis) = SHshape2Array (ARRAY_SHAPE (lb));
            } else {
                AVIS_SHAPE (ivavis) = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (lb)));
            }
        }

        ids = PART_IDS (arg_node);
        while (ids != NULL) {
            node *idsavis = IDS_AVIS (ids);

            if (AVIS_DIM (idsavis) == NULL) {
                AVIS_DIM (idsavis) = TBmakeNum (0);
            }

            if (AVIS_SHAPE (idsavis) == NULL) {
                AVIS_SHAPE (idsavis) = TCmakeIntVector (NULL);
            }

            ids = IDS_NEXT (ids);
        }
    }

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISAAcode");

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    AVIS_SUBST (IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)))) = NULL;

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISAAid");

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAcond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISAAcond");

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_then:
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

        BLOCK_INSTR (COND_THEN (arg_node))
          = PrependAssign (INFO_PREBLOCK (arg_info), BLOCK_INSTR (COND_THEN (arg_node)));
        INFO_PREBLOCK (arg_info) = NULL;
        break;

    case TM_else:
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

        BLOCK_INSTR (COND_ELSE (arg_node))
          = PrependAssign (INFO_PREBLOCK (arg_info), BLOCK_INSTR (COND_ELSE (arg_node)));
        INFO_PREBLOCK (arg_info) = NULL;

        break;

    case TM_all:
        arg_node = TRAVcont (arg_node, arg_info);
        break;

    default:
        DBUG_ASSERT ((0), "Illegal traversal mode");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAfuncond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ISAAfuncond");

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_then:
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
        break;

    case TM_else:
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
        break;

    case TM_all:
        arg_node = TRAVcont (arg_node, arg_info);
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
