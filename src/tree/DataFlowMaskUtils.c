/*
 *
 * $Log$
 * Revision 1.12  2000/07/12 15:15:49  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 1.11  2000/07/12 13:35:24  dkr
 * DFMDuplicateMask: NOTE replaced by DBUG_PRINT
 *
 * Revision 1.8  2000/07/04 14:36:43  jhs
 * Added DFMGetMaskBase and used it in DFMDuplicateMask
 *
 * Revision 1.7  2000/03/24 00:51:29  dkr
 * some DBUG_PRINT statements added
 *
 * Revision 1.6  2000/03/23 16:16:22  dkr
 * DFM2ReturnTypes() modified: reference flag in rettypes->attrib is
 * unset
 *
 * Revision 1.5  2000/03/23 12:33:55  dkr
 * DFM2Vardecs modified: VARDEC_ATTRIB is now set correctly
 *
 * Revision 1.4  2000/03/17 21:02:25  dkr
 * added type declarations and functios for a DFM stack
 *
 * Revision 1.3  2000/03/09 18:35:50  jhs
 * new copy routine
 *
 * Revision 1.2  2000/02/03 17:29:56  dkr
 * LUTs added
 *
 * Revision 1.1  2000/01/21 16:52:08  dkr
 * Initial revision
 *
 */

#include "tree.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "typecheck.h"

/************************************************
 *
 *  DFM Stack
 */

/*
 * type definition for DFM stack
 */

typedef struct STACK_T {
    DFMmask_t mask;
    struct STACK_T *next;
} stack_t;

typedef int (*fun_t) (DFMmask_t mask, char *id, node *decl);

/******************************************************************************
 *
 * Function:
 *   dfmstack_t GenerateDFMstack( void)
 *
 * Description:
 *   Generates a new DFMstack.
 *
 ******************************************************************************/

stack_t *
GenerateDFMstack (void)
{
    DBUG_ENTER ("GenerateDFMstack");

    DBUG_RETURN ((stack_t *)NULL);
}

/******************************************************************************
 *
 * Function:
 *   extern int IsEmptyDFMstack( DFMstack_t stack)
 *
 * Description:
 *   Checks, whether the given DFMstack is empty or not.
 *
 ******************************************************************************/

extern int
IsEmptyDFMstack (stack_t *stack)
{
    int res;

    DBUG_ENTER ("IsEmptyDFMstack");

    if (stack == NULL) {
        res = TRUE;
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   void PushDFMstack( stack_t **dfmstack, DFMmask_t mask)
 *
 * Description:
 *   Pushs a DFMmask onto the given DFMstack.
 *
 ******************************************************************************/

void
PushDFMstack (stack_t **stack, DFMmask_t mask)
{
    stack_t *new_stack;

    DBUG_ENTER ("PushDFMstack");

    new_stack = (stack_t *)Malloc (sizeof (stack_t));
    new_stack->mask = mask;
    new_stack->next = *stack;
    *stack = new_stack;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   DFMmask_t PopDFMstack( stack_t **stack)
 *
 * Description:
 *   Pops a DFMmask from the given DFMstack.
 *
 ******************************************************************************/

DFMmask_t
PopDFMstack (stack_t **stack)
{
    stack_t *old_stack;
    DFMmask_t mask;

    DBUG_ENTER ("PopDFMstack");

    DBUG_ASSERT ((!IsEmptyDFMstack (*stack)), "POP failed: stack is empty!");

    mask = (*stack)->mask;
    old_stack = *stack;
    (*stack) = (*stack)->next;
    FREE (old_stack);

    DBUG_RETURN (mask);
}

/******************************************************************************
 *
 * Function:
 *   void ForeachDFMstack( stack_t *stack, fun_t fun, char *id, node *decl)
 *
 * Description:
 *   Calls the function 'fun' for each DFMmask found in the given DFMstack.
 *
 ******************************************************************************/

void
ForeachDFMstack (stack_t *stack, fun_t fun, char *id, node *decl)
{
    stack_t *tmp;

    DBUG_ENTER ("ForeachDFMstack");

    tmp = stack;
    while (tmp != NULL) {
        fun (tmp->mask, id, decl);
        tmp = tmp->next;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void WhileDFMstack( stack_t stack, fun_t fun, char *id, node *decl)
 *
 * Description:
 *   Calls the function 'fun' for each DFMmask found in the given DFMstack
 *   until the return value of the function is >0.
 *
 ******************************************************************************/

void
WhileDFMstack (stack_t *stack, fun_t fun, char *id, node *decl)
{
    stack_t *tmp;
    int error = 0;

    DBUG_ENTER ("WhileDFMstack");

    tmp = stack;
    while ((tmp != NULL) && (!error)) {
        error |= fun (tmp->mask, id, decl);
        tmp = tmp->next;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void RemoveDFMstack( stack_t **stack)
 *
 * Description:
 *   Removes the given DFMstack.
 *
 ******************************************************************************/

void
RemoveDFMstack (stack_t **stack)
{
    DBUG_ENTER ("RemoveDFMstack");

    while ((*stack) != NULL) {
        DFMRemoveMask (PopDFMstack (stack));
    }
    (*stack) = NULL;

    DBUG_VOID_RETURN;
}

/*
 *  DFM Stack
 *
 ************************************************/

/************************************************
 *
 *  build of AST components based on a DFM
 */

/******************************************************************************
 *
 * function:
 *   types *DFM2ReturnTypes( DFMmask_t mask)
 *
 * description:
 *   Creates a types chain based on the given DFmask.
 *
 ******************************************************************************/

types *
DFM2ReturnTypes (DFMmask_t mask)
{
    node *decl;
    types *tmp;
    types *rettypes = NULL;

    DBUG_ENTER ("DFM2ReturnTypes");

    /*
     * build return types, return exprs (use SPMD_OUT).
     */
    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        tmp = rettypes;
        rettypes = DupTypes (VARDEC_OR_ARG_TYPE (decl));
        /*
         * Unfortunately the 'attrib' value is part of the type structure.
         * But an attrib value 'ST_reference' or 'ST_readonly_reference'
         *  makes no sense for a return type.
         */
        if ((rettypes->attrib == ST_reference)
            || (rettypes->attrib == ST_readonly_reference)) {
            rettypes->attrib = ST_unique;
        }

        DBUG_PRINT ("DFMU",
                    ("%-16s - attrib: %-16s, status: %-16s", TYPES_NAME (rettypes),
                     mdb_statustype[rettypes->attrib], mdb_statustype[rettypes->status]));

        TYPES_NEXT (rettypes) = tmp;
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    /*
     * CAUTION: FUNDEF_NAME is for the time being a part of FUNDEF_TYPES!!
     *          That's why we must build a void-type, when ('rettypes' == NULL).
     */
    if (rettypes == NULL) {
        rettypes = MakeType (T_void, 0, NULL, NULL, NULL);
    }

    DBUG_RETURN (rettypes);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2Vardecs( DFMmask_t mask, LUT_t lut)
 *
 * description:
 *   Creates a vardec-node chain based on the given DFmask.
 *   If (lut != NULL) the old/new declarations are inserted into the given LUT.
 *
 ******************************************************************************/

node *
DFM2Vardecs (DFMmask_t mask, LUT_t lut)
{
    node *decl, *tmp;
    node *vardecs = NULL;

    DBUG_ENTER ("DFM2Vardecs");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        if (NODE_TYPE (decl) == N_vardec) {
            tmp = vardecs;
            vardecs = DupNode (decl);
            VARDEC_NEXT (vardecs) = tmp;
        } else {
            DBUG_ASSERT ((NODE_TYPE (decl) == N_arg),
                         "mask entry is neither an arg nor a vardec.");
            vardecs = MakeVardec (StringCopy (ARG_NAME (decl)),
                                  DupTypes (ARG_TYPE (decl)), vardecs);
            VARDEC_ATTRIB (vardecs) = ARG_ATTRIB (decl);
            VARDEC_OBJDEF (vardecs) = ARG_OBJDEF (decl);
        }

        DBUG_PRINT ("DFMU",
                    ("%-16s - attrib: %-16s, status: %-16s", VARDEC_NAME (vardecs),
                     mdb_statustype[VARDEC_ATTRIB (vardecs)],
                     mdb_statustype[VARDEC_STATUS (vardecs)]));

        lut = InsertIntoLUT (lut, decl, vardecs);
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2Args( DFMmask_t mask, LUT_t lut)
 *
 * description:
 *   Creates a argument list (arg-node chain) based on the given DFmask.
 *   If (lut != NULL) the old/new declarations are inserted into the given LUT.
 *
 ******************************************************************************/

node *
DFM2Args (DFMmask_t mask, LUT_t lut)
{
    node *decl, *tmp;
    node *args = NULL;

    DBUG_ENTER ("DFM2Args");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        if (NODE_TYPE (decl) == N_arg) {
            tmp = args;
            args = DupNode (decl);
            ARG_NEXT (args) = tmp;
        } else {
            DBUG_ASSERT ((NODE_TYPE (decl) == N_vardec),
                         "mask entry is neither an arg nor a vardec.");
            args
              = MakeArg (StringCopy (VARDEC_NAME (decl)), DupTypes (VARDEC_TYPE (decl)),
                         VARDEC_STATUS (decl), VARDEC_ATTRIB (decl), args);
            ARG_OBJDEF (args) = VARDEC_OBJDEF (decl);
        }

        DBUG_PRINT ("DFMU", ("%-16s - attrib: %-16s, status: %-16s", ARG_NAME (args),
                             mdb_statustype[ARG_ATTRIB (args)],
                             mdb_statustype[ARG_STATUS (args)]));

        lut = InsertIntoLUT (lut, decl, args);
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2Exprs( DFMmask_t mask, LUT_t lut)
 *
 * description:
 *   Creates a exprs/id-node chain based on the given DFmask.
 *   If (lut != NULL) the attribute ID_VARDEC of the created IDs is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

node *
DFM2Exprs (DFMmask_t mask, LUT_t lut)
{
    node *decl, *id;
    node *exprs = NULL;

    DBUG_ENTER ("DFM2Exprs");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        id = MakeId1 (VARDEC_OR_ARG_NAME (decl));
        /*
         * ID_VARDEC and ID_OBJDEF are mapped to the same node!
         */
        ID_VARDEC (id) = SearchInLUT (lut, decl);
        ID_ATTRIB (id) = VARDEC_OR_ARG_ATTRIB (decl);
        ID_STATUS (id) = VARDEC_OR_ARG_STATUS (decl);
        exprs = MakeExprs (id, exprs);

        DBUG_PRINT ("DFMU",
                    ("%-16s - attrib: %-16s, status: %-16s", ID_NAME (id),
                     mdb_statustype[ID_ATTRIB (id)], mdb_statustype[ID_STATUS (id)]));

        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   ids *DFM2Ids( DFMmask_t mask, LUT_t lut)
 *
 * description:
 *   Creates an ids chain based on the given DFmask.
 *   If (lut != NULL) the attribute IDS_VARDEC of the created IDS is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

ids *
DFM2Ids (DFMmask_t mask, LUT_t lut)
{
    node *decl;
    ids *tmp;
    ids *_ids = NULL;

    DBUG_ENTER ("DFM2Ids");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        tmp = _ids;
        _ids = MakeIds1 (VARDEC_OR_ARG_NAME (decl));
        IDS_ATTRIB (_ids) = VARDEC_OR_ARG_ATTRIB (decl);
        IDS_STATUS (_ids) = VARDEC_OR_ARG_STATUS (decl);
        IDS_VARDEC (_ids) = SearchInLUT (lut, decl);
        IDS_NEXT (_ids) = tmp;

        DBUG_PRINT ("DFMU", ("%-16s - attrib: %-16s, status: %-16s", IDS_NAME (_ids),
                             mdb_statustype[IDS_ATTRIB (_ids)],
                             mdb_statustype[IDS_STATUS (_ids)]));

        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (_ids);
}

/*
 *  build of AST components based on a DFM
 *
 ************************************************/

/******************************************************************************
 *
 * function:
 *   DFMmask_t DFMDuplicateMask( DFMmask_t mask, DFMmask_base_t base)
 *
 * description:
 *   Creates a copy of the DFMmask mask, if mask == NULL, NULL is returned.
 *   If base != NULL: The DFMbase of the new mask is set to base.
 *   If base == NULL: The DFMbase of the new mask is set to the base of mask.
 *   The bases of orginal and copy can differ. The copy-procedure itself
 *   is based on identifiers, so different bases make no promblem, as long
 *   as they do not contain differnt names. If a unknown name is to be copied
 *   the routine fails.
 *
 *   This routine is used while duplicating functions, whereby the DFMbase
 *   of copied masks must be changed. The new mask can have other internal
 *   order of values, so we can't copy directly.
 *
 ******************************************************************************/

DFMmask_t
DFMDuplicateMask (DFMmask_t mask, DFMmask_base_t base)
{
    char *name;
    DFMmask_t new_mask;

    DBUG_ENTER ("DFMDuplicateMask");

    if (mask != NULL) {
        if ((base != NULL) && (base != DFMGetMaskBase (mask))) {
            /* copy by hand */
            new_mask = DFMGenMaskClear (base);

            /* copy each name, DFMSetMaskEntrySet will fail if name not found */
            name = DFMGetMaskEntryNameSet (mask);
            while (name != NULL) {
                DBUG_PRINT ("DFMU", ("Entry in duplicated mask: %s", name));

                DFMSetMaskEntrySet (new_mask, name, NULL);
                name = DFMGetMaskEntryNameSet (NULL);
            }
        } else {
            /* copy by native */
            new_mask = DFMGenMaskCopy (mask);
        }
    } else {
        new_mask = NULL;
    }

    DBUG_RETURN (new_mask);
}
