/*
 *
 * $Log$
 * Revision 1.11  2005/07/26 16:04:22  sbs
 * added SSIfreeAllTvars
 *
 * Revision 1.10  2005/07/25 17:14:25  sbs
 * changed to private heap in order to be able to collectively free tvars
 *
 * Revision 1.9  2005/01/11 14:20:44  cg
 * Converted output generation from Error.h to ctinfo.c
 *
 * Revision 1.8  2004/11/23 15:23:18  sbs
 * SacDevCamp04 done
 *
 * Revision 1.7  2004/08/08 16:05:08  sah
 * fixed some includes.
 *
 * Revision 1.6  2004/08/08 13:29:44  sbs
 * addded some DBUG_PRINTs
 *
 * Revision 1.5  2003/03/28 17:19:30  sbs
 * added to group ntc.
 *
 * Revision 1.4  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker !!
 *
 * Revision 1.3  2002/05/31 14:51:54  sbs
 * intermediate version to ensure compilable overall state.
 *
 * Revision 1.2  2002/03/12 16:47:40  sbs
 * ; after DBUG_VOID_REturn added.
 *
 * Revision 1.1  2002/03/12 15:16:27  sbs
 * Initial revision
 *
 *
 *
 */

#include "ssi.h"
#include "dbug.h"
#include "new_types.h"
#include "internal_lib.h"
#include "ctinfo.h"
#include "private_heap.h"

/**
 *
 * @addtogroup ntc
 *
 * @{
 */

/**
 *
 * @file ssi.c
 *
 * This file provides all those functions that are needed for representing
 * type variables as well as subtype inequalities imposed on them.
 * Therefore, it is crucial for deciding the "satisfiability of Subtype
 * Inequalities" problem (SSI in short).
 */

struct TVAR {
    int no;
    ntype *max;
    ntype *min;
    int maxbig;
    int nbig;
    tvar **big;
    int maxsmall;
    int nsmall;
    tvar **small;
    int maxass;
    int nass;
    sig_dep **handles;
};

#define CHUNK_SIZE 10

#define TVAR_NO(n) (n->no)
#define TVAR_MAX(n) (n->max)
#define TVAR_MIN(n) (n->min)

#define TVAR_MBIG(n) (n->maxbig)
#define TVAR_NBIG(n) (n->nbig)
#define TVAR_BIGS(n) (n->big)
#define TVAR_BIG(n, i) (n->big[i])

#define TVAR_MSMALL(n) (n->maxsmall)
#define TVAR_NSMALL(n) (n->nsmall)
#define TVAR_SMALLS(n) (n->small)
#define TVAR_SMALL(n, i) (n->small[i])

#define TVAR_MASS(n) (n->maxass)
#define TVAR_NASS(n) (n->nass)
#define TVAR_HANDS(n) (n->handles)
#define TVAR_HAND(n, i) (n->handles[i])

static int var_cntr = 0;
static bool ass_system_active = FALSE;
static tvar_ass_handle_fun ass_contra_handle = NULL;
static tvar_ass_handle_fun ass_fix_handle = NULL;

static heap *tvar_heap = NULL;

/*******************************************************************************
 ***
 ***
 ***  local helper functions:
 ***
 ***/

static bool
IsIn (tvar *var, int num, tvar **list)
{
    bool res = FALSE;
    int i = 0;

    DBUG_ENTER ("IsIn");

    while (!res && (i < num)) {
        res = (list[i] == var);
        i++;
    }
    DBUG_RETURN (res);
}

static void
AddBigger (tvar *small, tvar *big)
{
    tvar **new;
    int i;

    DBUG_ENTER ("AddBigger");
    if (TVAR_MBIG (small) == TVAR_NBIG (small)) {
        TVAR_MBIG (small) += CHUNK_SIZE;
        new = (tvar **)ILIBmalloc (sizeof (tvar *) * TVAR_MBIG (small));
        for (i = 0; i < TVAR_MBIG (small) - CHUNK_SIZE; i++) {
            new[i] = TVAR_BIG (small, i);
        }
        ILIBfree (TVAR_BIGS (small));
        TVAR_BIGS (small) = new;
    }
    /*
     * Now, we know that TVAR_NBIG( small) < TVAR_MBIG( small) !!
     */
    TVAR_BIG (small, TVAR_NBIG (small)) = big;
    TVAR_NBIG (small) += 1;

    DBUG_VOID_RETURN;
}

static void
AddSmaller (tvar *big, tvar *small)
{
    tvar **new;
    int i;

    DBUG_ENTER ("AddSmaller");
    if (TVAR_MSMALL (big) == TVAR_NSMALL (big)) {
        TVAR_MSMALL (big) += CHUNK_SIZE;
        new = (tvar **)ILIBmalloc (sizeof (tvar *) * TVAR_MSMALL (big));
        for (i = 0; i < TVAR_MSMALL (big) - CHUNK_SIZE; i++) {
            new[i] = TVAR_SMALL (big, i);
        }
        ILIBfree (TVAR_SMALLS (big));
        TVAR_SMALLS (big) = new;
    }
    /*
     * Now, we know that TVAR_NSMALL( big) < TVAR_MSMALL( big) !!
     */
    TVAR_SMALL (big, TVAR_NSMALL (big)) = small;
    TVAR_NSMALL (big) += 1;

    DBUG_VOID_RETURN;
}

static void
AddHandle (tvar *var, sig_dep *handle)
{
    sig_dep **new;
    int i;

    DBUG_ENTER ("AddHandle");
    if (TVAR_MASS (var) == TVAR_NASS (var)) {
        TVAR_MASS (var) += CHUNK_SIZE;
        new = (sig_dep **)ILIBmalloc (sizeof (sig_dep *) * TVAR_MASS (var));
        for (i = 0; i < TVAR_MASS (var) - CHUNK_SIZE; i++) {
            new[i] = TVAR_HAND (var, i);
        }
        ILIBfree (TVAR_HANDS (var));
        TVAR_HANDS (var) = new;
    }
    /*
     * Now, we know that TVAR_NASS( var) < TVAR_MASS( var) !!
     */
    TVAR_HAND (var, TVAR_NASS (var)) = handle;
    TVAR_NASS (var) += 1;

    DBUG_VOID_RETURN;
}

/*******************************************************************************
 ***
 ***
 ***  exported functions:
 ***
 ***/

/******************************************************************************
 *
 * function:
 *    tvar  * SSImakeVariable()
 *
 * description:
 *
 *
 ******************************************************************************/

tvar *
SSImakeVariable ()
{
    tvar *res;

    DBUG_ENTER ("SSImakeVariable");

    if (tvar_heap == NULL) {
        tvar_heap = PHPcreateHeap (sizeof (tvar), 1000);
    }

    res = (tvar *)PHPmalloc (tvar_heap);
    TVAR_NO (res) = var_cntr++;
    TVAR_MAX (res) = NULL;
    TVAR_MIN (res) = NULL;

    TVAR_MBIG (res) = 0;
    TVAR_NBIG (res) = 0;
    TVAR_BIGS (res) = NULL;

    TVAR_MSMALL (res) = 0;
    TVAR_NSMALL (res) = 0;
    TVAR_SMALLS (res) = NULL;

    TVAR_MASS (res) = 0;
    TVAR_NASS (res) = 0;
    TVAR_HANDS (res) = NULL;

    DBUG_PRINT ("SSI", ("new type var generated: #%d", var_cntr - 1));

    DBUG_RETURN (res);
}

void
SSIfreeAllTvars ()
{
    DBUG_ENTER ("SSIfreeAllTvars");

    tvar_heap = PHPfreeHeap (tvar_heap);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    bool    SSInewMax( tvar *var, ntype *cmax)
 *
 * description:
 *    Note here, that cmax is inspected only!!
 *
 *
 ******************************************************************************/

static bool
NewMax (tvar *var, ntype *cmax, bool outer)
{
    bool res;
    ct_res cmp;
    int i = 0;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NewMax");

    DBUG_EXECUTE ("SSI", tmp_str = TYtype2String (cmax, FALSE, 0););
    DBUG_PRINT ("SSI", ("    new max for #%d: %s", TVAR_NO (var), tmp_str));
    DBUG_EXECUTE ("SSI", tmp_str = ILIBfree (tmp_str););

    if (cmax == NULL) {
        res = TRUE;
    } else {
        if (TVAR_MAX (var) == NULL) {
            /*
             * we did not have a maximum yet
             */
            TVAR_MAX (var) = TYcopyType (cmax);
            res = TRUE;
        } else {
            /*
             * we do have a maximum
             */
            cmp = TYcmpTypes (cmax, TVAR_MAX (var));
            if (cmp == TY_dis) {
                res = FALSE;
            } else {
                if (cmp == TY_lt) {
                    /*
                     * cmax is a subtype of the existing maximum
                     * Therefore, we have to check compatibility with the minimum!
                     */
                    if (TVAR_MIN (var) == NULL) {
                        TYfreeType (TVAR_MAX (var));
                        TVAR_MAX (var) = TYcopyType (cmax);
                        res = TRUE;
                    } else {
                        if (TYleTypes (TVAR_MIN (var), cmax)) {
                            TYfreeType (TVAR_MAX (var));
                            TVAR_MAX (var) = TYcopyType (cmax);
                            res = TRUE;
                        } else {
                            res = FALSE;
                        }
                    }
                    /*
                     * iff the max has been changed we have to enforce new maxs to all
                     * SMALLs
                     */
                    if (outer) {
                        while (res && (i < TVAR_NSMALL (var))) {
                            res = NewMax (TVAR_SMALL (var, i), cmax, FALSE);
                            i++;
                        }
                    }
                } else {
                    /*
                     * The existing maximum is either less or equal to
                     * the new one! So no action is required!
                     */
                    res = TRUE;
                }
            }
        }
    }
    DBUG_RETURN (res);
}

bool
SSInewMax (tvar *var, ntype *cmax)
{
    bool res;

    DBUG_ENTER ("SSInewMax");

    res = NewMax (var, cmax, TRUE);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void InsertMinAndCheckAssumption( tvar *var, ntype *new_min)
 *
 * description:
 *
 *
 ******************************************************************************/

static void
InsertMinAndCheckAssumption (tvar *var, ntype *new_min)
{
    bool ok = TRUE;
    ntype *old_min;
    int i;

    DBUG_ENTER ("CheckAndHandleAssumption");

    old_min = TVAR_MIN (var);

    if ((TVAR_NASS (var) > 0) && ass_system_active
        && ((old_min == NULL)
            || ((old_min != NULL) && (TYcmpTypes (old_min, new_min) == TY_lt)))) {

        TVAR_MIN (var) = new_min;
        for (i = 0; i < TVAR_NASS (var); i++) {
            DBUG_PRINT ("SSI", ("Handling contradiction : %p", TVAR_HAND (var, i)));
            ok = ok && ass_contra_handle (TVAR_HAND (var, i));
        }

        if (!ok) {
            CTIabortLine (global.linenum, "Ugly squad type contradiction");
        }
    } else {
        TVAR_MIN (var) = new_min;
    }
    if (old_min != NULL) {
        TYfreeType (old_min);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    bool    SSInewMin( tvar *var, ntype *cmin)
 *
 * description:
 *
 *
 ******************************************************************************/

static bool
NewMin (tvar *var, ntype *cmin, bool outer)
{
    bool res = TRUE;
    ntype *tmp;
    int i = 0;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NewMin");

    DBUG_EXECUTE ("SSI", tmp_str = TYtype2String (cmin, FALSE, 0););
    DBUG_PRINT ("SSI", ("    new min for #%d: %s", TVAR_NO (var), tmp_str));
    DBUG_EXECUTE ("SSI", tmp_str = ILIBfree (tmp_str););

    if (cmin == NULL) {
        res = TRUE;
    } else {
        if (TVAR_MIN (var) == NULL) {
            /*
             * we did not have a minimum yet
             */
            tmp = TYcopyType (cmin);
        } else {
            /*
             * we do have a minimum
             */
            tmp = TYlubOfTypes (cmin, TVAR_MIN (var));
        }

        /*
         * Now, we check whether tmp can be used as new minimum:
         */
        if ((tmp != NULL) /* TYlubOfTypes may return NULL iff TY_dis! */
            && ((TVAR_MAX (var) == NULL)
                || ((TVAR_MAX (var) != NULL) && (TYleTypes (tmp, TVAR_MAX (var)))))) {
            /*
             * tmp is a subtype of the existing maximum
             * Therefore, tmp can replace the old minimum (iff it exists)
             */
            InsertMinAndCheckAssumption (var, TYcopyType (tmp));
            /*
             * if the min has been changed we have to enforce new mins to all BIGs
             */
            if (outer) {
                while (res && (i < TVAR_NBIG (var))) {
                    res = NewMin (TVAR_BIG (var, i), tmp, FALSE);
                    i++;
                }
            }
            TYfreeType (tmp);
        } else {
            res = FALSE;
        }
    }

    DBUG_RETURN (res);
}

bool
SSInewMin (tvar *var, ntype *cmin)
{
    bool res;

    DBUG_ENTER ("SSInewMin");
    res = NewMin (var, cmin, TRUE);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool    SSInewRel( tvar *small, tvar *big)
 *
 * description:
 *
 *
 ******************************************************************************/

bool
SSInewRel (tvar *small, tvar *big)
{
    bool res;
    int i;

    DBUG_ENTER ("SSInewRel");

    DBUG_PRINT ("SSI", ("    new rel #%d <= #%d", TVAR_NO (small), TVAR_NO (big)));

    if (IsIn (small, TVAR_NSMALL (big), TVAR_SMALLS (big))) {
        /*
         * relationship already established!
         */
        res = TRUE;
    } else {
        /*
         * add relation:
         */
        AddBigger (small, big);
        AddSmaller (big, small);
        /*
         * Build transitive closures:
         */
        for (i = 0; i < TVAR_NBIG (big); i++) {
            if (!IsIn (TVAR_BIG (big, i), TVAR_NBIG (small), TVAR_BIGS (small))) {
                AddBigger (small, TVAR_BIG (big, i));
                AddSmaller (TVAR_BIG (big, i), small);
            }
        }
        for (i = 0; i < TVAR_NSMALL (small); i++) {
            if (!IsIn (TVAR_SMALL (small, i), TVAR_NSMALL (big), TVAR_SMALLS (big))) {
                AddSmaller (big, TVAR_SMALL (small, i));
                AddBigger (TVAR_SMALL (small, i), big);
            }
        }

        res = SSInewMax (small, TVAR_MAX (big));
        res = (res && SSInewMin (big, TVAR_MIN (small)));
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool    SSInewTypeRel( tvar *small, tvar *big)
 *
 * description:
 *
 *
 ******************************************************************************/

bool
SSInewTypeRel (ntype *small, ntype *big)
{
    bool res;

    DBUG_ENTER ("SSInewTypeRel");

    if (TYisAlpha (small)) {
        if (TYisAlpha (big)) {
            res = SSInewRel (TYgetAlpha (small), TYgetAlpha (big));
        } else {
            res = SSInewMax (TYgetAlpha (small), big);
        }
    } else {
        if (TYisAlpha (big)) {
            res = SSInewMin (TYgetAlpha (big), small);
        } else {
            res = TYleTypes (small, big);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool     SSIinitAssumptionSystem( tvar_ass_handle_fun HandleContra,
 *                                         tvar_ass_handle_fun HandleFix)
 *
 * description:
 *
 *
 ******************************************************************************/

bool
SSIinitAssumptionSystem (tvar_ass_handle_fun HandleContra, tvar_ass_handle_fun HandleFix)
{
    bool res;

    DBUG_ENTER ("SSIinitAssumptionSystem");

    ass_contra_handle = HandleContra;
    ass_fix_handle = HandleFix;
    res = (!ass_system_active && (ass_fix_handle != NULL) && (ass_contra_handle != NULL));
    ass_system_active = TRUE;

    DBUG_PRINT ("SSI", ("Assumption system initialized"));

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @brief Checks whether assumption system is initialized.
 *
 * @return true is SSI has been initialized
 ******************************************************************************/
bool
SSIassumptionSystemIsInitialized ()
{
    DBUG_ENTER ("SSIassumptionSystemIsInitialized");

    DBUG_RETURN (ass_system_active);
}

/******************************************************************************
 *
 * function:
 *    bool     SSIassumeLow( tvar *var, sig_dep *handle)
 *
 * description:
 *
 *
 ******************************************************************************/

bool
SSIassumeLow (tvar *var, sig_dep *handle)
{
    DBUG_ENTER ("SSIassumeLow");

    DBUG_PRINT ("SSI",
                ("adding assumption for variable #%d, handle %p", TVAR_NO (var), handle));
    AddHandle (var, handle);
    DBUG_RETURN (ass_system_active);
}

/******************************************************************************
 *
 * function:
 *    bool    SSIfixLow( tvar *var)
 *
 * description:
 *
 *
 ******************************************************************************/

bool
SSIfixLow (tvar *var)
{
    sig_dep **hands;
    bool res;
    int i, n;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("SSIfixLow");

    DBUG_EXECUTE ("SSI", tmp_str = TYtype2String (TVAR_MIN (var), FALSE, 0););
    DBUG_PRINT ("SSI", ("fixing variable #%d to %s", TVAR_NO (var), tmp_str));
    DBUG_EXECUTE ("SSI", tmp_str = ILIBfree (tmp_str););

    SSInewMax (var, SSIgetMin (var));

    res = (SSIgetMin (var) != NULL);

    hands = TVAR_HANDS (var);
    n = TVAR_NASS (var);

    if (n > 0) {
        TVAR_MASS (var) = 0;
        TVAR_NASS (var) = 0;
        TVAR_HANDS (var) = NULL;

        for (i = 0; i < n; i++) {
            DBUG_PRINT ("SSI", ("Deleting handle : %p", hands[i]));
            res = res && ass_fix_handle (hands[i]);
        }

        hands = ILIBfree (hands);
    }

    DBUG_RETURN ((res && ass_system_active));
}

/******************************************************************************
 *
 * function:
 *    bool    SSIisFix( tvar *var)
 *    bool    SSIisLe( tvar *var1, tvar *var2)
 *    ntype * SSIgetMax( tvar *var)
 *    ntype * SSIgetMin( tvar *var)
 *
 * description:
 *    functions for retrieving the type restrictions imposed sofar.
 *    SSIisFix checks whether MAX and MIN are identical, SSIgetMax
 *    and SSIgetMin, respectively, yield the MAX and the MIN type
 *    attached.
 *
 ******************************************************************************/

bool
SSIisFix (tvar *var)
{
    DBUG_ENTER ("SSIisFix");
    DBUG_RETURN ((TVAR_MIN (var) != NULL) && (TVAR_MAX (var) != NULL)
                 && TYeqTypes (TVAR_MAX (var), TVAR_MIN (var)));
}

bool
SSIisLe (tvar *var1, tvar *var2)
{
    DBUG_ENTER ("SSIisLe");
    DBUG_RETURN (IsIn (var2, TVAR_NBIG (var1), TVAR_BIGS (var1)));
}

ntype *
SSIgetMax (tvar *var)
{
    DBUG_ENTER ("SSIgetMax");
    DBUG_RETURN (TVAR_MAX (var));
}

ntype *
SSIgetMin (tvar *var)
{
    DBUG_ENTER ("SSIgetMin");
    DBUG_RETURN (TVAR_MIN (var));
}

/******************************************************************************
 *
 * function:
 *    char  * SSIvariable2String( tvar *var)
 *
 * description:
 *
 *
 ******************************************************************************/

char *
SSIvariable2String (tvar *var)
{
    static char buf[4096];
    char *tmp = &buf[0];
    char *tmp_str, *tmp_str2;

    DBUG_ENTER ("SSIvariable2String");
    if (var == NULL) {
        tmp += sprintf (tmp, "--");
    } else {
        tmp_str = TYtype2String (TVAR_MIN (var), FALSE, 0);
        tmp_str2 = TYtype2String (TVAR_MAX (var), FALSE, 0);
        tmp += sprintf (tmp, "#%d in [ %s, %s]", TVAR_NO (var), tmp_str, tmp_str2);
        tmp_str = ILIBfree (tmp_str);
        tmp_str2 = ILIBfree (tmp_str2);
    }

    DBUG_RETURN (ILIBstringCopy (buf));
}

/******************************************************************************
 *
 * function:
 *    char  * SSIvariable2DebugString( tvar *var)
 *
 * description:
 *
 *
 ******************************************************************************/

char *
SSIvariable2DebugString (tvar *var)
{
    static char buf[65536];
    char *tmp = &buf[0];
    char *tmp_str, *tmp_str2;
    int i;

    DBUG_ENTER ("SSIvariable2DebugString");
    if (var == NULL) {
        tmp += sprintf (tmp, "--");
    } else {
        tmp_str = TYtype2String (TVAR_MIN (var), FALSE, 0);
        tmp_str2 = TYtype2String (TVAR_MAX (var), FALSE, 0);
        tmp += sprintf (tmp, "#%d: in [ %s, %s] le <", TVAR_NO (var), tmp_str, tmp_str2);
        tmp_str = ILIBfree (tmp_str);
        tmp_str2 = ILIBfree (tmp_str2);

        for (i = 0; i < TVAR_NBIG (var); i++) {
            tmp += sprintf (tmp, " %d", TVAR_NO (TVAR_BIG (var, i)));
        }
        tmp += sprintf (tmp, "> ge <");
        for (i = 0; i < TVAR_NSMALL (var); i++) {
            tmp += sprintf (tmp, " %d", TVAR_NO (TVAR_SMALL (var, i)));
        }
        tmp += sprintf (tmp, ">");
    }

    DBUG_RETURN (ILIBstringCopy (buf));
}

/* @} */ /* addtogroup ntc */
