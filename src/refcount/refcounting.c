/*
 *
 * $Log$
 * Revision 1.3  2004/07/18 08:50:42  ktr
 * all functions renamed into EMsomething
 *
 * Revision 1.2  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.1  2004/07/14 15:43:42  ktr
 * Initial revision
 *
 * Revision 1.20  2004/06/23 15:25:38  ktr
 * LHS identifiers of F_adjust_rc and F_fill prfs are now marked marked
 * with ST_artifical causing UndoSSATransform to rename them to their
 * original name.
 *
 * Revision 1.19  2004/06/23 09:33:44  ktr
 * Major code brush done.
 *
 * Revision 1.18  2004/06/09 09:53:18  ktr
 * F_fill now has the expression as the first argument
 *
 * Revision 1.17  2004/06/08 14:31:55  ktr
 * - ReuseCandidates for With-loops are inferred.
 * - ReuseCandidates of not matching type are discarded.
 *
 * Revision 1.16  2004/06/07 12:39:33  ktr
 * Invalid assumptions about C evaluation order had been made which led
 * to nasty lockups on x86-Systems.
 *
 * Revision 1.15  2004/06/03 15:22:53  ktr
 * New version featuring:
 * - alloc_or_reuse
 * - fill
 * - explicit index vector allocation
 *
 * Revision 1.14  2004/05/12 13:05:05  ktr
 * UndeSSA removed
 *
 * Revision 1.13  2004/05/10 16:23:28  ktr
 * RCOracle inserted.
 *
 * Revision 1.12  2004/05/10 16:08:19  ktr
 * Removed some printf output.
 *
 * Revision 1.11  2004/05/06 17:51:10  ktr
 * EMRefCount now should handle IVE ICMs, too. :)
 *
 * Revision 1.10  2004/05/05 20:23:31  ktr
 * Home -> ISP
 *
 * Revision 1.9  2004/05/05 19:18:44  ktr
 * C functions should now be correctly refcounted.
 *
 * Revision 1.8  2004/05/05 15:34:05  ktr
 * Log added.
 *
 *
 */

/**
 * @defgroup rc Reference Counting
 *
 * This group includes all the files needed by reference counting
 *
 * @{
 */

/**
 *
 * @file refcounting.c
 *
 * This file implements explicit reference counting in SSA form
 *
 */
#define NEW_INFO

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "print.h"
#include "ssa.h"
#include "refcounting.h"
#include "ReuseWithArrays.h"

/** <!--******************************************************************-->
 *
 *  Enumeration of possible RHS types.
 *
 ***************************************************************************/
typedef enum {
    rc_undef,
    rc_return,
    rc_copy,
    rc_funap,
    rc_prfap,
    rc_array,
    rc_const,
    rc_cond,
    rc_funcond,
    rc_cexprs,
    rc_icm,
    rc_with
} rc_rhs_type;

/** <!--******************************************************************-->
 *
 *  Enumeration of the different modes of EMRefcounting.
 *
 ***************************************************************************/
typedef enum { rc_default, rc_else, rc_annotate_cfuns } rc_mode;

/** <!--******************************************************************-->
 *
 *  Structure used for INCLIST and DECLIST.
 *
 ***************************************************************************/
typedef struct RC_LIST_STRUCT {
    node *avis;
    int count;
    struct RC_LIST_STRUCT *next;
} rc_list_struct;

/** <!--******************************************************************-->
 *
 *  Structure used for ALLOCLIST.
 *
 ***************************************************************************/
typedef struct AR_LIST_STRUCT {
    node *avis;
    node *rc;
    node *dim;
    node *shape;
    node *candidates; /* N_exprs */
    struct AR_LIST_STRUCT *next;
} ar_list_struct;

/** <!--******************************************************************-->
 *
 *  Structure used for the enviroments of a variable.
 *
 ***************************************************************************/
typedef struct RC_COUNTER {
    int depth;
    int count;
    struct RC_COUNTER *next;
} rc_counter;

/**
 * Oracle to tell which parameters of a external function must be
 * refcounted like primitive function parameters
 */
#define FUNDEF_EXT_NOT_REFCOUNTED(n, idx)                                                \
    ((FUNDEF_STATUS (n) == ST_Cfun)                                                      \
     && ((FUNDEF_PRAGMA (n) == NULL) || (FUNDEF_REFCOUNTING (n) == NULL)                 \
         || (PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (n)) <= (idx))                              \
         || (!(FUNDEF_REFCOUNTING (n)[idx]))))

/**
 * INFO structure
 */
struct INFO {
    rc_mode mode;
    int depth;
    rc_rhs_type rhs;
    rc_list_struct *declist;
    rc_list_struct *inclist;
    ar_list_struct *alloclist;
    node *fundef;
    ids *lhs;
    node *funap;
    node *reuselist;
};

/**
 * INFO macros
 */
#define INFO_EMRC_MODE(n) (n->mode)
#define INFO_EMRC_DEPTH(n) (n->depth)
#define INFO_EMRC_RHS(n) (n->rhs)
#define INFO_EMRC_DECLIST(n) (n->declist)
#define INFO_EMRC_INCLIST(n) (n->inclist)
#define INFO_EMRC_ALLOCLIST(n) (n->alloclist)
#define INFO_EMRC_FUNDEF(n) (n->fundef)
#define INFO_EMRC_LHS(n) (n->lhs)
#define INFO_EMRC_FUNAP(n) (n->funap)
#define INFO_EMRC_REUSELIST(n) (n->reuselist)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

#define AVIS_EMRC_DEFLEVEL(n) (n->int_data)
#define AVIS_EMRC_COUNTER(n) ((rc_counter *)(n->dfmask[0]))
#define AVIS_EMRC_COUNTER2(n) ((rc_counter *)(n->dfmask[1]))

/**
 *
 * @name RC LISTSTRUCT FUNCTIONS
 *
 * <!--
 * -->
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn MakeRCList
 *
 *  @brief creates a new reference counting list element
 *
 *  @param avis
 *  @param counter
 *  @param next
 *
 *  @return The new list element
 *
 ***************************************************************************/
static rc_list_struct *
MakeRCList (node *avis, int counter, rc_list_struct *next)
{
    rc_list_struct *rcls;

    DBUG_ENTER ("MakeRCList");

    rcls = Malloc (sizeof (rc_list_struct));

    rcls->avis = avis;
    rcls->count = counter;
    rcls->next = next;

    DBUG_RETURN (rcls);
}

/*@}*/

/**
 *
 * @name DECLIST FUNCTIONS
 *
 * <!--
 * -->
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn DecListHasNext
 *
 *  @brief returns whether the next element in the DECLIST (if any)
 *         has a certain depth
 *
 *  @param declist
 *  @param depth
 *
 *  @return ((rcls != NULL) && (declist->count == depth))
 *
 ***************************************************************************/
static bool
DecListHasNext (rc_list_struct *declist, int depth)
{
    return ((declist != NULL) && (declist->count == depth));
}

/** <!--******************************************************************-->
 *
 * @fn DecListInsert
 *
 *  @brief Inserts a list element representing an adjust_rc(x, -1) into
 *         DecList
 *
 *  @param declist
 *  @param avis
 *  @param depth
 *
 *  @return the modified declist
 *
 ***************************************************************************/
static rc_list_struct *
DecListInsert (rc_list_struct *declist, node *avis, int depth)
{
    DBUG_ENTER ("DecListInsert");

    if ((declist == NULL) || (declist->count < depth)) {
        declist = MakeRCList (avis, depth, declist);
    } else {
        declist->next = DecListInsert (declist->next, avis, depth);
    }
    DBUG_RETURN (declist);
}

/** <!--******************************************************************-->
 *
 * @fn DecListContains
 *
 *  @brief returns whether DECLIST contains an adjust_rc(x, -1) instruction
 *         at codelevel depth
 *
 *  @param declist
 *  @param avis
 *  @param depth
 *
 *  @return whether an element of declist refers to node avis
 *          at codelevel depth
 *
 ***************************************************************************/
static bool
DecListContains (rc_list_struct *declist, node *avis, int depth)
{
    bool res;
    DBUG_ENTER ("DecListContains");

    if (declist == NULL) {
        res = FALSE;
    } else {
        if ((declist->avis == avis) && (declist->count == depth)) {
            res = TRUE;
        } else {
            res = DecListContains (declist->next, avis, depth);
        }
    }
    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn DecListGetNext
 *
 *  @brief returns a copy of the first element from DECLIST with count == -1
 *
 *  @param declist
 *
 *  @return a copy of the first element from DECLIST with count == -1
 *
 ***************************************************************************/
static rc_list_struct *
DecListGetNext (rc_list_struct *declist)
{
    DBUG_ENTER ("DecListGetNext");
    DBUG_RETURN (MakeRCList (declist->avis, -1, NULL));
}

/** <!--******************************************************************-->
 *
 * @fn DecListPopNext
 *
 *  @brief removes the first element from DECLIST
 *
 *  @param declist
 *
 *  @return declist without the first element
 *
 ***************************************************************************/
static rc_list_struct *
DecListPopNext (rc_list_struct *declist)
{
    rc_list_struct *res;

    DBUG_ENTER ("DecListPopNext");

    res = declist->next;
    Free (declist);

    DBUG_RETURN (res);
}
/*@}*/

/**
 *
 *  @name ENVIRONMENT FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn MakeRCCounter
 *
 *  @brief Creates a new list element used in the ENIVORONMENT
 *
 *  @param depth
 *  @param count
 *  @param next
 *
 *  @return A new list element with depth, count and next set.
 *
 ***************************************************************************/
static rc_counter *
MakeRCCounter (int depth, int count, rc_counter *next)
{
    rc_counter *rcc;

    DBUG_ENTER ("MakeRCCounter");

    rcc = Malloc (sizeof (rc_counter));
    rcc->depth = depth;
    rcc->count = count;
    rcc->next = next;

    DBUG_RETURN (rcc);
}

/** <!--******************************************************************-->
 *
 * @fn InitializeEnvironment
 *
 *  @brief Initilizes a variable's environment with DEFLEVEL l
 *
 *  @param avis contains the environment
 *  @param deflevel
 *
 *  @return the initialized avis node
 *
 ***************************************************************************/
static node *
InitializeEnvironment (node *avis, int deflevel)
{
    DBUG_ENTER ("InitializeEnvironment");

    AVIS_EMRC_DEFLEVEL (avis) = deflevel;
    AVIS_EMRC_COUNTER2 (avis) = NULL;
    AVIS_EMRC_COUNTER (avis) = NULL;

    DBUG_RETURN (avis);
}

/** <!--******************************************************************-->
 *
 * @fn SetDefLevel
 *
 *  @brief Sets a variables DEFLEVEL iff this has not already be done
 *
 *  @param avis contains the environment
 *  @param deflevel
 *
 *  @return the modified avis node
 *
 ***************************************************************************/
static node *
SetDefLevel (node *avis, int deflevel)
{
    DBUG_ENTER ("SetDefLevel");

    if (AVIS_EMRC_DEFLEVEL (avis) == -1) {
        AVIS_EMRC_DEFLEVEL (avis) = deflevel;
    }

    DBUG_RETURN (avis);
}

/** <!--******************************************************************-->
 *
 * @fn GetDefLevel
 *
 *  @brief returns a variable's DEFLEVEL
 *
 *  @param avis contains the environment
 *
 *  @return the variable's DEFLEVEL
 *
 ***************************************************************************/
static int
GetDefLevel (node *avis)
{
    DBUG_ENTER ("GetDefLevel");
    DBUG_RETURN (AVIS_EMRC_DEFLEVEL (avis));
}

/** <!--******************************************************************-->
 *
 * @fn RescueEnvironment
 *
 *  @brief in order to handle conditionals, seperate environments for
 *         each branch must be maintained
 *         RescueEnvironment saves an already built up environment allowing
 *         the other branch to be recerence counted.
 *
 *  @param avis contains the environment
 *
 *  @return the modified avis node
 *
 ***************************************************************************/
static node *
RescueEnvironment (node *avis)
{
    DBUG_ENTER ("RescueEnvironment");

    AVIS_EMRC_COUNTER2 (avis) = AVIS_EMRC_COUNTER (avis);
    AVIS_EMRC_COUNTER (avis) = NULL;

    DBUG_RETURN (avis);
}

/** <!--******************************************************************-->
 *
 * @fn RestoreEnvironment
 *
 *  @brief in order to handle conditionals, seperate environments for
 *         each branch must be maintained
 *         RestoreEnvironment moves a rescued environment back into the
 *         active position.
 *
 *  @param avis contains the environment
 *
 *  @return the modified avis node
 *
 ***************************************************************************/
static node *
RestoreEnvironment (node *avis)
{
    DBUG_ENTER ("RestoreEnvironment");

    DBUG_ASSERT (AVIS_EMRC_COUNTER (avis) == NULL, "Environment != NULL!!!");

    AVIS_EMRC_COUNTER (avis) = AVIS_EMRC_COUNTER2 (avis);
    AVIS_EMRC_COUNTER2 (avis) = NULL;

    DBUG_RETURN (avis);
}

/** <!--******************************************************************-->
 *
 * @fn GetEnvironment
 *
 *  @brief returns a variable's environment counter on the given code level
 *
 *  @param avis contains the environment
 *  @param codelevel
 *
 *  @return returns a variable's environment counter on the given code level
 *
 ***************************************************************************/
static int
GetEnvironment (node *avis, int codelevel)
{
    int res;
    rc_counter *rcc;

    DBUG_ENTER ("GetEnvironment");

    rcc = AVIS_EMRC_COUNTER (avis);

    while ((rcc != NULL) && (rcc->depth > codelevel)) {
        rcc = rcc->next;
    }

    if ((rcc != NULL) && (rcc->depth == codelevel)) {
        res = rcc->count;
    } else {
        res = 0;
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn PopEnvironment
 *
 *  @brief Removes all environment counters below the given codelevel
 *
 *  @param avis contains the environment
 *  @param codelvel
 *
 *  @return the modified avis node
 *
 ***************************************************************************/
static node *
PopEnvironment (node *avis, int codelevel)
{
    rc_counter *tmp;

    DBUG_ENTER ("PopEnvironment");

    while ((AVIS_EMRC_COUNTER (avis) != NULL)
           && (AVIS_EMRC_COUNTER (avis)->depth >= codelevel)) {
        tmp = AVIS_EMRC_COUNTER (avis)->next;
        Free (AVIS_EMRC_COUNTER (avis));
        AVIS_EMRC_COUNTER (avis) = tmp;
    }

    DBUG_RETURN (avis);
}

/** <!--******************************************************************-->
 *
 * @fn IncreaseEnvCounter
 *
 *  @brief internal function to increase the environment counter at a
 *         certain codelevel
 *
 *  @param rcc a sublist of the environment list
 *  @param codelevel
 *  @param n
 *
 *  @return the modified environment list
 *
 ***************************************************************************/
static rc_counter *
IncreaseEnvCounter (rc_counter *rcc, int codelevel, int n)
{
    DBUG_ENTER ("IncreaseEnvCounter");

    if ((rcc == NULL) || (rcc->depth < codelevel)) {
        rcc = MakeRCCounter (codelevel, n, rcc);
    } else {
        if (rcc->depth == codelevel) {
            rcc->count += n;
        } else {
            rcc->next = IncreaseEnvCounter (rcc->next, codelevel, n);
        }
    }

    DBUG_RETURN (rcc);
}

/** <!--******************************************************************-->
 *
 * @fn AddEnvironment
 *
 *  @brief Adds n to a variable's environment at a given codelevel
 *
 *  @param avis contains the environment
 *  @param codelevel
 *  @param n
 *
 *  @return the modified avis node
 *
 ***************************************************************************/
static node *
AddEnvironment (node *avis, int codelevel, int n)
{
    DBUG_ENTER ("AddEnvironment");

    AVIS_EMRC_COUNTER (avis)
      = IncreaseEnvCounter (AVIS_EMRC_COUNTER (avis), codelevel, n);

    DBUG_RETURN (avis);
}

/*@}*/

/**
 *
 * @name INCLIST FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn IncListHasNext
 *
 *  @brief returns whether inclist has more elements
 *
 *  @param inclist
 *
 *  @return returns whether inclist has more elements
 *
 ***************************************************************************/
static bool
IncListHasNext (rc_list_struct *inclist)
{
    DBUG_ENTER ("IncListHasNext");
    DBUG_RETURN (inclist != NULL);
}

/** <!--******************************************************************-->
 *
 * @fn IncListInsert
 *
 *  @brief inserts an avis/count into the INCLIST
 *
 *  @param inclist
 *  @param avis
 *  @param count
 *
 *  @return the modified INCLIST
 *
 ***************************************************************************/
static rc_list_struct *
IncListInsert (rc_list_struct *inclist, node *avis, int count)
{
    DBUG_ENTER ("IncListInsert");

    if (inclist == NULL) {
        inclist = MakeRCList (avis, count, NULL);
    } else {
        if (inclist->avis == avis) {
            inclist->count += count;
        } else {
            inclist->next = IncListInsert (inclist->next, avis, count);
        }
    }

    DBUG_RETURN (inclist);
}

/** <!--******************************************************************-->
 *
 * @fn IncListGetNext
 *
 *  @brief returns the first element of the INCLIST
 *
 *  @param inclist
 *
 *  @return returns the first element of the INCLIST
 *
 ***************************************************************************/
static rc_list_struct *
IncListGetNext (rc_list_struct *inclist)
{
    DBUG_ENTER ("IncListGetNext");
    DBUG_RETURN (MakeRCList (inclist->avis, inclist->count, NULL));
}

/** <!--******************************************************************-->
 *
 * @fn IncListPopNext
 *
 *  @brief returns INCLIST without the first element
 *
 *  @param inclist
 *
 *  @return returns INCLIST without the first element
 *
 ***************************************************************************/
static rc_list_struct *
IncListPopNext (rc_list_struct *inclist)
{
    rc_list_struct *res;

    DBUG_ENTER ("IncListPopNext");

    res = inclist->next;
    Free (inclist);

    DBUG_RETURN (res);
}
/*@}*/

/**
 *
 * @name ADJUST_RC HELPER FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn RCOracle
 *
 *  @brief Oracle to decide wheter a variable must be reference counted
 *
 *  @param avis
 *  @param count
 *
 *  @return (count != 0)
 *
 ***************************************************************************/
static bool
RCOracle (node *avis, int count)
{
    return (count != 0);
}

/** <!--******************************************************************-->
 *
 * @fn MakeAdjustRC
 *
 *  @brief creates an N_assign-node containing an adjust_rc prf
 *
 *  @param avis the variable to be counted
 *  @param count count >= -1
 *  @param lhs Left hand side identifiers
 *  @param next_node
 *
 *  @return N_assign-node containing an adjust_rc prf
 *
 ***************************************************************************/
static node *
MakeAdjustRC (node *avis, int count, ids *lhs, node *next_node)
{
    node *n, *prf;
    ids *ids1, *ids2;

    if (!RCOracle (avis, count)) {
        n = next_node;
    } else {
        ids1 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL,
                        ST_regular);

        IDS_AVIS (ids1) = avis;
        IDS_VARDEC (ids1) = AVIS_VARDECORARG (avis);

        /*
         * By setting IDS_STATUS of the LHS identifier to ST_artificial,
         * SSATransform will mark this with SSAUNDOFLAG causing
         * UndoSSATransform to rename it to its original name:
         *
         * Before UndoSSATransform:
         * a' = inc_rc( a, -1);
         *
         * After UndoSSATransform:
         * a  = inc_rc( a, -1);
         */
        ids2 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL,
                        ST_artificial);

        IDS_AVIS (ids2) = avis;
        IDS_VARDEC (ids2) = AVIS_VARDECORARG (avis);

        if (count > 0) {
            /*
             * Make INC_RC
             */
            prf = MakePrf2 (F_inc_rc, MakeIdFromIds (ids1), MakeNum (count));

        } else {
            /*
             * Make DEC_RC
             */
            DBUG_ASSERT (count == -1, "DEC_RC must decrement RC by 1!!!");
            if (lhs != NULL) {
                /*
                 * Make dependent DEC_RC
                 */
                prf = MakePrf2 (F_dec_rc, MakeIdFromIds (ids1),
                                MakeIdFromIds (DupOneIds (lhs)));
            } else {
                /*
                 * Make independent DEC_RC
                 */
                prf = MakePrf1 (F_dec_rc, MakeIdFromIds (ids1));
            }
        }
        n = MakeAssign (MakeLet (prf, ids2),
                        (((next_node != NULL) && (NODE_TYPE (next_node) == N_assign))
                           ? next_node
                           : NULL));
    }
    return (n);
}

/** <!--******************************************************************-->
 *
 * @fn MakeAdjustRCFromRLS
 *
 *  @brief Converts a RCList Element into a adjust_rc operation
 *
 *  @param rls (will be consumed)
 *  @param lhs Left Hand Side Identifiers
 *  @param next_node
 *
 *  @return An adjust_rc operation
 *
 ***************************************************************************/
static node *
MakeAdjustRCFromRLS (rc_list_struct *rls, ids *lhs, node *next_node)
{
    node *res;

    DBUG_ENTER ("MakeAdjustRCFromRLS");

    res = MakeAdjustRC (rls->avis, rls->count, lhs, next_node);
    Free (rls);

    DBUG_RETURN (res);
}
/*@}*/

/**
 *
 * @name REUSE HELPER FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn equalTypes
 *
 *  @brief compares two types
 *
 *  @param t1
 *  @param t2
 *
 *  @return true iff t1 and t2 are AKS and have equal basetype and shape
 *
 ***************************************************************************/
static bool
equalTypes (types *t1, types *t2)
{
    bool compare;
    shpseg *shpseg1, *shpseg2;
    int dim1, dim2;
    int d;

    DBUG_ENTER ("TypesAreEqual");

    shpseg1 = Type2Shpseg (t1, &dim1);
    shpseg2 = Type2Shpseg (t2, &dim2);

    compare = ((dim1 >= 0) && (dim1 == dim2) && (GetBasetype (t1) == GetBasetype (t2)));

    if (compare) {
        for (d = 0; d < dim1; d++) {
            if (SHPSEG_SHAPE (shpseg1, d) != SHPSEG_SHAPE (shpseg2, d)) {
                compare = FALSE;
            }
        }
    }

    DBUG_RETURN (compare);
}

/** <!--******************************************************************-->
 *
 * @fn ReuseListContainsAvis
 *
 *  @brief returns whether an avis node appears in REUSELIST
 *
 *  @param reuselist
 *  @param avis
 *
 *  @return returns whether an avis node appears in REUSELIST
 *
 ***************************************************************************/
static bool
ReuseListContainsAvis (node *reuselist, node *avis)
{
    bool res;

    DBUG_ENTER ("ReuseListContainsAvis");

    if (reuselist == NULL) {
        res = FALSE;
    } else {
        if (ID_AVIS (EXPRS_EXPR (reuselist)) == avis) {
            res = TRUE;
        } else {
            res = ReuseListContainsAvis (EXPRS_NEXT (reuselist), avis);
        }
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn FilterReuseList
 *
 *  @brief Filters REUSELIST such that it only contains valid reuse
 *         candidates for a certain IDS
 *
 *  @param reuselist
 *  @param declist
 *  @param ids
 *
 *  @return a filtered copy of the reuselist
 *
 *  A valid reuse candidate must hold the following:
 *    - It has the same shape as IDS
 *    - It must not appear again in the list tail
 *    - It must not yet appear in DECLIST at its DEFINITION LEVEL
 *      <=> A variable can only be reused if it about to be freed
 *
 ***************************************************************************/
static node *
FilterReuseList (node *reuselist, rc_list_struct *declist, ids *ids)
{
    DBUG_ENTER ("FilterReuseList");

    if (reuselist != NULL) {
        if ((DecListContains (declist, ID_AVIS (EXPRS_EXPR (reuselist)),
                              AVIS_EMRC_DEFLEVEL (ID_AVIS (EXPRS_EXPR (reuselist)))))
            && (equalTypes (IDS_TYPE (ids), ID_TYPE (EXPRS_EXPR (reuselist))))
            && (!ReuseListContainsAvis (EXPRS_NEXT (reuselist),
                                        ID_AVIS (EXPRS_EXPR (reuselist))))) {
            reuselist
              = MakeExprs (DupNode (EXPRS_EXPR (reuselist)),
                           FilterReuseList (EXPRS_NEXT (reuselist), declist, ids));
        } else {
            reuselist = FilterReuseList (EXPRS_NEXT (reuselist), declist, ids);
        }
    }

    DBUG_RETURN (reuselist);
}

/*@}*/

/**
 *
 * @name ALLOC_OR_REUSE LIST FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn AllocListHasNext
 *
 *  @brief returns whether ALLOC_LIST has further elements
 *
 *  @param als ALLOC_LIST
 *
 *  @return als != NULL
 *
 ***************************************************************************/
static bool
AllocListHasNext (ar_list_struct *als)
{
    DBUG_ENTER ("AllocListHasNext");
    DBUG_RETURN (als != NULL);
}

/** <!--******************************************************************-->
 *
 * @fn MakeARList
 *
 *  @brief creates a new ALLOC_LIST element
 *
 *  @param next will be appended to the new element
 *  @param avis
 *  @param rc
 *  @param dim
 *  @param shape
 *  @param cand
 *
 *  @return a new ALLOC_LIST element
 *
 ***************************************************************************/
static ar_list_struct *
MakeARList (ar_list_struct *next, node *avis, node *rc, node *dim, node *shape,
            node *cand)
{
    ar_list_struct *als;

    DBUG_ENTER ("MakeARList");

    DBUG_ASSERT (NUM_VAL (rc) == 1, "RC must be == 1");
    DBUG_ASSERT (dim != NULL, "Dimension must not be NULL");
    DBUG_ASSERT (shape != NULL, "Shape must not be NULL");

    als = Malloc (sizeof (ar_list_struct));
    als->avis = avis;
    als->rc = rc;
    als->dim = dim;
    als->shape = shape;
    als->candidates = cand;

    als->next = next;

    DBUG_RETURN (als);
}

/** <!--******************************************************************-->
 *
 * @fn FreeARList
 *
 *  @brief Frees a list of ar_list_struct elements
 *
 *  @param als ar_list_struct list
 *
 *  @return NULL
 *
 ***************************************************************************/
static ar_list_struct *
FreeARList (ar_list_struct *als)
{
    DBUG_ENTER ("FreeARList");

    if (als != NULL) {
        als->next = FreeARList (als->next);
        als->avis = NULL;
        als->dim = FreeTree (als->dim);
        als->shape = FreeTree (als->shape);
        if (als->candidates != NULL) {
            als->candidates = FreeTree (als->candidates);
        }
        als = Free (als);
    }
    DBUG_RETURN (als);
}

/** <!--******************************************************************-->
 *
 * @fn AllocListGetNext
 *
 *  @brief returns the head element of ALLOCLIST
 *
 *  @param als ALLOCLIST
 *
 *  @return the head element of ALLOCLIST
 *
 ***************************************************************************/
static ar_list_struct *
AllocListGetNext (ar_list_struct *als)
{
    DBUG_ENTER ("AllocListGetNext");
    DBUG_RETURN (MakeARList (NULL, als->avis, DupNode (als->rc), DupNode (als->dim),
                             DupNode (als->shape), DupTree (als->candidates)));
}

/** <!--******************************************************************-->
 *
 * @fn AllocListPopNext
 *
 *  @brief returns tail of ALLOCLIST
 *
 *  @param als ALLOCLIST
 *
 *  @return tail of ALLOCLIST
 *
 ***************************************************************************/
static ar_list_struct *
AllocListPopNext (ar_list_struct *als)
{
    ar_list_struct *res;

    DBUG_ENTER ("AllocListPopNext");

    res = als->next;
    als->next = NULL;
    als = FreeARList (als);

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn MakeAllocOrReuseFromALS
 *
 *  @brief converts an element of ALLOC_LIST into an alloc_or_reuse
 *         assignment
 *
 *  @param als ALLOC_LIST will not be consumed but its components are reused.
 *  @param next_node
 *
 *  @return a new assignment
 *
 ***************************************************************************/
static node *
MakeAllocOrReuseFromALS (ar_list_struct *als, node *next_node)
{
    node *n;
    ids *ids;

    DBUG_ENTER ("MakeAllocOrReuseFromALS");

    ids = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (als->avis))), NULL,
                   ST_regular);

    IDS_AVIS (ids) = als->avis;
    IDS_VARDEC (ids) = AVIS_VARDECORARG (als->avis);

    n = MakeAssign (MakeLet (MakePrf (F_alloc_or_reuse,
                                      MakeExprs (als->rc,
                                                 MakeExprs (als->dim,
                                                            MakeExprs (als->shape,
                                                                       als
                                                                         ->candidates)))),
                             ids),
                    next_node);

    DBUG_RETURN (n);
}

/*@}*/

/**
 *
 * @name *List -> Assigment conversion
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn MakeIncAssignment
 *
 *  @brief converts DECLIST into a chain of adjust_rc prfs
 *
 *  @param arg_info contains DECLIST
 *  @param next_node
 *
 *  @return chain of adjust_rc prf
 *
 ***************************************************************************/
static node *
MakeDecAssignments (info *arg_info, node *next_node)
{
    rc_list_struct *rls;

    DBUG_ENTER ("MakeDecAssignment");

    if (DecListHasNext (INFO_EMRC_DECLIST (arg_info), INFO_EMRC_DEPTH (arg_info))) {

        rls = DecListGetNext (INFO_EMRC_DECLIST (arg_info));
        INFO_EMRC_DECLIST (arg_info) = DecListPopNext (INFO_EMRC_DECLIST (arg_info));

        next_node = MakeAdjustRCFromRLS (rls, INFO_EMRC_LHS (arg_info),
                                         MakeDecAssignments (arg_info, next_node));
    }

    DBUG_RETURN (next_node);
}

/** <!--******************************************************************-->
 *
 * @fn MakeIncAssignment
 *
 *  @brief converts INCLIST into a chain of adjust_rc prfs
 *
 *  @param arg_info contains INCLIST
 *  @param next_node
 *
 *  @return chain of adjust_rc prfs
 *
 ***************************************************************************/
static node *
MakeIncAssignments (info *arg_info, node *next_node)
{
    rc_list_struct *rls;

    DBUG_ENTER ("MakeIncAssignments");

    if (IncListHasNext (INFO_EMRC_INCLIST (arg_info))) {

        rls = IncListGetNext (INFO_EMRC_INCLIST (arg_info));

        INFO_EMRC_INCLIST (arg_info) = IncListPopNext (INFO_EMRC_INCLIST (arg_info));

        next_node = MakeAdjustRCFromRLS (rls, INFO_EMRC_LHS (arg_info),
                                         MakeIncAssignments (arg_info, next_node));
    }

    DBUG_RETURN (next_node);
}

/** <!--******************************************************************-->
 *
 * @fn MakeAllocAssignments
 *
 *  @brief converts ALLOCLIST into a chain of alloc_or_reuse prfs
 *
 *  @param arg_info contains ALLOCLIST
 *  @param next_node
 *
 *  @return chain of alloc_or_reuse prfs
 *
 ***************************************************************************/
static node *
MakeAllocAssignments (info *arg_info, node *next_node)
{
    ar_list_struct *als;

    DBUG_ENTER ("MakeAllocAssignments");

    if (AllocListHasNext (INFO_EMRC_ALLOCLIST (arg_info))) {

        als = AllocListGetNext (INFO_EMRC_ALLOCLIST (arg_info));

        INFO_EMRC_ALLOCLIST (arg_info)
          = AllocListPopNext (INFO_EMRC_ALLOCLIST (arg_info));

        next_node
          = MakeAllocOrReuseFromALS (als, MakeAllocAssignments (arg_info, next_node));
        als = Free (als);
    }

    DBUG_RETURN (next_node);
}
/*@}*/

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn EMRCap
 *
 *  @brief adds one to each of the argument's environments
 *
 *  By definition, a function application consumes a reference from each
 *  of its arguments. Therefore, one is added to each argument's environment
 *  at the current code level. If the current code level differs from the
 *  variable's definition level and the variable's environment at its
 *  definition level is zero, it is set to one and the variable is added
 *  to the DECLIST.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCap (node *arg_node, info *arg_info)
{
    node *args, *avis;
    int argc;

    DBUG_ENTER ("EMRCap");

    INFO_EMRC_RHS (arg_info) = rc_funap;

    INFO_EMRC_FUNAP (arg_info) = AP_FUNDEF (arg_node);

    args = AP_ARGS (arg_node);
    argc = CountIds (INFO_EMRC_LHS (arg_info));

    while (args != NULL) {

        if ((EXPRS_EXPR (args) != NULL) && (NODE_TYPE (EXPRS_EXPR (args)) == N_id)) {

            avis = ID_AVIS (EXPRS_EXPR (args));

            /*
             * Add one to the environment at the arguments DEFLEVEL
             * iff it is zero
             */
            if ((GetDefLevel (avis) != INFO_EMRC_DEPTH (arg_info))
                && (GetEnvironment (avis, GetDefLevel (avis)) == 0)) {
                INFO_EMRC_DECLIST (arg_info)
                  = DecListInsert (INFO_EMRC_DECLIST (arg_info), avis,
                                   GetDefLevel (avis));
                avis = AddEnvironment (avis, GetDefLevel (avis), 1);
            }

            if (!FUNDEF_EXT_NOT_REFCOUNTED (INFO_EMRC_FUNAP (arg_info), argc)) {
                /*
                 * If this function is not an external function which's argument
                 * must be refcounted like a prf argument
                 * we increase the environment at the current codelevel by one
                 */
                avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info), 1);
            }
        }
        argc += 1;
        args = EXPRS_NEXT (args);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCarg
 *
 *  @brief Function arguments are only traversed to initialize the
 *         variable environments
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node with initialized environment
 *
 ***************************************************************************/
node *
EMRCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCarg");

    ARG_AVIS (arg_node) = InitializeEnvironment (ARG_AVIS (arg_node), 0);

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCArray
 *
 *  @brief traverses the array's arguments and allocates memory iff the
 *         array appears on a RHS of a LET-Node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCarray");

    if (INFO_EMRC_RHS (arg_info) == rc_undef) {
        INFO_EMRC_RHS (arg_info) = rc_array;

        /*
         * Make memory allocation
         */
        if (ARRAY_AELEMS (arg_node) != NULL) {
            /*
             * [ a, ... ]
             * alloc_or_reuse( 1, outer_dim + dim(a), genarray( outer_shape, a))
             */
            INFO_EMRC_ALLOCLIST (arg_info)
              = MakeARList (INFO_EMRC_ALLOCLIST (arg_info),
                            IDS_AVIS (INFO_EMRC_LHS (arg_info)), MakeNum (1),

                            MakePrf2 (F_add_SxS,
                                      MakeNum (SHGetDim (ARRAY_SHAPE (arg_node))),
                                      MakePrf (F_dim, DupNode (EXPRS_EXPR (
                                                        ARRAY_AELEMS (arg_node))))),

                            MakePrf (F_shape,
                                     MakePrf2 (F_genarray,
                                               SHShape2Array (ARRAY_SHAPE (arg_node)),
                                               DupNode (
                                                 EXPRS_EXPR (ARRAY_AELEMS (arg_node))))),
                            NULL);
        } else {
            /*
             * []: empty array
             * alloc_or_reuse( 1, outer_dim, outer_shape)
             */
            INFO_EMRC_ALLOCLIST (arg_info)
              = MakeARList (INFO_EMRC_ALLOCLIST (arg_info),
                            IDS_AVIS (INFO_EMRC_LHS (arg_info)), MakeNum (1),

                            MakeNum (SHGetDim (ARRAY_SHAPE (arg_node))),

                            SHShape2Array (ARRAY_SHAPE (arg_node)),

                            NULL);
        }
    }

    if (ARRAY_AELEMS (arg_node) != NULL) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    }

    DBUG_ASSERT (INFO_EMRC_REUSELIST (arg_info) == NULL,
                 "REUSELIST must have no members");

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCassign
 *
 *  @brief traverses an N_assign node.
 *
 *  On down traversal the definition levels of each variable are annotated.
 *  On up traversal, first the RHS of an assignment is traversed and the
 *  various ALLOC_OR_REUSE and ADJUST_RC statements which are specified in
 *  ALLOCLIST, INCLIST and DECLIST are inserted.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node and several ALLOC_OR_REUSE and ADJUST_RC statements
 *
 ***************************************************************************/
node *
EMRCassign (node *arg_node, info *arg_info)
{
    node *n;
    ids *i;

    DBUG_ENTER ("EMRCassign");

    /*
     * Top down traversal:
     * Annotate definition level at avis nodes
     */
    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let) {
        i = LET_IDS (ASSIGN_INSTR (arg_node));
        while (i != NULL) {
            IDS_AVIS (i) = SetDefLevel (IDS_AVIS (i), INFO_EMRC_DEPTH (arg_info));
            i = IDS_NEXT (i);
        }
    }

    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_icm) {
        ID_AVIS (ICM_ARG1 (ASSIGN_INSTR (arg_node)))
          = SetDefLevel (ID_AVIS (ICM_ARG1 (ASSIGN_INSTR (arg_node))),
                         INFO_EMRC_DEPTH (arg_info));
    }

    /*
     * Bottom up traversal:
     * Annotate memory management instructions
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_EMRC_RHS (arg_info) = rc_undef;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * If this node happens to be a conditional,
     * traverse again!
     */
    if (INFO_EMRC_RHS (arg_info) == rc_cond) {
        INFO_EMRC_MODE (arg_info) = rc_else;

        n = FUNDEF_ARGS (INFO_EMRC_FUNDEF (arg_info));
        while (n != NULL) {
            ARG_AVIS (n) = RescueEnvironment (ARG_AVIS (n));
            n = ARG_NEXT (n);
        }

        n = BLOCK_VARDEC (FUNDEF_BODY (INFO_EMRC_FUNDEF (arg_info)));
        while (n != NULL) {
            VARDEC_AVIS (n) = RescueEnvironment (VARDEC_AVIS (n));
            n = VARDEC_NEXT (n);
        }

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }

        INFO_EMRC_RHS (arg_info) = rc_undef;
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        INFO_EMRC_MODE (arg_info) = rc_default;
    }

    /*
     * Insert ADJUST_RC prfs from DECLIST
     */
    ASSIGN_NEXT (arg_node) = MakeDecAssignments (arg_info, ASSIGN_NEXT (arg_node));

    /*
     * Insert ADJUST_RC prfs from INCLIST
     */
    ASSIGN_NEXT (arg_node) = MakeIncAssignments (arg_info, ASSIGN_NEXT (arg_node));

    /*
     * Insert Alloc_or_Reuse prfs before this N_assign
     */
    arg_node = MakeAllocAssignments (arg_info, arg_node);

    INFO_EMRC_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCblock
 *
 *  @brief traverse vardecs in order to initialize environments and
 *         traverses code
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCblock");

    /*
     * Traverse vardecs in order to initialize RC-Counters
     */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_ASSERT ((BLOCK_INSTR (arg_node) != NULL), "first instruction of block is NULL"
                                                   " (should be a N_empty node)");

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AnnotateBranches
 *
 *  @brief a helper function to EMRCcond which inserts ADJUST_RC statements
 *         for a given variable prior in front of each branches code
 *
 *  @param avis the given variable
 *  @param cond the conditional
 *  @param arg_info
 *
 *  @return the modified conditional
 *
 ***************************************************************************/
static node *
AnnotateBranches (node *avis, node *cond, info *arg_info)
{
    int m, t, e;

    DBUG_ENTER ("AnnotateBranches");

    DBUG_ASSERT (INFO_EMRC_DEPTH (arg_info) == 0, "Invalid DEPTH: != 0");

    e = GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
    avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
    avis = RestoreEnvironment (avis);

    t = GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
    avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
    avis = RestoreEnvironment (avis);

    if (e + t > 0) {
        m = e < t ? e : t;
        m = m > 1 ? m : 1;
        avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info), m);

        INFO_EMRC_INCLIST (arg_info)
          = IncListInsert (INFO_EMRC_INCLIST (arg_info), avis, t - m);
        BLOCK_INSTR (COND_THEN (cond))
          = MakeIncAssignments (arg_info, BLOCK_INSTR (COND_THEN (cond)));

        INFO_EMRC_INCLIST (arg_info)
          = IncListInsert (INFO_EMRC_INCLIST (arg_info), avis, e - m);
        BLOCK_INSTR (COND_ELSE (cond))
          = MakeIncAssignments (arg_info, BLOCK_INSTR (COND_ELSE (cond)));
    }
    DBUG_RETURN (avis);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCcond
 *
 *  @brief depending on INFO_EMRC_MODE (rc_default or rc_else) a
 *         conditionals then or else branches are traversed. When both
 *         branches have been traversed, adjust_rc instructions are inserted
 *         to make both branches behave equal from an external point of view.
 *
 *  @param arg_node the conditional
 *  @param arg_info
 *
 *  @return the modified conditional
 *
 ***************************************************************************/
node *
EMRCcond (node *arg_node, info *arg_info)
{
    node *n;
    node *avis;

    DBUG_ENTER ("EMRCcond");

    if (INFO_EMRC_MODE (arg_info) == rc_default) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

        if (NODE_TYPE (COND_COND (arg_node)) == N_id) {
            /*
             * Insert Adjust_RC for COND_COND into THEN-Branch
             */
            avis = ID_AVIS (COND_COND (arg_node));
            if (GetEnvironment (avis, GetDefLevel (avis)) == 0) {
                INFO_EMRC_DECLIST (arg_info)
                  = DecListInsert (INFO_EMRC_DECLIST (arg_info), avis,
                                   GetDefLevel (avis));
                avis = AddEnvironment (avis, GetDefLevel (avis), 1);
            }
        }

        /*
         * Prepend THEN branch with DEC-Assignments if necessary
         */
        BLOCK_INSTR (COND_THEN (arg_node))
          = MakeDecAssignments (arg_info, BLOCK_INSTR (COND_THEN (arg_node)));

    } else {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

        if (NODE_TYPE (COND_COND (arg_node)) == N_id) {
            /*
             * Insert Adjust_RC for COND_COND into ELSE-Branch
             */
            avis = ID_AVIS (COND_COND (arg_node));
            if (GetEnvironment (avis, GetDefLevel (avis)) == 0) {
                INFO_EMRC_DECLIST (arg_info)
                  = DecListInsert (INFO_EMRC_DECLIST (arg_info), avis,
                                   GetDefLevel (avis));
                avis = AddEnvironment (avis, GetDefLevel (avis), 1);
            }
        }

        /*
         * Prepend ELSE branch with DEC-Assignments if necessary
         */
        BLOCK_INSTR (COND_ELSE (arg_node))
          = MakeDecAssignments (arg_info, BLOCK_INSTR (COND_ELSE (arg_node)));

        /*
         * After both environments have been created,
         * annote missing ADJUST_RCs at the beginning of blocks and
         * simultaneously merge both environments
         */
        n = FUNDEF_ARGS (INFO_EMRC_FUNDEF (arg_info));
        while (n != NULL) {
            ARG_AVIS (n) = AnnotateBranches (ARG_AVIS (n), arg_node, arg_info);
            n = ARG_NEXT (n);
        }

        n = BLOCK_VARDEC (FUNDEF_BODY (INFO_EMRC_FUNDEF (arg_info)));
        while (n != NULL) {
            VARDEC_AVIS (n) = AnnotateBranches (VARDEC_AVIS (n), arg_node, arg_info);
            n = VARDEC_NEXT (n);
        }
    }

    INFO_EMRC_RHS (arg_info) = rc_cond;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCconst
 *
 *  @brief allocates memory iff this constand appears on a RHS of a LET-NODE
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCconst (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCconst");

    if (INFO_EMRC_RHS (arg_info) == rc_undef) {
        INFO_EMRC_RHS (arg_info) = rc_const;

        /*
         * Make memory allocation
         */
        INFO_EMRC_ALLOCLIST (arg_info)
          = MakeARList (INFO_EMRC_ALLOCLIST (arg_info),
                        IDS_AVIS (INFO_EMRC_LHS (arg_info)), MakeNum (1), MakeNum (0),
                        CreateZeroVector (0, T_int), NULL);

        DBUG_ASSERT (INFO_EMRC_REUSELIST (arg_info) == NULL,
                     "REUSELIST must have no members!!!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCfuncond
 *
 *  @brief traverses a funcond only to set INFO_EMRC_RHS to rc_funcond
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCfuncond");

    INFO_EMRC_RHS (arg_info) = rc_funcond;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCfundef
 *
 *  @brief traverses a fundef node by first initializing the argument
 *         variables and then traversing the functions block.
 *         After that, adjust_rc operations for the arguments are inserted.
 *
 *  @param fundef
 *  @param arg_info
 *
 *  @return fundef
 *
 ***************************************************************************/
node *
EMRCfundef (node *fundef, info *arg_info)
{
    node *arg;
    node *avis;

    DBUG_ENTER ("EMRCfundef");

    if (INFO_EMRC_MODE (arg_info) == rc_annotate_cfuns) {
        /*
         * special module name -> must be an external C-fun
         */
        if (((sbs == 1) && (strcmp (FUNDEF_MOD (fundef), EXTERN_MOD_NAME) == 0))
            || ((sbs == 0) && (FUNDEF_MOD (fundef) == NULL))) {
            FUNDEF_STATUS (fundef) = ST_Cfun;
        }
        if (FUNDEF_NEXT (fundef) != NULL) {
            FUNDEF_NEXT (fundef) = Trav (FUNDEF_NEXT (fundef), arg_info);
        }

        DBUG_RETURN (fundef);
    }

    INFO_EMRC_FUNDEF (arg_info) = fundef;
    INFO_EMRC_DEPTH (arg_info) = 0;
    INFO_EMRC_MODE (arg_info) = rc_default;

    /*
     * Traverse args in order to initialize refcounting environment
     */
    if (FUNDEF_ARGS (fundef) != NULL)
        FUNDEF_ARGS (fundef) = Trav (FUNDEF_ARGS (fundef), arg_info);

    /*
     * Traverse block
     */
    if (FUNDEF_BODY (fundef) != NULL) {
        FUNDEF_BODY (fundef) = Trav (FUNDEF_BODY (fundef), arg_info);

        /*
         * Annotate missing ADJUST_RCs
         */
        arg = FUNDEF_ARGS (fundef);
        while (arg != NULL) {
            avis = ARG_AVIS (arg);
            INFO_EMRC_INCLIST (arg_info)
              = IncListInsert (INFO_EMRC_INCLIST (arg_info), avis,
                               GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info)) - 1);
            avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));

            arg = ARG_NEXT (arg);
        }
        BLOCK_INSTR (FUNDEF_BODY (fundef))
          = MakeIncAssignments (arg_info, BLOCK_INSTR (FUNDEF_BODY (fundef)));

        PrintNode (fundef);
        /*
         * Restore SSA form
         */
        fundef = RestoreSSAOneFundef (fundef);
    }

    /*
     * Traverse other fundefs
     */
    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (fundef) = Trav (FUNDEF_NEXT (fundef), arg_info);
    }

    DBUG_RETURN (fundef);
}

/** <!--******************************************************************-->
 *
 * @fn EMRClet
 *
 *  @brief traverses the RHS and subsequently adds LHS identifiers to INCLIST
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRClet (node *arg_node, info *arg_info)
{
    ids *ids;
    node *avis;

    DBUG_ENTER ("EMRClet");

    /*
     * Remember LHS ids in order to be able to identify which
     * fun ap parameters must be refcounted like prf paramters using
     * PRAGMA_REFCOUNTING
     */
    INFO_EMRC_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    switch (INFO_EMRC_RHS (arg_info)) {

    case rc_with:
    case rc_prfap:
    case rc_const:
    case rc_array:

        /*
         * Wrap these instructions into a F_fill prf
         */
        ids = LET_IDS (arg_node);

        LET_EXPR (arg_node)
          = MakePrf (F_fill, MakeExprs (LET_EXPR (arg_node), Ids2Exprs (ids)));

        /*
         * By setting IDS_STATUS of LHS identifiers to ST_artificial,
         * SSATransform will mark these with SSAUNDOFLAG causing
         * UndoSSATransform to rename these into their original identifiers:
         *
         * Before UndoSSATransform:
         * a' = fill( ..., a);
         *
         * After UndoSSATransform:
         * a  = fill( ..., a);
         */
        while (ids != NULL) {
            IDS_STATUS (ids) = ST_artificial;
            ids = IDS_NEXT (ids);
        }

        /*
         * here is no break missing
         */
    case rc_funap:
        /*
         * Add all lhs ids to inclist
         */
        ids = LET_IDS (arg_node);

        while (ids != NULL) {
            /*
             * Subtraction needed because ALLOC / FunAp initializes RC with 1
             */
            avis = IDS_AVIS (ids);
            INFO_EMRC_INCLIST (arg_info)
              = IncListInsert (INFO_EMRC_INCLIST (arg_info), avis,
                               GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info)) - 1);
            avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));

            ids = IDS_NEXT (ids);
        }
        break;

    case rc_copy:
        /*
         * Copy assignment: a = b
         * Env(b) += Env(a)
         */
        avis = IDS_AVIS (LET_IDS (arg_node));
        IDS_AVIS (ID_IDS (LET_EXPR (arg_node)))
          = AddEnvironment (IDS_AVIS (ID_IDS (LET_EXPR (arg_node))),
                            INFO_EMRC_DEPTH (arg_info),
                            GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info)));
        avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
        break;

    case rc_funcond:
        /*
         * Treat FunCond like a variable Copy assignment
         */
        switch (INFO_EMRC_MODE (arg_info)) {
        case rc_default:
            avis = ID_AVIS (EXPRS_EXPR (FUNCOND_THEN (LET_EXPR (arg_node))));
            avis = SetDefLevel (avis, INFO_EMRC_DEPTH (arg_info));
            avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info),
                                   GetEnvironment (IDS_AVIS (LET_IDS (arg_node)),
                                                   INFO_EMRC_DEPTH (arg_info)));
            IDS_AVIS (LET_IDS (arg_node)) = PopEnvironment (IDS_AVIS (LET_IDS (arg_node)),
                                                            INFO_EMRC_DEPTH (arg_info));
            IDS_AVIS (LET_IDS (arg_node))
              = RestoreEnvironment (IDS_AVIS (LET_IDS (arg_node)));
            break;

        case rc_else:
            avis = ID_AVIS (EXPRS_EXPR (FUNCOND_ELSE (LET_EXPR (arg_node))));
            avis = SetDefLevel (avis, INFO_EMRC_DEPTH (arg_info));
            avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info),
                                   GetEnvironment (IDS_AVIS (LET_IDS (arg_node)),
                                                   INFO_EMRC_DEPTH (arg_info)));
            IDS_AVIS (LET_IDS (arg_node)) = PopEnvironment (IDS_AVIS (LET_IDS (arg_node)),
                                                            INFO_EMRC_DEPTH (arg_info));
            break;
        default:
            DBUG_ASSERT (FALSE, "Cannot happen");
        }
        break;
    default:
        Print (arg_node);
        DBUG_ASSERT (FALSE, "Cannot happen");
    }
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCicm
 *
 *  @brief ICMs introduced by IVE require a special treatment (see below)
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCicm (node *arg_node, info *arg_info)
{
    char *name;
    node *avis;

    DBUG_ENTER ("EMRCicm");

    INFO_EMRC_RHS (arg_info) = rc_icm;

    name = ICM_NAME (arg_node);

    if (strstr (name, "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET( off_nt, wl_nt)
         * does *not* consume its arguments! It is expanded to
         *      off_nt = wl_nt__off    ,
         * where 'off_nt' is a scalar and 'wl_nt__off' an internal variable!
         *   -> store actual RC of the first argument (defined)
         *   -> do NOT traverse the second argument (used)
         */
        avis = IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node)));
        INFO_EMRC_ALLOCLIST (arg_info)
          = MakeARList (INFO_EMRC_ALLOCLIST (arg_info), avis, MakeNum (1), MakeNum (0),
                        CreateZeroVector (0, T_int), NULL);
        INFO_EMRC_INCLIST (arg_info)
          = IncListInsert (INFO_EMRC_INCLIST (arg_info), avis,
                           GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info)));
        avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
    } else {
        if (strstr (name, "VECT2OFFSET") != NULL) {
            /*
             * VECT2OFFSET( off_nt, ., from_nt, ., ., ...)
             * needs RC on all but the first argument. It is expanded to
             *     off_nt = ... from_nt ...    ,
             * where 'off_nt' is a scalar variable.
             *  -> store actual RC of the first argument (defined)
             *  -> traverse all but the first argument (used)
             *  -> handle ICM like a prf (RCO)
             */
            avis = IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node)));
            INFO_EMRC_ALLOCLIST (arg_info)
              = MakeARList (INFO_EMRC_ALLOCLIST (arg_info), avis, MakeNum (1),
                            MakeNum (0), CreateZeroVector (0, T_int), NULL);
            INFO_EMRC_INCLIST (arg_info)
              = IncListInsert (INFO_EMRC_INCLIST (arg_info), avis,
                               GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info)));
            avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
            ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);
        } else {
            if (strstr (name, "IDXS2OFFSET") != NULL) {
                /*
                 * IDXS2OFFSET( off_nt, ., idx_1_nt ... idx_n_nt, ., ...)
                 * needs RC on all but the first argument. It is expanded to
                 *     off_nt = ... idx_1_nt[i] ... idx_n_nt[i] ...   ,
                 * where 'off_nt' is a scalar variable.
                 *  -> store actual RC of the first argument (defined)
                 *  -> traverse all but the first argument (used)
                 *  -> handle ICM like a prf (RCO)
                 */
                avis = IDS_AVIS (ID_IDS (ICM_ARG1 (arg_node)));
                INFO_EMRC_ALLOCLIST (arg_info)
                  = MakeARList (INFO_EMRC_ALLOCLIST (arg_info), avis, MakeNum (1),
                                MakeNum (0), CreateZeroVector (0, T_int), NULL);
                INFO_EMRC_INCLIST (arg_info)
                  = IncListInsert (INFO_EMRC_INCLIST (arg_info), avis,
                                   GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info)));
                avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));

                ICM_EXPRS2 (arg_node) = Trav (ICM_EXPRS2 (arg_node), arg_info);
            } else {
                DBUG_ASSERT ((0), "unknown ICM found during EMRC");
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCid
 *
 *  @brief traverses RHS identifiers. Depending on the surrounding construct,
 *         different actions are done. (see comments below)
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("EMRCid");

    avis = ID_AVIS (arg_node);

    switch (INFO_EMRC_RHS (arg_info)) {

    case rc_undef:
        INFO_EMRC_RHS (arg_info) = rc_copy;
        break;

    case rc_return:
        /*
         * Add one to the environment at the current level (level 0)
         */
        avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info), 1);
        break;

    case rc_cexprs:
        /*
         * If DEFLEVEL of CEXPR is undefinded, set it to current CODELEVEL
         * as this can only happen if the variable has
         * been defined in the current CBLOCK
         */
        avis = SetDefLevel (avis, INFO_EMRC_DEPTH (arg_info));

        /*
         * Here is no break missing
         */
    case rc_array:
    case rc_cond:
    case rc_icm:
    case rc_with:
        /*
         * Add one to the environment at the variable's DEFLEVEL iff it is zero
         * and put the variable into DECLIST
         */
        if (GetEnvironment (avis, GetDefLevel (avis)) == 0) {
            INFO_EMRC_DECLIST (arg_info)
              = DecListInsert (INFO_EMRC_DECLIST (arg_info), avis, GetDefLevel (avis));
            avis = AddEnvironment (avis, GetDefLevel (avis), 1);
        }
        break;

    case rc_prfap:
        /*
         * Add one to the environment at the variable's DEFLEVEL iff it is zero
         * and put the variable into DECLIST
         */
        if (GetEnvironment (avis, GetDefLevel (avis)) == 0) {
            INFO_EMRC_DECLIST (arg_info)
              = DecListInsert (INFO_EMRC_DECLIST (arg_info), avis, GetDefLevel (avis));
            avis = AddEnvironment (avis, GetDefLevel (avis), 1);

            /*
             * Potentially, this N_id could be reused.
             *  -> Add it to REUSELIST
             */
            INFO_EMRC_REUSELIST (arg_info)
              = AppendExprs (INFO_EMRC_REUSELIST (arg_info),
                             MakeExprs (DupNode (arg_node), NULL));
        }
        break;

    case rc_funcond:
    case rc_funap:
    case rc_const:
    case rc_copy:
        Print (arg_node);
        DBUG_ASSERT (FALSE, "No ID node must appear in this context");
    }
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AnnotateCBlock
 *
 *  @brief Helper function to EMRCNcode which annotates ADJUST_RC
 *         statement at a block's beginning
 *
 *  @param avis
 *  @param cblock
 *  @param arg_info
 *
 *  @return the modified cblock
 *
 ***************************************************************************/
static node *
AnnotateCBlock (node *avis, node *cblock, info *arg_info)
{
    int env;

    DBUG_ENTER ("AnnotateCBlock");

    env = GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
    avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));

    INFO_EMRC_INCLIST (arg_info)
      = IncListInsert (INFO_EMRC_INCLIST (arg_info), avis, env);

    BLOCK_INSTR (cblock) = MakeIncAssignments (arg_info, BLOCK_INSTR (cblock));

    DBUG_RETURN (cblock);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCNcode
 *
 *  @brief traverses a with-loop's code and inserts ADJUST_RCs before and
 *         after (in NCODE_EPILOGUE) the code block
 *
 *  @param arg_node
 *  @param arg_ino
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCNcode (node *arg_node, info *arg_info)
{
    node *n, *epicode;

    DBUG_ENTER ("EMRCNcode");

    /*
     * Traverse CEXPRS and insert adjust_rc operations into
     * NCODE_EPILOGUE
     */
    INFO_EMRC_RHS (arg_info) = rc_cexprs;
    NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);

    DBUG_ASSERT (NCODE_EPILOGUE (arg_node) == NULL, "Epilogue must not exist yet!");

    /*
     * Insert ADJUST_RC prfs from DECLIST into EPILOGUE
     */
    epicode = MakeDecAssignments (arg_info, NULL);
    if (epicode != NULL)
        NCODE_EPILOGUE (arg_node) = MakeBlock (epicode, NULL);

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    /*
     * Prepend block with Adjust_RC prfs
     */
    n = FUNDEF_ARGS (INFO_EMRC_FUNDEF (arg_info));
    while (n != NULL) {
        NCODE_CBLOCK (arg_node)
          = AnnotateCBlock (ARG_AVIS (n), NCODE_CBLOCK (arg_node), arg_info);

        n = ARG_NEXT (n);
    }

    n = BLOCK_VARDEC (FUNDEF_BODY (INFO_EMRC_FUNDEF (arg_info)));
    while (n != NULL) {
        NCODE_CBLOCK (arg_node)
          = AnnotateCBlock (VARDEC_AVIS (n), NCODE_CBLOCK (arg_node), arg_info);

        n = VARDEC_NEXT (n);
    }

    /*
     * count the references in next code
     */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AllocateWithID
 *
 *  @brief Helper function to EMRCNWith(2) which allocates memory for
 *         the index variables
 *
 *  @param withid
 *  @param arg_info
 *  @param iv_shp shape of the index vector
 *
 *  @return withid
 *
 ***************************************************************************/
static node *
AllocateWithID (node *withid, info *arg_info, node *iv_shp)
{
    ids *i;
    node *avis;

    DBUG_ENTER ("AllocateWithID");

    /*
     * Allocate the index vector and make sure to have an ADJUST_RC( iv, -1)
     * after the with-loop such that iv is deallocated
     */
    avis = IDS_AVIS (NWITHID_VEC (withid));
    INFO_EMRC_ALLOCLIST (arg_info) = MakeARList (INFO_EMRC_ALLOCLIST (arg_info), avis,
                                                 MakeNum (1), MakeNum (1), iv_shp, NULL);
    if (GetEnvironment (avis, GetDefLevel (avis)) == 0) {
        INFO_EMRC_DECLIST (arg_info)
          = DecListInsert (INFO_EMRC_DECLIST (arg_info), avis, GetDefLevel (avis));
        avis = AddEnvironment (avis, GetDefLevel (avis), 1);
    }
    avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info) - 1);

    /*
     * Allocate the index variables and make sure to have an ADJUST_RC( x, -1)
     * after the with-loop such that they are deallocated
     */
    i = NWITHID_IDS (withid);
    while (i != NULL) {
        avis = IDS_AVIS (i);
        INFO_EMRC_ALLOCLIST (arg_info)
          = MakeARList (INFO_EMRC_ALLOCLIST (arg_info), avis, MakeNum (1), MakeNum (0),
                        CreateZeroVector (0, T_int), NULL);
        if (GetEnvironment (avis, GetDefLevel (avis)) == 0) {
            INFO_EMRC_DECLIST (arg_info)
              = DecListInsert (INFO_EMRC_DECLIST (arg_info), avis, GetDefLevel (avis));
            avis = AddEnvironment (avis, GetDefLevel (avis), 1);
        }
        avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info) - 1);
        i = i->next;
    }

    DBUG_RETURN (withid);
}

/** <!--******************************************************************-->
 *
 * @fn MakeWithAlloc
 *
 *  @brief Helper function to EMRCNwith(2) which allocates memory for
 *         the result array
 *
 *  @param als ALLOCLIST
 *  @param ids the current WL-result
 *  @param withop the current withop
 *  @param cexpr the current cexpr
 *  @param arg_info
 *
 *  @return a new ALLOCLIST
 *
 ***************************************************************************/
static ar_list_struct *
MakeWithAlloc (ar_list_struct *als, ids *ids, node *withop, node *cexpr, info *arg_info)
{
    DBUG_ENTER ("MakeWithAllocation");

    switch (NWITHOP_TYPE (withop)) {
    case WO_genarray:
        if (KNOWN_SHAPE (GetShapeDim (IDS_TYPE (ids)))) {
            als
              = MakeARList (als, IDS_AVIS (ids), MakeNum (1),

                            MakeNum (GetShapeDim (IDS_TYPE (ids))),

                            Shpseg2Array (IDS_SHPSEG (ids), GetShapeDim (IDS_TYPE (ids))),

                            FilterReuseList (INFO_EMRC_REUSELIST (arg_info),
                                             INFO_EMRC_DECLIST (arg_info), ids));
        } else {
            if (KNOWN_SHAPE (GetShapeDim (ID_TYPE (cexpr)))) {
                als = MakeARList (als, IDS_AVIS (ids), MakeNum (1),

                                  MakePrf2 (F_add_SxS,
                                            MakePrf2 (F_sel, MakeNum (0),
                                                      MakePrf (F_shape,
                                                               DupNode (NWITHOP_SHAPE (
                                                                 withop)))),
                                            MakeNum (GetShapeDim (ID_TYPE (cexpr)))),

                                  MakePrf2 (F_cat_VxV, DupNode (NWITHOP_SHAPE (withop)),
                                            Shpseg2Array (ID_SHPSEG (cexpr),
                                                          GetShapeDim (ID_TYPE (cexpr)))),

                                  FilterReuseList (INFO_EMRC_REUSELIST (arg_info),
                                                   INFO_EMRC_DECLIST (arg_info), ids));
            } else {
                als = MakeARList (als, IDS_AVIS (ids), MakeNum (1),

                                  MakePrf2 (F_add_SxS,
                                            MakePrf2 (F_sel, MakeNum (0),
                                                      MakePrf (F_shape,
                                                               DupNode (NWITHOP_SHAPE (
                                                                 withop)))),
                                            MakePrf (F_dim,
                                                     DupNode (NWITHOP_DEFAULT (withop)))),

                                  MakePrf (F_shape,
                                           MakePrf2 (F_genarray,
                                                     DupNode (NWITHOP_SHAPE (withop)),
                                                     DupNode (NWITHOP_DEFAULT (withop)))),

                                  FilterReuseList (INFO_EMRC_REUSELIST (arg_info),
                                                   INFO_EMRC_DECLIST (arg_info), ids));
            }
        }
        break;

    case WO_modarray:
        als = MakeARList (als, IDS_AVIS (ids), MakeNum (1),

                          MakePrf (F_dim, DupNode (NWITHOP_ARRAY (withop))),

                          MakePrf (F_shape, DupNode (NWITHOP_ARRAY (withop))),

                          FilterReuseList (INFO_EMRC_REUSELIST (arg_info),
                                           INFO_EMRC_DECLIST (arg_info), ids));
        break;

    case WO_foldfun:
    case WO_foldprf:
        break;

    case WO_unknown:
        DBUG_ASSERT (FALSE, "non initialised WithOpType found.");
        break;
    }

    DBUG_RETURN (als);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCNwith
 *
 *  @brief traverses a withloop and thereby allocates memory for the index
 *         variables and the result
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCNwith (node *arg_node, info *arg_info)
{
    ids *ids;
    node *withop;
    node *cexprs;

    DBUG_ENTER ("EMRCNwith");

    /*
     * LHS must be rescued here in order to be accessible after
     * traversal of With-loop
     */
    ids = INFO_EMRC_LHS (arg_info);

    INFO_EMRC_DEPTH (arg_info) += 1;

    NWITH_WITHID (arg_node) = Trav (NWITH_WITHID (arg_node), arg_info);

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    NWITH_WITHID (arg_node)
      = AllocateWithID (NWITH_WITHID (arg_node), arg_info,
                        MakePrf (F_shape, DupNode (NWITH_SHAPE (arg_node))));

    INFO_EMRC_DEPTH (arg_info) -= 1;

    INFO_EMRC_RHS (arg_info) = rc_with;

    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /*
     * Make memory allocation
     */
    withop = NWITH_WITHOP (arg_node);
    cexprs = NCODE_CEXPRS (NWITH_CODE (arg_node));

    /*
     * REUSELIST must be empty
     */
    DBUG_ASSERT (INFO_EMRC_REUSELIST (arg_info) == NULL, "REUSELIST must be empty!");

    while (ids != NULL) {

        /*
         * Annotate allocation in ALLOCLIST
         */
        INFO_EMRC_ALLOCLIST (arg_info)
          = MakeWithAlloc (INFO_EMRC_ALLOCLIST (arg_info), ids, withop,
                           EXPRS_EXPR (cexprs), arg_info);

        ids = IDS_NEXT (ids);
        withop = NWITHOP_NEXT (withop);
        cexprs = EXPRS_NEXT (cexprs);
    }

    INFO_EMRC_RHS (arg_info) = rc_with;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCNwith2
 *
 *  @brief traverses a withloop and thereby allocates memory for the index
 *         variables and the result
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCNwith2 (node *arg_node, info *arg_info)
{
    ids *ids;
    node *withop;
    node *cexprs;

    DBUG_ENTER ("EMRCNwith2");

    /*
     * LHS must be rescued here in order to be accessible after
     * traversal of With-loop
     */
    ids = INFO_EMRC_LHS (arg_info);

    INFO_EMRC_DEPTH (arg_info) += 1;

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    if (NWITH2_CODE (arg_node) != NULL)
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    NWITH2_WITHID (arg_node)
      = AllocateWithID (NWITH2_WITHID (arg_node), arg_info,
                        MakePrf (F_shape, DupNode (NWITH2_SHAPE (arg_node))));

    INFO_EMRC_DEPTH (arg_info) -= 1;

    INFO_EMRC_RHS (arg_info) = rc_with;

    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    /*
     * Make memory allocation
     */
    withop = NWITH2_WITHOP (arg_node);
    cexprs = NCODE_CEXPRS (NWITH2_CODE (arg_node));

    while (ids != NULL) {
        /*
         * Get reuse candidates
         * Some nasty things happen with DFM.
         * They should probably be rebuilt here.
         */
        INFO_EMRC_REUSELIST (arg_info) = NULL;
        /*
         * GetReuseCandidates( arg_node,
         *                     INFO_EMRC_FUNDEF( arg_info),
         *                     ids);
         */

        /*
         * Annotate allocation in ALLOCLIST
         */
        INFO_EMRC_ALLOCLIST (arg_info)
          = MakeWithAlloc (INFO_EMRC_ALLOCLIST (arg_info), ids, withop,
                           EXPRS_EXPR (cexprs), arg_info);

        /*
         * Free REUSELIST
         */
        if (INFO_EMRC_REUSELIST (arg_info) != NULL) {
            INFO_EMRC_REUSELIST (arg_info) = FreeTree (INFO_EMRC_REUSELIST (arg_info));
        }

        ids = IDS_NEXT (ids);
        withop = NWITHOP_NEXT (withop);
        cexprs = EXPRS_NEXT (cexprs);
    }

    INFO_EMRC_RHS (arg_info) = rc_with;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCNwithid
 *
 *  @brief annotates the index variables' definition level as
 *         current code level - 1
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCNwithid (node *arg_node, info *arg_info)
{
    ids *iv_ids;
    DBUG_ENTER ("EMRCNwithid");

    /* IV must be refcounted like if it was taken into the WL
       from a higher level */
    AVIS_EMRC_DEFLEVEL (IDS_AVIS (NWITHID_VEC (arg_node)))
      = INFO_EMRC_DEPTH (arg_info) - 1;

    iv_ids = NWITHID_IDS (arg_node);
    while (iv_ids != NULL) {
        AVIS_EMRC_DEFLEVEL (IDS_AVIS (iv_ids)) = INFO_EMRC_DEPTH (arg_info) - 1;
        iv_ids = IDS_NEXT (iv_ids);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCprf
 *
 *  @brief traverses a prf's arguments and allocates memory for the result
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCprf");

    INFO_EMRC_RHS (arg_info) = rc_prfap;

    DBUG_ASSERT (INFO_EMRC_REUSELIST (arg_info) == NULL, "REUSELIST MUST BE EMPTY!!!");

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }

    /*
     * Make memory allocation
     */
#define ALLOC(dim, shape)                                                                \
    INFO_EMRC_ALLOCLIST (arg_info)                                                       \
      = MakeARList (INFO_EMRC_ALLOCLIST (arg_info), IDS_AVIS (INFO_EMRC_LHS (arg_info)), \
                    MakeNum (1), dim, shape,                                             \
                    FilterReuseList (INFO_EMRC_REUSELIST (arg_info),                     \
                                     INFO_EMRC_DECLIST (arg_info),                       \
                                     INFO_EMRC_LHS (arg_info)))

    switch (PRF_PRF (arg_node)) {
    case F_dim:
        /*
         * dim( A );
         * alloc_or_reuse( 1, 0, [] );
         */
        ALLOC (MakeNum (0), CreateZeroVector (0, T_int));
        break;
    case F_shape:
        /*
         * shape( A );
         * alloc_or_reuse( 1, 1, shape( shape( A )))
         */
        ALLOC (MakeNum (1), MakePrf (F_shape, DupTree (arg_node)));
        break;
    case F_reshape:
        /*
         * reshape( sh, A );
         * alloc_or_reuse( 1, shape( sh )[0], sh );
         */
        ALLOC (MakePrf2 (F_sel, MakeNum (0),
                         MakePrf (F_shape, DupNode (PRF_ARG1 (arg_node)))),
               DupNode (PRF_ARG1 (arg_node)));
        break;
    case F_drop_SxV:
    case F_take_SxV:
    case F_cat_VxV:
        /*
         * drop( dv, V );
         * alloc_or_reuse( 1, 1, shape( drop( dv, V )));
         * take( tv, V );
         * alloc_or_reuse( 1, 1, shape( take( tv, V )));
         * cat( V1, V2 );
         * alloc_or_reuse( 1, 1, shape( cat( V1, V2 )));
         */
        ALLOC (MakeNum (1), MakePrf (F_shape, DupTree (arg_node)));
        break;
    case F_sel:
        /*
         * sel( iv, A );
         * alloc_or_reuse( 1, dim(A) - shape(iv)[0], shape( sel( iv, A)))
         */
        ALLOC (MakePrf2 (F_sub_SxS, MakePrf (F_dim, DupNode (PRF_ARG2 (arg_node))),
                         MakePrf2 (F_sel, MakeNum (0),
                                   MakePrf (F_shape, DupNode (PRF_ARG1 (arg_node))))),
               MakePrf (F_shape, DupNode (arg_node)));
        break;

    case F_idx_sel:
        /*
         * idx_sel can only occur when the result shape is known!!!
         * For some reason, there is a special IDXSEL-Macro
         *
         * a = idx_sel( idx, A);
         * alloc_or_reuse( 1, dim( a), shape( idx_sel( idx, A)));
         */
        ALLOC (MakeNum (GetDim (IDS_TYPE (INFO_EMRC_LHS (arg_info)))),
               MakePrf (F_shape, DupNode (arg_node)));
        break;

    case F_modarray:
    case F_idx_modarray:
        /*
         * idx_modarray( A, idx, val);
         * alloc_or_reuse( 1, dim( A ), shape ( A ));
         */
        ALLOC (MakePrf (F_dim, DupNode (PRF_ARG1 (arg_node))),
               MakePrf (F_shape, DupNode (PRF_ARG1 (arg_node))));
        break;

    case F_add_SxS:
    case F_sub_SxS:
    case F_mul_SxS:
    case F_div_SxS:
    case F_toi_S:
    case F_tof_S:
    case F_tod_S:
        /*
         * single scalar operations
         * alloc_or_reuse( 1, 0, [])
         */
        ALLOC (MakeNum (0), CreateZeroVector (0, T_int));
        break;

    case F_mod:
    case F_min:
    case F_max:
    case F_neg:
    case F_not:
    case F_abs:
    case F_and:
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_neq:
    case F_ge:
    case F_gt:
    case F_toi_A:
    case F_tof_A:
    case F_tod_A:
    case F_add_AxS:
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
    case F_add_AxA:
    case F_mul_AxA:
    case F_sub_AxA:
    case F_div_AxA:
    case F_add_SxA:
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
        if ((CountExprs (PRF_ARGS (arg_node)) < 2)
            || (NODE_TYPE (PRF_ARG2 (arg_node)) != N_id)
            || (GetShapeDim (ID_TYPE (PRF_ARG2 (arg_node))) == SCALAR)) {
            ALLOC (MakePrf (F_dim, DupNode (PRF_ARG1 (arg_node))),
                   MakePrf (F_shape, DupNode (PRF_ARG1 (arg_node))));
        } else {
            ALLOC (MakePrf (F_dim, DupNode (PRF_ARG2 (arg_node))),
                   MakePrf (F_shape, DupNode (PRF_ARG2 (arg_node))));
        }
        break;

    case F_alloc_or_reuse:
    case F_free:
    case F_inc_rc:
    case F_dec_rc:
    case F_fill:
        DBUG_ASSERT ((0), "invalid prf found!");
        break;

    case F_take:
    case F_drop:
    case F_cat:
    case F_rotate:
    case F_genarray:
        DBUG_ASSERT ((0), "Non-instrinsic primitive functions not implemented!"
                          " Use array.lib instead!");
        break;

        /*
         *  otherwise
         */

    case F_type_error:
    case F_to_unq:
    case F_from_unq:
        break;

    default:
        DBUG_ASSERT ((0), "unknown prf found!");
        break;
    }

#undef ALLOC

    if (INFO_EMRC_REUSELIST (arg_info) != NULL) {
        INFO_EMRC_REUSELIST (arg_info) = FreeTree (INFO_EMRC_REUSELIST (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCreturn
 *
 *  @brief sets INFO_EMRC_RHS to rc_return and
 *         traverses the returned identifiers
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCreturn");

    INFO_EMRC_RHS (arg_info) = rc_return;

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCvardec
 *
 *  @brief initializes a vardec's environment
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCvardec");

    VARDEC_AVIS (arg_node) = InitializeEnvironment (VARDEC_AVIS (arg_node), -1);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRefCount
 *
 *  @brief Starting function of EM based reference counting inference
 *
 *  @param syntax_tree
 *
 *  @return modified synatx tree containing explicit memory management
 *          instructions
 *
 ***************************************************************************/
node *
EMRefCount (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMRefCount");

    info = MakeInfo ();

    INFO_EMRC_INCLIST (info) = NULL;
    INFO_EMRC_DECLIST (info) = NULL;
    INFO_EMRC_ALLOCLIST (info) = NULL;
    INFO_EMRC_REUSELIST (info) = NULL;

    act_tab = emrefcnt_tab;
    INFO_EMRC_MODE (info) = rc_annotate_cfuns;
    syntax_tree = Trav (syntax_tree, info);

    INFO_EMRC_MODE (info) = rc_default;
    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
/*@}*/
/*@}*/ /* defgroup ssarc */
