#include "prf_pvs_info.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "constants.h"

/**
 *
 * @prv_pvs_info.c
 *
 * @brief This file provides the propagation vectors for the prfs
 * And whath the F*CK is a propagation vector??
 *
 */

/**
 * global constants
 */

static bool pv_initialized = FALSE;
static constant *pv_id = NULL;
static constant *pv_0001 = NULL;
static constant *pv_0003 = NULL;
static constant *pv_0012 = NULL;
static constant *pv_0023 = NULL;
static constant *pv_0033 = NULL;
static constant *pv_0223 = NULL;
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
PPIinitializePVs (void)
{
    DBUG_ENTER ();

    if (pv_initialized != TRUE) {
        pv_id = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 1, 2, 3);
        pv_0001 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 0, 0, 1);
        pv_0003 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 0, 0, 3);
        pv_0012 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 0, 1, 2);
        pv_0023 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 0, 2, 3);
        pv_0033 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 0, 3, 3);
        pv_0223 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 2, 2, 3);
        pv_0233 = COmakeConstantFromDynamicArguments (T_int, 1, 4, 0, 2, 3, 3);

        pv_initialized = TRUE;
    }
    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVId()
 *
 *    @brief this functions returns the propagation vector for singe argument
 *           functions.
 *
 *    @param n indicates the nth argument
 *
 *    @return the id-pv, or NULL if n out of range
 ******************************************************************************/

constant *
PPIgetPVId (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    if (n == 0) {
        res = pv_id;
    }

    DBUG_RETURN (res);
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
 *    @return the pv of the nth argument or NULL if n out of range
 ******************************************************************************/

constant *
PPIgetPVIdxId (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_id;
        break;
    case 1:
        res = pv_id;
        break;
    default:
        break;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVS(int n)
 *
 *    @brief this function returns the propagation vector for a prf which just
 *           gets a scalar as an argument
 *
 *    @param n the nth argument (should be zero to be valid)
 *
 *    @return approbiate pv or NULL if n out of range
 ******************************************************************************/

constant *
PPIgetPVS (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    if (n == 0) {
        res = pv_0003;
    }
    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVV(int n)
 *
 *    @brief this function returns the propagation vector for a prf which just
 *           gets a vector as an arguments
 *
 *    @param n the nth argument (should be zero to be valid)
 *
 *    @return approbiate pv or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVV (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    if (n == 0) {
        res = pv_0023;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVSxS(int n)
 *
 *    @brief this function returns the propagation vector for a function which
 *           just gets two scalar arguments.
 *
 *    @param n indicates the nth argument
 *
 *    @return pv 0003 or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVSxS (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_0003;
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
 * @fn constant *PPIgetPVSxV(int n)
 *
 *    @brief this function returns the propagation vector for a prf which gets
 *           a scalar and a vectororial argument.
 *
 *    @param n indicates the nth argument
 *
 *    @return the pv of the nth argument or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVSxV (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_0003;
        break;
    case 1:
        res = pv_0023;
        break;
    default:
        break;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVVxS(int n)
 *
 *    @brief this function returns the propagation vector for a prf which
 *           gets a vectorial and a scalar argument.
 *
 *    @param n indicates the nth argument
 *
 *    @return the pv of the nth argument or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVVxS (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_0023;
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
 * @fn constant *PPIgetPVVxV(int n)
 *
 *    @brief this function returns the propagation vector for a prf which gets
 *           two vectorial agruments.
 *
 *    @param n indicates the nth argument
 *
 *    @return the pv 0023 or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVVxV (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_0023;
        break;
    case 1:
        res = pv_0023;
        break;
    default:
        break;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVDim(int n)
 *
 *   @brief this functions returns the pv for dim
 *
 *   @param n indicates the nth argument, should be zero to be valid
 *
 *   @return the pv of Dim or NULL if n is out of range
 *****************************************************************************/

constant *
PPIgetPVDim (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    if (n == 0) {
        res = pv_0001;
    }
    DBUG_RETURN (res);
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
 *    @return the pv of the nth argument or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVReshape (int n)
{
    DBUG_ENTER ();

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
 * @fn constant *PPIgetPVSel(int n)
 *
 *    @brief this functions returns the propagation vector sel
 *
 *    @param n indicates the nth arguments of sel
 *
 *    @return the pv of sel or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVSel (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    switch (n) {
    case 0:
        res = pv_0223;
        break;
    case 1:
        res = pv_id;
        break;
    default:
        break;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn constant *PPIgetPVShape(int n)
 *
 *    @brief this functions returns the propagation vector shape
 *
 *    @param n the nth argument, should be zero to be valid.
 *
 *    @return the pv of shape of NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVShape (int n)
{
    DBUG_ENTER ();

    constant *res = NULL;

    if (n == 0) {
        res = pv_0012;
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
 *    @return the pv of the nth argument or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVModarray (int n)
{
    DBUG_ENTER ();

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
 *    @return the pv of the nth argument or NULL if n is out of range
 ******************************************************************************/

constant *
PPIgetPVTakeAndDrop (int n)
{
    DBUG_ENTER ();

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

#undef DBUG_PREFIX
