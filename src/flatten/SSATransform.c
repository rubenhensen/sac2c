/*
 * $Log$
 * Revision 1.24  2004/11/29 20:44:49  sah
 * post-DK bugfixing
 *
 * Revision 1.23  2004/11/27 03:07:40  cg
 * Functions renamed.
 *
 * Revision 1.22  2004/11/27 00:41:46  mwe
 * function renaming
 *
 * Revision 1.21  2004/11/26 12:50:08  mwe
 * changes according to changes in tree_compound.h
 *
 * Revision 1.20  2004/11/25 22:50:01  mwe
 * changes according to changes in ast.xml
 *
 * Revision 1.19  2004/11/25 14:04:33  mwe
 * SacDevCamp Dk: Compiles!!
 *
 * Revision 1.18  2004/08/08 14:02:08  sbs
 * some doxygenic added
 *
 * Revision 1.17  2004/08/07 16:01:43  sbs
 * SSAwith2 added for N_Nwith2 support
 *
 * Revision 1.16  2004/08/07 13:20:11  sbs
 * code brushing finished for now. WL treatment redone.
 *
 * Revision 1.15  2004/08/07 10:10:27  sbs
 * SSANwithXXX renamed into SSAwithXXXX
 * further code brushing made
 * bug in SSAfuncond fixed;now, we can deal with funconds that are
 * not preceeded by if-then-elses! This may happen due to optimization
 * An example is gcd!
 *
 * Revision 1.14  2004/08/06 21:05:45  sbs
 * maior code brushing and additional commenting done.
 * In particular, Funcond generation rewritten.
 *
 * Revision 1.13  2004/08/05 20:58:08  sbs
 * some maior code brushing and implementation description added
 * AND
 * variable renaming changed into <varname>__SSA<x>_<Y>
 * (for explaination see comments 8-)
 * this should be a proper fix for bug 42 (hopefully 8-)
 *
 * Revision 1.12  2004/08/04 17:13:12  khf
 * quick fix for bug #42
 * new variable name created by TmpVarName()
 *
 * Revision 1.11  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.10  2004/07/14 15:20:57  ktr
 * WITHID is treated as RHS if the variables were allocated using alloc, too.
 *
 * Revision 1.9  2004/06/21 19:01:15  mwe
 * check compiler phase before creating new types (create no types before PH_typecheck)
 *
 * Revision 1.8  2004/06/10 14:46:53  mwe
 * after usage of SSANewVardec a ntype added to new avis node
 *
 * Revision 1.7  2004/06/08 14:30:46  ktr
 * WithIDs are treated as RHS-expressions iff the Index vector has been
 * allocated using F_alloc_or_reuse before.
 *
 * Revision 1.6  2004/05/12 12:59:40  ktr
 * Code for NCODE_EPILOGUE added
 *
 * Revision 1.5  2004/05/11 13:25:21  khf
 * NCODE_CEXPR in SSANcode() replaced by NCODE_CEXPRS
 *
 * Revision 1.4  2004/03/05 19:14:27  mwe
 * representation of conditional changed
 * using N_funcond node instead of phi
 *
 * Revision 1.3  2004/02/25 08:22:32  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 *
 * Revision 1.2  2004/02/06 14:19:33  mwe
 * replace usage of PHITARGET with primitive phi function
 *
 * Revision 1.1  2004/01/28 16:53:48  skt
 * Initial revision
 *
 *
 ************ Attention! ************
 * File was moved from ../tree
 * following older Revisions can be found there
 *
 *  [eliminated...]
 *
 * Revision 1.1  2001/02/13 15:16:15  nmw
 * Initial revision
 *
 */

/**
 *
 * @defgroup ssatransform SSATransform
 * @ingroup ssa
 *
 * @brief  SSATransform makes all LHS within a single function unique.
 *
 *******************************************************************************
 * <pre>
 *
 * description:
 *    1) Basics:
 *    ----------
 *    This module traverses the AST and transformes the code in SSA form.
 *    In ssa-form every identifier has exactly one defining assignment
 *    in the code. So redefinitions are not allowed and lead to new
 *    identifiers (here a renamed versions of the original variables extended
 *    by the postfix __SSA<x>_<y>  where <x> and <y> are integers >= 0.
 *
 *    NB: <x> indicates how many   SSA-UndoSSA  phases have been before.
 *        <y> is just a re-definition-counter specific to each var name.
 *
 *    Example:
 *      (usual form)   -->   (ssa form)
 *
 *      int f( int a)        int f( int a)
 *      {                    {
 *        a = a + 6;           a__SSA0_1 = a + 6;
 *        b = a;               b = a__SSA0_1;
 *        b = b + 1;           b__SSA0_1 = b + 1;
 *        a = a + 1;           a__SSA0_2 = a__SSA0_1 + 1;
 *        a = a + 1;           a__SSA0_3 = a__SSA0_2 + 1;
 *        return(a + b);       return(a__SSA0_3 + b__SSA0_1);
 *      }                    }
 *
 *    2) Conditionals:
 *    ----------------
 *    In principle, conditionals can be transformed in the same fashion.
 *    However, it has to be made sure, that
 *    I)  static binding is preserved in the else branch, e.g., the
 *        RHS of line ( 9) in fact is a__SSA0_1 rather than a__SSA0_2!
 *    II) the different renamings in both branches are joined together
 *        after the conditional by introducing a functional conditional
 *        (N_funcond) node.
 *
 *    NB: the latter in fact constitutes a functionally sound representation
 *        of the phi-functions that are usually found in SSA.
 *
 *    Example:
 *          (usual form)    -->  (ssa-form)
 *
 *    ( 1)   a = 6;                a = 6;
 *    ( 2)   a = a + 4;            a__SSA0_1 = a + 4;
 *    ( 3)   b = a + a;            b = a__SSA0_1 + a__SSA0_1;
 *    ( 4)   p = b < a;            p = b < a__SSA0_1;
 *    ( 5)   if( p) {              if( p) {
 *    ( 6)     a = 0;                a__SSA0_2 = 0;
 *    ( 7)     c = 1;                c = 1;
 *    ( 8)   } else {              } else {
 *    ( 9)     c = a;                c__SSA0_1 = a__SSA0_1;
 *    (10)   }                     }
 *    (11)                         a__SSA0_3 = cond( p, a__SSA0_2, a__SSA0_1);
 *    (12)                         c__SSA0_2 = cond( p, c, c__SSA0_1);
 *    (13)   print(a + b + c);     print( a__SSA0_3 + b + c__SSA0_2);
 *
 *
 *    2) With-Loops:
 *    --------------
 *    < to be inserted; any volunteers??? > (SBS Thu Aug  5 21:12:55 MEST 2004)
 *
 *
 * implementation:
 *    In performing a top-down traversal the "C-style" controlflow
 *    is mimicked. Renaming takes place (more precisely: MAY take place)
 *    whenever variables on RHS or LHS of assignments are traversed.
 *    ( implemented by SSArightids and SSAleftids, repectively )
 *
 *    A renaming is initiated whenever a LHS id  is found that has been
 *    previously defined or has been given as argument. This is observed
 *    by finding the flag   AVIS_SSADEFINED   true.
 *    In case of renaming, a new vardec is created (SSANewVardec) and
 *    properly inserted. The new name is generated by extending it
 *    with "__SSA<x>_<y>", where <x> is given by a global variable
 *    and <y> is a name specific counter. This counter is held in
 *    a SSACNT node which is stored as attribute of the AVIS node.
 *    All SSACNT nodes are chained as attribute of the top block of the
 *    concerning function. The counter is used in sharing for all renamed
 *    instances of one original variable. So every additional renaming of
 *    a renamed identifier gets a fresh, unique name and it is still linked
 *    with the original name (which is also stored in SSACNT as BASID
 *    attribute).
 *    After the creation and insertion of the new vardec, the avis has to be
 *    made available to all subsequent RHS uasages of the variables!
 *    This is done by storing the new AVIS (that of the renamed variable)
 *    in the old AVIS node (using AVIS_SSASTACK_TOP). So the generated
 *    structure in fact looks like this:
 *
 *     |---------------------|
 *     | Avis of "a"         | -- AVIS_SSACNT -----> |---------------------|
 *     |---------------------|                  ^    | Ssacnt: 1   "a"     |
 *               |                              |    |---------------------|
 *               |                              |
 *        AVIS_SSASTACK_TOP                     |
 *               |                              |
 *               v                              |
 *     |---------------------|                  |
 *     | Avis of "a__SSA0_1" | -- AVIS_SSACNT --/
 *     |---------------------|
 *
 *
 *    The traversal of RHS ids eventually for each id replaces the pointer to
 *    the original avis (the un-renamed old one), e.g. that to AVIS "a", by
 *    that found in AVIS_SSASTACK_TOP of it, i.e. the new, re-named one
 *    (e.g. AVIS "a__SSA0_1").
 *
 *    Unfortunately, it does not suffice to provide a single field
 *    AVIS_SSASTACK_TOP.
 *    The reason for this is the fact that conditionals and (potentially nested)
 *    with loops require these to be "reset" whenever a block is finished!!!!!
 *    Therefore, the AVIS nodes do not have an attribute SSASTACK_TOP per se,
 *    but an entire chain of AVIS nodes implemented vi SSASTACK nodes.
 *    NB: In fact, AVIS_SSASTACK_TOP constitutes a compound macro.
 *
 *    < to be continued; any volunteers??? > (SBS Thu Aug  5 21:12:55 MEST 2004)
 *
 *
 *
 * Remarks:
 *    This module requires loops and conditionals in explicit functions.
 *    This transformation has to be done by lac2fun before calling
 *    SSATransform! SSATransform can be called again to preserve the ssa-form
 *    of the AST (e.g. after old code that cannot deal with the ssa-form).
 *    After using the ssa-form the code can be cleaned up by UndoSSATransform,
 *    that e.g. removes copy assignments and renamings of global objects.
 *    Furthermore, all SSACNT nodes and SSASTACK nodes are eliminated as well.
 *
 * </pre>
 * @{
 *****************************************************************************/

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "internal_lib.h"
#include "dbug.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSATransform.h"
#include "Error.h"
#include "new_types.h"

#define TMP_STRING_LEN 16

/* helper functions for internal usage */
static node *CreateFuncondAssign (node *cond, node *id, node *assign);
static node *GetSsacnt (char *baseid, int initvalue, node *block);
static node *TreatIdAsLhs (node *arg_node, info *arg_info);
static node *TreatIdsAsRhs (node *, info *);

/* special ssastack operations for ONE avis-node */
static node *PopSsastack (node *avis);
static node *DupTopSsastack (node *avis);
static node *SaveTopSsastackThen (node *avis);
static node *SaveTopSsastackElse (node *avis);

/**
 *
 * @name The local info structure
 *
 * <!--
 * node *MakeInfo()        : constructor function
 * node *FreeInfo( node *) : destructor function
 * -->
 *
 * The local info structure contains the following fields:
 *
 * SINGLEFUNDEF     : steers how much code is to be traversed; legal values
 *                    are defined below.
 * ALLOW_GOS        : flag indicating whether global objects are potentially
 *                    contained in the code to be reformed
 * EXPLICIT_ALLOCS  : flag indicating whether mem allocs have been made
 *                    explicit.
 *
 * GENERATE_FUNCOND : indicates funcond generation mode rather than
 *                    renaming mode (toggled by SSAreturn)
 * RENAMING_MODE    : used in funconds only; steers renaming in SSArightids;
 *                    legal values are defined below.
 *
 * FUNDEF        : ptr to the actual fundef
 * ASSIGN        : ptr to the actual assign; if modified during RHS traversal
 *                 the new ptr has to be inserted and traversed again!
 * CONDSTMT      : ptr to the cond node passed by if any
 * FUNCOND_FOUND : flag indicating presence of funcond node
 * WITHID        : ptr to actual withid node
 * FIRST_WITHID  : ptr to the first withid node of a mgWL
 *
 */
/*@{*/

struct INFO {
    int singlefundef;
    bool allow_gos;
    bool explicit_allocs;

    bool generate_funcond;
    int renaming_mode;

    node *fundef;
    node *assign;
    node *condstmt;
    bool funcond_found;
    node *withid;
    node *first_withid;
};

/**
 * legal values for
 * INFO_SSA_SINGLEFUNDEF :
 */
#define SSA_TRAV_FUNDEFS 0
#define SSA_TRAV_SPECIALS 1
#define SSA_TRAV_NONE 2

/**
 * legal values for RENAMING_MODE:
 */
#define SSA_USE_TOP 0
#define SSA_USE_THEN 1
#define SSA_USE_ELSE 2

/*
 * access macros:
 */
#define INFO_SSA_SINGLEFUNDEF(n) (n->singlefundef)
#define INFO_SSA_ALLOW_GOS(n) (n->allow_gos)
#define INFO_SSA_EXPLICIT_ALLOCS(n) (n->explicit_allocs)

#define INFO_SSA_GENERATE_FUNCOND(n) (n->generate_funcond)
#define INFO_SSA_RENAMING_MODE(n) (n->renaming_mode)

#define INFO_SSA_FUNDEF(n) (n->fundef)
#define INFO_SSA_ASSIGN(n) (n->assign)
#define INFO_SSA_CONDSTMT(n) (n->condstmt)
#define INFO_SSA_FUNCOND_FOUND(n) (n->funcond_found)
#define INFO_SSA_WITHID(n) (n->withid)
#define INFO_SSA_FIRST_WITHID(n) (n->first_withid)

/*
 * INFO functions:
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_SSA_SINGLEFUNDEF (result) = 0;
    INFO_SSA_ALLOW_GOS (result) = FALSE;
    INFO_SSA_EXPLICIT_ALLOCS (result) = FALSE;

    INFO_SSA_GENERATE_FUNCOND (result) = FALSE;
    INFO_SSA_RENAMING_MODE (result) = SSA_USE_TOP;

    INFO_SSA_FUNDEF (result) = NULL;
    INFO_SSA_ASSIGN (result) = NULL;
    INFO_SSA_CONDSTMT (result) = NULL;
    INFO_SSA_FUNCOND_FOUND (result) = FALSE;
    INFO_SSA_WITHID (result) = NULL;
    INFO_SSA_FIRST_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*@}*/

/**
 *
 * @name Stacking mechanism support
 *
 * <!--
 * FOR_ALL_AVIS( fun , fundef )          : higher order function; maps "fun"
 *                                         to all avis nodes in the args/
 *                                         vardecs of "fundef"
 * node *PopSsastack(node *avis)         : pop top elem
 * node *DupTopSsastack(node *avis)      : push a copy of the top elem
 * node *SaveTopSsastackThen(node *avis) : save top elem in AVIS_SSA_THEN
 * node *SaveTopSsastackElse(node *avis) : save top elem in AVIS_SSA_ELSE
 * -->
 *
 */
/*@{*/

/**
 * Higher order function for modifying all N_avis nodes of a given fundef:
 */
#define FOR_ALL_AVIS(fun, fundef)                                                        \
    {                                                                                    \
        node *vardec;                                                                    \
        node *arg;                                                                       \
                                                                                         \
        vardec = FUNDEF_VARDEC (fundef);                                                 \
        while (vardec != NULL) {                                                         \
            VARDEC_AVIS (vardec) = fun (VARDEC_AVIS (vardec));                           \
            vardec = VARDEC_NEXT (vardec);                                               \
        }                                                                                \
        arg = FUNDEF_ARGS (fundef);                                                      \
        while (arg != NULL) {                                                            \
            ARG_AVIS (arg) = fun (ARG_AVIS (arg));                                       \
            arg = ARG_NEXT (arg);                                                        \
        }                                                                                \
    }

/** <!--********************************************************************-->
 *
 * @fn static node *PopSsastack(node *avis)
 *
 *   @brief frees top of SSAstack
 *          if stack is not in use to nothing
 *
 ******************************************************************************/
static node *
PopSsastack (node *avis)
{
    node *ssastack;

    DBUG_ENTER ("PopSsastack");

    if (AVIS_SSASTACK_INUSE (avis)) {
        ssastack = AVIS_SSASTACK (avis);
        AVIS_SSASTACK (avis) = SSASTACK_NEXT (ssastack);
        ssastack = FREEdoFreeNode (ssastack);
    }

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn static  node *DupTopSsastack(node *avis)
 *
 *   @brief Duplicates value on top of stack.
 *
 ******************************************************************************/
static node *
DupTopSsastack (node *avis)
{
    node *ssastack;

    DBUG_ENTER ("DupTopSsastack");

    if (AVIS_SSASTACK_INUSE (avis)) {
        ssastack = AVIS_SSASTACK (avis);
        AVIS_SSASTACK (avis) = TBmakeSsastack (SSASTACK_AVIS (ssastack), ssastack);
        AVIS_SSASTACK_INUSE (avis) = TRUE;
    }

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn static  node *SaveTopSsastackThen(node *avis)
 *
 *   @brief Saves Top-Value of stack to AVIS_SSATHEN
 *
 ******************************************************************************/
static node *
SaveTopSsastackThen (node *avis)
{
    DBUG_ENTER ("SaveTopSsastackThen");

    AVIS_SSATHEN (avis) = AVIS_SSASTACK_TOP (avis);

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn static  node *SaveTopSsastackElse(node *avis)
 *
 *   @brief Saves Top-Value of stack to AVIS_SSATHEN
 *
 ******************************************************************************/
static node *
SaveTopSsastackElse (node *avis)
{
    DBUG_ENTER ("SaveTopSsastackElse");

    AVIS_SSAELSE (avis) = AVIS_SSASTACK_TOP (avis);

    DBUG_RETURN (avis);
}
/*@}*/

/**
 *
 * @name global / local code transformation / generation functions:
 *
 * <!--
 * node *SSANewVardec(node *old_vardec_or_arg)
 * node *CreateFuncondAssign( node *cond, node *id, node *assign)
 * node* GetSsacnt(char *baseid, int initvalue, node *block)
 * -->
 *
 */
/*@{*/
/** <!--********************************************************************-->
 *
 * @fn node *SSATnewVardec(node *old_vardec_or_arg)
 *
 *   @brief creates a new (renamed) vardec of the given original vardec.
 *          The global ssa rename counter of the baseid is incremented.
 *          The ssacnt node is shared between all renamed instances of the
 *          original vardec.
 *
 ******************************************************************************/
node *
SSATnewVardec (node *old_vardec_or_arg)
{
    node *ssacnt;
    node *new_vardec;
    char tmpstring[TMP_STRING_LEN];

    DBUG_ENTER ("SSATnewVardec");

    ssacnt = AVIS_SSACOUNT (DECL_AVIS (old_vardec_or_arg));

    if (NODE_TYPE (old_vardec_or_arg) == N_arg) {
        new_vardec = TCmakeVardecFromArg (old_vardec_or_arg);
        if (global.compiler_phase <= PH_typecheck) {
            /**
             * we are running SSATransform prior or during TC! Therefore,
             * the type has to be virginized, i.e., set to _unknown_ !
             */
            VARDEC_TYPE (new_vardec) = FREEfreeAllTypes (VARDEC_TYPE (new_vardec));
            VARDEC_TYPE (new_vardec) = TBmakeTypes1 (T_unknown);
            DBUG_PRINT ("SBS", ("POOP"));
        }
    } else {
        new_vardec = DUPdoDupNode (old_vardec_or_arg);
    }

    /* increment ssa renaming counter */
    SSACNT_COUNT (ssacnt) = SSACNT_COUNT (ssacnt) + 1;

    /* create new unique name */
    sprintf (tmpstring, "__SSA%d_%d", global.ssaform_phase, SSACNT_COUNT (ssacnt));
    ILIBfree (VARDEC_NAME (new_vardec));
    VARDEC_NAME (new_vardec) = ILIBstringConcat (SSACNT_BASEID (ssacnt), tmpstring);
    ;

    DBUG_RETURN (new_vardec);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateFuncondAssign( node *cond, node *id, node *assign)
 *
 *   @brief create a funcond assignment of the form
 *             <id> = cond( <p>, <id>, <id>)
 *             where <id> is the identifier behind "id" and
 *                   <p> is the predicate variable behind cond
 *          and prepend it to "assign".
 *
 *          We do NOT need to deal with renaming here, as the traversal
 *          mechanism will be run on it anyways.
 *
 ******************************************************************************/
static node *
CreateFuncondAssign (node *cond, node *id, node *assign)
{
    node *funcond;
    node *new_assign;

    DBUG_ENTER ("CreateFuncondAssign");

    funcond = TBmakeFuncond (TBmakeExprs (DUPdoDupTree (COND_COND (cond)), NULL),
                             TBmakeExprs (DUPdoDupTree (id), NULL),
                             TBmakeExprs (DUPdoDupTree (id), NULL));

    new_assign = TCmakeAssignLet (ID_AVIS (id), funcond);

    ASSIGN_NEXT (new_assign) = assign;

    DBUG_RETURN (new_assign);
}

/** <!--********************************************************************-->
 *
 * @fn static node* GetSsacnt(char *baseid, int initvalue, node *block)
 *
 *   @brief looks in list of available ssacounter (in block) for matching
 *          baseid and returns the corresponding ssacnt node.
 *          if search fails it will create a new ssacnt in list (counter
 *          will be initialized with initvalue)
 *
 ******************************************************************************/
static node *
GetSsacnt (char *baseid, int initvalue, node *block)
{
    node *ssacnt;
    node *tmp;

    DBUG_ENTER ("GetSsacnt");

    ssacnt = NULL;

    if (BLOCK_SSACOUNTER (block) != NULL) {
        /* look for existing ssacnt to this base_id */

        tmp = BLOCK_SSACOUNTER (block);
        do {
            if (strcmp (SSACNT_BASEID (tmp), baseid) == 0) {
                /* matching baseid */
                ssacnt = tmp;
            }

            tmp = SSACNT_NEXT (tmp);
        } while ((tmp != NULL) && (ssacnt == NULL));
    }

    if (ssacnt == NULL) {
        /* insert NEW ssa-counter to this baseid */
        ssacnt
          = TBmakeSsacnt (initvalue, ILIBstringCopy (baseid), BLOCK_SSACOUNTER (block));

        /* add to list of ssacnt nodes */
        BLOCK_SSACOUNTER (block) = ssacnt;
    }

    DBUG_RETURN (ssacnt);
}
/*@}*/

/**
 *
 * @name Traversal functions:
 *
 */
/*@{*/
/** <!--********************************************************************-->
 *
 * @fn node *SSATfundef(node *arg_node, info *arg_info)
 *
 *   @brief traverses args and block of fundef in this order
 *
 ******************************************************************************/
node *
SSATfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATfundef");

    /*
     * process only fundefs with body
     */

    INFO_SSA_CONDSTMT (arg_info) = NULL;
    INFO_SSA_FUNCOND_FOUND (arg_info) = FALSE;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* stores access points for later insertions in this fundef */
        INFO_SSA_FUNDEF (arg_info) = arg_node;

        if (FUNDEF_ARGS (arg_node) != NULL) {
            /* there are some args */
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        if (FUNDEF_BODY (arg_node) != NULL) {
            /* there is a block */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    /* traverse next fundef */
    if ((INFO_SSA_SINGLEFUNDEF (arg_info) == SSA_TRAV_FUNDEFS)
        && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATblock(node *arg_node, info *arg_info)
 *
 *   @brief traverses vardecs and instructions in this order, subblocks do not
 *          have any vardecs.
 *
 ******************************************************************************/
node *
SSATblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("SSATblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* there are some vardecs */
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* there are some instructions */
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATassign(node *arg_node, info *arg_info)
 *
 *   @brief traverses assign chain top down. While traversing the RHS,
 *          the actual assign node is made available in INFO_SSA_ASSIGN.
 *          If this is changed during traversal of the RHS, the new value
 *          is considered an intended REPLACEMENT! Therefore, it has to be
 *          traversed and to be inserted into the actual chain of assignments.
 *
 ******************************************************************************/
node *
SSATassign (node *arg_node, info *arg_info)
{
    node *old_assign;

    DBUG_ENTER ("SSATassign");

    /* preserve the old assignment link */
    old_assign = INFO_SSA_ASSIGN (arg_info);

    INFO_SSA_ASSIGN (arg_info) = arg_node;

    /* traverse expr */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    /* check for potentially required insertions */
    if (INFO_SSA_ASSIGN (arg_info) != arg_node) {
        /* indirectly insert these here and traverse them again */
        arg_node = INFO_SSA_ASSIGN (arg_info);
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    /* traverse next exprs */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* restore old assignment link */
    INFO_SSA_ASSIGN (arg_info) = old_assign;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATlet( node *arg_node, info *arg_info)
 *
 *   @brief travereses in expression and assigned ids.
 *
 ******************************************************************************/
node *
SSATlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        /* there are some ids not in a special ssa copy let */
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSATicm( node *arg_node, info *arg_info)
 *
 * description:
 *   travereses in expression and assigned ids.
 *
 ******************************************************************************/
node *
SSATicm (node *arg_node, info *arg_info)
{
    node *id;
    char *name;

    DBUG_ENTER ("SSATicm");

    name = ICM_NAME (arg_node);

    if (strstr (name, "USE_GENVAR_OFFSET") != NULL) {
        /*
         * USE_GENVAR_OFFSET( off, wl)
         * is expanded to
         *      off = wl__off    ,
         * where 'off' is a scalar and 'wl__off' an internal variable!
         *   -> traverse 'off' as a LHS
         */
        id = ICM_ARG1 (arg_node);
        DBUG_ASSERT ((NODE_TYPE (id) == N_id),
                     "1st argument of USE_GENVAR_OFFSET icm is no N_id node");
        id = TreatIdAsLhs (id, arg_info);
    } else if (strstr (name, "VECT2OFFSET") != NULL) {
        /*
         * VECT2OFFSET( off, ., from, ., ., ...)
         * is expanded to
         *     off = ... from ...    ,
         * where 'off' is a scalar variable.
         *  -> traverse all but the first argument as RHS
         *  -> traverse first argument as LHS
         */
        ICM_EXPRS2 (arg_node) = TRAVdo (ICM_EXPRS2 (arg_node), arg_info);

        id = ICM_ARG1 (arg_node);
        DBUG_ASSERT ((NODE_TYPE (id) == N_id),
                     "1st argument of VECT2OFFSET icm is no N_id node");
        id = TreatIdAsLhs (id, arg_info);
    } else if (strstr (name, "IDXS2OFFSET") != NULL) {
        /*
         * IDXS2OFFSET( off_nt, ., idx_1_nt ... idx_n_nt, ., ...)
         * is expanded to
         *     off_nt = ... idx_1_nt[i] ... idx_n_nt[i] ...   ,
         * where 'off' is a scalar variable.
         *  -> traverse all but the first argument as RHS
         *  -> traverse first argument as LHS
         */
        ICM_EXPRS2 (arg_node) = TRAVdo (ICM_EXPRS2 (arg_node), arg_info);

        id = ICM_ARG1 (arg_node);
        DBUG_ASSERT ((NODE_TYPE (id) == N_id),
                     "1st argument of IDXS2OFFSET icm is no N_id node");
        id = TreatIdAsLhs (id, arg_info);
    } else {
        DBUG_ASSERT ((0), "unknown ICM found during RC");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATarg(node *arg_node, info *arg_info)
 *
 *   @brief check for missing SSACOUNT attribute in AVIS node. installs and
 *          inits new ssa-counter if necessary (init with 0, means unrenamed
 *          argument)
 *
 *          Note here, that quite some portion of the code only is required as
 *          SSATransform may be called several times. Therefore, all attributes
 *          have to be reset properly and the AVIS_SSACOUNT may actually exist
 *          already!
 *
 ******************************************************************************/
node *
SSATarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATarg");

    DBUG_PRINT ("SSA", ("working on arg %s", ARG_NAME (arg_node)));

    if (AVIS_SSACOUNT (ARG_AVIS (arg_node)) == NULL) {
        /* insert ssa-counter to this baseid */
        AVIS_SSACOUNT (ARG_AVIS (arg_node))
          = GetSsacnt (ARG_NAME (arg_node), 0, FUNDEF_BODY (INFO_SSA_FUNDEF (arg_info)));
    }

    /* actual rename-to target on stack*/
    AVIS_SSASTACK_TOP (ARG_AVIS (arg_node)) = ARG_AVIS (arg_node);
    AVIS_SSADEFINED (ARG_AVIS (arg_node)) = TRUE;

    /*
     * mark stack as active
     * (later added vardecs and stacks are ignored when stacking)
     */
    AVIS_SSASTACK_INUSE (ARG_AVIS (arg_node)) = TRUE;

    /* clear all traversal infos in avis node */
    AVIS_SSATHEN (ARG_AVIS (arg_node)) = NULL;
    AVIS_SSAELSE (ARG_AVIS (arg_node)) = NULL;

    /* no direct assignment available (yet) */
    AVIS_SSAASSIGN (ARG_AVIS (arg_node)) = NULL;

    /* traverse next arg */
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATvardec(node *arg_node, info *arg_info)
 *
 *   @brief check for missing SSACOUNT attribute in AVIS node.
 *          installs and inits new ssa-counter if necessary (init with undef)
 *
 *          Note here, that quite some portion of the code only is required as
 *          SSATransform may be called several times. Therefore, all attributes
 *          have to be reset properly and the AVIS_SSACOUNT may actually exist
 *          already!
 *
 ******************************************************************************/
node *
SSATvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATvardec");

    if (AVIS_SSACOUNT (VARDEC_AVIS (arg_node)) == NULL) {
        /* insert ssa-counter to this baseid */
        AVIS_SSACOUNT (VARDEC_AVIS (arg_node))
          = GetSsacnt (VARDEC_NAME (arg_node), 0,
                       FUNDEF_BODY (INFO_SSA_FUNDEF (arg_info)));
    }

    /* jet undefined on stack */
    AVIS_SSASTACK_TOP (VARDEC_AVIS (arg_node)) = NULL;
    AVIS_SSADEFINED (VARDEC_AVIS (arg_node)) = FALSE;

    /*
     * mark stack as activ
     * (later added vardecs and stacks are ignored when stacking)
     */
    AVIS_SSASTACK_INUSE (VARDEC_AVIS (arg_node)) = TRUE;

    /* clear all traversal infos in avis node */
    AVIS_SSATHEN (VARDEC_AVIS (arg_node)) = NULL;
    AVIS_SSAELSE (VARDEC_AVIS (arg_node)) = NULL;

    /* traverse next vardec */
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATid( node *arg_node, info *arg_info)
 *
 *   @brief depending on INFO_SSA_GENERATE_FUNCOND either generate funconds
 *          and prepend them to INFO_SSA_ASSIGN     or
 *          trigger renaming by traversing into the RHS ids.
 *
 ******************************************************************************/
node *
SSATid (node *arg_node, info *arg_info)
{
    node *new_avis;
    DBUG_ENTER ("SSATid");

    if (INFO_SSA_GENERATE_FUNCOND (arg_info)) {
        /* check for different assignments in then and else part */
        if (AVIS_SSATHEN (ID_AVIS (arg_node)) != AVIS_SSAELSE (ID_AVIS (arg_node))) {
            DBUG_ASSERT ((AVIS_SSATHEN (ID_AVIS (arg_node))),
                         "undefined variable in then part");
            DBUG_ASSERT ((AVIS_SSATHEN (ID_AVIS (arg_node))),
                         "undefined variable in then part");
            INFO_SSA_ASSIGN (arg_info)
              = CreateFuncondAssign (INFO_SSA_CONDSTMT (arg_info), arg_node,
                                     INFO_SSA_ASSIGN (arg_info));
        }
    } else {

        if (INFO_SSA_RENAMING_MODE (arg_info) == SSA_USE_TOP) {
            new_avis = AVIS_SSASTACK_TOP (ID_AVIS (arg_node));
        } else if (INFO_SSA_RENAMING_MODE (arg_info) == SSA_USE_THEN) {
            new_avis = AVIS_SSATHEN (ID_AVIS (arg_node));
        } else {
            DBUG_ASSERT ((INFO_SSA_RENAMING_MODE (arg_info) == SSA_USE_ELSE),
                         "illegal value for INFO_SSA_RENAMING_MODE");
            new_avis = AVIS_SSAELSE (ID_AVIS (arg_node));
        }

        /* do renaming to new ssa vardec */
        if (!AVIS_SSADEFINED (ID_AVIS (arg_node)) || (new_avis == NULL)) {
            /**
             * One may think, that it would suffice to check AVIS_SSADEFINED here.
             * However, it may happen that despite AVIS_SSADEFINED being set
             * AVIS_SSASTACK_TOP is NULL!! The reason for that is the stacking
             * mechanism for getting the scopes right.
             * If a variable is defined in one local scope and used in another one
             * which happens to be not within the first one, this situation occurs.
             *
             * Example:
             *
             *    int main()
             *    {
             *      res = with(. <= iv <.) {
             *              a=1;
             *            } genarray([20], a);
             *      res = with(. <= iv <.) {
             *            } genarray([20], a);
             *
             *      return(  0);
             *    }
             *
             * When the second occurance of a is traversed AVIS_SSADEFINED is true,
             * as there has been a definition before (remember: renaming is steered
             * without taking scopes into account!). However, wrt. the actual scope
             * there is NO valid definition. Hence AVIS_SSASTACK_TOP is NULL.
             */
            if (INFO_SSA_ALLOW_GOS (arg_info) == FALSE) {
                ERROR (global.linenum,
                       ("var %s used without definition", ID_NAME (arg_node)));
            }
        } else {
            ID_AVIS (arg_node) = new_avis;
        }

        /* restore all depended attributes with correct values */
        ID_DECL (arg_node) = AVIS_DECL (ID_AVIS (arg_node));

        if (INFO_SSA_WITHID (arg_info) != NULL) {
            AVIS_WITHID (ID_AVIS (arg_node)) = INFO_SSA_WITHID (arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATap(node *arg_node, info *arg_info)
 *
 *   @brief traverses args and does a recursive call in case of special
 *          function applications.
 *
 ******************************************************************************/
node *
SSATap (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ("SSATap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion (only in single fundef mode) */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (INFO_SSA_SINGLEFUNDEF (arg_info) == SSA_TRAV_SPECIALS)
        && (AP_FUNDEF (arg_node) != INFO_SSA_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSA", ("traverse in special fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_SSA_SINGLEFUNDEF (new_arg_info) = INFO_SSA_SINGLEFUNDEF (arg_info);

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSA", ("traversal of special fundef %s finished\n",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        new_arg_info = FreeInfo (new_arg_info);
    } else {
        DBUG_PRINT ("SSA", ("do not traverse in normal fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATwith(node *arg_node, info *arg_info)
 *
 *   @brief traverses with-op, partitions and code in this order
 *
 ******************************************************************************/
node *
SSATwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATwith");

    /* traverse in with-op */
    DBUG_ASSERT ((WITH_WITHOP (arg_node) != NULL), "Nwith without WITHOP node!");
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /**
     * traverse partitions: this implicitly checks withid consistency between
     * several partitions!
     */
    DBUG_ASSERT ((WITH_PART (arg_node) != NULL), "Nwith without PART node!");
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    /**
     * reset FIRST_WITHID (being set in SSAwithid) for the next with loop
     * traversal
     */
    INFO_SSA_FIRST_WITHID (arg_info) = NULL;

    /**
     * traverse code: as we may have more than one code stacking is done
     * at the code nodes themselves!
     */
    DBUG_ASSERT ((WITH_CODE (arg_node) != NULL), "Nwith without CODE node!");
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATwith2(node *arg_node, info *arg_info)
 *
 *   @brief traverses with-op, segs, withid and code in this order
 *
 ******************************************************************************/
node *
SSATwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATwith2");

    /* traverse in with-op */
    DBUG_ASSERT ((WITH2_WITHOP (arg_node) != NULL), "Nwith2 without WITHOP node!");
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    /* traverse segmented partitions */
    DBUG_ASSERT ((WITH2_SEGS (arg_node) != NULL), "Nwith2 without SEGS node!");
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);

    /* traverse withid */
    DBUG_ASSERT ((WITH2_WITHID (arg_node) != NULL), "Nwith2 without WITHID node!");
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
    /**
     * reset FIRST_WITHID (being set in SSAwithid) for the next with loop
     * traversal
     */
    INFO_SSA_FIRST_WITHID (arg_info) = NULL;

    /**
     * traverse code: as we may have more than one code stacking is done
     * at the code nodes themselves!
     */
    DBUG_ASSERT ((WITH2_CODE (arg_node) != NULL), "Nwith2 without CODE node!");
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATpart(node *arg_node, info *arg_info)
 *
 *   @brief as all generators have to be done BEFORE any of the withids (these
 *          are treated as LHS which should not be visible in the generators),
 *          we traverse the generators top down and the withids bottom up.
 *
 ******************************************************************************/
node *
SSATpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATpart");

    /* traverse generator */
    DBUG_ASSERT ((PART_GENERATOR (arg_node) != NULL), "Npart without Ngen node!");
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    /* traverse withid on our way back up: */
    DBUG_ASSERT ((PART_WITHID (arg_node) != NULL), "Npart without Nwithid node!");
    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATwithid(node *arg_node, info *arg_info)
 *
 *   @brief In principle, the withids have to be treated as LHS, as they
 *          introduce new variables. However, if we do have multi generator
 *          with loops, we expect all withids to be identical. Therefore,
 *          we only have to treat the very first one as LHS, all others as
 *          RHS. To distinguish these cases, we use INFO_SSA_FIRST_WITHID,
 *          which is reset in SSAwith.
 *          Another exception arises if SSATransform is called after memory
 *          allocation has been made explicit. In this case
 *          SSATransformExplicitAllocs has to be called which sets
 *          INFO_SSA_EXPLICIT_ALLOCS which forces ALL withids to be treated
 *          as RHSs.
 *
 ******************************************************************************/
node *
SSATwithid (node *arg_node, info *arg_info)
{
    node *assign;
    node *first;

    DBUG_ENTER ("SSATwithid");

    /* Set current assign to NULL for these special ids! */
    assign = INFO_SSA_ASSIGN (arg_info);
    INFO_SSA_ASSIGN (arg_info) = NULL;
    INFO_SSA_WITHID (arg_info) = arg_node;

    if (INFO_SSA_FIRST_WITHID (arg_info) == NULL) {
        /**
         * This is the first withid. Therefore, we have to treat it as LHS.
         * The only exception is when withids have been allocated explicitly
         * (refcounting). In that case, we treat them as RHSs like all the others.
         */
        INFO_SSA_FIRST_WITHID (arg_info) = arg_node;

        if (WITHID_VEC (arg_node) != NULL) {
            if (INFO_SSA_EXPLICIT_ALLOCS (arg_info)) {
                WITHID_VEC (arg_node) = TreatIdsAsRhs (WITHID_VEC (arg_node), arg_info);
            } else {
                WITHID_VEC (arg_node) = TRAVdo (WITHID_VEC (arg_node), arg_info);
            }
        }

        if (WITHID_IDS (arg_node) != NULL) {
            if (INFO_SSA_EXPLICIT_ALLOCS (arg_info)) {
                WITHID_IDS (arg_node) = TreatIdsAsRhs (WITHID_IDS (arg_node), arg_info);
            } else {
                WITHID_IDS (arg_node) = TRAVdo (WITHID_IDS (arg_node), arg_info);
            }
        }
    } else {
        /**
         * There have been prior partitions in this WL. INFO_SSA_FIRST_WITHID
         * points to the topmost one (in renamed form). => treat as RHS!
         * First, we do the renaming, if necessary. Then, we check consistency with
         * INFO_SSA_FIRST_WITHID.
         */
        first = INFO_SSA_FIRST_WITHID (arg_info);
        if (WITHID_VEC (arg_node) != NULL) {
            WITHID_VEC (arg_node) = TreatIdsAsRhs (WITHID_VEC (arg_node), arg_info);
            DBUG_ASSERT (IDS_AVIS (WITHID_VEC (arg_node))
                           == IDS_AVIS (WITHID_VEC (first)),
                         "multigenerator withloop with inconsistent withvec");
        } else {
            DBUG_ASSERT (WITHID_VEC (first) == NULL,
                         "multigenerator withloop with inconsistent withvec");
        }

        if (WITHID_IDS (arg_node) != NULL) {
            WITHID_IDS (arg_node) = TreatIdsAsRhs (WITHID_IDS (arg_node), arg_info);
            DBUG_ASSERT (IDS_AVIS (WITHID_IDS (arg_node))
                           == IDS_AVIS (WITHID_IDS (first)),
                         "multigenerator withloop with inconsistent withids");
        } else {
            DBUG_ASSERT (WITHID_IDS (first) == NULL,
                         "multigenerator withloop with inconsistent withids");
        }
    }

    /* restore currect assign for further processing */
    INFO_SSA_ASSIGN (arg_info) = assign;
    INFO_SSA_WITHID (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATcode(node *arg_node, info *arg_info)
 *
 *   @brief traverses block and expr and potentially epilogue in this order.
 *          While doing so, create new scope by pushing renaming stacks.
 *          Before traversing further code nodes reset the scope by popping.
 *
 ******************************************************************************/
node *
SSATcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATcode");

    /* do stacking of current renaming status */
    FOR_ALL_AVIS (DupTopSsastack, INFO_SSA_FUNDEF (arg_info));

    /* traverse block */
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    /* traverse expressions */
    DBUG_ASSERT ((CODE_CEXPRS (arg_node) != NULL), "Ncode without Ncexprs node!");
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    /* restore old rename stack !!! */
    FOR_ALL_AVIS (PopSsastack, INFO_SSA_FUNDEF (arg_info));

    if (CODE_NEXT (arg_node) != NULL) {
        /* traverse next part */
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATcond(node *arg_node, info *arg_info)
 *
 *   @brief this top-level conditional requires stacking of renaming status.
 *          traverses conditional, then and else branch in this order.
 *          The final renaming stati are stored in AVIS_SSATHEN and
 *          AVIS_SSAELSE, respectively.
 *
 ******************************************************************************/
node *
SSATcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATcond");

    /* save this cond_node for later insertions of copy assignments */
    INFO_SSA_CONDSTMT (arg_info) = arg_node;

    /* traverse conditional */
    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "Ncond without cond node!");
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    /* do stacking of current renaming status */
    FOR_ALL_AVIS (DupTopSsastack, INFO_SSA_FUNDEF (arg_info));

    /* traverse then */
    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    /* save to then for later merging */
    FOR_ALL_AVIS (SaveTopSsastackThen, INFO_SSA_FUNDEF (arg_info));

    /* so some status restauration */
    FOR_ALL_AVIS (PopSsastack, INFO_SSA_FUNDEF (arg_info));

    /* do stacking of current renaming status */
    FOR_ALL_AVIS (DupTopSsastack, INFO_SSA_FUNDEF (arg_info));

    /* traverse else */
    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    /* save to else for later merging */
    FOR_ALL_AVIS (SaveTopSsastackElse, INFO_SSA_FUNDEF (arg_info));

    /* so some status restauration */
    FOR_ALL_AVIS (PopSsastack, INFO_SSA_FUNDEF (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATfuncond(node *arg_node, info *arg_info)
 *
 *   @brief set INFO_SSA_FUNCOND_FOUND and traverse all sons
 *
 ******************************************************************************/
node *
SSATfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATfuncond");

    INFO_SSA_FUNCOND_FOUND (arg_info) = TRUE;

    if (FUNCOND_IF (arg_node) != NULL) {
        FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);
    }

    if (FUNCOND_THEN (arg_node) != NULL) {
        /**
         * iff there was a cond-instruction before, the correct renaming info
         * is found in the AVIS_SSATHEN node! Otherwise AVIS_SSASTACK_TOP can
         * be used as usual.
         */
        INFO_SSA_RENAMING_MODE (arg_info)
          = (INFO_SSA_CONDSTMT (arg_info) ? SSA_USE_THEN : SSA_USE_TOP);
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
        INFO_SSA_RENAMING_MODE (arg_info) = SSA_USE_TOP;
    }

    if (FUNCOND_ELSE (arg_node) != NULL) {
        INFO_SSA_RENAMING_MODE (arg_info)
          = (INFO_SSA_CONDSTMT (arg_info) ? SSA_USE_ELSE : SSA_USE_TOP);
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
        INFO_SSA_RENAMING_MODE (arg_info) = SSA_USE_TOP;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATreturn(node *arg_node, info *arg_info)
 *
 *   @brief inserts missing funcond nodes and traverses the return elements
 *
 ******************************************************************************/
node *
SSATreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SSATreturn");

    /**
     * check whether this function contains a conditional but does not contain
     * funconds yet. If this is the case, switch the traversal mode to
     * "GENERATE_FUNCOND". This will insert new funcond assignments into
     * INFO_SSA_ASSIGN, but do no SSA conversion at all!
     * On return, SSAassign will notice these insertions, traverse them
     * (including this return node again!) for SSA construction, and finally
     * insert them into the actual chain.
     */
    if ((INFO_SSA_CONDSTMT (arg_info) != NULL) && !INFO_SSA_FUNCOND_FOUND (arg_info)) {

        INFO_SSA_GENERATE_FUNCOND (arg_info) = TRUE;
        /**
         * Though a rare case, we may not have to generate funconds at all.
         * e.g., in void functions or if the return values do not depend
         * at all on vars defined in the branches.
         * To avoid non-termination in these cases, we set FUNCOND_FOUND!
         */
        INFO_SSA_FUNCOND_FOUND (arg_info) = TRUE;
    }

    /* traverse exprs */
    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    INFO_SSA_GENERATE_FUNCOND (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/*@}*/

/**
 *
 * @name IDS traversal functions:
 *
 * <!--
 * ids *TravLeftIDS(ids *arg_ids, info *arg_info)
 * ids *SSAleftids(ids *arg_ids, info *arg_info)
 *
 * ids *TravRightIDS(ids *arg_ids, info *arg_info)
 * ids *SSArightids(ids *arg_ids, info *arg_info)
 * -->
 *
 * while the Travxxxxx versions are merely wrappers, SSAleftids and SSArightids
 * constitute the core of the entire renaming mechanism!! SSAleftids is called
 * for ALL defining occurances of variables( i.e. LHSs), and SSArightids
 * is applied to all RHS occurances!
 *
 */
/*@{*/
/** <!--********************************************************************-->
 *
 * @fn node *SSATids(node *arg_ids, info *arg_info)
 *
 *   @brief creates new (renamed) instance of defined variables.
 *
 ******************************************************************************/

node *
SSATids (node *arg_ids, info *arg_info)
{
    node *new_vardec;

    DBUG_ENTER ("SSATids");

    if (!AVIS_SSADEFINED (IDS_AVIS (arg_ids))) {
        /* first definition of variable (no renaming) */
        AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids)) = IDS_AVIS (arg_ids);
        /* SSACNT_COUNT(AVIS_SSACOUNT(IDS_AVIS(arg_ids))) = 0; */
        AVIS_SSADEFINED (IDS_AVIS (arg_ids)) = TRUE;
        DBUG_PRINT ("SSA", ("first definition, no renaming: %s (" F_PTR ")",
                            AVIS_NAME (IDS_AVIS (arg_ids)), IDS_AVIS (arg_ids)));

        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = INFO_SSA_ASSIGN (arg_info);
        AVIS_WITHID (IDS_AVIS (arg_ids)) = INFO_SSA_WITHID (arg_info);

    } else {
        /* redefinition - create new unique variable/vardec */
        new_vardec = SSATnewVardec (AVIS_DECL (IDS_AVIS (arg_ids)));
        FUNDEF_VARDEC (INFO_SSA_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_SSA_FUNDEF (arg_info)), new_vardec);
        DBUG_PRINT ("SSA", ("re-definition, renaming: %s (" F_PTR ") -> %s",
                            AVIS_NAME (IDS_AVIS (arg_ids)), IDS_AVIS (arg_ids),
                            AVIS_NAME (VARDEC_AVIS (new_vardec))));

        /* new rename-to target for old vardec */
        AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids)) = VARDEC_AVIS (new_vardec);

        if ((global.compiler_phase != PH_flatten)
            && (global.compiler_phase != PH_typecheck))
            AVIS_TYPE (VARDEC_AVIS (new_vardec))
              = TYcopyType (AVIS_TYPE (IDS_AVIS (arg_ids)));

        /* rename this ids */
        IDS_AVIS (arg_ids) = VARDEC_AVIS (new_vardec);

#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        ILIBfree (IDS_NAME (arg_ids));
        IDS_NAME (arg_ids) = ILIBstringCopy (VARDEC_NAME (new_vardec));
#endif

        /*
         * mark this avis for undo ssa transform:
         * all global objects and artificial identifier must
         * be mapped back to their original name in undossa.
         */
        if (((NODE_TYPE (AVIS_DECL (IDS_AVIS (arg_ids))) == N_arg)
             && (ARG_ISARTIFICIAL (AVIS_DECL (IDS_AVIS (arg_ids)))))
            || (AVIS_ISUNIQUE (IDS_AVIS (arg_ids)))) {

            AVIS_SSAUNDOFLAG (IDS_AVIS (arg_ids)) = TRUE;
        }
        AVIS_SSAASSIGN (IDS_AVIS (arg_ids)) = INFO_SSA_ASSIGN (arg_info);
        AVIS_WITHID (IDS_AVIS (arg_ids)) = INFO_SSA_WITHID (arg_info);
    }

    /* traverese next ids */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TRAVdo (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/** <!--********************************************************************-->
 *
 * @fn node *TreatIdAsLhs(node *arg_ids, info *arg_info)
 *
 *   @brief creates new (renamed) instance of defined variables.
 *          is an identical copy from SSATids, here N_id nodes are
 *          used instead of N_ids.
 *
 ******************************************************************************/

node *
TreatIdAsLhs (node *arg_node, info *arg_info)
{
    node *new_vardec;

    DBUG_ENTER ("TreatIdAsLhs");

    if (!AVIS_SSADEFINED (ID_AVIS (arg_node))) {
        /* first definition of variable (no renaming) */
        AVIS_SSASTACK_TOP (ID_AVIS (arg_node)) = ID_AVIS (arg_node);
        /* SSACNT_COUNT(AVIS_SSACOUNT(ID_AVIS(arg_node))) = 0; */
        AVIS_SSADEFINED (ID_AVIS (arg_node)) = TRUE;
        DBUG_PRINT ("SSA", ("first definition, no renaming: %s (" F_PTR ")",
                            AVIS_NAME (ID_AVIS (arg_node)), ID_AVIS (arg_node)));

        AVIS_SSAASSIGN (ID_AVIS (arg_node)) = INFO_SSA_ASSIGN (arg_info);
        AVIS_WITHID (ID_AVIS (arg_node)) = INFO_SSA_WITHID (arg_info);

    } else {
        /* redefinition - create new unique variable/vardec */
        new_vardec = SSATnewVardec (AVIS_DECL (ID_AVIS (arg_node)));
        FUNDEF_VARDEC (INFO_SSA_FUNDEF (arg_info))
          = TCappendVardec (FUNDEF_VARDEC (INFO_SSA_FUNDEF (arg_info)), new_vardec);
        DBUG_PRINT ("SSA", ("re-definition, renaming: %s (" F_PTR ") -> %s",
                            AVIS_NAME (ID_AVIS (arg_node)), ID_AVIS (arg_node),
                            AVIS_NAME (VARDEC_AVIS (new_vardec))));

        /* new rename-to target for old vardec */
        AVIS_SSASTACK_TOP (ID_AVIS (arg_node)) = VARDEC_AVIS (new_vardec);

        if ((global.compiler_phase != PH_flatten)
            && (global.compiler_phase != PH_typecheck))
            AVIS_TYPE (VARDEC_AVIS (new_vardec))
              = TYcopyType (AVIS_TYPE (ID_AVIS (arg_node)));

        /* rename this ids */
        ID_AVIS (arg_node) = VARDEC_AVIS (new_vardec);

#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        ILIBfree (ID_NAME (arg_node));
        ID_NAME (arg_node) = ILIBstringCopy (VARDEC_NAME (new_vardec));
#endif

        /*
         * mark this avis for undo ssa transform:
         * all global objects and artificial identifier must
         * be mapped back to their original name in undossa.
         */
        if (((NODE_TYPE (ID_DECL (arg_node)) == N_arg)
             && (ARG_ISARTIFICIAL (ID_DECL (arg_node))))
            || (AVIS_ISUNIQUE (ID_AVIS (arg_node)))) {
            AVIS_SSAUNDOFLAG (ID_AVIS (arg_node)) = TRUE;
        }
        AVIS_SSAASSIGN (ID_AVIS (arg_node)) = INFO_SSA_ASSIGN (arg_info);
        AVIS_WITHID (ID_AVIS (arg_node)) = INFO_SSA_WITHID (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSArightids(ids *arg_ids, info *arg_info)
 *
 *   @brief rename variable to actual ssa renaming counter. Depending on
 *          INFO_SSA_RENAMING_MODE, this is either AVIS_SSASTACK_TOP,
 *          AVIS_SSATHEN, or AVIS_SSAELSE.
 *
 ******************************************************************************/
static node *
TreatIdsAsRhs (node *arg_ids, info *arg_info)
{
    node *new_avis;

    DBUG_ENTER ("TreatIdsAsRhs");

    if (INFO_SSA_RENAMING_MODE (arg_info) == SSA_USE_TOP) {
        new_avis = AVIS_SSASTACK_TOP (IDS_AVIS (arg_ids));
    } else if (INFO_SSA_RENAMING_MODE (arg_info) == SSA_USE_THEN) {
        new_avis = AVIS_SSATHEN (IDS_AVIS (arg_ids));
    } else {
        DBUG_ASSERT ((INFO_SSA_RENAMING_MODE (arg_info) == SSA_USE_ELSE),
                     "illegal value for INFO_SSA_RENAMING_MODE");
        new_avis = AVIS_SSAELSE (IDS_AVIS (arg_ids));
    }

    /* do renaming to new ssa vardec */
    if (!AVIS_SSADEFINED (IDS_AVIS (arg_ids)) || (new_avis == NULL)) {
        /**
         * One may think, that it would suffice to check AVIS_SSADEFINED here.
         * However, it may happen that despite AVIS_SSADEFINED being set
         * AVIS_SSASTACK_TOP is NULL!! The reason for that is the stacking
         * mechanism for getting the scopes right.
         * If a variable is defined in one local scope and used in another one
         * which happens to be not within the first one, this situation occurs.
         *
         * Example:
         *
         *    int main()
         *    {
         *      res = with(. <= iv <.) {
         *              a=1;
         *            } genarray([20], a);
         *      res = with(. <= iv <.) {
         *            } genarray([20], a);
         *
         *      return(  0);
         *    }
         *
         * When the second occurance of a is traversed AVIS_SSADEFINED is true,
         * as there has been a definition before (remember: renaming is steered
         * without taking scopes into account!). However, wrt. the actual scope
         * there is NO valid definition. Hence AVIS_SSASTACK_TOP is NULL.
         */
        if (INFO_SSA_ALLOW_GOS (arg_info) == FALSE) {
            ERROR (global.linenum,
                   ("var %s used without definition", IDS_NAME (arg_ids)));
        }
    } else {
        IDS_AVIS (arg_ids) = new_avis;
    }

    if (INFO_SSA_WITHID (arg_info) != NULL) {
        AVIS_WITHID (IDS_AVIS (arg_ids)) = INFO_SSA_WITHID (arg_info);
    }

    /* traverese next ids */
    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TreatIdsAsRhs (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/*@}*/

/**
 *
 * @name Start functions:
 *
 * <!--
 * node *SSATransform(node *ast)            : general traversal function
 * node *SSATransformAllowGOs(node *ast)    : ignore usages of non-defined vars
 * node *SSATransformExplicitAllocs(node *syntax_tree) : support explicit
 *                                                       withid allocations
 * node *SSATransformOneFunction(node *ast) : one fundef + included LC funs
 * node *SSATransformOneFundef(node *ast)   : one fundef only
 *
 * -->
 *
 */
/*@{*/
/** <!--********************************************************************-->
 *
 * @fn node *SSATdoTransform(node *syntax_tree)
 *
 *   @brief Starts traversal of AST to transform code in SSA form. Every
 *          variable has exaclty one definition. This code transformtion relies
 *          on the lac2fun transformation! After all the valid_ssaform flag is
 *          set to TRUE. It may be called several times until eventually
 *          UndoSSATransform is called. This sets valid_ssaform to false
 *          and increases the ssaform_phase global counter to avoid naming
 *          conflicts with further SSA-UndoSSA transformations.
 *
 ******************************************************************************/
node *
SSATdoTransform (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("SSATdoTransform");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "SSATransform is used for module nodes only");

#ifndef DBUG_OFF
    if (global.compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("starting ssa transformation for ast"));
    }
#endif

    arg_info = MakeInfo ();
    INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_FUNDEFS;
    INFO_SSA_ALLOW_GOS (arg_info) = FALSE;

    TRAVpush (TR_ssat);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    global.valid_ssaform = TRUE;

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATdoTransformAllowGOs(node *syntax_tree)
 *
 *   @brief In principle, this is only a wrapper for SSATransform. The only
 *          difference is that it does not require all variables to be defined
 *          before used!!!
 *          This variant is required when the SSA form has to be built before
 *          introducing explicit data dependencies for GOs (global objects).
 *
 ******************************************************************************/
node *
SSATdoTransformAllowGOs (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("SSATdoTransformAllowGOs");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "SSATdoTransformAllowGos is used for module nodes only");

#ifndef DBUG_OFF
    if (global.compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("starting ssa transformation allowing GOs for ast"));
    }
#endif

    arg_info = MakeInfo ();
    INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_FUNDEFS;
    INFO_SSA_ALLOW_GOS (arg_info) = TRUE;

    TRAVpush (TR_ssat);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    global.valid_ssaform = TRUE;

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATdoTransformExplicitAllocs(node *syntax_tree)
 *
 *   @brief In principle, this is only a wrapper for SSATransform. The only
 *          difference is that it assumes the withid elements to be initialized
 *          explicitlt BEFORE each WL. Therefore, SSAWithid has to behave
 *          slightly different.
 *          This variant is required after explicit reference counting
 *          has been introduced in the syntax tree.
 *
 ******************************************************************************/
node *
SSATdoTransformExplicitAllocs (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("SSATransformExplicitAllocs");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "SSATransformExplicitAllocs is used for module nodes only");

#ifndef DBUG_OFF
    if (global.compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT",
                    ("starting ssa transformation expecting explicit withid allocs"));
    }
#endif

    arg_info = MakeInfo ();
    INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_FUNDEFS;
    INFO_SSA_EXPLICIT_ALLOCS (arg_info) = TRUE;

    TRAVpush (TR_ssat);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    global.valid_ssaform = TRUE;

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATdoTransformOneFunction(node *fundef)
 *
 *   @brief same as SSATransform, but traverses the given function only.
 *          Does not trverse through LC functions per se, but if a call
 *          to an LC function is found, it will be followed through.
 *
 ******************************************************************************/
node *
SSATdoTransformOneFunction (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("SSATdoTransformOneFunction");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSATransformOneFunction is used for fundef nodes only");

    if (!(FUNDEF_ISLACFUN (fundef))) {
#ifndef DBUG_OFF
        if (global.compiler_phase == PH_sacopt) {
            DBUG_PRINT ("OPT",
                        ("starting ssa transformation for %s", FUNDEF_NAME (fundef)));
        }
#endif

        arg_info = MakeInfo ();
        INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_SPECIALS;

        TRAVpush (TR_ssat);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSATdoTransformOneFundef(node *fundef)
 *
 *   @brief same as SSATransform, but traverses the given fundef only.
 *          Any LC functions will not be followed through!
 *
 ******************************************************************************/
node *
SSATdoTransformOneFundef (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("SSATdoTransformOneFundef");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSATdoTransformOneFundef is applicable to fundef nodes only");

#ifndef DBUG_OFF
    if (global.compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("starting ssa transformation for %s", FUNDEF_NAME (fundef)));
    }
#endif

    arg_info = MakeInfo ();
    INFO_SSA_SINGLEFUNDEF (arg_info) = SSA_TRAV_NONE;

    TRAVpush (TR_ssat);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (fundef);
}
/*@}*/

/*@}*/ /* defgroup ssatransform */
