/*
 *
 * $Log$
 * Revision 3.4  2002/10/18 13:29:17  sbs
 * ID_ATTRIB replaced by accesses to the FLAGS of N_id
 *
 * Revision 3.3  2002/02/20 14:59:31  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.2  2001/03/22 13:28:54  dkr
 * InsertIntoLUT renamed into InsertIntoLUT_P
 * SearchInLUT renamed into SearchInLUT_P
 *
 * Revision 3.1  2000/11/20 18:03:05  sacbase
 * new release made
 *
 * Revision 1.11  2000/10/24 11:51:58  dkr
 * MakeTypes renamed into MakeTypes1
 *
 * Revision 1.10  2000/07/21 11:29:59  jhs
 * FUNDEF_MT2USE, FUNDEF_MT2DEF, FUNDEF_IDENTIFIER are set while
 * lifting mt-blocks.
 *
 * Revision 1.9  2000/07/12 15:15:00  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 1.8  2000/07/11 15:41:31  jhs
 * Added lut-entries of args while lifting.
 *
 * Revision 1.7  2000/07/05 14:21:35  dkr
 * DupTree.h is included only once now :-)
 *
 * Revision 1.6  2000/07/04 14:56:17  jhs
 * Changed lifting.
 *
 * Revision 1.5  2000/04/20 11:37:11  jhs
 * ID_VARDECs of return arguments are set correct now.
 *
 * Revision 1.4  2000/04/18 14:02:01  jhs
 * Added DBUGs.
 *
 * Revision 1.3  2000/03/30 15:11:29  jhs
 * fixed lifting
 * .,
 *
 * Revision 1.2  2000/03/29 16:08:51  jhs
 * Duplication of lifted function added.
 *
 * Revision 1.1  2000/03/29 09:23:28  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   blocks_lift.c
 *
 * prefix: BLKLI
 *
 * description
 *   ####
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"
#include "my_debug.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "multithread_lib.h"
#include "LookUpTable.h"

#include "internal_lib.h"

/******************************************************************************
 *
 * function:
 *   node *BlocksLift( node *arg_node, node *arg_info)
 *
 * description:
 *   Init the lifting of blocks, where each mt-block will be lifted into a
 *   function.
 *
 ******************************************************************************/
node *
BlocksLift (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("BlocksLift");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), ("wrong type of arg_node"));

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)
        && (FUNDEF_ATTRIB (arg_node) != ST_call_rep)) {
        old_tab = act_tab;
        act_tab = blkli_tab;

        /* push arg_info */

        arg_node = Trav (arg_node, arg_info);

        /* pop arg_info */

        act_tab = old_tab;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupNodeVardec( node *vardec, node *next)
 *
 * description:
 *   Duplicates a N_vardec and concatenates next to it and returns the copy.
 *
 ******************************************************************************/
node *
DupNodeVardec (node *vardec, node *next)
{
    node *result;

    DBUG_ENTER ("DupNodeVardec");

    result = DupNode (vardec);
    VARDEC_NEXT (result) = next;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *DupNodeArgToVardec( node *arg, node *next)
 *
 * description:
 *   Takes a N_arg "casts" it into an N_vardec concenates next to it and
 *   returns the copy.
 *
 ******************************************************************************/
node *
DupNodeArgToVardec (node *arg, node *next)
{
    node *result;

    DBUG_ENTER ("DupNodeArgToVardec");

    result = DupNode (arg);
    NODE_TYPE (result) = N_vardec;

    ARG_NEXT (result) = next;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *BLKLIfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses the body only.
 *
 * attention:
 *   DO NOT TRAVERSE NEXT HERE!!!
 *
 ******************************************************************************/
node *
BLKLIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("BLKLIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *BLKLImt(node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
BLKLImt (node *arg_node, node *arg_info)
{
    node *new_fundef; /* new constructed, lifted function         */
    node *fundef;     /* function the N_mt is in, stays unchanged */
    node *new_block;
    node *new_vardecs;
    node *old_vardecs;
    node *old_args;
    node *new_arg;  /* new arg to hanged into chain */
    node *new_args; /* the new chain of args */
    node *vardec;
    types *rettypes;
    node *retexprs;
    types *new_rettype;
    node *new_retexpr;
    LUT_t lut;
    DFMmask_base_t new_base;

    DBUG_ENTER ("BLKLImt");
    DBUG_PRINT ("BLKLI", ("begin"));

    fundef = INFO_MUTH_FUNDEF (arg_info);

    lut = GenerateLUT ();

    /*
     *  we have to copy all the vardecs and args to the new function ...
     *  MT_USEMASK does not say anything about local variales *<:(
     */
    new_vardecs = NULL;
    old_vardecs = FUNDEF_VARDEC (fundef);
    old_args = FUNDEF_ARGS (fundef);
    /*
     *  ... first we copy each vardec, that is easy ...
     */
    while (old_vardecs != NULL) {
        if (!DFMTestMaskEntry (MT_USEMASK (arg_node), NULL, old_vardecs)) {
            new_vardecs = DupNodeVardec (old_vardecs, new_vardecs);
            lut = InsertIntoLUT_P (lut, old_vardecs, new_vardecs);
        }
        old_vardecs = VARDEC_NEXT (old_vardecs);
    }

    /*
     *  ... now we have to copy the args, that is tricky, because we need
     *  to make N_vardec's out of the N_args ...
     *  ... so we hide the main part in DupNodeArgToVardec( old_arg) ...
     */
    while (old_args != NULL) {
        if (!DFMTestMaskEntry (MT_USEMASK (arg_node), NULL, old_args)) {
            new_vardecs = DupNodeArgToVardec (old_args, new_vardecs);
            lut = InsertIntoLUT_P (lut, old_args, new_vardecs);
        }
        old_args = ARG_NEXT (old_args);
    }
    /*
     *  ... now delete unnecessary vardecs
     */
    /* to be done #### */

    /*
     *  Build args by MT_USEMASK
     */
    /*
     * build formal parameters (MT_USEMASK).
     */
    new_args = NULL;
    vardec = DFMGetMaskEntryDeclSet (MT_USEMASK (arg_node));
    while (vardec != NULL) {
        if (NODE_TYPE (vardec) == N_arg) {
            new_arg = DupNode (vardec);
            ARG_NEXT (new_arg) = new_args;
        } else {
            new_arg = MakeArg (StringCopy (ARG_NAME (vardec)),
                               DupAllTypes (VARDEC_TYPE (vardec)), ST_regular, ST_regular,
                               new_args);
            /* refcnt and varno also need corrections */
            ARG_REFCNT (new_arg) = GET_STD_REFCNT (VARDEC, vardec);
            ARG_VARNO (new_arg) = VARDEC_VARNO (vardec);

            DBUG_PRINT ("BLKLI", ("inserted arg %s", ARG_NAME (vardec)));
        }

        lut = InsertIntoLUT_P (lut, vardec, new_arg);

        new_args = new_arg;
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    /*
     * build return types, return exprs (use MT_DEFMASK).
     */
    rettypes = NULL;
    retexprs = NULL;
    vardec = DFMGetMaskEntryDeclSet (MT_DEFMASK (arg_node));
    while (vardec != NULL) {
        if (NODE_TYPE (vardec) == N_arg) {
            new_rettype = DupAllTypes (ARG_TYPE (vardec));

            new_retexpr
              = MakeExprs (MakeId (StringCopy (ARG_NAME (vardec)), NULL, ST_regular),
                           retexprs);
            SET_FLAG (ID, EXPRS_EXPR (new_retexpr), IS_GLOBAL, FALSE);
            SET_FLAG (ID, EXPRS_EXPR (new_retexpr), IS_REFERENCE, FALSE);

            ID_REFCNT (EXPRS_EXPR (new_retexpr)) = GET_ZERO_REFCNT (ARG, vardec);
            ID_VARDEC (EXPRS_EXPR (new_retexpr)) = vardec;
        } else {
            new_rettype = DupAllTypes (VARDEC_TYPE (vardec));

            new_retexpr
              = MakeExprs (MakeId (StringCopy (VARDEC_NAME (vardec)), NULL, ST_regular),
                           retexprs);
            SET_FLAG (ID, EXPRS_EXPR (new_retexpr), IS_GLOBAL, FALSE);
            SET_FLAG (ID, EXPRS_EXPR (new_retexpr), IS_REFERENCE, FALSE);
            ID_REFCNT (EXPRS_EXPR (new_retexpr)) = GET_ZERO_REFCNT (VARDEC, vardec);
            ID_VARDEC (EXPRS_EXPR (new_retexpr)) = vardec;
        }

        /*
         * This is only a dummy value for the vardec-pointer.
         * After we have build the new fundef node, we must traverse it to correct
         * the vardec-pointers of all id's.
         */
        /*  ID_VARDEC( EXPRS_EXPR( new_retexpr)) = NULL; */

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

    new_base = DFMGenMaskBase (new_args, new_vardecs);

    lut = InsertIntoLUT_P (lut, FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)), new_base);

    new_block = DupNodeLUT (MT_REGION (arg_node), lut);
    BLOCK_VARDEC (new_block) = new_vardecs;

    lut = RemoveLUT (lut);

    new_fundef = MakeFundef (TmpVarName (FUNDEF_NAME (fundef)), "_MTLIFT", rettypes,
                             new_args /* args */, new_block /* body */, NULL /* next */);

    FUNDEF_STATUS (new_fundef) = ST_spmdfun;
    FUNDEF_ATTRIB (new_fundef) = ST_call_mtlift;
    FUNDEF_MT2USE (new_fundef) = DFMDuplicateMask (MT_USEMASK (arg_node), new_base);
    FUNDEF_MT2DEF (new_fundef) = DFMDuplicateMask (MT_DEFMASK (arg_node), new_base);
    FUNDEF_IDENTIFIER (new_fundef) = MT_IDENTIFIER (arg_node);
    FUNDEF_LIFTEDFROM (new_fundef) = fundef;
    FUNDEF_VARNO (new_fundef) = FUNDEF_VARNO (fundef); /* ####???? */

    FUNDEF_RETURN (new_fundef) = MakeReturn (retexprs);
    AppendAssign (BLOCK_INSTR (new_block), MakeAssign (FUNDEF_RETURN (new_fundef), NULL));

    FUNDEF_DFM_BASE (new_fundef) = new_base;

    FUNDEF_NEXT (new_fundef) = FUNDEF_NEXT (fundef);
    FUNDEF_NEXT (fundef) = new_fundef;

    MT_FUNDEF (arg_node) = new_fundef;

    DBUG_PRINT ("BLKLI", ("end"));
    DBUG_RETURN (arg_node);
}
