/*
 *
 * $Log$
 * Revision 1.4  2003/09/09 14:56:11  sbs
 * extended type error reporting added
 *
 * Revision 1.3  2002/09/06 15:16:40  sbs
 * FUNDEF_RETURN now set properly?!
 *
 * Revision 1.2  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 1.1  2002/08/05 16:57:48  sbs
 * Initial revision
 *
 *
 */

#include "ct_fun.h"
#include "new_typecheck.h"
#include "type_errors.h"
#include "specialize.h"

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

/******************************************************************************
 ***
 ***          local helper functions
 ***          ----------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    DFT_res *DispatchFunType( node *wrapper, ntype *args)
 *
 * description:
 *    This function encapsulates TYDispatchFunType.
 *    It basically extracts the wrapper type and calls that function with it.
 *    Furthermore, some DBUG output may be generated.
 *
 ******************************************************************************/

static DFT_res *
DispatchFunType (node *wrapper, ntype *args)
{
    DFT_res *res;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("DispatchFunType");

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (args, 0, 0););
    DBUG_PRINT ("NTC", ("dispatching %s for %s", FUNDEF_NAME (wrapper), tmp_str));

    res = TYDispatchFunType (FUNDEF_TYPE (wrapper), args);

    DBUG_EXECUTE ("NTC", tmp_str = TYDFT_res2DebugString (res););
    DBUG_PRINT ("NTC", ("%s", tmp_str));
    DBUG_EXECUTE ("NTC", Free (tmp_str););

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void TriggerTypeChecking( DFT_res *dft)
 *
 * description:
 *
 ******************************************************************************/

static void
TriggerTypeChecking (DFT_res *dft)
{
    int i;

    DBUG_ENTER ("TriggerTypeChecking");

    /*
     * trigger type checking of the partials:
     */
    if (dft->num_partials > 0) {
        for (i = 0; i < dft->num_partials; i++) {
            dft->partials[i] = NTCTriggerTypeCheck (dft->partials[i]);
        }
    }

    /*
     * trigger the typechecking the defining function:
     */
    if (dft->def != NULL) {
        dft->def = NTCTriggerTypeCheck (dft->def);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 ***
 ***          Exported helper functions:
 ***          --------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    DFT_res *NTCFUNDispatchFunType( node *wrapper, ntype *args)
 *
 * description:
 *    This function encapsulates TYDispatchFunType.
 *    It basically extracts the wrapper type and calls that function with it.
 *    Furthermore, some DBUG output may be generated.
 *
 ******************************************************************************/

DFT_res *
NTCFUNDispatchFunType (node *wrapper, ntype *args)
{
    DFT_res *res;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTCFUNDispatchFunType");

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (args, 0, 0););
    DBUG_PRINT ("NTC", ("dispatching %s for %s", FUNDEF_NAME (wrapper), tmp_str));

    res = TYDispatchFunType (FUNDEF_TYPE (wrapper), args);

    DBUG_EXECUTE ("NTC", tmp_str = TYDFT_res2DebugString (res););
    DBUG_PRINT ("NTC", ("%s", tmp_str));
    DBUG_EXECUTE ("NTC", Free (tmp_str););

    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 ***          Type computation for user defined functions:
 ***          --------------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    ntype *NTCFUN_udf( te_info info, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *
 ******************************************************************************/

ntype *
NTCFUN_udf (te_info *info, ntype *args)
{
    ntype *res;
    node *fundef, *assign;
    DFT_res *dft_res;
    te_info *old_info_chn;

    DBUG_ENTER ("NTCFUN_udf");
    DBUG_ASSERT ((TYIsProdOfArray (args)), "NTCFUN_udf called with non-fixed args!");

    fundef = TEGetWrapper (info);
    assign = TEGetAssign (info);

    old_info_chn = act_info_chn;
    act_info_chn = info;
    DBUG_PRINT ("NTC_INFOCHN", ("act_info_chn set to %p", info));

    if (FUNDEF_IS_LACFUN (fundef)) {
        /*
         * specialize it, trigger its typecheck, and pick its return type:
         */
        fundef = SPECHandleLacFun (fundef, assign, args);
        fundef = NTCTriggerTypeCheck (fundef);
        res = TYCopyType (FUNDEF_RET_TYPE (fundef));

    } else {
        /*
         * we are dealing with a (overloaded) udf. Therefore, we have to dispatch
         * the funap in order to find the function instances involved and the result type.
         */
        dft_res = DispatchFunType (fundef, args);

        if (dft_res == NULL) {
            /*
             * this is a 0-ary function!!
             * Therefore, the fundef we are looking for must be behind FUNDEF_IMPL
             * of the wrapper function (cf. create_wrappers.c)!!!
             * Trigger its type check, and pick its return type:
             */
            fundef = FUNDEF_IMPL (fundef);
            fundef = NTCTriggerTypeCheck (fundef);
            res = TYCopyType (FUNDEF_RET_TYPE (fundef));

        } else {
            /*
             * create specializations (if appropriate), trigger the type
             * check of all potentially involved fundefs and extract the
             * return type from the dft_res structure:
             */
            dft_res = SPECHandleDownProjections (dft_res, fundef, args);

            if ((dft_res->def == NULL) && (dft_res->num_partials == 0)) {
                /*
                 * no match at all!
                 */
                ERROR (linenum,
                       ("No matching definition found for the application of \"%s\" "
                        " to arguments %s",
                        FUNDEF_NAME (fundef), TYType2String (args, FALSE, 0)));
                act_info_chn = TEGetParent (act_info_chn);
                TEExtendedAbort ();
            }

            TriggerTypeChecking (dft_res);

            res = TYCopyType (dft_res->type);
            DBUG_ASSERT ((res != NULL),
                         "HandleDownProjections did not return proper return type");
            TYFreeDFT_res (dft_res);
        }
    }

    act_info_chn = old_info_chn;
    DBUG_PRINT ("NTC_INFOCHN", ("act_info_chn reset to %p", act_info_chn));

    DBUG_RETURN (res);
}
