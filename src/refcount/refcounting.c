/*
 *
 * $Log$
 * Revision 1.20  2004/10/22 15:39:19  ktr
 * Added support for F_reuse.
 *
 * Revision 1.19  2004/10/15 11:41:34  ktr
 * Removed cutreuse functionality. This is now done by filterrc.c.
 * Refcounting now works transparently over condfun boundaries.
 *
 * Revision 1.18  2004/10/11 14:47:54  ktr
 * Moved RCopts to rcphase.
 *
 * Revision 1.17  2004/10/10 09:55:45  ktr
 * Reference counting now works transparently over CONDFUN boundaries,
 * using the scheme presented in IFL04 paper.
 *
 * Revision 1.16  2004/09/30 19:53:49  sah
 * made rc_counter visible
 *
 * Revision 1.15  2004/08/13 09:21:43  ktr
 * Members of INFO structure are now properly initialized.
 *
 * Revision 1.14  2004/08/11 13:14:27  ktr
 * Superfluous reuse candidates are removed by EMRC now.
 *
 * Revision 1.13  2004/08/10 13:30:45  ktr
 * alloc_or_reuse initial rc is now set to one.
 *
 * Revision 1.12  2004/08/05 16:09:50  ktr
 * Scalar with-loops are now treated as they always were. By using the
 * F_wl_assign abstraction we can now explicitly refcount this case.
 *
 * Revision 1.11  2004/08/05 11:38:53  ktr
 * Support for NWITHID_VECNEEDED added.
 *
 * Revision 1.10  2004/07/31 21:27:06  ktr
 * corrected artificial argument inference
 *
 * Revision 1.9  2004/07/29 12:12:41  ktr
 * Refcounting of external functions now even works with global objects.
 *
 * Revision 1.8  2004/07/28 08:53:04  ktr
 * unneeded variable removed from EMRCicm.
 *
 * Revision 1.7  2004/07/27 13:05:07  ktr
 * ICMs now use their first argument like primitive functions.
 *
 * Revision 1.6  2004/07/23 08:49:48  ktr
 * fill operations and genarray/modarray withloops now behave like true
 * function applications with respect to the memory used.
 *
 * Revision 1.5  2004/07/19 14:53:38  ktr
 * Fixed a bug related to reference counting of conditionals.
 *
 * Revision 1.4  2004/07/19 12:39:37  ktr
 * Traversals for Nwithid, funcond and array reintroduced.
 *
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
 * @ingroup rcp
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
 *  Enumeration of the different modes of EMRefcounting.
 *
 ***************************************************************************/
typedef enum { rc_default, rc_else } rc_mode;

/** <!--******************************************************************-->
 *
 *  Enumeration of hthe different counting modes for N_id nodes.
 *
 ***************************************************************************/
typedef enum { rc_unknown, rc_apuse, rc_prfuse, rc_conduse, rc_retuse } rc_countmode;

/** <!--******************************************************************-->
 *
 *  Structure used for DEFLIST and USELIST.
 *
 ***************************************************************************/
typedef struct RC_LIST_STRUCT {
    node *avis;
    int count;
    struct RC_LIST_STRUCT *next;
} rc_list_struct;

/** <!--******************************************************************-->
 *
 *  Structure used for the enviroments of a variable.
 *
 ***************************************************************************/
struct RC_COUNTER {
    int depth;
    int count;
    struct RC_COUNTER *next;
};

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
    rc_list_struct *uselist;
    rc_list_struct *deflist;
    rc_countmode countmode;
    bool cond;
    bool mustcount;
    node *fundef;
    ids *lhs;
    node *condargs;
    node *retvals;
    node *retvals2;
    node *retenv;
    node *argenv;
};

/**
 * INFO macros
 */
#define INFO_EMRC_MODE(n) (n->mode)
#define INFO_EMRC_DEPTH(n) (n->depth)
#define INFO_EMRC_USELIST(n) (n->uselist)
#define INFO_EMRC_DEFLIST(n) (n->deflist)
#define INFO_EMRC_COUNTMODE(n) (n->countmode)
#define INFO_EMRC_COND(n) (n->cond)
#define INFO_EMRC_MUSTCOUNT(n) (n->mustcount)
#define INFO_EMRC_FUNDEF(n) (n->fundef)
#define INFO_EMRC_LHS(n) (n->lhs)
#define INFO_EMRC_CONDARGS(n) (n->condargs)
#define INFO_EMRC_RETVALS(n) (n->retvals)
#define INFO_EMRC_RETVALS2(n) (n->retvals2)
#define INFO_EMRC_RETENV(n) (n->retenv)
#define INFO_EMRC_ARGENV(n) (n->argenv)

/**
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_EMRC_MODE (result) = rc_default;
    INFO_EMRC_DEPTH (result) = 0;
    INFO_EMRC_COUNTMODE (result) = rc_unknown;
    INFO_EMRC_COND (result) = FALSE;
    INFO_EMRC_USELIST (result) = NULL;
    INFO_EMRC_DEFLIST (result) = NULL;
    INFO_EMRC_FUNDEF (result) = fundef;
    INFO_EMRC_LHS (result) = NULL;
    INFO_EMRC_CONDARGS (result) = NULL;
    INFO_EMRC_RETVALS (result) = NULL;
    INFO_EMRC_RETVALS2 (result) = NULL;
    INFO_EMRC_RETENV (result) = NULL;
    INFO_EMRC_ARGENV (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--******************************************************************-->
 *
 * @fn EMRefCount
 *
 *  @brief Starting function of EM based reference counting inference.
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree containing explicit memory management
 *          instructions
 *
 ***************************************************************************/
node *
EMRefCount (node *syntax_tree)
{
    DBUG_ENTER ("EMRefCount");

    DBUG_PRINT ("EMRC", ("Starting annotating C-functions..."));

    act_tab = emacf_tab;
    syntax_tree = Trav (syntax_tree, NULL);

    DBUG_PRINT ("EMRC", ("Annotating C-functions done."));

    DBUG_PRINT ("EMRC", ("Starting reference counting inference..."));

    act_tab = emrefcnt_tab;
    syntax_tree = Trav (syntax_tree, NULL);

    DBUG_PRINT ("EMRC", ("Reference counting inference complete."));

    DBUG_RETURN (syntax_tree);
}

/*****************************************************************************
 *
 * HELPER FUNCTION
 *
 ****************************************************************************/

/** <!--*******************************************************************-->
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

/** <!--******************************************************************-->
 *
 * @name USELIST FUNCTIONS
 *
 * <!--
 * -->
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn UseListHasNext
 *
 *  @brief returns whether the next element in USELIST (if any)
 *         has a certain depth
 *
 *  @param declist
 *  @param depth
 *
 *  @return ((rcls != NULL) && (declist->count == depth))
 *
 ***************************************************************************/
static bool
UseListHasNext (rc_list_struct *uselist, int depth)
{
    return ((uselist != NULL) && (uselist->count == depth));
}

/** <!--******************************************************************-->
 *
 * @fn UseListInsert
 *
 *  @brief Inserts a list element representing an dec_rc(x) into
 *         UseList
 *
 *  @param uselist
 *  @param avis
 *  @param depth
 *
 *  @return the modified uselist
 *
 ***************************************************************************/
static rc_list_struct *
UseListInsert (rc_list_struct *uselist, node *avis, int depth)
{
    DBUG_ENTER ("UseListInsert");

    if ((uselist == NULL) || (uselist->count < depth)) {
        uselist = MakeRCList (avis, depth, uselist);
    } else {
        uselist->next = UseListInsert (uselist->next, avis, depth);
    }
    DBUG_RETURN (uselist);
}

/** <!--******************************************************************-->
 *
 * @fn UseListContains
 *
 *  @brief returns whether USELIST contains an dec_rc( x) instruction
 *         at codelevel depth
 *
 *  @param uselist
 *  @param avis
 *  @param depth
 *
 *  @return whether an element of uselist refers to node avis
 *          at codelevel depth
 *
 ***************************************************************************/
static bool
UseListContains (rc_list_struct *uselist, node *avis, int depth)
{
    bool res;
    DBUG_ENTER ("UseListContains");

    if (uselist == NULL) {
        res = FALSE;
    } else {
        if ((uselist->avis == avis) && (uselist->count == depth)) {
            res = TRUE;
        } else {
            res = UseListContains (uselist->next, avis, depth);
        }
    }
    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn UseListGetNext
 *
 *  @brief returns a copy of the first element from USELIST with count == -1
 *
 *  @param uselist
 *
 *  @return a copy of the first element from USELIST with count == -1
 *
 ***************************************************************************/
static rc_list_struct *
UseListGetNext (rc_list_struct *uselist)
{
    DBUG_ENTER ("UseListGetNext");
    DBUG_RETURN (MakeRCList (uselist->avis, -1, NULL));
}

/** <!--******************************************************************-->
 *
 * @fn UseListPopNext
 *
 *  @brief removes the first element from USELIST
 *
 *  @param USElist
 *
 *  @return USElist without the first element
 *
 ***************************************************************************/
static rc_list_struct *
UseListPopNext (rc_list_struct *uselist)
{
    rc_list_struct *res;

    DBUG_ENTER ("UseListPopNext");

    res = uselist->next;
    Free (uselist);

    DBUG_RETURN (res);
}

/*@}*/

/** <!--*******************************************************************-->
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
 * @fn InitializeEnvironment
 *
 *  @brief Initilizes a variable's environment with DEFLEVEL l
 *
 *  @param avis contains the environment
 *  @param deflevel
 *  @param start
 *
 *  @return the initialized avis node
 *
 ***************************************************************************/
static node *
InitializeEnvironment (node *avis, int deflevel, int start)
{
    DBUG_ENTER ("InitializeEnvironment");

    AVIS_EMRC_DEFLEVEL (avis) = deflevel;
    AVIS_EMRC_COUNTER2 (avis) = NULL;
    AVIS_EMRC_COUNTER (avis) = NULL;

    /*
     * In conditional functions, both environments must be initialized
     * with start value start
     */
    if (start != 0) {
        AVIS_EMRC_COUNTER (avis)
          = IncreaseEnvCounter (AVIS_EMRC_COUNTER (avis), deflevel, start);
        AVIS_EMRC_COUNTER2 (avis)
          = IncreaseEnvCounter (AVIS_EMRC_COUNTER2 (avis), deflevel, start);
    }

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
 * @fn FlipEnvironment
 *
 *  @brief in order to handle conditionals, seperate environments for
 *         each branch must be maintained
 *         FlipEnvironment switches between both environments.
 *
 *  @param avis contains the environment
 *
 *  @return the modified avis node
 *
 ***************************************************************************/
static node *
FlipEnvironment (node *avis)
{
    rc_counter *temp;

    DBUG_ENTER ("FlipEnvironment");

    temp = AVIS_EMRC_COUNTER2 (avis);
    AVIS_EMRC_COUNTER2 (avis) = AVIS_EMRC_COUNTER (avis);
    AVIS_EMRC_COUNTER (avis) = temp;

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

/** <!--*******************************************************************-->
 *
 * @name DEFLIST FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn DefListHasNext
 *
 *  @brief returns whether deflist has more elements
 *
 *  @param deflist
 *
 *  @return returns whether deflist has more elements
 *
 ***************************************************************************/
static bool
DefListHasNext (rc_list_struct *deflist)
{
    DBUG_ENTER ("DefListHasNext");
    DBUG_RETURN (deflist != NULL);
}

/** <!--******************************************************************-->
 *
 * @fn DefListInsert
 *
 *  @brief inserts an avis/count into the DEFLIST
 *
 *  @param deflist
 *  @param avis
 *  @param count
 *
 *  @return the modified DEFLIST
 *
 ***************************************************************************/
static rc_list_struct *
DefListInsert (rc_list_struct *deflist, node *avis, int count)
{
    DBUG_ENTER ("DefListInsert");

    if (deflist == NULL) {
        deflist = MakeRCList (avis, count, NULL);
    } else {
        if (deflist->avis == avis) {
            deflist->count += count;
        } else {
            deflist->next = DefListInsert (deflist->next, avis, count);
        }
    }

    DBUG_RETURN (deflist);
}

/** <!--******************************************************************-->
 *
 * @fn DefListGetNext
 *
 *  @brief returns the first element of the DEFLIST
 *
 *  @param deflist
 *
 *  @return returns the first element of the DEFLIST
 *
 ***************************************************************************/
static rc_list_struct *
DefListGetNext (rc_list_struct *deflist)
{
    DBUG_ENTER ("DefListGetNext");
    DBUG_RETURN (MakeRCList (deflist->avis, deflist->count, NULL));
}

/** <!--******************************************************************-->
 *
 * @fn DefListPopNext
 *
 *  @brief returns DEFLIST without the first element
 *
 *  @param deflist
 *
 *  @return returns DEFLIST without the first element
 *
 ***************************************************************************/
static rc_list_struct *
DefListPopNext (rc_list_struct *deflist)
{
    rc_list_struct *res;

    DBUG_ENTER ("DefListPopNext");

    res = deflist->next;
    Free (deflist);

    DBUG_RETURN (res);
}
/*@}*/

/** <!--******************************************************************-->
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
 *  @param next_node
 *
 *  @return N_assign-node containing an adjust_rc prf
 *
 ***************************************************************************/
static node *
MakeAdjustRC (node *avis, int count, node *next_node)
{
    node *n, *prf;
    ids *ids1;

    if (!RCOracle (avis, count)) {
        n = next_node;
    } else {
        ids1 = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL,
                        ST_regular);
        IDS_AVIS (ids1) = avis;
        IDS_VARDEC (ids1) = AVIS_VARDECORARG (avis);

        if (count > 0) {
            /*
             * Make INC_RC
             */
            prf = MakePrf2 (F_inc_rc, MakeIdFromIds (ids1), MakeNum (count));

        } else {
            /*
             * Make DEC_RC
             */
            prf = MakePrf2 (F_dec_rc, MakeIdFromIds (ids1), MakeNum (-count));
        }

        n = MakeAssign (MakeLet (prf, NULL),
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
 *  @param next_node
 *
 *  @return An adjust_rc operation
 *
 ***************************************************************************/
static node *
MakeAdjustRCFromRLS (rc_list_struct *rls, node *next_node)
{
    node *res;

    DBUG_ENTER ("MakeAdjustRCFromRLS");

    res = MakeAdjustRC (rls->avis, rls->count, next_node);
    Free (rls);

    DBUG_RETURN (res);
}
/*@}*/

/** <!--*******************************************************************-->
 *
 * @name *List -> Assigment conversion
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn MakeUseAssignment
 *
 *  @brief converts USELIST into a chain of adjust_rc prfs
 *
 *  @param arg_info contains USELIST
 *  @param next_node
 *
 *  @return chain of adjust_rc prf
 *
 ***************************************************************************/
static node *
MakeUseAssignments (info *arg_info, node *next_node)
{
    rc_list_struct *rls;

    DBUG_ENTER ("MakeUseAssignment");

    if (UseListHasNext (INFO_EMRC_USELIST (arg_info), INFO_EMRC_DEPTH (arg_info))) {

        rls = UseListGetNext (INFO_EMRC_USELIST (arg_info));
        INFO_EMRC_USELIST (arg_info) = UseListPopNext (INFO_EMRC_USELIST (arg_info));

        next_node = MakeAdjustRCFromRLS (rls, MakeUseAssignments (arg_info, next_node));
    }

    DBUG_RETURN (next_node);
}

/** <!--******************************************************************-->
 *
 * @fn MakeDefAssignment
 *
 *  @brief converts DefLIST into a chain of adjust_rc prfs
 *
 *  @param arg_info contains DEFLIST
 *  @param next_node
 *
 *  @return chain of adjust_rc prfs
 *
 ***************************************************************************/
static node *
MakeDefAssignments (info *arg_info, node *next_node)
{
    rc_list_struct *rls;

    DBUG_ENTER ("MakeDefAssignments");

    if (DefListHasNext (INFO_EMRC_DEFLIST (arg_info))) {
        rls = DefListGetNext (INFO_EMRC_DEFLIST (arg_info));

        INFO_EMRC_DEFLIST (arg_info) = DefListPopNext (INFO_EMRC_DEFLIST (arg_info));

        next_node = MakeAdjustRCFromRLS (rls, MakeDefAssignments (arg_info, next_node));
    }

    DBUG_RETURN (next_node);
}
/*@}*/

/** <!--*******************************************************************-->
 *
 *  IDS TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/
/** <!--******************************************************************-->
 *
 * @fn TravRightIds
 *
 *  @brief traverses RHS identifiers. Depending on the surrounding construct,
 *         different actions are done. (see comments below)
 *
 *  @param arg_ids
 *  @param arg_info
 *
 *  @return arg_ids
 *
 ***************************************************************************/
static ids *
TravRightIds (ids *arg_ids, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("TravRightIds");

    avis = IDS_AVIS (arg_ids);

    /*
     * If DEFLEVEL of this id is undefinded, set it to current CODELEVEL
     * as this can only happen if we are traversind cexprs and the variable
     * has been defined in the current CBLOCK
     */
    if (GetDefLevel (avis) == -1) {
        avis = SetDefLevel (avis, INFO_EMRC_DEPTH (arg_info));
    }

    /*
     * Add one to the environment at the arguments DEFLEVEL
     * iff it is zero
     */
    if ((GetDefLevel (avis) != INFO_EMRC_DEPTH (arg_info))
        && (GetEnvironment (avis, GetDefLevel (avis)) == 0)) {
        INFO_EMRC_USELIST (arg_info)
          = UseListInsert (INFO_EMRC_USELIST (arg_info), avis, GetDefLevel (avis));
        avis = AddEnvironment (avis, GetDefLevel (avis), 1);
    }

    switch (INFO_EMRC_COUNTMODE (arg_info)) {
    case rc_unknown:
        /*
         * Copy assignment a = b;
         */

        /*
         * Env(b) += Env(a)
         */
        avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info),
                               GetEnvironment (IDS_AVIS (INFO_EMRC_LHS (arg_info)),
                                               INFO_EMRC_DEPTH (arg_info)));

        IDS_AVIS (INFO_EMRC_LHS (arg_info))
          = PopEnvironment (IDS_AVIS (INFO_EMRC_LHS (arg_info)),
                            INFO_EMRC_DEPTH (arg_info));

        INFO_EMRC_MUSTCOUNT (arg_info) = FALSE;
        break;

    case rc_apuse:
        /*
         * If this function is not an external function whose argument
         * must be refcounted like a prf argument we increase the environment
         * at the current codelevel by one
         */
        avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info), 1);
        break;

    case rc_conduse:
        /*
         * The environment at the current codelevel must be increased by
         * the number of uses in CONDFUN
         */
        avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info),
                               NUM_VAL (EXPRS_EXPR (INFO_EMRC_CONDARGS (arg_info))));
        INFO_EMRC_CONDARGS (arg_info) = FreeNode (INFO_EMRC_CONDARGS (arg_info));
        break;

    case rc_retuse:
        /*
         * The environment at the current codelevel must be increased by
         * the number of uses in CONDFUN
         */
        avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info),
                               NUM_VAL (EXPRS_EXPR (INFO_EMRC_RETVALS (arg_info))));
        INFO_EMRC_RETVALS (arg_info) = FreeNode (INFO_EMRC_RETVALS (arg_info));
        break;

    case rc_prfuse:
        /*
         * Add one to the environment at the variable's DEFLEVEL iff it is zero
         * and put the variable into USELIST
         */
        if (GetEnvironment (avis, GetDefLevel (avis)) == 0) {
            INFO_EMRC_USELIST (arg_info)
              = UseListInsert (INFO_EMRC_USELIST (arg_info), avis, GetDefLevel (avis));
            avis = AddEnvironment (avis, GetDefLevel (avis), 1);
        }
    }

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravRightIds (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/**
 * @}
 */

/** <!--*******************************************************************-->
 *
 *  TRAVERSAL FUNCTIONS
 *
 *  emrc_tab
 *  Prefix: EMRC
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
    node *args;
    ids *let_ids;
    int argc;

    DBUG_ENTER ("EMRCap");

    /*
     * CONDFUNs are traversed in order of appearance
     */
    if (FUNDEF_IS_CONDFUN (AP_FUNDEF (arg_node))) {
        /*
         * Pass the environments of the expected return values
         */
        node *avis;
        int rc;
        ids *_ids = INFO_EMRC_LHS (arg_info);

        while (_ids != NULL) {
            avis = IDS_AVIS (_ids);
            rc = GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
            avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));

            INFO_EMRC_RETENV (arg_info)
              = AppendExprs (INFO_EMRC_RETENV (arg_info), MakeExprs (MakeNum (rc), NULL));

            _ids = IDS_NEXT (_ids);
        }

        /*
         * Pass the environments of the arguments
         */
        args = AP_ARGS (arg_node);
        while (args != NULL) {
            avis = ID_AVIS (EXPRS_EXPR (args));
            rc = GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
            avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));

            INFO_EMRC_ARGENV (arg_info)
              = AppendExprs (INFO_EMRC_ARGENV (arg_info), MakeExprs (MakeNum (rc), NULL));

            args = EXPRS_NEXT (args);
        }

        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);
        INFO_EMRC_MUSTCOUNT (arg_info) = FALSE;
    }

    args = AP_ARGS (arg_node);
    argc = 0;
    let_ids = INFO_EMRC_LHS (arg_info);

    /*
     * Find out the number of NON-ARTIFICIAL argumuments in the LHS
     */
    while (let_ids != NULL) {
        if (VARDEC_STATUS (IDS_VARDEC (let_ids)) != ST_artificial) {
            argc += 1;
        }
        let_ids = IDS_NEXT (let_ids);
    }

    while (args != NULL) {

        DBUG_ASSERT (EXPRS_EXPR (args) != NULL, "Missing argument!");
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (args)) == N_id,
                     "Function arguments must be N_id nodes");

        if ((VARDEC_OR_ARG_STATUS (ID_VARDEC (EXPRS_EXPR (args))) != ST_artificial)
            && (FUNDEF_EXT_NOT_REFCOUNTED (AP_FUNDEF (arg_node), argc))) {
            INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
        } else {
            if (FUNDEF_IS_CONDFUN (AP_FUNDEF (arg_node))) {
                INFO_EMRC_COUNTMODE (arg_info) = rc_conduse;
            } else {
                INFO_EMRC_COUNTMODE (arg_info) = rc_apuse;
            }
        }

        if (EXPRS_EXPR (args) != NULL) {
            EXPRS_EXPR (args) = Trav (EXPRS_EXPR (args), arg_info);
        }

        if (VARDEC_OR_ARG_STATUS (ID_VARDEC (EXPRS_EXPR (args))) != ST_artificial) {
            argc += 1;
        }

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
    int env = 0;

    DBUG_ENTER ("EMRCarg");

    if (INFO_EMRC_ARGENV (arg_info) != NULL) {
        env = NUM_VAL (EXPRS_EXPR (INFO_EMRC_ARGENV (arg_info)));
        INFO_EMRC_ARGENV (arg_info) = FreeNode (INFO_EMRC_ARGENV (arg_info));
    }

    ARG_AVIS (arg_node) = InitializeEnvironment (ARG_AVIS (arg_node), 0, env);

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCarray
 *
 *  @brief
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

    INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;

    if (ARRAY_AELEMS (arg_node) != NULL) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    }

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

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * If this node happens to be a conditional,
     * traverse again!
     */
    if (INFO_EMRC_COND (arg_info)) {
        INFO_EMRC_COND (arg_info) = FALSE;

        INFO_EMRC_MODE (arg_info) = rc_else;

        n = FUNDEF_ARGS (INFO_EMRC_FUNDEF (arg_info));
        while (n != NULL) {
            ARG_AVIS (n) = FlipEnvironment (ARG_AVIS (n));
            n = ARG_NEXT (n);
        }

        n = BLOCK_VARDEC (FUNDEF_BODY (INFO_EMRC_FUNDEF (arg_info)));
        while (n != NULL) {
            VARDEC_AVIS (n) = FlipEnvironment (VARDEC_AVIS (n));
            n = VARDEC_NEXT (n);
        }

        INFO_EMRC_RETVALS (arg_info) = INFO_EMRC_RETVALS2 (arg_info);
        INFO_EMRC_RETVALS2 (arg_info) = NULL;

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        INFO_EMRC_COND (arg_info) = FALSE;
        INFO_EMRC_MODE (arg_info) = rc_default;
    }

    /*
     * Insert ADJUST_RC prfs from USELIST
     */
    ASSIGN_NEXT (arg_node) = MakeUseAssignments (arg_info, ASSIGN_NEXT (arg_node));

    /*
     * Insert ADJUST_RC prfs from DEFLIST
     */
    ASSIGN_NEXT (arg_node) = MakeDefAssignments (arg_info, ASSIGN_NEXT (arg_node));

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
    avis = FlipEnvironment (avis);

    t = GetEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
    avis = PopEnvironment (avis, INFO_EMRC_DEPTH (arg_info));
    avis = FlipEnvironment (avis);

    if (e + t > 0) {
        if (e * t == 0) {
            m = e > t ? e : t;
        } else {
            m = e < t ? e : t;
        }

        avis = AddEnvironment (avis, INFO_EMRC_DEPTH (arg_info), m);

        INFO_EMRC_DEFLIST (arg_info)
          = DefListInsert (INFO_EMRC_DEFLIST (arg_info), avis, t - m);
        BLOCK_INSTR (COND_THEN (cond))
          = MakeDefAssignments (arg_info, BLOCK_INSTR (COND_THEN (cond)));

        INFO_EMRC_DEFLIST (arg_info)
          = DefListInsert (INFO_EMRC_DEFLIST (arg_info), avis, e - m);
        BLOCK_INSTR (COND_ELSE (cond))
          = MakeDefAssignments (arg_info, BLOCK_INSTR (COND_ELSE (cond)));
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

    DBUG_ENTER ("EMRCcond");

    if (INFO_EMRC_MODE (arg_info) == rc_default) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

        INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
        if (COND_COND (arg_node) != NULL) {
            COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
        }

        /*
         * Prepend THEN branch with DEC-Assignments if necessary
         */
        BLOCK_INSTR (COND_THEN (arg_node))
          = MakeUseAssignments (arg_info, BLOCK_INSTR (COND_THEN (arg_node)));

    } else {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

        INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
        if (COND_COND (arg_node) != NULL) {
            COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
        }

        /*
         * Prepend ELSE branch with DEC-Assignments if necessary
         */
        BLOCK_INSTR (COND_ELSE (arg_node))
          = MakeUseAssignments (arg_info, BLOCK_INSTR (COND_ELSE (arg_node)));

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

    INFO_EMRC_COND (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCfuncond
 *
 *  @brief
 *
 *  @param funcond
 *  @param arg_info
 *
 *  @return funcond
 *
 ***************************************************************************/
node *
EMRCfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRCfuncond");

    DBUG_ASSERT (INFO_EMRC_COUNTMODE (arg_info) == rc_unknown, "Illegal counting mode!");

    switch (INFO_EMRC_MODE (arg_info)) {
    case rc_default:
        FUNCOND_THEN (arg_node) = Trav (FUNCOND_THEN (arg_node), arg_info);

        IDS_AVIS (INFO_EMRC_LHS (arg_info))
          = PopEnvironment (IDS_AVIS (INFO_EMRC_LHS (arg_info)),
                            INFO_EMRC_DEPTH (arg_info));

        IDS_AVIS (INFO_EMRC_LHS (arg_info))
          = FlipEnvironment (IDS_AVIS (INFO_EMRC_LHS (arg_info)));
        break;

    case rc_else:
        FUNCOND_ELSE (arg_node) = Trav (FUNCOND_ELSE (arg_node), arg_info);

        IDS_AVIS (INFO_EMRC_LHS (arg_info))
          = PopEnvironment (IDS_AVIS (INFO_EMRC_LHS (arg_info)),
                            INFO_EMRC_DEPTH (arg_info));
        break;
    }

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
    DBUG_ENTER ("EMRCfundef");

    DBUG_ASSERT ((arg_info == NULL) || (FUNDEF_IS_CONDFUN (fundef)), "Illegal arguments");

    /*
     * The body must be traversed if fundef is a regular function
     * or this is a nested traversal of a CONDFUN
     */
    if ((!FUNDEF_IS_CONDFUN (fundef)) || (arg_info != NULL)) {
        node *arg;
        node *avis;

        info *info = MakeInfo (fundef);

        DBUG_PRINT ("EMRC", ("Inferencing reference counters in function %s...",
                             FUNDEF_NAME (fundef)));

        /*
         * Traverse block
         */
        if (FUNDEF_BODY (fundef) != NULL) {
            if (FUNDEF_IS_CONDFUN (fundef)) {
                /*
                 * CONDFUN:
                 * Initial counters of arguments and return values are
                 * given as argument
                 */
                INFO_EMRC_RETVALS (info) = INFO_EMRC_RETENV (arg_info);
                INFO_EMRC_ARGENV (info) = INFO_EMRC_ARGENV (arg_info);
                INFO_EMRC_RETENV (arg_info) = NULL;
                INFO_EMRC_ARGENV (arg_info) = NULL;
            } else {
                /*
                 * REGULAR FUNCTION:
                 * Initialize RETVALS with ones
                 */
                int i = CountTypes (FUNDEF_TYPES (fundef));
                while (i > 0) {
                    INFO_EMRC_RETVALS (info)
                      = MakeExprs (MakeNum (1), INFO_EMRC_RETVALS (info));
                    i -= 1;
                }
            }

            /*
             * In case there is a conditional inside the function,
             * prepare a copy of the return RCs
             */
            if (FUNDEF_IS_LACFUN (fundef)) {
                INFO_EMRC_RETVALS2 (info) = DupTree (INFO_EMRC_RETVALS (info));
            }

            /*
             * Traverse args in order to initialize refcounting environment
             */
            if (FUNDEF_ARGS (fundef) != NULL) {
                FUNDEF_ARGS (fundef) = Trav (FUNDEF_ARGS (fundef), info);
            }

            FUNDEF_BODY (fundef) = Trav (FUNDEF_BODY (fundef), info);

            /*
             * Annotate missing ADJUST_RCs
             */
            arg = FUNDEF_ARGS (fundef);
            while (arg != NULL) {
                avis = ARG_AVIS (arg);

                if (FUNDEF_IS_CONDFUN (fundef)) {
                    /*
                     * CONDFUN:
                     * Counter states of function arguments are returned via CONDARGS
                     */
                    INFO_EMRC_CONDARGS (arg_info)
                      = AppendExprs (INFO_EMRC_CONDARGS (arg_info),
                                     MakeExprsNum (
                                       GetEnvironment (avis, INFO_EMRC_DEPTH (info))));

                    avis = PopEnvironment (avis, INFO_EMRC_DEPTH (info));

                    DBUG_ASSERT (NUM_VAL (EXPRS_EXPR (INFO_EMRC_CONDARGS (arg_info))) > 0,
                                 "Unused argument of CONDFUN encountered!!!");
                } else {
                    /*
                     * REGULAR FUNCTION:
                     * Put counters into DEFLIST in order to create INC_RC statements
                     * at the beginning of the function
                     */
                    INFO_EMRC_DEFLIST (info)
                      = DefListInsert (INFO_EMRC_DEFLIST (info), avis,
                                       GetEnvironment (avis, INFO_EMRC_DEPTH (info)) - 1);

                    avis = PopEnvironment (avis, INFO_EMRC_DEPTH (info));
                }
                arg = ARG_NEXT (arg);
            }

            BLOCK_INSTR (FUNDEF_BODY (fundef))
              = MakeDefAssignments (info, BLOCK_INSTR (FUNDEF_BODY (fundef)));
        }

        info = FreeInfo (info);

        DBUG_PRINT ("EMRC", ("Reference counting inference in function %s complete.",
                             FUNDEF_NAME (fundef)));
    }

    /*
     * Traverse other fundefs if this is a regular fundef traversal
     */
    if ((arg_info == NULL) && (FUNDEF_NEXT (fundef) != NULL)) {
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

    DBUG_ENTER ("EMRClet");

    INFO_EMRC_COUNTMODE (arg_info) = rc_unknown;
    INFO_EMRC_MUSTCOUNT (arg_info) = TRUE;
    INFO_EMRC_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    if (INFO_EMRC_MUSTCOUNT (arg_info)) {
        /*
         * Add all lhs ids to deflist
         */
        ids = LET_IDS (arg_node);

        while (ids != NULL) {
            /*
             * Subtraction needed because ALLOC / FunAp initializes RC with 1
             */
            INFO_EMRC_DEFLIST (arg_info)
              = DefListInsert (INFO_EMRC_DEFLIST (arg_info), IDS_AVIS (ids),
                               GetEnvironment (IDS_AVIS (ids), INFO_EMRC_DEPTH (arg_info))
                                 - 1);
            IDS_AVIS (ids) = PopEnvironment (IDS_AVIS (ids), INFO_EMRC_DEPTH (arg_info));

            ids = IDS_NEXT (ids);
        }
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

    DBUG_ENTER ("EMRCicm");

    name = ICM_NAME (arg_node);

    INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;

    if (strstr (name, "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET( off_nt, wl_nt)
         * does *not* consume its arguments! It is expanded to
         *      off_nt = wl_nt__off    ,
         * where 'off_nt' is a scalar and 'wl_nt__off' an internal variable!
         *   -> do NOT traverse the second argument (used)
         */
        ICM_EXPRS1 (arg_node) = Trav (ICM_EXPRS1 (arg_node), arg_info);
    } else {
        if (strstr (name, "VECT2OFFSET") != NULL) {
            /*
             * VECT2OFFSET( off_nt, ., from_nt, ., ., ...)
             * needs RC on all but the first argument. It is expanded to
             *     off_nt = ... from_nt ...    ,
             * where 'off_nt' is a scalar variable.
             *  -> handle ICM like a prf (RCO)
             */
            ICM_ARGS (arg_node) = Trav (ICM_ARGS (arg_node), arg_info);
        } else {
            if (strstr (name, "IDXS2OFFSET") != NULL) {
                /*
                 * IDXS2OFFSET( off_nt, ., idx_1_nt ... idx_n_nt, ., ...)
                 * needs RC on all but the first argument. It is expanded to
                 *     off_nt = ... idx_1_nt[i] ... idx_n_nt[i] ...   ,
                 * where 'off_nt' is a scalar variable.
                 *  -> handle ICM like a prf (RCO)
                 */
                ICM_ARGS (arg_node) = Trav (ICM_ARGS (arg_node), arg_info);
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
 *  @brief traverses RHS identifiers.
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
    DBUG_ENTER ("EMRCid");

    ID_IDS (arg_node) = TravRightIds (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
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
    int env;
    node *n, *epicode;

    DBUG_ENTER ("EMRCNcode");

    /*
     * Traverse CEXPRS and insert adjust_rc operations into
     * NCODE_EPILOGUE
     */
    INFO_EMRC_COUNTMODE (arg_info) = rc_apuse;
    NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);

    /*
     * Insert ADJUST_RC prfs from DECLIST
     */
    epicode = MakeUseAssignments (arg_info, NULL);
    DBUG_ASSERT (epicode == NULL, "Epicode should not contain antything!");

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    BLOCK_INSTR (NCODE_CBLOCK (arg_node))
      = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (arg_node)), epicode);

    /*
     * Prepend block with Adjust_RC prfs
     */
    n = FUNDEF_ARGS (INFO_EMRC_FUNDEF (arg_info));
    while (n != NULL) {
        env = GetEnvironment (ARG_AVIS (n), INFO_EMRC_DEPTH (arg_info));
        ARG_AVIS (n) = PopEnvironment (ARG_AVIS (n), INFO_EMRC_DEPTH (arg_info));

        INFO_EMRC_DEFLIST (arg_info)
          = DefListInsert (INFO_EMRC_DEFLIST (arg_info), ARG_AVIS (n), env);

        BLOCK_INSTR (NCODE_CBLOCK (arg_node))
          = MakeDefAssignments (arg_info, BLOCK_INSTR (NCODE_CBLOCK (arg_node)));

        n = ARG_NEXT (n);
    }

    n = BLOCK_VARDEC (FUNDEF_BODY (INFO_EMRC_FUNDEF (arg_info)));
    while (n != NULL) {
        env = GetEnvironment (VARDEC_AVIS (n), INFO_EMRC_DEPTH (arg_info));
        VARDEC_AVIS (n) = PopEnvironment (VARDEC_AVIS (n), INFO_EMRC_DEPTH (arg_info));

        INFO_EMRC_DEFLIST (arg_info)
          = DefListInsert (INFO_EMRC_DEFLIST (arg_info), VARDEC_AVIS (n), env);

        BLOCK_INSTR (NCODE_CBLOCK (arg_node))
          = MakeDefAssignments (arg_info, BLOCK_INSTR (NCODE_CBLOCK (arg_node)));

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
    DBUG_ENTER ("EMRCNwith");

    INFO_EMRC_DEPTH (arg_info) += 1;

    NWITH_WITHID (arg_node) = Trav (NWITH_WITHID (arg_node), arg_info);

    /*
     * index vector needs an extra traversal because it is
     * definitely needed in AUD with-loops
     */
    INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
    NWITH_VEC (arg_node) = TravRightIds (NWITH_VEC (arg_node), arg_info);

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    INFO_EMRC_DEPTH (arg_info) -= 1;

    INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    INFO_EMRC_MUSTCOUNT (arg_info) = TRUE;

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
    DBUG_ENTER ("EMRCNwith2");

    INFO_EMRC_DEPTH (arg_info) += 1;

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    NWITHID_VECNEEDED (NWITH2_WITHID (arg_node))
      = GetEnvironment (IDS_AVIS (NWITHID_VEC (NWITH2_WITHID (arg_node))),
                        GetDefLevel (IDS_AVIS (NWITHID_VEC (NWITH2_WITHID (arg_node)))))
        > 0;

    if (!NWITHID_VECNEEDED (NWITH2_WITHID (arg_node))) {
        DBUG_PRINT ("EMRC", ("Index vector %s will not be built!\n",
                             IDS_NAME (NWITHID_VEC (NWITH2_WITHID (arg_node)))));
    }

    INFO_EMRC_DEPTH (arg_info) -= 1;

    INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    INFO_EMRC_MUSTCOUNT (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCNwithid
 *
 *  @brief
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
    DBUG_ENTER ("EMRCNwithid");

    INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;

    if (NWITHID_IDS (arg_node) != NULL) {
        NWITHID_IDS (arg_node) = TravRightIds (NWITHID_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCNwithop
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 ***************************************************************************/
node *
EMRCNwithop (node *arg_node, info *arg_info)
{
    rc_mode old_rc_mode;

    DBUG_ENTER ("EMRCNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        /*
         * genarray( shp, def, mem)
         *
         * - shp, def must be refcounted like a prf use
         * - mem must be refcounted like a funap use
         */
        INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        if (NWITHOP_DEFAULT (arg_node) != NULL) {
            NWITHOP_DEFAULT (arg_node) = Trav (NWITHOP_DEFAULT (arg_node), arg_info);
        }
        INFO_EMRC_COUNTMODE (arg_info) = rc_apuse;
        NWITHOP_MEM (arg_node) = Trav (NWITHOP_MEM (arg_node), arg_info);
        break;

    case WO_modarray:
        /*
         * modarray( A, mem);
         *
         * - A must be refcounted like a prf use
         * - mem must be refcoutned like a funap use
         */
        INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
        NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        INFO_EMRC_COUNTMODE (arg_info) = rc_apuse;
        NWITHOP_MEM (arg_node) = Trav (NWITHOP_MEM (arg_node), arg_info);
        break;

    case WO_foldfun:
    case WO_foldprf:
        /*
         * fold( op, n);
         *
         * - op is not a variable
         * - n must be refcoutned like a funap use
         */
        INFO_EMRC_COUNTMODE (arg_info) = rc_apuse;
        NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        break;

    case WO_unknown:
        DBUG_ASSERT (FALSE, "non initialised WithOpType found.");
        break;
    }

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn EMRCprf
 *
 *  @brief traverses a prf's arguments
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
    rc_mode old_rc_mode;

    DBUG_ENTER ("EMRCprf");

    switch (PRF_PRF (arg_node)) {
    case F_fill:
        /*
         * fill( expr, a);
         *
         * - expr must be traversed
         * - a must be counted like a funap use of a
         */
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        INFO_EMRC_COUNTMODE (arg_info) = rc_apuse;
        PRF_ARG2 (arg_node) = Trav (PRF_ARG2 (arg_node), arg_info);
        break;

    case F_accu:
        /*
         * accu( iv, n)
         *
         * - iv must not be counted as it is already counted by WITHID traversal
         * - n  must not be counted as it is already counted by WITHOP traversal
         */
        break;

    case F_alloc:
    case F_alloc_or_reuse:
    case F_reuse:
        /*
         * alloc( dim, shp)
         *
         * - initialize rc with 1
         */
        PRF_ARGS (arg_node) = MakeExprs (MakeNum (1), PRF_ARGS (arg_node));
        break;

    case F_suballoc:
        /*
         * suballoc( A, iv);
         *
         * - A  must not be counted as it is already counted by WITHOP traversal
         * - iv must not be counted as it is already counted by WITHID traversal
         */
        break;

    case F_wl_assign:
        /*
         * wl_assign( a, A, iv);
         *
         * - a must be refcounted like prf use
         * - A  must not be counted as it is already counted by WITHOP traversal
         * - iv must not be counted as it is already counted by WITHID traversal
         */
        INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        break;
    default:
        INFO_EMRC_COUNTMODE (arg_info) = rc_prfuse;
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        }
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

    INFO_EMRC_COUNTMODE (arg_info) = rc_retuse;

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

    VARDEC_AVIS (arg_node) = InitializeEnvironment (VARDEC_AVIS (arg_node), -1, 0);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/

/** <!--*******************************************************************-->
 *
 *  TRAVERSAL FUNCTIONS: ANNOTATE C FUNS
 *
 *  This traversal marks all external C Functions with flag ST_Cfun.
 *
 *  Tab:    emacf_tab
 *  Prefix: EMACF
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn EMACFfundef
 *
 *  @brief
 *
 *  @param fundef
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
EMACFfundef (node *fundef, info *arg_info)
{
    DBUG_ENTER ("EMACFfundef");

    /*
     * special module name -> must be an external C-fun
     */
    if (((sbs == 1) && (strcmp (FUNDEF_MOD (fundef), EXTERN_MOD_NAME) == 0))
        || ((sbs == 0) && (FUNDEF_MOD (fundef) == NULL))) {
        DBUG_PRINT ("EMRC", ("%s marked as ST_Cfun", FUNDEF_NAME (fundef)));
        FUNDEF_STATUS (fundef) = ST_Cfun;
    }

    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (fundef) = Trav (FUNDEF_NEXT (fundef), arg_info);
    }

    DBUG_RETURN (fundef);
}
/*@}*/

/*@}*/ /* defgroup ssarc */
