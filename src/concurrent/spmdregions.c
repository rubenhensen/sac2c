/*
 *
 * $Log$
 * Revision 1.30  1998/06/06 18:30:59  dkr
 * added SYNC_WITH_PTRS
 *
 * Revision 1.29  1998/06/03 15:00:20  cg
 * bugs fixed in lifting spmd function.
 * Now, a link back to the original function is stored for each spmd function.
 *
 * Revision 1.28  1998/05/21 15:08:08  dkr
 * fixed a bug in SPMDLiftFundef
 *
 * Revision 1.27  1998/05/21 13:32:17  dkr
 * for lifted spmd-funs FUNDEF_DFM_BASE is created now
 * added SPMDLiftNwith2 (to correct the DFMs)
 *
 * Revision 1.26  1998/05/19 08:54:21  cg
 * new functions for retrieving variables from masks provided by
 * DataFlowMask.c utilized.
 *
 * Revision 1.23  1998/05/15 15:44:38  cg
 * minor bugs removed in the construction of the spmd-function
 *
 * Revision 1.22  1998/05/12 22:40:36  dkr
 * removed attributes for N_sync
 *
 * Revision 1.21  1998/05/12 14:54:09  dkr
 * fixed a bug in SPMDLift...:
 *   in lifted regions the vardec-pointers of ids are now corrected, too.
 *
 * Revision 1.19  1998/05/12 12:21:43  dkr
 * added SPMDLiftNwithid, SPMDLiftLet, SPMDLiftIds
 *
 * Revision 1.18  1998/05/07 10:15:31  dkr
 * uses DFMasks now
 *
 * Revision 1.17  1998/05/02 18:14:50  dkr
 * lifting of SPMD-funs temporary deactivated
 *
 * Revision 1.16  1998/05/02 17:46:27  dkr
 * added new attributes to N_spmd, N_sync
 *
 * Revision 1.6  1998/04/24 12:16:11  dkr
 * added SPMD_LOCAL
 *
 * Revision 1.5  1998/04/24 01:15:58  dkr
 * added N_sync
 *
 * Revision 1.4  1998/04/20 02:39:33  dkr
 * includes now tree.h
 *
 * Revision 1.3  1998/04/19 23:18:00  dkr
 * changed comments
 *
 * Revision 1.2  1998/04/17 19:16:20  dkr
 * lifting of spmd-fun is performed in precompile now
 *
 * Revision 1.1  1998/04/17 17:22:07  dkr
 * Initial revision
 *
 *
 */

#include "tree.h"
#include "free.h"

#include "internal_lib.h"
#include "traverse.h"
#include "convert.h"

#include "DupTree.h"
#include "DataFlowMask.h"
#include "optimize.h"
#include "refcount.h"
#include "typecheck.h"
#include "dbug.h"

#include "spmdregions.h"

/*
 * returns 0 for refcounting-objects and -1 otherwise
 */
#define GET_ZERO_REFCNT(prefix, node) ((prefix##_REFCNT (node) >= 0) ? 0 : -1)

/*
 * returns 1 for refcounting-objects and -1 otherwise
 */
#define GET_STD_REFCNT(prefix, node) ((prefix##_REFCNT (node) >= 0) ? 1 : -1)

/******************************************************************************
 * build SPMD-regions
 */

/******************************************************************************
 *
 * function:
 *   node *SPMDInitAssign( node *arg_node, node *arg_info)
 *
 * description:
 *   Generates a SPMD-region for each first level with-loop.
 *   Then in SPMD_IN/OUT/INOUT/LOCAL the in/out/inout/local-vars of the
 *   SPMD-region are stored.
 *
 ******************************************************************************/

node *
SPMDInitAssign (node *arg_node, node *arg_info)
{
    node *with, *spmd_let, *spmd;

    DBUG_ENTER ("SPMDInitAssign");

    spmd_let = ASSIGN_INSTR (arg_node);

    /* contains the current assignment a with-loop?? */
    if ((NODE_TYPE (spmd_let) == N_let)
        && (NODE_TYPE (LET_EXPR (spmd_let)) == N_Nwith2)) {

        with = LET_EXPR (spmd_let);

        /*
         * current assignment contains a with-loop
         *  -> create a SPMD-region containing the current assignment only
         *      and insert it into the syntaxtree.
         */
        spmd = MakeSpmd (MakeBlock (MakeAssign (spmd_let, NULL), NULL));
        ASSIGN_INSTR (arg_node) = spmd;

        /*
         * get INOUT_IDS, IN/INOUT/OUT/LOCAL from the N_Nwith2 node.
         */

        SPMD_INOUT_IDS (spmd) = DupOneIds (LET_IDS (spmd_let), NULL);

        SPMD_IN (spmd) = DFMGenMaskCopy (NWITH2_IN (with));
        SPMD_INOUT (spmd) = DFMGenMaskCopy (NWITH2_INOUT (with));
        SPMD_OUT (spmd) = DFMGenMaskCopy (NWITH2_OUT (with));
        SPMD_LOCAL (spmd) = DFMGenMaskCopy (NWITH2_LOCAL (with));

        /*
         * we only traverse the following assignments to prevent nested
         *  SPMD-regions
         */
    } else {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 * optimize SPMD-regions
 */

/* not yet implemented :( */

/******************************************************************************
 * lift SPMD-regions
 */

/******************************************************************************
 *
 * function:
 *   node *GetVardec( char *name, node *fundef)
 *
 * description:
 *   returns a pointer to the vardec of var with name 'name'
 *
 ******************************************************************************/

node *
GetVardec (char *name, node *fundef)
{
    node *tmp, *vardec = NULL;

    DBUG_ENTER ("GetVardec");

    FOREACH_VARDEC_AND_ARG (fundef, tmp,
                            if (strcmp (VARDEC_OR_ARG_NAME (tmp), name) == 0) {
                                vardec = tmp;
                            }) /* FOREACH_VARDEC_AND_ARG */

    DBUG_RETURN (vardec);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLiftFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   fills 'INFO_SPMD_FUNDEF(info_node)' with the current fundef
 *    --- needed for creation of name for spmd-fun in 'SPMDLiftSpmd'.
 *
 ******************************************************************************/

node *
SPMDLiftFundef (node *arg_node, node *arg_info)
{
    funptr *old_tab;

    DBUG_ENTER ("SPMDLiftFundef");

    /*
     * save current fundef in INFO_SPMD_FUNDEF
     */
    INFO_SPMD_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /****************************************************************************
     * build and optimize sync-regions in SMPD-fun
     */

    if (FUNDEF_STATUS (arg_node) == ST_spmdfun) {

        old_tab = act_tab;

        /*
         * set flag: next N_sync node is the first one in SPMD-region
         */
        INFO_SPMD_FIRST (arg_info) = 1;
        act_tab = syncinit_tab; /* first traversal */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        act_tab = syncopt_tab; /* second traversal */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        act_tab = old_tab;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLiftSpmd( node *arg_node, *arg_info)
 *
 * description:
 *   lifts a SPMD-region into a function.
 *
 * remarks:
 *   - 'INFO_SPMD_FUNDEF( arg_info)' points to the current fundef-node.
 *   - 'INFO_SPMD_FIRST( arg_info)' shows, weather the next sync-region would
 *     be the first one of the SPMD_region or not.
 *
 ******************************************************************************/

node *
SPMDLiftSpmd (node *arg_node, node *arg_info)
{
    node *vardec, *fundef, *new_fundef, *body;
    node *new_vardec, *last_vardec;
    node *fargs, *new_farg;
    node *retexprs, *new_retexpr;
    types *rettypes, *new_rettype;

    DBUG_ENTER (" SPMDLiftSpmd");

    fundef = INFO_SPMD_FUNDEF (arg_info);

    /****************************************************************************
     * build fundef for this spmd region
     */

    /*
     * generate body of SPMD-function
     */
    body = DupTree (SPMD_REGION (arg_node), NULL);

    /*
     * insert vardecs of SPMD_OUT/LOCAL-vars into body
     */
    last_vardec = NULL;
    vardec = DFMGetMaskEntryDeclSet (SPMD_OUT (arg_node));
    while (vardec != NULL) {
        if (NODE_TYPE (vardec) == N_vardec) {
            new_vardec = DupNode (vardec);
            VARDEC_NEXT (new_vardec) = last_vardec;
        } else {
            new_vardec = MakeVardec (StringCopy (ARG_NAME (vardec)),
                                     DuplicateTypes (ARG_TYPE (vardec), 1), last_vardec);
            VARDEC_REFCNT (new_vardec) = ARG_REFCNT (vardec);
        }
        last_vardec = new_vardec;
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    vardec = DFMGetMaskEntryDeclSet (SPMD_LOCAL (arg_node));
    while (vardec != NULL) {
        if (NODE_TYPE (vardec) == N_vardec) {
            new_vardec = DupNode (vardec);
            VARDEC_NEXT (new_vardec) = last_vardec;
        } else {
            new_vardec = MakeVardec (StringCopy (ARG_NAME (vardec)),
                                     DuplicateTypes (ARG_TYPE (vardec), 1), last_vardec);
            VARDEC_REFCNT (new_vardec) = ARG_REFCNT (vardec);
        }
        last_vardec = new_vardec;
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
            new_farg = MakeArg (StringCopy (ARG_NAME (vardec)),
                                DuplicateTypes (VARDEC_TYPE (vardec), 1), ST_regular,
                                ST_regular, fargs);
            ARG_REFCNT (new_farg) = GET_STD_REFCNT (VARDEC, vardec);
        }
        fargs = new_farg;
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    vardec = DFMGetMaskEntryDeclSet (SPMD_INOUT (arg_node));
    while (vardec != NULL) {
        if (NODE_TYPE (vardec) == N_arg) {
            new_farg = DupNode (vardec);
            ARG_NEXT (new_farg) = fargs;
        } else {
            new_farg = MakeArg (StringCopy (ARG_NAME (vardec)),
                                DuplicateTypes (VARDEC_TYPE (vardec), 1), ST_regular,
                                ST_spmd_inout, fargs);
            ARG_REFCNT (new_farg) = GET_STD_REFCNT (VARDEC, vardec);
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
            new_rettype = DuplicateTypes (ARG_TYPE (vardec), 1);

            new_retexpr
              = MakeExprs (MakeId (StringCopy (ARG_NAME (vardec)), NULL, ST_regular),
                           retexprs);
            ID_ATTRIB (EXPRS_EXPR (new_retexpr)) = ST_regular;
            ID_REFCNT (EXPRS_EXPR (new_retexpr)) = GET_ZERO_REFCNT (ARG, vardec);
        } else {
            new_rettype = DuplicateTypes (VARDEC_TYPE (vardec), 1);

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
        rettypes = MakeType (T_void, 0, NULL, NULL, NULL);
    }

    new_fundef = MakeFundef (TmpVarName (FUNDEF_NAME (fundef)), "_SPMD", rettypes, fargs,
                             body, NULL);
    FUNDEF_STATUS (new_fundef) = ST_spmdfun;
    FUNDEF_LIFTEDFROM (new_fundef) = fundef;

    SPMD_LIFTED_FROM (arg_node) = StringCopy (FUNDEF_NAME (fundef));
    SPMD_FUNNAME (arg_node) = StringCopy (FUNDEF_NAME (new_fundef));

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
#if 01
    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (new_fundef) = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = new_fundef;
    } else {
        FUNDEF_NEXT (fundef) = new_fundef;
    }
#endif

    /*
     * build fundef for this spmd region
     ****************************************************************************/

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLiftId( node *arg_node, node *arg_info)
 *
 * description:
 *   Corrects the vardec-pointer of N_id nodes in SPMD-funs.
 *
 * remarks:
 *   INFO_SPMD_FUNDEF( arg_info) points to the current fundef-node.
 *
 ******************************************************************************/

node *
SPMDLiftId (node *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("SPMDLiftId");

    fundef = INFO_SPMD_FUNDEF (arg_info);

    if (FUNDEF_STATUS (fundef) == ST_spmdfun) {

        /*
         * we are inside a body of a SPMD-fun
         *   -> correct the pointer to the vardec (ID_VARDEC)
         */

        ID_VARDEC (arg_node) = GetVardec (ID_NAME (arg_node), fundef);
        DBUG_ASSERT ((ID_VARDEC (arg_node) != NULL), "vardec not found");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLiftLet( node *arg_node, node *arg_info)
 *
 * description:
 *   Corrects the vardec-pointers of 'LET_IDS( arg_node)' in SPMD-funs
 *    and traverses the let-expr.
 *
 ******************************************************************************/

node *
SPMDLiftLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDLiftLet");

    LET_IDS (arg_node) = SPMDLiftIds (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLiftNwithid( node *arg_node, node *arg_info)
 *
 * description:
 *   Corrects the vardec-pointers of the with-ids in SPMD-funs.
 *
 ******************************************************************************/

node *
SPMDLiftNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDLiftNwithid");

    NWITHID_IDS (arg_node) = SPMDLiftIds (NWITHID_IDS (arg_node), arg_info);
    NWITHID_VEC (arg_node) = SPMDLiftIds (NWITHID_VEC (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLiftNwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   Generates new DFMasks in NWITH2_IN/INOUT/OUT/LOCAL.
 *
 ******************************************************************************/

node *
SPMDLiftNwith2 (node *arg_node, node *arg_info)
{
    node *vardec;
    DFMmask_t in, inout, out, local;

    DBUG_ENTER ("SPMDLiftNwith2");

    /*
     * traverse sons
     */
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    /*
     * gnerate new DFMasks
     */

    in = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_SPMD_FUNDEF (arg_info)));
    inout = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_SPMD_FUNDEF (arg_info)));
    out = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_SPMD_FUNDEF (arg_info)));
    local = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_SPMD_FUNDEF (arg_info)));

    FOREACH_VARDEC_AND_ARG (INFO_SPMD_FUNDEF (arg_info), vardec, {
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

/******************************************************************************
 *
 * function:
 *   ids *SPMDLiftIds( ids *arg_node, node *arg_info)
 *
 * description:
 *   Corrects the vardec-pointers of ids.
 *
 * remarks:
 *   INFO_SPMD_FUNDEF( arg_info) points to the current fundef-node.
 *
 ******************************************************************************/

ids *
SPMDLiftIds (ids *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("SPMDLiftIds");

    fundef = INFO_SPMD_FUNDEF (arg_info);

    if (FUNDEF_STATUS (fundef) == ST_spmdfun) {

        /*
         * we are inside a body of a SPMD-fun
         *   -> correct the pointers to the vardec
         */

        IDS_VARDEC (arg_node) = GetVardec (IDS_NAME (arg_node), fundef);
        DBUG_ASSERT ((IDS_VARDEC (arg_node) != NULL), "vardec not found");

        if (IDS_NEXT (arg_node) != NULL) {
            IDS_NEXT (arg_node) = SPMDLiftIds (IDS_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 * build sync-regions
 */

/******************************************************************************
 *
 * function:
 *   node *SYNCInitAssign( node *arg_node, node *arg_info)
 *
 * description:
 *   Generates a sync-region for each first level with-loop.
 *   Then in SYNC_IN/OUT/INOUT/LOCAL the in/out/inout/local-vars of the
 *   sync-region are stored.
 *
 * remarks:
 *   INFO_SPMD_FIRST( arg_info) shows, weather the next sync-region would be
 *   the first one of the SPMD_region or not.
 *
 ******************************************************************************/

node *
SYNCInitAssign (node *arg_node, node *arg_info)
{
    node *with, *sync_let, *sync;
    ids *with_ids;

    DBUG_ENTER ("SYNCInitAssign");

    sync_let = ASSIGN_INSTR (arg_node);

    /* contains the current assignment a with-loop?? */
    if ((NODE_TYPE (sync_let) == N_let)
        && (NODE_TYPE (LET_EXPR (sync_let)) == N_Nwith2)) {

        with_ids = LET_IDS (sync_let);
        with = LET_EXPR (sync_let);

        /*
         * current assignment contains a with-loop
         *  -> create a SYNC-region containing the current assignment only
         *      and insert it into the syntaxtree.
         */
        sync = MakeSync (MakeBlock (MakeAssign (sync_let, NULL), NULL),
                         INFO_SPMD_FIRST (arg_info));
        ASSIGN_INSTR (arg_node) = sync;

        /*
         * get IN/INOUT/OUT/LOCAL from the N_Nwith2 node.
         */
        SYNC_IN (sync) = DFMGenMaskCopy (NWITH2_IN (with));
        SYNC_INOUT (sync) = DFMGenMaskCopy (NWITH2_INOUT (with));
        SYNC_OUT (sync) = DFMGenMaskCopy (NWITH2_OUT (with));
        SYNC_LOCAL (sync) = DFMGenMaskCopy (NWITH2_LOCAL (with));

        /*
         * store pointer to with-loop assignment in SYNC_WITH_PTRS
         */
        SYNC_WITH_PTRS (sync) = MakeExprs (sync_let, NULL);

        /*
         * unset flag: next N_sync node is not the first one in SPMD-region
         */
        INFO_SPMD_FIRST (arg_info) = 0;

        /*
         * we only traverse the following assignments to prevent nested
         *  sync-regions
         */
    } else {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 * optimize sync-regions
 */

/* not yet implemented :( */

/******************************************************************************
 *
 * function:
 *   node *SpmdRegions( node *syntax_tree)
 *
 * description:
 *   In a first traversal we generate the initial SPMD-regions
 *   (Every with-loop-assignment is put into a SPMD-region):
 *     A = with ( ... )       ->>      SPMD {
 *                                       A = with ( ... )
 *                                     }
 *   In a second traversal we optimize the SPMD-regions:
 *     SPMD {                          SPMD {
 *       A = with ( ... )                A = with ( ... )
 *     }                      ->>        B = with ( ... )
 *     SPMD {                          }
 *       B = with ( ... )
 *     }
 *   (not yet implemented :(
 *   In a last traversal we ...
 *     ... lift SPMD-regions to a function, ...
 *     ... build and optimize sync-regions in the lifted function.
 *         (here we traverse every lifted function two times ...)
 *
 ******************************************************************************/

node *
SpmdRegions (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("SpmdRegions");

    info = MakeInfo ();

    act_tab = spmdinit_tab; /* first traversal */
    syntax_tree = Trav (syntax_tree, info);

    act_tab = spmdopt_tab; /* second traversal */
    syntax_tree = Trav (syntax_tree, info);

    act_tab = spmdlift_tab; /* third traversal */
    syntax_tree = Trav (syntax_tree, info);

    FREE (info);

    DBUG_RETURN (syntax_tree);
}
