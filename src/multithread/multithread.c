/*
 *
 * $Log$
 * Revision 3.24  2004/11/24 20:55:18  skt
 * some namechanging
 *
 * Revision 3.23  2004/11/24 19:40:47  skt
 * SACDevCampDK 2k4
 *
 * Revision 3.22  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * [...]
 *
 * Revision 1.1  2000/01/21 11:11:38  jhs
 * Initial revision
 *
 */

/**
 * @defgroup muth Multithread
 *
 * This group contains all those files/ modules that apply support for
 * multithreaded execution for sac-code (mtmode 3)
 */

/**
 * @file multithread.c
 *
 * @brief This file iniates and guides the compilation process of the new
 *        multihread-support (mtmode 3)
 * @attention The entire process is still under development
 *
 *
 * prefix:      MUTH
 *
 */

#include "tree_basic.h"
#include "traverse.h"
#include <string.h>
#include "internal_lib.h"

#include "multithread.h"
#include "tag_executionmode.h"
#include "create_withinwith.h"
#include "propagate_executionmode.h"
#include "create_dataflowgraph.h"
#include "assignments_rearrange.h"
#include "create_cells.h"
#include "cell_growth.h"
#include "replicate_functions.h"
#include "DeadFunctionRemoval.h"
#include "consolidate_cells.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
};

/*
 * INFO macros
 *    node*    MUTH_MODULE                  (just a placeholder)
 */
#define INFO_MUTH_MODULE(n) (n->module)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_MUTH_MODULE (result) = NULL;

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
 *
 * @fn node *MUTHdoMultiThread( node *syntax_tree)
 *
 *   This function starts the process of building the *new* support for
 *   multithread. Throughout this process arg_info points to an N_info which
 *   is generated here.
 *
 * @param syntax_tree The whole syntax-tree of the SAC-program
 * @pre syntax_tree is flat and in SSA-form
 * @return The whole syntax-tree, prepared to be compiled as a multithreaded
 *         application
 *****************************************************************************/
node *
MUTHdoMultiThread (node *syntax_tree)
{
    trav_t traversaltable;

    info *arg_info;

    DBUG_ENTER ("MUTHdoMultiThread");

    arg_info = MakeInfo ();

    TRAVpush (TR_muth);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_muth), "Popped incorrect traversal table");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *MUTHfundef(node *arg_node, info *arg_info)
 *
 *   @brief accomplish the initialization of the function-executionmode
 *
 *   @param arg_node a N_fundef
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
MUTHfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MUTHfundef");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "MUTHfundef expects a N_fundef as arg_node");

    /* initialize the executionmode*/
    FUNDEF_EXECMODE (arg_node) = MUTH_ANY;
    /* initialize the companion */
    FUNDEF_COMPANION (arg_node) = NULL;

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into function-body"));
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from function-body"));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into function-next"));
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from function-next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MUTHassign(node *arg_node, info *arg_info)
 *
 *   @brief accomplish the initialization of the assignment-executionmode
 *
 *   @param arg_node a N_assign
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
MUTHassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MUTHassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign),
                 "MUTHassign expects a N_assign as 1st argument");

    ASSIGN_EXECMODE (arg_node) = MUTH_ANY;
    if (ASSIGN_INSTR (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into assignment-instruction"));
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from assignment-instruction"));
    }
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into assignment-next"));
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from assignment-next"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MUTHmodule(node *arg_node, info *arg_info)
 *
 *   @brief executes the first parts of the EX-/ST-/MT-multithreading
 *
 *   @param arg_node a N_modul
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
MUTHmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MUTHmodule");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "MUTHmodule expects a N_module as 1st argument");

    /*
     *  --- initializing (init) ---
     */
    DBUG_PRINT ("MUTH", ("begin initializing"));
    if (MODULE_IMPORTS (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into module-imports"));
        MODULE_IMPORTS (arg_node) = TRAVdo (MODULE_IMPORTS (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from module-imports"));
    }
    if (MODULE_OBJS (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into module-objs"));
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from module-objs"));
    }
    if (MODULE_TYPES (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into module-types"));
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from module-types"));
    }
    if (MODULE_FUNS (arg_node) != NULL) {
        DBUG_PRINT ("MUTH", ("trav into module-funs"));
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
        DBUG_PRINT ("MUTH", ("trav from module-funs"));
    }
    global.executionmodes_available = TRUE;
    DBUG_PRINT ("MUTH", ("end initializing"));

    if ((global.break_after == PH_multithread)
        && (strcmp ("init", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- TEMdoTagExecutionmode (tem) ---
     */
    DBUG_PRINT ("MUTH", ("begin TEMdoTagExecutionmode"));

    arg_node = TEMdoTagExecutionmode (arg_node);

    DBUG_PRINT ("MUTH", ("end TEMdoTagExecutionmode"));

    if ((global.break_after == PH_multithread)
        && (strcmp ("tem", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- CRWIWdoCreateWithinwith (crwiw) ---
     */
    DBUG_PRINT ("MUTH", ("begin CRWIWdoCreateWithinwith"));

    arg_node = CRWIWdoCreateWithinwith (arg_node);

    DBUG_PRINT ("MUTH", ("end CRWIWdoCreateWihitinwith"));

    if ((global.break_after == PH_multithread)
        && (strcmp ("crwiw", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- PEMdoPropagateExecutionmode (pem) ---
     */
    DBUG_PRINT ("MUTH", ("begin PEMdoPropagateExecutionmode"));

    arg_node = PEMdoPropagateExecutionmode (arg_node);

    DBUG_PRINT ("MUTH", ("end PEMdoPropagateExecutionmode"));

    if ((global.break_after == PH_multithread)
        && (strcmp ("pem", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- CDFGdoCreateDataflowgraph (cdfg) ---
     */
    DBUG_PRINT ("MUTH", ("begin CDFGdoCreateDataflowgraph"));

    arg_node = CDFGdoCreateDataflowgraph (arg_node);

    DBUG_PRINT ("MUTH", ("end CDFGdoCreateDataflowgraph"));

    if ((global.break_after == PH_multithread)
        && (strcmp ("cdfg", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- ASMRAdoAssignmentsRearange (asmra) ---
     */
    DBUG_PRINT ("MUTH", ("begin ASMRAdoAssignmentsRearrange"));

    arg_node = ASMRAdoAssignmentsRearrange (arg_node);

    DBUG_PRINT ("MUTH", ("end ASMRAdoAssignmentsRearrange"));

    if ((global.break_after == PH_multithread)
        && (strcmp ("asmra", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- CRECEdoCreateCells (crece) ---
     */
    DBUG_PRINT ("MUTH", ("begin CRECEdoCreateCells"));

    arg_node = CRECEdoCreateCells (arg_node);

    DBUG_PRINT ("MUTH", ("end CRECEDoCreateCells"));
    if ((global.break_after == PH_multithread)
        && (strcmp ("crece", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     *  --- CEGROdoCellGrowth (cegro) ---
     */
    DBUG_PRINT ("MUTH", ("begin CEGROdoCellGrowth"));

    arg_node = CEGROdoCellGrowth (arg_node);

    DBUG_PRINT ("MUTH", ("end CEGROdoCellGrowth"));
    if ((global.break_after == PH_multithread)
        && (strcmp ("cegro", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     * --- REPFUNdoReplicateFunctions (repfun) ---
     */
    DBUG_PRINT ("MUTH", ("begin REPFUNdoReplicateFunctions"));

    arg_node = REPFUNdoReplicateFunctions (arg_node);

    /* extra-call of DFR to remove superflous functions */
    arg_node = DFRdoDeadFunctionRemoval (arg_node);

    DBUG_PRINT ("MUTH", ("end REPFUNdoReplicateFunctions"));
    if ((global.break_after == PH_multithread)
        && (strcmp ("repfun", global.break_specifier) == 0)) {
        goto cont;
    }

    /*
     * --- CONDELdoConsolidateCells (concel) ---
     */
    DBUG_PRINT ("MUTH", ("begin CONCELdoConsolidateCells"));

    arg_node = CONCELdoConsolidateCells (arg_node);

    DBUG_PRINT ("MUTH", ("end CONCELdoConsolidateCells"));
    if ((global.break_after == PH_multithread)
        && (strcmp ("concel", global.break_specifier) == 0)) {
        goto cont;
    }

    global.executionmodes_available = FALSE;

cont:

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */
