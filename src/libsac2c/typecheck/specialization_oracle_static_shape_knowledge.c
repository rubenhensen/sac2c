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
    int num_rets;
    int num_args;
    int pos_prf_arg;
    node *args;
    node *ids;
    constant *demand;
    bool estimation;
    bool copy_demand;
    bool num_args_eq_num_args;
    bool demand_has_changed;
    bool ap_found;
    bool ap_call;
    prf prf_name;
};

/**
 * INFO macros
 */

#define INFO_ITERATION_ROUND(n) (n->iteration_round)
#define INFO_NUM_RETS(n) (n->num_rets)
#define INFO_NUM_ARGS(n) (n->num_args)
#define INFO_POS_PRF_ARG(n) (n->pos_prf_arg)
#define INFO_ARGS(n) (n->args)
#define INFO_IDS(n) (n->ids)
#define INFO_DEMAND(n) (n->demand)
#define INFO_ESTIMATION(n) (n->estimation)
#define INFO_COPY_DEMAND(n) (n->copy_demand)
#define INFO_NUM_ARGS_EQ_NUM_ARGS(n) (n->num_args_eq_num_args)
#define INFO_DEMAND_HAS_CHANGED(n) (n->demand_has_changed)
#define INFO_AP_FOUND(n) (n->ap_found)
#define INFO_AP_CALL(n) (n->ap_call)
#define INFO_PRF_NAME(n) (n->prf_name)

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
    INFO_NUM_RETS (result) = 0;
    INFO_NUM_ARGS (result) = 0;
    INFO_POS_PRF_ARG (result) = -1;
    INFO_ARGS (result) = NULL;
    INFO_IDS (result) = NULL;
    INFO_DEMAND (result)
      = COmakeConstantFromDynamicArguments (T_int, 2, 1, 2, 0, 0, 0, 0);
    INFO_ESTIMATION (result) = FALSE;
    INFO_COPY_DEMAND (result) = FALSE;
    INFO_NUM_ARGS_EQ_NUM_ARGS (result) = TRUE;
    INFO_DEMAND_HAS_CHANGED (result) = FALSE;
    INFO_AP_FOUND (result) = FALSE;
    INFO_AP_CALL (result) = FALSE;
    INFO_PRF_NAME (result) = 0;

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
 * @fn constant *computeDemand(node *ids, node *ap_arg, node *fundef_arg,
 *                             int num_rets)
 *
 *    @brief this function computes the demand of an fundef.
 *
 *    @param ids ids of the let which was leading to this fundef
 *    @param fundef_arg args of the fundef
 *    @param num_rets number of returnvalues of the fundef
 *
 *    @return the computed demand of the given argument
 ******************************************************************************/

static constant *
computeDemand (node *ids, node *fundef_arg, int num_rets)
{
    DBUG_ENTER ("computeDemand");
    DBUG_PRINT ("SOSSK", (">>> ENTER computeDemand"));

    int i = 0;
    int j = 0;
    int pos = 0;
    node *current_ids = ids;

    constant *new_demand = NULL;

    shape *shape_extent = NULL;
    shape *oversel_shape = NULL;
    constant *current_ids_demand = NULL;
    constant *oversel_shape_constant = NULL;
    constant *reshaped_ids_demand = NULL;
    constant *current_fundef_arg_demand = NULL;
    constant *selection_matrix = NULL;

    char *s;

    /* get demands*/
    if (AVIS_DEMAND (ARG_AVIS (fundef_arg)) != NULL) {
        current_fundef_arg_demand = AVIS_DEMAND (ARG_AVIS (fundef_arg));
    } else { /* if no demand exists, use id*/
        int elems[num_rets * 4];
        int shape[2] = {num_rets, 4};

        for (i = 0; i < num_rets; i++) {
            pos = i * 4;
            for (j = 0; j < 4; j++) {
                elems[pos + j] = j;
            }
        }
        current_fundef_arg_demand = COmakeConstantFromArray (T_int, 2, shape, elems);
    }

    i = 0;
    while (current_ids != NULL) {
        /* Get right fundef-argument-demand-vector*/
        int shape_arg_dem[1];
        int elem_arg_dem[1];
        constant *arg_dem_sel_constant = NULL;
        constant *arg_dem_sel = NULL;

        shape_arg_dem[0] = 1;
        elem_arg_dem[0] = i;
        arg_dem_sel_constant
          = COmakeConstantFromArray (T_int, 1, shape_arg_dem, elem_arg_dem);
        arg_dem_sel = COsel (arg_dem_sel_constant, current_fundef_arg_demand);

        current_ids_demand = AVIS_DEMAND (IDS_AVIS (current_ids));

        /* construct shape for the selection array (only once)*/
        if (new_demand == NULL) {
            shape_extent = SHcreateShape (1, 1);
            oversel_shape
              = SHappendShapes (COgetShape (current_ids_demand), shape_extent);
            oversel_shape_constant = COmakeConstantFromShape (oversel_shape);
        }

        /* reshape ids demand to the right selection-vector-shape*/
        reshaped_ids_demand = COreshape (oversel_shape_constant, current_ids_demand);

        /* compute the selection*/
        selection_matrix = COoverSel (reshaped_ids_demand, arg_dem_sel);

        s = COconstant2String (selection_matrix);
        DBUG_PRINT ("SOSSK", ("Selection-Matrix: %s", s));

        if (new_demand == NULL) {
            new_demand = selection_matrix;
            selection_matrix = NULL;
        } else { /*MAX(all ids)*/
            constant *tmp_demand = NULL;
            tmp_demand = COmax (new_demand, selection_matrix);
            new_demand = COfreeConstant (new_demand);
            new_demand = tmp_demand;
            tmp_demand = NULL;
            selection_matrix = COfreeConstant (selection_matrix);
        }
        current_ids = IDS_NEXT (current_ids);

        arg_dem_sel = COfreeConstant (arg_dem_sel);
        arg_dem_sel_constant = COfreeConstant (arg_dem_sel_constant);
    } /* while*/

    /* free constants and shapes*/
    reshaped_ids_demand = COfreeConstant (reshaped_ids_demand);
    oversel_shape_constant = COfreeConstant (oversel_shape_constant);
    oversel_shape = SHfreeShape (oversel_shape);
    shape_extent = SHfreeShape (shape_extent);
    if (AVIS_DEMAND (ARG_AVIS (fundef_arg)) == NULL) {
        current_fundef_arg_demand = COfreeConstant (current_fundef_arg_demand);
    }

    DBUG_PRINT ("SOSSK", ("<<< LEAVE computeDemand"));
    DBUG_RETURN (new_demand);
}

/** <!-- ****************************************************************** -->
 *
 * @fn static constant *doOverSel(constant *idx, constant *matrix)
 *
 *    @brief This function executes an Overselection. Therefor it first
 *           reshapes the matrix in a certain way.
 *
 *    @param idx Selectionarray
 *    @param matrix Matrixarray to select from.
 *
 *    @return the new array
 ******************************************************************************/

static constant *
doOverSel (constant *idx, constant *matrix)
{
    DBUG_ENTER ("doOverSel");
    DBUG_PRINT ("SOSSK", (">>> ENTER doOverSel"));

    shape *shape_extent = NULL;
    shape *oversel_shape = NULL;
    constant *oversel_shape_constant = NULL;
    constant *reshaped_matrix = NULL;
    constant *result = NULL;

    /* First reshape the matrix*/
    shape_extent = SHcreateShape (1, 1);
    oversel_shape = SHappendShapes (COgetShape (matrix), shape_extent);
    oversel_shape_constant = COmakeConstantFromShape (oversel_shape);
    reshaped_matrix = COreshape (oversel_shape_constant, matrix);

    /* execute Overselection*/
    result = COoverSel (reshaped_matrix, idx);

    /* Free reshape constants and shapes*/
    reshaped_matrix = COfreeConstant (reshaped_matrix);
    oversel_shape_constant = COfreeConstant (oversel_shape_constant);
    oversel_shape = SHfreeShape (oversel_shape);
    shape_extent = SHfreeShape (shape_extent);

    DBUG_PRINT ("SOSSK", ("<<< LEAVE doOverSel"));
    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKap(node *arg_node, info *arg_info)
 *
 *    @brief This function ap first inserts the arguments into the INFO
 *           structur and follows the link to the fundef.
 *
 *    @param arg_node N_ap node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_ap node
 ******************************************************************************/

node *
SOSSKap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKap");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKap"));

    INFO_ARGS (arg_info) = AP_ARGS (arg_node);
    INFO_AP_CALL (arg_info) = TRUE;

    AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

    INFO_AP_CALL (arg_info) = FALSE;
    INFO_ARGS (arg_info) = NULL;

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKap"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKarg(node *arg_node, info *arg_info)
 *
 *    @brief This function computes the new demand for the function
 *           arguments of the function application
 *
 *    @param arg_node N_arg node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_arg node
 ******************************************************************************/

node *
SOSSKarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKarg");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKarg"));

    constant *new_demand = NULL;
    node *current_ap_args = INFO_ARGS (arg_info);
    constant *current_ap_arg_demand = NULL;

    /* This Part figures out the new demand*/
    if (INFO_COPY_DEMAND (arg_info) == TRUE) {

        current_ap_arg_demand = AVIS_DEMAND (ID_AVIS (EXPRS_EXPR (current_ap_args)));
        new_demand
          = computeDemand (INFO_IDS (arg_info), arg_node, INFO_NUM_RETS (arg_info));
        DBUG_PRINT ("SOSSK", ("Dimension: %i\n", COgetDim (new_demand)));
        DBUG_ASSERT (COgetDim (new_demand) == 2, "Dimension have to be 2!");

        if (current_ap_arg_demand == NULL) {
            AVIS_DEMAND (ID_AVIS (EXPRS_EXPR (current_ap_args))) = new_demand;
            INFO_DEMAND_HAS_CHANGED (arg_info) = TRUE;
        } else {
            char *s = NULL;
            constant *tmp_constant = NULL;
            s = COconstant2String (current_ap_arg_demand);
            DBUG_PRINT ("SOSSK", ("current_ap_arg_demand: %s", s));
            s = COconstant2String (new_demand);
            DBUG_PRINT ("SOSSK", ("new_demand:            %s", s));
            tmp_constant = COmax (current_ap_arg_demand, new_demand);

            if (COcompareConstants (tmp_constant, current_ap_arg_demand) != TRUE) {
                INFO_DEMAND_HAS_CHANGED (arg_info) = TRUE;
            }
            current_ap_arg_demand = COfreeConstant (current_ap_arg_demand);
            new_demand = COfreeConstant (new_demand);

            AVIS_DEMAND (ID_AVIS (EXPRS_EXPR (current_ap_args))) = tmp_constant;
            tmp_constant = NULL;
        }

        /* There are ap_args left*/
        if (current_ap_args != NULL) {

            if (ARG_NEXT (arg_node) != NULL) {

                INFO_ARGS (arg_info) = EXPRS_NEXT (INFO_ARGS (arg_info));

                ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
            } else if (EXPRS_NEXT (current_ap_args) != NULL) {
                INFO_NUM_ARGS_EQ_NUM_ARGS (arg_info) = FALSE;
            }
        } /* (current_ap_args != NULL)*/
        else {
            INFO_NUM_ARGS_EQ_NUM_ARGS (arg_info) = FALSE;
        }
    }      /* (INFO_COPY_DEMAND(arg_info) == TRUE)*/
    else { /* Count number of arguments*/
        INFO_NUM_ARGS (arg_info) = INFO_NUM_ARGS (arg_info) + 1;

        if (ARG_NEXT (arg_node) != NULL) {
            ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
        }
    }
    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKarg"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKassign(node *arg_node, info *arg_info)
 *
 *    @brief This function travels the the sons in a specific way. First
 *           the next assignments have to be visited. On the way back
 *           the Instruction is visited.
 *
 *    @param arg_node N_assign node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_assign node
 ******************************************************************************/

node *
SOSSKassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKassign");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKassign"));

    constant *old_demand = INFO_DEMAND (arg_info);
    int num_rets = SHgetExtent (COgetShape (old_demand), 0);
    int dim = SHgetDim (COgetShape (old_demand));
    int new_shape[2] = {num_rets, 4};
    int elems[dim];
    int i = 0;
    int j = 0;
    int pos = 0;

    /* Constuct the demand [0,1,2,3] for the lamda app.*/
    for (i = 0; i < num_rets; i++) {
        pos = i * 4;
        for (j = 0; j < 4; j++) {
            elems[pos + j] = j;
        }
    }

    INFO_DEMAND (arg_info) = COmakeConstantFromArray (T_int, dim, new_shape, elems);

    /* Go on to the next assignment first to compute the demands*/
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKassign"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKexprs(node *arg_node, info *arg_info)
 *
 *    @brief If this function is called from a prf-node, get demand for the
 *           argument-position and traverse into the expr.
 *
 *    @param arg_node N_exprs node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_exprs node
 ******************************************************************************/

node *
SOSSKexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKexprs");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKexprs"));

    constant *arg_constant = NULL;
    constant *old_demand = INFO_DEMAND (arg_info);

    /* If this function was called from a prf get the demand*/
    if (INFO_PRF_NAME (arg_info) > 0) {
        arg_constant = prf_shape_oracle_funtab[INFO_PRF_NAME (arg_info)](
          INFO_POS_PRF_ARG (arg_info));

        /* If there exists a demand, do Overselection*/
        if (arg_constant != NULL) {
            INFO_DEMAND (arg_info) = doOverSel (old_demand, arg_constant);
        } else {
            arg_node = TRAVcont (arg_node, arg_info);
        }

        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);

        INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
        INFO_DEMAND (arg_info) = old_demand;
        old_demand = NULL;

        if (EXPRS_NEXT (arg_node) != NULL) {
            EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
        }
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKexprs"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKgenarray(node *arg_node, info *arg_info)
 *
 *    @brief This function traversed further into the shape-node and into the
 *           node of the default-element. Therefor specific demands are
 *           computed.
 *
 *    @param arg_node N_genarray node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_genarray node
 ******************************************************************************/

node *
SOSSKgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKgenarray");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKgenarray"));

    constant *old_demand = INFO_DEMAND (arg_info);
    constant *new_demand = NULL;
    int num_rets = SHgetExtent (COgetShape (old_demand), 0);
    int dim = SHgetDim (COgetShape (old_demand));
    int new_shape[2] = {num_rets, 4};
    int elems[dim];
    int i = 0;
    int offset = 0;

    /* construct demand [0,2,3,3]*/
    for (i = 0; i < num_rets; i++) {
        offset = 4 * i;
        elems[offset] = 0;
        elems[offset + 1] = 2;
        elems[offset + 2] = 3;
        elems[offset + 3] = 3;
    }

    new_demand = COmakeConstantFromArray (T_int, dim, new_shape, elems);

    /* compute shape-demand*/
    INFO_DEMAND (arg_info) = doOverSel (old_demand, new_demand);

    new_demand = COfreeConstant (new_demand);

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;

    GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKgenarray"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKgenerator(node *arg_node, info *arg_info)
 *
 *    @brief Traverses into the bounds.
 *
 *    @param arg_node N_generator node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_generator node
 ******************************************************************************/

node *
SOSSKgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKgenarray");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKgenerator"));

    constant *old_demand = COcopyConstant (INFO_DEMAND (arg_info));

    GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = COcopyConstant (old_demand);

    GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;

    old_demand = NULL;

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKgenerator"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKid(node *arg_node, info *arg_info)
 *
 *    @brief This function computes the demand of a Id.
 *
 *    @param arg_node N_id node
 *    @param arg_info INFO structure
 *
 *    @return N_id node
 *******************************************************************************/

node *
SOSSKid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKid");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKid"));

    constant *tmp_constant = NULL;
    constant *id_demand = AVIS_DEMAND (ID_AVIS (arg_node));

    /* If there exists a demand, compute the maximum demand of the current
     * demand and the old demand.*/
    if (id_demand != NULL) {
        tmp_constant = COmax (id_demand, INFO_DEMAND (arg_info));
        id_demand = COfreeConstant (id_demand);
        AVIS_DEMAND (ID_AVIS (arg_node)) = tmp_constant;
        tmp_constant = NULL;
    } else {
        AVIS_DEMAND (ID_AVIS (arg_node)) = COcopyConstant (INFO_DEMAND (arg_info));
    }

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKid"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKids(node *arg_node, info *arg_info)
 *
 *    @brief This function computes the demand for the right side of a
 *           let-expr.
 *
 *    @param arg_node N_ids node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_ids node
 ******************************************************************************/

node *
SOSSKids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKids");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKids"));

    constant *old_demand = INFO_DEMAND (arg_info);
    constant *ids_demand = AVIS_DEMAND (IDS_AVIS (arg_node));
    constant *sel_old_demand = NULL;
    constant *sel_ids_demand = NULL;
    constant *idx = NULL;
    constant *tmp_demand = NULL;
    constant *tmp2_demand = NULL;
    constant *shape_constant = NULL;
    int rows = SHgetExtent (COgetShape (old_demand), 0);
    int i = 0;

    DBUG_ASSERT (COgetDim (old_demand) == COgetDim (ids_demand),
                 "Demand have different dimensions!");

    INFO_DEMAND (arg_info) = NULL;
    shape_constant = COmakeConstantFromDynamicArguments (T_int, 1, 2, 1, 4);

    /* Compute the new demand*/
    for (i = 0; i < rows; i++) {
        int idx_elems[1];
        int idx_shape[1];

        idx_elems[0] = i;
        idx_shape[0] = 1;
        constant *reshaped_sel_old_demand = NULL;
        constant *reshaped_sel_ids_demand = NULL;

        /* Get a single row*/
        idx = COmakeConstantFromArray (T_int, 1, idx_shape, idx_elems);

        sel_old_demand = COsel (idx, old_demand);
        sel_ids_demand = COsel (idx, ids_demand);

        /* Reshape the selected row to [1,4]*/
        reshaped_sel_ids_demand = COreshape (shape_constant, sel_ids_demand);

        tmp_demand = doOverSel (sel_old_demand, reshaped_sel_ids_demand);

        /* Concatenate the rows to get the whole demand-matrix*/
        if (INFO_DEMAND (arg_info) == NULL) {
            INFO_DEMAND (arg_info) = COcopyConstant (tmp_demand);
        } else {
            tmp2_demand = COcat (INFO_DEMAND (arg_info), tmp_demand);
            INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
            INFO_DEMAND (arg_info) = tmp2_demand;
            tmp2_demand = NULL;
        }

        tmp_demand = COfreeConstant (tmp_demand);
        reshaped_sel_ids_demand = COfreeConstant (reshaped_sel_ids_demand);
        sel_ids_demand = COfreeConstant (sel_ids_demand);
        sel_old_demand = COfreeConstant (sel_old_demand);
        idx = COfreeConstant (idx);
    }

    DBUG_ASSERTF (COgetDim (INFO_DEMAND (arg_info)) == 2,
                  ("Dim should be 2! BUT IS %i", COgetDim (INFO_DEMAND (arg_info))));

    shape_constant = COfreeConstant (shape_constant);
    old_demand = COfreeConstant (old_demand);

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKids"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKret(node *arg_node, info *arg_info)
 *
 *    @brief This function computes the number of return-values.
 *
 *    @param arg_node N_ret node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_ret node
 ******************************************************************************/

node *
SOSSKret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKret");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKret"));

    INFO_NUM_RETS (arg_info) = INFO_NUM_RETS (arg_info) + 1;

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKret"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKprf(node *arg_node, info *arg_info)
 *
 *    @brief If there exists a demand-function for the specific prf-function
 *           traverse further to the arguments to compute the demands.
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
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKprf"));

    prf prf;

    prf = PRF_PRF (arg_node);

    DBUG_PRINT ("SOSSK", (">>>> enter PRF: %s", PRF_NAME (prf)));

    /* If there exists a demand-function for this prf, compute the demands*/
    if (prf_shape_oracle_funtab[prf] != NULL) {

        INFO_PRF_NAME (arg_info) = prf;
        INFO_POS_PRF_ARG (arg_info) = 0;

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        INFO_POS_PRF_ARG (arg_info) = -1;
        INFO_PRF_NAME (arg_info) = 0;
    } else {
        DBUG_PRINT ("SOSSK", ("Points to NULL!"));
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_PRINT ("SOSSK", ("<<<< leave PRF: %s", PRF_NAME (prf)));
    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKprf"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKfundef(node *arg_node, info arg_info)
 *
 *    @brief This function computes the demands of the arguments of the calling
 *           function-ap. Therefor first of all the general demand of the
 *           arguments of the function itself has to be computed. This is done
 *           with a fixpoint-iteration.
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
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKfundef"));

    int last_iteration_round = FUNDEF_LASTITERATIONROUND (arg_node);
    int i = 0;
    int j = 0;
    int pos = 0;

    /* Keep info*/
    info *old_info = arg_info;
    arg_info = MakeInfo ();
    INFO_ITERATION_ROUND (arg_info) = INFO_ITERATION_ROUND (old_info);

    DBUG_PRINT ("SOSSK", (">>>> Enter Function: %s", FUNDEF_NAME (arg_node)));

    FUNDEF_LASTITERATIONROUND (arg_node) = INFO_ITERATION_ROUND (old_info);

    /* Count the number of return values and numbers of arguments*/
    FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);

    /* If the function has no arguments, there is nothing to do*/
    if (INFO_NUM_ARGS (arg_info) != 0) {
        /* If this function has no body the demand should be annotated*/
        if (FUNDEF_BODY (arg_node) == NULL) {
            FUNDEF_FIXPOINTFOUND (arg_node) = TRUE;
        }

        /* This is the first time in this iter. round we handel this function AND
         * the fixpoint has not been found yet. */
        if ((last_iteration_round != INFO_ITERATION_ROUND (old_info))
            && (FUNDEF_FIXPOINTFOUND (arg_node) != TRUE)) {
            DBUG_PRINT ("SOSSK", (" IF fixpoint not found AND not handled yet"));
            int elems[INFO_NUM_RETS (arg_info) * 4];
            int shape[2] = {INFO_NUM_RETS (arg_info), 4};

            for (i = 0; i < INFO_NUM_RETS (arg_info); i++) {
                pos = i * 4;
                for (j = 0; j < 4; j++) {
                    elems[pos + j] = 0;
                }
            }
            INFO_DEMAND (arg_info) = COmakeConstantFromArray (T_int, 2, shape, elems);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
        }

        /* If there are ap-args, compute demand*/
        if (INFO_ARGS (old_info) != NULL) {
            INFO_IDS (arg_info) = INFO_IDS (old_info);
            INFO_ARGS (arg_info) = INFO_ARGS (old_info);
            INFO_COPY_DEMAND (arg_info) = TRUE;

            if (last_iteration_round == INFO_ITERATION_ROUND (arg_info)) {
                INFO_ESTIMATION (arg_info) = TRUE;
            }

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
            } else if (INFO_IDS (arg_info) != NULL) {
                INFO_NUM_ARGS_EQ_NUM_ARGS (arg_info) = TRUE;
            }

            DBUG_ASSERT (INFO_NUM_ARGS_EQ_NUM_ARGS (arg_info) == TRUE,
                         "#fundef args != #ap args in SOSSKfundef!");
        }

        /* If the demand has changed during this iteration, maybe the fixpoint
         * has not been found yet. Otherwise if the demand has not changed for
         * two iteration-rounds, the fixpoint has been found.*/
        if (INFO_DEMAND_HAS_CHANGED (arg_info) == TRUE) {
            FUNDEF_LASTCHANGE (arg_node) = INFO_ITERATION_ROUND (arg_info);
            INFO_DEMAND_HAS_CHANGED (old_info) = TRUE;
        } else if (((FUNDEF_LASTCHANGE (arg_node) - INFO_ITERATION_ROUND (arg_info)) >= 2)
                   || (INFO_AP_FOUND (arg_info) == FALSE)) {
            FUNDEF_FIXPOINTFOUND (arg_node) = TRUE;
        }
    }

    /* Restore old Information*/
    INFO_IDS (arg_info) = NULL;
    INFO_ARGS (arg_info) = NULL;

    arg_info = FreeInfo (arg_info);
    arg_info = old_info;
    old_info = NULL;

    /* If this is NOT an ap-link, follow the fundef-chain*/
    if (INFO_AP_CALL (arg_info) != TRUE) {
        DBUG_PRINT ("SOSSK", ("Follow FUNDEF-chain"));
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_PRINT ("SOSSK", ("<<<< Leave Function %s", FUNDEF_NAME (arg_node)));
    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKfundef"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKlet(node *arg_node, info *arg_info)
 *
 *    @brief This function inserts the IDS into the INFO-structure. This is
 *           needed in case of a funap. Also this function traverses further
 *           into the IDS to compute the demand which is needed if this is
 *           no funap.
 *
 *    @param arg_node N_let node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_let node
 *******************************************************************************/

node *
SOSSKlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKlet");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKlet"));

    INFO_IDS (arg_info) = LET_IDS (arg_node);

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKlet"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKpart(node *arg_node, info *arg_info)
 *
 *    @brief This function gets the demand of the Id and traverses futher into
 *           the generator.
 *
 *    @param arg_node N_part node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_part node
 ******************************************************************************/

node *
SOSSKpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKpart");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKpart"));
    constant *old_demand = COcopyConstant (INFO_DEMAND (arg_info));

    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;
    old_demand = NULL;

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKpart"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKwith(node *arg_node, info *arg_info)
 *
 *    @brief This function just saves the old-demand and traverses further into
 *           certain parts of the withloop. After each traverse, the old demand
 *           is restored because most likely the demand has been changed in the
 *           INFO structure.
 *
 *    @param arg_node N_with node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_with node
 *******************************************************************************/

node *
SOSSKwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SOSSKwith");
    DBUG_PRINT ("SOSSK", (">>> ENTER SOSSKwith"));
    constant *old_demand = COcopyConstant (INFO_DEMAND (arg_info));

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));

    INFO_DEMAND (arg_info) = COcopyConstant (old_demand);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));

    INFO_DEMAND (arg_info) = COcopyConstant (old_demand);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    old_demand = COfreeConstant (old_demand);
    DBUG_PRINT ("SOSSK", ("<<< LEAVE SOSSKwith"));
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKdoSpecializationOracleSSK(node *syntax_tree)
 *
 *    @brief This function starts new traversal until all fixpoints of all
 *           functions have been found.
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

    do {
        INFO_DEMAND_HAS_CHANGED (info) = FALSE;
        INFO_ITERATION_ROUND (info) = INFO_ITERATION_ROUND (info) + 1;

        syntax_tree = TRAVdo (syntax_tree, info);
    } while (INFO_DEMAND_HAS_CHANGED (info) == TRUE);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
