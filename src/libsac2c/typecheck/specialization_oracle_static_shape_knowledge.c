#include "specialization_oracle_static_shape_knowledge.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str_buffer.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "SOSSK_DEMAND"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "shape.h"
#include "prf_pvs_info.h"
#include "new_types.h"
#include "map_avis_trav.h"

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
    int pos_ret;
    node *args;
    node *ids;
    constant *demand;
    bool estimation;
    bool copy_demand;
    bool num_args_eq_num_args;
    bool demand_has_changed;
    bool ap_found;
    bool ap_call;
    bool ext_fun;
    prf prf_name;
};

/**
 * INFO macros
 */

#define INFO_ITERATION_ROUND(n) (n->iteration_round)
#define INFO_NUM_RETS(n) (n->num_rets)
#define INFO_NUM_ARGS(n) (n->num_args)
#define INFO_POS_PRF_ARG(n) (n->pos_prf_arg)
#define INFO_POS_RET(n) (n->pos_ret)
#define INFO_ARGS(n) (n->args)
#define INFO_IDS(n) (n->ids)
#define INFO_DEMAND(n) (n->demand)
#define INFO_ESTIMATION(n) (n->estimation)
#define INFO_COPY_DEMAND(n) (n->copy_demand)
#define INFO_NUM_ARGS_EQ_NUM_ARGS(n) (n->num_args_eq_num_args)
#define INFO_DEMAND_HAS_CHANGED(n) (n->demand_has_changed)
#define INFO_AP_FOUND(n) (n->ap_found)
#define INFO_AP_CALL(n) (n->ap_call)
#define INFO_EXT_FUN(n) (n->ext_fun)
#define INFO_PRF_NAME(n) (n->prf_name)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER MakeInfo");

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ITERATION_ROUND (result) = 0;
    INFO_NUM_RETS (result) = 0;
    INFO_NUM_ARGS (result) = 0;
    INFO_POS_PRF_ARG (result) = -1;
    INFO_POS_RET (result) = -1;
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
    INFO_EXT_FUN (result) = FALSE;
    INFO_PRF_NAME (result) = (prf)0;

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE MakeInfo");
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER FreeInfo");

    info = MEMfree (info);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE FreeInfo");
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
 * @fn node *SOSSKresetFundefDemand(node *fundef_node)
 *
 *    @brief is a function which can be called from the outside. It resets the
 *           flags and attributes of the given function. Furthermore, all
 *           annotations of demands at avis nodes are freed.
 *
 *    @param N_fundef fundef_node
 *
 *    @return N_fundef node
 ******************************************************************************/
static node *
FreeAvisDemand (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_DEMAND (arg_node) != NULL) {
        AVIS_DEMAND (arg_node) = COfreeConstant (AVIS_DEMAND (arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
SOSSKresetFundefDemand (node *fundef_node)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (fundef_node) == N_fundef,
                 "SOSSKresetFundefFlags is intended to run only on N_fundef nodes");

    FUNDEF_FIXPOINTFOUND (fundef_node) = FALSE;
    FUNDEF_LASTCHANGE (fundef_node) = 0;
    FUNDEF_LASTITERATIONROUND (fundef_node) = 0;

    fundef_node = MATdoMapAvisTravOneFundef (fundef_node, NULL, FreeAvisDemand);

    DBUG_RETURN (fundef_node);
}

#ifndef DBUG_OFF

/** <!-- ****************************************************************** -->
 *
 * @fn char *demand2String(constant *demand)
 *
 *    @brief This function converts the given demand into a string. The
 *           the difference to COconstant2String is that this function
 *           returns " " if the demand is NULL.
 *
 *    @param demand the demand to be converted
 *
 *    @return the string which represents the demand
 ******************************************************************************/
static char *
demand2String (constant *demand)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER demand2String");

    char *res = NULL;

    if (demand != NULL) {
        res = COconstant2String (demand);
    } else {
        res = STRcpy (" ");
    }

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE demand2String");
    DBUG_RETURN (res);
}

#endif /* DBUG_OFF */

/** <!-- ****************************************************************** -->
 *
 * @fn constant *computeDemand(node *ids, node *ap_arg, node *fundef_arg,
 *                             int num_rets)
 *
 *    @brief this function computes the demand of a fundef.
 *
 *    @param ids ids of the let which was leading to this fundef
 *    @param fundef_arg args of the fundef
 *    @param num_rets number of returnvalues of the fundef
 *
 *    @return the computed demand of the given argument
 ******************************************************************************/

static constant *
computeDemand (node *ids, node *fundef_arg, int num_rets, bool is_ext_fun)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER computeDemand");

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
#ifndef DBUG_OFF
    char *string = NULL;
#endif
    if (AVIS_DEMAND (ARG_AVIS (fundef_arg)) == NULL) { /* if no demand exists, ...*/
        int *elems;
        int shape[2] = {num_rets, 4};

        elems = (int *)MEMmalloc (num_rets * 4 * sizeof (int));
        for (i = 0; i < num_rets; i++) {
            pos = i * 4;
            for (j = 0; j < 4; j++) {
                elems[pos + j] = 0;
            }
        }
        AVIS_DEMAND (ARG_AVIS (fundef_arg))
          = COmakeConstantFromArray (T_int, 2, shape, elems);
        MEMfree (elems);
    }

    /* get demands*/
    current_fundef_arg_demand = AVIS_DEMAND (ARG_AVIS (fundef_arg));
    i = 0;
    while (current_ids != NULL) {
        /* Get right fundef-argument-demand-vector*/
        int shape_arg_dem[1];
        int elem_arg_dem[1];
        constant *arg_dem_sel_constant = NULL;
        constant *arg_dem_sel = NULL;

        /* If i >= #returnValues, than the returns of the fundef contains dotted
         * returnvals. As it is a ext.fun., every returnval has the same demand,
         * so use the last one.*/
        if (i >= num_rets) {
            i = num_rets - 1;
        }

        shape_arg_dem[0] = 1;
        elem_arg_dem[0] = i;

        arg_dem_sel_constant
          = COmakeConstantFromArray (T_int, 1, shape_arg_dem, elem_arg_dem);
        arg_dem_sel = COsel (arg_dem_sel_constant, current_fundef_arg_demand, NULL);

        current_ids_demand = AVIS_DEMAND (IDS_AVIS (current_ids));

        if (current_ids_demand != NULL) {
            /* construct shape for the selection array (only once)*/
            if (oversel_shape_constant == NULL) {
                shape_extent = SHcreateShape (1, 1);
                oversel_shape
                  = SHappendShapes (COgetShape (current_ids_demand), shape_extent);
                oversel_shape_constant = COmakeConstantFromShape (oversel_shape);
            }
            /* reshape ids demand to the right selection-vector-shape*/
            reshaped_ids_demand
              = COreshape (oversel_shape_constant, current_ids_demand, NULL);

            /* compute the selection*/
            selection_matrix = COoverSel (reshaped_ids_demand, arg_dem_sel, NULL);
        } else {
            selection_matrix = NULL;
        }

        DBUG_EXECUTE_TAG ("SOSSK", string = demand2String (selection_matrix));
        DBUG_PRINT_TAG ("SOSSK", "Selection-Matrix: %s", string);
        DBUG_EXECUTE_TAG ("SOSSK", string = MEMfree (string));

        if (new_demand == NULL) {
            new_demand = selection_matrix;
            selection_matrix = NULL;
        } else if (selection_matrix != NULL) { /*MAX(all ids)*/
            constant *tmp_demand = NULL;
            tmp_demand = COmax (new_demand, selection_matrix, NULL);
            new_demand = COfreeConstant (new_demand);
            new_demand = tmp_demand;
            tmp_demand = NULL;
            selection_matrix = COfreeConstant (selection_matrix);
        }
        current_ids = IDS_NEXT (current_ids);

        arg_dem_sel = COfreeConstant (arg_dem_sel);
        arg_dem_sel_constant = COfreeConstant (arg_dem_sel_constant);
        i = i + 1;
    } /* while*/

    /* free constants and shapes*/
    if (reshaped_ids_demand != NULL) {
        reshaped_ids_demand = COfreeConstant (reshaped_ids_demand);
    }
    if (oversel_shape_constant != NULL) {
        oversel_shape_constant = COfreeConstant (oversel_shape_constant);
    }
    if (oversel_shape != NULL) {
        oversel_shape = SHfreeShape (oversel_shape);
    }
    if (shape_extent != NULL) {
        shape_extent = SHfreeShape (shape_extent);
    }
    if (AVIS_DEMAND (ARG_AVIS (fundef_arg)) == NULL) {
        current_fundef_arg_demand = COfreeConstant (current_fundef_arg_demand);
    }

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE computeDemand");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER doOverSel");

    shape *shape_extent = NULL;
    shape *oversel_shape = NULL;
    constant *oversel_shape_constant = NULL;
    constant *reshaped_idx = NULL;
    constant *result = NULL;

    /* First reshape the idx*/
    shape_extent = SHcreateShape (1, 1);
    oversel_shape = SHappendShapes (COgetShape (idx), shape_extent);
    oversel_shape_constant = COmakeConstantFromShape (oversel_shape);
    reshaped_idx = COreshape (oversel_shape_constant, idx, NULL);

    /* execute Overselection*/
    result = COoverSel (reshaped_idx, matrix, NULL);

    /* Free reshape constants and shapes*/
    reshaped_idx = COfreeConstant (reshaped_idx);
    oversel_shape_constant = COfreeConstant (oversel_shape_constant);
    oversel_shape = SHfreeShape (oversel_shape);
    shape_extent = SHfreeShape (shape_extent);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE doOverSel");
    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *doOverSelMatrix(constant *idx_matrix, constant *sel_matrix)
 *
 *    @brief This function gets two demand-matrixes. It then applies a
 *           row-wise overselection.
 *
 *    @param idx_matrix The matrix which contains the index-vectors
 *    @param sel_matrix The matrix on which the oversel is applied
 *
 *    @return a matrix which contains the results of the row-wise oversel
 ******************************************************************************/

static constant *
doOverSelMatrix (constant *idx_matrix, constant *sel_matrix)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> doOverSelMatrix");

    constant *res_matrix = NULL;
    constant *sel_idx_matrix = NULL;
    constant *sel_sel_matrix = NULL;
    constant *idx = NULL;
    constant *tmp_demand = NULL;
    constant *tmp2_demand = NULL;
    constant *shape_constant = NULL;
    int rows = SHgetExtent (COgetShape (idx_matrix), 0);
    int i = 0;
#ifndef DBUG_OFF
    char *string = NULL;
#endif

    DBUG_ASSERT (COgetDim (idx_matrix) == COgetDim (sel_matrix),
                 "Demands have different dimensions!");
    shape_constant = COmakeConstantFromDynamicArguments (T_int, 1, 2, 1, 4);

    /* Compute the new demand-matrix*/
    for (i = 0; i < rows; i++) {
        int idx_elems[1];
        int idx_shape[1];

        idx_elems[0] = i;
        idx_shape[0] = 1;
        constant *reshaped_sel_idx_matrix = NULL;

        /* Get a single row*/
        idx = COmakeConstantFromArray (T_int, 1, idx_shape, idx_elems);

        sel_idx_matrix = COsel (idx, idx_matrix, NULL);
        sel_sel_matrix = COsel (idx, sel_matrix, NULL);

        /* Reshape the selected idx-row to [1,4]*/
        reshaped_sel_idx_matrix = COreshape (shape_constant, sel_idx_matrix, NULL);

        tmp_demand = doOverSel (reshaped_sel_idx_matrix, sel_sel_matrix);

        DBUG_PRINT ("--------------------");
        DBUG_PRINT ("<<---->>");
        DBUG_EXECUTE (string = demand2String (reshaped_sel_idx_matrix));
        DBUG_PRINT ("reshaped_sel_idx_matrix: %s", string);
        DBUG_EXECUTE (string = MEMfree (string));
        DBUG_EXECUTE (string = demand2String (sel_sel_matrix));
        DBUG_PRINT ("sel_sel_matrix: %s", string);
        DBUG_EXECUTE (string = MEMfree (string));
        DBUG_EXECUTE (string = demand2String (tmp_demand));
        DBUG_PRINT ("doOverSel: %s", string);
        DBUG_EXECUTE (string = MEMfree (string));

        /* Concatenate the rows to get the whole demand-matrix*/
        if (res_matrix == NULL) {
            res_matrix = COcopyConstant (tmp_demand);
        } else {
            tmp2_demand = COcat (res_matrix, tmp_demand, NULL);
            res_matrix = COfreeConstant (res_matrix);
            res_matrix = tmp2_demand;
            tmp2_demand = NULL;
        }

        DBUG_PRINT ("<----Whole View---->");
        DBUG_EXECUTE (string = demand2String (idx_matrix));
        DBUG_PRINT ("idx_matrix: %s", string);
        DBUG_EXECUTE (string = MEMfree (string));
        DBUG_EXECUTE (string = demand2String (sel_matrix));
        DBUG_PRINT ("sel_matrix: %s", string);
        DBUG_EXECUTE (string = MEMfree (string));
        DBUG_EXECUTE (string = demand2String (res_matrix));
        DBUG_PRINT ("doOverSel(rows): %s", string);
        DBUG_EXECUTE (string = MEMfree (string));

        tmp_demand = COfreeConstant (tmp_demand);
        reshaped_sel_idx_matrix = COfreeConstant (reshaped_sel_idx_matrix);
        sel_sel_matrix = COfreeConstant (sel_sel_matrix);
        sel_idx_matrix = COfreeConstant (sel_idx_matrix);
        idx = COfreeConstant (idx);
    }

    /*If res_matrix is NULL, the idx-matrix has the shape (0,4). Therefor
     * the result is the same.*/
    if (res_matrix == NULL) {
        res_matrix = COcopyConstant (idx_matrix);
    }
    DBUG_ASSERT (COgetDim (res_matrix) == 2, "Dim should be 2! BUT IS %i",
                 COgetDim (res_matrix));
    shape_constant = COfreeConstant (shape_constant);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE doOverSelMatrix");
    DBUG_RETURN (res_matrix);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *wrapperMax(node *fundef, node *args)
 *
 *    @brief This function is the fold function which is used to compute the
 *           maximum demand over all functions which belong to the wrapper.
 *
 *    @param fundef a N_fundef node
 *    @param wrapper the N_fundef node of the wrapper
 *
 *    @return the N_fundef node of the wrapper
 ******************************************************************************/

static node *
wrapperMax (node *fundef, node *wrapper)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER wrapperMax");
    DBUG_PRINT_TAG ("SOSSK_WRAPPER", "!###ENTER WRAPPERMAX %s###!", FUNDEF_NAME (fundef));

    node *cur_wrapper_arg = FUNDEF_ARGS (wrapper);
    node *cur_fundef_arg = FUNDEF_ARGS (fundef);

    constant *cur_wrapper_arg_dem = NULL;
    constant *cur_fundef_arg_dem = NULL;

#ifndef DBUG_OFF
    char *string = NULL;
#endif

    /* Reset the flag of the wrapper if fixpoint has not been found yet*/
    FUNDEF_FIXPOINTFOUND (wrapper)
      = FUNDEF_FIXPOINTFOUND (wrapper) && FUNDEF_FIXPOINTFOUND (fundef);

    /* Handle all arguments*/
    while (cur_wrapper_arg != NULL) {

        cur_wrapper_arg_dem = AVIS_DEMAND (ARG_AVIS (cur_wrapper_arg));
        cur_fundef_arg_dem = AVIS_DEMAND (ARG_AVIS (cur_fundef_arg));

        /* If the current fundef is NULL, there is no need to change the args_arg*/
        if (cur_fundef_arg_dem != NULL) {

            DBUG_PRINT ("--------------------");

            /* if the current args argument has no demand, just copy the one from
             * the fundef*/
            if (cur_wrapper_arg_dem != NULL) {
                constant *tmp_demand = NULL;
                tmp_demand = COmax (cur_wrapper_arg_dem, cur_fundef_arg_dem, NULL);

                DBUG_EXECUTE (string = demand2String (cur_wrapper_arg_dem));
                DBUG_PRINT ("cur_args_arg_dem: %s", string);
                DBUG_EXECUTE (string = MEMfree (string));
                DBUG_EXECUTE (string = demand2String (cur_fundef_arg_dem));
                DBUG_PRINT ("cur_fundef_arg_demand: %s", string);
                DBUG_EXECUTE (string = MEMfree (string));
                DBUG_EXECUTE (string = demand2String (tmp_demand));
                DBUG_PRINT ("COmax: %s", string);
                DBUG_EXECUTE (string = MEMfree (string));

                cur_wrapper_arg_dem = COfreeConstant (cur_wrapper_arg_dem);
                AVIS_DEMAND (ARG_AVIS (cur_wrapper_arg)) = tmp_demand;
                tmp_demand = NULL;
            } else {
                AVIS_DEMAND (ARG_AVIS (cur_wrapper_arg))
                  = COcopyConstant (cur_fundef_arg_dem);
                DBUG_EXECUTE (string = demand2String (cur_fundef_arg_dem));
                DBUG_PRINT ("cur_fundef_arg_demand: %s", string);
                DBUG_EXECUTE (string = MEMfree (string));
            }
            DBUG_EXECUTE_TAG ("SOSSK", string = demand2String (
                                         AVIS_DEMAND (ARG_AVIS (cur_wrapper_arg))));
            DBUG_PRINT_TAG ("SOSSK", "Add demand %s to %s", string,
                            ARG_NAME (cur_wrapper_arg));
            DBUG_EXECUTE_TAG ("SOSSK", string = MEMfree (string));
        } /* if(cur_fundef_arg_dem != NULL)*/
        else {
            DBUG_PRINT ("FUNDEF_ARG CONTAINS NO DEMAND!");
        }

        DBUG_ASSERT (((ARG_NEXT (cur_wrapper_arg) != NULL)
                      && (ARG_NEXT (cur_fundef_arg) != NULL))
                       || ((ARG_NEXT (cur_wrapper_arg) == NULL)
                           && (ARG_NEXT (cur_fundef_arg) == NULL)),
                     "Wrapper fun and funct. have different number of arguments!");

        /* Go on with the next argument*/
        cur_wrapper_arg = ARG_NEXT (cur_wrapper_arg);
        cur_fundef_arg = ARG_NEXT (cur_fundef_arg);
    } /* while*/

    DBUG_PRINT_TAG ("SOSSK_WRAPPER", "!###LEAVE WRAPPERMAX###!");
    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE wrapperMax");
    DBUG_RETURN (wrapper);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *getReturnDemand(constant *demand, int row)
 *
 *    @brief This function returns a demand matrix where every row except the
 *           in "row" indicated one is zero
 *           This is needed to set the initial demand of the return arguments
 *           right
 *
 *    @param demand the actual demand matrix
 *    @param row the needed row
 *
 *    @return demand matrix for the return argumen at position "row"
 ******************************************************************************/

static constant *
getReturnDemand (constant *demand, int row)
{
    DBUG_ENTER ();

    constant *idx = COmakeConstantFromDynamicArguments (T_int, 1, 1, row);
    constant *res_row = COsel (idx, demand, NULL);
    constant *res = NULL;

    int *res_row_int = (int *)COgetDataVec (res_row);
    int num_rets = SHgetExtent (COgetShape (demand), 0);
    int dim = SHgetDim (COgetShape (demand));
    int new_shape[2] = {num_rets, 4};
    int *elems;
    int i = 0;
    int j = 0;
    int pos = 0;

#ifndef DBUG_OFF
    char *string = NULL;
#endif

    elems = (int *)MEMmalloc (num_rets * 4 * sizeof (int));
    for (i = 0; i < num_rets; i++) {
        pos = i * 4;
        for (j = 0; j < 4; j++) {
            if (i == row) {
                elems[pos + j] = res_row_int[j];
            } else {
                elems[pos + j] = 0;
            }
        }
    }

    res = COmakeConstantFromArray (T_int, dim, new_shape, elems);

    DBUG_PRINT ("<--getReturnDemand-->");
    DBUG_EXECUTE (string = COconstant2String (res));
    DBUG_PRINT ("demand res %i: %s", row, string);
    DBUG_EXECUTE (string = MEMfree (string));
    DBUG_PRINT (">-------------------<");
    COfreeConstant (idx);
    COfreeConstant (res_row);
    MEMfree (elems);
    DBUG_RETURN (res);
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKap");

    INFO_ARGS (arg_info) = AP_ARGS (arg_node);
    INFO_AP_CALL (arg_info) = TRUE;

    AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

    INFO_AP_FOUND (arg_info) = TRUE;
    INFO_AP_CALL (arg_info) = FALSE;
    INFO_ARGS (arg_info) = NULL;

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKap");
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
    DBUG_ENTER ();
    constant *new_demand = NULL;
    node *current_ap_args = INFO_ARGS (arg_info);
    constant *current_ap_arg_demand = NULL;

#ifndef DBUG_OFF
    char *string = NULL;
#endif

    /* This Part figures out the new demand*/
    if (INFO_COPY_DEMAND (arg_info) == TRUE) {
        DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKarg %s (COPY)",
                        ARG_NAME (arg_node));

        current_ap_arg_demand = AVIS_DEMAND (ID_AVIS (EXPRS_EXPR (current_ap_args)));
        new_demand = computeDemand (INFO_IDS (arg_info), arg_node,
                                    INFO_NUM_RETS (arg_info), INFO_EXT_FUN (arg_info));
        DBUG_ASSERT (COgetDim (new_demand) == 2, "Dimension have to be 2! But is %i",
                     COgetDim (new_demand));
        DBUG_PRINT ("--------------------");
        DBUG_EXECUTE (string = demand2String (current_ap_arg_demand));
        DBUG_PRINT ("ap_arg_demand: %s", string);
        DBUG_EXECUTE (string = MEMfree (string));
        DBUG_EXECUTE (string = demand2String (new_demand));
        DBUG_PRINT ("new_demand:    %s", string);
        DBUG_EXECUTE (string = MEMfree (string));

        if (current_ap_arg_demand == NULL) {
            AVIS_DEMAND (ID_AVIS (EXPRS_EXPR (current_ap_args))) = new_demand;
            INFO_DEMAND_HAS_CHANGED (arg_info) = TRUE;
        } else { /* Get the max-demand*/
            constant *tmp_constant = NULL;

            DBUG_ASSERT (SHcompareShapes (COgetShape (current_ap_arg_demand),
                                          COgetShape (new_demand))
                           == TRUE,
                         "shape(current_ap_arg) %s != shape(new_demand) %s! (%s)",
                         SHshape2String (0, COgetShape (current_ap_arg_demand)),
                         SHshape2String (0, COgetShape (new_demand)),
                         ID_NAME (EXPRS_EXPR (current_ap_args)));

            tmp_constant = COmax (current_ap_arg_demand, new_demand, NULL);

            DBUG_EXECUTE (string = demand2String (current_ap_arg_demand));
            DBUG_PRINT ("ap_arg_demand: %s", string);
            DBUG_EXECUTE (string = MEMfree (string));
            DBUG_EXECUTE (string = demand2String (tmp_constant));
            DBUG_PRINT ("demand(tmp_constant): %s", string);
            DBUG_EXECUTE (string = MEMfree (string));

            if (COcompareConstants (tmp_constant, current_ap_arg_demand) != TRUE) {
                INFO_DEMAND_HAS_CHANGED (arg_info) = TRUE;
            }
            current_ap_arg_demand = COfreeConstant (current_ap_arg_demand);
            new_demand = COfreeConstant (new_demand);

            AVIS_DEMAND (ID_AVIS (EXPRS_EXPR (current_ap_args))) = tmp_constant;
            tmp_constant = NULL;
        }

        DBUG_EXECUTE_TAG ("SOSSK", string = demand2String (AVIS_DEMAND (
                                     ID_AVIS (EXPRS_EXPR (current_ap_args)))));
        DBUG_PRINT_TAG ("SOSSK", "Add demand %s to %s", string,
                        ID_NAME (EXPRS_EXPR (current_ap_args)));
        DBUG_EXECUTE_TAG ("SOSSK", string = MEMfree (string));

        /* There are ap_args left*/
        if (EXPRS_NEXT (current_ap_args) != NULL) {

            if ((ARG_NEXT (arg_node) != NULL) || (INFO_EXT_FUN (arg_info) == TRUE)) {

                INFO_ARGS (arg_info) = EXPRS_NEXT (INFO_ARGS (arg_info));

                /* if fundef is ext. funct. the same demand is used an maybe*/
                /* there exists dot-args, so use just the same argument*/

                if ((INFO_EXT_FUN (arg_info) == TRUE) && (ARG_NEXT (arg_node) == NULL)) {
                    arg_node = TRAVdo (arg_node, arg_info);
                } else {
                    ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
                }
            }
        }  /* (current_ap_args != NULL)*/
    }      /* (INFO_COPY_DEMAND(arg_info) == TRUE)*/
    else { /* Count number of arguments*/
        DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKarg %s (COUNTING)",
                        ARG_NAME (arg_node));

        INFO_NUM_ARGS (arg_info) = INFO_NUM_ARGS (arg_info) + 1;

        ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);
    }

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKarg");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKassign");

    constant *old_demand = INFO_DEMAND (arg_info);
    int num_rets = SHgetExtent (COgetShape (old_demand), 0);
    int dim = SHgetDim (COgetShape (old_demand));
    int new_shape[2] = {num_rets, 4};
    int *elems;
    int i = 0;
    int j = 0;
    int pos = 0;
#ifndef DBUG_OFF
    char *string = NULL;
#endif

    elems = (int *)MEMmalloc (num_rets * 4 * sizeof (int));
    /* Constuct the demand [0,1,2,3] for the lamda app.*/
    for (i = 0; i < num_rets; i++) {
        pos = i * 4;
        for (j = 0; j < 4; j++) {
            elems[pos + j] = j;
        }
    }

    INFO_DEMAND (arg_info) = COmakeConstantFromArray (T_int, dim, new_shape, elems);

    DBUG_PRINT ("-------------------");
    DBUG_EXECUTE (string = demand2String (INFO_DEMAND (arg_info)));
    DBUG_PRINT ("INFO_DEMAND: %s", string);
    DBUG_EXECUTE (string = MEMfree (string));

    /* Go on to the next assignment first to compute the demands*/
    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    if (INFO_DEMAND (arg_info) != NULL) {
        INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    }
    INFO_DEMAND (arg_info) = old_demand;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKassign");

    MEMfree (elems);
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node * SOSSKcond(node *arg_node, info *arg_info)
 *
 *    @brief This function handles a conditional. It therefor first traverses
 *           into the THEN and ELSE part. Afterwards it does a selection
 *           from [0, 0, 0, 3] into the demand
 *
 *    @param arg_node N_cond node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_cond node
 ******************************************************************************/

node *
SOSSKcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKcond");

    constant *old_demand = NULL;

    constant *new_demand = NULL;
    int num_rets = SHgetExtent (COgetShape (INFO_DEMAND (arg_info)), 0);
    int dim = SHgetDim (COgetShape (INFO_DEMAND (arg_info)));
    int new_shape[2] = {num_rets, 4};
    int *elems;
    int i = 0;
    int offset = 0;

    elems = (int *)MEMmalloc (num_rets * 4 * sizeof (int));
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    old_demand = INFO_DEMAND (arg_info);

    /* construct demand [0,0,0,3], which is an approximation*/
    for (i = 0; i < num_rets; i++) {
        offset = 4 * i;
        elems[offset] = 0;
        elems[offset + 1] = 0;
        elems[offset + 2] = 0;
        elems[offset + 3] = 3;
    }

    new_demand = COmakeConstantFromArray (T_int, dim, new_shape, elems);

    /* compute shape-demand*/
    INFO_DEMAND (arg_info) = doOverSelMatrix (old_demand, new_demand);

    new_demand = COfreeConstant (new_demand);

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;
    old_demand = NULL;

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKcond");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKexprs");

    constant *arg_constant = NULL;
    constant *old_demand = INFO_DEMAND (arg_info);
    bool new_demand = FALSE;
#ifndef DBUG_OFF
    char *string = NULL;
#endif

    /* If this function was called from a prf get the demand*/
    if (INFO_PRF_NAME (arg_info) > 0) {
        arg_constant = prf_shape_oracle_funtab[INFO_PRF_NAME (arg_info)](
          INFO_POS_PRF_ARG (arg_info));

        DBUG_PRINT ("--------------------");
        DBUG_EXECUTE (string = demand2String (old_demand));
        DBUG_PRINT ("old_demand: %s", string);
        DBUG_EXECUTE (string = MEMfree (string));
        DBUG_EXECUTE (string = demand2String (arg_constant));
        DBUG_PRINT ("prf_demand: %s", string);
        DBUG_EXECUTE (string = MEMfree (string));

        /* If there exists a demand, do Overselection*/
        if ((arg_constant != NULL) && (old_demand != NULL)) {
            new_demand = TRUE;

            INFO_DEMAND (arg_info) = doOverSel (old_demand, arg_constant);

            DBUG_EXECUTE (string = demand2String (INFO_DEMAND (arg_info)));
            DBUG_PRINT ("doOverSel: %s", string);
            DBUG_EXECUTE (string = MEMfree (string));
        }

        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);

        if ((INFO_DEMAND (arg_info) != NULL) && (new_demand == TRUE)) {
            INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));

            INFO_DEMAND (arg_info) = old_demand;
            old_demand = NULL;
        }

        if (EXPRS_NEXT (arg_node) != NULL) {
            INFO_POS_PRF_ARG (arg_info) = INFO_POS_PRF_ARG (arg_info) + 1;
            EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
        }
    }                                        // if(INFO_PRF_NAME(arg_info) > 0)
    else if (INFO_POS_RET (arg_info) >= 0) { /* Call from return-statement*/
        if (old_demand != NULL) {
            INFO_DEMAND (arg_info)
              = getReturnDemand (old_demand, INFO_POS_RET (arg_info));
        }
        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);
        if (INFO_DEMAND (arg_info) != NULL) {
            INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
        }
        INFO_DEMAND (arg_info) = old_demand;
        old_demand = NULL;
        if (EXPRS_NEXT (arg_node) != NULL) {
            INFO_POS_RET (arg_info) = INFO_POS_RET (arg_info) + 1;
            EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
        }
    } else {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKexprs");
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKfold(node *arg_node, info *arg_info)
 *
 *    @brief Works like genarray, just with [0, 1, 2 ,3] instead of
 *           [0, 2, 3, 3].
 *
 *    @param arg_node N_fold node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_fold node
 ******************************************************************************/

node *
SOSSKfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKfold");

    constant *old_demand = INFO_DEMAND (arg_info);
    constant *new_demand = NULL;
    int num_rets = SHgetExtent (COgetShape (old_demand), 0);
    int dim = SHgetDim (COgetShape (old_demand));
    int new_shape[2] = {num_rets, 4};
    int *elems;
    int i = 0;
    int offset = 0;

    elems = (int *)MEMmalloc (num_rets * 4 * sizeof (int));
    /* construct demand [0,1,2,3]*/
    for (i = 0; i < num_rets; i++) {
        offset = 4 * i;
        elems[offset] = 0;
        elems[offset + 1] = 1;
        elems[offset + 2] = 2;
        elems[offset + 3] = 3;
    }

    new_demand = COmakeConstantFromArray (T_int, dim, new_shape, elems);

    /* compute shape-demand*/
    INFO_DEMAND (arg_info) = doOverSelMatrix (old_demand, new_demand);

    new_demand = COfreeConstant (new_demand);

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;

    FOLD_NEXT (arg_node) = TRAVopt(FOLD_NEXT (arg_node), arg_info);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKfold");
    MEMfree (elems);
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKgenarray");

    constant *old_demand = INFO_DEMAND (arg_info);
    constant *new_demand = NULL;
    int num_rets = SHgetExtent (COgetShape (old_demand), 0);
    int dim = SHgetDim (COgetShape (old_demand));
    int new_shape[2] = {num_rets, 4};
    int *elems;
    int i = 0;
    int offset = 0;

    elems = (int *)MEMmalloc (num_rets * 4 * sizeof (int));
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
    INFO_DEMAND (arg_info) = doOverSelMatrix (old_demand, new_demand);

    new_demand = COfreeConstant (new_demand);

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;

    GENARRAY_DEFAULT (arg_node) = TRAVopt(GENARRAY_DEFAULT (arg_node), arg_info);

    GENARRAY_NEXT (arg_node) = TRAVopt(GENARRAY_NEXT (arg_node), arg_info);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKgenarray");
    MEMfree (elems);
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKmodarray(node *arg_node, info *arg_info)
 *
 *    @brief constructs the demand for the shape-argument and traverses
 *           into the different arguments of modarray
 *
 *    @param arg_node N_modarray node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_modarray node
 ******************************************************************************/

node *
SOSSKmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKmodarray");

    constant *old_demand = INFO_DEMAND (arg_info);
    constant *new_demand = NULL;
    int num_rets = SHgetExtent (COgetShape (old_demand), 0);
    int dim = SHgetDim (COgetShape (old_demand));
    int new_shape[2] = {num_rets, 4};
    int *elems;
    int i = 0;
    int offset = 0;

    elems = (int *)MEMmalloc (num_rets * 4 * sizeof (int));
    /* construct demand [0,2,3,3]*/
    for (i = 0; i < num_rets; i++) {
        offset = 4 * i;
        elems[offset] = 0;
        elems[offset + 1] = 1;
        elems[offset + 2] = 2;
        elems[offset + 3] = 3;
    }

    new_demand = COmakeConstantFromArray (T_int, dim, new_shape, elems);

    /* compute shape-demand*/
    INFO_DEMAND (arg_info) = doOverSelMatrix (old_demand, new_demand);

    new_demand = COfreeConstant (new_demand);

    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;

    MODARRAY_NEXT (arg_node) = TRAVopt(MODARRAY_NEXT (arg_node), arg_info);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKmodarray");
    MEMfree (elems);
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKgenerator");

    constant *old_demand = NULL;

    if (INFO_DEMAND (arg_info) != NULL) {
        old_demand = COcopyConstant (INFO_DEMAND (arg_info));
    }

    GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);

    if (INFO_DEMAND (arg_info) != NULL) {
        INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    }
    if (old_demand != NULL) {
        INFO_DEMAND (arg_info) = COcopyConstant (old_demand);
    }

    GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);

    if (INFO_DEMAND (arg_info) != NULL) {
        INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    }
    INFO_DEMAND (arg_info) = old_demand;

    old_demand = NULL;

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKgenerator");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKid %s", ID_NAME (arg_node));

    constant *tmp_constant = NULL;
    constant *id_demand = AVIS_DEMAND (ID_AVIS (arg_node));
#ifndef DBUG_OFF
    char *string = NULL;
#endif

    if (INFO_DEMAND (arg_info) != NULL) {
        /* If there exists a demand, compute the maximum demand of the current
         * demand and the old demand.*/
        if (id_demand != NULL) {
            DBUG_ASSERT (SHcompareShapes (COgetShape (id_demand),
                                          COgetShape (INFO_DEMAND (arg_info)))
                           == TRUE,
                         "shape(id_demand) %s not equal shape(info_demand) %s!",
                         SHshape2String (0, COgetShape (id_demand)),
                         SHshape2String (0, COgetShape (INFO_DEMAND (arg_info))));

            tmp_constant = COmax (id_demand, INFO_DEMAND (arg_info), NULL);
            DBUG_PRINT ("--------------------");
            DBUG_EXECUTE (string = demand2String (id_demand));
            DBUG_PRINT ("id_demand: %s", string);
            DBUG_EXECUTE (string = MEMfree (string));
            DBUG_EXECUTE (string = demand2String (INFO_DEMAND (arg_info)));
            DBUG_PRINT ("INFO_DEMAND: %s", string);
            DBUG_EXECUTE (string = MEMfree (string));
            DBUG_EXECUTE (string = demand2String (tmp_constant));
            DBUG_PRINT ("COmax: %s", string);
            DBUG_EXECUTE (string = MEMfree (string));
            if (COcompareConstants (tmp_constant, id_demand) != TRUE) {
                INFO_DEMAND_HAS_CHANGED (arg_info) = TRUE;
            }
            id_demand = COfreeConstant (id_demand);
            DBUG_EXECUTE_TAG ("SOSSK", string = demand2String (tmp_constant));
            DBUG_PRINT_TAG ("SOSSK", "Add %s to %s", string, ID_NAME (arg_node));
            DBUG_EXECUTE_TAG ("SOSSK", string = MEMfree (string));
            AVIS_DEMAND (ID_AVIS (arg_node)) = tmp_constant;
            tmp_constant = NULL;
        } else {
            DBUG_EXECUTE_TAG ("SOSSK", string = demand2String (INFO_DEMAND (arg_info)));
            DBUG_PRINT_TAG ("SOSSK", "Add %s to %s", string, ID_NAME (arg_node));
            DBUG_EXECUTE_TAG ("SOSSK", string = MEMfree (string));
            AVIS_DEMAND (ID_AVIS (arg_node)) = COcopyConstant (INFO_DEMAND (arg_info));
            INFO_DEMAND_HAS_CHANGED (arg_info) = TRUE;
        }
    } /* if(INFO_DEMAND(arg_info) != NULL)*/

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKid");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKids %s", IDS_NAME (arg_node));

    constant *old_demand = INFO_DEMAND (arg_info);
    constant *ids_demand = AVIS_DEMAND (IDS_AVIS (arg_node));

    /* If the demand is NULL, the variable is not used below. So demand is
     * (0, 0, 0, 0)*/
    if (ids_demand == NULL) {
        int *elems;
        int shape[2] = {INFO_NUM_RETS (arg_info), 4};
        int i = 0;
        int j = 0;
        int pos = 0;
#ifndef DBUG_OFF
        char *string = NULL;
#endif

        elems = (int *)MEMmalloc (INFO_NUM_RETS (arg_info) * 4 * sizeof (int));
        for (i = 0; i < INFO_NUM_RETS (arg_info); i++) {
            pos = i * 4;
            for (j = 0; j < 4; j++) {
                elems[pos + j] = j;
            }
        }
        AVIS_DEMAND (IDS_AVIS (arg_node))
          = COmakeConstantFromArray (T_int, 2, shape, elems);
        ids_demand = AVIS_DEMAND (IDS_AVIS (arg_node));
        DBUG_PRINT_TAG ("SOSSK", "------------------");
        DBUG_EXECUTE_TAG ("SOSSK", string = demand2String (ids_demand));
        DBUG_PRINT_TAG ("SOSSK", "Add demand %s to ids %s", string, IDS_NAME (arg_node));
        DBUG_EXECUTE_TAG ("SOSSK", string = MEMfree (string));
        MEMfree (elems);
    }

    INFO_DEMAND (arg_info) = doOverSelMatrix (old_demand, ids_demand);
    old_demand = COfreeConstant (old_demand);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKids");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKret");

    INFO_NUM_RETS (arg_info) = INFO_NUM_RETS (arg_info) + 1;

    RET_NEXT (arg_node) = TRAVopt(RET_NEXT (arg_node), arg_info);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKret");
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKreturn(node *arg_node, info *arg_info)
 *
 *    @brief This function sets a flag which indicates that the further
 *           traversal was indicated by a N_return node.
 *           It then traverses further and resets the flag afterwards.
 *
 *    @param arg_node N_return node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_return node
 ******************************************************************************/

node *
SOSSKreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKret");

    if (RETURN_EXPRS (arg_node) != NULL) {
        INFO_POS_RET (arg_info) = 0;
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        INFO_POS_RET (arg_info) = -1;
    }

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKret");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKprf");

    prf xprf;

    xprf = PRF_PRF (arg_node);

    DBUG_PRINT_TAG ("SOSSK_FCT", ">>>> enter PRF: %s", PRF_NAME (xprf));

    /* If there exists a demand-function for this prf, compute the demands*/
    if (prf_shape_oracle_funtab[xprf] != NULL) {

        INFO_PRF_NAME (arg_info) = xprf;
        INFO_POS_PRF_ARG (arg_info) = 0;

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        INFO_POS_PRF_ARG (arg_info) = -1;
        INFO_PRF_NAME (arg_info) = (prf)0;
    } else {
        DBUG_PRINT_TAG ("SOSSK", "Points to NULL!");
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_PRINT_TAG ("SOSSK_FCT", "<<<< leave PRF: %s", PRF_NAME (xprf));
    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKprf");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKfundef");

    int last_iteration_round = FUNDEF_LASTITERATIONROUND (arg_node);
    int i = 0;
    int j = 0;
    int pos = 0;

    /* Keep info*/
    info *old_info = arg_info;
    arg_info = MakeInfo ();
    INFO_ITERATION_ROUND (arg_info) = INFO_ITERATION_ROUND (old_info);

    DBUG_PRINT_TAG ("SOSSK_FCT", ">>>> Enter Function: %s", FUNDEF_NAME (arg_node));

    FUNDEF_LASTITERATIONROUND (arg_node) = INFO_ITERATION_ROUND (old_info);

    /* Count the number of return values and numbers of arguments*/
    FUNDEF_RETS (arg_node) = TRAVopt(FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

    /* If the function has no arguments, there is nothing to do*/
    if (INFO_NUM_ARGS (arg_info) != 0) {
        /* If this function has no body, the demand is known (except for wrapper
         * funs)
         */
        if (!FUNDEF_ISWRAPPERFUN (arg_node) && (FUNDEF_BODY (arg_node) == NULL)) {
            FUNDEF_FIXPOINTFOUND (arg_node) = TRUE;
            INFO_EXT_FUN (arg_info) = TRUE;
        }

        /* This is the first time in this iter. round we handel this function AND
         * the fixpoint has not been found yet. */
        if ((last_iteration_round != INFO_ITERATION_ROUND (old_info))
            && (FUNDEF_FIXPOINTFOUND (arg_node) != TRUE)) {

            DBUG_PRINT_TAG ("SOSSK", " IF fixpoint not found AND not handled yet");

            if (FUNDEF_ISWRAPPERFUN (arg_node) != TRUE) {
#ifndef DBUG_OFF
                char *string = NULL;
#endif
                int *elems;
                int shape[2] = {INFO_NUM_RETS (arg_info), 4};

                elems = (int *)MEMmalloc (INFO_NUM_RETS (arg_info) * 4 * sizeof (int));
                for (i = 0; i < INFO_NUM_RETS (arg_info); i++) {
                    pos = i * 4;
                    for (j = 0; j < 4; j++) {
                        elems[pos + j] = j;
                    }
                }
                INFO_DEMAND (arg_info) = COmakeConstantFromArray (T_int, 2, shape, elems);

                DBUG_PRINT ("---Fresh Demand---");
                DBUG_EXECUTE (string = demand2String (INFO_DEMAND (arg_info)));
                DBUG_PRINT_TAG ("DBUG_PRINT", "INFO_DEMAND: %s", string);
                DBUG_EXECUTE (string = MEMfree (string));

                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
                if (INFO_DEMAND (arg_info) != NULL) {
                    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
                }
                if (INFO_DEMAND_HAS_CHANGED (arg_info) == TRUE) {
                    FUNDEF_LASTCHANGE (arg_node) = INFO_ITERATION_ROUND (arg_info);
                    INFO_DEMAND_HAS_CHANGED (old_info) = TRUE;
                } else if (((FUNDEF_LASTCHANGE (arg_node)
                             - INFO_ITERATION_ROUND (arg_info))
                            >= 2)
                           || (INFO_AP_FOUND (arg_info) == FALSE)) {
                    FUNDEF_FIXPOINTFOUND (arg_node) = TRUE;
                    DBUG_PRINT_TAG ("SOSSK", "FIXPOINT FOUND");
                }
                MEMfree (elems);
            } else { /* Fundef is Wrapper*/
                /* Set flag, if fixpoint is not found, this will be reseted in the
                 * fold-fun*/
                FUNDEF_FIXPOINTFOUND (arg_node) = TRUE;

                FUNDEF_WRAPPERTYPE (arg_node)
                  = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (arg_node), SOSSKfundef,
                                            arg_info);

                arg_node = (node *)TYfoldFunctionInstances (FUNDEF_WRAPPERTYPE (arg_node),
                                                            (void *(*)(node *,
                                                                       void *))wrapperMax,
                                                            (void *)arg_node);
            }
        }
        /* If there are ap-args, compute demand*/
        if (INFO_ARGS (old_info) != NULL) {
            INFO_IDS (arg_info) = INFO_IDS (old_info);
            INFO_ARGS (arg_info) = INFO_ARGS (old_info);
            INFO_COPY_DEMAND (arg_info) = TRUE;

            if (last_iteration_round == INFO_ITERATION_ROUND (arg_info)) {
                INFO_ESTIMATION (arg_info) = TRUE;
            }

            FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);
        }
    }

    /* Restore old Information*/
    INFO_IDS (arg_info) = NULL;
    INFO_ARGS (arg_info) = NULL;

    if (INFO_DEMAND_HAS_CHANGED (arg_info)) {
        INFO_DEMAND_HAS_CHANGED (old_info) = TRUE;
    }

    arg_info = FreeInfo (arg_info);
    arg_info = old_info;
    old_info = NULL;

    /* If this is NOT an ap-link, follow the fundef-chain*/
    if (INFO_AP_CALL (arg_info) != TRUE) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            DBUG_PRINT_TAG ("SOSSK_FCT", "Follow FUNDEF-chain (%s -> ?)",
                            FUNDEF_NAME (arg_node));
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            DBUG_PRINT_TAG ("SOSSK_FCTR", "^^^^^ Back in Function %s",
                            FUNDEF_NAME (arg_node));
        }
    }

    DBUG_PRINT_TAG ("SOSSK_FCT", "<<<< Leave Function: %s", FUNDEF_NAME (arg_node));
    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKfundef");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKlet");

    INFO_IDS (arg_info) = LET_IDS (arg_node);

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKlet");
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SOSSKcode(node *arg_node, info *arg_info)
 *
 *    @brief Just orders the way of traversing.
 *
 *    @param arg_node N_code node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_code node
 ******************************************************************************/

node *
SOSSKcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKcode");

    constant *old_demand = COcopyConstant (INFO_DEMAND (arg_info));

    if (CODE_NEXT (arg_node) != NULL) {
        DBUG_PRINT_TAG ("SOSSK_WITH", "--> Traverse into the next CODE");
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);

        INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
        INFO_DEMAND (arg_info) = COcopyConstant (old_demand);
    }

    DBUG_PRINT_TAG ("SOSSK_WITH", "--> Traverse into the CEXPRS");
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    INFO_DEMAND (arg_info) = old_demand;
    old_demand = NULL;

    if (CODE_CBLOCK (arg_node) != NULL) {
        DBUG_PRINT_TAG ("SOSSK_WITH", "--> Traverse into the CBLOCK");
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKcode");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKpart");

    constant *old_demand = COcopyConstant (INFO_DEMAND (arg_info));

    DBUG_PRINT_TAG ("SOSSK_WITH", "--> Traverse into WITHID");
    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
    DBUG_PRINT_TAG ("SOSSK_WITH", "--> Traverse into GENERATOR");
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (INFO_DEMAND (arg_info) != NULL) {
        INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));
    }
    INFO_DEMAND (arg_info) = old_demand;
    old_demand = NULL;

    if (PART_NEXT (arg_node) != NULL) {
        DBUG_PRINT_TAG ("SOSSK_WITH", "--> Traverse to the next PART");
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKpart");
    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *SOSSKwithid(node *arg_node, info *arg_info)
 *
 *    @brief extracts the demand out of the withid. If needed, it also
 *           computes the maximum of the id demand and the vec demand
 *
 *    @param arg_node N_withid node
 *    @param arg_info INFO structure
 *
 *    @return unchanged N_withid node
 ******************************************************************************/

node *
SOSSKwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKwithid");

    constant *old_demand = COcopyConstant (INFO_DEMAND (arg_info));
    constant *ids_demand = NULL;
    constant *vec_demand = NULL;
#ifndef DBUG_OFF
    char *string = NULL;
#endif
    WITHID_VEC (arg_node) = TRAVdo (WITHID_VEC (arg_node), arg_info);

    if (WITHID_IDS (arg_node) != NULL) {
        vec_demand = INFO_DEMAND (arg_info);
        INFO_DEMAND (arg_info) = COcopyConstant (old_demand);
        WITHID_IDS (arg_node) = TRAVdo (WITHID_IDS (arg_node), arg_info);
        ids_demand = INFO_DEMAND (arg_info);

        if ((vec_demand != NULL) && (ids_demand != NULL)) {
            DBUG_PRINT ("--------------------");
            DBUG_EXECUTE (string = demand2String (vec_demand));
            DBUG_PRINT ("vec_demand: %s", string);
            DBUG_EXECUTE (string = MEMfree (string));
            DBUG_EXECUTE (string = demand2String (ids_demand));
            DBUG_PRINT ("ids_demand: %s", string);
            DBUG_EXECUTE (string = MEMfree (string));
            INFO_DEMAND (arg_info) = COmax (vec_demand, ids_demand, NULL);

            COfreeConstant (vec_demand);
            COfreeConstant (ids_demand);
        } else if (vec_demand != NULL) {
            INFO_DEMAND (arg_info) = vec_demand;
            vec_demand = NULL;
        }
    }

    old_demand = COfreeConstant (old_demand);

    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKwithid");
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
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("SOSSK_PATH", ">>> ENTER SOSSKwith");
    constant *old_demand = COcopyConstant (INFO_DEMAND (arg_info));

    DBUG_PRINT_TAG ("SOSSK_WITH", "-> Traverse into the WITHOP");
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));

    INFO_DEMAND (arg_info) = COcopyConstant (old_demand);
    DBUG_PRINT_TAG ("SOSSK_WITH", "-> Traverse into the CODE");
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    INFO_DEMAND (arg_info) = COfreeConstant (INFO_DEMAND (arg_info));

    INFO_DEMAND (arg_info) = COcopyConstant (old_demand);
    DBUG_PRINT_TAG ("SOSSK_WITH", "-> Traverse into the PART");
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    old_demand = COfreeConstant (old_demand);
    DBUG_PRINT_TAG ("SOSSK_PATH", "<<< LEAVE SOSSKwith");
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

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module,
                 "SOSSKdoSpecializationOracleSSK is intended to run on the entire tree");

    PPIinitializePVs ();

    info = MakeInfo ();

    TRAVpush (TR_sossk);

    do {
        INFO_DEMAND_HAS_CHANGED (info) = FALSE;
        INFO_ITERATION_ROUND (info) = INFO_ITERATION_ROUND (info) + 1;
        DBUG_PRINT_TAG ("SOSSK", "##### Iteration Round: %i #####",
                        INFO_ITERATION_ROUND (info));
        syntax_tree = TRAVdo (syntax_tree, info);
    } while (INFO_DEMAND_HAS_CHANGED (info) == TRUE);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
