/*
 *
 * $Log$
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

#include "types.h"
#include "new_types.h"

#include "ssi.h"

typedef struct ASSUMPTION {
    int no;
    ntype *min;
} assumption;

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
    assumption **ass;
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
#define TVAR_ASSS(n) (n->ass)
#define TVAR_ASS(n, i) (n->ass[i])

static int ass_cntr = 0;
static int var_cntr = 0;

/*******************************************************************************
 ***
 ***
 ***  local helper functions:
 ***
 ***/

bool
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

void
AddBigger (tvar *small, tvar *big)
{
    tvar **new;
    int i;

    DBUG_ENTER ("AddBigger");
    if (TVAR_MBIG (small) == TVAR_NBIG (small)) {
        TVAR_MBIG (small) += CHUNK_SIZE;
        new = (tvar **)Malloc (sizeof (tvar *) * TVAR_MBIG (small));
        for (i = 0; i < TVAR_MBIG (small) - CHUNK_SIZE; i++) {
            new[i] = TVAR_BIG (small, i);
        }
        Free (TVAR_BIGS (small));
        TVAR_BIGS (small) = new;
    }
    /*
     * Now, we know that TVAR_NBIG( small) < TVAR_MBIG( small) !!
     */
    TVAR_BIG (small, TVAR_NBIG (small)) = big;
    TVAR_NBIG (small) += 1;

    DBUG_VOID_RETURN;
}

void
AddSmaller (tvar *big, tvar *small)
{
    tvar **new;
    int i;

    DBUG_ENTER ("AddSmaller");
    if (TVAR_MSMALL (big) == TVAR_NSMALL (big)) {
        TVAR_MSMALL (big) += CHUNK_SIZE;
        new = (tvar **)Malloc (sizeof (tvar *) * TVAR_MSMALL (big));
        for (i = 0; i < TVAR_MSMALL (big) - CHUNK_SIZE; i++) {
            new[i] = TVAR_SMALL (big, i);
        }
        Free (TVAR_SMALLS (big));
        TVAR_SMALLS (big) = new;
    }
    /*
     * Now, we know that TVAR_NSMALL( big) < TVAR_MSMALL( big) !!
     */
    TVAR_SMALL (big, TVAR_NSMALL (big)) = small;
    TVAR_NSMALL (big) += 1;

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
 *    tvar  * SSIMakeVariable()
 *
 * description:
 *
 *
 ******************************************************************************/

tvar *
SSIMakeVariable ()
{
    tvar *res;

    DBUG_ENTER ("SSIMakeVariable");

    res = (tvar *)Malloc (sizeof (tvar));
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
    TVAR_ASSS (res) = NULL;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool    SSINewMax( tvar *var, ntype *cmax)
 *
 * description:
 *    Note here, that cmax is inspected only!!
 *
 *
 ******************************************************************************/

bool
SSINewMax (tvar *var, ntype *cmax)
{
    bool res;
    CT_res cmp;
    int i = 0;

    DBUG_ENTER ("SSINewMax");
    if (TVAR_MAX (var) == NULL) {
        /*
         * we did not have a maximum yet
         */
        TVAR_MAX (var) = TYCopyType (cmax);
        res = TRUE;
    } else {
        /*
         * we do have a maximum
         */
        cmp = TYCmpTypes (cmax, TVAR_MAX (var));
        if (cmp == TY_dis) {
            res = FALSE;
        } else {
            if (cmp == TY_lt) {
                /*
                 * cmax is a subtype of the existing maximum
                 * Therefore, we have to check compatibility with the minimum!
                 */
                if (TVAR_MIN (var) == NULL) {
                    TYFreeType (TVAR_MAX (var));
                    TVAR_MAX (var) = TYCopyType (cmax);
                    res = TRUE;
                } else {
                    if (TYLeTypes (TVAR_MIN (var), cmax)) {
                        TYFreeType (TVAR_MAX (var));
                        TVAR_MAX (var) = TYCopyType (cmax);
                        res = TRUE;
                    } else {
                        res = FALSE;
                    }
                }
                /*
                 * iff the max has been changed we have to enforce new maxs to all SMALLs
                 */
                while (res && (i < TVAR_NSMALL (var))) {
                    res = SSINewMax (TVAR_SMALL (var, i), cmax);
                    i++;
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
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool    SSINewMin( tvar *var, ntype *cmin)
 *
 * description:
 *
 *
 ******************************************************************************/

bool
SSINewMin (tvar *var, ntype *cmin)
{
    bool res = TRUE;
    ntype *tmp;
    int i = 0;

    DBUG_ENTER ("SSINewMin");

    if (TVAR_MIN (var) == NULL) {
        /*
         * we did not have a minimum yet
         */
        tmp = TYCopyType (cmin);
    } else {
        /*
         * we do have a minimum
         */
        tmp = TYLubOfTypes (cmin, TVAR_MIN (var));
    }

    /*
     * Now, we check whether tmp can be used as new minimum:
     */
    if (TVAR_MAX (var) == NULL) {
        TVAR_MIN (var) = tmp;
    } else {
        if (TYLeTypes (tmp, TVAR_MAX (var))) {
            /*
             * tmp is a subtype of the existing maximum
             * Therefore, tmp can replace the old minimum (iff it exists)
             */
            if (TVAR_MIN (var) != NULL) {
                TYFreeType (TVAR_MIN (var));
            }
            TVAR_MIN (var) = tmp;
            /*
             * if the min has been changed we have to enforce new mins to all BIGs
             */
            while (res && (i < TVAR_NBIG (var))) {
                res = SSINewMin (TVAR_BIG (var, i), tmp);
                i++;
            }
        } else {
            res = FALSE;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool    SSINewRel( tvar *small, tvar *big)
 *
 * description:
 *
 *
 ******************************************************************************/

bool
SSINewRel (tvar *small, tvar *big)
{
    bool res;
    int i;

    DBUG_ENTER ("SSINewRel");
    if (IsIn (small, TVAR_NSMALL (big), TVAR_SMALLS (big))) {
        /*
         * relationship already established!
         */
        res = TRUE;
    } else {
        /*
         * add relation:
         */
        res = SSINewMax (small, TVAR_MAX (big));
        res = (res && SSINewMin (big, TVAR_MIN (small)));

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
                AddBigger (TVAR_SMALL (small, i), small);
            }
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    int     SSIAssume( tvar *var, ntype *val)
 *
 * description:
 *
 *
 ******************************************************************************/

int
SSIAssume (tvar *var, ntype *val)
{
    DBUG_ENTER ("SSIAssume");
    DBUG_RETURN (ass_cntr++);
}

/******************************************************************************
 *
 * function:
 *    void    SSIKillAssumption( int assumption)
 *
 * description:
 *
 *
 ******************************************************************************/

void
SSIKillAssumption (int assumption)
{
    DBUG_ENTER ("SSIKillAssumption");
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    bool    SSIIsFix( tvar *var)
 *    ntype * SSIGetMax( tvar *var)
 *    ntype * SSIGetMin( tvar *var)
 *
 * description:
 *    functions for retrieving the type restrictions imposed sofar.
 *    SSIIsFix checks whether MAX and MIN are identical, SSIGetMax
 *    and SSIGetMin, respectively, yield the MAX and the MIN type
 *    attached.
 *
 ******************************************************************************/

bool
SSIIsFix (tvar *var)
{
    DBUG_ENTER ("SSIIsFix");
    DBUG_RETURN ((TVAR_MIN (var) != NULL) && (TVAR_MAX (var) != NULL)
                 && TYEqTypes (TVAR_MAX (var), TVAR_MIN (var)));
}

ntype *
SSIGetMax (tvar *var)
{
    DBUG_ENTER ("SSIGetMax");
    DBUG_RETURN (TVAR_MAX (var));
}

ntype *
SSIGetMin (tvar *var)
{
    DBUG_ENTER ("SSIGetMin");
    DBUG_RETURN (TVAR_MIN (var));
}

/******************************************************************************
 *
 * function:
 *    char  * SSIVariable2String( tvar *var)
 *
 * description:
 *
 *
 ******************************************************************************/

char *
SSIVariable2String (tvar *var)
{
    char buf[256];
    char *tmp = &buf[0];
    char *tmp_str, *tmp_str2;

    DBUG_ENTER ("SSIVariable2String");
    tmp_str = TYType2String (TVAR_MIN (var), FALSE, 0);
    tmp_str2 = TYType2String (TVAR_MAX (var), FALSE, 0);
    tmp += sprintf (tmp, "[ %s, %s]", tmp_str, tmp_str2);
    tmp_str = Free (tmp_str);
    tmp_str2 = Free (tmp_str2);

    DBUG_RETURN (StringCopy (buf));
}

/******************************************************************************
 *
 * function:
 *    char  * SSIVariable2DebugString( tvar *var)
 *
 * description:
 *
 *
 ******************************************************************************/

char *
SSIVariable2DebugString (tvar *var)
{
    char buf[256];
    char *tmp = &buf[0];
    char *tmp_str, *tmp_str2;
    int i;

    DBUG_ENTER ("SSIVariable2DebugString");
    tmp_str = TYType2String (TVAR_MIN (var), FALSE, 0);
    tmp_str2 = TYType2String (TVAR_MAX (var), FALSE, 0);
    tmp += sprintf (tmp, "#%d: in [ %s, %s] le <", TVAR_NO (var), tmp_str, tmp_str2);
    tmp_str = Free (tmp_str);
    tmp_str2 = Free (tmp_str2);

    for (i = 0; i < TVAR_NBIG (var); i++) {
        tmp += sprintf (tmp, " %d", TVAR_NO (TVAR_BIG (var, i)));
    }
    tmp += sprintf (tmp, "> ge <");
    for (i = 0; i < TVAR_NSMALL (var); i++) {
        tmp += sprintf (tmp, " %d", TVAR_NO (TVAR_SMALL (var, i)));
    }
    tmp += sprintf (tmp, ">");
    DBUG_RETURN (StringCopy (buf));
}
