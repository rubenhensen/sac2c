/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:02:30  sacbase
 * new release made
 *
 * Revision 2.10  2000/10/31 23:19:34  dkr
 * Trav: NWITH2_CODE might be NULL
 *
 * Revision 2.9  2000/10/24 11:51:31  dkr
 * MakeTypes renamed into MakeTypes1
 *
 * Revision 2.8  2000/07/12 15:15:06  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 2.7  2000/06/23 15:13:19  dkr
 * signature of DupTree changed
 *
 * Revision 2.6  2000/01/25 13:42:57  dkr
 * function GetVardec moved to tree_compound.h and renamed to
 * FindVardec_Name
 *
 * Revision 2.5  1999/08/27 11:57:37  jhs
 * Added copying of varnos (fundef and vardecs) while lifting the
 * spmd-blocks to spmd-functions.
 *
 * Revision 2.4  1999/08/09 11:32:20  jhs
 * Cleaned up info-macros for concurrent-phase.
 *
 * Revision 2.3  1999/07/30 13:47:41  jhs
 * Clean sweep, deleted unused parts.
 *
 * Revision 2.2  1999/06/25 15:36:33  jhs
 * Checked these in just to provide compileabilty.
 *
 * Revision 2.1  1999/02/23 12:44:14  sacbase
 * new release made
 *
 * Revision 1.2  1998/06/23 12:56:32  cg
 * added handling of new attribute NWITH2_MT
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   spmd_lift.c
 *
 * prefix: SPMDL
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   copy spmd-blocks to dedicated functions, the so-called spmd-functions.
 *   These are needed to actually execute spmd-blocks non-sequentially.
 *
 *   The newly generated function is inserted behind the original function.
 *   This allows to traverse it in a second traversal by spmd_lift_tab in
 *   order to adjust back references and data flow masks.
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "typecheck.h"
#include "internal_lib.h"
#include "concurrent_lib.h"

/******************************************************************************
 *
 * function:
 *   ids *SPMDLids( ids *arg_node, node *arg_info)
 *
 * description:
 *   Corrects the vardec-pointers of ids.
 *
 * remarks:
 *   INFO_CONC_FUNDEF( arg_info) points to the current fundef-node.
 *
 ******************************************************************************/

ids *
SPMDLids (ids *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("SPMDLids");

    fundef = INFO_CONC_FUNDEF (arg_info);

    if (FUNDEF_STATUS (fundef) == ST_spmdfun) {

        /*
         * we are inside a body of a SPMD-fun
         *   -> correct the pointers to the vardec
         */

        IDS_VARDEC (arg_node) = FindVardec_Name (IDS_NAME (arg_node), fundef);
        DBUG_ASSERT ((IDS_VARDEC (arg_node) != NULL), "vardec not found");

        if (IDS_NEXT (arg_node) != NULL) {
            IDS_NEXT (arg_node) = SPMDLids (IDS_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLspmd( node *arg_node, *arg_info)
 *
 * description:
 *   lifts a SPMD-region into a function.
 *
 * remarks:
 *   - 'INFO_CONC_FUNDEF( arg_info)' points to the current fundef-node.
 *
 ******************************************************************************/

node *
SPMDLspmd (node *arg_node, node *arg_info)
{
    node *vardec, *fundef, *new_fundef, *body;
    node *new_vardec, *last_vardec;
    node *fargs, *new_farg;
    node *retexprs, *new_retexpr;
    types *rettypes, *new_rettype;

    DBUG_ENTER (" SPMDLspmd");

    fundef = INFO_CONC_FUNDEF (arg_info);

    /****************************************************************************
     * build fundef for this spmd region
     */

    /*
     * generate body of SPMD-function
     */
    body = DupTree (SPMD_REGION (arg_node));

    /*
     * insert vardecs of SPMD_OUT/LOCAL-vars into body
     */
    last_vardec = NULL;
    vardec = DFMGetMaskEntryDeclSet (SPMD_OUT (arg_node));
    while (vardec != NULL) {
        /* reduce outs by ins */
        if (!(DFMTestMaskEntry (SPMD_IN (arg_node), NULL, vardec))) {
            if (NODE_TYPE (vardec) == N_vardec) {
                new_vardec = DupNode (vardec);
                VARDEC_NEXT (new_vardec) = last_vardec;

                DBUG_PRINT ("SPMDL", ("inserted vardec out %s", VARDEC_NAME (vardec)));
            } else {
                new_vardec = MakeVardec (StringCopy (ARG_NAME (vardec)),
                                         DupTypes (ARG_TYPE (vardec)), last_vardec);
                VARDEC_REFCNT (new_vardec) = ARG_REFCNT (vardec);

                DBUG_PRINT ("SPMDL", ("inserted arg out %s", ARG_NAME (vardec)));
            }
            last_vardec = new_vardec;
        }
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    vardec = DFMGetMaskEntryDeclSet (SPMD_LOCAL (arg_node));
    while (vardec != NULL) {
        if (NODE_TYPE (vardec) == N_vardec) {
            new_vardec = DupNode (vardec);
            VARDEC_NEXT (new_vardec) = last_vardec;
        } else {
            new_vardec = MakeVardec (StringCopy (ARG_NAME (vardec)),
                                     DupTypes (ARG_TYPE (vardec)), last_vardec);
            VARDEC_REFCNT (new_vardec) = ARG_REFCNT (vardec);
        }
        last_vardec = new_vardec;
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }
    BLOCK_VARDEC (body) = last_vardec;

    vardec = DFMGetMaskEntryDeclSet (SPMD_SHARED (arg_node));
    while (vardec != NULL) {
        /* reduce shareds by ins and outs */
        if ((!(DFMTestMaskEntry (SPMD_IN (arg_node), NULL, vardec)))
            && (!(DFMTestMaskEntry (SPMD_OUT (arg_node), NULL, vardec)))) {
            if (NODE_TYPE (vardec) == N_vardec) {
                new_vardec = DupNode (vardec);
                VARDEC_NEXT (new_vardec) = last_vardec;

                DBUG_PRINT ("SPMDL", ("inserted vardec shared %s", VARDEC_NAME (vardec)));
            } else {
                new_vardec = MakeVardec (StringCopy (ARG_NAME (vardec)),
                                         DupTypes (ARG_TYPE (vardec)), last_vardec);
                VARDEC_REFCNT (new_vardec) = ARG_REFCNT (vardec);

                DBUG_PRINT ("SPMDL", ("inserted arg shared %s", ARG_NAME (vardec)));
            }
            last_vardec = new_vardec;
        }
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }
    BLOCK_VARDEC (body) = last_vardec;

    /*
     * build formal parameters (SPMD_IN/INOUT).
     */
    fargs = NULL;
    vardec = DFMGetMaskEntryDeclSet (SPMD_IN (arg_node));
    while (vardec != NULL) {
        if (NODE_TYPE (vardec) == N_arg) {
            new_farg = DupNode (vardec);
            ARG_NEXT (new_farg) = fargs;
        } else {
            new_farg
              = MakeArg (StringCopy (ARG_NAME (vardec)), DupTypes (VARDEC_TYPE (vardec)),
                         ST_regular, ST_regular, fargs);
            /* refcnt and varno also need corrections */
            ARG_REFCNT (new_farg) = GET_STD_REFCNT (VARDEC, vardec);
            ARG_VARNO (new_farg) = VARDEC_VARNO (vardec);

            DBUG_PRINT ("SPMDL", ("inserted arg %s", ARG_NAME (vardec)));
        }
        fargs = new_farg;
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    /*
     * build return types, return exprs (use SPMD_OUT).
     */
    rettypes = NULL;
    retexprs = NULL;
    vardec = DFMGetMaskEntryDeclSet (SPMD_OUT (arg_node));
    while (vardec != NULL) {
        if (NODE_TYPE (vardec) == N_arg) {
            new_rettype = DupTypes (ARG_TYPE (vardec));

            new_retexpr
              = MakeExprs (MakeId (StringCopy (ARG_NAME (vardec)), NULL, ST_regular),
                           retexprs);
            ID_ATTRIB (EXPRS_EXPR (new_retexpr)) = ST_regular;
            ID_REFCNT (EXPRS_EXPR (new_retexpr)) = GET_ZERO_REFCNT (ARG, vardec);
        } else {
            new_rettype = DupTypes (VARDEC_TYPE (vardec));

            new_retexpr
              = MakeExprs (MakeId (StringCopy (VARDEC_NAME (vardec)), NULL, ST_regular),
                           retexprs);
            ID_ATTRIB (EXPRS_EXPR (new_retexpr)) = ST_regular;
            ID_REFCNT (EXPRS_EXPR (new_retexpr)) = GET_ZERO_REFCNT (VARDEC, vardec);
        }

        /*
         * This is only a dummy value for the vardec-pointer.
         * After we have build the new fundef node, we must traverse it to correct
         * the vardec-pointers of all id's.
         */
        ID_VARDEC (EXPRS_EXPR (new_retexpr)) = NULL;

        TYPES_NEXT (new_rettype) = rettypes;

        rettypes = new_rettype;
        retexprs = new_retexpr;

        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    /*
     * CAUTION: FUNDEF_NAME is for the time being a part of FUNDEF_TYPES!!
     *          That's why we must build a void-type, when ('rettypes' == NULL).
     */
    if (rettypes == NULL) {
        rettypes = MakeTypes1 (T_void);
    }

    new_fundef = MakeFundef (TmpVarName (FUNDEF_NAME (fundef)), "_SPMD", rettypes, fargs,
                             body, NULL);

    FUNDEF_STATUS (new_fundef) = ST_spmdfun;
    FUNDEF_LIFTEDFROM (new_fundef) = fundef;
    FUNDEF_VARNO (new_fundef) = FUNDEF_VARNO (fundef);

    SPMD_FUNDEF (arg_node) = new_fundef;

    /*
     * append return expressions to body of SPMD-function
     */
    FUNDEF_RETURN (new_fundef) = MakeReturn (retexprs);
    AppendAssign (BLOCK_INSTR (body), MakeAssign (FUNDEF_RETURN (new_fundef), NULL));

    /*
     * generate DFMaskBase for the new fundef
     */
    FUNDEF_DFM_BASE (new_fundef)
      = DFMGenMaskBase (FUNDEF_ARGS (new_fundef), FUNDEF_VARDEC (new_fundef));

    /*
     * insert SPMD-function into fundef-chain of modul
     *
     * CAUTION: we must insert the SPMD-fundef *behind* the current fundef,
     *          because it must be traversed to correct the vardec-pointers of
     *          all id's and to generate new DFMasks!!
     */

    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (new_fundef) = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = new_fundef;
    } else {
        FUNDEF_NEXT (fundef) = new_fundef;
    }

    /*
     * build fundef for this spmd region
     ****************************************************************************/

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLid( node *arg_node, node *arg_info)
 *
 * description:
 *   Corrects the vardec-pointer of N_id nodes in SPMD-funs.
 *
 * remarks:
 *   INFO_CONC_FUNDEF( arg_info) points to the current fundef-node.
 *
 ******************************************************************************/

node *
SPMDLid (node *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("SPMDLid");

    fundef = INFO_CONC_FUNDEF (arg_info);

    if (FUNDEF_STATUS (fundef) == ST_spmdfun) {

        /*
         * we are inside a body of a SPMD-fun
         *   -> correct the pointer to the vardec (ID_VARDEC)
         */

        ID_VARDEC (arg_node) = FindVardec_Name (ID_NAME (arg_node), fundef);
        DBUG_ASSERT ((ID_VARDEC (arg_node) != NULL), "vardec not found");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLlet( node *arg_node, node *arg_info)
 *
 * description:
 *   Corrects the vardec-pointers of 'LET_IDS( arg_node)' in SPMD-funs
 *    and traverses the let-expr.
 *
 ******************************************************************************/

node *
SPMDLlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDLlet");

    LET_IDS (arg_node) = SPMDLids (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLnwithid( node *arg_node, node *arg_info)
 *
 * description:
 *   Corrects the vardec-pointers of the with-ids in SPMD-funs.
 *
 ******************************************************************************/

node *
SPMDLnwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDLnwithid");

    NWITHID_IDS (arg_node) = SPMDLids (NWITHID_IDS (arg_node), arg_info);
    NWITHID_VEC (arg_node) = SPMDLids (NWITHID_VEC (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLnwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   Generates new DFMasks in NWITH2_IN/INOUT/OUT/LOCAL.
 *
 * remark:
 *   During this phase each with-loop is marked as being multi-threaded
 *   or not. A with-loop is multi-threaded iff it is situated on the top level
 *   of an spmd-function. This information is used during code generation
 *   in order to produce slightly different ICMs.
 *
 ******************************************************************************/

node *
SPMDLnwith2 (node *arg_node, node *arg_info)
{
    node *vardec;
    DFMmask_t in, inout, out, local;

    DBUG_ENTER ("SPMDLnwith2");

    /*
     * mark with-loop as being multi-threaded or not depending on arg_info
     */

    NWITH2_MT (arg_node) = INFO_SPMDL_MT (arg_info);

    /*
     * traverse sons
     */
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    INFO_SPMDL_MT (arg_info) = 0;

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    INFO_SPMDL_MT (arg_info) = NWITH2_MT (arg_node);
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    /*
     * gnerate new DFMasks
     */

    in = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_CONC_FUNDEF (arg_info)));
    inout = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_CONC_FUNDEF (arg_info)));
    out = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_CONC_FUNDEF (arg_info)));
    local = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_CONC_FUNDEF (arg_info)));

    FOREACH_VARDEC_AND_ARG (INFO_CONC_FUNDEF (arg_info), vardec, {
        if (DFMTestMaskEntry (NWITH2_IN (arg_node), VARDEC_OR_ARG_NAME (vardec), NULL)) {
            DFMSetMaskEntrySet (in, VARDEC_OR_ARG_NAME (vardec), NULL);
        }
        if (DFMTestMaskEntry (NWITH2_INOUT (arg_node), VARDEC_OR_ARG_NAME (vardec),
                              NULL)) {
            DFMSetMaskEntrySet (inout, VARDEC_OR_ARG_NAME (vardec), NULL);
        }
        if (DFMTestMaskEntry (NWITH2_OUT (arg_node), VARDEC_OR_ARG_NAME (vardec), NULL)) {
            DFMSetMaskEntrySet (out, VARDEC_OR_ARG_NAME (vardec), NULL);
        }
        if (DFMTestMaskEntry (NWITH2_LOCAL (arg_node), VARDEC_OR_ARG_NAME (vardec),
                              NULL)) {
            DFMSetMaskEntrySet (local, VARDEC_OR_ARG_NAME (vardec), NULL);
        }
    }) /* FOREACH_VARDEC_OR_ARG */

    NWITH2_IN (arg_node) = DFMRemoveMask (NWITH2_IN (arg_node));
    NWITH2_IN (arg_node) = in;
    NWITH2_INOUT (arg_node) = DFMRemoveMask (NWITH2_INOUT (arg_node));
    NWITH2_INOUT (arg_node) = inout;
    NWITH2_OUT (arg_node) = DFMRemoveMask (NWITH2_OUT (arg_node));
    NWITH2_OUT (arg_node) = out;
    NWITH2_LOCAL (arg_node) = DFMRemoveMask (NWITH2_LOCAL (arg_node));
    NWITH2_LOCAL (arg_node) = local;

    DBUG_RETURN (arg_node);
}
