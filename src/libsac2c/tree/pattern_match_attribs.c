/*
 *
 * $Id$
 */

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

#include "dbug.h"
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

typedef bool attribFun (attrib *, node *);

struct PATTR {
    nodetype nt;
    prf fun;
    constant **c_arg1;
    node **n_arg1;
    int *i_arg1;
    int *i_arg2;
    attribFun *matcher;
};

#define PATTR_NT(p) (p->nt)
#define PATTR_PRF(p) (p->fun)
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
    if (PATTR_N1 (attr) != NULL) {
        *PATTR_N1 (attr) = arg;
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
 * @fn attrib *PMAisVar( node **var)
 *
 * @brief attrib for PMvar checks for equality
 *
 *****************************************************************************/
bool
attribIsVar (attrib *attr, node *arg)
{
    return (ID_AVIS (arg) == ID_AVIS (*PATTR_N1 (attr)));
}

attrib *
PMAisVar (node **var)
{
    attrib *res;

    res = makeAttrib (N_id, attribIsVar);
    PATTR_N1 (res) = var;

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

    c = PATTR_C1 (attr);
    if (*c != NULL) {
        *c = COfreeConstant (*c);
    }
    *c = COaST2Constant (arg);

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

    c = *PATTR_C1 (attr);
    c2 = COaST2Constant (arg);

    res = COcompareConstants (c, c2);
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
 * @fn attrib *PMAgetIVal( int *i)
 *
 * @brief attrib for PMint creates and hands back integer i
 *
 *****************************************************************************/
bool
attribGetIVal (attrib *attr, node *arg)
{
    *PATTR_I1 (attr) = NUM_VAL (arg);

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
    return (*PATTR_I1 (attr) == NUM_VAL (arg));
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
    return (*PATTR_I1 (attr) <= NUM_VAL (arg));
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
    *PATTR_I1 (attr) = SHgetUnrLen (ARRAY_FRAMESHAPE (arg));

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
    return (*PATTR_I1 (attr) == SHgetUnrLen (ARRAY_FRAMESHAPE (arg)));
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
    if (*PATTR_C1 (attr) != NULL) {
        *PATTR_C1 (attr) = COfreeConstant (*PATTR_C1 (attr));
    }
    *PATTR_C1 (attr) = COmakeConstantFromShape (ARRAY_FRAMESHAPE (arg));

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

    c2 = COmakeConstantFromShape (ARRAY_FRAMESHAPE (arg));
    res = COcompareConstants (c2, *PATTR_C1 (attr));
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
 * @fn attrib *PMAisPrf( prf fun)
 *
 * @brief attrib for PMprf checks prf against fun.
 *
 *****************************************************************************/
bool
attribIsPrf (attrib *attr, node *arg)
{
    return (PRF_PRF (arg) == PATTR_PRF (attr));
}

attrib *
PMAisPrf (prf fun)
{
    attrib *res;

    res = makeAttrib (N_prf, attribIsPrf);
    PATTR_PRF (res) = fun;

    return (res);
}
