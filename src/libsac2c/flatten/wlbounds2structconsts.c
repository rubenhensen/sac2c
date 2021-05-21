/**
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

#define DBUG_PREFIX "WLBSC"
#include "debug.h"

#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "traverse.h"
#include "pattern_match.h"
#include "globals.h"
#include "str.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *nassigns;
    ntype *idxtype;
    bool genflat;
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
#define INFO_GENFLAT(n) (n->genflat)

/**
 * INFO functions
 */
static info *
MakeInfo (bool flat)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_IV_TYPE (result) = NULL;
    INFO_GENFLAT (result) = flat;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();
    avis = TBmakeAvis (TRAVtmpVarName (prefix), ty);

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateArrayOfShapeSels( node *id_avis, int dim, info *arg_info)
 *
 *   @brief creates a structural constant of shape selections into "id"
 *          and either
 *          -  returns that array (in case INFO_GENFLAT is FALSE)
 *             or
 *          -  assigns it to a new var sc_bound and returns a corresponding
 *             id node (otherwise)
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
 *   @return node *        :  the array or the avis of sc_bound
 *
 ******************************************************************************/

static node *
CreateArrayOfShapeSels (node *id_avis, int dim, info *arg_info)
{
    node *res = NULL, *assigns = NULL;
    node *res_avis;
    node *iv_avis, *elem_avis;
    int i;

    DBUG_ENTER ();

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

        assigns = TBmakeAssign (TBmakeLet (TBmakeIds (iv_avis, NULL),
                                           TCcreateIntVector (1, i, 0)),
                                assigns);
        AVIS_SSAASSIGN (iv_avis) = assigns;

        /*
         * create element of structural constant
         */
        res = TBmakeExprs (TBmakeId (elem_avis), res);
    }

    res = TCmakeIntVector (res);

    /*
     * create structural constant
     */
    if (INFO_GENFLAT (arg_info)) {

        res_avis = CreateAvisAndInsertVardec ("sc_bound",
                                              TYmakeAKS (TYmakeSimpleType (T_int),
                                                         SHcreateShape (1, dim)),
                                              arg_info);
        res = TBmakeAssign (TBmakeLet (TBmakeIds (res_avis, NULL), res), NULL);
        assigns = TCappendAssign (assigns, res);

        AVIS_SSAASSIGN (res_avis) = res;

        res = TBmakeId (res_avis);
    }

    INFO_PREASSIGN (arg_info) = TCappendAssign (assigns, INFO_PREASSIGN (arg_info));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *EnsureStructConstant(node *arg_node, ntype *type, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_id
 *           info *arg_info:  N_info
 *   @return node *        :  N_id
 ******************************************************************************/
static node *
EnsureStructConstant (node *bound, ntype *type, info *arg_info)
{
    static pattern *pat = NULL;
    static node *array = NULL;
    node *new_bound;
    int dim;

    DBUG_ENTER ();

    if (pat == NULL) {
        pat = PMarray (1, PMAgetNode (&array), 1, PMskip (0));
    }

    DBUG_EXECUTE ( if (NODE_TYPE (bound) == N_id) {
                       DBUG_PRINT ("ensuring structural constant for \"%s\"", ID_NAME (bound));
                   } else if (NODE_TYPE (bound) == N_array) {
                       DBUG_PRINT ("ensuring structural constant ...");
                   } else {
                       DBUG_ASSERT (FALSE, "found neither N_id nor N_array in generator/shape position");
                   } );

    if (PMmatchFlat (pat, bound)) {
        /* this is somehow defined as an array */

        if (!INFO_GENFLAT (arg_info)) {
            if (PMmatchFlat (pat, bound)) {
                /* but maybe flattened */
                new_bound = array;

                DBUG_PRINT ("...potentially already inline, store" F_PTR, (void *)array);

                if (!PMmatchExact (pat, bound)) {
                    /* it is flattened -> de-flatten */
                    DBUG_PRINT ("...was flat, replacing.");
                    bound = FREEdoFreeTree (bound);
                    bound = DUPdoDupTree (new_bound);
                }
            } else {
                /* there are some obstacles in the way, i.e. extrema -> create vector */
                DBUG_PRINT ("...otherwise defined.");
                dim = SHgetExtent (TYgetShape (type), 0);
                new_bound = CreateArrayOfShapeSels (ID_AVIS (bound), dim, arg_info);
                bound = FREEdoFreeTree (bound);
                bound = new_bound;
            }
        }
    } else if (TUshapeKnown (type)) {
        /* not an array at all but AKS -> create vector */
        DBUG_PRINT ("...creating struct const.");
        dim = SHgetExtent (TYgetShape (type), 0);
        new_bound = CreateArrayOfShapeSels (ID_AVIS (bound), dim, arg_info);
        bound = FREEdoFreeTree (bound);
        bound = new_bound;
    } else {
        DBUG_PRINT ("neither array nor AKS/AKV");
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
    DBUG_ENTER ();

    DBUG_PRINT ("WLPartitionGeneration module-wise");

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
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

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
    DBUG_ENTER ();
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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
 *   @brief  ensure structural constants for shape
 *
 *****************************************************************************/
node *
WLBSCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_id) {
        GENARRAY_SHAPE (arg_node)
          = EnsureStructConstant (GENARRAY_SHAPE (arg_node),
                                  ID_NTYPE (GENARRAY_SHAPE (arg_node)), arg_info);
    }

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

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

static node *
Wlbounds2structConsts (node *arg_node, bool flat)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo (flat);

    TRAVpush (TR_wlbsc);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

node *
WLBSCdoWlbounds2structConsts (node *arg_node)
{
    DBUG_ENTER ();
    DBUG_RETURN (Wlbounds2structConsts (arg_node, TRUE));
}

node *
WLBSCdoWlbounds2nonFlatStructConsts (node *arg_node)
{
    DBUG_ENTER ();
    DBUG_RETURN (Wlbounds2structConsts (arg_node, FALSE));
}

#undef DBUG_PREFIX
