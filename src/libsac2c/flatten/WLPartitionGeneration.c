/**
 *
 * @file WLPartition Generation.c
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
 * In this traversal, AKS and AKD genarray/modarray withloops containing default
 * partitions are transformed into withloops with full partitions.
 *
 * Ex. of AKS withloop:
 *    A = with {
 *         ([3] <= iv < [6]) {
 *           res = ...;
 *         } : res
 *         default : d;
 *        } : genarray([9]);
 *
 * is transformed into this form:
 *
 *    t0 = 0;
 *    t3 = 3;
 *    t7 = 7;
 *    t9 = 9;
 *    A = with {
 *         ([3] <= iv < [6]) {
 *           res = ...;
 *         }: res
 *         (t0 <= iv < t3) : d;
 *         (t7 <= iv < t9) : d;
 *        } : genarray([9]);
 *
 * Note here, that the generator bounds are NOT modified. However, this should
 * work in the flattened as well as in the non-flattened case!
 *
 * Ex. of AKD withloop:
 *   int[1] a;
 *   int[1] b;
 *   int[1] c;
 *
 *    B = with {
 *         (a < iv <= b) {
 *           res = ...;
 *         }: res
 *         default : d;
 *        } : genarray(c);
 *
 * is transformed into
 *
 *    t0 = [0];
 *    A = with {
 *         (a <= iv < b) {
 *           res = ...;
 *         }: res;
 *         (t0 <= iv < a) : d;
 *         (b <= iv < c) : d;
 *        } : genarray(c0);
 *
 */

#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"
#include "str.h"
#include "memory.h"
#include "shape.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "globals.h"

#define DBUG_PREFIX "WLPG"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "wlanalysis.h"
#include "LookUpTable.h"
#include "wldefaultpartition.h"
#include "WLPartitionGeneration.h"
#include "pattern_match.h"
#include "deserialize.h"
#include "namespaces.h"
#include "wls.h"
#include "with_loop_utilities.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *nassigns;
};

/*******************************************************************************
 *  Usage of arg_info:
 *  - node : FUNDEF    : pointer to last fundef node. needed to access vardecs.
 *  - node : PREASSIGNS: pointer to a list of new assigns, which where needed
 *                       to build structural constants and had to be inserted
 *                       in front of the considered with-loop.
 *
 ******************************************************************************/
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->nassigns)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;

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
 * @fn node *CreateMaxFrameShapeAvis( node *withop, int fdim, info *arg_info)
 *
 *   @brief in case withop is genarray( shp, def):
 *             just return the avis of shp
 *
 *          in case of modarray(a) , we create and insert into arg_info:
 *                 shp = _shape_A_( a)
 *                 elems = fdim;
 *                 new_shp = _take_SxV_( elems, shp);
 *            and we return the avis of new_shp
 *
 ******************************************************************************/

static node *
CreateMaxFrameShapeAvis (node *withop, int fdim, info *arg_info)
{
    node *res_avis;
    node *shp_avis, *elems_avis, *nshp_avis;

    DBUG_ENTER ();

    switch (NODE_TYPE (withop)) {
    case N_genarray:
        if (N_id == NODE_TYPE (GENARRAY_SHAPE (withop))) {
            res_avis = ID_AVIS (GENARRAY_SHAPE (withop));
        } else {
            DBUG_ASSERT (N_array == NODE_TYPE (GENARRAY_SHAPE (withop)),
                         ("Expected N_array"));
            shp_avis = CreateAvisAndInsertVardec ("shp",
                                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                                             SHcreateShape (1, fdim)),
                                                  arg_info);
            INFO_PREASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (shp_avis, NULL),
                                         DUPdoDupTree (GENARRAY_SHAPE (withop))),
                              INFO_PREASSIGN (arg_info));
            AVIS_SSAASSIGN (shp_avis) = INFO_PREASSIGN (arg_info);
            res_avis = shp_avis;
        }
        break;
    case N_modarray:
        shp_avis = CreateAvisAndInsertVardec ("shp",
                                              TYmakeAKD (TYmakeSimpleType (T_int), 1,
                                                         SHmakeShape (0)),
                                              arg_info);
        elems_avis = CreateAvisAndInsertVardec ("elems",
                                                TYmakeAKS (TYmakeSimpleType (T_int),
                                                           SHcreateShape (0)),
                                                arg_info);
        nshp_avis = CreateAvisAndInsertVardec ("new_shp",
                                               TYmakeAKS (TYmakeSimpleType (T_int),
                                                          SHcreateShape (1, fdim)),
                                               arg_info);

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (nshp_avis, NULL),
                                     TCmakePrf2 (F_take_SxV, TBmakeId (elems_avis),
                                                 TBmakeId (shp_avis))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (nshp_avis) = INFO_PREASSIGN (arg_info);

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (elems_avis, NULL), TBmakeNum (fdim)),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (elems_avis) = INFO_PREASSIGN (arg_info);

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (shp_avis, NULL),
                                     TCmakePrf1 (F_shape_A, TBmakeId (ID_AVIS (
                                                              MODARRAY_ARRAY (withop))))),
                          INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (shp_avis) = INFO_PREASSIGN (arg_info);
        res_avis = nshp_avis;
        break;
    default:
        res_avis = NULL;
        DBUG_UNREACHABLE ("CreateMaxFrameShapeAvis called on other than"
                          " genarray or modarray WL!");
    }

    DBUG_RETURN (res_avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateVarOfHomogeneousIntVect( int fdim, int val, info *arg_info)
 *
 *   @brief creates an Array of length (fdim) and valueis (val) and
 *          assigns it to a new tmp variable, whose avis is returned:
 *
 *            zeros = [ val , ... , val];
 *
 ******************************************************************************/

static node *
CreateVarOfHomogeneousIntVect (int fdim, int val, info *arg_info)
{
    node *zeros_avis;
    node *exprs;
    int i;

    DBUG_ENTER ();

    zeros_avis = CreateAvisAndInsertVardec ("zeros",
                                            TYmakeAKS (TYmakeSimpleType (T_int),
                                                       SHcreateShape (1, fdim)),
                                            arg_info);

    exprs = NULL;
    for (i = 0; i < fdim; i++) {
        exprs = TBmakeExprs (TBmakeNum (val), exprs);
    }

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (zeros_avis, NULL), TCmakeIntVector (exprs)),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (zeros_avis) = INFO_PREASSIGN (arg_info);

    DBUG_RETURN (zeros_avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *CutSlices( node *ls, node *us, int dim,
 *                       node *wln, node *coden, info *arg_info,
 *                       node *withid)
 *
 *   @brief  Creates a (full) partition by adding new N_generator nodes
 *           to the N_with node.
 *           If the known part is a grid, this is ignored here (so the
 *           resulting N_with node may still not be a full partition,
 *           see CompleteGrid()).
 *
 *   @param  node *ls, *us : bounds of the whole array
 *           int dim       : number of elements in ls, us, l, u
 *           node *wln     : Pointer of N_with node where the new generators
 *                           shall be inserted.
 *           node *coden   : Pointer of N_code node where the new generators
 *                           shall point to.
 *           info *arg_info: local information block
 *   @return node *        : modified N_with
 ******************************************************************************/

static node *
CutSlices (node *min_avis, node *max_avis, int fdim, node *part_lb_avis,
           node *part_ub_avis, node *def_withid, node *def_coden, info *arg_info)
{
    int d;
    node *res = NULL, *npart, *npart2, *assigns;
    node *axis_avis, *lmax_avis, *umin_avis, *nmin_avis, *nmax_avis;
    ntype *bound_type;
    ntype *scalar_type;

    DBUG_ENTER ();

    for (d = 0; d < fdim; d++) {
        /*
         * First,  we create:
         *
         * axis = d;
         * lmax, umin, nmin, nmax =
         *      sacprelude::partitionSlicer( min, max, axis, lb, ub);
         *
         * and insert it into INFO_PREASSIGN:
         */
        bound_type = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, fdim));
        scalar_type = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0));
        axis_avis = CreateAvisAndInsertVardec ("axis", scalar_type, arg_info);
        lmax_avis = CreateAvisAndInsertVardec ("lmax", TYcopyType (bound_type), arg_info);
        umin_avis = CreateAvisAndInsertVardec ("umin", TYcopyType (bound_type), arg_info);
        nmin_avis = CreateAvisAndInsertVardec ("nmin", TYcopyType (bound_type), arg_info);
        nmax_avis = CreateAvisAndInsertVardec ("nmax", TYcopyType (bound_type), arg_info);
        assigns = TBmakeAssign (
          TBmakeLet (TCcreateIdsChainFromAvises (4, lmax_avis, umin_avis, nmin_avis,
                                                 nmax_avis),
                     DSdispatchFunCall (NSgetNamespace (global.preludename),
                                        "partitionSlicer",
                                        TCcreateExprsChainFromAvises (5, min_avis,
                                                                      max_avis, axis_avis,
                                                                      part_lb_avis,
                                                                      part_ub_avis))),
          NULL);
        AVIS_SSAASSIGN (lmax_avis) = assigns;
        AVIS_SSAASSIGN (umin_avis) = assigns;
        AVIS_SSAASSIGN (nmin_avis) = assigns;
        AVIS_SSAASSIGN (nmax_avis) = assigns;

        assigns = TBmakeAssign (TBmakeLet (TBmakeIds (axis_avis, NULL), TBmakeNum (d)),
                                assigns);
        AVIS_SSAASSIGN (axis_avis) = assigns;

        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), assigns);

        /*
         * Then, we create 2 new partitions:
         *   ( min  <= def_withid < lmax) : def_coden;
         *   ( umin <= def_withid < max ) : def_coden;
         */
        npart = TBmakePart (def_coden, DUPdoDupTree (def_withid),
                            TBmakeGenerator (F_wl_le, F_wl_lt, TBmakeId (umin_avis),
                                             TBmakeId (max_avis), NULL, NULL));
        npart2 = TBmakePart (def_coden, DUPdoDupTree (def_withid),
                             TBmakeGenerator (F_wl_le, F_wl_lt, TBmakeId (min_avis),
                                              TBmakeId (lmax_avis), NULL, NULL));
        PART_NEXT (npart) = npart2;
        PART_NEXT (npart2) = NULL;
        res = TCappendPart (res, npart);
        CODE_USED (def_coden) += 2;
        /*
         * Finally, we advance min and max to nmin and nmax, respectively:
         */
        min_avis = nmin_avis;
        max_avis = nmax_avis;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *CompleteGrid( node *lb_avis, node *ub_avis, node *step_avis,
 *                         node *width_avis, int fdim,
 *                         node *def_withid, node *def_coden, info *arg_info)
 *
 *   @brief  creates a chain of new partitions that complement the grid
 *           specified by the first four arguments. The length of all these
 *           vector is supposed to be fdim. The withids and code are reusing
 *           those of the default partition given by the last two arguments.
 *
 ******************************************************************************/

static node *
CompleteGrid (node *lb_avis, node *ub_avis, node *step_avis, node *width_avis, int fdim,
              node *def_withid, node *def_coden, info *arg_info)
{
    node *maxwidth_avis, *axis_avis, *nlb_avis, *nwidth_avis, *nmaxwidth_avis;
    node *assigns, *npart, *res = NULL;
    int d;
    ntype *bound_type;

    DBUG_ENTER ();

    maxwidth_avis = step_avis;
    for (d = 0; d < fdim; d++) {
        /*
         * First,  we create:
         *
         * axis = d;
         * nlb, nwidth, nmaxwidth =
         *      sacprelude::gridFiller( lb, ub, width, fdim, nmaxwidth_avis);
         *
         * and insert it into INFO_PREASSIGN:
         */
        bound_type = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, fdim));
        axis_avis = CreateAvisAndInsertVardec ("axis", bound_type, arg_info);
        nlb_avis = CreateAvisAndInsertVardec ("nlb", TYcopyType (bound_type), arg_info);
        nwidth_avis
          = CreateAvisAndInsertVardec ("nwidth", TYcopyType (bound_type), arg_info);
        nmaxwidth_avis
          = CreateAvisAndInsertVardec ("nmaxwidth", TYcopyType (bound_type), arg_info);
        assigns = TBmakeAssign (
          TBmakeLet (TCcreateIdsChainFromAvises (3, nlb_avis, nwidth_avis,
                                                 nmaxwidth_avis),
                     DSdispatchFunCall (NSgetNamespace (global.preludename), "gridFiller",
                                        TCcreateExprsChainFromAvises (5, lb_avis, ub_avis,
                                                                      width_avis,
                                                                      axis_avis,
                                                                      maxwidth_avis))),
          NULL);
        AVIS_SSAASSIGN (nlb_avis) = assigns;
        AVIS_SSAASSIGN (nwidth_avis) = assigns;
        AVIS_SSAASSIGN (nmaxwidth_avis) = assigns;

        assigns = TBmakeAssign (TBmakeLet (TBmakeIds (axis_avis, NULL), TBmakeNum (d)),
                                assigns);
        AVIS_SSAASSIGN (axis_avis) = assigns;

        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), assigns);

        /*
         * Then, we create a new partitions:
         *   ( nlb  <= def_withid < ub step step width nwidth) : def_coden;
         */
        npart = TBmakePart (def_coden, DUPdoDupTree (def_withid),
                            TBmakeGenerator (F_wl_le, F_wl_lt, TBmakeId (nlb_avis),
                                             TBmakeId (ub_avis), TBmakeId (step_avis),
                                             TBmakeId (nwidth_avis)));
        res = TCappendPart (res, npart);
        CODE_USED (def_coden) += 1;
        /*
         * Finally, we advance nmaxwidth to maxwidth
         */
        maxwidth_avis = nmaxwidth_avis;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  node *CreateFullPartition( node *parts, node *withop, info *arg_info)
 *
 *   @brief  generates full partition if possible:
 *           - if withop is genarray and index vector has as many elements as
 *             dimension of resulting WL (withloop on scalars).
 *           - if withop is modarray: always (needed for compilation phase).
 *           Returns wln.
 *
 *   @param  node *wl       :  N_with node of the WL to transform
 *           info *arg_info :  is needed to access the vardecs of the current
 *                             function
 *   @return node *         :  modified N_with
 ******************************************************************************/

static node *
CreateFullPartition (node *parts, node *withop, info *arg_info)
{
    node *def_coden, *def_withid;
    node *lb_avis, *ub_avis, *min_avis, *max_avis;
    node *new_parts;
    int fdim;

    DBUG_ENTER ();

    /* pointers to parts that are to be recycled: */
    def_withid = PART_WITHID (PART_NEXT (parts));
    def_coden = PART_CODE (PART_NEXT (parts));

    /* get length of the index vector (generator) */
    fdim = SHgetExtent (TYgetShape (IDS_NTYPE (PART_VEC (parts))), 0);

    min_avis = CreateVarOfHomogeneousIntVect (fdim, 0, arg_info);
    max_avis = CreateMaxFrameShapeAvis (withop, fdim, arg_info);

    lb_avis
      = WLSflattenBound (PART_BOUND1 (parts), &FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                         &INFO_PREASSIGN (arg_info));
    ub_avis
      = WLSflattenBound (PART_BOUND2 (parts), &FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                         &INFO_PREASSIGN (arg_info));

    /* create surrounding cuboids */
    new_parts = CutSlices (min_avis, max_avis, fdim, lb_avis, ub_avis, def_withid,
                           def_coden, arg_info);

    if (PART_STEP (parts)) {
        if (PART_WIDTH (parts) == NULL) {
            PART_WIDTH (parts)
              = TBmakeId (CreateVarOfHomogeneousIntVect (fdim, 1, arg_info));
        }
        new_parts
          = TCappendPart (CompleteGrid (lb_avis, ub_avis, ID_AVIS (PART_STEP (parts)),
                                        ID_AVIS (PART_WIDTH (parts)), fdim, def_withid,
                                        def_coden, arg_info),
                          new_parts);
    }

    DBUG_RETURN (new_parts);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGmodule(node *arg_node, info *arg_info)
 *
 *   @brief first traversal of function definitions of WLPartitionGeneration
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  N_info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
WLPGmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("WLPartitionGeneration module-wise");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGfundef(node *arg_node, info *arg_info)
 *
 *   @brief starts the traversal of the given fundef.
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
WLPGfundef (node *arg_node, info *arg_info)
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
 * @fn node *WLPGassign(node *arg_node, info *arg_info)
 *
 *   @brief traverse instruction. If this creates new assignments in
 *          INFO_PREASSIGN these are inserted in front of the actual one.
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
WLPGassign (node *arg_node, info *arg_info)
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
 * @fn node *WLPGwith(node *arg_node, info *arg_info)
 *
 *   @brief  start traversal of this WL and store information in new arg_info
 *           node. The only N_part node (inclusive body) is traversed.
 *           Afterwards, if certain conditions are fulfilled,
 *           the WL is transformed into a WL with generators describing a full
 *           partition.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
WLPGwith (node *arg_node, info *arg_info)
{
    node *parts, *withop, *ub_avis, *res;
    int fdim;
    size_t num_parts;

    DBUG_ENTER ();

    /*
     * The CODEs have to be traversed as they may contain further (nested) WLs
     * and I want to modify bottom up.
     */
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    parts = WITH_PART (arg_node);

    if (TCcontainsDefaultPartition (parts)
        && TUshapeKnown (IDS_NTYPE (WITH_VEC (arg_node)))) {

        num_parts = TCcountParts (parts);
        withop = WITH_WITHOP (arg_node);

        if (num_parts == 1) {
            /*
             * The default partition is the only partition.
             */
            if (NODE_TYPE (withop) == N_genarray) {
                /*
                 * generate one big partition that covers the entire range:
                 */
                PART_GENERATOR (parts) = FREEdoFreeTree (PART_GENERATOR (parts));
                ub_avis = ID_AVIS (GENARRAY_SHAPE (withop));
                fdim = SHgetExtent (TYgetShape (IDS_NTYPE (WITH_VEC (arg_node))), 0);
                PART_GENERATOR (parts)
                  = TBmakeGenerator (F_le_VxV, F_lt_VxV,
                                     CreateVarOfHomogeneousIntVect (fdim, 0, arg_info),
                                     ub_avis, NULL, NULL);

            } else if (NODE_TYPE (withop) == N_modarray) {
                /*
                 * delete the entire with-loop and replace it by the array
                 * in the withop:
                 */
                res = MODARRAY_ARRAY (withop);
                MODARRAY_ARRAY (withop) = NULL;
                arg_node = FREEdoFreeTree (arg_node);
                arg_node = res;
            } else {
                DBUG_UNREACHABLE (
                  "default partition in non genarray/modarray WL encountered!");
            }

        } else if (num_parts == 2) {
            DBUG_ASSERT (NODE_TYPE (PART_GENERATOR (PART_NEXT (parts))) == N_default,
                         "default part expected to be the last part");

            if (!TULSisFullGenerator (PART_GENERATOR (parts), withop)) {
                /*
                 * In case it was full, there is no replacement needed for the default
                 * partition!
                 */
                WITH_PART (arg_node)
                  = TCappendPart (CreateFullPartition (parts, withop, arg_info), parts);
            }
            /*
             * delete default partition:
             */
            PART_NEXT (parts) = FREEdoFreeTree (PART_NEXT (parts));
            WITH_CODE (arg_node) = WLUTremoveUnusedCodes (WITH_CODE (arg_node));

        } else {
            DBUG_UNREACHABLE ("more than one partition alongside a default partition!");
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGdoPartitionGeneration( node *arg_node)
 *
 *   @brief  Starting point for the partition generation if it was called
 *           from main.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLPGdoWlPartitionGeneration (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_PRINT ("starting WLPGdoWlPartitionGeneration");

    arg_info = MakeInfo ();

    DSinitDeserialize (global.syntax_tree);

    TRAVpush (TR_wlpg);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DSfinishDeserialize (global.syntax_tree);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
