/** <!--********************************************************************-->
 *
 * @defgroup Insert Distributed memory transfer primitives
 *
 *
 *   This module inserts distributed type conversion primitives before and after
 *   each usage of an array. The two kinds of primitives are <dist2conc> and
 *   <conc2dist>. They are used to trasfer the data of a distributed array
 *   variable to a device(host) array variable. This is essentially
 *   compiled into host<->device memory transfers in the backend. As an
 *   example:
 *
 *   a_dist = with
 *            {
 *              ... = b_dist;
 *              ... = c_dist;
 *              ... = d_dist;
 *            }:genarray( shp);
 *
 *   is transformed into:
 *
 *   b_conc = dist2conc( b_dist);
 *   c_conc = dist2conc( c_dist);
 *   d_conc = dist2conc( d_dist);
 *   a_conc = with
 *            {
 *              ... = b_dev;
 *              ... = c_dev;
 *              ... = d_dev;
 *            }:genarray( shp);
 *   a_dist = conc2dist( a_conc);
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file insert_memtrans_dist.c
 *
 * Prefix: IMEMDIST
 *
 *****************************************************************************/
#include "insert_memtrans_dist.h"

/*
 * Other includes go here
 */
#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "shape.h"

#define DBUG_PREFIX "IMEMDIST"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "new_types.h"
#include "LookUpTable.h"
#include "math_utils.h"
#include "types.h"
#include "type_utils.h"
#include "cuda_utils.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "remove_dfms.h"
#include "infer_dfms.h"
#include "NumLookUpTable.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    bool in_wl;
    bool cudarizable;
    node *postassigns;
    node *preassigns;
    lut_t *lut;
    node *idxavis;

    node *dist_avis;
    node *let_expr;
    bool is_modarr;
    bool in_cexprs;
    bool from_ap;
    node *letids;
    node *apids;
    node *topblock;
    nlut_t *at_nlut;
};

/*
 * INFO_FUNDEF        N_fundef node of the enclosing function
 *
 * INFO_INWL          Flag indicating whether the code currently being
 *                    traversed is in a N_with
 *
 * INFO_CUDARIZABLE   Flag indicating whether the code currently being
 *                    traversed is in a cudarizable N_with
 *
 * INFO_POSTASSIGNS   Chain of <conc2dist> that needs to be appended
 *                    at the end of the current N_assign
 *
 * INFO_PREASSIGNS    Chain of <dist2conc> that needs to be prepended
 *                    at the beginning of the current N_assign
 *
 * INFO_LUT           Lookup table storing pairs of Avis(host)->Avis(device)
 *                    e.g. Given a_dev = dist2conc( a_dist),
 *                    Avis(a_dist)->Avis(a_dev) will be stored into the table
 *
 * INFO_IDXAVIS       Avis of the index vector
 *
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INWL(n) (n->in_wl)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LUT(n) (n->lut)
#define INFO_IDXAVIS(n) (n->idxavis)

#define INFO_DISTAVIS(n) (n->dist_avis)
#define INFO_LETEXPR(n) (n->let_expr)
#define INFO_IS_MODARR(n) (n->is_modarr)
#define INFO_IN_CEXPRS(n) (n->in_cexprs)
#define INFO_FROM_AP(n) (n->from_ap)
#define INFO_LETIDS(n) (n->letids)
#define INFO_APIDS(n) (n->apids)
#define INFO_TOPBLOCK(n) (n->topblock)
#define INFO_AT_NLUT(n) (n->at_nlut)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INWL (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_IDXAVIS (result) = NULL;

    INFO_DISTAVIS (result) = NULL;
    INFO_IS_MODARR (result) = FALSE;
    INFO_IN_CEXPRS (result) = FALSE;
    INFO_FROM_AP (result) = FALSE;
    INFO_LETIDS (result) = NULL;
    INFO_APIDS (result) = NULL;
    INFO_TOPBLOCK (result) = NULL;
    INFO_AT_NLUT (result) = NULL;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

static bool CUisDistributedType (ntype *ty);
static bool CUisConcreteTypeNew (ntype *ty);
static ntype *DISTNtypeConversion (ntype *dist_type, bool to_dev_type);
static ntype *TypeConvert (ntype *dist_type, nodetype nty, info *arg_info);
static void Createdist2conc (node *id, node *host_avis, node *dev_avis, info *arg_info);
// static void Createdistcont( node *dist_avis, info *arg_info);
static node *ATravWith (node *arg_node, info *arg_info);
static node *ATravId (node *arg_node, info *arg_info);
static node *ATravGenarray (node *arg_node, info *arg_info);

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTdoInsertWithloopMemtran( node *syntax_tree)
 *
 *****************************************************************************/
node *
IMEMDISTdoInsertMemtranDist (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_imemdist);
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
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node* CUisDistributedType( ntype* ty)
 *
 * @brief Returns whether ty is a distributed type or not
 *
 *****************************************************************************/
static bool
CUisDistributedType (ntype *ty)
{
    bool res;
    simpletype conc_type;

    DBUG_ENTER ();
    conc_type = TYgetSimpleType (TYgetScalar (ty));

    res = (conc_type == T_float_dist || conc_type == T_int_dist
           || conc_type == T_double_dist);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* CUisConcreteTypeNew( ntype* ty)
 *
 * @brief Returns whether ty is a concrete type or not
 *
 *****************************************************************************/
static bool
CUisConcreteTypeNew (ntype *ty)
{
    bool res;
    simpletype conc_type;

    DBUG_ENTER ();
    conc_type = TYgetSimpleType (TYgetScalar (ty));

    res = (conc_type == T_float_dev || conc_type == T_int_dev || conc_type == T_double_dev
           || conc_type == T_float || conc_type == T_int || conc_type == T_double);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* DISTNtypeConversion( ntype *dist_type, bool to_dev_type)
 *
 * @brief Returns the host or device type corresponding to type dist_type,
 *        according to argument to_dev_type.
 *
 *****************************************************************************/
static ntype *
DISTNtypeConversion (ntype *dist_type, bool to_dev_type)
{
    ntype *scalar_type, *conc_type = NULL;
    simpletype scalar_simple_type, res;

    DBUG_ENTER ();
    /* If the scalar type is simple, e.g. int, float ... */
    if (TYisSimple (TYgetScalar (dist_type)) && !CUisShmemTypeNew (dist_type)) {
        conc_type = TYcopyType (dist_type);
        scalar_type = TYgetScalar (conc_type);
        scalar_simple_type = TYgetSimpleType (scalar_type);
        /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
        if (to_dev_type) {
            switch (scalar_simple_type) {
            case T_int_dist:
                res = T_int_dev;
                break;
            case T_float_dist:
                res = T_float_dev;
                break;
            case T_double_dist:
                res = T_double_dev;
                break;
            default:
                DBUG_ASSERT (0,
                             "Simple type conversion found undefined dist simple type!");
            }
        } else {
            switch (scalar_simple_type) {
            case T_int_dist:
                res = T_int;
                break;
            case T_float_dist:
                res = T_float;
                break;
            case T_double_dist:
                res = T_double;
                break;
            default:
                res = scalar_simple_type;
            }
        }
        /* Set the device simple type */
        scalar_type = TYsetSimpleType (scalar_type, res);
    }

    DBUG_RETURN (conc_type);
}

/** <!--********************************************************************-->
 *
 * @fn node* TypeConvert( ntype *dist_type, nodetype nty, info *arg_info)
 *
 * @brief Returns a concrete type corresponding to the dist_type. When applied
 * in N_id, converts only AKD/AKS non-scalar arrays. Applied on N_ids, converts
 * only if the RHS is a with-loop or a primitive function.
 *
 *****************************************************************************/
static ntype *
TypeConvert (ntype *dist_type, nodetype nty, info *arg_info)
{
    ntype *conc_type = NULL;
    nodetype letexpr_ntype;

    DBUG_ENTER ();

    if (nty == N_id) {
        /* If the N_id is of known dimension and is not a scalar */
        if (TUdimKnown (dist_type) && TYgetDim (dist_type) > 0) {
            conc_type = DISTNtypeConversion (dist_type, INFO_CUDARIZABLE (arg_info));
        }
    }
    /*
     * LHS of a let
     */
    else if (nty == N_ids) {
        letexpr_ntype = NODE_TYPE (INFO_LETEXPR (arg_info));
        /* We convert the LHS if the RHS of the let is a withloop or a primitive
         * function
         */
        if ((letexpr_ntype == N_with || letexpr_ntype == N_prf)) {
            conc_type = DISTNtypeConversion (dist_type, INFO_CUDARIZABLE (arg_info));
        }
    } else {
        DBUG_ASSERT (0, "Neither N_id nor N_ids found in TypeConvert!");
    }

    DBUG_RETURN (conc_type);
}

/** <!--********************************************************************-->
 *
 * @fn node* Createdist2conc( node *id, node *dist_avis,
 *                           node *conc_avis, info *arg_info)
 *
 * @brief Create distributed to concrete convertion primitive function
 *
 *****************************************************************************/
static void
Createdist2conc (node *id, node *dist_avis, node *conc_avis, info *arg_info)
{

    DBUG_ENTER ();

    ID_AVIS (id) = conc_avis;
    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (conc_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    INFO_PREASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (conc_avis, NULL),
                                 TBmakePrf (F_dist2conc,
                                            TBmakeExprs (TBmakeId (dist_avis), NULL))),
                      INFO_PREASSIGNS (arg_info));

    /* Maintain SSA property */
    AVIS_SSAASSIGN (conc_avis) = INFO_PREASSIGNS (arg_info);

    /* Insert pair dist_avis->conc_avis into lookup table. */
    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), dist_avis, conc_avis);
    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node* Createdistcont( node *dist_avis, info *arg_info)
 *
 * @brief Create distributed continuous blocks primitive function
 *
 ***************************************************************************** /
static void Createdistcont( node *dist_avis, info *arg_info)
{
  node *cont_avis;

  DBUG_ENTER ();

  cont_avis = TBmakeAvis(TRAVtmpVarName("cont_block"),
                         TYmakeAKS(TYmakeSimpleType(T_int), SHmakeShape( 0)));

  FUNDEF_VARDECS( INFO_FUNDEF( arg_info)) =
  TBmakeVardec( cont_avis,  FUNDEF_VARDECS( INFO_FUNDEF( arg_info)));

  INFO_PREASSIGNS( arg_info) =
  TBmakeAssign( TBmakeLet( TBmakeIds( cont_avis, NULL),
                          TBmakePrf( F_dist_cont_block,
                                    TBmakeExprs( TBmakeId( dist_avis),
                                                NULL))),
               INFO_PREASSIGNS( arg_info));

  / * Maintain SSA property * /
  AVIS_SSAASSIGN( cont_avis) = INFO_PREASSIGNS( arg_info);

  DBUG_RETURN ();
}*/

/** <!--********************************************************************-->
 *
 * Anonymous traversal functions
 *
 *****************************************************************************/

static node *
ATravWith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ATravId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    NLUTincNum (INFO_AT_NLUT (arg_info), ID_AVIS (arg_node), 1);

    DBUG_RETURN (arg_node);
}

static node *
ATravGenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (GENARRAY_DEFAULT (arg_node) != NULL
        && NLUTgetNum (INFO_AT_NLUT (arg_info), ID_AVIS (GENARRAY_DEFAULT (arg_node)))
             == 0) {
        DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFAULT (arg_node)),
                     "Default element of genarray is not N_id!");
        GENARRAY_DEFAULT (arg_node) = FREEdoFreeNode (GENARRAY_DEFAULT (arg_node));
        GENARRAY_DEFAULT (arg_node) = NULL;
    }

    GENARRAY_RC (arg_node) = TRAVopt (GENARRAY_RC (arg_node), arg_info);
    GENARRAY_PRC (arg_node) = TRAVopt (GENARRAY_PRC (arg_node), arg_info);
    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
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
 * @fn node *IMEMDISTfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMDISTfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;
    node *old_topblock;

    DBUG_ENTER ();

    /* During the main traversal, we only look at non-lac functions */
    if (!FUNDEF_ISLACFUN (arg_node) && !FUNDEF_ISSTICKY (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else if (INFO_FROM_AP (arg_info)) {
        old_fundef = INFO_FUNDEF (arg_info);
        old_topblock = INFO_TOPBLOCK (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;
        INFO_DISTAVIS (arg_info) = NULL;
        /* Traversal of lac functions are initiated from the calling site */
        INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = old_fundef;
        INFO_TOPBLOCK (arg_info) = old_topblock;
    } else {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMDISTap (node *arg_node, info *arg_info)
{
    bool traverse_lac_fun, old_from_ap;
    node *ap_args, *fundef_args;
    node *avis, *id_avis, *new_avis, *dup_avis;
    ntype *conc_type;
    node *fundef, *old_apids;
    const char *tmpVarName;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);

    /* For us to traverse a function from calling site, it must be a
     * condictional function or a loop function and must not be the
     * recursive function call in the loop function. */
    traverse_lac_fun = (FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info));

    if (traverse_lac_fun) {
        old_from_ap = INFO_FROM_AP (arg_info);
        INFO_FROM_AP (arg_info) = TRUE;

        old_apids = INFO_APIDS (arg_info);
        INFO_APIDS (arg_info) = INFO_LETIDS (arg_info);

        if (!INFO_INWL (arg_info)) {
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            /* we have allocated a distributed variable in the conditional, we have to
             pass it as a parameter now */
            if (INFO_DISTAVIS (arg_info) != NULL) {
                /* create new local variable and add to vardecs */
                INFO_DISTAVIS (arg_info)
                  = TBmakeAvis (TRAVtmpVarName ("dist"),
                                TYcopyType (AVIS_TYPE (INFO_DISTAVIS (arg_info))));
                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                                    TBmakeVardec (INFO_DISTAVIS (arg_info), NULL));

                /* add new local variable as argument to ap */
                AP_ARGS (arg_node)
                  = TCappendExprs (AP_ARGS (arg_node),
                                   TBmakeExprs (TBmakeId (INFO_DISTAVIS (arg_info)),
                                                NULL));
            }
        } else {
            ap_args = AP_ARGS (arg_node);
            fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));

            while (ap_args != NULL) {
                DBUG_ASSERT (fundef_args != NULL, "# of Ap args != # of Fundef args!");

                DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (ap_args)) == N_id,
                             "N_ap argument is not N_id node!");

                id_avis = ID_AVIS (EXPRS_EXPR (ap_args));
                avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);

                /* If the avis has NOT been come across before */
                if (avis == id_avis) {
                    DBUG_PRINT ("fundef %s, id %s\n", FUNDEF_NAME (AP_FUNDEF (arg_node)),
                                AVIS_NAME (avis));
                    DBUG_PRINT ("There will be transfer for %s\n", AVIS_NAME (id_avis));
                    conc_type = TypeConvert (AVIS_TYPE (id_avis), N_id, arg_info);

                    if (conc_type != NULL) {
                        tmpVarName = INFO_CUDARIZABLE (arg_info) ? "dev" : "host";
                        new_avis = TBmakeAvis (TRAVtmpVarName (tmpVarName), conc_type);
                        Createdist2conc (EXPRS_EXPR (ap_args), id_avis, new_avis,
                                         arg_info);

                        dup_avis = DUPdoDupNode (new_avis);
                        AVIS_SSAASSIGN (dup_avis) = NULL;

                        INFO_LUT (arg_info)
                          = LUTinsertIntoLutP (INFO_LUT (arg_info),
                                               ARG_AVIS (fundef_args), dup_avis);
                        ARG_AVIS (fundef_args) = dup_avis;
                        AVIS_DECL (dup_avis) = fundef_args;
                    }
                } else {
                    /* If the N_avis has been come across before, replace its
                     * N_avis by the device N_avis */
                    ID_AVIS (EXPRS_EXPR (ap_args)) = avis;
                    dup_avis = DUPdoDupNode (avis);
                    AVIS_SSAASSIGN (dup_avis) = NULL;

                    /* Insert the pair of N_avis(fun arg)->N_avis(device variable)
                     * into the lookup table, so that when we later traverse the
                     * body of the fundef, old reference to the arg will be replaced
                     * by the new device varaible.  */
                    INFO_LUT (arg_info)
                      = LUTinsertIntoLutP (INFO_LUT (arg_info), ARG_AVIS (fundef_args),
                                           dup_avis);

                    /* Change N_avis of the fun arg to the device variable */
                    ARG_AVIS (fundef_args) = dup_avis;
                    AVIS_DECL (dup_avis) = fundef_args;
                }

                /* make sure the based type of the ap id and the fundef arg is the same
                 */
                if (TYgetSimpleType (TYgetScalar (ID_NTYPE (EXPRS_EXPR (ap_args))))
                    != TYgetSimpleType (
                         TYgetScalar (AVIS_TYPE (ARG_AVIS (fundef_args))))) {
                    TYsetSimpleType (TYgetScalar (AVIS_TYPE (ARG_AVIS (fundef_args))),
                                     TYgetSimpleType (
                                       TYgetScalar (ID_NTYPE (EXPRS_EXPR (ap_args)))));
                }

                ap_args = EXPRS_NEXT (ap_args);
                fundef_args = ARG_NEXT (fundef_args);
            } // wnd of while loop

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        }

        INFO_FROM_AP (arg_info) = old_from_ap;
        INFO_APIDS (arg_info) = old_apids;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTassign( node *arg_node, info *arg_info)
 *
 * @brief  Add newly created <dist2conc> and <conc2dist> to
 *         the assign chain.
 *
 *****************************************************************************/
node *
IMEMDISTassign (node *arg_node, info *arg_info)
{
    node *next;
    static int counter;

    DBUG_ENTER ();

    /*
     * Here we have to do a top-down traversal for the following reason:
     * We need to check whether there is any array variables being defined
     * in a N_with. If there is, we don't want to create a
     * dist2conc when we later come across it in the same block of code.
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* If we are no longer in a N_with, we insert
     * data transfer primitives into the AST */
    if (!INFO_INWL (arg_info)) {
        next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        if (INFO_POSTASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (arg_node, INFO_POSTASSIGNS (arg_info));
            INFO_POSTASSIGNS (arg_info) = NULL;
        }

        if (INFO_PREASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
            INFO_PREASSIGNS (arg_info) = NULL;
        }

        /* add distributed variable allocations before the conditional. */
        if (INFO_DISTAVIS (arg_info) != NULL
            && NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let
            && NODE_TYPE (ASSIGN_RHS (arg_node)) == N_ap) {
            DBUG_ASSERT (FUNDEF_ISCONDFUN (AP_FUNDEF (ASSIGN_RHS (arg_node))),
                         "We have not placed the distributed variable allocation, but "
                         "found ap to non-condfun!");

            /* add distributed variable allocation */
            arg_node = TCappendAssign (
              TBmakeAssign (TBmakeLet (TBmakeIds (INFO_DISTAVIS (arg_info), NULL),
                                       TBmakePrf (F_dist_alloc,
                                                  TBmakeExprs (TBmakeNum (counter++),
                                                               NULL))),
                            NULL),
              arg_node);
            /* Maintain SSA property */
            AVIS_SSAASSIGN (INFO_DISTAVIS (arg_info)) = arg_node;
            INFO_DISTAVIS (arg_info) = NULL;
        }

        node *last_assign = arg_node;
        while (ASSIGN_NEXT (last_assign) != NULL) {
            last_assign = ASSIGN_NEXT (last_assign);
        }

        ASSIGN_NEXT (last_assign) = next;
        ASSIGN_NEXT (last_assign) = TRAVopt (ASSIGN_NEXT (last_assign), arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMDISTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LETEXPR (arg_info) = LET_EXPR (arg_node);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTfuncond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMDISTfuncond (node *arg_node, info *arg_info)
{
    node *then_id, *else_id;
    node *ids, *apids;
    ntype *then_sclty, *else_sclty, *ids_sclty;
    node *ret_st, *ret_exprs, *fundef_ret;
    const char *tmpVarName;

    DBUG_ENTER ();

    if (INFO_INWL (arg_info)) {
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);

        then_id = FUNCOND_THEN (arg_node);
        else_id = FUNCOND_ELSE (arg_node);
        ids = INFO_LETIDS (arg_info);

        if (TYisArray (IDS_NTYPE (ids))) {
            then_sclty = TYgetScalar (ID_NTYPE (then_id));
            else_sclty = TYgetScalar (ID_NTYPE (else_id));
            ids_sclty = TYgetScalar (IDS_NTYPE (ids));

            if (TYgetSimpleType (then_sclty) != TYgetSimpleType (else_sclty)) {
                apids = INFO_APIDS (arg_info);
                tmpVarName = INFO_CUDARIZABLE (arg_info) ? "dev" : "host";

                if (CUisConcreteTypeNew (ID_NTYPE (then_id))
                    && !CUisConcreteTypeNew (ID_NTYPE (else_id))) {
                    TYsetSimpleType (else_sclty, TYgetSimpleType (then_sclty));
                    AVIS_ISCUDALOCAL (ID_AVIS (else_id)) = TRUE;
                    ID_NAME (else_id) = MEMfree (ID_NAME (else_id));
                    ID_NAME (else_id) = TRAVtmpVarName (tmpVarName);
                    TYsetSimpleType (ids_sclty, TYgetSimpleType (then_sclty));
                    IDS_NAME (ids) = MEMfree (IDS_NAME (ids));
                    IDS_NAME (ids) = TRAVtmpVarName (tmpVarName);
                } else if (CUisConcreteTypeNew (ID_NTYPE (else_id))
                           && !CUisConcreteTypeNew (ID_NTYPE (then_id))) {
                    TYsetSimpleType (then_sclty, TYgetSimpleType (else_sclty));
                    AVIS_ISCUDALOCAL (ID_AVIS (then_id)) = TRUE;
                    ID_NAME (then_id) = MEMfree (ID_NAME (then_id));
                    ID_NAME (then_id) = TRAVtmpVarName (tmpVarName);
                    TYsetSimpleType (ids_sclty, TYgetSimpleType (else_sclty));
                    IDS_NAME (ids) = MEMfree (IDS_NAME (ids));
                    IDS_NAME (ids) = TRAVtmpVarName (tmpVarName);
                } else {
                    // .... TODO ...
                    DBUG_ASSERT (0, "Found arrays of unequal types while not one"
                                    " distributed type and one concrete type!");
                }

                AVIS_ISCUDALOCAL (IDS_AVIS (ids)) = TRUE;

                ret_st = FUNDEF_RETURN (INFO_FUNDEF (arg_info));
                DBUG_ASSERT (ret_st != NULL, "N_return is null for lac fun!");
                ret_exprs = RETURN_EXPRS (ret_st);
                fundef_ret = FUNDEF_RETS (INFO_FUNDEF (arg_info));

                while (ret_exprs != NULL && fundef_ret != NULL && apids != NULL) {
                    if (ID_AVIS (EXPRS_EXPR (ret_exprs)) == IDS_AVIS (ids)) {
                        TYsetSimpleType (TYgetScalar (RET_TYPE (fundef_ret)),
                                         TYgetSimpleType (ids_sclty));
                        TYsetSimpleType (TYgetScalar (IDS_NTYPE (apids)),
                                         TYgetSimpleType (ids_sclty));
                        AVIS_ISCUDALOCAL (IDS_AVIS (apids)) = TRUE;
                        IDS_NAME (apids) = MEMfree (IDS_NAME (apids));
                        IDS_NAME (apids) = TRAVtmpVarName (tmpVarName);
                    }
                    ret_exprs = EXPRS_NEXT (ret_exprs);
                    fundef_ret = RET_NEXT (fundef_ret);
                    apids = IDS_NEXT (apids);
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTwith( node *arg_node, info *arg_info)
 *
 * @brief Traverse both withop and N_code of a cudarizable N_with
 *
 *****************************************************************************/
node *
IMEMDISTwith (node *arg_node, info *arg_info)
{
    info *anon_info;
    node *idxvec_avis;
    ntype *idxvec_type;

    DBUG_ENTER ();

    /* If we are not already in a withloop */
    if (!INFO_INWL (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = WITH_CUDARIZABLE (arg_node);

        /************ Anonymous Traversal ************/
        /* This anon traversal remove all default elements in genarray
         * that are not used in the withloop body at all. */
        anontrav_t atrav[4] = {{N_with, &ATravWith},
                               {N_genarray, &ATravGenarray},
                               {N_id, &ATravId},
                               {(nodetype)0, NULL}};

        TRAVpushAnonymous (atrav, &TRAVsons);

        anon_info = MakeInfo ();

        INFO_AT_NLUT (anon_info)
          = NLUTgenerateNlut (FUNDEF_ARGS (INFO_FUNDEF (arg_info)),
                              FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));
        arg_node = TRAVdo (arg_node, anon_info);

        INFO_AT_NLUT (anon_info) = NLUTremoveNlut (INFO_AT_NLUT (anon_info));

        anon_info = FreeInfo (anon_info);
        TRAVpop ();
        /*********************************************/

        INFO_LUT (arg_info) = LUTgenerateLut ();

        INFO_INWL (arg_info) = TRUE;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        /* We do not want to create a dist2conc for index vector,
         * We store pair N_id->N_empty to signal this. */
        idxvec_avis = IDS_AVIS (WITH_VEC (arg_node));
        if (WITH_CUDARIZABLE (arg_node)) {
            AVIS_ISCUDALOCAL (idxvec_avis) = TRUE;
        }
        // change the index vector back to host type
        idxvec_type = AVIS_TYPE (idxvec_avis);
        AVIS_TYPE (idxvec_avis) = DISTNtypeConversion (idxvec_type, FALSE);
        TYfreeType (idxvec_type);
        INFO_IDXAVIS (arg_info) = idxvec_avis;

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        /* Cleanup */
        INFO_INWL (arg_info) = FALSE;
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    } else {

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTcode( node *arg_node, info *arg_info)
 *
 * @brief Traverse the code block
 *
 *****************************************************************************/
node *
IMEMDISTcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    INFO_IN_CEXPRS (arg_info) = TRUE;
    CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);
    INFO_IN_CEXPRS (arg_info) = FALSE;

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTgenarray( node *arg_node, info *arg_info)
 *
 * @brief Traverse default element of a N_genarray
 *
 *****************************************************************************/
node *
IMEMDISTgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INWL (arg_info)) {
        /* Note that we do not traverse N_genarray->shape. This is
         * because it can be an N_id node and we do not want to insert
         * <dist2conc> for it in this case. Therefore, the only sons
         * of N_genarray we traverse are the default element and the
         * potential reuse candidates. */
        if (GENARRAY_DEFAULT (arg_node) != NULL) {
            DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFAULT (arg_node)) == N_id,
                         "Non N_id default element found in N_genarray!");
            GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        }

        GENARRAY_RC (arg_node) = TRAVopt (GENARRAY_RC (arg_node), arg_info);

        GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTmodarray( node *arg_node, info *arg_info)
 *
 * @brief Traverse default element of a N_modarray
 *
 *****************************************************************************/
node *
IMEMDISTmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INWL (arg_info)) {
        DBUG_ASSERT (NODE_TYPE (MODARRAY_ARRAY (arg_node)) == N_id,
                     "Non N_id modified array found in N_modarray!");
        INFO_IS_MODARR (arg_info) = TRUE;
        MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
        INFO_IS_MODARR (arg_info) = FALSE;
        MODARRAY_RC (arg_node) = TRAVopt (MODARRAY_RC (arg_node), arg_info);
        MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTids( node *arg_node, info *arg_info)
 *
 * @brief For N_ids needed to be type converted, create <conc2dist>.
 *
 *****************************************************************************/
node *
IMEMDISTids (node *arg_node, info *arg_info)
{
    node *new_conc_avis, *new_dist_avis, *ids_avis;
    ntype *ids_type, *conc_type;
    const char *tmpVarName;
    prf conc2dist;

    DBUG_ENTER ();

    ids_avis = IDS_AVIS (arg_node);
    ids_type = AVIS_TYPE (ids_avis);

    /* check if ids is of a distributed type we want to convert */
    if (CUisDistributedType (ids_type)) {
        conc_type = TypeConvert (ids_type, NODE_TYPE (arg_node), arg_info);
        if (conc_type != NULL) {
            /* we can convert this ids */

            // get a new distributed avis
            if (INFO_DISTAVIS (arg_info) != NULL) {
                /* in sequential section, re-use a distributed avis found on the RHS */
                new_dist_avis = INFO_DISTAVIS (arg_info);
            } else {
                /* create new distributed avis's */
                new_dist_avis
                  = TBmakeAvis (TRAVtmpVarName ("dist"), TYcopyType (ids_type));
                /* add argument to function */
                FUNDEF_ARGS (INFO_FUNDEF (arg_info))
                  = TCappendArgs (FUNDEF_ARGS (INFO_FUNDEF (arg_info)),
                                  TBmakeArg (new_dist_avis, NULL));

                INFO_DISTAVIS (arg_info) = new_dist_avis;
            }

            /* create new concrete avis's */
            tmpVarName = INFO_CUDARIZABLE (arg_info) ? "dev" : "host";
            new_conc_avis = TBmakeAvis (TRAVtmpVarName (tmpVarName), conc_type);

            /* change current avis*/
            IDS_AVIS (arg_node) = new_conc_avis;

            /* add new vardec */
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (TBmakeVardec (new_conc_avis, NULL),
                                FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            /* add concrete to distributed transfer */
            conc2dist = INFO_CUDARIZABLE (arg_info) ? F_device2dist : F_host2dist_st;
            INFO_POSTASSIGNS (arg_info) = TBmakeAssign (
              TBmakeLet (TBmakeIds (ids_avis, NULL),
                         TBmakePrf (conc2dist,
                                    TBmakeExprs (TBmakeId (new_conc_avis),
                                                 TBmakeExprs (TBmakeId (new_dist_avis),
                                                              NULL)))),
              INFO_POSTASSIGNS (arg_info));
            /* Maintain SSA property */
            AVIS_SSAASSIGN (new_conc_avis) = AVIS_SSAASSIGN (ids_avis);
            AVIS_SSAASSIGN (ids_avis) = INFO_POSTASSIGNS (arg_info);
        }
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTid( node *arg_node, info *arg_info)
 *
 * @brief For each distributed array N_id in the N_with, either create
 *        type conversion for it (i.e. <dist2conc>) or set its N_avis to
 *        that of an already converted device array N_id depending on whether
 *        the N_id is encountered for the first time or not.
 *
 *****************************************************************************/
node *
IMEMDISTid (node *arg_node, info *arg_info)
{
    node *new_avis, *avis, *id_avis;
    ntype *conc_type, *id_type;
    const char *tmpVarName;

    DBUG_ENTER ();

    id_avis = ID_AVIS (arg_node);
    id_type = AVIS_TYPE (id_avis);

    /* skip cuda shared memory variables and idx vector */
    if (!CUisShmemTypeNew (id_type) && (INFO_IDXAVIS (arg_info) != id_avis)) {
        conc_type = TypeConvert (id_type, NODE_TYPE (arg_node), arg_info);
        /* if we can convert */
        if (conc_type != NULL) {
            avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);
            if (avis != id_avis) {
                /* If the N_avis has been come across before, replace its
                 * N_avis by the device N_avis */
                ID_AVIS (arg_node) = avis;
            } else {
                /* Otherwise, create transfer */
                tmpVarName = INFO_CUDARIZABLE (arg_info) ? "dev" : "host";
                new_avis = TBmakeAvis (TRAVtmpVarName (tmpVarName), conc_type);
                Createdist2conc (arg_node, id_avis, new_avis, arg_info);

                /*if not in with-loop, we are in a sequential section. The source
                 distributed variable, which we'll replace, is most likely this one*/
                if (!INFO_INWL (arg_info))
                    INFO_DISTAVIS (arg_info) = id_avis;
            }
            /* create distributed continous blocks function for with-loops * /
            if( INFO_INWL(arg_info) / *&& INFO_CUDARIZABLE(arg_info)* /) {
              Createdistcont( id_avis, arg_info );
            }*/
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTreturn( node *arg_node, info *arg_info)
 *
 * @brief Skip return statements
 *
 *****************************************************************************/
node *
IMEMDISTreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
