/*
 *
 * $Log$
 * Revision 3.13  2004/11/17 09:03:35  ktr
 * added support for AVIS nodes.
 *
 * Revision 3.12  2004/09/29 16:55:48  sah
 * added lots of DBUG_PRINT statements and switched DFM2ProductType to
 * a recursive implementation avoiding copying types
 *
 * Revision 3.11  2004/09/27 12:29:02  sah
 * DFM2FunctionType and DFM2ProductType
 * handle empty types (prior to ti)
 * correct now
 *
 * Revision 3.10  2004/09/27 10:40:26  sah
 * Added DFM2ProductType and DFM2FunctionType
 * both are needed by lac2fun to generate the
 * ntype function signature for fresh generated
 * LaC-funs
 *
 * Revision 3.9  2002/10/18 13:48:58  sbs
 * several flag settings for freshly made N_id nodes inserted.
 *
 * Revision 3.8  2002/08/13 13:46:40  dkr
 * SearchInLUT_PP used instead of SearchInLUT_P
 *
 * Revision 3.7  2002/02/22 14:30:57  dkr
 * DFM2ReturnTypes: workaround for FUNDEF_NAME as a part of TYPES no
 * longer needed
 *
 * Revision 3.6  2002/02/20 14:37:23  dkr
 * function DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.5  2001/05/17 11:39:08  dkr
 * MALLOC FREE aliminated
 *
 * Revision 3.4  2001/03/22 20:03:14  dkr
 * include of tree.h eliminated
 *
 * Revision 3.3  2001/03/22 13:29:53  dkr
 * InsertIntoLUT renamed into InsertIntoLUT_P
 * SearchInLUT renamed into SearchInLUT_P
 *
 * Revision 3.2  2001/02/14 14:37:56  dkr
 * DFM2...(): STATUS and ATTRIB are set correctly now
 *
 * Revision 3.1  2000/11/20 18:03:17  sacbase
 * new release made
 *
 * Revision 1.15  2000/10/24 11:56:11  dkr
 * MakeTypes renamed into MakeTypes1
 *
 * Revision 1.14  2000/10/23 11:33:48  dkr
 * MakeIds1 renamed into MakeIds_Copy
 *
 * Revision 1.13  2000/10/23 10:27:19  dkr
 * MakeId1 replaced by MakeId_Copy
 *
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

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
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
    old_stack = Free (old_stack);

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
        rettypes = DupAllTypes (VARDEC_OR_ARG_TYPE (decl));

        /*
         * VARDEC_OR_ARG_ATTRIB == 'ST_was_reference'
         *   -> TYPES_STATUS = 'ST_artificial'
         */
        if (VARDEC_OR_ARG_ATTRIB (decl) == ST_was_reference) {
            TYPES_STATUS (rettypes) = ST_artificial;
#ifndef DBUG_OFF
            if (TYPES_NAME (rettypes) != NULL) {
                DBUG_PRINT ("DFMU", ("TYPES_STATUS[ %s ] := ST_artificial !!!",
                                     TYPES_NAME (rettypes)));
            } else {
                DBUG_PRINT ("DFMU", ("TYPES_STATUS[ (null) ] := ST_artificial !!!"));
            }
#endif
        } else {
            TYPES_STATUS (rettypes) = VARDEC_OR_ARG_STATUS (decl);
#ifndef DBUG_OFF
            if (TYPES_NAME (rettypes) != NULL) {
                DBUG_PRINT ("DFMU", ("TYPES_STATUS[ %s ] == %s", TYPES_NAME (rettypes),
                                     mdb_statustype[TYPES_STATUS (rettypes)]));
            } else {
                DBUG_PRINT ("DFMU", ("TYPES_STATUS[ (null) ] == %s",
                                     mdb_statustype[TYPES_STATUS (rettypes)]));
            }
#endif
        }

        TYPES_NEXT (rettypes) = tmp;
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    /*
     * we must build a void-type if ('rettypes' == NULL) is hold
     */
    if (rettypes == NULL) {
        rettypes = MakeTypes1 (T_void);
    }

    DBUG_RETURN (rettypes);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *DFM2ProductType( DFMmask_t mask)
 *
 * @brief recursive helper function for creating a product type
 *
 * @param decl DFM iterator on vardecs/args
 * @param pos  position in DFM iterator
 *
 ******************************************************************************/
static ntype *
DFM2ProductTypeRec (node *decl, int pos)
{
    ntype *result;

    DBUG_ENTER ("DFM2ProductTypeRec");

    if (decl == NULL) {
        if (pos == 0) {
            result = NULL;
        } else {
            result = TYMakeEmptyProductType (pos);
        }
    } else {
        ntype *ptype = AVIS_TYPE (VARDEC_OR_ARG_AVIS (decl));

        if (ptype != NULL) {
            result = DFM2ProductTypeRec (DFMGetMaskEntryDeclSet (NULL), pos + 1);
        } else {
            /**
             * We have found a return value without a ntype attached to it.
             */

            DBUG_PRINT ("DFMU", ("Found return value with missing ntype"));

            result = NULL;
        }

        if (result != NULL) {
            TYSetProductMember (result, pos, TYCopyType (ptype));
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *DFM2ProductType( DFMmask_t mask)
 *
 * @brief Creates the product type of all types attached to to given mask
 *
 * @param mask in or out mask of a function
 *
 ******************************************************************************/
ntype *
DFM2ProductType (DFMmask_t mask)
{
    node *decl;
    ntype *result = NULL;

    DBUG_ENTER ("DFM2ProductType");

    decl = DFMGetMaskEntryDeclSet (mask);

    result = DFM2ProductTypeRec (decl, 0);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *DFM2FunctionType( DFMMask_t in, DFMMask_t out, node *fundef)
 *
 * @brief Creates the type of the function given as arguments
 *
 * @param in in mask of the given function
 * @param out out mask of the given function
 * @param fundef fundef node to process
 *
 ******************************************************************************/
ntype *
DFM2FunctionType (DFMmask_t in, DFMmask_t out, node *fundef)
{
    ntype *result;
    node *decl;

    DBUG_ENTER ("DFM2FunctionType");

    DBUG_PRINT ("DFMU", ("Creating ntype for function `%s'", FUNDEF_NAME (fundef)));

    result = DFM2ProductType (out);

    if (result != NULL) {
        decl = DFMGetMaskEntryDeclSet (in);

        while (decl != NULL) {
            ntype *tmp = AVIS_TYPE (VARDEC_OR_ARG_AVIS (decl));

            if (tmp != NULL) {
                result = TYMakeFunType (TYCopyType (tmp), result, fundef);
            } else {
                /**
                 * We have found an argument without a ntype! This may happen
                 * when using the old typechecker and the argument is a T_dots
                 * which cannot be transformed into an ntype. Therefor the
                 * function (containing the T_dots) cannot be given a proper
                 * function type.
                 *
                 * For the time beeing, we just return a NULL pointer!
                 */
                if (result != NULL) {
                    result = TYFreeType (result);
                }

                DBUG_PRINT ("DFMU", ("Missing Argument type for function `%s'",
                                     FUNDEF_NAME (fundef)));

                break;
            }

            decl = DFMGetMaskEntryDeclSet (NULL);
        }
    } else {
        /**
         * For some reason we were not able to create a return type.
         * Maybe one of the results has a T_dots type assigned by
         * the old typechecker, so there simply is no ntype type for
         * it. In this case there is no valid function ntype as well.
         *
         * So, again we just return a NULL pointer
         */

        DBUG_PRINT ("DFMU", ("Unable to create return type for function `%s'",
                             FUNDEF_NAME (fundef)));
    }

    DBUG_PRINT ("DFMU",
                ("Finished creating ntype for function `%s'", FUNDEF_NAME (fundef)));

    DBUG_RETURN (result);
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
                                  DupAllTypes (ARG_TYPE (decl)), vardecs);
            VARDEC_ATTRIB (vardecs) = ARG_ATTRIB (decl);
            VARDEC_OBJDEF (vardecs) = ARG_OBJDEF (decl);
        }

        DBUG_PRINT ("DFMU", ("VARDEC_ATTRIB/STATUS[ %s ] == %s/%s", VARDEC_NAME (vardecs),
                             mdb_statustype[VARDEC_ATTRIB (vardecs)],
                             mdb_statustype[VARDEC_STATUS (vardecs)]));

        lut = InsertIntoLUT_P (lut, decl, vardecs);
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
            args = MakeArg (StringCopy (VARDEC_NAME (decl)),
                            DupAllTypes (VARDEC_TYPE (decl)), VARDEC_STATUS (decl),
                            VARDEC_ATTRIB (decl), args);
            ARG_OBJDEF (args) = VARDEC_OBJDEF (decl);
        }

        DBUG_PRINT ("DFMU", ("ARG_ATTRIB/STATUS[ %s ] == %s/%s", ARG_NAME (args),
                             mdb_statustype[ARG_ATTRIB (args)],
                             mdb_statustype[ARG_STATUS (args)]));

        lut = InsertIntoLUT_P (lut, decl, args);
        if (VARDEC_AVIS (decl) != NULL) {
            if (AVIS_TYPE (VARDEC_AVIS (decl)) != NULL) {
                AVIS_TYPE (args) = TYCopyType (AVIS_TYPE (VARDEC_AVIS (decl)));
            }
            lut = InsertIntoLUT_P (lut, VARDEC_AVIS (decl), ARG_AVIS (args));
        }
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2ReturnExprs( DFMmask_t mask, LUT_t lut)
 *
 * description:
 *   Creates a exprs/id-node chain for RETURN_EXPRS based on the given DFmask.
 *   If (lut != NULL) the attribute ID_VARDEC of the created IDs is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

node *
DFM2ReturnExprs (DFMmask_t mask, LUT_t lut)
{
    node *decl, *id;
    node *exprs = NULL;

    DBUG_ENTER ("DFM2ReturnExprs");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        id = MakeId_Copy (VARDEC_OR_ARG_NAME (decl));
        /*
         * ID_VARDEC and ID_OBJDEF are mapped to the same node!
         */
        ID_VARDEC (id) = SearchInLUT_PP (lut, decl);
        ID_AVIS (id) = VARDEC_AVIS (ID_VARDEC (id));
        SET_FLAG (ID, id, IS_GLOBAL, (NODE_TYPE (ID_VARDEC (id)) == N_objdef));
        SET_FLAG (ID, id, IS_REFERENCE, FALSE);

        /*
         * VARDEC_OR_ARG_ATTRIB == 'ST_was_reference'
         *   -> ID_STATUS = 'ST_artificial'
         */
        if (VARDEC_OR_ARG_ATTRIB (decl) == ST_was_reference) {
            /*
             * who does use this ????
             *
             *    IDS_ATTRIB( IDS_ID(id)) = ST_unique;
             */
            ID_STATUS (id) = ST_artificial;
            DBUG_PRINT ("DFMU", ("ID_STATUS[ return( %s) ] :="
                                 " ST_artificial !!!",
                                 ID_NAME (id)));
        } else {
            SET_FLAG (ID, id, IS_GLOBAL, (VARDEC_OR_ARG_ATTRIB (decl) == ST_global));
            SET_FLAG (ID, id, IS_REFERENCE,
                      ((VARDEC_OR_ARG_ATTRIB (decl) == ST_reference)
                       || (VARDEC_OR_ARG_ATTRIB (decl) == ST_readonly_reference)));
            SET_FLAG (ID, id, IS_READ_ONLY,
                      (VARDEC_OR_ARG_ATTRIB (decl) == ST_readonly_reference));

            ID_STATUS (id) = VARDEC_OR_ARG_STATUS (decl);

            DBUG_PRINT ("DFMU", ("ID_FLAGS[ return( %s) ] %s%s%s", ID_NAME (id),
                                 FLAG2STRING (ID, id, IS_GLOBAL),
                                 FLAG2STRING (ID, id, IS_REFERENCE),
                                 FLAG2STRING (ID, id, IS_READ_ONLY)));
            DBUG_PRINT ("DFMU", ("ID_STATUS[ return( %s) ] == %s", ID_NAME (id),
                                 mdb_statustype[ID_STATUS (id)]));
        }
        exprs = MakeExprs (id, exprs);

        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2ApArgs( DFMmask_t mask, LUT_t lut)
 *
 * description:
 *   Creates a exprs/id-node chain for AP/PRF_ARGS based on the given DFmask.
 *   If (lut != NULL) the attribute ID_VARDEC of the created IDs is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

node *
DFM2ApArgs (DFMmask_t mask, LUT_t lut)
{
    node *decl, *id;
    node *exprs = NULL;

    DBUG_ENTER ("DFM2ApArgs");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        id = MakeId_Copy (VARDEC_OR_ARG_NAME (decl));
        /*
         * ID_VARDEC and ID_OBJDEF are mapped to the same node!
         */
        ID_VARDEC (id) = SearchInLUT_PP (lut, decl);
        ID_AVIS (id) = VARDEC_AVIS (ID_VARDEC (id));

        SET_FLAG (ID, id, IS_GLOBAL, (NODE_TYPE (ID_VARDEC (id)) == N_objdef));
        SET_FLAG (ID, id, IS_REFERENCE,
                  ((VARDEC_OR_ARG_ATTRIB (decl) == ST_reference)
                   || (VARDEC_OR_ARG_ATTRIB (decl) == ST_readonly_reference)));
        SET_FLAG (ID, id, IS_READ_ONLY,
                  (VARDEC_OR_ARG_ATTRIB (decl) == ST_readonly_reference));

        ID_STATUS (id) = VARDEC_OR_ARG_STATUS (decl);

        DBUG_PRINT ("DFMU", ("ID_STATUS[ fun( %s) ] == %s", ID_NAME (id),
                             mdb_statustype[ID_STATUS (id)]));

        exprs = MakeExprs (id, exprs);

        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   ids *DFM2LetIds( DFMmask_t mask, LUT_t lut)
 *
 * description:
 *   Creates an ids chain based on the given DFmask.
 *   If (lut != NULL) the attribute IDS_VARDEC of the created IDS is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

ids *
DFM2LetIds (DFMmask_t mask, LUT_t lut)
{
    node *decl;
    ids *tmp;
    ids *_ids = NULL;

    DBUG_ENTER ("DFM2LetIds");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        tmp = _ids;
        _ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (decl));
        IDS_VARDEC (_ids) = SearchInLUT_PP (lut, decl);
        IDS_AVIS (_ids) = VARDEC_AVIS (IDS_VARDEC (_ids));
        IDS_NEXT (_ids) = tmp;

        /*
         * VARDEC_OR_ARG_ATTRIB == 'ST_was_reference'
         *   -> ID_STATUS = 'ST_artificial'
         * All left hand side ids with attrib 'ST_was_reference' must have the status
         *  'ST_artificial'
         */
        if (VARDEC_OR_ARG_ATTRIB (decl) == ST_was_reference) {
            IDS_ATTRIB (_ids) = ST_unique;
            IDS_STATUS (_ids) = ST_artificial;

            DBUG_PRINT ("DFMU", ("IDS_ATTRIB/STATUS[ %s = ... ] :="
                                 " ST_unique/ST_artificial !!!",
                                 IDS_NAME (_ids)));
        } else {
            IDS_ATTRIB (_ids) = VARDEC_OR_ARG_ATTRIB (decl);
            IDS_STATUS (_ids) = VARDEC_OR_ARG_STATUS (decl);

            DBUG_PRINT ("DFMU", ("IDS_ATTRIB/STATUS[ %s = ... ] == %s/%s",
                                 IDS_NAME (_ids), mdb_statustype[IDS_ATTRIB (_ids)],
                                 mdb_statustype[IDS_STATUS (_ids)]));
        }

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
