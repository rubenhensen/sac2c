/**
 *
 * $Id$
 *
 * @file WLbounds2structConsts.c
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   | yes |       |
 * can be called on N_fundef               |   -----   | yes |       |
 * expects LaC funs                        |   -----   | yes |       |
 * follows N_ap to LaC funs                |   -----   | no  |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    | yes |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |     |       |
 * utilises SAA annotations                |   -----   |     |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    |     |       |
 * tolerates flattened Generators          |    yes    |     |       |
 * tolerates flattened operation parts     |    yes    |     |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |     |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |     |       |
 * =============================================================================
 * </pre>
 *
 *
 */

#include "wlbounds2structconsts.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "dbug.h"
#include "dbug.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "traverse.h"
#include "pattern_match.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *nassigns;
    ntype *idxtype;
    bool onefundef;
};

/*******************************************************************************
 *  Usage of arg_info:
 *  - node : FUNDEF  : pointer to last fundef node. needed to access vardecs.
 *  - node : PREASSIGN: pointer to a list of new assigns, which where needed
 *                     to build structural constants and had to be inserted
 *                     in front of the considered with-loop.
 *
 ******************************************************************************/
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->nassigns)
#define INFO_IV_TYPE(n) (n->idxtype)
#define INFO_ONEFUNDEF(n) (n->onefundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_IV_TYPE (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;

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
 * @fn node *CreateAvisAndInsertVardec( char *prefix, ntype ty, info *arg_info)
 *
 *   @brief creates an avis node for a temp variable with given prefix and
 *          type and inserts a corresponding vardec into the AST.
 *
 ******************************************************************************/
static node *
CreateAvisAndInsertVardec (char *prefix, ntype *ty, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("CreateAvisAndInsertVardec");
    avis = TBmakeAvis (TRAVtmpVarName (prefix), ty);

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateArrayOfShapeSels( node *id_avis, int dim, info *arg_info)
 *
 *   @brief creates a structural constant of shape selections into "id"
 *          and assigns it to a new var sc_bound:
 *
 *     sc_iv_0 = [0];
 *     sc_e_0 = _sel_VxA_( sc_iv_0, id);
 *     ...
 *     sc_iv_n = [dim-1];
 *     sc_e_n = _sel_VxA_( sc_iv_n, id);
 *     sc_bound = [ sc_e_0, ..., sc_e_n];
 *
 *   @param  node *array   :  N_id
 *           int dim       :
 *           info *arg_info:  info node, will contain flattened code
 *   @return node *        :  the avis of sc_bound
 *
 ******************************************************************************/

static node *
CreateArrayOfShapeSels (node *id_avis, int dim, info *arg_info)
{
    node *res = NULL, *assigns = NULL;
    node *res_avis;
    node *iv_avis, *elem_avis;
    int i;

    DBUG_ENTER ("CreateArrayOfShapeSels");

    for (i = dim - 1; i >= 0; i--) {
        iv_avis = CreateAvisAndInsertVardec ("sc_iv",
                                             TYmakeAKS (TYmakeSimpleType (T_int),
                                                        SHcreateShape (1, 1)),
                                             arg_info);
        elem_avis = CreateAvisAndInsertVardec ("sc_e",
                                               TYmakeAKS (TYmakeSimpleType (T_int),
                                                          SHcreateShape (0)),
                                               arg_info);
        assigns = TBmakeAssign (TBmakeLet (TBmakeIds (elem_avis, NULL),
                                           TCmakePrf2 (F_sel_VxA, TBmakeId (iv_avis),
                                                       TBmakeId (id_avis))),
                                assigns);
        AVIS_SSAASSIGN (elem_avis) = assigns;

        assigns
          = TBmakeAssign (TBmakeLet (TBmakeIds (iv_avis, NULL), TCcreateIntVector (1, i)),
                          assigns);
        AVIS_SSAASSIGN (iv_avis) = assigns;

        /*
         * create element of structural constant
         */
        res = TBmakeExprs (TBmakeId (elem_avis), res);
    }

    /*
     * create structural constant
     */
    res_avis = CreateAvisAndInsertVardec ("sc_bound",
                                          TYmakeAKS (TYmakeSimpleType (T_int),
                                                     SHcreateShape (1, dim)),
                                          arg_info);
    res = TBmakeAssign (TBmakeLet (TBmakeIds (res_avis, NULL), TCmakeIntVector (res)),
                        INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (res_avis) = res;

    INFO_PREASSIGN (arg_info) = TCappendAssign (assigns, res);

    DBUG_RETURN (res_avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *EnsureStructConstant(node *arg_node, ntype *type, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  N_info
 *   @return node *        :  N_module
 ******************************************************************************/
static node *
EnsureStructConstant (node *bound, ntype *type, info *arg_info)
{
    static pattern *pat = NULL;
    int dim;

    DBUG_ENTER ("EnsureStructConstant");

    if (pat == NULL) {
        pat = PMarray (0, 1, PMskip (0));
    }
    if (!PMmatchFlat (pat, bound) && TUshapeKnown (type)) {
        dim = SHgetExtent (TYgetShape (type), 0);
        bound = TBmakeId (CreateArrayOfShapeSels (ID_AVIS (bound), dim, arg_info));
    }

    DBUG_RETURN (bound);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLBSCmodule(node *arg_node, info *arg_info)
 *
 *   @brief first traversal of function definitions of WLPartitionGeneration
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  N_info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
WLBSCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLBSCmodule");

    DBUG_PRINT ("WLBSC", ("WLPartitionGeneration module-wise"));

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLBSCfundef(node *arg_node, info *arg_info)
 *
 *   @brief starts the traversal of the given fundef.
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
WLBSCfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("WLBSCfundef");

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLBSCassign(node *arg_node, info *arg_info)
 *
 *   @brief traverse instruction. If this creates new assignments in
 *          INFO_PREASSIGN these are inserted in front of the actual one.
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
WLBSCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLBSCassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);

    INFO_PREASSIGN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLBSCwith(node *arg_node, info *arg_info)
 *
 *   @brief ensure that the code is traversed BEFORE the partitions;
 *          otherwise, any created assignments would appear in the body
 *          rather than the surrounding context.
 *
 ******************************************************************************/

node *
WLBSCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLBSCwith");
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLBSCpart(node *arg_node, info *arg_info)
 *
 *   @brief  insert withid type into INFO_IV_TYPE while traversing the
 *           genarator
 *
 ******************************************************************************/

node *
WLBSCpart (node *arg_node, info *arg_info)
{
    ntype *old_iv_type;

    DBUG_ENTER ("WLBSCpart");

    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    old_iv_type = INFO_IV_TYPE (arg_info);
    INFO_IV_TYPE (arg_info) = IDS_NTYPE (PART_VEC (arg_node));
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    INFO_IV_TYPE (arg_info) = old_iv_type;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLBSCgenerator(node *arg_node, info *arg_info)
 *
 *   @brief  ensure struct constants for all components of each generator.
 *
 ******************************************************************************/

node *
WLBSCgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLBSCgenerator");

    GENERATOR_BOUND1 (arg_node)
      = EnsureStructConstant (GENERATOR_BOUND1 (arg_node), INFO_IV_TYPE (arg_info),
                              arg_info);
    GENERATOR_BOUND2 (arg_node)
      = EnsureStructConstant (GENERATOR_BOUND2 (arg_node), INFO_IV_TYPE (arg_info),
                              arg_info);
    if (GENERATOR_STEP (arg_node) != NULL) {
        GENERATOR_STEP (arg_node)
          = EnsureStructConstant (GENERATOR_STEP (arg_node), INFO_IV_TYPE (arg_info),
                                  arg_info);
    }
    if (GENERATOR_WIDTH (arg_node) != NULL) {
        GENERATOR_WIDTH (arg_node)
          = EnsureStructConstant (GENERATOR_WIDTH (arg_node), INFO_IV_TYPE (arg_info),
                                  arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLBSCgenarray(node *arg_node, info *arg_info)
 *
 *   @brief  ensure struct constants for genarray shape.
 *
 ******************************************************************************/

node *
WLBSCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLBSCgenarray");

    if (N_id == NODE_TYPE (GENARRAY_SHAPE (arg_node))) {
        GENARRAY_SHAPE (arg_node)
          = EnsureStructConstant (GENARRAY_SHAPE (arg_node),
                                  ID_NTYPE (GENARRAY_SHAPE (arg_node)), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLBSCdoWlbounds2structConsts( node *arg_node)
 *
 *   @brief  Starting point for the partition generation if it was called
 *           from main.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLBSCdoWlbounds2structConsts (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("WLBSCdoWlbounds2structConsts");

    arg_info = MakeInfo ();

    if (NODE_TYPE (arg_node) == N_module) {
        INFO_ONEFUNDEF (arg_info) = FALSE;
    } else if (NODE_TYPE (arg_node) == N_fundef) {
        INFO_ONEFUNDEF (arg_info) = FALSE;
    } else {
        DBUG_ASSERT (FALSE, "Illegal call to WLBSCdoWlbounds2structConsts!");
    }

    TRAVpush (TR_wlbsc);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
