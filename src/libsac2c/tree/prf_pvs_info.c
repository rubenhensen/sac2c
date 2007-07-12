#include "prf_pvs_info.h"

#include "dbug.h"
#include "constants.h"

/**
 *
 * @prv_pvs_info.c
 *
 * @brief This file provides the propagation vectors for the prfs
 *
 */

/**
 * global constants
 */

static bool pv_initialized = FALSE;
static constant *pv_id = NULL;
static constant *pv_0003 = NULL;
static constant *pv_0033 = NULL;
static constant *pv_0233 = NULL;

/** <!-- ****************************************************************** -->
 *
 * @fn static void PPIinitializePVs()
 *
 *    @brief This function initializes the global constants which descripe
 *           certain pvs
 *
 ******************************************************************************/

void
PPIinitializePVs ()
{
    DBUG_ENTER ("PVinitializePVs");

    if (pv_initialized != TRUE) {
        pv_id = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 1, 2, 3);
        pv_0003 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 0, 0, 3);
        pv_0033 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 0, 3, 3);
        pv_0233 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 2, 3, 3);

        pv_initialized = TRUE;
    }
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVIdxId(int n)
 *
 *    @brief this functions returns the propagation vector for the nth
 *           argument.
 *
 *    @param n indicates which arguments pv has to be returned
 *
 *    @return the pv of the nth argument
 ******************************************************************************/

constant *
PPIgetPVIdxId (int n)
{
    DBUG_ENTER ("PVgetPVidxId");

    DBUG_RETURN (pv_id);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVReshape(int n)
 *
 *    @brief this functions returns the propagation vector for the nth
 *           argument.
 *
 *    @param n indicates which arguments pv has to be returned
 *
 *    @return the pv of the nth argument
 ******************************************************************************/

constant *
PPIgetPVReshape (int n)
{
    DBUG_ENTER ("PVgetPVreshape");

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_0233;
        break;
    case 1:
        res = pv_0003;
        break;
    default:
        break;
    }
    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVModarray(int n)
 *
 *    @brief this functions returns the propagation vector for the nth
 *           argument.
 *
 *    @param n indicates which arguments pv has to be returned
 *
 *    @return the pv of the nth argument
 ******************************************************************************/

constant *
PPIgetPVModarray (int n)
{
    DBUG_ENTER ("PPIgetPVModarray");

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_id;
        break;
    case 1:
        res = pv_0003;
        break;
    case 2:
        res = pv_0003;
        break;
    default:
        break;
    }
    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVTakeAndDrop(int n)
 *
 *    @brief this functions returns the propagation vector for the nth
 *           argument.
 *
 *    @param n indicates which arguments pv has to be returned
 *
 *    @return the pv of the nth argument
 ******************************************************************************/

constant *
PPIgetPVTakeAndDrop (int n)
{
    DBUG_ENTER ("PPIgetPVTakeAndDrop");

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_0033;
        break;
    case 1:
        res = pv_0003;
        break;
    default:
        break;
    }
    DBUG_RETURN (res);
}
