/*
 *
 * $Log$
 * Revision 1.1  2002/08/05 16:58:37  sbs
 * Initial revision
 *
 *
 */

#include "dbug.h"
#include "specialize.h"
#include "ct_fun.h"
#include "create_wrappers.h"
#include "DupTree.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

static node *specialized_fundefs = NULL;

/******************************************************************************
 ***
 ***          local helper functions
 ***          ----------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    bool SpecializationOracle( node *wrapper, node *fundef, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

static bool
SpecializationOracle (node *wrapper, node *fundef, ntype *args)
{
    DBUG_ENTER ("SpecializationOracle");
    DBUG_RETURN (TRUE);
}

/******************************************************************************
 *
 * function:
 *    node *UpdateSignature( node *fundef, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *    This function replaces the old type siganture (in the N_arg nodes)
 *    by the given argument types (arg_ts), and updates the function type
 *    ( FUNDEF_TYPE( fundef) ) as well. It returns the modified N_fundef
 *    node.
 *
 ******************************************************************************/

static node *
UpdateSignature (node *fundef, ntype *arg_ts)
{
    node *args;
    ntype *type;
    int i = 0;

    DBUG_ENTER ("UpdateSignature");
    DBUG_ASSERT ((CountArgs (FUNDEF_ARGS (fundef)) == TYGetProductSize (arg_ts)),
                 "UpdateSignature called with incompatible no of arguments!");
    DBUG_ASSERT ((TYIsProdOfArrayOrFixedAlpha (arg_ts)),
                 "UpdateSignature called with non-fixed args!");

    args = FUNDEF_ARGS (fundef);
    while (args) {
        type = TYGetProductMember (arg_ts, i);
        ARG_TYPE (args) = FreeOneTypes (ARG_TYPE (args));
        ARG_TYPE (args) = TYType2OldType (type);
        args = ARG_NEXT (args);
        i++;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *    node *DoSpecialize(node *wrapper, node *fundef, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

static node *
DoSpecialize (node *wrapper, node *fundef, ntype *args)
{
    node *res;
    int i, n;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("DoSpecialize");

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (args, FALSE, 0););
    DBUG_PRINT ("NTC", ("specializing %s for %s", FUNDEF_NAME (fundef), tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

    /* copy the fundef to be specialized */
    res = DupNode (fundef);

    /* insert the new fundef into the specialiazed chain */
    FUNDEF_NEXT (res) = specialized_fundefs;
    specialized_fundefs = res;

    /* do actually specialize the copy !! */
    UpdateSignature (res, args);
    FUNDEF_TYPE (res) = CreateFuntype (res);

    /*
     * Finally, we make the result type variable(s) (a) subtype(s) of the
     * the original one(s)!
     */
    n = TYGetProductSize (FUNDEF_RET_TYPE (res));
    for (i = 0; i < n; i++) {
        SSINewRel (TYGetAlpha (TYGetProductMember (FUNDEF_RET_TYPE (res), i)),
                   TYGetAlpha (TYGetProductMember (FUNDEF_RET_TYPE (fundef), i)));
    }

    /* insert the new type signature into the wrapper */
    FUNDEF_TYPE (wrapper)
      = TYMakeOverloadedFunType (TYCopyType (FUNDEF_TYPE (res)), FUNDEF_TYPE (wrapper));

    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 ***       Exported functions that may trigger function specializations:
 ***       -------------------------------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    DFT_res *SPECHandleDownProjections( DFT_res *dft,
 *                                          node *wrapper,
 *                                            ntype *args)
 *
 * description:
 *
 ******************************************************************************/

DFT_res *
SPECHandleDownProjections (DFT_res *dft, node *wrapper, ntype *args)
{
    node *new_fundef;

    DBUG_ENTER ("HandleDownProjections");
    if (dft->deriveable != NULL) {
        /*
         * down case:
         */
        if (SpecializationOracle (wrapper, dft->deriveable, args)) {
            new_fundef = DoSpecialize (wrapper, dft->deriveable, args);
            dft = NTCFUNDispatchFunType (wrapper, args);
        } else {
            /* not yet ok! */
            DBUG_ASSERT (FALSE, "non-specialization not yet implemented!");
        }
    }
    if (dft->num_deriveable_partials > 0) {
        DBUG_ASSERT (FALSE, "specialization of partials not yet implemented");
    }

    /*
     * all potential specializations are done, i.e., we do not
     * have any deriveable stuff any more!
     */
    DBUG_ASSERT ((dft->deriveable == NULL) && (dft->num_deriveable_partials == 0),
                 "specialization did not eliminate all deriveables!");

    DBUG_RETURN (dft);
}

/******************************************************************************
 *
 * function:
 *    node *SPECHandleLacFun( node *fundef, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

node *
SPECHandleLacFun (node *fundef, ntype *args)
{
    node *tmp, *fun;

    DBUG_ENTER ("SPECHandleLacFun");
    DBUG_ASSERT (FUNDEF_IS_LACFUN (fundef), "SPECHandleLacFun called with non LaC fun!");

    if (FUNDEF_USED (fundef) > 1) {
        /*
         * The function that calls this LACfun has been specialized.
         * Unfortunately, the "specialization" of LAC functions is postponed
         * until actualy found for type checking, i.e., until here:
         */
        FUNDEF_USED (fundef) = FUNDEF_USED (fundef) - 1;

        tmp = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = NULL;
        fun = DupTree (fundef);
        FUNDEF_NEXT (fundef) = tmp;

        FUNDEF_TCSTAT (fun) = 0; /* NTC_not_checked; */
        FUNDEF_USED (fun) = 1;

        /* insert the new fundef into the specialized chain */
        FUNDEF_NEXT (fun) = specialized_fundefs;
        specialized_fundefs = fun;

    } else {
        fun = fundef;
    }
    UpdateSignature (fun, args);
    FUNDEF_TYPE (fun) = CreateFuntype (fun);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *    node *SPECResetSpecChain()
 *
 * description:
 *
 ******************************************************************************/

node *
SPECResetSpecChain ()
{
    node *res;

    DBUG_ENTER ("SPECResetSpecChain");

    res = specialized_fundefs;
    specialized_fundefs = NULL;

    DBUG_RETURN (res);
}
