/**
 *
 * @file pattern_match_attribs.c
 *
 * Overview:
 * =========
 *
 */

#include <stdarg.h>
#include "pattern_match_attribs.h"
#include "pattern_match.h"

#define DBUG_PREFIX "PMA"
#include "debug.h"

#include "memory.h"
#include "print.h"
#include "globals.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "new_types.h"
#include "tree_compound.h"
#include "constants.h"
#include "shape.h"
#include "str.h"

typedef bool attribFun (attrib *, node *);

struct PATTR {
    nodetype nt;
    prf fun;
    prf *fun_p;
    constant **c_arg1;
    node **n_arg1;
    int *i_arg1;
    int *i_arg2;
    attribFun *matcher;
};

#define PATTR_NT(p) (p->nt)
#define PATTR_PRF(p) (p->fun)
#define PATTR_PRFP(p) (p->fun_p)
#define PATTR_C1(p) (p->c_arg1)
#define PATTR_I1(p) (p->i_arg1)
#define PATTR_I2(p) (p->i_arg2)
#define PATTR_N1(p) (p->n_arg1)
#define PATTR_FUN(p) (p->matcher)

static attrib *
makeAttrib (nodetype nt, attribFun f)
{
    attrib *res;

    res = (attrib *)MEMmalloc (sizeof (attrib));

    PATTR_NT (res) = nt;
    PATTR_FUN (res) = f;

    return (res);
}

#define PMASTART PMINDENT " attrib: "
#define PMARESULT PMINDENT " ------> "

/** <!--*********************************************************************-->
 *
 * local helper functions:
 */

/** <!--*********************************************************************-->
 *
 * Exported functions for pattern matching on node attributes:
 */

attrib *
PMAfree (attrib *a)
{
    if (a != NULL) {
        a = (attrib *)MEMfree (a);
    }
    return (a);
}

bool
PMAmatch (attrib *attr, node *arg)
{
    return (PATTR_FUN (attr) (attr, arg));
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetNode( node **match)
 *
 * @brief genericly applicable attrib matcher, fetches the entire node
 *        and saves it behind *match.
 *
 *****************************************************************************/
bool
attribGetNode (attrib *attr, node *arg)
{
    DBUG_PRINT (PMASTART "PMAgetNode( " F_PTR " ):", (void *)PATTR_N1 (attr));

    if (PATTR_N1 (attr) != NULL) {
        *PATTR_N1 (attr) = arg;
        if (arg != NULL) {
            DBUG_PRINT (PMARESULT "%s %s%s%s (" F_PTR ").",
                        global.mdb_nodetype[NODE_TYPE (arg)],
                        (NODE_TYPE (arg) == N_id ? "\"" : ""),
                        (NODE_TYPE (arg) == N_id ? ID_NAME (arg) : ""),
                        (NODE_TYPE (arg) == N_id ? "\"" : ""), (void *)arg);
        } else {
            DBUG_PRINT (PMARESULT "NULL");
        }
    } else {
        DBUG_PRINT (PMARESULT "redundant PMAgetNode attribute!");
    }
    return (TRUE);
}

attrib *
PMAgetNode (node **match)
{
    attrib *res;

    res = makeAttrib (N_module, attribGetNode);
    PATTR_N1 (res) = match;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAisNode( node **match)
 *
 * @brief generic attrib checks for pointer equality
 *
 *****************************************************************************/
bool
attribIsNode (attrib *attr, node *arg)
{
    bool res;

    DBUG_ASSERT (*PATTR_N1 (attr) != NULL, "node in PMAisNode compared without"
                                           "being set yet!");
    DBUG_PRINT (PMASTART "PMAisNode( " F_PTR " ):", (void *)*PATTR_N1 (attr));

    res = (arg == *PATTR_N1 (attr));
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "fail"));

    return (res);
}

attrib *
PMAisNode (node **match)
{
    attrib *res;

    DBUG_ASSERT (match != NULL, "PMAisNode called with NULL argument");
    res = makeAttrib (N_module, attribIsNode);
    PATTR_N1 (res) = match;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetNodeOrAvis( node **match)
 *
 * @brief genericly applicable attrib matcher, fetches the entire node
 *        and either    saves it behind *match         (if not N_id node)
 *            or        saves its N_avis behind *match (if N_id node).
 *
 *****************************************************************************/
bool
attribGetNodeOrAvis (attrib *attr, node *arg)
{
    DBUG_PRINT (PMASTART "PMAgetNodeOrAvis( " F_PTR " ):", (void *)PATTR_N1 (attr));

    if (PATTR_N1 (attr) != NULL) {
        *PATTR_N1 (attr) = (NODE_TYPE (arg) == N_id ? ID_AVIS (arg) : arg);
        if (arg != NULL) {
            DBUG_PRINT (PMARESULT "%s %s%s%s (" F_PTR ").",
                        global.mdb_nodetype[NODE_TYPE (arg)],
                        (NODE_TYPE (arg) == N_id ? "\"" : ""),
                        (NODE_TYPE (arg) == N_id ? ID_NAME (arg) : ""),
                        (NODE_TYPE (arg) == N_id ? "\"" : ""), (void *)arg);
        } else {
            DBUG_PRINT (PMARESULT "NULL");
        }
    } else {
        DBUG_PRINT (PMARESULT "redundant PMAgetNodeOrAvis attribute!");
    }
    return (TRUE);
}

attrib *
PMAgetNodeOrAvis (node **match)
{
    attrib *res;

    res = makeAttrib (N_module, attribGetNodeOrAvis);
    PATTR_N1 (res) = match;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAisNodeOrAvis( node **match)
 *
 * @brief generic attrib checks for pointer equality
 *
 *****************************************************************************/
bool
attribIsNodeOrAvis (attrib *attr, node *arg)
{
    bool res;

    DBUG_ASSERT (*PATTR_N1 (attr) != NULL, "node in PMAisNodeOrAvis compared without"
                                           "being set yet!");
    DBUG_PRINT (PMASTART "PMAisNodeOrAvis( " F_PTR " ):", (void *)*PATTR_N1 (attr));

    arg = (NODE_TYPE (arg) == N_id ? ID_AVIS (arg) : arg);
    res = (arg == *PATTR_N1 (attr));
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "fail"));

    return (res);
}

attrib *
PMAisNodeOrAvis (node **match)
{
    attrib *res;

    DBUG_ASSERT (match != NULL, "PMAisNodeOrAvis called with NULL argument");
    res = makeAttrib (N_module, attribIsNodeOrAvis);
    PATTR_N1 (res) = match;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAisVar( node **var)
 *
 * @brief attrib for PMvar/PMparam checks for equality
 *
 *****************************************************************************/
bool
attribIsVar (attrib *attr, node *arg)
{
    bool res;

    DBUG_ASSERT (*PATTR_N1 (attr) != NULL, "var in PMAisVar compared without"
                                           "being set yet!");
    DBUG_ASSERT (NODE_TYPE (*PATTR_N1 (attr)) == N_id,
                 "var in PMAisVar points to a non N_id node");
    DBUG_PRINT (PMASTART "PMAisVar( & \"%s\" (" F_PTR ") ):", ID_NAME (*PATTR_N1 (attr)),
                (void *)*PATTR_N1 (attr));

    res = ID_AVIS (arg) == ID_AVIS (*PATTR_N1 (attr));
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "fail"));

    return (res);
}

attrib *
PMAisVar (node **var)
{
    attrib *res;

    DBUG_ASSERT (var != NULL, "PMAisVar called with NULL argument");
    res = makeAttrib (N_id, attribIsVar);
    PATTR_N1 (res) = var;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetSaaShape( node **shp)
 *
 * @brief attrib for PMvar/PMparam.
 *        Matches if AVIS-SHAPE exists and hands it back.
 *
 *****************************************************************************/
bool
attribgetSaaShape (attrib *attr, node *arg)
{
    bool res;

    DBUG_PRINT (PMASTART "PMAgetSaaShape( " F_PTR " ):", (void *)PATTR_N1 (attr));

    *PATTR_N1 (attr) = AVIS_SHAPE (ID_AVIS (arg));
    res = ((*PATTR_N1 (attr)) != NULL);
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "fail"));

    return (res);
}

attrib *
PMAgetSaaShape (node **shp)
{
    attrib *res;

    res = makeAttrib (N_id, attribgetSaaShape);
    PATTR_N1 (res) = shp;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetAvis( node **shp)
 *
 * @brief attrib for PMvar/PMparam. Matches iff Avis exists and hands it back.
 *
 *****************************************************************************/
bool
attribgetAvis (attrib *attr, node *arg)
{
    bool res;

    DBUG_PRINT (PMASTART "PMAgetAvis( " F_PTR " ):", (void *)PATTR_N1 (attr));

    *PATTR_N1 (attr) = ID_AVIS (arg);
    res = ((*PATTR_N1 (attr)) != NULL);
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "fail"));

    return (res);
}

attrib *
PMAgetAvis (node **avis)
{
    attrib *res;

    res = makeAttrib (N_id, attribgetAvis);
    PATTR_N1 (res) = avis;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAhasAvis( node **shp)
 *
 * @brief attrib for PMvar/PMparam. Matches if the attached Avis is identical
 *        to the one provided.
 *
 *****************************************************************************/
bool
attribHasAvis (attrib *attr, node *arg)
{
    bool res;

    DBUG_PRINT (PMASTART "PMAhasAvis( & \"%s\" (" F_PTR ") ):",
                AVIS_NAME (*PATTR_N1 (attr)), (void *)*PATTR_N1 (attr));

    res = ((*PATTR_N1 (attr)) == ID_AVIS (arg));
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "fail"));

    return (res);
}

attrib *
PMAhasAvis (node **avis)
{
    attrib *res;

    DBUG_ASSERT (avis != NULL, "PMAhasAvis called with NULL argument");
    res = makeAttrib (N_id, attribHasAvis);
    PATTR_N1 (res) = avis;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetVal( constant **c)
 *
 * @brief attrib for PMconst creates and hands back constant c
 *        NB: in case *c != NULL, that constant is freed!!
 *            This enables automated reuse without memory leaks!
 *
 *****************************************************************************/
bool
attribGetVal (attrib *attr, node *arg)
{
    constant **c;
#ifndef DBUG_OFF
    char *co_str = NULL;
#endif

    c = PATTR_C1 (attr);
    DBUG_PRINT (PMASTART "PMAgetVal( " F_PTR " ):", (void *)c);

    if (*c != NULL) {
        DBUG_PRINT (PMARESULT "pre-existing constant freed!");
        *c = COfreeConstant (*c);
    }
    *c = COaST2Constant (arg);
    DBUG_EXECUTE (co_str = COconstant2String (*c));
    DBUG_PRINT (PMARESULT "%s in *" F_PTR, co_str, (void *)c);
    DBUG_EXECUTE (co_str = MEMfree (co_str));
    return (TRUE);
}

attrib *
PMAgetVal (constant **c)
{
    attrib *res;

    res = makeAttrib (N_module, attribGetVal);
    PATTR_C1 (res) = c;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAisVal( constant **c)
 *
 * @brief attrib for PMconst creates and compares constant against c
 *
 *****************************************************************************/
bool
attribIsVal (attrib *attr, node *arg)
{
    constant *c;
    constant *c2;
    bool res;
#ifndef DBUG_OFF
    char *co_str = NULL;
#endif

    c = *PATTR_C1 (attr);
    DBUG_EXECUTE (co_str = COconstant2String (c));
    DBUG_PRINT (PMASTART "PMAisVal( %s in *" F_PTR " ):", co_str, (void *)c);
    DBUG_EXECUTE (co_str = MEMfree (co_str));
    c2 = COaST2Constant (arg);

    res = COcompareConstants (c, c2);
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "no match"));
    c2 = COfreeConstant (c2);

    return (res);
}

attrib *
PMAisVal (constant **c)
{
    attrib *res;

    res = makeAttrib (N_module, attribIsVal);
    PATTR_C1 (res) = c;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAanyLeVal( constant **c)
 *
 * @brief attrib for PMconst checks whether the found constant has any
 *        component <= c
 *
 *****************************************************************************/
bool
attribAnyLeVal (attrib *attr, node *arg)
{
    bool res;
    constant *c, *c2, *c3;
#ifndef DBUG_OFF
    char *co_str = NULL;
#endif

    c = *PATTR_C1 (attr);
    DBUG_EXECUTE (co_str = COconstant2String (c));
    DBUG_PRINT (PMASTART "PMAanyLeVal( %s in *" F_PTR " ):", co_str, (void *)c);
    DBUG_EXECUTE (co_str = MEMfree (co_str));
    c2 = COaST2Constant (arg);
    c3 = COle (c, c2, NULL);

    res = COisTrue (c3, FALSE);

    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "no match"));
    c2 = COfreeConstant (c2);
    c3 = COfreeConstant (c3);

    return (res);
}

attrib *
PMAanyLeVal (constant **c)
{
    attrib *res;

    res = makeAttrib (N_module, attribAnyLeVal);
    PATTR_C1 (res) = c;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetIVal( int *i)
 *
 * @brief attrib for PMint creates and hands back integer i
 *
 *****************************************************************************/
bool
attribGetIVal (attrib *attr, node *arg)
{
    DBUG_PRINT (PMASTART "PMAgetVal( " F_PTR " ):", (void *)PATTR_I1 (attr));

    *PATTR_I1 (attr) = NUM_VAL (arg);
    DBUG_PRINT (PMARESULT "%d in *" F_PTR, NUM_VAL (arg), (void *)PATTR_I1 (attr));

    return (TRUE);
}

attrib *
PMAgetIVal (int *i)
{
    attrib *res;

    res = makeAttrib (N_num, attribGetIVal);
    PATTR_I1 (res) = i;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAisIVal( int *i)
 *
 * @brief attrib for PMint checks whether the found integer is equal to *i.
 *
 *****************************************************************************/
bool
attribIsIVal (attrib *attr, node *arg)
{
    bool res;

    DBUG_PRINT (PMASTART "PMAisIVal( %d in *" F_PTR " ):", *PATTR_I1 (attr),
                (void *)PATTR_I1 (attr));
    res = *PATTR_I1 (attr) == NUM_VAL (arg);
    DBUG_PRINT (PMARESULT "%s (%d found)", (res ? "match" : "no match"), NUM_VAL (arg));

    return (res);
}

attrib *
PMAisIVal (int *i)
{
    attrib *res;

    res = makeAttrib (N_num, attribIsIVal);
    PATTR_I1 (res) = i;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAleIVal( int *i)
 *
 * @brief attrib for PMint checks whether the found integer is <= *i.
 *
 *****************************************************************************/
bool
attribLeIVal (attrib *attr, node *arg)
{
    bool res;

    DBUG_PRINT (PMASTART "PMAisLEIVal( %d in *" F_PTR " ):", *PATTR_I1 (attr),
                (void *)PATTR_I1 (attr));
    res = *PATTR_I1 (attr) <= NUM_VAL (arg);

    DBUG_PRINT (PMARESULT "%s (%d found)", (res ? "match" : "no match"), NUM_VAL (arg));

    return (res);
}

attrib *
PMAleIVal (int *i)
{
    attrib *res;

    res = makeAttrib (N_num, attribLeIVal);
    PATTR_I1 (res) = i;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetLen( int *l)
 *
 * @brief attrib for PMarray returns the ravel of the FS of the array in *l
 *
 *****************************************************************************/
bool
attribGetLen (attrib *attr, node *arg)
{
    DBUG_PRINT (PMASTART "PMAgetLen( " F_PTR " ):", (void *)PATTR_I1 (attr));

    *PATTR_I1 (attr) = SHgetUnrLen (ARRAY_FRAMESHAPE (arg));
    DBUG_PRINT (PMARESULT "%d in *" F_PTR, *PATTR_I1 (attr), (void *)PATTR_I1 (attr));

    return (TRUE);
}

attrib *
PMAgetLen (int *l)
{
    attrib *res;

    res = makeAttrib (N_array, attribGetLen);
    PATTR_I1 (res) = l;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAhasLen( int *l)
 *
 * @brief attrib for PMarray checks ravel-len against *l
 *
 *****************************************************************************/
bool
attribHasLen (attrib *attr, node *arg)
{
    bool res;

    DBUG_PRINT (PMASTART "PMAhasLen( %d in *" F_PTR " ):", *PATTR_I1 (attr),
                (void *)PATTR_I1 (attr));
    res = *PATTR_I1 (attr) == SHgetUnrLen (ARRAY_FRAMESHAPE (arg));
    DBUG_PRINT (PMARESULT "%s (length %lld found)", (res ? "match" : "no match"),
                SHgetUnrLen (ARRAY_FRAMESHAPE (arg)));
    return (res);
}

attrib *
PMAhasLen (int *l)
{
    attrib *res;

    res = makeAttrib (N_array, attribHasLen);
    PATTR_I1 (res) = l;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetFS( constant **fs)
 *
 * @brief attrib for PMarray returns the frameshape.
 *        NB: if *fs != NULL we free that constant!
 *
 *****************************************************************************/
bool
attribGetFS (attrib *attr, node *arg)
{
#ifndef DBUG_OFF
    char *co_str = NULL;
#endif

    DBUG_PRINT (PMASTART "PMAgetFS( " F_PTR " ):", (void *)PATTR_C1 (attr));

    if (*PATTR_C1 (attr) != NULL) {
        DBUG_PRINT (PMARESULT "pre-existing frame shape freed!");
        *PATTR_C1 (attr) = COfreeConstant (*PATTR_C1 (attr));
    }
    *PATTR_C1 (attr) = COmakeConstantFromShape (ARRAY_FRAMESHAPE (arg));

    DBUG_EXECUTE (co_str = COconstant2String (*PATTR_C1 (attr)));
    DBUG_PRINT (PMARESULT "%s in *" F_PTR, co_str, (void *)PATTR_C1 (attr));
    DBUG_EXECUTE (co_str = MEMfree (co_str));

    return (TRUE);
}

attrib *
PMAgetFS (constant **fs)
{
    attrib *res;

    res = makeAttrib (N_array, attribGetFS);
    PATTR_C1 (res) = fs;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAhasFS( constant **fs)
 *
 * @brief attrib for PMarray checks frameshape against *fs.
 *
 *****************************************************************************/
bool
attribHasFS (attrib *attr, node *arg)
{
    constant *c2;
    bool res;
#ifndef DBUG_OFF
    char *co_str = NULL;
#endif

    DBUG_EXECUTE (co_str = COconstant2String (*PATTR_C1 (attr)));
    DBUG_PRINT (PMASTART "PMAhasFS( %s in *" F_PTR " ):", co_str, (void *)PATTR_C1 (attr));
    DBUG_EXECUTE (co_str = MEMfree (co_str));

    c2 = COmakeConstantFromShape (ARRAY_FRAMESHAPE (arg));
    res = COcompareConstants (c2, *PATTR_C1 (attr));
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "no match"));
    c2 = COfreeConstant (c2);

    return (res);
}

attrib *
PMAhasFS (constant **fs)
{
    attrib *res;

    res = makeAttrib (N_array, attribHasFS);
    PATTR_C1 (res) = fs;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAgetPrf( prf *fun)
 *
 * @brief attrib for PMprf extracts the prf
 *
 *****************************************************************************/
bool
attribGetPrf (attrib *attr, node *arg)
{
    bool res;

    DBUG_PRINT (PMASTART "PMgetPrf( " F_PTR " )", (void *)PATTR_PRFP (attr));
    *PATTR_PRFP (attr) = PRF_PRF (arg);
    res = TRUE;
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "no match"));

    return (res);
}

attrib *
PMAgetPrf (prf *fun)
{
    attrib *res;

    res = makeAttrib (N_prf, attribGetPrf);
    PATTR_PRFP (res) = fun;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAisPrf( prf fun)
 *
 * @brief attrib for PMprf checks prf against fun.
 *
 *****************************************************************************/
bool
attribIsPrf (attrib *attr, node *arg)
{
    bool res;

    DBUG_PRINT (PMASTART "PMisPrf( %s)", global.prf_name[PATTR_PRF (attr)]);
    res = PRF_PRF (arg) == PATTR_PRF (attr);
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "no match"));

    return (res);
}

attrib *
PMAisPrf (prf fun)
{
    attrib *res;

    res = makeAttrib (N_prf, attribIsPrf);
    PATTR_PRF (res) = fun;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAisNodePrf( prf fun)
 *
 * @brief attrib for PMprf checks prf against fun and negates.
 *
 *****************************************************************************/
bool
attribIsNotPrf (attrib *attr, node *arg)
{

    return (!attribIsPrf (attr, arg));
}

attrib *
PMAisNotPrf (prf fun)
{
    attrib *res;

    res = makeAttrib (N_prf, attribIsNotPrf);
    PATTR_PRF (res) = fun;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAhasCountWithop( int count)
 *
 * @brief
 *
 *****************************************************************************/
bool
attribHasCountWithop (attrib *attr, node *arg)
{
    bool res;

    /* 
     * FIXME May need to change PATTR_I1 and where it is used -
     * It is used for assignments, checks and printing. SHgetUnrLen
     * and TCcountWithOps(modified) are not size int while i_arg1 is int pointer.
     * Meanwhile, it gets used with NUM_VAL and attrib passing such as in PMAhasLen, 
     * so not a simple change.
     * 
     */
    res = (TCcountWithops (WITH_OR_WITH2_OR_WITH3_WITHOP (arg)) == *PATTR_I1 (attr));
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "no match"));

    return (res);
}

attrib *
PMAhasCountWithop (int *count)
{
    attrib *res;

    res = makeAttrib (N_module, attribHasCountWithop);
    PATTR_I1 (res) = count;

    return (res);
}

/** <!--*******************************************************************-->
 *
 * @fn attrib *PMAhasCountRange( int count)
 *
 * @brief
 *
 *****************************************************************************/
bool
attribHasCountRange (attrib *attr, node *arg)
{
    bool res;

    res = TCcountRanges (WITH3_RANGES (arg)) == *PATTR_I1 (attr);
    DBUG_PRINT (PMARESULT "%s", (res ? "match" : "no match"));

    return (res);
}

attrib *
PMAhasCountRange (int *count)
{
    attrib *res;

    res = makeAttrib (N_with3, attribHasCountRange);
    PATTR_I1 (res) = count;

    return (res);
}

#undef DBUG_PREFIX
