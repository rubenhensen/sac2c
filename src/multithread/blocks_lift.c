/*
 *
 * $Log$
 * Revision 1.1  2000/03/29 09:23:28  jhs
 * Initial revision
 *
 *
 */

/* #### filecomment */

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
#include "DupTree.h"
#include "DataFlowMask.h"
#include "multithread_lib.h"

#include "internal_lib.h"

node *
DupNodeArgToVardec (node *arg, node *next)
{
    node *result;

    DBUG_ENTER ("DupTreeArgToVardec");

    DBUG_ASSERT (0, ("not implemented yet"));

    DBUG_RETURN (result);
}

/* #### */
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

    DBUG_ENTER ("BLKLImt");

    fundef = INFO_MUTH_FUNDEF (arg_info);

    /*
     *  we have to copy all the veardecs and args to the new function ...
     *  MT_USEMASK does not say anything about local variales *<:(
     */
    new_vardecs = NULL;
    old_vardecs = FUNDEF_VARDEC (fundef);
    old_args = FUNDEF_ARGS (fundef);
    /*
     *  ... first we copy the vardecs, that is easy ...
     */
    new_vardecs = DupTree (old_vardecs, NULL);
    /*
     *  ... now we have to copy the args, that is tricky,
     *  so we hide the main part in DupNodeArgToVardec( old_arg) ...
     */
    while (old_args != NULL) {
        new_vardecs = DupNodeArgToVardec (old_args, new_vardecs);
        old_args = ARG_NEXT (old_args);
    }
    /*
     *  ... now delete unnecessary vardecs
     */
    /* to be done #### */

    new_block = DupNode (MT_REGION (arg_node));
    BLOCK_VARDEC (new_block) = new_vardecs;

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
                               DuplicateTypes (VARDEC_TYPE (vardec), 1), ST_regular,
                               ST_regular, new_args);
            /* refcnt and varno also need corrections */
            ARG_REFCNT (new_arg) = GET_STD_REFCNT (VARDEC, vardec);
            ARG_VARNO (new_arg) = VARDEC_VARNO (vardec);

            DBUG_PRINT ("BLKLI", ("inserted arg %s", ARG_NAME (vardec)));
        }
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

    new_fundef = MakeFundef (TmpVarName (FUNDEF_NAME (fundef)), "_MTLIFT", rettypes,
                             new_args /* args */, new_block /* body */, NULL /* next */);

    FUNDEF_STATUS (new_fundef) = ST_mtlift;
    FUNDEF_LIFTEDFROM (new_fundef) = fundef;
    FUNDEF_VARNO (new_fundef) = FUNDEF_VARNO (fundef); /* ####???? */

    FUNDEF_RETURN (new_fundef) = MakeReturn (retexprs);
    AppendAssign (BLOCK_INSTR (new_block), MakeAssign (FUNDEF_RETURN (new_fundef), NULL));

    FUNDEF_DFM_BASE (new_fundef)
      = DFMGenMaskBase (FUNDEF_ARGS (new_fundef), FUNDEF_VARDEC (new_fundef));

    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (new_fundef) = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = new_fundef;
    } else {
        FUNDEF_NEXT (fundef) = new_fundef;
    }

    MT_FUNDEF (arg_node) = new_fundef;

    DBUG_RETURN (arg_node);
}
