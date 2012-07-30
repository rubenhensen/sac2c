/** <!--********************************************************************-->
 *
 * @defgroup
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file create_dist_wl_cond.c
 *
 * Prefix: DISTCOND
 *
 *****************************************************************************/
#include "create_dist_wl_cond.h"

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

#define DBUG_PREFIX "DISTCOND"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"

typedef enum { CTY_unknown = 0, CTY_host = 1, CTY_cuda = 2 } concrete_type;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *assigns;
    node *letids;
    node *predassigns;
    node *predavis;
    node *thenblock;
    prf conc2distprf;
    concrete_type conctype;
};

/*
 * INFO_FUNDEF        N_fundef node of the enclosing function
 *
 * INFO_ASSIGNS       N_assign chain to replace the distributed wl assignment
 *
 * INFO_LETIDS        N_ids of a N_let
 *
 * INFO_PREDASSIGNS   N_assign chain for the predicate of each branch of the
 *                    conditional
 *
 * INFO_PREDAVIS      N_avis for the predicate variable
 *
 * INFO_THENBLOCK     N_assign chain for one of the branches of the N_cond
 *
 * INFO_CONC2DISTPRF  The primitive function to use for the concrete to
 *                    distributed conversion
 *
 * INFO_CONCTYPE      The concrete type to use for the concrete to
 *                    distributed conversion
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ASSIGNS(n) (n->assigns)
#define INFO_LETIDS(n) (n->letids)
#define INFO_PREDASSIGNS(n) (n->predassigns)
#define INFO_PREDAVIS(n) (n->predavis)
#define INFO_THENBLOCK(n) (n->thenblock)
#define INFO_CONC2DISTPRF(n) (n->conc2distprf)
#define INFO_CONCTYPE(n) (n->conctype)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ASSIGNS (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_PREDASSIGNS (result) = NULL;
    INFO_PREDAVIS (result) = NULL;
    INFO_THENBLOCK (result) = NULL;
    INFO_CONC2DISTPRF (result) = F_unknown;
    INFO_CONCTYPE (result) = CTY_unknown;

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

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDdoCreateDistWlCond( node *syntax_tree)
 *
 *****************************************************************************/
node *
DISTCONDdoCreateDistWlCond (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_distcond);
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

static void CreatePredicateAssignments (node *expr, info *arg_info);
static void CreateConc2DistAssignments (node *with, node *withop, info *arg_info);
static node *ATravWithop (node *arg_node, info *arg_info);

/** <!--********************************************************************-->
 *
 * @fn void CreatePredicateAssignments( node *expr, info *arg_info)
 *
 * @brief Create variables and memory assignments for the predicate given by
 *        expr.
 *
 *****************************************************************************/
static void
CreatePredicateAssignments (node *expr, info *arg_info)
{
    node *alloc_avis, *pred_avis, *fill, *alloc_prf;

    DBUG_ENTER ();

    /* create predicate avises */
    alloc_avis = TBmakeAvis (TRAVtmpVarName ("pred_alloc"),
                             TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
    pred_avis = TBmakeAvis (TRAVtmpVarName ("pred"),
                            TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
    INFO_PREDAVIS (arg_info) = pred_avis;

    /* add new avises to variable declarations */
    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (alloc_avis,
                      TBmakeVardec (pred_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info))));

    /* create assignment of new predicate variable */
    fill = TBmakeAssign (TBmakeLet (TBmakeIds (pred_avis, NULL),
                                    TCmakePrf2 (F_fill, expr, TBmakeId (alloc_avis))),
                         NULL);

    /* create assignment for allocation of predicate variable */
    alloc_prf = TCmakePrf2 (F_alloc, TBmakeNum (0), TCcreateZeroVector (0, T_bool));
    INFO_PREDASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (alloc_avis, NULL), alloc_prf), fill);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CreateConc2DistAssignments(node* with, node *withop, info *arg_info)
 *
 * @brief Changes return type of withop to concrete type and creates the
 *        conversion functions.
 *
 *****************************************************************************/
static void
CreateConc2DistAssignments (node *with, node *withop, info *arg_info)
{
    node *conc2dist_chain, *letids, *old_letids;

    DBUG_ENTER ();

    /* duplicate letids, saving old version */
    old_letids = INFO_LETIDS (arg_info);
    letids = DUPdoDupTree (INFO_LETIDS (arg_info));
    /* we keep letids as the start of the chain, since all ids will be popped out
     of info in the next traversal. */
    INFO_LETIDS (arg_info) = letids;

    DBUG_ASSERT (INFO_LETIDS (arg_info) != NULL,
                 "Empty Ids Chain before withop traversal!");
    /************ Anonymous Traversal ************/
    anontrav_t atrav[4] = {{N_genarray, &ATravWithop},
                           {N_modarray, &ATravWithop},
                           {N_fold, &ATravWithop},
                           {(nodetype)0, NULL}};

    TRAVpushAnonymous (atrav, &TRAVsons);

    conc2dist_chain = TRAVdo (withop, arg_info);

    /************ Anonymous Traversal ************/
    TRAVpop ();
    /*********************************************/
    DBUG_ASSERT (INFO_LETIDS (arg_info) == NULL,
                 "Non-Empty Ids Chain after withop traversal!");

    /*
     * create the assignments for the then branch of the conditional, which
     * consist of the withloop let and the conversion functions.
     * The withloop is duplicated here because we will be freeing the assignment
     * statement containing the distributed withloops.
     */
    INFO_THENBLOCK (arg_info)
      = TBmakeAssign (TBmakeLet (letids, DUPdoDupTree (with)), conc2dist_chain);

    /* restore letids in info */
    INFO_LETIDS (arg_info) = old_letids;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *ATRAVwithop( node *arg_node, info *arg_info)
 *
 * @brief Create assignment chain of concrete to distributed primitive function
 *        calls for each withop
 *
 *****************************************************************************/
static node *
ATravWithop (node *arg_node, info *arg_info)
{
    node *ids, *res;
    const char *suffix;
    node *ids_avis, *new_avis, *prf;
    ntype *scalar_type, *dist_type, *new_type = NULL;
    simpletype scalar_simple_type, new_simple_type;
    DBUG_ENTER ();

    ids_avis = IDS_AVIS (INFO_LETIDS (arg_info));
    dist_type = AVIS_TYPE (ids_avis);
    /* check if we are generating an array*/
    if (TUdimKnown (dist_type) && TYgetDim (dist_type) > 0) {
        /* for arrays, we create a new concrete variable for the computation and
         the convertion statement afterwards. */
        new_type = TYcopyType (dist_type);
        scalar_type = TYgetScalar (new_type);
        scalar_simple_type = TYgetSimpleType (scalar_type);
        /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
        switch (INFO_CONCTYPE (arg_info)) {
        case CTY_cuda:
            suffix = "_cuda";
            switch (scalar_simple_type) {
            case T_int_dist:
                new_simple_type = T_int_dev;
                break;
            case T_float_dist:
                new_simple_type = T_float_dev;
                break;
            case T_double_dist:
                new_simple_type = T_double_dev;
                break;
            default:
                DBUG_ASSERT (0,
                             "Simple type conversion found undefined dist simple type!");
            }
            break;
        case CTY_host:
            suffix = "_host";
            switch (scalar_simple_type) {
            case T_int_dist:
                new_simple_type = T_int;
                break;
            case T_float_dist:
                new_simple_type = T_float;
                break;
            case T_double_dist:
                new_simple_type = T_double;
                break;
            default:
                DBUG_ASSERT (FALSE,
                             "Simple type conversion found undefined dist simple type!");
            }
            break;
        default:
            DBUG_ASSERT (FALSE, "Simple type conversion found unknown concrete type!");
            break;
        }
        scalar_type = TYsetSimpleType (scalar_type, new_simple_type);

        /* create new avis for concrete result of genarray */
        new_avis
          = TBmakeAvis (TRAVtmpVarName (STRcat (AVIS_NAME (ids_avis), suffix)), new_type);
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                            TBmakeVardec (new_avis, NULL));

        /* create primitive function call. Arguments are id for this node and id for
         the first node from withids. */
        prf = TCmakePrf2 (INFO_CONC2DISTPRF (arg_info), TBmakeId (ids_avis),
                          TBmakeId (new_avis));

        ids = TBmakeIds (ids_avis, NULL);
        IDS_AVIS (INFO_LETIDS (arg_info)) = new_avis;

        /* pop one ids from the chain */
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));

        res = TBmakeAssign (TBmakeLet (ids, prf),
                            TRAVopt (WITHOP_NEXT (arg_node), arg_info));
    } else {
        /* we are generating a scalar, so we skip this withop */

        /* pop one ids from the chain */
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        res = TRAVopt (WITHOP_NEXT (arg_node), arg_info);
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
 * @fn node *DISTCONDfundef( node *arg_node, info *arg_info)
 *
 * @brief Check only SPMD functions.
 *
 *****************************************************************************/
node *
DISTCONDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSPMDFUN (arg_node)) {
        DBUG_ASSERT (FUNDEF_BODY (arg_node) != NULL, "Found SPMD function with no body!");

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDassign( node *arg_node, info *arg_info)
 *
 * @brief Insert Predicate and Conditional if required.
 *
 *****************************************************************************/
node *
DISTCONDassign (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* if during the traversal we generated new assignments, we replace this node
     with those assignments */
    if (INFO_ASSIGNS (arg_info) != NULL) {
        res = TCappendAssign (INFO_ASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_ASSIGNS (arg_info) = NULL;
        FREEdoFreeNode (arg_node);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        res = arg_node;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDlet( node *arg_node, info *arg_info)
 *
 * @brief Traverse RHS first, then LHS.
 *
 *****************************************************************************/
node *
DISTCONDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDwiths( node *arg_node, info *arg_info)
 *
 * @brief Create nested conditional branch for each with-loop
 *
 *****************************************************************************/
node *
DISTCONDwiths (node *arg_node, info *arg_info)
{
    node *cond;

    DBUG_ENTER ();

    /*
     * traverse with-loop to get predicate avis and assignments for this withloop.
     */
    WITHS_WITH (arg_node) = TRAVdo (WITHS_WITH (arg_node), arg_info);

    /* create conditional with the previous withloop's assignments in the else
     * branch*/
    cond = TBmakeCond (TBmakeId (INFO_PREDAVIS (arg_info)),
                       TBmakeBlock (INFO_THENBLOCK (arg_info), NULL),
                       TBmakeBlock (INFO_ASSIGNS (arg_info), NULL));
    /* the assignments for this with-loop are the predicate assignments and the
     conditional*/
    INFO_ASSIGNS (arg_info)
      = TCappendAssign (INFO_PREDASSIGNS (arg_info), TBmakeAssign (cond, NULL));

    WITHS_NEXT (arg_node) = TRAVopt (WITHS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDwith( node *arg_node, info *arg_info)
 *
 * @brief Create CUDA predicate and concrete to distributed function calls
 *
 *****************************************************************************/
node *
DISTCONDwith (node *arg_node, info *arg_info)
{
    static int counter = 0;
    node *new_rhs;

    DBUG_ENTER ();

    /* create predicate value */
    new_rhs = TBmakePrf (F_is_cuda_thread, TBmakeExprs (TBmakeNum (counter++), NULL));

    CreatePredicateAssignments (new_rhs, arg_info);

    INFO_CONC2DISTPRF (arg_info) = F_device2dist;
    INFO_CONCTYPE (arg_info) = CTY_cuda;

    CreateConc2DistAssignments (arg_node, WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDwith2( node *arg_node, info *arg_info)
 *
 * @brief Create Host predicate and concrete to distributed function calls
 *
 *****************************************************************************/
node *
DISTCONDwith2 (node *arg_node, info *arg_info)
{
    node *new_rhs;

    DBUG_ENTER ();

    /* create predicate value */
    new_rhs = TBmakeBool (TRUE);

    CreatePredicateAssignments (new_rhs, arg_info);

    INFO_CONC2DISTPRF (arg_info) = F_host2dist_spmd;
    INFO_CONCTYPE (arg_info) = CTY_host;

    CreateConc2DistAssignments (arg_node, WITH2_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
