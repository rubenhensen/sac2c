/*
 *
 * $Log$
 * Revision 3.18  2004/11/30 16:14:39  sah
 * converted DFMUdfm2ReturnExprs to avis nodes
 * made DFMUdfm2ReturnTypes robust
 *
 * Revision 3.17  2004/11/25 22:33:12  mwe
 * SacDevCamp Dk: Compiles!
 *
 * Revision 3.16  2004/11/23 10:05:24  sah
 * SaC DevCamp 04
 *
 * Revision 3.15  2004/11/21 11:22:03  sah
 * removed some old ast infos
 *
 * Revision 1.1  2000/01/21 16:52:08  dkr
 * Initial revision
 *
 */

#include "DataFlowMaskUtils.h"
#include "new_types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "internal_lib.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "LookUpTable.h"

/************************************************
 *
 *  DFM Stack
 */

/*
 * type definition for DFM stack
 */

struct STACK_T {
    dfmask_t *mask;
    struct STACK_T *next;
};

/******************************************************************************
 *
 * Function:
 *   dfmstack_t DFMUgenerateDfmStack( void)
 *
 * Description:
 *   Generates a new DFMstack.
 *
 ******************************************************************************/

dfmstack_t *
DFMUgenerateDfmStack (void)
{
    DBUG_ENTER ("DFMUgenerateDfmStack");

    DBUG_RETURN ((dfmstack_t *)NULL);
}

/******************************************************************************
 *
 * Function:
 *   extern int DFMUisEmptyDfmStack( DFMstack_t stack)
 *
 * Description:
 *   Checks, whether the given DFMstack is empty or not.
 *
 ******************************************************************************/

extern int
DFMUisEmptyDfmStack (dfmstack_t *stack)
{
    int res;

    DBUG_ENTER ("DFMUisEmptyDfmStack");

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
 *   void PushDfmStack( dfmstack_t **dfmstack, dfmask_t *mask)
 *
 * Description:
 *   Pushs a DFMmask onto the given DfmStack.
 *
 ******************************************************************************/

void
DFMUpushDfmStack (dfmstack_t **stack, dfmask_t *mask)
{
    dfmstack_t *new_stack;

    DBUG_ENTER ("DFMUpushDfmStack");

    new_stack = (dfmstack_t *)ILIBmalloc (sizeof (dfmstack_t));
    new_stack->mask = mask;
    new_stack->next = *stack;
    *stack = new_stack;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   dfmask_t* PopDfmStack( dfmstack_t **stack)
 *
 * Description:
 *   Pops a DFMmask from the given DfmStack.
 *
 ******************************************************************************/

dfmask_t *
DFMUpopDfmStack (dfmstack_t **stack)
{
    dfmstack_t *old_stack;
    dfmask_t *mask;

    DBUG_ENTER ("DFMUpopDfmStack");

    DBUG_ASSERT ((!DFMUisEmptyDfmStack (*stack)), "POP failed: stack is empty!");

    mask = (*stack)->mask;
    old_stack = *stack;
    (*stack) = (*stack)->next;
    old_stack = ILIBfree (old_stack);

    DBUG_RETURN (mask);
}

/******************************************************************************
 *
 * Function:
 *   void DFMUforeachDfmStack( dfmstack_t *stack, fun_t fun, char *id, node *decl)
 *
 * Description:
 *   Calls the function 'fun' for each DFMmask found in the given DfmStack.
 *
 ******************************************************************************/

void
DFMUforeachDfmStack (dfmstack_t *stack, fun_t fun, char *id, node *decl)
{
    dfmstack_t *tmp;

    DBUG_ENTER ("DFMUforeachDfmStack");

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
 *   void DFMUwhileDfmStack( dfmstack_t stack, fun_t fun, char *id, node *decl)
 *
 * Description:
 *   Calls the function 'fun' for each DFMmask found in the given DfmStack
 *   until the return value of the function is >0.
 *
 ******************************************************************************/

void
DFMUwhileDfmStack (dfmstack_t *stack, fun_t fun, char *id, node *decl)
{
    dfmstack_t *tmp;
    bool error = FALSE;

    DBUG_ENTER ("DFMUwhileDfmStack");

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
 *   void DFMUremoveDfmStack( dfmstack_t **stack)
 *
 * Description:
 *   Removes the given DfmStack.
 *
 ******************************************************************************/

void
DFMUremoveDfmStack (dfmstack_t **stack)
{
    DBUG_ENTER ("DFMUremoveDfmStack");

    while ((*stack) != NULL) {
        DFMremoveMask (DFMUpopDfmStack (stack));
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
 *   types *DFMUdfm2ReturnTypes( dfmask_t* mask)
 *
 * description:
 *   Creates a types chain based on the given DFmask.
 *
 ******************************************************************************/

types *
DFMUdfm2ReturnTypes (dfmask_t *mask)
{
    node *decl;
    types *tmp;
    types *rettypes = NULL;

    DBUG_ENTER ("DFMUdfm2ReturnTypes");

    /*
     * build return types, return exprs (use SPMD_OUT).
     */
    decl = DFMgetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        /*
         * there might be no old types, in which case
         * we cannot generate a return type.
         */
        if (VARDEC_OR_ARG_TYPE (decl) != NULL) {
            tmp = rettypes;
            rettypes = DUPdupAllTypes (VARDEC_OR_ARG_TYPE (decl));

            /*
             * VARDEC_OR_ARG_ATTRIB == 'ST_was_reference'
             *   -> TYPES_STATUS = 'ST_artificial'
             */

            if ((N_arg == NODE_TYPE (decl))
                && ((ARG_WASREFERENCE (decl)) || (ARG_ISARTIFICIAL (decl)))) {
                TYPES_STATUS (rettypes) = ST_artificial;
            }

            TYPES_NEXT (rettypes) = tmp;
        }

        decl = DFMgetMaskEntryDeclSet (NULL);
    }

    /*
     * we must build a void-type if ('rettypes' == NULL) is hold
     */
    if (rettypes == NULL) {
        rettypes = TBmakeTypes1 (T_void);
    }

    DBUG_RETURN (rettypes);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *DFMUdfm2ProductType( dfmask_t* mask)
 *
 * @brief recursive helper function for creating a product type
 *
 * @param decl DFM iterator on vardecs/args
 * @param pos  position in DFM iterator
 *
 ******************************************************************************/
static ntype *
DFMUdfm2ProductTypeRec (node *decl, int pos)
{
    ntype *result;

    DBUG_ENTER ("DFMUdfm2ProductTypeRec");

    if (decl == NULL) {
        if (pos == 0) {
            result = NULL;
        } else {
            result = TYmakeEmptyProductType (pos);
        }
    } else {
        ntype *ptype = AVIS_TYPE (DECL_AVIS (decl));

        if (ptype != NULL) {
            result = DFMUdfm2ProductTypeRec (DFMgetMaskEntryDeclSet (NULL), pos + 1);
        } else {
            /**
             * We have found a return value without a ntype attached to it.
             */

            DBUG_PRINT ("DFMU", ("Found return value with missing ntype"));

            result = NULL;
        }

        if (result != NULL) {
            TYsetProductMember (result, pos, TYcopyType (ptype));
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *DFMUdfm2ProductType( dfmask_t* mask)
 *
 * @brief Creates the product type of all types attached to to given mask
 *
 * @param mask in or out mask of a function
 *
 ******************************************************************************/
ntype *
DFMUdfm2ProductType (dfmask_t *mask)
{
    node *decl;
    ntype *result = NULL;

    DBUG_ENTER ("DFMUdfm2ProductType");

    decl = DFMgetMaskEntryDeclSet (mask);

    result = DFMUdfm2ProductTypeRec (decl, 0);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *DFMUdfm2FunctionType( DFMMask_t in, DFMMask_t out, node *fundef)
 *
 * @brief Creates the type of the function given as arguments
 *
 * @param in in mask of the given function
 * @param out out mask of the given function
 * @param fundef fundef node to process
 *
 ******************************************************************************/
ntype *
DFMUdfm2FunctionType (dfmask_t *in, dfmask_t *out, node *fundef)
{
    ntype *result;
    node *decl;

    DBUG_ENTER ("DFMUdfm2FunctionType");

    DBUG_PRINT ("DFMU", ("Creating ntype for function `%s'", FUNDEF_NAME (fundef)));

    result = DFMUdfm2ProductType (out);

    if (result != NULL) {
        decl = DFMgetMaskEntryDeclSet (in);

        while (decl != NULL) {
            ntype *tmp = AVIS_TYPE (DECL_AVIS (decl));

            if (tmp != NULL) {
                result = TYmakeFunType (TYcopyType (tmp), result, fundef);
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
                    result = TYfreeType (result);
                }

                DBUG_PRINT ("DFMU", ("Missing Argument type for function `%s'",
                                     FUNDEF_NAME (fundef)));

                break;
            }

            decl = DFMgetMaskEntryDeclSet (NULL);
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
 *   node *DFMUdfm2Vardecs( dfmask_t* mask, LUT_t lut)
 *
 * description:
 *   Creates a vardec-node chain based on the given DFmask.
 *   If (lut != NULL) the old/new declarations are inserted into the given LUT.
 *
 ******************************************************************************/

node *
DFMUdfm2Vardecs (dfmask_t *mask, lut_t *lut)
{
    node *decl, *tmp;
    node *vardecs = NULL;

    DBUG_ENTER ("DFMUdfm2Vardecs");

    decl = DFMgetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        if (NODE_TYPE (decl) == N_vardec) {
            tmp = vardecs;
            vardecs = DUPdoDupNode (decl);
            VARDEC_NEXT (vardecs) = tmp;
        } else {
            DBUG_ASSERT ((NODE_TYPE (decl) == N_arg),
                         "mask entry is neither an arg nor a vardec.");
            vardecs
              = TBmakeVardec (TBmakeAvis (ILIBstringCopy (ARG_NAME (decl)),
                                          TYcopyType (AVIS_TYPE (DECL_AVIS (decl)))),
                              vardecs);
        }

        lut = LUTinsertIntoLutP (lut, decl, vardecs);
        decl = DFMgetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *   node *DFMUdfm2Args( dfmask_t* mask, lut_t *lut)
 *
 * description:
 *   Creates a argument list (arg-node chain) based on the given DFmask.
 *   If (lut != NULL) the old/new declarations are inserted into the given LUT.
 *
 ******************************************************************************/

node *
DFMUdfm2Args (dfmask_t *mask, lut_t *lut)
{
    node *decl, *tmp;
    node *args = NULL;

    DBUG_ENTER ("DFMUdfm2Args");

    decl = DFMgetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        if (NODE_TYPE (decl) == N_arg) {
            tmp = args;
            args = DUPdoDupNode (decl);
            ARG_NEXT (args) = tmp;
        } else {
            DBUG_ASSERT ((NODE_TYPE (decl) == N_vardec),
                         "mask entry is neither an arg nor a vardec.");
            args = TBmakeArg (TBmakeAvis (ILIBstringCopy (VARDEC_NAME (decl)),
                                          TYcopyType (AVIS_TYPE (DECL_AVIS (decl)))),
                              args);
        }

        lut = LUTinsertIntoLutP (lut, decl, args);
        if (DECL_AVIS (decl) != NULL) {
            if (AVIS_TYPE (DECL_AVIS (decl)) != NULL) {
                AVIS_TYPE (ARG_AVIS (args)) = TYcopyType (AVIS_TYPE (DECL_AVIS (decl)));
            }
            lut = LUTinsertIntoLutP (lut, DECL_AVIS (decl), ARG_AVIS (args));
        }
        decl = DFMgetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *DFMUdfm2ReturnExprs( dfmask_t* mask, LUT_t lut)
 *
 * description:
 *   Creates a exprs/id-node chain for RETURN_EXPRS based on the given DFmask.
 *   If (lut != NULL) the attribute ID_VARDEC of the created IDs is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

node *
DFMUdfm2ReturnExprs (dfmask_t *mask, lut_t *lut)
{
    node *newavis, *id;
    node *exprs = NULL;
    node *avis;

    DBUG_ENTER ("DFMUdfm2ReturnExprs");

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        DBUG_ASSERT ((NODE_TYPE (avis) == N_avis), "found a non N_avis node in the DFM!");

        newavis = LUTsearchInLutPp (lut, avis);

        DBUG_ASSERT ((NODE_TYPE (newavis) == N_avis),
                     "found a non N_avis node in the LUT!");

        id = TBmakeId (newavis);

        exprs = TBmakeExprs (id, exprs);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   node *DFMUdfm2ApArgs( dfmask_t* mask, LUT_t lut)
 *
 * description:
 *   Creates a exprs/id-node chain for AP/PRF_ARGS based on the given DFmask.
 *   If (lut != NULL) the attribute ID_VARDEC of the created IDs is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

node *
DFMUdfm2ApArgs (dfmask_t *mask, lut_t *lut)
{
    node *decl, *id, *vardec_or_arg;
    node *exprs = NULL;

    DBUG_ENTER ("DFMUdfm2ApArgs");

    decl = DFMgetMaskEntryDeclSet (mask);
    while (decl != NULL) {

        /*
         * ID_VARDEC and ID_OBJDEF are mapped to the same node!
         */

        vardec_or_arg = LUTsearchInLutPp (lut, decl);
        id = TBmakeId (DECL_AVIS (vardec_or_arg));

        exprs = TBmakeExprs (id, exprs);

        decl = DFMgetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   ids *DFMUdfm2LetIds( dfmask_t* mask, LUT_t lut)
 *
 * description:
 *   Creates an ids chain based on the given DFmask.
 *   If (lut != NULL) the attribute IDS_VARDEC of the created IDS is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

node *
DFMUdfm2LetIds (dfmask_t *mask, lut_t *lut)
{
    node *decl, *vardec_or_arg;
    node *tmp;
    node *_ids = NULL;

    DBUG_ENTER ("DFMUdfm2LetIds");

    decl = DFMgetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        tmp = _ids;
        vardec_or_arg = LUTsearchInLutPp (lut, decl);
        _ids = TBmakeIds (DECL_AVIS (vardec_or_arg), NULL);
        IDS_NEXT (_ids) = tmp;

        decl = DFMgetMaskEntryDeclSet (NULL);
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
 *   dfmask_t* DFMDuplicateMask( dfmask_t* mask, DFMmask_base_t base)
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

dfmask_t *
DFMUduplicateMask (dfmask_t *mask, dfmask_base_t *base)
{
    char *name;
    dfmask_t *new_mask;

    DBUG_ENTER ("DFMUduplicateMask");

    if (mask != NULL) {
        if ((base != NULL) && (base != DFMgetMaskBase (mask))) {
            /* copy by hand */
            new_mask = DFMgenMaskClear (base);

            /* copy each name, DFMSetMaskEntrySet will fail if name not found */
            name = DFMgetMaskEntryNameSet (mask);
            while (name != NULL) {
                DBUG_PRINT ("DFMU", ("Entry in duplicated mask: %s", name));

                DFMsetMaskEntrySet (new_mask, name, NULL);
                name = DFMgetMaskEntryNameSet (NULL);
            }
        } else {
            /* copy by native */
            new_mask = DFMgenMaskCopy (mask);
        }
    } else {
        new_mask = NULL;
    }

    DBUG_RETURN (new_mask);
}
