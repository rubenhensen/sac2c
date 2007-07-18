#include "specialization_oracle_static_shape_knowledge.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "shape.h"
#include "prf_pvs_info.h"

/**
 *
 * @specialistation_oracle_static_shape_knowledge.c
 *
 * @brief
 *
 */

/**
 * INFO structure
 */

struct INFO {
    int iteration_round;
    node *args;
    node *ids;
    bool estimation;
    bool copy_demand;
    bool num_args_eq_num_ids;
};

/**
 * INFO macros
 */

#define INFO_ITERATION_ROUND(n) (n->iteration_round)
#define INFO_ARGS(n) (n->args)
#define INFO_IDS(n) (n->ids)
#define INFO_ESTIMATION(n) (n->estimation)
#define INFO_COPY_DEMAND(n) (n->copy_demand)
#define INFO_NUM_ARGS_EQ_NUM_IDS(n) (n->num_args_eq_num_ids)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ITERATION_ROUND (result) = 0;
    INFO_ARGS (result) = NULL;
    INFO_IDS (result) = NULL;
    INFO_ESTIMATION (result) = FALSE;
    INFO_COPY_DEMAND (result) = FALSE;
    INFO_NUM_ARGS_EQ_NUM_IDS (result) = TRUE;

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
 * Static global variables
 */

static const shape_oracle_funptr prf_shape_oracle_funtab[] = {
#define PRFshape_oracle_fun(shape_oracle_fun) shape_oracle_fun
#include "prf_info.mac"
};

/**
 * HELPER functions
 */

/** <!-- ****************************************************************** -->
 *
 * @fn static int *makeElems(int num_elems, int *elem_array)
 *
 *    @brief This function converts a local array into a global array. This
 *           functionality is required because the function to produce a
 *           constant just inserts the given array without copying it.
 *
 *    @param num_elems Integer which indicates the number of elements
 *    @param elem_array indicates the array itself
 *
 *    @return int *elems global integer array
 *****************************************************************************/

static int *
makeElems (int num_elems, int *elem_array)
{
    DBUG_ENTER ("makeElems");

    int *elems = NULL;
    int i = 0;

    elems = (int *)MEMmalloc (num_elems * sizeof (int));

    for (i = 0; i < num_elems; i++) {
        elems[i] = elem_array[i];
    }

    DBUG_RETURN (elems);
}

/*
static
void modifyDemandMatrixIdx(constant *demand, int x, int y, int new_value)
{
  DBUG_ENTER("modifyDemandMatrixIdx");

  constant *new_elem = COmakeConstantFromInt(new_value);
  shape *idx_shape = SHcreateShape(2, x, y);
  constant *idx_elem = COmakeConstantFromShape(idx_shape);
  idx_shape = SHfreeShape(idx_shape);

  constant *new_demand = COmodarray(demand, idx_elem, new_elem);
  demand = COfreeConstant(demand);
  demand = new_demand;
}

static
void modifyDemandMatrix(constant *demand, int x, int row, int *new_demand)
{
  DBUG_ENTER("modifyDemandMatrix");

  shape *demand_shape = COgetShape(demand);
  int length = SHgetExtent(demand_shape, 1);

  int i = 0;

  for(i = 0; i<length; i++) {
    modifyDemandMatrixIdx(demand, x, i, new_demand[i]);
  }
}
*/

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKap(node *arg_node, info *arg_info)
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
SOSSKap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKap");

    INFO_ARGS (arg_info) = AP_ARGS (arg_node);
    AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKarg(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
SOSSKarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKarg");

    if (INFO_IDS (arg_info) != NULL) {
        if (AVIS_DEMAND (ARG_AVIS (arg_node)) != NULL) {

            AVIS_DEMAND (IDS_AVIS (INFO_IDS (arg_info)))
              = COcopyConstant (AVIS_DEMAND (ARG_AVIS (arg_node)));
        } else {
            AVIS_DEMAND (IDS_AVIS (INFO_IDS (arg_info)))
              = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 1, 2, 3);
        }

        if (ARG_NEXT (arg_node) != NULL) {
            INFO_IDS (arg_info) = IDS_NEXT (INFO_IDS (arg_info));
            ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
        } else if (IDS_NEXT (INFO_IDS (arg_info)) != NULL) {
            INFO_NUM_ARGS_EQ_NUM_IDS (arg_info) = FALSE;
        }
    } else {
        INFO_NUM_ARGS_EQ_NUM_IDS (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKconst(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node N_const node
 *    @param arg_info INFO structure
 *
 *    @return N_const node
 *****************************************************************************/

node *
SOSSKconst (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKconst");
    /*
     * Is this function needed? If const returns empty set, its shouldnt be needed...
     */
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKid(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node N_id node
 *    @param arg_info INFO structure
 *
 *    @return N_id node
 *******************************************************************************/

node *
SOSSKid (node *arg_node, info *arg_info)
{

    /*constant *cst = NULL;
     */
    DBUG_ENTER ("SOSSKid");
    /*  DBUG_PRINT("SOSSK",("Enter SOSSKid..."));

      DBUG_PRINT("SOSSK",("Make Constants..."));
      AVIS_DEMAND(ID_AVIS(arg_node)) = COmakeConstantFromDynamicArguments(T_int, 3, 3,
         3, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 );
    */
    /*  cst = COmakeConstant(T_int, sp, elems);
      DBUG_PRINT("SOSSK",("piep %s", COconstant2String(AVIS_DEMAND(ID_AVIS(arg_node)))));
    */
    /*  cst = COmakeConstantFromDynamicArguments(T_int, 3, 3, 3, 0);
      char *cr = COconstant2String(AVIS_DEMAND(ID_AVIS(arg_node)));
      DBUG_PRINT("SOSSK",("Array: %s",cr));
      cr = COconstant2String(cst);
      DBUG_PRINT("SOSSK",("Idx: %s", cr));
      DBUG_PRINT("SOSSK",("Do OverSel..."));
      constant *result = COoverSel(cst, AVIS_DEMAND(ID_AVIS(arg_node)));
      cr = COconstant2String(result);
      DBUG_PRINT("SOSSK",("Result: %s",cr));
      arg_node = TRAVcont(arg_node, arg_info);

      COfreeConstant(result);
      COfreeConstant(cst);
      DBUG_PRINT("SOSSK",("Exit SOSSKid..."));
    */
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKids(node *arg_node, info *arg_info)
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
SOSSKids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKids");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKprf(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node N_prf node
 *    @param arg_info INFO structure
 *
 *    @return N_prf node
 *******************************************************************************/

node *
SOSSKprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKprf");

    prf prf;

    char *pv_char = NULL;
    constant *pv_constant = NULL;

    prf = PRF_PRF (arg_node);

    DBUG_PRINT ("SOSSK", ("PRF: %s", PRF_NAME (prf)));

    if (prf_shape_oracle_funtab[prf] != NULL) {
        int i = 0;
        for (i = 0; i <= 2; i++) {
            pv_constant = prf_shape_oracle_funtab[prf](i);

            if (pv_constant != NULL) {
                pv_char = COconstant2String (pv_constant);

                DBUG_PRINT ("SOSSK", ("%s", pv_char));
            }
        }
    } else {
        DBUG_PRINT ("SOSSK", ("Points to NULL!"));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKfundef(node *arg_node, info arg_info)
 *
 *    @brief
 *
 *    @param arg_node N_fundef node
 *    @param arg_info INFO structure
 *
 *    @return N_fundef node
 *******************************************************************************/

node *
SOSSKfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKfundef");

    info *old_info = arg_info;
    arg_info = MakeInfo ();

    INFO_IDS (arg_info) = INFO_IDS (old_info);
    INFO_ARGS (arg_info) = INFO_ARGS (old_info);

    DBUG_PRINT ("SOSSK", ("Enter Function: %s", FUNDEF_NAME (arg_node)));

    if ((FUNDEF_FIXPOINTFOUND (arg_node) == TRUE)
        || (FUNDEF_LASTITERATIONROUND (arg_node) == INFO_ITERATION_ROUND (arg_info))) {

        INFO_COPY_DEMAND (arg_info) = TRUE;

        if (FUNDEF_LASTITERATIONROUND (arg_node) == INFO_ITERATION_ROUND (arg_info)) {
            INFO_ESTIMATION (arg_info) = TRUE;
        }

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        } else if (INFO_IDS (arg_info) != NULL) {
            INFO_NUM_ARGS_EQ_NUM_IDS (arg_info) = TRUE;
        }

        DBUG_ASSERT (INFO_NUM_ARGS_EQ_NUM_IDS (arg_info) == TRUE,
                     "#args != #ids in SOSSKfundef!");
    } else {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_IDS (arg_info) = NULL;
    INFO_ARGS (arg_info) = NULL;

    arg_info = FreeInfo (arg_info);
    arg_info = old_info;
    old_info = NULL;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKlet(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node N_let node
 *    @param arg_info INFO structure
 *
 *    @return N_let node
 *******************************************************************************/

node *
SOSSKlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKlet");

    INFO_IDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKwith(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node N_with node
 *    @param arg_info INFO structure
 *
 *    @return N_with node
 *******************************************************************************/

node *
SOSSKwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKwith");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKdoSpecializationOracleSSK(node *syntax_tree)
 *
 *    @brief
 *
 *    @param syntax_tree N_module node
 *
 *    @return transformed syntax tree
 *******************************************************************************/
node *
SOSSKdoSpecializationOracleSSK (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SOSSKdoSpecializationOracleSSK");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "SOSSKdoSpecializationOracleSSK is intended to run on the entire tree");

    PPIinitializePVs ();

    info = MakeInfo ();
    TRAVpush (TR_sossk);

    INFO_ITERATION_ROUND (info) = INFO_ITERATION_ROUND (info) + 1;

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
