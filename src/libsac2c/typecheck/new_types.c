/**
 *
 * @defgroup nty New Types
 * @ingroup ntc
 *
 * @brief The module "New Types" encapsulates the abstract data type "ntype".
 *
 *  It contains all functions needed for constructing and manipuloating these
 *  structures.
 *
 * @{
 */

/**
 *
 * @file new_types.c
 *
 * This file implements a new abstract data type for representing
 * types. This file describes the interface to that module.
 * Unfortunately, it is not yet used in all parts of the compiler 8-(.
 * In fact, the only part where it is used is the new typechecker.
 * In all other parts of the compiler, the structure "types" which in the
 * long term should be replaced by ntype, holds this information.
 * One of the reasons for this intended change of data structures is
 * that the types-structure often carries more than just type information
 * AND it is not flexible enough for the intended extensions of the type
 * system concerning union-types, type variables, constant types or higher-
 * order functions.
 * Therefore, this module also implements some conversion routines, e.g.
 *   TYOldType2Type, and TYType2OldType.
 *
 *
 * The central idea of the ntype is that all types are represented by
 * recursively nested type-constructors with varying arity.
 * All "scalar types", e.g., pre-defined types or user-defined types, are
 * represented by typeconstructors of arity 0, e.g. TC_simple or TC_user.
 * Further information concerning type constructors can be found in
 *     type_constructor_info.mac    .
 *
 *
 * For avoiding un-intended pointer sharing and for avoiding memory leaks
 * we establish the following rules:
 *
 * - whenever an ntype is given as argument, it will be inspected only!
 *   Neither the pointer to it nor any pointer to a sub structure will be
 *   returned or used within a data structure that serves as a result!
 *   There are EXACTLY THREE CLASSES OF FUNCTIONS that are an EXEPTION OF
 *   THIS RULE: - the MAKExyz - functions for generating ntype structures
 *              - the SETxyz - functions for inserting components into ntypes
 *              - the GETxyz - functions for extracting components of ntypes
 *
 * - The only functions for freeing an ntype constructor are
 *     TYfreeTypeConstructor  for freeing the topmost constructor only, and
 *     TYfreeType             for freeing the entire type.
 *
 * - If the result is an ntype structure, it has been freshly allocated!
 *
 */

#include <stdarg.h>

#include <limits.h>

#include "new_types.h"

#define DBUG_PREFIX "NTY"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "LookUpTable.h"

#include "ctinfo.h"
#include "DupTree.h"
#include "free.h"
#include "convert.h"
#include "constants.h"
#include "check_mem.h"

#include "user_types.h"
#include "type_utils.h"
#include "shape.h"
#include "ssi.h"

#include "serialize.h"
#include "deserialize.h"
#include "namespaces.h"
#include "globals.h"
#include "traverse.h"
#include "flattengenerators.h"

/*
 * Since all type constructors may have different attributes,
 * for each type constructor TC_xx that requires at least two attributes
 * a new type attr_xx is defined:
 */

typedef struct ATTR_AKD {
    shape *shp;
    size_t dots;
} attr_akd;

typedef struct ATTR_SYMBOL {
    namespace_t *mod;
    char *name;
} attr_symbol;

typedef struct ATTR_IRES {
    size_t num_funs;
    node **fundefs;
    int *poss;
} attr_ires;

typedef struct ATTR_POLYUSER {
    char *outer;
    char *inner;
    char *shape;
    bool denest : 1;
    bool renest : 1;
} attr_polyuser;

typedef struct DFT_STATE {
    size_t max_funs;
    size_t cnt_funs;
    node **fundefs;
    bool *legal;
    int *ups;
    int *downs;
} dft_state;

typedef struct SIMPLE {
    simpletype simple;
    usertype udt;
} simple;

/*
 * In order to have a uniform type for ALL type constructors, we define
 * a union type over all potential attributes:
 */

typedef union {
    simple a_simple;
    attr_symbol a_symbol;
    usertype a_user;
    shape *a_aks;
    attr_akd a_akd;
    constant *a_akv;
    struct NTYPE *a_ibase;
    size_t a_idim;
    shape *a_ishape;
    attr_ires a_ires;
    tvar *a_alpha;
    char *a_bottom;
    char *a_poly;
    attr_polyuser a_polyuser;
} typeattr;

/*
 * Finally, the new type structure "ntype" can be defined:
 *  it consists of - a type constructor tag
 *                 - the arity of the constructor
 *                 - its respective attribute
 *                 - a pointer to a list of (arity-many) son-constructors
 */

struct NTYPE {
    typeconstr mtypeconstr;
    size_t arity;
    typeattr mtypeattr;
    mutcScope mutcscope;
    mutcUsage mutcusage;
    bool unique;
    distmem_dis distributed;
    struct NTYPE **sons;
};

/*
 * For internal usage within this module only, we define the following
 * shape access macros:
 *
 * First, we define some basic ntype-access-macros:
 */

#define NTYPE_CON(n) (n->mtypeconstr)
#define NTYPE_ARITY(n) (n->arity)
#define NTYPE_SONS(n) (n->sons)
#define NTYPE_SON(n, i) (n->sons[i])
#define NTYPE_MUTC_SCOPE(n) (n->mutcscope)
#define NTYPE_MUTC_USAGE(n) (n->mutcusage)
#define NTYPE_UNIQUE(n) (n->unique)
#define NTYPE_DISTRIBUTED(n) (n->distributed)
/*
 * Macros for accessing the attributes...
 */
#define SIMPLE_TYPE(n) (n->mtypeattr.a_simple.simple)
#define SIMPLE_HIDDEN_UDT(n) (n->mtypeattr.a_simple.udt)
#define SYMBOL_NS(n) (n->mtypeattr.a_symbol.mod)
#define SYMBOL_NAME(n) (n->mtypeattr.a_symbol.name)
#define USER_TYPE(n) (n->mtypeattr.a_user)
#define AKV_CONST(n) (n->mtypeattr.a_akv)
#define AKS_SHP(n) (n->mtypeattr.a_aks)
#define AKD_SHP(n) (n->mtypeattr.a_akd.shp)
#define AKD_DOTS(n) (n->mtypeattr.a_akd.dots)

#define IBASE_BASE(n) (n->mtypeattr.a_ibase)
#define IDIM_DIM(n) (n->mtypeattr.a_idim)
#define ISHAPE_SHAPE(n) (n->mtypeattr.a_ishape)
#define IRES_NUMFUNS(n) (n->mtypeattr.a_ires.num_funs)
#define IRES_FUNDEFS(n) (n->mtypeattr.a_ires.fundefs)
#define IRES_FUNDEF(n, i) (n->mtypeattr.a_ires.fundefs[i])
#define IRES_POSS(n) (n->mtypeattr.a_ires.poss)
#define IRES_POS(n, i) (n->mtypeattr.a_ires.poss[i])

#define ALPHA_SSI(n) (n->mtypeattr.a_alpha)

#define BOTTOM_MSG(n) (n->mtypeattr.a_bottom)

#define POLY_NAME(n) (n->mtypeattr.a_poly)

#define POLYUSER_OUTER(n) (n->mtypeattr.a_polyuser.outer)
#define POLYUSER_INNER(n) (n->mtypeattr.a_polyuser.inner)
#define POLYUSER_SHAPE(n) (n->mtypeattr.a_polyuser.shape)
#define POLYUSER_DENEST(n) (n->mtypeattr.a_polyuser.denest)
#define POLYUSER_RENEST(n) (n->mtypeattr.a_polyuser.renest)

/*
 * Macros for accessing the sons...
 */
#define AKV_BASE(n) (n->sons[0])
#define AKS_BASE(n) (n->sons[0])
#define AKD_BASE(n) (n->sons[0])
#define AUDGZ_BASE(n) (n->sons[0])
#define AUD_BASE(n) (n->sons[0])
#define UNION_MEMBER(n, i) (n->sons[i])
#define PROD_MEMBER(n, i) (n->sons[i])

#define FUN_POLY(n) (n->sons[0])
#define FUN_UPOLY(n) (n->sons[1])
#define FUN_IBASE(n, i) (n->sons[i + 2])

#define IBASE_GEN(n) (n->sons[0])
#define IBASE_SCAL(n) (n->sons[1])
#define IBASE_IARR(n) (n->sons[2])

#define IARR_GEN(n) (n->sons[0])
#define IARR_IDIM(n, i) (n->sons[i + 1])

#define IDIM_GEN(n) (n->sons[0])
#define IDIM_ISHAPE(n, i) (n->sons[i + 1])

#define ISHAPE_GEN(n) (n->sons[0])

#define IRES_TYPE(n) (n->sons[0])

/*
 * For dbug-output purposes we keep an array of strings for the individual
 * type constructors and an array of flags indicating whether the number of
 * sons is fixed or not:
 */

static char *dbug_str[] = {
#define TCIDbugString(a) a
#include "type_constructor_info.mac"
};

static int variable_arity[] = {
#define TCIVariableArity(a) a
#include "type_constructor_info.mac"
};

/***
 *** local helper functions:
 ***/

/******************************************************************************
 *
 * function:
 *    ntype *MakeNtype( typeconstr con, int arity)
 *
 * description:
 *    internal function for allocating an ntype-node. According to arity,
 *    an array of pointers to subtypes is allocated as well (iff arity>0).
 *
 ******************************************************************************/

static ntype *
MakeNtype (typeconstr con, size_t arity)
{
    ntype *res;
    size_t i;

    DBUG_ENTER ();

    res = (ntype *)MEMmalloc (sizeof (ntype));
    NTYPE_CON (res) = con;
    NTYPE_ARITY (res) = arity;
    if (NTYPE_ARITY (res) > 0) {
        NTYPE_SONS (res) = (ntype **)MEMmalloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < arity; i++) {
            NTYPE_SON (res, i) = NULL;
        }
    } else {
        NTYPE_SONS (res) = NULL;
    }

    NTYPE_MUTC_SCOPE (res) = MUTC_GLOBAL;
    NTYPE_MUTC_USAGE (res) = MUTC_US_DEFAULT;
    NTYPE_UNIQUE (res) = FALSE;
    NTYPE_DISTRIBUTED (res) = distmem_dis_ndi;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   ntype *MakeNewSon( ntype *father, ntype *son)
 *
 * description:
 *   Internal function for increasing an ntype's arity.
 *   Like all Makexxx functions it consumes (reuses) both its arguments!!
 *
 ******************************************************************************/

ntype *
MakeNewSon (ntype *father, ntype *son)
{
    ntype **new_sons;
    size_t i, arity;

    DBUG_ENTER ();

    arity = NTYPE_ARITY (father);
    NTYPE_ARITY (father) = arity + 1;
    new_sons = (ntype **)MEMmalloc (sizeof (ntype *) * NTYPE_ARITY (father));
    for (i = 0; i < arity; i++) {
        new_sons[i] = NTYPE_SON (father, i);
    }
    new_sons[i] = son;
    NTYPE_SONS (father) = MEMfree (NTYPE_SONS (father));
    NTYPE_SONS (father) = new_sons;

    DBUG_RETURN (father);
}

/******************************************************************************
 *
 * function:
 *   ntype *DeleteSon( ntype *father, int son)
 *
 * description:
 *   Internal function for decreasing an ntype's arity.
 *   Like all Makexxx functions it consumes its argument!!
 *
 ******************************************************************************/

ntype *
DeleteSon (ntype *father, size_t son)
{
    ntype **new_sons;
    size_t i, arity;

    DBUG_ENTER ();

    arity = NTYPE_ARITY (father) - 1;
    NTYPE_ARITY (father) = arity;
    new_sons = (ntype **)MEMmalloc (sizeof (ntype *) * arity);
    for (i = 0; i < son; i++) {
        new_sons[i] = NTYPE_SON (father, i);
    }
    for (; i < arity; i++) {
        new_sons[i] = NTYPE_SON (father, i + 1);
    }
    NTYPE_SONS (father) = MEMfree (NTYPE_SONS (father));
    NTYPE_SONS (father) = new_sons;

    DBUG_RETURN (father);
}

/******************************************************************************
 *
 * function:
 *   ntype *IncreaseArity( ntype *type, size_t amount)
 *
 * description:
 *   Internal function for increasing an ntype's arity by amount new sons.
 *   Like all Makexxx functions it consumes its argument!!
 *
 ******************************************************************************/

ntype *
IncreaseArity (ntype *type, size_t amount)
{
    ntype **new_sons;
    size_t i, arity;

    DBUG_ENTER ();

    arity = NTYPE_ARITY (type);
    NTYPE_ARITY (type) = arity + amount;
    new_sons = (ntype **)MEMmalloc (sizeof (ntype *) * NTYPE_ARITY (type));
    for (i = 0; i < arity; i++) {
        new_sons[i] = NTYPE_SON (type, i);
    }
    for (i = arity; i < NTYPE_ARITY (type); i++) {
        new_sons[i] = NULL;
    }
    NTYPE_SONS (type) = MEMfree (NTYPE_SONS (type));
    NTYPE_SONS (type) = new_sons;

    DBUG_RETURN (type);
}

/******************************************************************************
 *
 * function:
 *   ntype *MakeNewFundefsPoss( ntype *ires, int num,
 *                               node **fundefs, int *poss)
 *
 * description:
 *   Internal function for adding <num> fundefs and <num> poss to an ires's
 *   fundefs and poss list.
 *   Like all Makexxx functions it consumes (reuses) both its arguments!!
 *
 ******************************************************************************/

ntype *
MakeNewFundefsPoss (ntype *ires, size_t num, node **fundefs, int *poss)
{
    node **new_fundefs;
    int *new_poss;
    size_t i, arity;

    DBUG_ENTER ();

    arity = IRES_NUMFUNS (ires);
    IRES_NUMFUNS (ires) = arity + num;
    new_fundefs = (node **)MEMmalloc (sizeof (node *) * IRES_NUMFUNS (ires));
    new_poss = (int *)MEMmalloc (sizeof (int) * IRES_NUMFUNS (ires));
    for (i = 0; i < arity; i++) {
        new_fundefs[i] = IRES_FUNDEF (ires, i);
        new_poss[i] = IRES_POS (ires, i);
    }
    for (; i < IRES_NUMFUNS (ires); i++) {
        new_fundefs[i] = fundefs[i - arity];
        new_poss[i] = poss[i - arity];
    }
    IRES_FUNDEFS (ires) = MEMfree (IRES_FUNDEFS (ires));
    IRES_POSS (ires) = MEMfree (IRES_POSS (ires));
    fundefs = MEMfree (fundefs);
    poss = MEMfree (poss);
    IRES_FUNDEFS (ires) = new_fundefs;
    IRES_POSS (ires) = new_poss;

    DBUG_RETURN (ires);
}

/***
 *** Functions for creating and inspecting ntypes (MakeXYZ, GetXYZ):
 ***   these functions are the sole functions that use arg-pointers as
 ***   part of their result!
 ***/

/******************************************************************************
 *
 * function:
 *    typeconstr TYgetConstr( ntype *type)
 *
 * description:
 *
 ******************************************************************************/

typeconstr
TYgetConstr (ntype *type)
{
    DBUG_ENTER ();

    DBUG_RETURN (NTYPE_CON (type));
}

/******************************************************************************
 *
 * function:
 *    mutcScope TYgetMutcScope( ntype *type)
 *
 * description:
 *
 ******************************************************************************/
mutcScope
TYgetMutcScope (ntype *type)
{
    DBUG_ENTER ();

    DBUG_RETURN (NTYPE_MUTC_SCOPE (type));
}

/******************************************************************************
 *
 * function:
 *    mutcUsage TYgetMutcUsage( ntype *type)
 *
 * description:
 *
 ******************************************************************************/
mutcUsage
TYgetMutcUsage (ntype *type)
{
    DBUG_ENTER ();

    DBUG_RETURN (NTYPE_MUTC_USAGE (type));
}

/******************************************************************************
 *
 * function:
 *    bool TYisUnique( ntype *type)
 *
 * description:
 *
 ******************************************************************************/
bool
TYisUnique (ntype *type)
{
    DBUG_ENTER ();

    DBUG_RETURN (NTYPE_UNIQUE (type));
}

/******************************************************************************
 *
 * function:
 *    distmem_dis TYDistributed( ntype *type)
 *
 * description:
 *
 ******************************************************************************/
distmem_dis
TYgetDistributed (ntype *type)
{
    DBUG_ENTER ();

    DBUG_RETURN (NTYPE_DISTRIBUTED (type));
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakeSimpleType( simpletype base)
 *   ntype * TYmakeHiddenSimpleType( usertype udt)
 *   ntype * TYmakeSymbType( char *name, namespace_t *ns)
 *   ntype * TYmakeUserType( usertype udt)
 *
 *   ntype * TYsetSimpleType( ntype *simple, simpletype base)
 *
 * description:
 *   Several functions for creating scalar types
 *   (type-constructors with arity 0).
 *
 ******************************************************************************/

ntype *
TYmakeSimpleType (simpletype base)
{
    ntype *res;

    DBUG_ENTER ();

    DBUG_ASSERT (base != T_hidden, "TYmakeSimpleType called with T_hidden arg!"
                                   "Please use TYmakeHiddenSimpleType instead!");

    res = MakeNtype (TC_simple, 0);
    SIMPLE_TYPE (res) = base;
    SIMPLE_HIDDEN_UDT (res) = UT_NOT_DEFINED;

    DBUG_RETURN (res);
}

ntype *
TYmakeHiddenSimpleType (usertype udt)
{
    ntype *res;

    DBUG_ENTER ();

    res = MakeNtype (TC_simple, 0);
    SIMPLE_TYPE (res) = T_hidden;
    SIMPLE_HIDDEN_UDT (res) = udt;

    DBUG_RETURN (res);
}

ntype *
TYmakeSymbType (char *name, namespace_t *ns)
{
    ntype *res;

    DBUG_ENTER ();

    DBUG_ASSERT (name != NULL, "TYmakeSymbType called with NULL name!");

    res = MakeNtype (TC_symbol, 0);
    SYMBOL_NS (res) = ns;
    SYMBOL_NAME (res) = name;

    DBUG_RETURN (res);
}

ntype *
TYmakeUserType (usertype udt)
{
    ntype *res;

    DBUG_ENTER ();

    DBUG_PRINT_TAG ("TYmakeUserType", "MAKING USER TYPE: %s", UTgetName (udt));

    res = MakeNtype (TC_user, 0);
    USER_TYPE (res) = udt;

    DBUG_RETURN (res);
}

ntype *
TYsetSimpleType (ntype *simple, simpletype base)
{
    DBUG_ENTER ();

    SIMPLE_TYPE (simple) = base;

    DBUG_RETURN (simple);
}

ntype *
TYsetHiddenUserType (ntype *simple, usertype udt)
{
    DBUG_ENTER ();

    SIMPLE_HIDDEN_UDT (simple) = udt;

    DBUG_RETURN (simple);
}

ntype *
TYsetMutcUsage (ntype *type, mutcUsage usage)
{
    DBUG_ENTER ();

    NTYPE_MUTC_USAGE (type) = usage;

    DBUG_RETURN (type);
}

ntype *
TYsetUnique (ntype *type, bool val)
{
    DBUG_ENTER ();

    NTYPE_UNIQUE (type) = val;

    DBUG_RETURN (type);
}

ntype *
TYsetDistributed (ntype *type, distmem_dis val)
{
    DBUG_ENTER ();

    NTYPE_DISTRIBUTED (type) = val;

    DBUG_RETURN (type);
}

ntype *
TYsetMutcScope (ntype *type, mutcScope scope)
{
    DBUG_ENTER ();

    NTYPE_MUTC_SCOPE (type) = scope;

    DBUG_RETURN (type);
}

/******************************************************************************
 *
 * function:
 *   simpletype    TYgetSimpleType( ntype *simple)
 *   usertype      TYgetHiddenUserType( ntype *simple)
 *   usertype      TYgetUserType( ntype *user)
 *   char        * TYgetName( ntype *symb)
 *   namespace_t * TYgetNamespace( ntype *symb)
 *
 * description:
 *   Several functions for extracting the attributes from scalar types.
 *
 ******************************************************************************/

simpletype
TYgetSimpleType (ntype *simple)
{
    DBUG_ENTER ();
    if (NTYPE_CON (simple) != TC_simple) {
        printf ("NTYPE_CON() returns %i instead of %i (TC_simple)\n", NTYPE_CON (simple),
                TC_simple);
    }
    DBUG_ASSERT (NTYPE_CON (simple) == TC_simple,
                 "TYgetSimpleType applied to nonsimple-type!");
    DBUG_RETURN (SIMPLE_TYPE (simple));
}

usertype
TYgetHiddenUserType (ntype *simple)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NTYPE_CON (simple) == TC_simple,
                 "TYgetHiddenUserType applied to nonsimple-type!");
    DBUG_ASSERT (SIMPLE_TYPE (simple) == T_hidden,
                 "TYgetHiddenUserType applied to non hidden type!");
    DBUG_RETURN (SIMPLE_HIDDEN_UDT (simple));
}

usertype
TYgetUserType (ntype *user)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NTYPE_CON (user) == TC_user, "TYgetUserType applied to non-user-type!");
    DBUG_RETURN (USER_TYPE (user));
}

char *
TYgetName (ntype *symb)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NTYPE_CON (symb) == TC_symbol, "TYgetName applied to nonsymbol-type!");
    DBUG_RETURN (SYMBOL_NAME (symb));
}

const namespace_t *
TYgetNamespace (ntype *symb)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NTYPE_CON (symb) == TC_symbol,
                 "TYgetNamespace applied to nonsymbol-type!");
    DBUG_RETURN (SYMBOL_NS (symb));
}

/******************************************************************************
 *
 * function:
 *   ntype     * TYsetNamespace( ntype *symb, namespace_t *ns)
 *
 * description:
 *   Several functions for changing the attributes from scalar types.
 *
 ******************************************************************************/
ntype *
TYsetNamespace (ntype *symb, namespace_t *ns)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NTYPE_CON (symb) == TC_symbol,
                 "TYsetNamespace applied to nonsymbol-type!");

    if (SYMBOL_NS (symb) != NULL) {
        SYMBOL_NS (symb) = NSfreeNamespace (SYMBOL_NS (symb));
    }

    SYMBOL_NS (symb) = ns;

    DBUG_RETURN (symb);
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakeAKV( ntype *scalar, constant *val)
 *   ntype * TYmakeAKS( ntype *scalar, shape *shp)
 *   ntype * TYmakeAKD( ntype *scalar, int dots, shape *shp)
 *   ntype * TYmakeAUD( ntype *scalar)
 *   ntype * TYmakeAUDGZ( ntype *scalar)
 *
 *   ntype * TYsetScalar( ntype *array, ntype *scalar)
 *
 * description:
 *   Several functions for creating array-types.
 *
 ******************************************************************************/

ntype *
TYmakeAKV (ntype *scalar, constant *val)
{
    ntype *res;

    DBUG_ENTER ();

    res = MakeNtype (TC_akv, 1);
    AKV_CONST (res) = val;
    AKV_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYmakeAKS (ntype *scalar, shape *shp)
{
    ntype *res;

    DBUG_ENTER ();

    res = MakeNtype (TC_aks, 1);
    AKS_SHP (res) = shp;
    AKS_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYmakeAKD (ntype *scalar, size_t dots, shape *shp)
{
    ntype *res;

    DBUG_ENTER ();

    DBUG_ASSERT (dots != 0, "attempting to create AKD scalar; "
                            "should create AKS instead!");
    res = MakeNtype (TC_akd, 1);
    AKD_DOTS (res) = dots;
    AKD_SHP (res) = shp;
    AKD_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYmakeAUDGZ (ntype *scalar)
{
    ntype *res;

    DBUG_ENTER ();

    res = MakeNtype (TC_audgz, 1);
    AUDGZ_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYmakeAUD (ntype *scalar)
{
    ntype *res;

    DBUG_ENTER ();

    res = MakeNtype (TC_aud, 1);
    AUD_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYsetScalar (ntype *array, ntype *scalar)
{
    DBUG_ENTER ();

    NTYPE_SON (array, 0) = TYfreeType (NTYPE_SON (array, 0));
    NTYPE_SON (array, 0) = scalar;

    DBUG_RETURN (array);
}

/******************************************************************************
 *
 * function:
 *   int        TYgetDim( ntype *array)
 *   shape *    TYgetShape( ntype *array)
 *   constant * TYgetValue( ntype *array)
 *   ntype *    TYgetScalar( ntype *array)
 *
 * description:
 *   Several functions for extracting the attributes / sons of array types.
 *
 ******************************************************************************/

int
TYgetDim (ntype *array)
{
    shape *shp;
    int res;

    DBUG_ENTER ();
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akv)
                   || (NTYPE_CON (array) == TC_akd),
                 "TYgetDim applied to other than AKV, AKS or AKD type!");
    if (NTYPE_CON (array) == TC_akv) {
        res = COgetDim (AKV_CONST (array));
    } else if (NTYPE_CON (array) == TC_aks) {
        res = SHgetDim (AKS_SHP (array));
    } else {
        shp = AKD_SHP (array);
        if (shp != NULL) {
            res = SHgetDim (shp) + AKD_DOTS (array);
        } else {
            res = AKD_DOTS (array);
        }
    }

    DBUG_RETURN (res);
}

shape *
TYgetShape (ntype *array)
{
    shape *res;

    DBUG_ENTER ();
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akv)
                   || (NTYPE_CON (array) == TC_akd),
                 "TYgetShape applied to other than AKV, AKS or AKD type!");
    if (NTYPE_CON (array) == TC_akv) {
        res = COgetShape (AKV_CONST (array));
    } else if (NTYPE_CON (array) == TC_aks) {
        res = AKS_SHP (array);
    } else {
        res = AKD_SHP (array);
    }

    DBUG_RETURN (res);
}

constant *
TYgetValue (ntype *array)
{
    constant *res;

    DBUG_ENTER ();
    DBUG_ASSERT (NTYPE_CON (array) == TC_akv,
                 "TYgetValue applied to other than AKV type!");
    res = AKV_CONST (array);

    DBUG_RETURN (res);
}

ntype *
TYgetScalar (ntype *array)
{
    DBUG_ENTER ();
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akv)
                   || (NTYPE_CON (array) == TC_akd) || (NTYPE_CON (array) == TC_audgz)
                   || (NTYPE_CON (array) == TC_aud),
                 "TYgetScalar applied to other than array type!");
    DBUG_RETURN (NTYPE_SON (array, 0));
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakeUnionType( ntype *t1, ntype *t2)
 *
 * description:
 *   functions for creating union-types. Note here, that this function, like
 *   all MakeXYZ and GetXYZ functions consumes its arguments!!
 *   Since the representation of the union type constructor does not allow the
 *   two argument constructors to be re-used iff the are already
 *   union-constructors, those constructors are freed!!
 *
 ******************************************************************************/

ntype *
TYmakeUnionType (ntype *t1, ntype *t2)
{
    ntype *res;
    int pos = 0;
    size_t i, arity = 2;

    DBUG_ENTER ();

    if (NTYPE_CON (t1) == TC_union) {
        arity += NTYPE_ARITY (t1) - 1;
    }
    if (NTYPE_CON (t2) == TC_union) {
        arity += NTYPE_ARITY (t2) - 1;
    }
    res = MakeNtype (TC_union, arity);

    if (NTYPE_CON (t1) == TC_union) {
        for (i = 0; i < NTYPE_ARITY (t1); i++) {
            UNION_MEMBER (res, pos++) = UNION_MEMBER (t1, i);
        }
        t1 = TYfreeTypeConstructor (t1);
    } else {
        UNION_MEMBER (res, pos++) = t1;
    }
    if (NTYPE_CON (t2) == TC_union) {
        for (i = 0; i < NTYPE_ARITY (t2); i++) {
            UNION_MEMBER (res, pos++) = UNION_MEMBER (t2, i);
        }
        t2 = TYfreeTypeConstructor (t2);
    } else {
        UNION_MEMBER (res, pos++) = t2;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakeProductType( int size, ...)
 *
 *   ntype * TYmakeEmptyProductType( int size)
 *   ntype * TYsetProductMember( ntype *prod, int pos, ntype *member)
 *
 * description:
 *   Functions for creating product types. Note here, that this function, like
 *   all MakeXYZ and GetXYZ functions consumes its arguments!!
 *   At the time being, only array types, bottom types, or type variables
 *   may be given as arguments.
 *   The first version is useful in all situations where the number of
 *   components is statically known. However, in many situations this is not
 *   the case. In these situations, the latter two functions are to be used.
 *
 ******************************************************************************/

ntype *
TYmakeProductType (size_t size, ...)
{
    va_list Argp;
    size_t i;
    ntype *res, *arg;

    DBUG_ENTER ();

    res = MakeNtype (TC_prod, size);

    if (size > 0) {
        va_start (Argp, size);
        for (i = 0; i < size; i++) {
            arg = va_arg (Argp, ntype *);
            /*
             * We also want non AKS user types here!
             */
            DBUG_ASSERT ((TYisArray (arg) || TYisBottom (arg) || TYisAlpha (arg)
                          || (TYisUser (arg) && !TYisAKS (arg))),
                         "non array type / bottom / type var components of product types"
                         " are not yet supported!");
            PROD_MEMBER (res, i) = arg;
        }
    }

    DBUG_RETURN (res);
}

ntype *
TYmakeEmptyProductType (size_t size)
{
    ntype *res;

    DBUG_ENTER ();
    
    res = MakeNtype (TC_prod, size);

    DBUG_RETURN (res);
}

ntype *
TYsetProductMember (ntype *prod, size_t pos, ntype *member)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (prod) == TC_prod,
                 "TYsetProductMember applied to non-product type");
    DBUG_ASSERT (pos < NTYPE_ARITY (prod),
                 "TYsetProductMember applied to product type with"
                 " too few elements");

    NTYPE_SON (prod, pos) = member;

    DBUG_RETURN (prod);
}

/******************************************************************************
 *
 * function:
 *   int       TYgetProductSize( ntype *prod)
 *   ntype  *  TYgetProductMember( ntype *prod, int pos)
 *
 * description:
 *   functions inspecting or extracting components of product types!
 *   Note here, that TYgetProductMember does not copy the member to be
 *   extracted!!
 *
 ******************************************************************************/

size_t
TYgetProductSize (ntype *prod)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (prod) == TC_prod,
                 "TYgetProductSize applied to non-product type");

    DBUG_RETURN (NTYPE_ARITY (prod));
}

ntype *
TYgetProductMember (ntype *prod, size_t pos)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (prod) == TC_prod,
                 "TYgetProductMember applied to non-product type");
    DBUG_ASSERT (NTYPE_ARITY (prod) > pos,
                 "TYgetProductMember applied with illegal index");

    DBUG_RETURN (PROD_MEMBER (prod, pos));
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakePolyType( char *name)
 *
 * description:
 *   Function for creating poly types.
 *
 ******************************************************************************/

ntype *
TYmakePolyType (char *name)
{
    ntype *res;

    DBUG_ENTER ();

    res = MakeNtype (TC_poly, 0);
    POLY_NAME (res) = name;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   char * TYgetPolyName( ntype *type)
 *
 * description:
 *  function for extracting the name of a polymorphic type.
 *
 ******************************************************************************/

char *
TYgetPolyName (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (type) == TC_poly, "TYgetPolyName applied to non poly type!");

    DBUG_RETURN (POLY_NAME (type));
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakePolyUserType( char *outer, char *inner, char *shape,
 *                               bool denest, bool renest)
 *
 * description:
 *   Function for creating poly user types.
 *
 ******************************************************************************/

ntype *
TYmakePolyUserType (char *outer, char *inner, char *shape, bool denest, bool renest)
{
    ntype *res;

    DBUG_ENTER ();

    DBUG_ASSERT ((!denest || !renest), "polyuser with de- and renest is illegal!");

    res = MakeNtype (TC_polyuser, 0);
    POLYUSER_OUTER (res) = outer;
    POLYUSER_INNER (res) = inner;
    POLYUSER_SHAPE (res) = shape;
    POLYUSER_DENEST (res) = denest;
    POLYUSER_RENEST (res) = renest;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   char * TYgetPolyUserOuter( ntype *type)
 *
 * description:
 *  function for extracting the outer name of a polymorphic user type.
 *
 ******************************************************************************/

char *
TYgetPolyUserOuter (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (type) == TC_polyuser,
                 "TYgetPolyUserOuter applied to non polyuser type!");

    DBUG_RETURN (POLYUSER_OUTER (type));
}

/******************************************************************************
 *
 * function:
 *   char * TYgetPolyUserInner( ntype *type)
 *
 * description:
 *  function for extracting the inner (nested) name of a polymorphic user type.
 *
 ******************************************************************************/

char *
TYgetPolyUserInner (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (type) == TC_polyuser,
                 "TYgetPolyUserInner applied to non polyuser type!");

    DBUG_RETURN (POLYUSER_INNER (type));
}

/******************************************************************************
 *
 * function:
 *   char * TYgetPolyUserShape( ntype *type)
 *
 * description:
 *  function for extracting the name of the shape of a polymorphic user type.
 *
 ******************************************************************************/

char *
TYgetPolyUserShape (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (type) == TC_polyuser,
                 "TYgetPolyUserShape applied to non polyuser type!");

    DBUG_RETURN (POLYUSER_SHAPE (type));
}

/******************************************************************************
 *
 * function:
 *   bool TYgetPolyUserDeNest( ntype *type)
 *
 * description:
 *  function for checking whether a polymorphic user type includes an implicit
 *  denesting.
 *
 ******************************************************************************/

bool
TYgetPolyUserDeNest (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (type) == TC_polyuser,
                 "TYgetPolyUserDeNest applied to non polyuser type!");

    DBUG_RETURN (POLYUSER_DENEST (type));
}

/******************************************************************************
 *
 * function:
 *   bool TYgetPolyUserReNest( ntype *type)
 *
 * description:
 *  function for checking whether a polymorphic user type includes an implicit
 *  renesting.
 *
 ******************************************************************************/

bool
TYgetPolyUserReNest (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (type) == TC_polyuser,
                 "TYgetPolyUserReNest applied to non polyuser type!");

    DBUG_RETURN (POLYUSER_RENEST (type));
}
/** <!--********************************************************************-->
 *
 * @fn ntype *TYmakeBottomType( char *err_msg)
 *
 *   @brief creates bottom type
 *   @param err_msg error that generated this type
 *   @return freshly created bottom type
 *
 ******************************************************************************/

ntype *
TYmakeBottomType (char *err_msg)
{
    ntype *res;

    DBUG_ENTER ();

    res = MakeNtype (TC_bottom, 0);
    BOTTOM_MSG (res) = err_msg;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn void TYextendBottomError( ntype *type, char *err_msg)
 *
 *   @brief replaces the err_msg contained in the bottom type by a concatinantion
 *          of the existing message and the new one.
 *   @param err_msg error that generated this type
 *
 ******************************************************************************/

void
TYextendBottomError (ntype *type, char *err_msg)
{
    char *new_msg;

    DBUG_ENTER ();

    new_msg = STRcatn (3, BOTTOM_MSG (type), "@", err_msg);
    BOTTOM_MSG (type) = MEMfree (BOTTOM_MSG (type));
    BOTTOM_MSG (type) = new_msg;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn char *TYgetBottomError( ntype *type)
 *
 *   @brief extracts the error messgae from the bottom type
 *   @param type bottom type given
 *   @return erroro message contained
 *
 ******************************************************************************/

char *
TYgetBottomError (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (type) == TC_bottom,
                 "TYgetBottomError applied to non bottom type!");

    DBUG_RETURN (BOTTOM_MSG (type));
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakeFunType( ntype *arg, ntype *res, node *fun_info)
 *
 * description:
 *  function for creating function types. It implicitly creates the intersection
 *  type that contains all up-projections!
 *  For efficiency reasons, function types are represented by a kind of decision
 *  tree made up of type constructors. Its structure is similar to the hierarchy
 *  of array types but adjusted to the needs of dispatching overloaded functions
 *  by means of a given argument type.
 *  In general, we have the following structure :
 *
 *         TC_fun
 *       /   |    \  ...
 *      /<a> |<a=> \
 *   {trees here!}  TC_ibase -- scalar type (e.g. INT)
 *                 /    |    \
 *                /[]   |[*]  \
 *                             TC_iarr
 *                             /     \  ...
 *                            /[+]    \
 *                                     TC_idim -- dimensionality (e.g. 2)
 *                                     /     \  ...
 *                                    /[.,.]  \
 *                                             TC_ishape -- shape (e.g. [3,4])
 *                                             /
 *                                            /[3,4]
 *
 *  All "open ends" of this structure point to TC_ires nodes which hold the
 *  return types of the given function. The dots right of some edges indicate
 *  that there may be multiple of the lower nodes attached, i.e., there may be
 *  several TC_ibase nodes under a single TC_fun node, and there may also be
 *  several TC_idim and several TC_ishape nodes.
 *  However, since this function only creates the type for a single
 *  non-overloaded function, it does only create single ones. In case the
 *  parameter type of the function is a generic one, e.g. int[.,.], only that
 *  part of the tree is constructed that is necessary to accomodate it. In
 *  case of int[.,.], simply the lowest TC_ishape node would be missing.
 *
 ******************************************************************************/

ntype *
TYmakeFunType (ntype *arg, ntype *res_type, node *fundef)
{
    ntype *fun = NULL;
    ntype *base = NULL;
    ntype *arr = NULL;
    ntype *dim = NULL;
    ntype *shape = NULL;
    ntype *res = NULL;
    ntype *aks = NULL;

#ifndef DBUG_OFF
    char *tmp = NULL;
#endif

    DBUG_ENTER ();
    DBUG_PRINT_TAG ("NTY_MEM", "Allocated mem on entering TYmakeFunType: %zu",
                    global.current_allocated_mem);
    DBUG_PRINT_TAG ("NTY", "fun: %s", CTIitemName (fundef));
    DBUG_PRINT_TAG ("NTY", "rets: %zu", TCcountRets (FUNDEF_RETS (fundef)));

    node *r = FUNDEF_RETS (fundef);
    int i = 0;
    while (r != NULL) {
        DBUG_PRINT_TAG ("NTY", "  arg %d: %s", i,
                        TYtype2DebugString (RET_TYPE (r), FALSE, 0));
        r = RET_NEXT (r);
        i++;
    }

    res = MakeNtype (TC_ires, 1);

    IRES_TYPE (res) = res_type;

    IRES_NUMFUNS (res) = 1;
    IRES_FUNDEFS (res) = (node **)MEMmalloc (sizeof (node *));
    IRES_FUNDEF (res, 0) = fundef;
    IRES_POSS (res) = (int *)MEMmalloc (sizeof (int));
    IRES_POS (res, 0) = 0;

    base = MakeNtype (TC_ibase, 3);

    switch (NTYPE_CON (arg)) {
    case TC_akv:
        DBUG_PRINT_TAG ("NTY", "AKV");
        aks = TYeliminateAKV (arg);
        arg = TYfreeType (arg);
        arg = aks;
        aks = NULL;
        /**
         * there is no break here, as we wish to re-use the AKS implementation.
         * At a later stage, this has to be replaced by proper code suporting
         * value specialized function instances.
         */
        /* Falls through. */

    case TC_aks:
        DBUG_PRINT_TAG ("NTY", "AKS");
        if (TYgetDim (arg) == 0) {
            IBASE_SCAL (base) = TYcopyType (res); /* scalar: definition case */
            /* finally, we make res an up-projection as res will be used in AUD! */
            IRES_POS (res, 0) = 1;
        } else {
            shape = MakeNtype (TC_ishape, 1);
            ISHAPE_SHAPE (shape) = SHcopyShape (AKS_SHP (arg));
            ISHAPE_GEN (shape) = TYcopyType (res); /* array AKS: definition case */

            dim = MakeNtype (TC_idim, 2);
            IDIM_DIM (dim) = TYgetDim (arg);
            IDIM_ISHAPE (dim, 0) = shape;
            IDIM_GEN (dim) = TYcopyType (res);
            IRES_POS (IDIM_GEN (dim), 0) = 1; /* projecting AKS to AKD */

            arr = MakeNtype (TC_iarr, 2);
            IARR_IDIM (arr, 0) = dim;
            IARR_GEN (arr) = TYcopyType (res);
            IRES_POS (IARR_GEN (arr), 0) = 2; /* projecting AKS to AUDGZ */
            /* finally, we make res an up-projection as res will be used in AUD! */
            IRES_POS (res, 0) = 3;
        }
        break;

    case TC_akd:
        DBUG_PRINT_TAG ("NTY", "AKD");
        if (TYgetDim (arg) == 0) {
            IBASE_SCAL (base) = TYcopyType (res); /* scalar: definition case */
        } else {
            dim = MakeNtype (TC_idim, 1);
            IDIM_DIM (dim) = TYgetDim (arg);
            IDIM_GEN (dim) = TYcopyType (res); /* array AKD: definition case */

            arr = MakeNtype (TC_iarr, 2);
            IARR_IDIM (arr, 0) = dim;
            IARR_GEN (arr) = TYcopyType (res);
            IRES_POS (IARR_GEN (arr), 0) = 1; /* projecting AKD to AUDGZ */
        }
        /* finally, we make res an up-projection as res will be used in AUD! */
        IRES_POS (res, 0) = 2;
        break;

    case TC_audgz:
        DBUG_PRINT_TAG ("NTY", "AUDGZ");
        arr = MakeNtype (TC_iarr, 1);
        IARR_GEN (arr) = TYcopyType (res); /* AUDGZ definition case */

        /* finally, we make res an up-projection as res will be used in AUD! */
        IRES_POS (res, 0) = 1;
        break;

    case TC_aud:
        DBUG_PRINT_TAG ("NTY", "AUD");
        break;

    default:
        DBUG_UNREACHABLE ("argument type not yet supported");
    }

    IBASE_GEN (base) = res;
    IBASE_BASE (base) = TYgetScalar (arg);
    IBASE_IARR (base) = arr;

    if (TYisPoly (IBASE_BASE (base))) {
        DBUG_PRINT_TAG ("NTY", "fun type poly");
        fun = MakeNtype (TC_fun, 2);
        FUN_POLY (fun) = base;
    } else if (TYisPolyUser (IBASE_BASE (base))) {
        DBUG_PRINT_TAG ("NTY", "fun type poly user");
        fun = MakeNtype (TC_fun, 2);
        FUN_UPOLY (fun) = base;
    } else {
        DBUG_PRINT_TAG ("NTY", "fun type not poly");
        fun = MakeNtype (TC_fun, 3);
        FUN_IBASE (fun, 0) = base;
    }

    /*
     * the only son of the arg type has been reused, now we free its constructor!
     */
    arg = TYfreeTypeConstructor (arg);

    DBUG_EXECUTE (tmp = TYtype2DebugString (fun, TRUE, 0));
    DBUG_PRINT ("fun type built: %s\n", tmp);
    DBUG_EXECUTE (tmp = MEMfree (tmp));

    DBUG_PRINT_TAG ("NTY_MEM", "Allocated mem on leaving  TYmakeFunType: %zu",
                    global.current_allocated_mem);

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakeOverloadedFunType( ntype *fun1, ntype *fun2)
 *
 * description:
 *  function for merging two function types. It inserts fun1 into fun2.
 *  Therefore, it most likely performs better, if fun1 contains less instances.
 *
 *  Due to the complexity of this operation, several helper functions are
 *  introduced first. These are:
 *
 *    ntype *MergeSons( ntype *fun1, ntype *fun2 , int start, int stop)
 *
 *    ntype *ProjDown( ntype *ires, ntype *template)
 *    void AdjustSons( ntype **fun1_p, ntype **fun2_p, int start, int stop)
 *
 ******************************************************************************/

#ifndef DBUG_OFF
static void DebugPrintDispatchInfo (char *dbug_str, ntype *ires);
#endif

static ntype *MakeOverloadedFunType (ntype *fun1, ntype *fun2);

ntype *
FilterFundefs (ntype *fun, int num_kills, node **kill_list)
{
    size_t i;
    int j;
    size_t new_numfuns = 0;
    node **new_fundefs;
    int *new_poss;

    DBUG_ENTER ();

    if (fun != NULL) {
        switch (NTYPE_CON (fun)) {
        case TC_fun:
            for (i = 2; i < NTYPE_ARITY (fun);) {
                NTYPE_SON (fun, i)
                  = FilterFundefs (NTYPE_SON (fun, i), num_kills, kill_list);
                if (NTYPE_SON (fun, i) == NULL) {
                    fun = DeleteSon (fun, i);
                } else {
                    i++;
                }
            }
            break;
        case TC_ibase:
            IBASE_GEN (fun) = FilterFundefs (IBASE_GEN (fun), num_kills, kill_list);
            if (IBASE_GEN (fun) == NULL) {
                fun = TYfreeType (fun);
            } else {
                IBASE_SCAL (fun) = FilterFundefs (IBASE_SCAL (fun), num_kills, kill_list);
                IBASE_IARR (fun) = FilterFundefs (IBASE_IARR (fun), num_kills, kill_list);
            }
            break;
        case TC_iarr:
        case TC_idim:
        case TC_ishape:
            NTYPE_SON (fun, 0) = FilterFundefs (NTYPE_SON (fun, 0), num_kills, kill_list);
            if (NTYPE_SON (fun, 0) == NULL) {
                fun = TYfreeType (fun);
            } else {
                i = 1;
                while (i < NTYPE_ARITY (fun)) {
                    NTYPE_SON (fun, i)
                      = FilterFundefs (NTYPE_SON (fun, i), num_kills, kill_list);
                    if (NTYPE_SON (fun, i) == NULL) {
                        fun = DeleteSon (fun, i);
                    } else {
                        i++;
                    }
                }
            }
            break;
        case TC_ires:
            /* First, we count the number of functions that survive: */
            for (i = 0; i < IRES_NUMFUNS (fun); i++) {
                j = 0;
                while ((j < num_kills) && (IRES_FUNDEF (fun, i) != kill_list[j])) {
                    j++;
                }
                if (j == num_kills) { /* not found! */
                    new_numfuns++;
                } else {
                    IRES_FUNDEF (fun, i) = NULL;
                }
            }

            /*
             * Now, we do know that
             *  a) new_numfuns  fundefs survived
             *  b) all fundefs to be killed are NULLed
             */
            if (new_numfuns == 0) {
                fun = TYfreeType (fun);
            } else {
                new_fundefs = (node **)MEMmalloc (sizeof (node *) * new_numfuns);
                new_poss = (int *)MEMmalloc (sizeof (int) * new_numfuns);
                j = 0;
                for (i = 0; i < IRES_NUMFUNS (fun); i++) {
                    if (IRES_FUNDEF (fun, i) != NULL) {
                        new_fundefs[j] = IRES_FUNDEF (fun, i);
                        new_poss[j] = IRES_POS (fun, i);
                        j++;
                    }
                }
                IRES_FUNDEFS (fun) = MEMfree (IRES_FUNDEFS (fun));
                IRES_POSS (fun) = MEMfree (IRES_POSS (fun));
                IRES_NUMFUNS (fun) = new_numfuns;
                IRES_FUNDEFS (fun) = new_fundefs;
                IRES_POSS (fun) = new_poss;

                IRES_TYPE (fun) = FilterFundefs (IRES_TYPE (fun), num_kills, kill_list);
            }
            break;
        case TC_prod:
        case TC_alpha:
            break;
        default:
            DBUG_UNREACHABLE ("FilterFundefs called with illegal funtype!");
        }
    }

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *   ntype *ProjDown( ntype *ires, ntype *template)
 *
 * description:
 *   Copies ires and inspects the positioning of the associated fundefs.
 *   Those that are upward projections are NOT projected down!
 *   (If all fundefs turn out to be upward projections, NULL is returned!)
 *   If the template ntype-node is not an ires node, a copy of it is put
 *   in front of the freshly generated ires node and returned.
 *
 ******************************************************************************/

static ntype *
ProjDown (ntype *ires, ntype *xtemplate)
{
    size_t i;
    int new_numfuns = 0;
    int num_kills = 0;
    ntype *res = NULL;
    ntype *tmp = NULL;
    node **kill_list;

    DBUG_ENTER ();

    kill_list = (node **)MEMmalloc (sizeof (node *) * IRES_NUMFUNS (ires));

    /*
     * First, we count the number of functions that can be projected and
     * we initialize the kill_list with all those that cannot be projected.
     */
    for (i = 0; i < IRES_NUMFUNS (ires); i++) {
        if (IRES_POS (ires, i) <= 0) {
            new_numfuns++;
        } else {
            kill_list[num_kills] = IRES_FUNDEF (ires, i);
            num_kills++;
        }
    }

    /* If no function can be projected, we are done */
    if (new_numfuns > 0) {
        res = TYcopyFixedType (ires);
        res = FilterFundefs (res, num_kills, kill_list);
        for (i = 0; i < IRES_NUMFUNS (res); i++) {
            IRES_POS (res, i) = IRES_POS (res, i) - 1;
        }
        if (NTYPE_CON (xtemplate) != TC_ires) {
            tmp = res;
            res = TYcopyTypeConstructor (xtemplate);
            NTYPE_ARITY (res) = 1;
            NTYPE_SONS (res) = (ntype **)MEMmalloc (sizeof (ntype *) * NTYPE_ARITY (res));
            NTYPE_SON (res, 0) = tmp;
        }
    }

    kill_list = MEMfree (kill_list);

    DBUG_RETURN (res);
}

typedef bool (*cmp_ntype_fun_t) (ntype *, ntype *);

static bool
CmpIbase (ntype *ibase1, ntype *ibase2)
{
    DBUG_ASSERT (((NTYPE_CON (ibase1) == TC_ibase) && (NTYPE_CON (ibase2) == TC_ibase)),
                 "CmpIbase called with non TC_ibase arg!");

    return (TYeqTypes (IBASE_BASE (ibase1), IBASE_BASE (ibase2)));
}

static bool
CmpIdim (ntype *idim1, ntype *idim2)
{
    DBUG_ASSERT (((NTYPE_CON (idim1) == TC_idim) && (NTYPE_CON (idim2) == TC_idim)),
                 "CmpIdim called with non TC_idim arg!");

    return (IDIM_DIM (idim1) == IDIM_DIM (idim2));
}

static bool
CmpIshape (ntype *ishape1, ntype *ishape2)
{
    DBUG_ASSERT (((NTYPE_CON (ishape1) == TC_ishape)
                  && (NTYPE_CON (ishape2) == TC_ishape)),
                 "CmpIshape called with non TC_ishape arg!");

    return (SHcompareShapes (ISHAPE_SHAPE (ishape1), ISHAPE_SHAPE (ishape2)));
}

static ntype *
FindAndMergeSons (ntype *fun1, ntype *fun2, size_t start, cmp_ntype_fun_t CmpFun)
{
    size_t i, j;
    bool found;

    for (i = start; i < NTYPE_ARITY (fun1); i++) {
        found = FALSE;
        j = start;
        while ((j < NTYPE_ARITY (fun2)) && !found) {
            found = CmpFun (NTYPE_SON (fun1, i), NTYPE_SON (fun2, j));
            j++;
        }
        if (found) {
            NTYPE_SON (fun2, j - 1)
              = MakeOverloadedFunType (NTYPE_SON (fun1, i), NTYPE_SON (fun2, j - 1));
        } else {
            fun2 = MakeNewSon (fun2, NTYPE_SON (fun1, i));
        }
    }
    return (fun2);
}

static void
FindOrAdjustSons (ntype **fun1_p, ntype **fun2_p, size_t start, cmp_ntype_fun_t CmpFun)
{
    size_t i, j;
    bool found;
    ntype *fun1, *fun2, *tmp;

    fun1 = *fun1_p;
    fun2 = *fun2_p;

    for (i = start; i < NTYPE_ARITY (fun1); i++) {
        found = FALSE;
        j = start;
        while ((j < NTYPE_ARITY (fun2)) && !found) {
            found = CmpFun (NTYPE_SON (fun1, i), NTYPE_SON (fun2, j));
            j++;
        }
        if (!found) {
            tmp = ProjDown (NTYPE_SON (fun2, 0), NTYPE_SON (fun1, i));
            if (tmp != NULL) {
                fun2 = MakeNewSon (fun2, tmp);
            }
        }
    }

    for (i = start; i < NTYPE_ARITY (fun2); i++) {
        found = FALSE;
        j = start;
        while ((j < NTYPE_ARITY (fun1)) && !found) {
            found = CmpFun (NTYPE_SON (fun2, i), NTYPE_SON (fun1, j));
            j++;
        }
        if (!found) {
            tmp = ProjDown (NTYPE_SON (fun1, 0), NTYPE_SON (fun2, i));
            if (tmp != NULL) {
                fun1 = MakeNewSon (fun1, tmp);
            }
        }
    }

    *fun1_p = fun1;
    *fun2_p = fun2;
}

/******************************************************************************
 *
 * function:
 *    ntype *MergeSons( ntype *fun1, ntype *fun2 , int start, int stop)
 *
 * description:
 *    "zips" MakeOverloadedFunType to all sons of fun1 and fun2.
 *    The sons considered are those between start and (stop-1).
 *
 ******************************************************************************/

static ntype *
MergeSons (ntype *fun1, ntype *fun2, size_t start, size_t stop)
{
    size_t i;

    DBUG_ENTER ();

    for (i = start; i < (stop); i++) {
        NTYPE_SON (fun2, i)
          = MakeOverloadedFunType (NTYPE_SON (fun1, i), NTYPE_SON (fun2, i));
    }

    DBUG_RETURN (fun2);
}

/******************************************************************************
 *
 * function:
 *    void AdjustSons( ntype **fun1_p, ntype **fun2_p, int start, int stop)
 *
 * description:
 *    "zips" ProjDown to all those sons of fun1 and fun2 that exist once only.
 *    The sons considered are those between start and (stop-1).
 *
 ******************************************************************************/

static void
AdjustSons (ntype **fun1_p, ntype **fun2_p, int start, int stop)
{
    ntype *fun1, *fun2;
    int i;

    DBUG_ENTER ();

    fun1 = *fun1_p;
    fun2 = *fun2_p;

    for (i = start; i < (stop); i++) {
        if (NTYPE_SON (fun1, i) != NULL) {
            if (NTYPE_SON (fun2, i) == NULL) {
                NTYPE_SON (fun2, i) = ProjDown (NTYPE_SON (fun2, 0), NTYPE_SON (fun1, i));
            }
        } else {
            if (NTYPE_SON (fun2, i) != NULL) {
                NTYPE_SON (fun1, i) = ProjDown (NTYPE_SON (fun1, 0), NTYPE_SON (fun2, i));
            }
        }
    }

    *fun1_p = fun1;
    *fun2_p = fun2;

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * NOW, the implementation of   TYmakeOverloadedFunType    itself!!
 *
 ******************************************************************************/

#ifndef DBUG_OFF
static tvar **overload_fun1_alphas;
#endif
static size_t overload_num_luts = 0;
static int overload_pos = 0;
static lut_t **overload_luts;

ntype *
TYmakeOverloadedFunType (ntype *fun1, ntype *fun2)
{
    ntype *res;
    size_t i;
#ifndef DBUG_OFF
    char *tmp = NULL, *tmp2 = NULL;
#endif

    DBUG_ENTER ();

    DBUG_EXECUTE (tmp = TYtype2DebugString (fun1, TRUE, 0);
                  tmp2 = TYtype2DebugString (fun2, TRUE, 0));
    DBUG_PRINT ("functions:        %s", tmp);
    DBUG_PRINT ("and               %s", tmp2);
    DBUG_EXECUTE (tmp = MEMfree (tmp); tmp2 = MEMfree (tmp2));

    /*
     * iff this is the very first call, instantiate rel. free vars 8-))
     *
     * we need max num rets many LUTs here.
     * Since we do not statically now, we start with 5 LUTs. If it turns
     * out during overloading that these are not enough, we simply allocate
     * further ones...
     */
    if (overload_num_luts == 0) {
        overload_num_luts = 5;
#ifndef DBUG_OFF
        overload_fun1_alphas = (tvar **)MEMmalloc (overload_num_luts * sizeof (tvar *));
        for (i = 0; i < overload_num_luts; i++) {
            overload_fun1_alphas[i] = NULL;
        }
#endif
        overload_luts = (lut_t **)MEMmalloc (overload_num_luts * sizeof (lut_t *));
        for (i = 0; i < overload_num_luts; i++) {
            overload_luts[i] = LUTgenerateLut ();
        }
    }

    if ((fun1 != NULL) && (NTYPE_CON (fun1) != TC_fun) && (fun2 != NULL)
        && (NTYPE_CON (fun2) != TC_fun)) {
        CTIabortLine (global.linenum, "Cannot overload functions of arity 0");
    }

    res = MakeOverloadedFunType (fun1, fun2);

    /*
     * remove rel free vars
     */
    for (i = 0; i < overload_num_luts; i++) {
#ifndef DBUG_OFF
        overload_fun1_alphas[i] = NULL;
#endif
        overload_luts[i] = LUTremoveLut (overload_luts[i]);
    }
    overload_luts = MEMfree (overload_luts);
    overload_num_luts = 0;

    DBUG_EXECUTE (tmp = TYtype2DebugString (res, TRUE, 0));
    DBUG_PRINT ("overloaded into : %s", tmp);
    DBUG_EXECUTE (tmp = MEMfree (tmp));

    DBUG_RETURN (res);
}

static ntype *
MakeOverloadedFunType (ntype *fun1, ntype *fun2)
{
    ntype *lub, *res;
    tvar *old_alpha;
    bool ok;
    size_t i;
    size_t new_num_luts;
#ifndef DBUG_OFF
    tvar **new_alphas;
    char *tmpstring = NULL;
#endif
    lut_t **new_luts;

    DBUG_ENTER ();

    if (fun1 == NULL) {
        res = fun2;
    } else if (fun2 == NULL) {
        res = fun1;
    } else {
        DBUG_EXECUTE (tmpstring = TYtype2DebugString (fun1, TRUE, 0));
        DBUG_PRINT ("fun1: %s", tmpstring);

        DBUG_EXECUTE (tmpstring = TYtype2DebugString (fun2, TRUE, 0));
        DBUG_PRINT ("fun2: %s", tmpstring);

        DBUG_ASSERT (NTYPE_CON (fun1) == NTYPE_CON (fun2),
                     "TYOverloadFunType called with incompatible types!");

        res = fun2;
        switch (NTYPE_CON (fun1)) {
        case TC_fun:
            fun2 = MergeSons (fun1, fun2, 0, 2);
            fun2 = FindAndMergeSons (fun1, fun2, 2, CmpIbase);
            break;
        case TC_ibase:
            AdjustSons (&fun1, &fun2, 1, 3);
            fun2 = MergeSons (fun1, fun2, 0, 3);
            break;
        case TC_iarr:
            FindOrAdjustSons (&fun1, &fun2, 1, CmpIdim);
            fun2 = MergeSons (fun1, fun2, 0, 1);
            fun2 = FindAndMergeSons (fun1, fun2, 1, CmpIdim);
            break;
        case TC_idim:
            FindOrAdjustSons (&fun1, &fun2, 1, CmpIshape);
            fun2 = MergeSons (fun1, fun2, 0, 1);
            fun2 = FindAndMergeSons (fun1, fun2, 1, CmpIshape);
            break;
        case TC_ishape:
            fun2 = MergeSons (fun1, fun2, 0, 1);
            break;
        case TC_ires:
            DBUG_ASSERT (((TYisProd (IRES_TYPE (fun1)) && TYisProd (IRES_TYPE (fun2)))
                          || (TYisFun (IRES_TYPE (fun1)) && TYisFun (IRES_TYPE (fun2)))),
                         "trying to overload incompatible function types");
            res = MakeNewFundefsPoss (fun2, IRES_NUMFUNS (fun1), IRES_FUNDEFS (fun1),
                                      IRES_POSS (fun1));

            DBUG_PRINT_TAG ("NTOVLD", "new ires:");
            DBUG_EXECUTE_TAG ("NTOVLD", DebugPrintDispatchInfo ("NTOVLD", res));

            MakeOverloadedFunType (IRES_TYPE (fun1), IRES_TYPE (fun2));
            break;
        case TC_prod:
            DBUG_ASSERT (NTYPE_ARITY (fun1) == NTYPE_ARITY (fun2),
                         "trying to overload function types with different number"
                         " of return types");
            if (NTYPE_ARITY (fun1) > overload_num_luts) {
                new_num_luts = overload_num_luts + NTYPE_ARITY (fun1);
#ifndef DBUG_OFF
                new_alphas = (tvar **)MEMmalloc (new_num_luts * sizeof (tvar *));
                for (i = 0; i < overload_num_luts; i++) {
                    new_alphas[i] = overload_fun1_alphas[i];
                }
                for (; i < new_num_luts; i++) {
                    new_alphas[i] = NULL;
                }
                overload_fun1_alphas = MEMfree (overload_fun1_alphas);
                overload_fun1_alphas = new_alphas;
#endif
                new_luts = (lut_t **)MEMmalloc (new_num_luts * sizeof (lut_t *));
                for (i = 0; i < overload_num_luts; i++) {
                    new_luts[i] = overload_luts[i];
                }
                for (; i < new_num_luts; i++) {
                    new_luts[i] = LUTgenerateLut ();
                }
                overload_luts = MEMfree (overload_luts);
                overload_luts = new_luts;
                overload_num_luts = new_num_luts;
            }
            overload_pos = 0;
            fun2 = MergeSons (fun1, fun2, 0, NTYPE_ARITY (fun1));
            break;
        case TC_alpha:
#ifndef DBUG_OFF
            /*
             * check whether fun1 is not yet overloaded!
             */
            if (overload_fun1_alphas[overload_pos] == NULL) {
                overload_fun1_alphas[overload_pos] = ALPHA_SSI (fun1);
            } else {
                DBUG_ASSERT (overload_fun1_alphas[overload_pos] == ALPHA_SSI (fun1),
                             "TYmakeOverloadedFunType called with overloaded fun1!");
            }
#endif
            if (SSIisLe (ALPHA_SSI (fun1), ALPHA_SSI (fun2))) {
                res = fun2;
            } else if (SSIisLe (ALPHA_SSI (fun2), ALPHA_SSI (fun1))) {
                res = TYcopyType (fun1);
                fun2 = TYfreeTypeConstructor (fun2);
            } else {
                old_alpha = (tvar *)LUTsearchInLutPp (overload_luts[overload_pos],
                                                      ALPHA_SSI (fun2));
                if (old_alpha != ALPHA_SSI (fun2)) { /* found! */
                    res = MakeNtype (TC_alpha, 0);
                    ALPHA_SSI (res) = old_alpha;
                } else {
                    lub = TYlubOfTypes (SSIgetMax (ALPHA_SSI (fun1)),
                                        SSIgetMax (ALPHA_SSI (fun2)));
                    if (lub == NULL) {
                        CTIabortLine (global.linenum,
                                      "Cannot overload functions with disjoint result "
                                      "type;"
                                      " types found: \"%s\" and \"%s\"",
                                      TYtype2String (SSIgetMax (ALPHA_SSI (fun1)), FALSE,
                                                     0),
                                      TYtype2String (SSIgetMax (ALPHA_SSI (fun2)), FALSE,
                                                     0));
                    } else {
                        res = TYmakeAlphaType (lub);
                        ok = SSInewRel (ALPHA_SSI (fun1), ALPHA_SSI (res));
                        DBUG_ASSERT (ok,
                                     "SSInewRel did not work in TYmakeOverloadFunType");
                        ok = SSInewRel (ALPHA_SSI (fun2), ALPHA_SSI (res));
                        DBUG_ASSERT (ok,
                                     "SSInewRel did not work in TYmakeOverloadFunType");
                        overload_luts[overload_pos]
                          = LUTinsertIntoLutP (overload_luts[overload_pos],
                                               ALPHA_SSI (fun2), ALPHA_SSI (res));
                    }
                }
            }
            overload_pos++;
            break;
        default:
            DBUG_UNREACHABLE ("TYmakeOverloadFunType called with illegal funtype!");
        }
        fun1 = TYfreeTypeConstructor (fun1);
    }

    DBUG_RETURN (res);
}

/** <!-- ***************************************************************** -->
 * @fn ntype *mapFunctionInstances( ntype *type,
 *                                  node *(*mapfun)( node  *, info *),
 *                                  info *info)
 *
 * @brief Helper function used by TYmapFunctionInstances.
 *
 * @param type a valid ntype
 * @param mapfun the function to map to the instances
 * @param info info node to pass along
 *
 * @return the (unmodified) ntype
 */
ntype *
mapFunctionInstances (ntype *type, node *(*mapfun) (node *, info *), info *info)
{
    size_t cnt;

    DBUG_ENTER ();

    if (type != NULL) {
        switch (NTYPE_CON (type)) {
        case TC_ires:
            /*
             * we want to walk down until we reach the leaf (which is
             * a product type). Once we arrived there, we know that
             * this IRES node contains all instances for the
             * given basetype combination.
             */
            if (TYisProd (IRES_TYPE (type))) {
                for (cnt = 0; cnt < IRES_NUMFUNS (type); cnt++) {
                    IRES_FUNDEF (type, cnt) = mapfun (IRES_FUNDEF (type, cnt), info);
                }
            } else {
                IRES_TYPE (type) = mapFunctionInstances (IRES_TYPE (type), mapfun, info);
            }
            break;

        case TC_fun:
            /*
             * starting at a fun node, we walk down the tree for every
             * basetype.
             */
            for (cnt = 0; cnt < NTYPE_ARITY (type); cnt++) {
                NTYPE_SON (type, cnt)
                  = mapFunctionInstances (NTYPE_SON (type, cnt), mapfun, info);
            }
            break;

        case TC_ibase:
            /*
             * we only walk down the [*] edge, as this will contain
             * all instances
             */
            IBASE_GEN (type) = mapFunctionInstances (IBASE_GEN (type), mapfun, info);
            break;

        default:
            DBUG_UNREACHABLE ("mapFunctionInstances reached a type-constructur it never "
                              "was intended to reach!");
        }
    }

    DBUG_RETURN (type);
}

/** <!-- ***************************************************************** -->
 * @fn ntype *TYmapFunctionInstances( ntype *funtype,
 *                                    node *(*mapfun)(node  *, info *),
 *                                    info *info)
 *
 * @brief Maps the given function mapfun to all N_fundef nodes
 *        that are contained within the given funtype. Be aware that
 *        only the instances in the leafs are reached. This should be
 *        all instances anyways, but if some code (e.g. SplitWrappers)
 *        messes the set of instances up, this may cause unexpected
 *        behaviour!
 *
 * @param funtype a function type
 * @param mapfun the function to map to the instances
 * @param info info node to pass along
 *
 * @return the (unmodified) funtype
 */

ntype *
TYmapFunctionInstances (ntype *funtype, node *(*mapfun) (node *, info *), info *info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (funtype) == TC_fun,
                 "called TYmapFunctionInstances with non function type");

    funtype = mapFunctionInstances (funtype, mapfun, info);

    DBUG_RETURN (funtype);
}

void *
foldFunctionInstances (ntype *type, void *(*foldfun) (node *, void *), void *result)
{
    size_t cnt;

    DBUG_ENTER ();

    if (type != NULL) {
        switch (NTYPE_CON (type)) {
        case TC_ires:
            /*
             * we want to walk down until we reach the leaf (which is
             * a product type). Once we arrived there, we know that
             * this IRES node contains all instances for the
             * given basetype combination.
             */
            if (TYisProd (IRES_TYPE (type))) {
                for (cnt = 0; cnt < IRES_NUMFUNS (type); cnt++) {
                    result = foldfun (IRES_FUNDEF (type, cnt), result);
                }
            } else {
                result = foldFunctionInstances (IRES_TYPE (type), foldfun, result);
            }
            break;

        case TC_fun:
            /*
             * starting at a fun node, we walk down the tree for every
             * basetype.
             */
            for (cnt = 0; cnt < NTYPE_ARITY (type); cnt++) {
                result = foldFunctionInstances (NTYPE_SON (type, cnt), foldfun, result);
            }
            break;

        case TC_ibase:
            /*
             * we only walk down the [*] edge, as this will contain
             * all instances
             */
            result = foldFunctionInstances (IBASE_GEN (type), foldfun, result);
            break;

        default:
            DBUG_UNREACHABLE (
              "foldFunctionInstances passed a typeconstructur it never was "
              "intended to pass!");

            result = NULL;
        }
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn void *TYfoldFunctionInstances( ntype *funtype,
 *                                    void *(*foldfun)(node *, void*),
 *                                    void *initial)
 *
 * @brief Folds the given function mapfun to all N_fundef nodes
 *        that are contained within the given funtype. Be aware that
 *        only the instances in the leafs are reached. This should be
 *        all instances anyways, but if some code (e.g. SplitWrappers)
 *        messes the set of instances up, this may cause unexpected
 *        behaviour!
 *
 * @param funtype a function type
 * @param foldfun the fold function
 * @param initial neutral element of the fold operation
 ******************************************************************************/
void *
TYfoldFunctionInstances (ntype *funtype, void *(*foldfun) (node *, void *), void *initial)
{
    void *result;

    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (funtype) == TC_fun,
                 "TYfoldFunctionInstances called with non-function type");

    result = foldFunctionInstances (funtype, foldfun, initial);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn int TYgetArity( ntype *fun)
 *
 *   @brief counts the number of TC_fun constructors found on the "topmost"
 *          chain.
 *   @param TC_fun node
 *   @return the number of TC_fun nodes found
 *
 ******************************************************************************/

size_t
TYgetArity (ntype *fun)
{
    size_t res = 0;
    ntype *next;

    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (fun) == TC_fun, "TYgetArity applied to non function type");
    DBUG_ASSERT (NTYPE_ARITY (fun) >= 3, "TC_fun with (ARITY < 3) found!");
    if (FUN_IBASE (fun, 0) != NULL) {
        next = IRES_TYPE (IBASE_GEN (FUN_IBASE (fun, 0)));
    } else if (FUN_POLY (fun) != NULL) {
        next = IRES_TYPE (IBASE_GEN (FUN_POLY (fun)));
    } else if (FUN_UPOLY (fun) != NULL) {
        next = IRES_TYPE (IBASE_GEN (FUN_UPOLY (fun)));
    } else {
        DBUG_UNREACHABLE ("TC_fun without bases found!");
        next = NULL;
    }

    if (NTYPE_CON (next) == TC_fun) {
        res = 1 + TYgetArity (next);
    } else {
        res = 1;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   dft_res * TYmakedft_res( ntype *type, int max_funs)
 *
 * Description:
 *
 *
 ******************************************************************************/

dft_res *
TYmakedft_res (ntype *type, size_t max_funs)
{
    dft_res *res;

    DBUG_ENTER ();

    res = (dft_res *)MEMmalloc (sizeof (dft_res));

    res->type = type;
    res->def = NULL;
    res->deriveable = NULL;
    res->num_partials = 0;
    res->partials = (node **)MEMmalloc (sizeof (node *) * max_funs);
    res->num_deriveable_partials = 0;
    res->deriveable_partials = (node **)MEMmalloc (sizeof (node *) * max_funs);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   dft_res * TYfreedft_res( dft_res *res)
 *
 * Description:
 *
 *
 ******************************************************************************/

dft_res *
TYfreeDft_res (dft_res *res)
{
    DBUG_ENTER ();

    DBUG_ASSERT (res != NULL, "argument is NULL");

    if (res->partials != NULL) {
        res->partials = MEMfree (res->partials);
    }
    if (res->deriveable_partials != NULL) {
        res->deriveable_partials = MEMfree (res->partials);
    }

    res = MEMfree (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   dft_state *AllocDFT_state( int max_funs)
 *
 * Description:
 *
 *
 ******************************************************************************/

static dft_state *
AllocDFT_state (size_t max_funs)
{
    dft_state *state;

    DBUG_ENTER ();

    state = (dft_state *)MEMmalloc (sizeof (dft_state));

    state->max_funs = max_funs;
    state->cnt_funs = 0;
    state->fundefs = (node **)MEMmalloc (max_funs * sizeof (node *));
    state->legal = (bool *)MEMmalloc (max_funs * sizeof (bool));
    state->ups = (int *)MEMmalloc (max_funs * sizeof (int));
    state->downs = (int *)MEMmalloc (max_funs * sizeof (int));

    DBUG_RETURN (state);
}

/******************************************************************************
 *
 * Function:
 *   dft_state *freeDFT_state( dft_state *state)
 *
 * Description:
 *
 *
 ******************************************************************************/

static dft_state *
freeDFT_state (dft_state *state)
{
    DBUG_ENTER ();

    state->fundefs = MEMfree (state->fundefs);
    state->legal = MEMfree (state->legal);
    state->ups = MEMfree (state->ups);
    state->downs = MEMfree (state->downs);

    state = MEMfree (state);

    DBUG_RETURN (state);
}

/******************************************************************************
 *
 * Function:
 *   dft_state *CopyDFT_state( dft_state *state)
 *
 * Description:
 *
 *
 ******************************************************************************/

static dft_state *
CopyDFT_state (dft_state *state)
{
    dft_state *new_state;
    size_t i;

    DBUG_ENTER ();

    new_state = AllocDFT_state (state->max_funs);

    new_state->cnt_funs = state->cnt_funs;
    for (i = 0; i < new_state->max_funs; i++) {
        new_state->fundefs[i] = state->fundefs[i];
        new_state->legal[i] = state->legal[i];
        new_state->ups[i] = state->ups[i];
        new_state->downs[i] = state->downs[i];
    }

    DBUG_RETURN (new_state);
}

/******************************************************************************
 *
 * Function:
 *   dft_state *InsertFirstArgDFT_state( dft_state *state,
 *                                       ntype *ires, int lower)
 *
 * Description:
 *
 *
 ******************************************************************************/

static dft_state *
InsertFirstArgDFT_state (dft_state *state, ntype *ires, int lower)
{
    size_t cnt, i;

    DBUG_ENTER ();

    cnt = 0;
    for (i = 0; i < state->max_funs; i++) {
        state->fundefs[i] = IRES_FUNDEF (ires, i);
        if ((IRES_POS (ires, i) <= 0) || (lower == 0)) {
            cnt++;
            state->legal[i] = TRUE;
            if (IRES_POS (ires, i) > 0) {
                state->ups[i] = IRES_POS (ires, i);
                state->downs[i] = 0;
            } else {
                state->ups[i] = 0;
                state->downs[i] = IRES_POS (ires, i) - lower;
            }
        } else {
            state->legal[i] = FALSE;
        }
    }

    state->cnt_funs = cnt;

    DBUG_RETURN (state);
}

/******************************************************************************
 *
 * Function:
 *   dft_state *InsertNextArgDFT_state( dft_state *state,
 *                                      ntype *ires, int lower)
 *
 * Description:
 *
 *
 ******************************************************************************/

static dft_state *
InsertNextArgDFT_state (dft_state *state, ntype *ires, int lower)
{
    size_t cnt, i, j;

    DBUG_ENTER ();

    cnt = 0;
    j = 0;
    for (i = 0; i < state->max_funs; i++) {
        if ((j < IRES_NUMFUNS (ires)) && (IRES_FUNDEF (ires, j) == state->fundefs[i])) {
            if (IRES_POS (ires, j) > 0) {
                if (lower > 0) {
                    state->legal[i] = FALSE;
                } else {
                    state->ups[i] += IRES_POS (ires, j);
                    cnt++;
                }
            } else {
                state->downs[i] += IRES_POS (ires, j) - lower;
                cnt++;
            }
            j++;
        } else {
            state->fundefs[i] = NULL;
        }
    }

    state->cnt_funs = cnt;

    DBUG_RETURN (state);
}

/******************************************************************************
 *
 * Function:
 *   dft_state *FinalizeDFT_state( dft_state *state)
 *
 * Description:
 *
 *
 ******************************************************************************/

static dft_state *
FinalizeDFT_state (dft_state *state)
{
    size_t i;

    DBUG_ENTER ();

    /*
     * all those that are lower than up-projections were marked as illegal
     * before; now, we set the according fundefs to NULL.
     * In contrast to (errorneous) considerations made before, these cannot(!)
     * be set to NULL earlier since that would not guarantee the i / j mechanism
     * in InsertNextArgDFT_state() to work properly anymore.
     */

    for (i = 0; i < state->max_funs; i++) {
        if (!state->legal[i]) {
            state->fundefs[i] = NULL;
        }
    }

    DBUG_RETURN (state);
}

/******************************************************************************
 *
 * Function:
 *   dft_res *DFT_state2dft_res( dft_state *state)
 *
 * Description:
 *
 *
 ******************************************************************************/

static bool
AllArgTypesLe (node *fundef, node *fundef2)
{
    node *arg, *arg2;

    DBUG_ENTER ();

    arg = FUNDEF_ARGS (fundef);
    arg2 = FUNDEF_ARGS (fundef2);
    while ((arg != NULL)
           && TYleTypes (AVIS_TYPE (ARG_AVIS (arg)), AVIS_TYPE (ARG_AVIS (arg2)))) {
        arg = ARG_NEXT (arg);
        arg2 = ARG_NEXT (arg2);
    }

    DBUG_RETURN ((arg == NULL));
}

static void
EliminateDeriveablePartial (node **dp_list, size_t *dp2ud, int length, int pos)
{
    int i;

    DBUG_ENTER ();

    for (i = pos; i < length - 1; i++) {
        dp_list[i] = dp_list[i + 1];
        dp2ud[i] = dp2ud[i + 1];
    }

    DBUG_RETURN ();
}

static dft_res *
DFT_state2dft_res (dft_state *state)
{
    dft_res *res;
    int max_deriveable;
    bool exact_found = FALSE;
    int i, j;
    size_t k;
    int dom, irr;
    size_t *dp2ud;
    size_t *p2ud;

    DBUG_ENTER ();

    res = TYmakedft_res (NULL, state->cnt_funs);
    dp2ud = (size_t *)MEMmalloc (state->cnt_funs * sizeof (size_t));
    p2ud = (size_t *)MEMmalloc (state->cnt_funs * sizeof (size_t));

    /*
     * First, we analyze the accumulated ups and downs:
     *   ups   downs   consequences
     *    0     0      - defined here
     *                     => shadows all potential deriveables
     *                     => shadows indirectly all deriveable_partials
     *                        reason: we do have a violated monotonic condition
     *                                this may only result from specialization
     *                                which in fact MUST have triggered a
     *                                specialization of the conflicting definition
     *                                (the deriveable_partial) as well
     *   0     -m      - down projection
     *                     choose the one with minimum |m| !
     *   n      0      - partial
     *                     keep them all!
     *   n     -m      - deriveable partial
     *                     iff we do not have an exact definition, keep them all
     *                     (see comment above).
     *                     However, in SOME cases where we have partials /
     *                     deriveable partials with identical n, these may shadow
     *                     each other. This shadowing is eliminated at the very
     *                     end of this function (see comment down below!)
     *
     * While doing so, we set up dp2ud and p2ud which are needed for the
     * correction to be made later (see comment down below!)
     */
    /* Due to a bug in limits.h (!!), we have to use INT_MAX here!! */
    max_deriveable = 1 - INT_MAX;
    i = 0;
    for (k = 0; k < state->max_funs; k++, i++) {
        if (state->fundefs[k] != NULL) {
            if (state->ups[k] == 0) {
                if (state->downs[k] == 0) {
                    res->def = state->fundefs[k];
                    /* no down projections in case of an exact definition! */
                    max_deriveable = 0;
                    res->deriveable = NULL;
                    /* no deriveable partials in case of an exact definition! */
                    exact_found = TRUE;
                    res->num_deriveable_partials = 0;
                } else {
                    if (state->downs[k] > max_deriveable) {
                        res->deriveable = state->fundefs[k];
                        max_deriveable = state->downs[k];
                    }
                }
            } else {
                if (state->downs[k] == 0) {
                    res->partials[res->num_partials] = state->fundefs[k];
                    p2ud[res->num_partials] = k;
                    res->num_partials++;
                } else {
                    if (!exact_found) {
                        res->deriveable_partials[res->num_deriveable_partials]
                          = state->fundefs[k];
                        dp2ud[res->num_deriveable_partials] = k;
                        res->num_deriveable_partials++;
                    } /* else ignore it */
                }
            }
        }
    }

    /*
     * Finally, we might have to filter out some deriveable partials.
     * The problem here is that we cannot see from the ups and downs numbers
     * alone, whether a deriveable partial is shadowed by another deriveable
     * partial or a partial. Example:
     *
     *   fundef0:  int[*] -> int[3] -> alpha
     *   fundef1:  int[.] -> int[3] -> beta
     *   fundef2:  int[.] -> int[4] -> gamma
     *
     *  assuming an application int[.] int[.] we obtain:
     *  fundef0:  1  -2   => deriveable partial
     *  fundef1:  1   0   => partial            [ shadows fundef0 ]
     *  fundef2:  1   0   => partial            [ does not shadow fundef0 ]
     *
     *  assuming an application int[2] int[.] we obtain:
     *  fundef0:  1  -3   => deriveable partial
     *  fundef1:  1  -1   => deriveable partial [ shadows fundef0 ]
     *  fundef2:  1  -1   => deriveable partial [ does not shadow fundef0 ]
     */
    /*
     * Therefore, we establish the following filter mechanism:
     *  for each deriveable partial:
     *    find all partials and deriveable partials with identical ups values:
     *      check whether the up positions are identical;
     *      if so leave the one with smaller downs value
     */

    DBUG_PRINT_TAG ("NTDIS", "filtering derived partials:");

    for (i = 0; i < res->num_deriveable_partials; i++) {
        for (j = i + 1; j < res->num_deriveable_partials; j++) {
            if (state->ups[dp2ud[i]] == state->ups[dp2ud[j]]) {
                if (state->downs[dp2ud[i]] > state->downs[dp2ud[j]]) {
                    dom = i;
                    irr = j;
                } else {
                    dom = j;
                    irr = i;
                }
                DBUG_PRINT_TAG ("NTDIS", "  %p might shadow %p here",
                                (void *)state->fundefs[dp2ud[dom]],
                                (void *)state->fundefs[dp2ud[irr]]);
                if (AllArgTypesLe (state->fundefs[dp2ud[dom]],
                                   state->fundefs[dp2ud[irr]])) {
                    DBUG_PRINT_TAG ("NTDIS", "  eliminating %p",
                                    (void *)state->fundefs[dp2ud[irr]]);
                    EliminateDeriveablePartial (res->deriveable_partials, dp2ud,
                                                res->num_deriveable_partials, irr);
                    res->num_deriveable_partials--;
                    if (irr == i) {
                        i--;
                        j = res->num_deriveable_partials;
                    } else {
                        j--;
                    }
                }
            }
        }
    }
    for (i = 0; i < res->num_deriveable_partials; i++) {
        for (j = 0; j < res->num_partials; j++) {
            if (state->ups[dp2ud[i]] == state->ups[p2ud[j]]) {
                dom = j;
                irr = i;
                DBUG_PRINT_TAG ("NTDIS", "  %p might shadow %p here",
                                (void *)state->fundefs[p2ud[dom]],
                                (void *)state->fundefs[dp2ud[irr]]);
                if (AllArgTypesLe (state->fundefs[p2ud[dom]],
                                   state->fundefs[dp2ud[irr]])) {
                    DBUG_PRINT_TAG ("NTDIS", "  eliminating %p",
                                    (void *)state->fundefs[dp2ud[irr]]);
                    EliminateDeriveablePartial (res->deriveable_partials, dp2ud,
                                                res->num_deriveable_partials, irr);
                    res->num_deriveable_partials--;
                    i--;
                    j = res->num_partials;
                }
            }
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   ntype * TYdispatchFunType( ntype *fun, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

static ntype *
FindIbase (ntype *fun, ntype *scalar)
{
    ntype *res = NULL;
    size_t i = 0;

    DBUG_ENTER ();

    while ((i < NTYPE_ARITY (fun) - 2)
           && !TYeqTypes (IBASE_BASE (FUN_IBASE (fun, i)), scalar)) {
        i++;
    }
    if (i < (NTYPE_ARITY (fun) - 2)) {
        res = FUN_IBASE (fun, i);
    }

    DBUG_RETURN (res);
}

static ntype *
FindIdim (ntype *iarr, size_t dim)
{
    ntype *res = NULL;
    size_t i = 0;

    DBUG_ENTER ();

    while ((i < (NTYPE_ARITY (iarr) - 1)) && (IDIM_DIM (IARR_IDIM (iarr, i)) != dim)) {
        i++;
    }
    if (i < (NTYPE_ARITY (iarr) - 1)) {
        res = IARR_IDIM (iarr, i);
    }

    DBUG_RETURN (res);
}

static ntype *
FindIshape (ntype *idim, shape *shp)
{
    ntype *res = NULL;
    size_t i = 0;

    DBUG_ENTER ();

    while ((i < (NTYPE_ARITY (idim) - 1))
           && !SHcompareShapes (ISHAPE_SHAPE (IDIM_ISHAPE (idim, i)), shp)) {
        i++;
    }
    if (i < (NTYPE_ARITY (idim) - 1)) {
        res = IDIM_ISHAPE (idim, i);
    }

    DBUG_RETURN (res);
}

static ntype *
DispatchOneArg (int *lower_p, ntype *fun, ntype *arg)
{
    ntype *res = NULL;
    int lower = 0;

    /* a matching base type is mendatory! */
    fun = FindIbase (fun, TYgetScalar (arg));

    if (fun != NULL) {
        /*   new default:   <base>[*]   */
        res = IBASE_GEN (fun);

        if (((NTYPE_CON (arg) == TC_akv) || (NTYPE_CON (arg) == TC_aks)
             || (NTYPE_CON (arg) == TC_akd))
            && (TYgetDim (arg) == 0)) {
            /* argument is a scalar! */
            if (IBASE_SCAL (fun) == NULL) {
                lower = ((NTYPE_CON (arg) == TC_akv) ? 2 : 1);
            } else {
                res = IBASE_SCAL (fun);
            }
        } else {
            if (NTYPE_CON (arg) != TC_aud) {
                fun = IBASE_IARR (fun);
                if (fun == NULL) {
                    lower = ((NTYPE_CON (arg) == TC_akv)
                               ? 4
                               : ((NTYPE_CON (arg) == TC_aks)
                                    ? 3
                                    : ((NTYPE_CON (arg) == TC_akd) ? 2 : 1)));
                } else {

                    /*   new default:   <base>[+]   */
                    res = IARR_GEN (fun);

                    if (NTYPE_CON (arg) != TC_audgz) {
                        fun = FindIdim (fun, TYgetDim (arg));
                        if (fun == NULL) {
                            lower = ((NTYPE_CON (arg) == TC_akv)
                                       ? 3
                                       : (NTYPE_CON (arg) == TC_aks ? 2 : 1));
                        } else {

                            /*   new default:   <base>[...]   */
                            res = IDIM_GEN (fun);

                            if (NTYPE_CON (arg) != TC_akd) {
                                fun = FindIshape (fun, TYgetShape (arg));
                                if (fun == NULL) {
                                    lower = (NTYPE_CON (arg) == TC_akv ? 2 : 1);
                                } else {
                                    res = ISHAPE_GEN (fun);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    *lower_p = lower;

    return (res);
}

#ifndef DBUG_OFF

static void
DebugPrintDispatchInfo (char *dbug_str, ntype *ires)
{
    size_t i;

    DBUG_ENTER ();

    for (i = 0; i < IRES_NUMFUNS (ires); i++) {
        DBUG_PRINT_TAG (dbug_str, "  fundef %8p: %d", (void *)IRES_FUNDEF (ires, i),
                        IRES_POS (ires, i));
    }

    DBUG_RETURN ();
}

static void
DebugPrintDFT_state (dft_state *state)
{
    size_t i;

    DBUG_ENTER ();

    for (i = 0; i < state->max_funs; i++) {
        DBUG_PRINT_TAG ("NTDIS", "  fundef %8p: ups %2d | downs %2d", (void *)state->fundefs[i],
                        state->ups[i], state->downs[i]);
    }

    DBUG_RETURN ();
}

#endif /* DBUG_OFF */

dft_res *
TYdispatchFunType (ntype *fun, ntype *args)
{
    int lower;
    size_t i, n;
    ntype *arg, *ires;
    node *fundef;
    dft_res *res;
    dft_state *state = NULL;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    DBUG_ASSERT (fun != NULL, "first arg of TYDispatchFunType is NULL funtype !");
    DBUG_ASSERT (NTYPE_CON (args) == TC_prod,
                 "second arg of TYDispatchFunType non-product type!");

    n = NTYPE_ARITY (args);

    if (n == 0) {
        res = NULL;

    } else {

        for (i = 0; i < n; i++) {
            arg = PROD_MEMBER (args, i);
            ires = DispatchOneArg (&lower, fun, arg);
            if (ires == NULL) {
                fundef = IRES_FUNDEF (IBASE_GEN (FUN_IBASE (fun, 0)), 0);
                CTIabortLine (global.linenum,
                              "No definition found for a function \"%s\" that"
                              " accepts an argument of type \"%s\" as parameter"
                              " no %zu. Full argument types are \"%s\".",
                              CTIitemName (fundef), TYtype2String (arg, FALSE, 0), i + 1,
                              TYtype2String (args, FALSE, 0));
            }

            DBUG_EXECUTE_TAG ("NTDIS", tmp_str = TYtype2String (arg, FALSE, 0));
            DBUG_PRINT_TAG ("NTDIS", "arg #%zu: %s yields (lifted by %d):", i, tmp_str,
                            lower);
            DBUG_EXECUTE_TAG ("NTDIS", tmp_str = MEMfree (tmp_str));
            DBUG_EXECUTE_TAG ("NTDIS", DebugPrintDispatchInfo ("NTDIS", ires));

            /*
             * Now, we accumulate the ups and downs:
             */
            if (i == 0) {
                state = AllocDFT_state (IRES_NUMFUNS (ires));
                state = InsertFirstArgDFT_state (state, ires, lower);
            } else {
                state = InsertNextArgDFT_state (state, ires, lower);
            }

            DBUG_PRINT_TAG ("NTDIS", "accumulated ups and downs:");
            DBUG_EXECUTE_TAG ("NTDIS", DebugPrintDFT_state (state));

            fun = IRES_TYPE (ires);
            if (NTYPE_CON (fun) != TC_fun) {
                i = n;
            }
        }

        state = FinalizeDFT_state (state);

        DBUG_PRINT_TAG ("NTDIS", "final ups and downs:");
        DBUG_EXECUTE_TAG ("NTDIS", DebugPrintDFT_state (state));

        /*
         * Finally, we export our findings via a dft_res structure.
         * However, in case of 0 args (n==0), no dispatch has to be made
         * (since no overloading is allowed) so we return NULL!!
         */

        res = DFT_state2dft_res (state);

        res->type = fun; /* insert the result type */

        state = freeDFT_state (state);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   char * TYdft_res2DebugString( dft_res *dft)
 *
 * description:
 *
 ******************************************************************************/

char *
TYdft_res2DebugString (dft_res *dft)
{
    static str_buf *buf = NULL;
    int i;
    char *tmp_str;

    DBUG_ENTER ();

    if (buf == NULL) {
        buf = SBUFcreate (100);
    }
    if (dft == NULL) {
        buf = SBUFprintf (buf, "--");
    } else {
        if (dft->def) {
            tmp_str = TUtypeSignature2String (dft->def);
            buf = SBUFprintf (buf, "exact : (%s) ", tmp_str);
            tmp_str = MEMfree (tmp_str);
        }
        if (dft->deriveable) {
            tmp_str = TUtypeSignature2String (dft->deriveable);
            buf = SBUFprintf (buf, "deriveable : (%s) ", tmp_str);
            tmp_str = MEMfree (tmp_str);
        }
        if (dft->num_partials > 0) {
            buf = SBUFprintf (buf, "partials : ");
            for (i = 0; i < dft->num_partials; i++) {
                tmp_str = TUtypeSignature2String (dft->partials[i]);
                buf = SBUFprintf (buf, "%s ", tmp_str);
                tmp_str = MEMfree (tmp_str);
            }
        }
        if (dft->num_deriveable_partials > 0) {
            buf = SBUFprintf (buf, "deriveable_partials : ");
            for (i = 0; i < dft->num_deriveable_partials; i++) {
                tmp_str = TUtypeSignature2String (dft->deriveable_partials[i]);
                buf = SBUFprintf (buf, "%s ", tmp_str);
                tmp_str = MEMfree (tmp_str);
            }
        }

        if (SBUFisEmpty (buf)) {
            buf = SBUFprintf (buf, "no match!");
        }
    }

    tmp_str = SBUF2str (buf);
    SBUFflush (buf);

    DBUG_RETURN (tmp_str);
}

/******************************************************************************
 *
 * function:
 *   ntype * TYmakeAlphaType( ntype *maxtype)
 *
 * description:
 *  function for creating a not yet determined subtype of maxtype.
 *
 ******************************************************************************/

ntype *
TYmakeAlphaType (ntype *maxtype)
{
    ntype *res;
    tvar *alpha;

    DBUG_ENTER ();

    res = MakeNtype (TC_alpha, 0);
    alpha = SSImakeVariable ();
    if (maxtype != NULL) {
        SSInewMax (alpha, maxtype);
    }
    ALPHA_SSI (res) = alpha;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   tvar * TYgetAlpha( ntype *type)
 *
 * description:
 *  function for extracting the ssa variable from a type variable.
 *
 ******************************************************************************/

tvar *
TYgetAlpha (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NTYPE_CON (type) == TC_alpha,
                 "TYgetAlpha applied to non type variable!");

    DBUG_RETURN (ALPHA_SSI (type));
}

bool
TYcontainsAlpha (ntype *type)
{
    bool result = FALSE;
    size_t cnt;

    DBUG_ENTER ();

    if (type != NULL) {
        switch (NTYPE_CON (type)) {
        case TC_fun:
            for (cnt = 0; ((cnt < NTYPE_ARITY (type)) && !result); cnt++) {
                result = TYcontainsAlpha (NTYPE_SON (type, cnt));
            }
            break;

        case TC_ibase:
            result = TYcontainsAlpha (IBASE_GEN (type));
            if (!result) {
                result = TYcontainsAlpha (IBASE_SCAL (type));

                if (!result) {
                    result = TYcontainsAlpha (IBASE_IARR (type));
                }
            }
            break;

        case TC_iarr:
            result = TYcontainsAlpha (IARR_GEN (type));

            for (cnt = 0; ((cnt < NTYPE_ARITY (type) - 1) && !result); cnt++) {
                result = TYcontainsAlpha (IARR_IDIM (type, cnt));
            }
            break;

        case TC_idim:
            result = TYcontainsAlpha (IDIM_GEN (type));

            for (cnt = 0; ((cnt < NTYPE_ARITY (type) - 1) && !result); cnt++) {
                result = TYcontainsAlpha (IDIM_ISHAPE (type, cnt));
            }
            break;

        case TC_ishape:
            result = TYcontainsAlpha (ISHAPE_GEN (type));
            break;

        case TC_ires:
            result = TYcontainsAlpha (IRES_TYPE (type));
            break;

        case TC_prod:
            for (cnt = 0; ((cnt < NTYPE_ARITY (type)) && !result); cnt++) {
                result = TYcontainsAlpha (PROD_MEMBER (type, cnt));
            }
            break;

        case TC_union:
            for (cnt = 0; ((cnt < NTYPE_ARITY (type)) && !result); cnt++) {
                result = TYcontainsAlpha (UNION_MEMBER (type, cnt));
            }
            break;

        case TC_aud:
        case TC_audgz:
        case TC_akd:
        case TC_aks:
        case TC_akv:
            result = TYcontainsAlpha (TYgetScalar (type));
            break;

        case TC_simple:
        case TC_user:
        case TC_symbol:
            result = FALSE;
            break;

        case TC_alpha:
            result = TRUE;
            break;

        default:
            DBUG_UNREACHABLE ("found unhandeled type constructor!");
        }
    }

    DBUG_RETURN (result);
}

/***
 *** Functions inspecting types / matching on specific types:
 ***/

/******************************************************************************
 *
 * function:
 *    bool TYisSimple( ntype *type)
 *    bool TYisUser( ntype *type)
 *    bool TYisSymb( ntype *type)
 *    bool TYisPoly( ntype *type)
 *    bool TYisPolyUser( ntype *type)
 *    bool TYisScalar( ntype *type)
 *    bool TYisBottom( ntype *type)
 *    bool TYisAlpha( ntype *type)
 *    bool TYisFixedAlpha( ntype *type)
 *    bool TYisNonFixedAlpha( ntype *type)
 *    bool TYisAKV( ntype *type)
 *    bool TYisAKS( ntype *type)
 *    bool TYisAKD( ntype *type)
 *    bool TYisAUDGZ( ntype *type)
 *    bool TYisAUD( ntype *type)
 *    bool TYisArray( ntype *type)
 *    bool TYisArrayOrAlpha( ntype *type)
 *    bool TYisArrayOrFixedAlpha( ntype *type)
 *    bool TYisUnion( ntype *type)
 *    bool TYisProd( ntype *type)
 *    bool TYisFun( ntype *type)
 *
 * description:
 *  several predicate functions for inspecting the top level ntype!
 *
 ******************************************************************************/

bool
TYisSimple (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_simple);
}

bool
TYisUser (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_user);
}

bool
TYisPoly (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_poly);
}

bool
TYisPolyUser (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_polyuser);
}

bool
TYisSymb (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_symbol);
}

/*
 * I hope the author of TYisScalar provides some documentation
 * on what this predicate is intended to do.
 * It does NOT mean: "This is a rank-0 array."
 * For that, use TUisScalar( type).
 */

bool
TYisScalar (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_ARITY (type) == 0);
}

bool
TYisBottom (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_bottom);
}

bool
TYisAlpha (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_alpha);
}

bool
TYisFixedAlpha (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN ((NTYPE_CON (type) == TC_alpha) && SSIisFix (ALPHA_SSI (type)));
}

bool
TYisNonFixedAlpha (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN ((NTYPE_CON (type) == TC_alpha) && !SSIisFix (ALPHA_SSI (type)));
}

/* See bug #630. Constant N_array nodes are not AKV, but AKS.
 * If you need TYisAKV( ARRAY_ELEMTYPE( array), use the
 * IsFullyConstantNode( node *array) function in constant_folding.c
 */
bool
TYisAKV (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_akv);
}

bool
TYisAKS (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_aks);
}

bool
TYisAKD (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_akd);
}

bool
TYisAUD (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_aud);
}

bool
TYisAUDGZ (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_audgz);
}

bool
TYisArray (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN ((NTYPE_CON (type) == TC_aud) || (NTYPE_CON (type) == TC_audgz)
                 || (NTYPE_CON (type) == TC_akd) || (NTYPE_CON (type) == TC_aks)
                 || (NTYPE_CON (type) == TC_akv));
}

bool
TYisArrayOrAlpha (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (TYisArray (type) || (NTYPE_CON (type) == TC_alpha));
}

bool
TYisArrayOrFixedAlpha (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (TYisArray (type)
                 || ((NTYPE_CON (type) == TC_alpha) && SSIisFix (ALPHA_SSI (type))));
}

bool
TYisUnion (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_union);
}

bool
TYisProd (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_prod);
}

bool
TYisFun (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (NTYPE_CON (type) == TC_fun);
}

/******************************************************************************
 *
 * function:
 *    bool TYisAKSSymb( ntype *type)
 *    bool TYisAKSUdt( ntype *type)
 *    bool TYisProdOfArrayOrFixedAlpha( ntype *type)
 *    bool TYisProdOfAKV( ntype *type)
 *    bool TYisProdOfAKVafter( ntype *type, int 1)
 *    bool TYisProdContainingAKV( ntype *type)
 *    bool TYisProdOfArray( ntype *type)
 *
 * description:
 *   several predicate functions for checking particular nestings of
 *   type constructors
 *
 ******************************************************************************/

bool
TYisAKSSymb (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (
      ((NTYPE_CON (type) == TC_aks) && (NTYPE_CON (AKS_BASE (type)) == TC_symbol)));
}

bool
TYisAKSUdt (ntype *type)
{
    DBUG_ENTER ();
    DBUG_RETURN (
      ((NTYPE_CON (type) == TC_aks) && (NTYPE_CON (AKS_BASE (type)) == TC_user)));
}

bool
TYisProdOfArrayOrFixedAlpha (ntype *args)
{
    bool res = TRUE;
    ntype *arg;
    size_t i;

    DBUG_ENTER ();

    if (TYisProd (args)) {
        for (i = 0; i < TYgetProductSize (args); i++) {
            arg = TYgetProductMember (args, i);
            res = res && TYisArrayOrFixedAlpha (arg);
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
TYisProdOfArray (ntype *args)
{
    bool res = TRUE;
    ntype *arg;
    size_t i;

    DBUG_ENTER ();

    if (TYisProd (args)) {
        for (i = 0; i < TYgetProductSize (args); i++) {
            arg = TYgetProductMember (args, i);
            res = res && TYisArray (arg);
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
TYisProdOfAKV (ntype *args)
{
    bool res = TRUE;
    ntype *arg;
    size_t i;

    DBUG_ENTER ();

    if (TYisProd (args)) {
        for (i = 0; i < TYgetProductSize (args); i++) {
            arg = TYgetProductMember (args, i);
            res = res && TYisAKV (arg);
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
TYisProdOfAKVafter (ntype *args, size_t i)
{
    bool res = TRUE;
    ntype *arg;

    DBUG_ENTER ();

    if (TYisProd (args)) {
        for (; i < TYgetProductSize (args); i++) {
            arg = TYgetProductMember (args, i);
            res = res && TYisAKV (arg);
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
TYisProdContainingAKV (ntype *args)
{
    bool res = FALSE;
    ntype *arg;
    size_t i;

    DBUG_ENTER ();

    if (TYisProd (args)) {
        for (i = 0; i < TYgetProductSize (args); i++) {
            arg = TYgetProductMember (args, i);
            res = res || TYisAKV (arg);
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    int TYcountNonFixedAlpha( ntype *type)
 *
 * description:
 *   counts the number of non-fixed type variables contained in the type given.
 *
 ******************************************************************************/

int
TYcountNonFixedAlpha (ntype *type)
{
    int res = 0;
    size_t i, n;

    DBUG_ENTER ();

    if (TYisProd (type)) {
        n = TYgetProductSize (type);
        for (i = 0; i < n; i++) {
            res += TYcountNonFixedAlpha (TYgetProductMember (type, i));
        }
    } else {
        res += (TYisNonFixedAlpha (type) ? 1 : 0);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    int TYcountNoMinAlpha( ntype *type)
 *
 * description:
 *   counts the number of type variables that do not have a lower limit.
 *
 ******************************************************************************/

int
TYcountNoMinAlpha (ntype *type)
{
    int res = 0;
    size_t i, n;

    DBUG_ENTER ();

    if (TYisProd (type)) {
        n = TYgetProductSize (type);
        for (i = 0; i < n; i++) {
            res += TYcountNoMinAlpha (TYgetProductMember (type, i));
        }
    } else {
        res += (TYisAlpha (type) && (SSIgetMin (TYgetAlpha (type)) == NULL) ? 1 : 0);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *TYgetBottom( ntype *type)
 *
 *   @brief searches for bottom types contained in type
 *   @param type is the typeto be examined
 *   @return pointer to the first bottom type found, NULL otherwise
 *
 ******************************************************************************/

ntype *
TYgetBottom (ntype *type)
{
    ntype *res = NULL;
    size_t i, n;

    DBUG_ENTER ();

    if (TYisProd (type)) {
        n = TYgetProductSize (type);
        i = 0;
        while ((i < n) && (res == NULL)) {
            res = (TYisBottom (TYgetProductMember (type, i))
                       || TYisUser (TYgetProductMember (type, i))
                     ? TYgetProductMember (type, i)
                     : NULL);
            i++;
        }
    } else if (TYisBottom (type)) {
        res = type;
    } else {
        res = NULL;
    }

    DBUG_RETURN (res);
}

/***
 *** functions that check for the relationship of types:
 ***/

/******************************************************************************
 *
 * function:
 *    ct_res TYcmpTypes( ntype *t1, ntype *t2)
 *    bool   TYleTypes( ntype *t1, ntype *t2)
 *    bool   TYeqTypes( ntype *t1, ntype *t2)
 *
 * description:
 *
 *
 ******************************************************************************/

ct_res
TYcmpTypes (ntype *t1, ntype *t2)
{
#ifndef DBUG_OFF
    char *tmp_str = NULL, *tmp_str2 = NULL;
#endif

    size_t cnt;
    ct_res res = TY_dis;

    DBUG_ENTER ();

    DBUG_EXECUTE_TAG ("NTY_CMP", tmp_str = TYtype2DebugString (t1, FALSE, 0);
                      tmp_str2 = TYtype2DebugString (t2, FALSE, 0));
    DBUG_PRINT_TAG ("NTY_CMP", "comparing %s and %s", tmp_str, tmp_str2);
    DBUG_EXECUTE_TAG ("NTY_CMP", tmp_str = MEMfree (tmp_str);
                      tmp_str2 = MEMfree (tmp_str2));

    switch (NTYPE_CON (t1)) {
    case TC_prod:
        if ((NTYPE_CON (t2) == TC_prod) && (NTYPE_ARITY (t1) == NTYPE_ARITY (t2))) {
            res = TY_eq;
            for (cnt = 0; cnt < NTYPE_ARITY (t1); cnt++) {
                ct_res local = TYcmpTypes (NTYPE_SON (t1, cnt), NTYPE_SON (t2, cnt));
                switch (res) {
                case TY_eq:
                    res = local;
                    break;
                case TY_lt:
                    if ((local == TY_gt) || (local == TY_hcs)) {
                        res = TY_hcs;
                    } else if (local == TY_dis) {
                        res = TY_dis;
                    }
                    break;
                case TY_gt:
                    if ((local == TY_lt) || (local == TY_hcs)) {
                        res = TY_hcs;
                    } else if (local == TY_dis) {
                        res = TY_dis;
                    }
                    break;
                case TY_hcs:
                    if (local == TY_dis) {
                        res = TY_dis;
                    }
                    break;
                case TY_dis:
                    break;
                }
            }
        }
        break;
    case TC_bottom:
        if (NTYPE_CON (t2) == TC_bottom) {
            res = TY_eq;
        } else if (NTYPE_CON (t2) == TC_akv) {
            res = TY_hcs;
        } else {
            res = TY_lt;
        }
        break;
    case TC_simple:
        if ((NTYPE_CON (t2) == TC_simple) && (SIMPLE_TYPE (t1) == SIMPLE_TYPE (t2))) {
            res = TY_eq;
        } else if (NTYPE_CON (t2) == TC_bottom) {
            res = TY_gt;
        }
        break;
    case TC_symbol:
        if ((NTYPE_CON (t2) == TC_symbol) && (NSequals (SYMBOL_NS (t1), SYMBOL_NS (t2)))
            && STReq (SYMBOL_NAME (t1), SYMBOL_NAME (t2))) {
            res = TY_eq;
        } else if (NTYPE_CON (t2) == TC_bottom) {
            res = TY_gt;
        }
        break;
    case TC_user:
        if ((NTYPE_CON (t2) == TC_user) && UTeq (USER_TYPE (t1), USER_TYPE (t2))) {
            res = TY_eq;
        } else if (NTYPE_CON (t2) == TC_bottom) {
            res = TY_gt;
        }
        break;
    case TC_akv:
        switch (NTYPE_CON (t2)) {
        case TC_bottom:
            res = TY_hcs;
            break;
        case TC_akv:
            if (TYcmpTypes (AKV_BASE (t1), AKV_BASE (t2)) == TY_eq) {
                if (COcompareConstants (AKV_CONST (t1), AKV_CONST (t2))) {
                    res = TY_eq;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aks:
            if (TYcmpTypes (AKV_BASE (t1), AKS_BASE (t2)) == TY_eq) {
                if (SHcompareShapes (COgetShape (AKV_CONST (t1)), AKS_SHP (t2))) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_akd:
            if (TYcmpTypes (AKV_BASE (t1), AKD_BASE (t2)) == TY_eq) {
                if (TYgetDim (t1) == TYgetDim (t2)) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_audgz:
            if (TYcmpTypes (AKV_BASE (t1), AUDGZ_BASE (t2)) == TY_eq) {
                if (TYgetDim (t1) > 0) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aud:
            if (TYcmpTypes (AKV_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_lt;
            }
            break;
        default:
            break;
        }
        break;
    case TC_aks:
        switch (NTYPE_CON (t2)) {
        case TC_bottom:
            res = TY_gt;
            break;
        case TC_akv:
            if (TYcmpTypes (AKS_BASE (t1), AKV_BASE (t2)) == TY_eq) {
                if (SHcompareShapes (AKS_SHP (t1), COgetShape (AKV_CONST (t2)))) {
                    res = TY_gt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aks:
            if (TYcmpTypes (AKS_BASE (t1), AKS_BASE (t2)) == TY_eq) {
                if (SHcompareShapes (AKS_SHP (t1), AKS_SHP (t2))) {
                    res = TY_eq;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_akd:
            if (TYcmpTypes (AKS_BASE (t1), AKD_BASE (t2)) == TY_eq) {
                if (TYgetDim (t1) == TYgetDim (t2)) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_audgz:
            if (TYcmpTypes (AKS_BASE (t1), AUDGZ_BASE (t2)) == TY_eq) {
                if (TYgetDim (t1) > 0) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aud:
            if (TYcmpTypes (AKS_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_lt;
            }
            break;
        default:
            break;
        }
        break;
    case TC_akd:
        switch (NTYPE_CON (t2)) {
        case TC_bottom:
            res = TY_gt;
            break;
        case TC_akv:
        case TC_aks:
            if (TYcmpTypes (AKD_BASE (t1), AKS_BASE (t2)) == TY_eq) {
                if (TYgetDim (t1) == TYgetDim (t2)) {
                    res = TY_gt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_akd:
            if (TYcmpTypes (AKD_BASE (t1), AKD_BASE (t2)) == TY_eq) {
                if (TYgetDim (t1) == TYgetDim (t2)) {
                    res = TY_eq;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_audgz:
            if (TYcmpTypes (AKD_BASE (t1), AUDGZ_BASE (t2)) == TY_eq) {
                if (TYgetDim (t1) > 0) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aud:
            if (TYcmpTypes (AKD_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_lt;
            }
            break;
        default:
            break;
        }
        break;
    case TC_audgz:
        switch (NTYPE_CON (t2)) {
        case TC_bottom:
            res = TY_gt;
            break;
        case TC_akv:
        case TC_aks:
        case TC_akd:
            if (TYcmpTypes (AUDGZ_BASE (t1), TYgetScalar (t2)) == TY_eq) {
                if (TYgetDim (t2) > 0) {
                    res = TY_gt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_audgz:
            if ((TYcmpTypes (AUDGZ_BASE (t1), AUDGZ_BASE (t2)) == TY_eq)) {
                res = TY_eq;
            }
            break;
        case TC_aud:
            if (TYcmpTypes (AUDGZ_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_lt;
            }
            break;
        default:
            break;
        }
        break;
    case TC_aud:
        switch (NTYPE_CON (t2)) {
        case TC_bottom:
            res = TY_gt;
            break;
        case TC_akv:
        case TC_aks:
        case TC_akd:
        case TC_audgz:
            if (TYcmpTypes (AUD_BASE (t1), TYgetScalar (t2)) == TY_eq) {
                res = TY_gt;
            }
            break;
        case TC_aud:
            if (TYcmpTypes (AUD_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_eq;
            }
            break;
        default:
            break;
        }
        break;
    default:
        DBUG_UNREACHABLE ("Type comparison for non-array types not yet implemented!");
    }

    DBUG_PRINT_TAG ("NTY_CMP", "result: %d", res);
    DBUG_RETURN (res);
}

bool
TYeqTypes (ntype *t1, ntype *t2)
{
    DBUG_ENTER ();
    DBUG_RETURN (TYcmpTypes (t1, t2) == TY_eq);
}

bool
TYleTypes (ntype *t1, ntype *t2)
{
    ct_res cmp;

    DBUG_ENTER ();
    cmp = TYcmpTypes (t1, t2);
    DBUG_RETURN ((cmp == TY_eq) || (cmp == TY_lt));
}

/******************************************************************************
 *
 * function:
 *    ntype * TYlubOfTypes( ntype *t1, ntype *t2)
 *
 * description:
 *    computes the least upper bound of types t1 and t2. In case it does not
 *    exist, NULL is returned.
 *
 ******************************************************************************/

ntype *
TYlubOfTypes (ntype *t1, ntype *t2)
{
    ntype *res, *new_t1;

    DBUG_ENTER ();

    switch (TYcmpTypes (t1, t2)) {
    case TY_eq:
        if (TYisBottom (t1)) {
            res = TUcombineBottom (t1, t2);
        } else {
            res = TYcopyType (t1);
        }
        break;
    case TY_lt:
        res = TYcopyType (t2);
        break;
    case TY_gt:
        res = TYcopyType (t1);
        break;
    case TY_hcs:
        switch (NTYPE_CON (t1)) {
        case TC_bottom:
            /**
             * we need to increase the non-bottom type. Hence, we switch
             * the arguments.
             */
            res = TYlubOfTypes (t2, t1);
            break;
        case TC_akv:
            new_t1 = TYmakeAKS (TYcopyType (AKV_BASE (t1)),
                                SHcopyShape (COgetShape (AKV_CONST (t1))));
            res = TYlubOfTypes (new_t1, t2);
            new_t1 = TYfreeType (new_t1);
            break;
        case TC_aks:
            if (SHgetDim (AKS_SHP (t1)) == 0) {
                new_t1 = TYmakeAUD (TYcopyType (AKS_BASE (t1)));
            } else {
                new_t1 = TYmakeAKD (TYcopyType (AKS_BASE (t1)), SHgetDim (AKS_SHP (t1)),
                                    SHcreateShape (0));
            }
            res = TYlubOfTypes (new_t1, t2);
            new_t1 = TYfreeType (new_t1);
            break;
        case TC_akd:
            if (AKD_DOTS (t1) == 0) {
                new_t1 = TYmakeAUD (TYcopyType (AKD_BASE (t1)));
            } else {
                new_t1 = TYmakeAUDGZ (TYcopyType (AKD_BASE (t1)));
            }
            res = TYlubOfTypes (new_t1, t2);
            new_t1 = TYfreeTypeConstructor (new_t1);
            break;
        case TC_audgz:
            new_t1 = TYmakeAUD (AUDGZ_BASE (t1));
            res = TYlubOfTypes (new_t1, t2);
            new_t1 = TYfreeTypeConstructor (new_t1);
            break;
        case TC_aud:
            DBUG_UNREACHABLE ("Cannot compute LUB!");
            res = NULL;
            break;
        default:
            DBUG_UNREACHABLE ("Cannot compute LUB!");
            res = NULL;
            break;
        }
        break;
    case TY_dis:
        res = NULL;
        break;
    default:
        res = NULL;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYeliminateAlpha( ntype *t1)
 *
 * description:
 *    if t1 is a type variable with identical upper and lower bound the
 *    boundary type is returned, otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYeliminateAlpha (ntype *t1)
{
    ntype *res;
    size_t i;

    DBUG_ENTER ();

    if (TYisProd (t1)) {
        res = MakeNtype (TC_prod, NTYPE_ARITY (t1));
        for (i = 0; i < NTYPE_ARITY (t1); i++) {
            PROD_MEMBER (res, i) = TYeliminateAlpha (PROD_MEMBER (t1, i));
        }
    } else {
        if (TYisFixedAlpha (t1)) {
            res = TYcopyType (SSIgetMin (ALPHA_SSI (t1)));
        } else {
            res = TYcopyType (t1);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYfixAndEliminateAlpha( ntype *t1)
 *
 * description:
 *    if t1 is a type variable with a lower bound the lower bound
 *    is returned, otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYfixAndEliminateAlpha (ntype *t1)
{
    ntype *res;

    DBUG_ENTER ();

    if (t1 == NULL) {
        res = t1;
    } else if (TYisAlpha (t1)) {
        if (SSIgetMin (TYgetAlpha (t1)) != NULL) {
            res = TYcopyType (SSIgetMin (ALPHA_SSI (t1)));
            DBUG_PRINT_TAG ("SSIMEM", "fixing var at %p", (void *)TYgetAlpha (t1));
        } else {
            res = TYcopyType (t1);
            DBUG_PRINT_TAG ("SSIMEM", "copying var at %p to %p", (void *)TYgetAlpha (t1),
                            (void *)TYgetAlpha (res));
        }
    } else {
        size_t cnt;

        res = TYcopyTypeConstructor (t1);

        res = IncreaseArity (res, NTYPE_ARITY (t1));

        for (cnt = 0; cnt < NTYPE_ARITY (t1); cnt++) {
            NTYPE_SON (res, cnt) = TYfixAndEliminateAlpha (NTYPE_SON (t1, cnt));
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYliftBottomFixAndEliminateAlpha( ntype *t1)
 *
 * description:
 *    if t1 is a type variable with a lower bound the lower bound
 *    is returned, otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYliftBottomFixAndEliminateAlpha (ntype *t1)
{
    ntype *res;
    ntype *min;

    DBUG_ENTER ();

    if (t1 == NULL) {
        res = t1;
    } else if (TYisAlpha (t1)) {
        min = SSIgetMin (TYgetAlpha (t1));
        if (min != NULL) {
            if (TYisBottom (min)) {
                res = TYcopyType (SSIgetMax (ALPHA_SSI (t1)));
                DBUG_ASSERT (res != NULL, "TYliftBottomFixAndEliminateAlpha applied to "
                                          "alpha wo upper bound");
            } else {
                res = TYcopyType (SSIgetMin (ALPHA_SSI (t1)));
            }
            DBUG_PRINT_TAG ("SSIMEM", "fixing var at %p", (void *)TYgetAlpha (t1));
        } else {
            res = TYcopyType (t1);
            DBUG_PRINT_TAG ("SSIMEM", "copying var at %p to %p", (void *)TYgetAlpha (t1),
                            (void *)TYgetAlpha (res));
        }
    } else {
        size_t cnt;

        res = TYcopyTypeConstructor (t1);

        res = IncreaseArity (res, NTYPE_ARITY (t1));

        for (cnt = 0; cnt < NTYPE_ARITY (t1); cnt++) {
            NTYPE_SON (res, cnt) = TYliftBottomFixAndEliminateAlpha (NTYPE_SON (t1, cnt));
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYeliminateUser( ntype *t1)
 *
 * description:
 *    if t1 is a user defined type, its base type is returned;
 *    otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYeliminateUser (ntype *t1)
{
    ntype *res;
    size_t i;
    usertype udt;

    DBUG_ENTER ();

    if (TYisProd (t1)) {
        res = MakeNtype (TC_prod, NTYPE_ARITY (t1));
        for (i = 0; i < NTYPE_ARITY (t1); i++) {
            PROD_MEMBER (res, i) = TYeliminateUser (PROD_MEMBER (t1, i));
        }
    } else {
        if (TYisArray (t1) && TYisUser (TYgetScalar (t1))) {
            /*
             * we have to dealias the type here, as we are interested
             * in the real implementation type. Otherwise, external
             * types (aka 'real hiddens') would be hidden behind the
             * alias udt!
             */
            udt = UTgetUnAliasedType (USER_TYPE (TYgetScalar (t1)));
            res = TYnestTypes (t1, UTgetBaseType (udt));
        } else {
            res = TYcopyType (t1);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype * TYeliminateAKV( ntype *t1)
 *
 *   @brief if t1 is a AKV type, the respective AKS type is returned;
 *          otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYeliminateAKV (ntype *t1)
{
    ntype *res;
    size_t i;

    DBUG_ENTER ();

    if (TYisProd (t1)) {
        res = MakeNtype (TC_prod, NTYPE_ARITY (t1));
        for (i = 0; i < NTYPE_ARITY (t1); i++) {
            PROD_MEMBER (res, i) = TYeliminateAKV (PROD_MEMBER (t1, i));
        }
    } else {
        if (TYisAKV (t1)) {
            res = TYmakeAKS (TYcopyType (TYgetScalar (t1)),
                             SHcopyShape (COgetShape (AKV_CONST (t1))));
        } else {
            res = TYcopyType (t1);
        }
    }

    DBUG_RETURN (res);
}

/***
 *** functions for handling types in general:
 ***/

/******************************************************************************
 *
 * function:
 *    ntype * TYfreeTypeConstructor( ntype *type)
 *    ntype * TYfreeType( ntype *type)
 *
 * description:
 *   functions for freeing types. While TYfreeTypeConstructor only frees the
 *   topmost type constructor, TYfreeType frees the entire type including all
 *   sons!
 *
 ******************************************************************************/

ntype *
TYfreeTypeConstructor (ntype *type)
{
    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "argument is NULL");

    switch (NTYPE_CON (type)) {
    case TC_bottom:
        BOTTOM_MSG (type) = MEMfree (BOTTOM_MSG (type));
        break;
    case TC_symbol:
        SYMBOL_NS (type) = NSfreeNamespace (SYMBOL_NS (type));
        SYMBOL_NAME (type) = MEMfree (SYMBOL_NAME (type));
        break;
    case TC_akv:
        AKV_CONST (type) = COfreeConstant (AKV_CONST (type));
        break;
    case TC_aks:
        AKS_SHP (type) = SHfreeShape (AKS_SHP (type));
        break;
    case TC_akd:
        AKD_SHP (type) = SHfreeShape (AKD_SHP (type));
        break;
    case TC_ibase:
        IBASE_BASE (type) = TYfreeType (IBASE_BASE (type));
        break;
    case TC_ishape:
        ISHAPE_SHAPE (type) = SHfreeShape (ISHAPE_SHAPE (type));
        break;
    case TC_poly:
        POLY_NAME (type) = MEMfree (POLY_NAME (type));
        break;
    case TC_polyuser:
        POLYUSER_OUTER (type) = MEMfree (POLYUSER_OUTER (type));
        POLYUSER_INNER (type) = MEMfree (POLYUSER_INNER (type));
        POLYUSER_SHAPE (type) = MEMfree (POLYUSER_SHAPE (type));
        break;
    case TC_alpha:
        /* type variables are never freed since they are used in sharing! */
    case TC_fun:
    case TC_iarr:
    case TC_idim:
    case TC_ires:
    case TC_aud:
    case TC_audgz:
    case TC_union:
    case TC_prod:
    case TC_simple:
    case TC_user:
        break;
    default:
        DBUG_UNREACHABLE ("trying to free illegal type constructor!");
    }
    if (NTYPE_CON (type) == TC_simple) {
        type = NULL;
    } else {
        type = MEMfree (type);
    }

    DBUG_RETURN (type);
}

ntype *
TYfreeType (ntype *type)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "argument is NULL");

    for (i = 0; i < NTYPE_ARITY (type); i++) {
        if (NTYPE_SON (type, i) != NULL) {
            NTYPE_SON (type, i) = TYfreeType (NTYPE_SON (type, i));
        }
    }
    if (NTYPE_SONS (type) != NULL) {
        NTYPE_SONS (type) = MEMfree (NTYPE_SONS (type));
    }
    type = TYfreeTypeConstructor (type);

    DBUG_RETURN (type);
}

/******************************************************************************
 *
 * function:
 *    void TYtouchTypeConstructor( ntype *type, info *arg_info)
 *    void TYtouchType( ntype *type, info *arg_info)
 *
 * description:
 *   functions for touching types. While TYtouchTypeConstructor only touch the
 *   topmost type constructor, TYtouchType touch the entire type including all
 *   sons!
 *
 ******************************************************************************/

void
TYtouchTypeConstructor (ntype *type, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "argument is NULL");

    switch (NTYPE_CON (type)) {
    case TC_bottom:
        CHKMtouch (BOTTOM_MSG (type), arg_info);
        break;
    case TC_symbol:
        NStouchNamespace (SYMBOL_NS (type), arg_info);
        CHKMtouch (SYMBOL_NAME (type), arg_info);
        break;
    case TC_akv:
        COtouchConstant (AKV_CONST (type), arg_info);
        break;
    case TC_aks:
        SHtouchShape (AKS_SHP (type), arg_info);
        break;
    case TC_akd:
        SHtouchShape (AKD_SHP (type), arg_info);
        break;
    case TC_ibase:
        TYtouchType (IBASE_BASE (type), arg_info);
        break;
    case TC_ishape:
        SHtouchShape (ISHAPE_SHAPE (type), arg_info);
        break;
    case TC_alpha:
        /* type variables are never freed since they are used in sharing! */
    case TC_fun:
    case TC_iarr:
    case TC_idim:
    case TC_ires:
    case TC_aud:
    case TC_audgz:
    case TC_union:
    case TC_prod:
    case TC_simple:
    case TC_user:
        break;
    default:
        DBUG_UNREACHABLE ("trying to free illegal type constructor!");
    }
    CHKMtouch (type, arg_info);

    DBUG_RETURN ();
}

void
TYtouchType (ntype *type, info *arg_info)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "argument is NULL");

    for (i = 0; i < NTYPE_ARITY (type); i++) {
        if (NTYPE_SON (type, i) != NULL) {
            TYtouchType (NTYPE_SON (type, i), arg_info);
        }
    }

    if (NTYPE_SONS (type) != NULL) {
        CHKMtouch (NTYPE_SONS (type), arg_info);
    }

    TYtouchTypeConstructor (type, arg_info);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *    ntype * TYcopyType( ntype *type)
 *    ntype * TYcopyFixedType( ntype *type)
 *    ntype * TYDeriveSubtype( ntype *type)
 *
 * description:
 *    all these function copy types entirely! That does not only include
 *    copying the attributes but it includes copying the sons as well!
 *    The only difference of them is the way type variables are treated.
 *    - TYcopyType copies type variables as they are, i.e., they point to
 *      the same tvar structure, whereas
 *    - TYcopyFixedType does not copy type variables at all but inserts
 *      NULL pointers instead, and
 *    - TYDeriveSubtype creates a new tvar structure with the same upper
 *      bound!
 *    Therefore, all these functions are just wrappers for a static function
 *
 *          ntype * CopyType( ntype *type, TV_treatment new_tvars)
 *
 ******************************************************************************/

typedef enum {
    tv_id,  /* make a 1:1 copy */
    tv_sub, /* create a subtype */
    tv_none /* do not copy at all */
} TV_treatment;

static ntype *
CopyTypeConstructor (ntype *type, TV_treatment new_tvars)
{
    ntype *res;
    tvar *alpha;
    size_t i;
    bool ok;

    DBUG_ENTER ();

    if (type == NULL) {
        res = NULL;
    } else {
        /*
         * First, we copy the type node itself!
         */
        res = MakeNtype (NTYPE_CON (type), 0);

        /*
         * Then we copy the attributes:
         */
        switch (NTYPE_CON (type)) {
        case TC_bottom:
            BOTTOM_MSG (res) = STRcpy (BOTTOM_MSG (type));
            break;
        case TC_simple:
            SIMPLE_TYPE (res) = SIMPLE_TYPE (type);
            SIMPLE_HIDDEN_UDT (res) = SIMPLE_HIDDEN_UDT (type);
            break;
        case TC_symbol:
            SYMBOL_NS (res) = NSdupNamespace (SYMBOL_NS (type));
            SYMBOL_NAME (res) = STRcpy (SYMBOL_NAME (type));
            break;
        case TC_poly:
            POLY_NAME (res) = STRcpy (POLY_NAME (type));
            break;
        case TC_user:
            USER_TYPE (res) = USER_TYPE (type);
            break;
        case TC_akv:
            AKV_CONST (res) = COcopyConstant (AKV_CONST (type));
            break;
        case TC_aks:
            AKS_SHP (res) = SHcopyShape (AKS_SHP (type));
            break;
        case TC_akd:
            AKD_SHP (res) = SHcopyShape (AKD_SHP (type));
            AKD_DOTS (res) = AKD_DOTS (type);
            break;
        case TC_ibase:
            IBASE_BASE (res) = TYcopyType (IBASE_BASE (type));
            break;
        case TC_idim:
            IDIM_DIM (res) = IDIM_DIM (type);
            break;
        case TC_ishape:
            ISHAPE_SHAPE (res) = SHcopyShape (ISHAPE_SHAPE (type));
            break;
        case TC_ires:
            IRES_NUMFUNS (res) = IRES_NUMFUNS (type);
            if (IRES_NUMFUNS (type) != 0) {
                IRES_FUNDEFS (res)
                  = (node **)MEMmalloc (IRES_NUMFUNS (type) * sizeof (node *));
                IRES_POSS (res) = (int *)MEMmalloc (IRES_NUMFUNS (type) * sizeof (int));
                for (i = 0; i < IRES_NUMFUNS (type); i++) {
                    IRES_FUNDEF (res, i) = IRES_FUNDEF (type, i);
                    IRES_POS (res, i) = IRES_POS (type, i);
                }
            } else {
                IRES_FUNDEFS (res) = NULL;
                IRES_POSS (res) = NULL;
            }
            break;
        case TC_alpha:
            switch (new_tvars) {
            case tv_id:
                ALPHA_SSI (res) = ALPHA_SSI (type);
                break;
            case tv_sub:
                alpha = SSImakeVariable ();
                SSInewMax (alpha, TYcopyType (SSIgetMax (ALPHA_SSI (type))));
                ALPHA_SSI (res) = alpha;
                ok = SSInewRel (alpha, ALPHA_SSI (type));
                DBUG_ASSERT (ok, "SSInewRel did not work in TYDeriveSubtype");
                break;
            case tv_none:
                res = MEMfree (res);
                break;
            }
            break;
        default:
            break;
        }
    }

    if (res != NULL) {
        res = TYsetUnique (res, TYisUnique (type));
        res = TYsetDistributed (res, TYgetDistributed (type));
    }

    DBUG_RETURN (res);
}

ntype *
TYcopyTypeConstructor (ntype *type)
{
    ntype *res;

    DBUG_ENTER ();

    res = CopyTypeConstructor (type, tv_id);

    DBUG_RETURN (res);
}

ntype *
TYcopyType (ntype *type)
{
    ntype *res;
    size_t i;
#ifndef DBUG_OFF
    size_t mem_entry = 0;
#endif

    DBUG_ENTER ();

    DBUG_EXECUTE_TAG ("NTY_MEM", mem_entry = global.current_allocated_mem);

    res = CopyTypeConstructor (type, tv_id);
    if (res != NULL) {
        NTYPE_ARITY (res) = NTYPE_ARITY (type);
        NTYPE_SONS (res) = (ntype **)MEMmalloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < NTYPE_ARITY (res); i++) {
            NTYPE_SON (res, i) = TYcopyType (NTYPE_SON (type, i));
        }
    }

    DBUG_PRINT_TAG ("NTY_MEM", "size of type copied by TYcopyType: %zu",
                    global.current_allocated_mem - mem_entry);

    if (res != NULL) {
        res = TYsetMutcScope (res, TYgetMutcScope (type));
    }

    DBUG_RETURN (res);
}

ntype *
TYcopyFixedType (ntype *type)
{
    ntype *res;
    size_t i;

    DBUG_ENTER ();

    res = CopyTypeConstructor (type, tv_none);
    if (res != NULL) {
        NTYPE_ARITY (res) = NTYPE_ARITY (type);
        NTYPE_SONS (res) = (ntype **)MEMmalloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < NTYPE_ARITY (res); i++) {
            NTYPE_SON (res, i) = TYcopyFixedType (NTYPE_SON (type, i));
        }
    }

    DBUG_RETURN (res);
}

ntype *
TYDeriveSubtype (ntype *type)
{
    ntype *res;
    size_t i;

    DBUG_ENTER ();

    res = CopyTypeConstructor (type, tv_sub);
    if (res != NULL) {
        NTYPE_ARITY (res) = NTYPE_ARITY (type);
        NTYPE_SONS (res) = (ntype **)MEMmalloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < NTYPE_ARITY (res); i++) {
            NTYPE_SON (res, i) = TYDeriveSubtype (NTYPE_SON (type, i));
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    char * TYtype2String( ntype *type, bool multiline, int offset)
 *
 * description:
 *   iff "multiline" is TRUE, strings for function types contain new line
 *   symbols. Each new line is followed by "offset" preceeding blanks.
 *   This function is implemented by means of two local helper functions
 *   - ArrayType2String       for printing array types, and
 *   - FunType2String         for printing function types
 *   - ScalarType2String      for printing scalar types
 *
 ******************************************************************************/

static char *
ScalarType2String (ntype *type)
{
    static str_buf *buf = NULL;
    char *res;
    usertype udt;

    DBUG_ENTER ();

    if (buf == NULL) {
        buf = SBUFcreate (64);
    }

    switch (NTYPE_CON (type)) {
    case TC_simple:
        if (SIMPLE_TYPE (type) == T_hidden) {
            udt = SIMPLE_HIDDEN_UDT (type);
            if (udt == UT_NOT_DEFINED) {
                buf = SBUFprintf (buf, "hidden");
            } else {
                if (UTgetNamespace (udt) == NULL) {
                    buf = SBUFprintf (buf, "enclosed(%s)", UTgetName (udt));
                } else {
                    buf = SBUFprintf (buf, "enclosed(%s::%s)",
                                           NSgetName (UTgetNamespace (udt)),
                                           UTgetName (udt));
                }
            }
        } else {
            buf = SBUFprintf (buf, "%s", global.mdb_type[SIMPLE_TYPE (type)]);
        }
        break;
    case TC_symbol:
        if (SYMBOL_NS (type) == NULL) {
            buf = SBUFprintf (buf, "%s", SYMBOL_NAME (type));
        } else {
            buf = SBUFprintf (buf, "%s::%s", NSgetName (SYMBOL_NS (type)),
                              SYMBOL_NAME (type));
        }
        break;
    case TC_user:
        if (UTgetNamespace (USER_TYPE (type)) == NULL) {
            buf = SBUFprintf (buf, "%s", UTgetName (USER_TYPE (type)));
        } else {
            buf
              = SBUFprintf (buf, "%s::%s", NSgetName (UTgetNamespace (USER_TYPE (type))),
                            UTgetName (USER_TYPE (type)));
        }
        break;
    case TC_poly:
        buf = SBUFprintf (buf, "<%s>", POLY_NAME (type));
        break;
    case TC_polyuser:
        buf = SBUFprintf (buf, "<%s%s%s[%s]>", POLYUSER_OUTER (type),
                          POLYUSER_DENEST (type) ? "->"
                                                 : (POLYUSER_RENEST (type) ? "<-" : "="),
                          POLYUSER_INNER (type), POLYUSER_SHAPE (type));
        break;
    default:
        DBUG_UNREACHABLE ("ScalarType2String called with non-scalar type!");
    }

    res = SBUF2str (buf);
    SBUFflush (buf);

    DBUG_RETURN (res);
}

static char *
ArrayType2String (ntype *type)
{
    static str_buf *buf = NULL;
    char *tmp_str;

    DBUG_ENTER ();

    if (buf == NULL) {
        buf = SBUFcreate (128);
    }

    DBUG_ASSERT (type, "ArrayType2String called with NULL!");
    DBUG_ASSERT (TYisArray (type), "ArrayType2String called with non-array type!");

    tmp_str = ScalarType2String (AKS_BASE (type));
    buf = SBUFprint (buf, tmp_str);
    tmp_str = MEMfree (tmp_str);

    if (TYisUnique (type)) {
        buf = SBUFprintf (buf, "!");
    }

    switch (NTYPE_CON (type)) {
    case TC_akv:
        if (TYgetDim (type) > 0) {
            tmp_str = SHshape2String (0, COgetShape (AKV_CONST (type)));
            buf = SBUFprint (buf, tmp_str);
            tmp_str = MEMfree (tmp_str);
        }
        tmp_str = COconstantData2String (3, AKV_CONST (type));
        buf = SBUFprintf (buf, "{%s}", tmp_str);
        tmp_str = MEMfree (tmp_str);
        break;
    case TC_aks:
        if (TYgetDim (type) > 0) {
            tmp_str = SHshape2String (0, AKS_SHP (type));
            buf = SBUFprint (buf, tmp_str);
            tmp_str = MEMfree (tmp_str);
        }
        break;
    case TC_akd:
        tmp_str = SHshape2String (AKD_DOTS (type), AKD_SHP (type));
        buf = SBUFprint (buf, tmp_str);
        tmp_str = MEMfree (tmp_str);
        break;
    case TC_audgz:
        buf = SBUFprintf (buf, "[+]");
        break;
    case TC_aud:
        buf = SBUFprintf (buf, "[*]");
        break;
    default:
        DBUG_UNREACHABLE ("ArrayType2String called with non-array type!");
    }

    tmp_str = SBUF2str (buf);
    SBUFflush (buf);

    DBUG_RETURN (tmp_str);
}

static str_buf *
PrintFunSep (str_buf *buf, bool multiline, size_t offset)
{
    DBUG_ENTER ();
    if (multiline) {
        buf = SBUFprintf (buf, ",\n%*s", offset, "");
    } else {
        buf = SBUFprintf (buf, ", ");
    }
    DBUG_RETURN (buf);
}

static char *
FunType2String (ntype *type, char *scal_str, bool multiline, size_t offset)
{
    str_buf *buf;
    char *tmp_str, *shp_str;
    shape *empty_shape;
    size_t i;
    size_t scal_len = 0;
    bool sep_needed = FALSE;

    DBUG_ENTER ();

    buf = SBUFcreate (4096);
    switch (NTYPE_CON (type)) {
    case TC_fun:
        buf = SBUFprintf (buf, "{ ");
        offset += 2;
        for (i = 0; i < NTYPE_ARITY (type); i++) {
            if (NTYPE_SON (type, i) != NULL) {
                tmp_str
                  = FunType2String (NTYPE_SON (type, i), scal_str, multiline, offset);
                if (sep_needed) {
                    buf = PrintFunSep (buf, multiline, offset);
                }
                buf = SBUFprint (buf, tmp_str);
                tmp_str = MEMfree (tmp_str);
                sep_needed = TRUE;
            }
        }
        buf = SBUFprintf (buf, "}");
        break;

    case TC_ibase:
        DBUG_ASSERT (IBASE_GEN (type), "fun type without generic instance!");
        DBUG_ASSERT (scal_str == NULL,
                     "FunType2String called on ibase with non NULL scal_str!");

        scal_str = ScalarType2String (IBASE_BASE (type));
        scal_len = STRlen (scal_str);

        /*
         * print "<scal_str>[*]" instance:
         */
        buf = SBUFprintf (buf, "%s[*]", scal_str);
        tmp_str
          = FunType2String (IBASE_GEN (type), scal_str, multiline, offset + scal_len + 3);
        buf = SBUFprint (buf, tmp_str);
        tmp_str = MEMfree (tmp_str);

        /*
         * print "<scal_str>[]" instance:
         */
        if (IBASE_SCAL (type)) {
            tmp_str = FunType2String (IBASE_SCAL (type), scal_str, multiline,
                                      offset + scal_len);
            buf = PrintFunSep (buf, multiline, offset);
            buf = SBUFprint (buf, scal_str);
            buf = SBUFprint (buf, tmp_str);
            tmp_str = MEMfree (tmp_str);
        }

        /*
         * Print TC_iarr
         */
        if (IBASE_IARR (type)) {
            tmp_str = FunType2String (IBASE_IARR (type), scal_str, multiline, offset);
            buf = SBUFprint (buf, tmp_str);
            tmp_str = MEMfree (tmp_str);
        }

        scal_str = MEMfree (scal_str);
        break;

    case TC_iarr:
        /*
         * print "<scal_str>[+]" instance:
         */
        if (IARR_GEN (type)) {
            scal_len = STRlen (scal_str);
            tmp_str = FunType2String (IBASE_GEN (type), scal_str, multiline,
                                      offset + scal_len + 3);
            buf = PrintFunSep (buf, multiline, offset);
            buf = SBUFprintf (buf, "%s[+]", scal_str);
            buf = SBUFprint (buf, tmp_str);
            tmp_str = MEMfree (tmp_str);
        }

        /*
         * Print TC_idims
         */

        for (i = 0; i < (NTYPE_ARITY (type) - 1); i++) {
            tmp_str = FunType2String (IARR_IDIM (type, i), scal_str, multiline, offset);
            buf = SBUFprint (buf, tmp_str);
            tmp_str = MEMfree (tmp_str);
        }

        break;

    case TC_idim:
        /*
         * print "<scal_str>[<dots>]" instance:
         */
        if (IDIM_GEN (type)) {
            empty_shape = SHmakeShape (0);
            shp_str = SHshape2String (IDIM_DIM (type), empty_shape);
            empty_shape = SHfreeShape (empty_shape);

            tmp_str = FunType2String (IDIM_GEN (type), scal_str, multiline,
                                      offset + STRlen (scal_str) + STRlen (shp_str));
            buf = PrintFunSep (buf, multiline, offset);
            buf = SBUFprintf (buf, "%s%s", scal_str, shp_str);
            buf = SBUFprint (buf, tmp_str);
            shp_str = MEMfree (shp_str);
            tmp_str = MEMfree (tmp_str);
        }

        /*
         * Print TC_ishapes
         */

        for (i = 0; i < (NTYPE_ARITY (type) - 1); i++) {
            tmp_str = FunType2String (IDIM_ISHAPE (type, i), scal_str, multiline, offset);
            buf = SBUFprint (buf, tmp_str);
            tmp_str = MEMfree (tmp_str);
        }

        break;

    case TC_ishape:
        /*
         * print "<scal_str>[<shp>]" instance:
         */
        if (ISHAPE_GEN (type)) {
            shp_str = SHshape2String (0, ISHAPE_SHAPE (type));

            tmp_str = FunType2String (IDIM_GEN (type), scal_str, multiline,
                                      offset + STRlen (scal_str) + STRlen (shp_str));
            buf = PrintFunSep (buf, multiline, offset);
            buf = SBUFprintf (buf, "%s%s", scal_str, shp_str);
            buf = SBUFprint (buf, tmp_str);
            shp_str = MEMfree (shp_str);
            tmp_str = MEMfree (tmp_str);
        }

        break;

    case TC_ires:
        offset += 4;
        tmp_str = TYtype2String (IRES_TYPE (type), multiline, offset);
        buf = SBUFprintf (buf, " -> ");
        buf = SBUFprint (buf, tmp_str);
        tmp_str = MEMfree (tmp_str);
        break;
    default:
        DBUG_UNREACHABLE ("FunType2String called with non-legal type!");
        break;
    }

    tmp_str = SBUF2str (buf);
    buf = SBUFfree (buf);

    DBUG_RETURN (tmp_str);
}

char *
TYtype2String (ntype *type, bool multiline, size_t offset)
{
    str_buf *buf;
    char *tmp_str, *res;
    size_t i;

    DBUG_ENTER ();

    if (type == NULL) {
        res = STRcpy ("--");
    } else {

        switch (NTYPE_CON (type)) {
        case TC_bottom:
            res = STRcpy ("_|_");
            break;
        case TC_aud:
        case TC_audgz:
        case TC_akd:
        case TC_aks:
        case TC_akv:
            res = ArrayType2String (type);
            break;
        case TC_fun:
            res = FunType2String (type, NULL, multiline, offset);
            break;
        case TC_prod:
            buf = SBUFcreate (256);
            buf = SBUFprintf (buf, "(");
            if (NTYPE_ARITY (type) > 0) {
                tmp_str = TYtype2String (NTYPE_SON (type, 0), multiline, offset);
                buf = SBUFprintf (buf, " %s", tmp_str);
                tmp_str = MEMfree (tmp_str);
                for (i = 1; i < NTYPE_ARITY (type); i++) {
                    tmp_str = TYtype2String (NTYPE_SON (type, i), multiline, offset);
                    buf = SBUFprintf (buf, ", %s", tmp_str);
                    tmp_str = MEMfree (tmp_str);
                }
            }
            buf = SBUFprintf (buf, ")");
            res = SBUF2str (buf);
            buf = SBUFfree (buf);
            break;
        case TC_alpha:
            res = SSIvariable2DebugString (ALPHA_SSI (type));
            break;
        case TC_user:
            res = ScalarType2String (type);
            break;
        default:
            DBUG_UNREACHABLE ("TYtype2String applied to non-SAC type!");
            res = NULL;
            break;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    char * TYtype2DebugString( ntype *type, bool multiline, int offset)
 *
 * description:
 *   constructs a string that represents the internal type constructor
 *   structure of the type!
 *   iff "multiline" is TRUE, strings for function types contain new line
 *   symbols. Each new line is followed by "offset" preceeding blanks.
 *
 ******************************************************************************/

char *
TYtype2DebugString (ntype *type, bool multiline, size_t offset)
{
    str_buf *buf;
    char *tmp_str;
    size_t i, n;

    DBUG_ENTER ();

    buf = SBUFcreate (8192);
    if (type == NULL) {
        buf = SBUFprintf (buf, "--");
    } else {
        buf = SBUFprintf (buf, "%s{ ", dbug_str[NTYPE_CON (type)]);

        switch (NTYPE_CON (type)) {
        case TC_bottom:
            multiline = FALSE;
            buf = SBUFprint (buf, BOTTOM_MSG (type));
            break;
        case TC_akv:
            multiline = FALSE;
            tmp_str = COconstant2String (AKV_CONST (type));
            buf = SBUFprintf (buf, "%s, ", tmp_str);
            tmp_str = MEMfree (tmp_str);
            break;
        case TC_aks:
            multiline = FALSE;
            tmp_str = SHshape2String (0, AKS_SHP (type));
            buf = SBUFprintf (buf, "%s, ", tmp_str);
            tmp_str = MEMfree (tmp_str);
            break;
        case TC_akd:
            multiline = FALSE;
            tmp_str = SHshape2String (AKD_DOTS (type), AKD_SHP (type));
            buf = SBUFprintf (buf, "%s, ", tmp_str);
            tmp_str = MEMfree (tmp_str);
            break;
        case TC_aud:
            multiline = FALSE;
            break;
        case TC_simple:
            multiline = FALSE;
            buf = SBUFprintf (buf, "%s", global.mdb_type[SIMPLE_TYPE (type)]);
            if (SIMPLE_TYPE (type) == T_hidden) {
                buf = SBUFprintf (buf, "(%d)", SIMPLE_HIDDEN_UDT (type));
            }
            break;
        case TC_symbol:
            multiline = FALSE;
            if (SYMBOL_NS (type) == NULL) {
                buf = SBUFprint (buf, SYMBOL_NAME (type));
            } else {
                buf = SBUFprintf (buf, "%s::%s", NSgetName (SYMBOL_NS (type)),
                                  SYMBOL_NAME (type));
            }
            break;
        case TC_user:
            multiline = FALSE;
            buf = SBUFprintf (buf, "%d", USER_TYPE (type));
            break;
        case TC_poly:
            multiline = FALSE;
            buf = SBUFprint (buf, POLY_NAME (type));
            break;
        case TC_ibase:
            tmp_str = TYtype2DebugString (IBASE_BASE (type), FALSE, offset);
            buf = SBUFprint (buf, tmp_str);
            buf = SBUFprint (buf, ",");
            tmp_str = MEMfree (tmp_str);
            break;
        case TC_idim:
            buf = SBUFprintf (buf, "%zu,", IDIM_DIM (type));
            break;
        case TC_ishape:
            tmp_str = SHshape2String (0, ISHAPE_SHAPE (type));
            buf = SBUFprintf (buf, "%s,", tmp_str);
            tmp_str = MEMfree (tmp_str);
            break;
        case TC_ires:
            if (IRES_NUMFUNS (type) > 0) {
                buf = SBUFprintf (buf, "poss: {");
                for (i = 0; i < IRES_NUMFUNS (type); i++) {
                    buf = SBUFprintf (buf, "%d ", IRES_POS (type, i));
                }
                buf = SBUFprintf (buf, "} ");
            }
            if (IRES_NUMFUNS (type) > 0) {
                buf = SBUFprintf (buf, "fundefs: {");
                for (i = 0; i < IRES_NUMFUNS (type); i++) {
                    buf = SBUFprintf (buf, F_PTR " ", (void *)IRES_FUNDEF (type, i));
                }
                buf = SBUFprintf (buf, "} ");
            }
            break;
        case TC_alpha:
            multiline = FALSE;
            tmp_str = SSIvariable2DebugString (ALPHA_SSI (type));
            buf = SBUFprintf (buf, "%s", tmp_str);
            tmp_str = MEMfree (tmp_str);
            break;
        default:
            break;
        }

        if (variable_arity[NTYPE_CON (type)]) {
            buf = SBUFprintf (buf, " <");
        }
        n = NTYPE_ARITY (type);
        offset += 3;
        for (i = 0; i < n; i++) {
            tmp_str = TYtype2DebugString (NTYPE_SON (type, i), multiline, offset);
            if (i == 0) {
                if (multiline) {
                    buf = SBUFprintf (buf, "\n%*s", offset - 1, "");
                }
                buf = SBUFprint (buf, tmp_str);
            } else {
                buf = PrintFunSep (buf, multiline, offset);
                buf = SBUFprint (buf, tmp_str);
            }
            tmp_str = MEMfree (tmp_str);
        }
        offset -= 3;
        if (variable_arity[NTYPE_CON (type)]) {
            buf = SBUFprintf (buf, ">");
        }
        buf = SBUFprintf (buf, "}");
    }

    tmp_str = SBUF2str (buf);
    buf = SBUFfree (buf);

    DBUG_RETURN (tmp_str);
}

char *
TYArgs2FunTypeString (node *args, ntype *rettype)
{
    str_buf *buf;
    char *tmp;

    DBUG_ENTER ();

    buf = SBUFcreate (4096);

    SBUFprintf (buf, "PROJ { ");

    while (args != NULL) {
        ntype *atype = AVIS_TYPE (ARG_AVIS (args));

        if (atype != NULL) {
            tmp = TYtype2String (atype, 0, 0);

            SBUFprintf (buf, "%s -> ", tmp);

            tmp = MEMfree (tmp);
        }

        args = ARG_NEXT (args);
    }

    tmp = TYtype2String (rettype, 0, 0);

    SBUFprintf (buf, "%s }", tmp);

    tmp = SBUF2str (buf);

    buf = SBUFfree (buf);

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYnestTypes( ntype *outer, ntype *inner)
 *
 * description:
 *    nests (array) types. Since this function is NOT considered a type
 *    constructing (MakeXYZ) function, it does NOT re-use the argument types
 *    but inspects them only!
 *
 ******************************************************************************/

#if AKD_MAY_CONTAIN_SHAPE

ntype *
TYnestTypes (ntype *outer, ntype *inner)
{
    ntype *res;

    DBUG_ENTER ();

    if (NTYPE_CON (outer) == TC_aks) {
        /*
         * AKS{ a, s1}, AKS{ b, s2}      => AKS{ b, s1++s2}}
         * AKS{ a, s1}, AKD{ b, do2, s2} => AKD{ b, d1+do2, s2}
         * AKS{ a, s1}, AUD{ b}          => AUD{ b}
         * AKS{ a, s1}, b                => AKS{ b, s1}
         *
         */
        if (NTYPE_CON (inner) == TC_aks) {
            res = TYmakeAKS (TYcopyType (AKS_BASE (inner)),
                             SHappendShapes (AKS_SHP (outer), AKS_SHP (inner)));
        } else if (NTYPE_CON (inner) == TC_akd) {
            res = TYmakeAKD (TYcopyType (AKD_BASE (inner)),
                             TYgetDim (outer) + AKD_DOTS (inner),
                             SHcopyShape (AKD_SHP (inner)));
        } else if (NTYPE_CON (inner) == TC_aud) {
            res = TYmakeAUD (TYcopyType (AKS_BASE (inner)));
        } else {
            res = TYmakeAKS (TYcopyType (inner), SHcopyShape (AKS_SHP (outer)));
        }
    } else if (NTYPE_CON (outer) == TC_akd) {
        /*
         * AKD{ a, do1, s1}, AKS{ b, s2}      => AKD{ b, do1, s1++s2}}
         * AKD{ a, do1, s1}, AKD{ b, do2, s2} => AKD{ b, d1+do2, s2}
         * AKD{ a, do1, s1}, AUD{ b}          => AUD{ b}
         * AKD{ a, do1, s1}, b                => AKD{ b, do1, s1}
         *
         */
        if (NTYPE_CON (inner) == TC_aks) {
            res = NULL;
        } else if (NTYPE_CON (inner) == TC_akd) {
            res = TYmakeAKD (TYcopyType (AKS_BASE (inner)), AKD_DOTS (outer),
                             SHappendShapes (AKD_SHP (outer), AKS_SHP (inner)));
            res = TYmakeAKD (TYcopyType (AKD_BASE (inner)),
                             TYgetDim (outer) + AKD_DOTS (inner),
                             SHcopyShape (AKD_SHP (inner)));
        } else if (NTYPE_CON (inner) == TC_aud) {
            res = TYmakeAUD (TYcopyType (AKS_BASE (inner)));
        } else {
            res = TYmakeAKD (TYcopyType (inner), AKD_DOTS (inner),
                             SHcopyShape (AKD_SHP (outer)));
        }
    } else if (NTYPE_CON (outer) == TC_aud) {
        /*
         * AUD{ a}, AKS{ b, s2}      => AUD{ b}
         * AUD{ a}, AKD{ b, do2, s2} => AUD{ b}
         * AUD{ a}, AUD{ b}          => AUD{ b}
         * AUD{ a}, b                => AUD{ b}
         *
         */
        if (NTYPE_CON (inner) == TC_aks) {
            res = TYmakeAUD (TYcopyType (AKS_BASE (inner)));
        } else if (NTYPE_CON (inner) == TC_akd) {
            res = TYmakeAUD (TYcopyType (AKD_BASE (inner)));
        } else if (NTYPE_CON (inner) == TC_aud) {
            res = TYmakeAUD (TYcopyType (AUD_BASE (inner)));
        } else {
            res = TYmakeAUD (TYcopyType (inner));
        }
    } else {
        /*
         * a, b => b
         *
         */
        res = TYcopyType (inner);
    }

    DBUG_RETURN (res);
}

#else /* AKD_MAY_CONTAIN_SHAPE */

ntype *
TYnestTypes (ntype *outer, ntype *inner)
{
    ntype *res;

    DBUG_ENTER ();

    switch (NTYPE_CON (outer)) {
    case TC_aks:
        /*
         * AKS{ a, s1}, AKV{ b, s2, v2}  => AKS{ b, s1++s2}}
         * AKS{ a, s1}, AKS{ b, s2}      => AKS{ b, s1++s2}}
         * AKS{ a, s1}, AKD{ b, do2, --} => AKD{ b, d1+do2, --}
         * AKS{ a, s1}, AUDGZ{ b}        => AUDGZ{ b}
         * AKS{ a, s1}, AUD{ b}          => AUDGZ{ b} / AUD{ b}
         * AKS{ a, s1}, b                => AKS{ b, s1}
         *
         */
        switch (NTYPE_CON (inner)) {
        case TC_akv:
            res = TYmakeAKS (TYcopyType (AKV_BASE (inner)),
                             SHappendShapes (AKS_SHP (outer), TYgetShape (inner)));
            break;
        case TC_aks:
            res = TYmakeAKS (TYcopyType (AKS_BASE (inner)),
                             SHappendShapes (AKS_SHP (outer), AKS_SHP (inner)));
            break;
        case TC_akd:
            res = TYmakeAKD (TYcopyType (AKD_BASE (inner)),
                             TYgetDim (outer) + AKD_DOTS (inner), SHmakeShape (0));
            break;
        case TC_audgz:
            res = TYcopyType (inner);
            break;
        case TC_aud:
            if (TYgetDim (outer) > 0) {
                res = TYmakeAUDGZ (TYcopyType (AKD_BASE (inner)));
            } else {
                res = TYcopyType (inner);
            }
            break;
        default:
            res = TYmakeAKS (TYcopyType (inner), SHcopyShape (AKS_SHP (outer)));
        }
        break;

    case TC_akd:
        /*
         * AKD{ a, do1, --}, AKV{ b, s2, v2}  => AKD{ b, do1+d2, --}}
         * AKD{ a, do1, --}, AKS{ b, s2}      => AKD{ b, do1+d2, --}}
         * AKD{ a, do1, --}, AKD{ b, do2, --} => AKD{ b, do1+do2, --}
         * AKD{ a, do1, --}, AUDGZ{ b}        => AUDGZ{ b}
         * AKD{ a, do1, --}, AUD{ b}          => AUDGZ{ b} / AUD{ b}
         * AKD{ a, do1, --}, b                => AKD{ b, do1, --}
         *
         */
        switch (NTYPE_CON (inner)) {
        case TC_akv:
        case TC_aks:
        case TC_akd:
            res = TYmakeAKD (TYcopyType (AKD_BASE (inner)),
                             TYgetDim (outer) + TYgetDim (inner), SHmakeShape (0));
            break;
        case TC_audgz:
            res = TYcopyType (inner);
            break;
        case TC_aud:
            if (TYgetDim (outer) > 0) {
                res = TYmakeAUDGZ (TYcopyType (AKD_BASE (inner)));
            } else {
                res = TYcopyType (inner);
            }
            break;
        default:
            res = TYmakeAKD (TYcopyType (inner), AKD_DOTS (inner),
                             SHcopyShape (AKD_SHP (outer)));
        }
        break;

    case TC_audgz:
        /*
         * AUDGZ{ a}, AKV{ b, s2, v2}  => AUDGZ{ b}
         * AUDGZ{ a}, AKS{ b, s2}      => AUDGZ{ b}
         * AUDGZ{ a}, AKD{ b, do2, s2} => AUDGZ{ b}
         * AUDGZ{ a}, AUDGZ{ b}        => AUDGZ{ b}
         * AUDGZ{ a}, AUD{ b}          => AUDGZ{ b}
         * AUDGZ{ a}, b                => AUDGZ{ b}
         *
         */
        res = TYmakeAUDGZ (TYcopyType (AKD_BASE (inner)));
        break;

    case TC_aud:
        /*
         * AUD{ a}, AKV{ b, s2, v2}  => AUDGZ{ b} / AUD{ b}
         * AUD{ a}, AKS{ b, s2}      => AUDGZ{ b} / AUD{ b}
         * AUD{ a}, AKD{ b, do2, s2} => AUDGZ{ b} / AUD{ b}
         * AUD{ a}, AUDGZ{ b}        => AUDGZ{ b}
         * AUD{ a}, AUD{ b}          => AUD{ b}
         * AUD{ a}, b                => AUD{ b}
         *
         */
        switch (NTYPE_CON (inner)) {
        case TC_akv:
        case TC_aks:
        case TC_akd:
            if (TYgetDim (inner) > 0) {
                res = TYmakeAUDGZ (TYcopyType (AKD_BASE (inner)));
            } else {
                res = TYmakeAUD (TYcopyType (AKD_BASE (inner)));
            }
            break;
        case TC_audgz:
        case TC_aud:
            res = TYcopyType (inner);
            break;
        default:
            res = TYmakeAUD (TYcopyType (inner));
            break;
        }
        break;

    default:
        /*
         * a, b => b
         *
         */
        res = TYcopyType (inner);
        break;
    }

    DBUG_RETURN (res);
}

#endif /* AKD_MAY_CONTAIN_SHAPE */

/******************************************************************************
 *
 * function:
 *    ntype * TYdeNestTypeFromInner( ntype *nested, ntype *inner)
 *    ntype * TYdeNestTypeFromOuter( ntype *nested, ntype *inner)
 *
 * description:
 *    de-nests (array) types. Since this function is NOT considered a type
 *    constructing (MakeXYZ) function, it does NOT re-use the argument types
 *    but inspects them only!
 *
 ******************************************************************************/

ntype *
TYdeNestTypeFromInner (ntype *nested, ntype *inner)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    if (TYisAKS (inner)) {
        switch (NTYPE_CON (nested)) {
        case TC_aks:
            /*
             * AKS{ a, s1}, AKS{ b, s2}      => AKS{ a, drop( [-d2], s1) }
             * AKS{ a, s1}, AKD{ b, do2, --} => AKS{ a, drop( [-do2], s1) }
             * AKS{ a, s1}, AUDGZ{ b}        => AUD{ a} / AKS{ a, []}
             * AKS{ a, s1}, AUD{ b}          => AUD{ a} / AKS{ a, []}
             * AKS{ a, s1}, b                => AKS{ a, s1}
             *
             */
            switch (NTYPE_CON (inner)) {
            case TC_aks:
            case TC_akd:
                res = TYmakeAKS (TYcopyType (AKS_BASE (nested)),
                                 SHdropFromShape (-TYgetDim (inner), AKS_SHP (nested)));
                break;
            case TC_audgz:
                if (TYgetDim (nested) == 1) {
                    res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
                } else {
                    res = TYmakeAUD (TYcopyType (AKD_BASE (nested)));
                }
                break;
            case TC_aud:
                if (TYgetDim (nested) == 0) {
                    res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
                } else {
                    res = TYmakeAUD (TYcopyType (AKD_BASE (nested)));
                }
                break;
            default:
                res = TYcopyType (nested);
            }
            break;

        case TC_akd:
            /*
             * AKD{ a, do1, --}, AKS{ b, s2}      => AKD{ a, do1-d2, --}} / AKS{ a, []}
             * AKD{ a, do1, --}, AKD{ b, do2, --} => AKD{ a, do1-do2, --} / AKS{ a, []}
             * AKD{ a, do1, --}, AUDGZ{ b}        => AUD{ a} / AKS{ a, []}
             * AKD{ a, do1, --}, AUD{ b}          => AUD{ a} / AKS{ a, []}
             * AKD{ a, do1, --}, b                => AKD{ a, do1, --}
             *
             */
            switch (NTYPE_CON (inner)) {
            case TC_aks:
            case TC_akd:
                if ((TYgetDim (nested) - TYgetDim (inner)) == 0) {
                    res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
                } else {
                    res
                      = TYmakeAKD (TYcopyType (AKD_BASE (nested)),
                                   TYgetDim (nested) - TYgetDim (inner), SHmakeShape (0));
                }
                break;
            case TC_audgz:
                if (TYgetDim (nested) == 1) {
                    res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
                } else {
                    res = TYmakeAUD (TYcopyType (AKS_BASE (nested)));
                }
                break;
            case TC_aud:
                if (TYgetDim (nested) == 0) {
                    res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
                } else {
                    res = TYmakeAUD (TYcopyType (AKS_BASE (nested)));
                }
                break;
            default:
                res = TYcopyType (nested);
            }
            break;

        case TC_audgz:
            /*
             * AUDGZ{ a}, AKS{ b, s2}      => AUD{ a} / AUDGZ{ a}
             * AUDGZ{ a}, AKD{ b, do2, s2} => AUD{ a} / AUDGZ{ a}
             * AUDGZ{ a}, AUDGZ{ b}        => AUD{ a}
             * AUDGZ{ a}, AUD{ b}          => AUD{ a}
             * AUDGZ{ a}, b                => AUDGZ{ a}
             *
             */
            switch (NTYPE_CON (inner)) {
            case TC_aks:
            case TC_akd:
                if (TYgetDim (inner) == 0) {
                    res = TYmakeAUDGZ (TYcopyType (AKD_BASE (nested)));
                } else {
                    res = TYmakeAUD (TYcopyType (AKD_BASE (nested)));
                }
                break;
            case TC_audgz:
            case TC_aud:
                res = TYmakeAUD (TYcopyType (AKD_BASE (nested)));
                break;
            default:
                res = TYcopyType (nested);
                break;
            }
            break;

        case TC_aud:
            /*
             * AUD{ a}, AKS{ b, s2}      => AUD{ a}
             * AUD{ a}, AKD{ b, do2, s2} => AUD{ a}
             * AUD{ a}, AUDGZ{ b}        => AUD{ a}
             * AUD{ a}, AUD{ b}          => AUD{ a}
             * AUD{ a}, b                => AUD{ a}
             *
             */
            res = TYcopyType (nested);
            break;

        default:
            /*
             * a, b => a
             *
             */
            res = TYcopyType (nested);
            break;
        }
    } else if (TYisAKD (inner)) {
        res = TYcopyType (nested);
        printf ("WE ARE HERE\n");
    } else {
        DBUG_UNREACHABLE (
          "TYDeNestTypeFromInner with non AKS/AKD inner type not yet implemented!");
    }

    DBUG_RETURN (res);
}

ntype *
TYdeNestTypeFromOuter (ntype *nested, ntype *outer)
{
    ntype *res;

    DBUG_ENTER ();

    DBUG_ASSERT (TYisAKS (outer),
                 "TYDeNestTypeFromOuter with non AKS outer type not yet implemented!");

    switch (NTYPE_CON (nested)) {
    case TC_akv:
        /*
         * AKV{ a, s1}, AKS{ b, s2}      => AKS{ a, drop( [d2], s1) }
         * AKV{ a, s1}, AKD{ b, do2, --} => AKS{ a, drop( [do2], s1) }
         * AKV{ a, s1}, AUDGZ{ b}        => AUD{ a} / AKS{ a, []}
         * AKV{ a, s1}, AUD{ b}          => AUD{ a} / AKS{ a, []}
         * AKV{ a, s1}, b                => AKS{ a, s1}
         *
         */
        switch (NTYPE_CON (outer)) {
        case TC_aks:
        case TC_akd:
            res = TYmakeAKS (TYcopyType (AKV_BASE (nested)),
                             SHdropFromShape (TYgetDim (outer),
                                              COgetShape (AKV_CONST (nested))));
            break;
        case TC_audgz:
            if (TYgetDim (nested) == 1) {
                res = TYmakeAKS (TYcopyType (AKV_BASE (nested)), SHmakeShape (0));
            } else {
                res = TYmakeAUD (TYcopyType (AKV_BASE (nested)));
            }
            break;
        case TC_aud:
            if (TYgetDim (nested) == 0) {
                res = TYmakeAKS (TYcopyType (AKV_BASE (nested)), SHmakeShape (0));
            } else {
                res = TYmakeAUD (TYcopyType (AKV_BASE (nested)));
            }
            break;
        default:
            res = TYcopyType (nested);
        }
        break;

    case TC_aks:
        /*
         * AKS{ a, s1}, AKS{ b, s2}      => AKS{ a, drop( [d2], s1) }
         * AKS{ a, s1}, AKD{ b, do2, --} => AKS{ a, drop( [do2], s1) }
         * AKS{ a, s1}, AUDGZ{ b}        => AUD{ a} / AKS{ a, []}
         * AKS{ a, s1}, AUD{ b}          => AUD{ a} / AKS{ a, []}
         * AKS{ a, s1}, b                => AKS{ a, s1}
         *
         */
        switch (NTYPE_CON (outer)) {
        case TC_aks:
        case TC_akd:
            res = TYmakeAKS (TYcopyType (AKS_BASE (nested)),
                             SHdropFromShape (TYgetDim (outer), AKS_SHP (nested)));
            break;
        case TC_audgz:
            if (TYgetDim (nested) == 1) {
                res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
            } else {
                res = TYmakeAUD (TYcopyType (AKV_BASE (nested)));
            }
            break;
        case TC_aud:
            if (TYgetDim (nested) == 0) {
                res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
            } else {
                res = TYmakeAUD (TYcopyType (AKV_BASE (nested)));
            }
            break;
        default:
            res = TYcopyType (nested);
        }
        break;

    case TC_akd:
        /*
         * AKD{ a, do1, --}, AKS{ b, s2}      => AKD{ a, do1-d2, --}} / AKS{ a, []}
         * AKD{ a, do1, --}, AKD{ b, do2, --} => AKD{ a, do1-do2, --} / AKS{ a, []}
         * AKD{ a, do1, --}, AUDGZ{ b}        => AUD{ a} / AKS{ a, []}
         * AKD{ a, do1, --}, AUD{ b}          => AUD{ a} / AKS{ a, []}
         * AKD{ a, do1, --}, b                => AKD{ a, do1, --}
         *
         */
        switch (NTYPE_CON (outer)) {
        case TC_aks:
        case TC_akd:
            if ((TYgetDim (nested) - TYgetDim (outer)) == 0) {
                res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
            } else {
                res = TYmakeAKD (TYcopyType (AKD_BASE (nested)),
                                 TYgetDim (nested) - TYgetDim (outer), SHmakeShape (0));
            }
            break;
        case TC_audgz:
            if (TYgetDim (nested) == 1) {
                res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
            } else {
                res = TYmakeAUD (TYcopyType (AKD_BASE (nested)));
            }
            break;
        case TC_aud:
            if (TYgetDim (nested) == 0) {
                res = TYmakeAKS (TYcopyType (AKS_BASE (nested)), SHmakeShape (0));
            } else {
                res = TYmakeAUD (TYcopyType (AKD_BASE (nested)));
            }
            break;
        default:
            res = TYcopyType (nested);
        }
        break;

    case TC_audgz:
        /*
         * AUDGZ{ a}, AKS{ b, s2}      => AUD{ a} / AUDGZ{ a}
         * AUDGZ{ a}, AKD{ b, do2, s2} => AUD{ a} / AUDGZ{ a}
         * AUDGZ{ a}, AUDGZ{ b}        => AUD{ a}
         * AUDGZ{ a}, AUD{ b}          => AUD{ a}
         * AUDGZ{ a}, b                => AUDGZ{ a}
         *
         */
        switch (NTYPE_CON (outer)) {
        case TC_aks:
        case TC_akd:
            if (TYgetDim (outer) == 0) {
                res = TYmakeAUDGZ (TYcopyType (AKD_BASE (nested)));
            } else {
                res = TYmakeAUD (TYcopyType (AKD_BASE (nested)));
            }
            break;
        case TC_audgz:
        case TC_aud:
            res = TYmakeAUD (TYcopyType (AKD_BASE (nested)));
            break;
        default:
            res = TYcopyType (nested);
            break;
        }
        break;

    case TC_aud:
        /*
         * AUD{ a}, AKS{ b, s2}      => AUD{ a}
         * AUD{ a}, AKD{ b, do2, s2} => AUD{ a}
         * AUD{ a}, AUDGZ{ b}        => AUD{ a}
         * AUD{ a}, AUD{ b}          => AUD{ a}
         * AUD{ a}, b                => AUD{ a}
         *
         */
        res = TYcopyType (nested);
        break;

    default:
        /*
         * a, b => b
         *
         */
        res = TYcopyType (nested);
        break;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYoldType2ScalarType( types *old)
 *
 * description:
 *    converts an old TYPES node into an ntype node for the base type.
 *
 ******************************************************************************/

ntype *
TYoldType2ScalarType (types *old)
{
    ntype *res;
    usertype udt;

#ifndef DBUG_OFF
    char *tmp = NULL, *tmp2 = NULL;
#endif

    DBUG_ENTER ();

    switch (TYPES_BASETYPE (old)) {
    case T_user:
        if (TYPES_POLY (old)) {
            res = TYmakePolyType (TYPES_NAME (old));
        } else {
            udt = UTfindUserType (TYPES_NAME (old), NSgetNamespace (TYPES_MOD (old)));
            if (udt == UT_NOT_DEFINED) {
                res = TYmakeSymbType (STRcpy (TYPES_NAME (old)),
                                      NSgetNamespace (TYPES_MOD (old)));
            } else {
                res = TYmakeUserType (udt);
            }
        }
        break;
    case T_byte:
    case T_short:
    case T_int:
    case T_long:
    case T_longlong:
    case T_ubyte:
    case T_ushort:
    case T_uint:
    case T_ulong:
    case T_ulonglong:
    case T_float:
    case T_floatvec:
    case T_double:
    case T_longdbl:
    case T_bool:
    case T_str:
    case T_char:
    case T_hidden:
        res = TYmakeSimpleType (TYPES_BASETYPE (old));
        break;
    case T_void:
    case T_unknown:
    case T_nothing:
        res = NULL;
        break;
    case T_dots:
        res = NULL;
        DBUG_UNREACHABLE ("TYoldType2Type applied to T_dots");
        break;
    default:
        res = NULL;
        DBUG_UNREACHABLE ("TYoldType2Type applied to illegal type");
    }

#if 0
    DBUG_EXECUTE (tmp = CVtype2String (old, 3, TRUE);
                  tmp2 = TYtype2DebugString (res, TRUE, 0));
    DBUG_PRINT ("base type of %s converted into : %s\n", tmp, tmp2);
    DBUG_EXECUTE (tmp = MEMfree (tmp); tmp2 = MEMfree (tmp2));
#endif

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYoldType2Type( types *old)
 *
 * description:
 *    converts an old TYPES node into an ntype node (or - if neccessary -
 *    a nesting of ntype nodes).
 *
 ******************************************************************************/

ntype *
TYoldType2Type (types *old)
{
    ntype *res;

#ifndef DBUG_OFF
    char *tmp = NULL, *tmp2 = NULL;
#endif

    DBUG_ENTER ();

    if (TYPES_AKV (old)) {
        CTInote ("AKV information lost in newtype->oldtype->newtype conversion");
    }

    res = TYoldType2ScalarType (old);

    if (res != NULL) {
        if (TYPES_DIM (old) > SCALAR) {
            res
              = TYmakeAKS (res, SHoldShpseg2Shape (TYPES_DIM (old), TYPES_SHPSEG (old)));
        } else if (TYPES_DIM (old) < KNOWN_DIM_OFFSET) {
            /* the result of this subtraction is always positive so safe  */
            res = TYmakeAKD (res, (size_t)(KNOWN_DIM_OFFSET - TYPES_DIM (old)), SHmakeShape (0));
        } else if (TYPES_DIM (old) == UNKNOWN_SHAPE) {
            res = TYmakeAUDGZ (res);
        } else if (TYPES_DIM (old) == ARRAY_OR_SCALAR) {
            res = TYmakeAUD (res);
        } else { /* TYPES_DIM( old) == SCALAR */
            res = TYmakeAKS (res, SHcreateShape (0));
        }
    }

#if 0
    DBUG_EXECUTE (tmp = CVtype2String (old, 3, TRUE);
                  tmp2 = TYtype2DebugString (res, TRUE, 0));
    DBUG_PRINT ("%s converted into : %s\n", tmp, tmp2);
    DBUG_EXECUTE (tmp = MEMfree (tmp); tmp2 = MEMfree (tmp2));
#endif

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYoldTypes2ProdType( types *old)
 *
 * description:
 *    converts a (linked list of) old TYPES node(s) into a product type of ntypes.
 *
 ******************************************************************************/

ntype *
TYoldTypes2ProdType (types *old)
{
    size_t i, num_types;
    ntype *res;

    num_types = TCcountTypes (old);
    res = TYmakeEmptyProductType (num_types);
    for (i = 0; i < num_types; i++) {
        res = TYsetProductMember (res, i, TYoldType2Type (old));
        old = TYPES_NEXT (old);
    }
    return (res);
}

/******************************************************************************
 *
 * function:
 *    types * TYType2OldType( ntype *new)
 *
 * description:
 *
 *
 ******************************************************************************/

static types *
Type2OldType (ntype *xnew)
{
    types *res = NULL;
    types *tmp = NULL;
    int i;

    DBUG_ENTER ();

    switch (NTYPE_CON (xnew)) {
    case TC_alpha:
        DBUG_ASSERT (TYcmpTypes (SSIgetMin (TYgetAlpha (xnew)),
                                 SSIgetMax (TYgetAlpha (xnew)))
                       == TY_eq,
                     "Type2OldType applied to non-unique alpha type");
        res = Type2OldType (SSIgetMin (TYgetAlpha (xnew)));
        break;
    case TC_prod:
        if (NTYPE_ARITY (xnew) == 0) {
            res = TBmakeTypes1 (T_void);
        } else {
            for (i = (int)NTYPE_ARITY (xnew) - 1; i >= 0; i--) {
                res = Type2OldType (PROD_MEMBER (xnew, i));
                TYPES_NEXT (res) = tmp;
                tmp = res;
            }
        }
        break;
    case TC_akv:
        res = Type2OldType (AKS_BASE (xnew));
        TYPES_DIM (res) = TYgetDim (xnew);
        TYPES_SHPSEG (res) = SHshape2OldShpseg (TYgetShape (xnew));
        TYPES_AKV (res) = TRUE;
        break;
    case TC_aks:
        res = Type2OldType (AKS_BASE (xnew));
        TYPES_DIM (res) = SHgetDim (AKS_SHP (xnew));
        TYPES_SHPSEG (res) = SHshape2OldShpseg (AKS_SHP (xnew));

        break;
    case TC_akd:
        res = Type2OldType (AKD_BASE (xnew));
        TYPES_DIM (res) = KNOWN_DIM_OFFSET - (int)AKD_DOTS (xnew);
        break;
    case TC_audgz:
        res = Type2OldType (AUDGZ_BASE (xnew));
        TYPES_DIM (res) = UNKNOWN_SHAPE;
        break;
    case TC_aud:
        res = Type2OldType (AUD_BASE (xnew));
        TYPES_DIM (res) = ARRAY_OR_SCALAR;
        break;
    case TC_simple:
        if ((SIMPLE_TYPE (xnew) == T_hidden)
            && (SIMPLE_HIDDEN_UDT (xnew) != UT_NOT_DEFINED)) {
            res = TBmakeTypes (T_user, 0, NULL,
                               STRcpy (UTgetName (SIMPLE_HIDDEN_UDT (xnew))),
                               STRcpy ((UTgetNamespace (SIMPLE_HIDDEN_UDT (xnew)) == NULL)
                                         ? NULL
                                         : NSgetName (
                                             UTgetNamespace (SIMPLE_HIDDEN_UDT (xnew)))));
            TYPES_TDEF (res) = UTgetTdef (SIMPLE_HIDDEN_UDT (xnew));
        } else {
            res = TBmakeTypes (SIMPLE_TYPE (xnew), 0, NULL, NULL, NULL);
        }
        break;
    case TC_user:
        res = TBmakeTypes (T_user, 0, NULL, STRcpy (UTgetName (USER_TYPE (xnew))),
                           STRcpy ((UTgetNamespace (USER_TYPE (xnew)) == NULL)
                                     ? NULL
                                     : NSgetName (UTgetNamespace (USER_TYPE (xnew)))));
        TYPES_TDEF (res) = UTgetTdef (USER_TYPE (xnew));
        break;
    default:
        DBUG_UNREACHABLE ("Type2OldType not yet entirely implemented!");
        res = NULL;
        break;
    }

    if (res != NULL && xnew != NULL) {
        TYPES_MUTC_SCOPE (res) = NTYPE_MUTC_SCOPE (xnew);
        TYPES_MUTC_USAGE (res) = NTYPE_MUTC_USAGE (xnew);
        if (TYisUnique (xnew)) {
            TYPES_UNIQUE (res) = TRUE;
        }
    }

    /* Decide whether the type is distributable. */
    /* TODO: This is not the best location but we only need this during code generation
     * so at this moment we do not need this in the new type system. */
    if (global.backend == BE_distmem) {
        if (TYgetDistributed (xnew) == distmem_dis_dsm) {
            TYPES_DISTRIBUTED (res) = distmem_dis_dsm;
        }

        /* It seems like the basetype is not yet supported by the distributed memory
         * backend. Don't distribute. */
        else if (
          global.type_cbasetype[TYPES_BASETYPE (res)] != C_btother
          /* To avoid problems with string functions, we do not distribute unsigned char
             arrays. */
          && global.type_cbasetype[TYPES_BASETYPE (res)] != C_btuchar
          /* It doesn't make sense to distribute scalars. */
          && TYPES_DIM (res) != SCALAR
          /* We do not distribute hidden types. It is not practical since we would have to
           * think about (de-)serialization. But since hidden types come from the
           * non-distributed C world it doesn't make make sense to distribute them
           * anyways. */
          && !TCisHidden (res)
          /* It doesn't make sense to distribute unique types. These are only used by the
           * master node.
           * TODO: TYPES_UNIQUE seems to be always FALSE. */
          && !TYPES_UNIQUE (res)
          /* For now we do not distribute nested types. TODO: What are these actually? */
          && !TCisNested (res)) {
            TYPES_DISTRIBUTED (res) = distmem_dis_dis;
        }
    }

    DBUG_RETURN (res);
}

types *
TYtype2OldType (ntype *xnew)
{
    types *res;
#ifndef DBUG_OFF
    char *tmp_str = NULL, *tmp_str2 = NULL;
#endif

    DBUG_ENTER ();

    DBUG_EXECUTE (tmp_str = TYtype2DebugString (xnew, FALSE, 0));
    DBUG_PRINT ("converting %s", tmp_str);

    res = Type2OldType (xnew);

#if 0
    DBUG_EXECUTE (tmp_str2 = CVtype2String (res, 0, TRUE));
    DBUG_PRINT ("... result is %s", tmp_str2);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));
    DBUG_EXECUTE (tmp_str2 = MEMfree (tmp_str2));
#endif

    DBUG_RETURN (res);
}

/**
 **  functions for creating wrapper function code
 **/

/** <!--********************************************************************-->
 *
 * @fn void ExtractTopBaseSignature( ntype *fun, ntype **frame)
 *
 *   @brief extracts ntype pointers that point to scalar types
 *          defining the signature of the "topmost" chain and puts these into
 *          the frame provided.
 *   @param fun TC_fun node
 *   @param frame array big enough to accomadate the scalar types of the signature
 *
 ******************************************************************************/

static void
ExtractTopBaseSignature (ntype *fun, ntype **frame)
{
    ntype *next;

    DBUG_ENTER ();

    *frame = TYcopyType (IBASE_BASE (FUN_IBASE (fun, 0)));
    next = IRES_TYPE (IBASE_GEN (FUN_IBASE (fun, 0)));

    if (NTYPE_CON (next) == TC_fun) {
        ExtractTopBaseSignature (next, frame + 1);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn int FindBase( ntype *scalar, ntype *fun)
 *
 *   @brief searches for scalar in the ibases attached to fun.
 *   @param scalar scalar type we are looking for
 *   @param fun TC_fun node
 *   @return position of the ibase or -1 in case no suitable ibase is present
 *
 ******************************************************************************/

static int
FindBase (ntype *scalar, ntype *fun)
{
    int res = -1;
    size_t i;

    DBUG_ENTER ();

    for (i = 2; i < NTYPE_ARITY (fun); i++) {
        if (TYeqTypes (scalar, IBASE_BASE (NTYPE_SON (fun, i)))) {
            res = (int)i;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   ntype *SplitWrapperType( ntype *type, bool *finished)
 *
 * Description:
 *   Extracts a single FUN_IBASE path from the given type 'type'.
 *   (The found path is removed from 'type'!!)
 *   Iff 'type' contains a single FUN_IBASE path only, '*finished' is set to
 *   TRUE.
 *
 ******************************************************************************/

static ntype *
SplitWrapperType (ntype *type, int level, ntype **frame, int *pathes_remaining)
{
    ntype *new_type, *son;
    int pos_i, pathes_found;
    size_t i, pos;
    size_t mandatory = 1;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    if (type == NULL) {
        new_type = NULL;
    } else {
        new_type = CopyTypeConstructor (type, tv_id);

        DBUG_PRINT_TAG ("NTY_SPLIT", "processing %s node at" F_PTR "(cpy:" F_PTR ")",
                        dbug_str[NTYPE_CON (type)], (void *)type, (void *)new_type);
        switch (NTYPE_CON (type)) {

        case TC_fun:
            DBUG_ASSERT (NTYPE_ARITY (type) >= 3, "TC_fun with (ARITY < 3) found!");
            DBUG_ASSERT (FUN_POLY (type) == NULL,
                         "SplitWrapperType called in the presence of poly version!");
            DBUG_ASSERT (FUN_UPOLY (type) == NULL,
                         "SplitWrapperType called in the presence of poly-user version!");

            DBUG_EXECUTE_TAG ("NTY_SPLIT",
                              tmp_str = TYtype2DebugString (frame[level], FALSE, 0));
            DBUG_PRINT_TAG ("NTY_SPLIT", "--looking for %s", tmp_str);
            DBUG_EXECUTE_TAG ("NTY_SPLIT", tmp_str = MEMfree (tmp_str));

            pos_i = FindBase (frame[level], type);
            if (pos_i < 0) {
                /**
                 * we found a dead end => switch to kill-mode
                 */

                DBUG_PRINT_TAG ("NTY_SPLIT", "--not found switching to kill mode");
                *pathes_remaining = 0;

            } else {
                pos = (size_t)pos_i;
                DBUG_PRINT_TAG ("NTY_SPLIT", "--found in position %zu", pos);
                son = SplitWrapperType (NTYPE_SON (type, pos), level + 1, frame,
                                        pathes_remaining);

                DBUG_PRINT_TAG ("NTY_SPLIT", "--adding " F_PTR " to " F_PTR,
                                (void *)son,
                                (void *)new_type);
                new_type = MakeNewSon (new_type, NULL);
                new_type = MakeNewSon (new_type, NULL);
                new_type = MakeNewSon (new_type, son);

                if (*pathes_remaining == 1) {
                    *pathes_remaining = (int)NTYPE_ARITY (type) - 2;
                    DBUG_PRINT_TAG ("NTY_SPLIT", "--deleting " F_PTR " from " F_PTR,
                                    (void *)NTYPE_SON (type, pos), (void *)type);
                    type = DeleteSon (type, pos);
                } else {
                    *pathes_remaining *= (int)NTYPE_ARITY (type) - 2;
                }
            }
            break;

        case TC_ibase:
            mandatory = 3;
            /* Falls through. */
        case TC_iarr:
        case TC_idim:
        case TC_ishape:
        case TC_ires:
            pathes_found = 0;
            for (i = 0; i < NTYPE_ARITY (type); i++) {
                *pathes_remaining = 1;
                son = SplitWrapperType (NTYPE_SON (type, i), level, frame,
                                        pathes_remaining);
                if ((*pathes_remaining > 0) || (i < mandatory)) {
                    /**
                     * NB: In certain situations NULL sons NEED to be inserted.
                     *     These are mandatory nodes such as [] and [*] in IBASE.
                     */
                    DBUG_PRINT_TAG ("NTY_SPLIT", "--adding " F_PTR " to " F_PTR, (void *)son,
                                    (void *)new_type);
                    new_type = MakeNewSon (new_type, son);
                    if (*pathes_remaining == 1) {
                        if (i >= mandatory) {
                            DBUG_PRINT_TAG ("NTY_SPLIT",
                                            "**deleting " F_PTR " from " F_PTR,
                                            (void *)NTYPE_SON (type, i), (void *)type);
                            type = DeleteSon (type, i);
                            /**
                             * ATTENTION: DeleteSons decrements our arity and thus the
                             * position of all sons that follow!
                             * therefore, we must not increment i !!!
                             * Instead we compensate the i++ from the for loop:
                             */
                            i--;
                        } else {
                            DBUG_PRINT_TAG ("NTY_SPLIT",
                                            "**setting " F_PTR " from " F_PTR " to NULL",
                                            (void *)NTYPE_SON (type, i), (void *)type);
                            NTYPE_SON (type, i) = NULL;
                        }
                    }
                }
                pathes_found
                  = (pathes_found < *pathes_remaining ? *pathes_remaining : pathes_found);
            }
            *pathes_remaining = pathes_found;
            break;
        default:
            new_type = MEMfree (new_type);
            new_type = TYcopyType (type);
        }

        if (*pathes_remaining == 0) {
            DBUG_PRINT_TAG ("NTY_SPLIT", "--killing %s node at" F_PTR,
                            dbug_str[NTYPE_CON (type)], (void *)new_type);
            new_type = MEMfree (new_type);
        } else if (*pathes_remaining == 1) {
            DBUG_PRINT_TAG ("NTY_SPLIT", "**freeing " F_PTR, (void *)type);
            type = TYfreeTypeConstructor (type);
        }
    }

    DBUG_RETURN (new_type);
}

ntype *
TYsplitWrapperType (ntype *type, int *pathes_remaining)
{
    size_t n;
    ntype **frame;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    if (NTYPE_CON (type) == TC_fun) {

        DBUG_EXECUTE_TAG ("NTY_SPLIT", tmp_str = TYtype2DebugString (type, TRUE, 20));

        DBUG_PRINT_TAG ("NTY_SPLIT", "wrapper is: %s", tmp_str);

        DBUG_EXECUTE_TAG ("NTY_SPLIT", tmp_str = MEMfree (tmp_str));

        n = TYgetArity (type);
        frame = (ntype **)MEMmalloc (n * sizeof (ntype *));
        ExtractTopBaseSignature (type, frame);

        *pathes_remaining = 1;

        type = SplitWrapperType (type, 0, frame, pathes_remaining);

        while (n > 0) {
            n--;
            frame[n] = MEMfree (frame[n]);
        }
        frame = MEMfree (frame);

        DBUG_EXECUTE_TAG ("NTY_SPLIT", tmp_str = TYtype2DebugString (type, TRUE, 20));

        DBUG_PRINT_TAG ("NTY_SPLIT", "wrapper split-off: %s", tmp_str);

        DBUG_EXECUTE_TAG ("NTY_SPLIT", tmp_str = MEMfree (tmp_str));
    } else {
        /**
         * we are dealing with a parameterless function here!
         */
        type = TYcopyType (type);
        *pathes_remaining = 1;
    }
    DBUG_RETURN (type);
}

/******************************************************************************
 *
 * Function:
 *   ntype *TYgetWrapperRetType( ntype *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

ntype *
TYgetWrapperRetType (ntype *type)
{
    ntype *ret_type;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "no type found!");

    if (TYisFun (type)) {
        DBUG_ASSERT (NTYPE_ARITY (type) == 3, "multiple FUN_IBASE found!");

        type = IRES_TYPE (IBASE_GEN (FUN_IBASE (type, 0)));
        DBUG_ASSERT (type != NULL, "IBASE_GEN not found!");
        ret_type = TYgetWrapperRetType (type);
    } else {
        DBUG_ASSERT (TYisProd (type), "neither TC_fun nor TC_prod found!");
        ret_type = type;
    }

    DBUG_RETURN (ret_type);
}

/******************************************************************************
 *
 * Function:
 *   node *TYcorrectWrapperArgTypes( node *args, ntype *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TYcorrectWrapperArgTypes (node *args, ntype *type)
{
    DBUG_ENTER ();

    if (args != NULL) {
        DBUG_ASSERT (NODE_TYPE (args) == N_arg, "no N_arg node found!");
        DBUG_ASSERT (TYisFun (type), "no TC_fun found!");
        DBUG_ASSERT (NTYPE_ARITY (type) == 3, "multiple FUN_IBASE found!");

        AVIS_TYPE (ARG_AVIS (args)) = TYfreeType (AVIS_TYPE (ARG_AVIS (args)));

        if (ARG_ISARTIFICIAL (args) || ARG_ISREFERENCE (args)
            || ARG_WASREFERENCE (args)) {
            AVIS_TYPE (ARG_AVIS (args))
              = TYmakeAKS (TYcopyType (IBASE_BASE (FUN_IBASE (type, 0))),
                           SHcreateShape (0));
        } else {
            AVIS_TYPE (ARG_AVIS (args))
              = TYmakeAUD (TYcopyType (IBASE_BASE (FUN_IBASE (type, 0))));
        }

        AVIS_DECLTYPE (ARG_AVIS (args)) = TYfreeType (AVIS_DECLTYPE (ARG_AVIS (args)));
        AVIS_DECLTYPE (ARG_AVIS (args)) = TYcopyType (AVIS_TYPE (ARG_AVIS (args)));

        type = IRES_TYPE (IBASE_GEN (FUN_IBASE (type, 0)));
        ARG_NEXT (args) = TYcorrectWrapperArgTypes (ARG_NEXT (args), type);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * Function:
 *   node *TYCreateWrapperCode( node *fundef, node *vardecs,
 *                              node **new_vardecs)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
Args2Exprs (node *args)
{
    node *exprs;

    DBUG_ENTER ();

    if (args != NULL) {
        DBUG_ASSERT (NODE_TYPE (args) == N_arg, "no N_arg found!");

        exprs = TBmakeExprs (TBmakeId (ARG_AVIS (args)), Args2Exprs (ARG_NEXT (args)));
    } else {
        exprs = NULL;
    }

    DBUG_RETURN (exprs);
}

#ifdef THIS_FUNCTION_WILL_EVER_BE_USED

static node *
ReferenceArgs2Ids (node *args)
{
    node *ret_ids;

    DBUG_ENTER ();

    if (args != NULL) {
        DBUG_ASSERT (NODE_TYPE (args) == N_arg, "no N_arg found!");

        if (ARG_ISREFERENCE (args)) {
            ret_ids = TBmakeIds (ARG_AVIS (args), ReferenceArgs2Ids (ARG_NEXT (args)));
        } else {
            ret_ids = ReferenceArgs2Ids (ARG_NEXT (args));
        }
    } else {
        ret_ids = NULL;
    }

    DBUG_RETURN (ret_ids);
}

#endif /* THIS_FUNCTION_WILL_EVER_BE_USED */

static node *
BuildTmpId (ntype *type, node **new_vardecs)
{
    node *id, *avis;

    DBUG_ENTER ();

    avis = TBmakeAvis (TRAVtmpVar (), type);
    id = TBmakeId (avis);
    *new_vardecs = TBmakeVardec (avis, *new_vardecs);

    DBUG_RETURN (id);
}

static node *
BuildTmpIds (ntype *type, node **new_vardecs)
{
    node *ids, *avis;

    DBUG_ENTER ();

    avis = TBmakeAvis (TRAVtmpVar (), type);
    ids = TBmakeIds (avis, NULL);
    *new_vardecs = TBmakeVardec (avis, *new_vardecs);

    DBUG_RETURN (ids);
}

static node *
GetPrfOrFundef (node *assigns)
{
    node *res;

    DBUG_ENTER ();

    DBUG_ASSERT (((assigns != NULL) && (NODE_TYPE (assigns) == N_assign)),
                 "no assignment found!");

    if ((ASSIGN_NEXT (assigns) == NULL) && (NODE_TYPE (ASSIGN_STMT (assigns)) == N_let)) {
        if (NODE_TYPE (LET_EXPR (ASSIGN_STMT (assigns))) == N_prf) {
            res = LET_EXPR (ASSIGN_STMT (assigns));
        } else if (NODE_TYPE (LET_EXPR (ASSIGN_STMT (assigns))) == N_ap) {
            res = AP_FUNDEF (LET_EXPR (ASSIGN_STMT (assigns)));
            DBUG_ASSERT (res != NULL, "AP_FUNDEF not found!");
            DBUG_ASSERT (NODE_TYPE (res) == N_fundef, "no N_fundef node found!");
        } else {
            res = NULL;
        }
    } else {
        res = NULL;
    }

    DBUG_RETURN (res);
}

static bool
BranchesAreEquivalent (node *assigns1, node *assigns2)
{
    node *prf_or_fundef1, *prf_or_fundef2;
    bool res;

    DBUG_ENTER ();

    prf_or_fundef1 = GetPrfOrFundef (assigns1);
    prf_or_fundef2 = GetPrfOrFundef (assigns2);

    if ((prf_or_fundef1 != NULL) && (prf_or_fundef2 != NULL)) {
        if ((NODE_TYPE (prf_or_fundef1) == N_prf)
            && (NODE_TYPE (prf_or_fundef2) == N_prf)) {
            DBUG_ASSERT ((((PRF_PRF (prf_or_fundef1) == F_dispatch_error)
                           || (PRF_PRF (prf_or_fundef1) == F_type_error))
                          && ((PRF_PRF (prf_or_fundef2) == F_dispatch_error)
                              || (PRF_PRF (prf_or_fundef2) == F_type_error))),
                         "illegal prf found!");
            res = PRF_PRF (prf_or_fundef1) == PRF_PRF (prf_or_fundef2);
        } else if ((NODE_TYPE (prf_or_fundef1) == N_fundef)
                   && (NODE_TYPE (prf_or_fundef2) == N_fundef)) {
            res = (prf_or_fundef1 == prf_or_fundef2);
        } else {
            res = FALSE;
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

static node *
BuildDimAssign (node *arg, node **new_vardecs)
{
    node *assign;
    node *preassign = NULL;
    node *dim;
    ntype *type;
    node *dimnum;

    DBUG_ENTER ();

    /*
     * the generated dim statement has to calculate the
     * dimensionality of the given argument A. In case of
     * a built in type this can be done by
     *
     * assign = _dim_( A)
     *
     * in case of a user defined type, _dim_ returns the
     * dimensionality of the inner type. Thus the dimensionality
     * of A wrt. the user defined type can be calculated by
     *
     * assign = _sub_SxS( _dim_( A), _dim_( base( A)))
     *
     * where _dim_( base( A)) is statically known
     *
     * As the user-defined types might have been resolved by now,
     * we have to use the declared type here!
     *
     */
    type = AVIS_DECLTYPE (ARG_AVIS (arg));

    if (TYisArray (type)) {
        type = TYgetScalar (type);
    }

    if (TYisUser (type)) {
        ntype *basetype = UTgetBaseType (TYgetUserType (type));

        if (TYisArray (basetype)) {
            preassign
              = TBmakeAssign (TBmakeLet (BuildTmpIds (TYmakeAKS (TYmakeSimpleType (T_int),
                                                                 SHcreateShape (0)),
                                                      new_vardecs),
                                         TCmakePrf1 (F_dim_A, TBmakeId (ARG_AVIS (arg)))),
                              NULL);

            dimnum = TBmakeNum (TYgetDim (basetype));
            dimnum = FLATGexpression2Avis (dimnum, new_vardecs, &preassign, NULL);

            dim = TCmakePrf2 (F_sub_SxS, TBmakeId (IDS_AVIS (ASSIGN_LHS (preassign))),
                              TBmakeId (dimnum));
        } else {
            dim = TBmakePrf (F_dim_A, TBmakeExprs (TBmakeId (ARG_AVIS (arg)), NULL));
        }
    } else {
        dim = TBmakePrf (F_dim_A, TBmakeExprs (TBmakeId (ARG_AVIS (arg)), NULL));
    }

    assign = TBmakeAssign (TBmakeLet (BuildTmpIds (TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (0)),
                                                   new_vardecs),
                                      dim),
                           NULL);

    assign = TCappendAssign (preassign, assign);

    DBUG_RETURN (assign);
}

static node *
BuildShapeAssign (node *arg, node **new_vardecs)
{
    node *assign;
    node *preassign = NULL;
    node *shape;
    ntype *type;
    node *dimnum;
    node *lhsid;

    DBUG_ENTER ();

    /*
     * the generated shape statement has to calculate the
     * shape of the given argument A. In case of a built-in
     * type this can be done by
     *
     * assign = _shape_( A)
     *
     * In case of a user-defined type, the prf _shape_
     * computes the shape of the inner type. Thus to
     * yield the shape of the udt:
     *
     * assign = drop( - _dim_( base( A)), _shape_( A))
     *
     * As the user types might have been resolved by now,
     * we have to use the declared type here!
     */
    type = AVIS_DECLTYPE (ARG_AVIS (arg));

    if (TYisArray (type)) {
        type = TYgetScalar (type);
    }

    if (TYisUser (type)) {
        ntype *basetype = UTgetBaseType (TYgetUserType (type));

        if (TYisArray (basetype)) {
            preassign
              = TBmakeAssign (TBmakeLet (BuildTmpIds (TYmakeAKD (TYmakeSimpleType (T_int),
                                                                 1, SHcreateShape (0)),
                                                      new_vardecs),
                                         TCmakePrf1 (F_shape_A,
                                                     TBmakeId (ARG_AVIS (arg)))),
                              NULL);

            lhsid = TBmakeId (IDS_AVIS (ASSIGN_LHS (preassign)));
            dimnum = TBmakeNum (TYgetDim (basetype));
            dimnum = FLATGexpression2Avis (dimnum, new_vardecs, &preassign, NULL);
            shape = TCmakePrf2 (F_drop_SxV, TBmakeId (dimnum), lhsid);
        } else {
            shape = TBmakePrf (F_shape_A, TBmakeExprs (TBmakeId (ARG_AVIS (arg)), NULL));
        }
    } else {
        shape = TBmakePrf (F_shape_A, TBmakeExprs (TBmakeId (ARG_AVIS (arg)), NULL));
    }

    assign = TBmakeAssign (TBmakeLet (BuildTmpIds (TYmakeAUDGZ (TYmakeSimpleType (T_int)),
                                                   new_vardecs),
                                      shape),
                           NULL);

    assign = TCappendAssign (preassign, assign);

    DBUG_RETURN (assign);
}

static node *
BuildCondAssign (node *prf_ass, prf rel_prf, node *expr, node *then_ass, node *else_ass,
                 node **new_vardecs)
{
    prf prf;
    node *assigns = NULL;
    node *prf_ids;

    DBUG_ENTER ();

    if (BranchesAreEquivalent (then_ass, else_ass)) {
        /*
         * both parts of the conditional contain the same code
         *   -> no need to build the conditional!
         */
        assigns = then_ass;
        else_ass = FREEdoFreeTree (else_ass);
    } else {
        /*
         * first go to the final assign in the given
         * assigns chain (as a dim on a udt consists of
         * more than one operation)
         */
        while (ASSIGN_NEXT (prf_ass) != NULL) {
            prf_ass = ASSIGN_NEXT (prf_ass);
        }

        prf = PRF_PRF (ASSIGN_RHS (prf_ass));
        prf_ids = ASSIGN_LHS (prf_ass);

        switch (prf) {
        case F_sub_SxS: /* last op in dim on udt */
        case F_dim_A: {
            node *prf2;
            node *prf_ids2, *prf_ids3;
            node *id, *id2, *id3;

            DBUG_ASSERT (NODE_TYPE (expr) == N_num, "illegal expression found!");

            id = DUPdupIdsId (prf_ids);

            prf_ids3
              = BuildTmpIds (TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)),
                             new_vardecs);
            id3 = DUPdupIdsId (prf_ids3);

            prf2 = TBmakePrf (rel_prf, TBmakeExprs (id, TBmakeExprs (id3, NULL)));
            prf_ids2
              = BuildTmpIds (TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)),
                             new_vardecs);
            id2 = DUPdupIdsId (prf_ids2);

            assigns = TBmakeAssign (
              TBmakeLet (prf_ids3, expr),
              TBmakeAssign (TBmakeLet (prf_ids2, prf2),
                            TBmakeAssign (TBmakeCond (id2, TBmakeBlock (then_ass, NULL),
                                                      TBmakeBlock (else_ass, NULL)),
                                          NULL)));
        } break;

        case F_drop_SxV: /* last op of shape on udts */
        case F_shape_A: {
            node *prf2, *prf3, *prf4, *array;
            node *flt_prf2, *flt_prf3, *flt_prf4, *flt_array;
            node *aexprs;
            node *id;
            node *el;
            int dim;

            DBUG_ASSERT (NODE_TYPE (expr) == N_array, "illegal expression found!");

            aexprs = ARRAY_AELEMS (expr);
            dim = 0;
            flt_prf4
              = BuildTmpId (TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)),
                            new_vardecs);
            assigns
              = TCappendAssign (assigns, TBmakeAssign (TBmakeLet (DUPdupIdIds (flt_prf4),
                                                                  TBmakeBool (TRUE)),
                                                       NULL));

            while (aexprs != NULL) {
                id = DUPdupIdsId (prf_ids);
                array = TCmakeIntVector (TBmakeExprs (TBmakeNum (dim), NULL));
                flt_array = BuildTmpId (TYmakeAKV (TYmakeSimpleType (T_int),
                                                   COaST2Constant (array)),
                                        new_vardecs);

                prf2 = TBmakePrf (F_sel_VxA,
                                  TBmakeExprs (flt_array, TBmakeExprs (id, NULL)));
                flt_prf2
                  = BuildTmpId (TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)),
                                new_vardecs);

                el = EXPRS_EXPR (aexprs);
                el = FLATGexpression2Avis (el, new_vardecs, &assigns, NULL);
                prf3
                  = TBmakePrf (rel_prf,
                               TBmakeExprs (flt_prf2, TBmakeExprs (TBmakeId (el), NULL)));
                flt_prf3
                  = BuildTmpId (TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)),
                                new_vardecs);

                prf4 = TBmakePrf (F_and_SxS,
                                  TBmakeExprs (flt_prf3, TBmakeExprs (flt_prf4, NULL)));
                /*
                 * cg: 11.6.07
                 *
                 * I'm not 100% sure if the scalar version is correct above.
                 */

                flt_prf4
                  = BuildTmpId (TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)),
                                new_vardecs);

                assigns = TCappendAssign (
                  assigns,
                  TBmakeAssign (
                    TBmakeLet (DUPdupIdIds (flt_array), array),
                    TBmakeAssign (TBmakeLet (DUPdupIdIds (flt_prf2), prf2),
                                  TBmakeAssign (TBmakeLet (DUPdupIdIds (flt_prf3), prf3),
                                                TBmakeAssign (TBmakeLet (DUPdupIdIds (
                                                                           flt_prf4),
                                                                         prf4),
                                                              NULL)))));
                aexprs = EXPRS_NEXT (aexprs);
                dim++;
            }

            assigns
              = TCappendAssign (assigns,
                                TBmakeAssign (TBmakeCond (flt_prf4,
                                                          TBmakeBlock (then_ass, NULL),
                                                          TBmakeBlock (else_ass, NULL)),
                                              NULL));
            ARRAY_AELEMS (expr) = NULL;
            expr = FREEdoFreeNode (expr);
        } break;

        default:
            DBUG_UNREACHABLE ("illegal prf found!");
            assigns = NULL;
            break;
        }
    }

    DBUG_RETURN (assigns);
}

static node *
BuildApAssign (node *fundef, node *args, node *vardecs, node **new_vardecs)
{
    node *assigns;
    node *ap;
    node *tmp_id;
    node *lhs;
    node *tmp_ids;
    ntype *ret_type;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef found!");

    assigns = NULL;
    lhs = NULL;
    ret_type = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
    //loop from ARITY - 1 to 0 
    for (i = NTYPE_ARITY (ret_type); i-- > 0;){
        DBUG_ASSERT (vardecs != NULL, "inconsistant application found");

        tmp_id = BuildTmpId (TYcopyType (PROD_MEMBER (ret_type, i)), new_vardecs);
        assigns
          = TBmakeAssign (TBmakeLet (TBmakeIds (VARDEC_AVIS (vardecs), NULL), tmp_id),
                          assigns);

        tmp_ids = DUPdupIdIds (tmp_id);
        IDS_NEXT (tmp_ids) = lhs;
        lhs = tmp_ids;

        vardecs = VARDEC_NEXT (vardecs);
    }
    DBUG_ASSERT (vardecs == NULL, "inconsistant application found");

    ap = TBmakeAp (fundef, Args2Exprs (args));

    assigns = TBmakeAssign (TBmakeLet (lhs, ap), assigns);

    DBUG_RETURN (assigns);
}

static node *
BuildDispatchErrorAssign (char *funname, node *args, node *rets, node *vardecs)
{
    node *assigns = NULL;
    node *exprs;
    node *retcount;

    DBUG_ENTER ();

    exprs = TBmakeExprs (TCmakeStrCopy (funname), Args2Exprs (args));
    exprs = TCappendExprs (TUmakeTypeExprsFromRets (rets), exprs);
    retcount = TBmakeNum (TCcountRets (rets));
    // ?? retcount = FLATGexpression2Avis( retcount, &vardecs, &assigns, NULL);
    exprs = TBmakeExprs (retcount, exprs);

    assigns = TBmakeAssign (TBmakeLet (TCmakeIdsFromVardecs (vardecs),
                                       TBmakePrf (F_dispatch_error, exprs)),
                            assigns);

    DBUG_RETURN (assigns);
}

static node *
BuildTypeErrorAssign (ntype *bottom, node *vardecs)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TBmakeAssign (TBmakeLet (TCmakeIdsFromVardecs (vardecs),
                                       TCmakePrf1 (F_type_error, TBmakeType (bottom))),
                            NULL);

    DBUG_RETURN (assigns);
}

static bool
IsRelevant (ntype *type)
{
    bool ret;
    ntype *ires;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "no type found!");

    switch (TYgetConstr (type)) {
    case TC_iarr:
        ires = IARR_GEN (type);
        break;

    case TC_idim:
        ires = IDIM_GEN (type);
        break;

    case TC_ishape:
        ires = ISHAPE_GEN (type);
        break;

    default:
        DBUG_UNREACHABLE ("illegal ntype constructor found!");
        ires = NULL;
        break;
    }
    DBUG_ASSERT (ires != NULL, "I..._GEN not found!");

    ret = FALSE;
    for (i = 0; i < IRES_NUMFUNS (ires); i++) {
        if (IRES_POS (ires, i) == 0) {
            ret = TRUE;
        }
    }

    DBUG_RETURN (ret);
}

static node *
CreateWrapperCode (ntype *type, dft_state *state, int lower, char *funname, node *arg,
                   node *args, node *rets, node *vardecs, node **new_vardecs)
{
    node *assigns;
    node *tmp_ass;
    size_t i;
    node *dimnum;
#ifndef DBUG_OFF
    char *dbug_str = NULL;
#endif

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "no type found!");

    DBUG_EXECUTE (dbug_str = TYtype2DebugString (type, TRUE, 0));
    DBUG_PRINT ("building wrapper for type: %s\n", dbug_str);
    DBUG_EXECUTE (dbug_str = MEMfree (dbug_str));

    switch (TYgetConstr (type)) {
    case TC_fun:
        DBUG_ASSERT (NTYPE_ARITY (type) == 3, "multipe FUN_IBASE found!");
        assigns = CreateWrapperCode (FUN_IBASE (type, 0), state, lower, funname, arg,
                                     args, rets, vardecs, new_vardecs);
        break;

    case TC_ibase:
        if (ARG_ISREFERENCE (arg) || ARG_ISARTIFICIAL (arg) || ARG_WASREFERENCE (arg)) {
            /*
             * this should identify all legal unique argument types!
             */
            DBUG_ASSERT (IBASE_SCAL (type) != NULL,
                         "fun without instance for scalar unique argument found");
            assigns = CreateWrapperCode (IBASE_SCAL (type), state, 0, funname, arg, args,
                                         rets, vardecs, new_vardecs);
        } else {

            DBUG_ASSERT (IBASE_GEN (type) != NULL, "IBASE_GEN not found!");
            if (IBASE_IARR (type) != NULL) {
                assigns = CreateWrapperCode (IBASE_IARR (type), state, lower, funname,
                                             arg, args, rets, vardecs, new_vardecs);
                if (IsRelevant (IBASE_IARR (type))) {
                    /*
                     * this conditional is needed only iff an instance with type [+]
                     * exists
                     */
                    tmp_ass = BuildDimAssign (arg, new_vardecs);
                    dimnum = TBmakeNum (0);
                    // dimnum = FLATGexpression2Avis( dimnum, new_vardecs, &assigns,
                    // NULL);
                    assigns
                      = BuildCondAssign (tmp_ass, F_gt_SxS, dimnum, assigns,
                                         CreateWrapperCode (IBASE_GEN (type), state, 3,
                                                            funname, arg, args, rets,
                                                            vardecs, new_vardecs),
                                         new_vardecs);
                    assigns = TCappendAssign (tmp_ass, assigns);
                }
            } else {
                assigns = CreateWrapperCode (IBASE_GEN (type), state, 3, funname, arg,
                                             args, rets, vardecs, new_vardecs);
            }
            if (IBASE_SCAL (type) != NULL) {
                tmp_ass = BuildDimAssign (arg, new_vardecs);
                dimnum = TBmakeNum (0);
                // dimnum = FLATGexpression2Avis( dimnum, new_vardecs, &assigns, NULL);
                assigns = BuildCondAssign (tmp_ass, F_eq_SxS, dimnum,
                                           CreateWrapperCode (IBASE_SCAL (type), state, 0,
                                                              funname, arg, args, rets,
                                                              vardecs, new_vardecs),
                                           assigns, new_vardecs);
                assigns = TCappendAssign (tmp_ass, assigns);
            }
        }
        break;

    case TC_iarr:
        DBUG_ASSERT (IARR_GEN (type) != NULL, "IARR_GEN not found!");
        assigns = CreateWrapperCode (IARR_GEN (type), state, 2, funname, arg, args, rets,
                                     vardecs, new_vardecs);

        if (NTYPE_ARITY (type) >= 2) {
            tmp_ass = BuildDimAssign (arg, new_vardecs);
            /* 
             * decrement after check for > 0, safe method for reverse loop ending on 0
             * i : (arity - 2) to 0
             */
            for (i = NTYPE_ARITY (type) - 1; i-- > 0; ) {
                if (IARR_IDIM (type, i) != NULL) {
                    dimnum = TBmakeNum (IDIM_DIM (IARR_IDIM (type, i)));
                    // dimnum = FLATGexpression2Avis( dimnum, new_vardecs, &assigns,
                    // NULL);
                    assigns
                      = BuildCondAssign (tmp_ass, F_eq_SxS, dimnum,
                                         CreateWrapperCode (IARR_IDIM (type, i), state,
                                                            lower, funname, arg, args,
                                                            rets, vardecs, new_vardecs),
                                         assigns, new_vardecs);
                }
            }
            assigns = TCappendAssign (tmp_ass, assigns);
        }
        break;

    case TC_idim:
        DBUG_ASSERT (IDIM_GEN (type) != NULL, "IDIM_GEN not found!");
        assigns = CreateWrapperCode (IDIM_GEN (type), state, 1, funname, arg, args, rets,
                                     vardecs, new_vardecs);

        if (NTYPE_ARITY (type) >= 2) {
            tmp_ass = BuildShapeAssign (arg, new_vardecs);
            /* 
             * decrement after check for > 0, safe method for reverse loop ending on 0
             * i : (arity - 2) to 0
             */
            for (i = NTYPE_ARITY (type) - 1; i-- > 0; ) {
                if (IDIM_ISHAPE (type, i) != NULL) {
                    assigns
                      = BuildCondAssign (tmp_ass, F_eq_SxS,
                                         SHshape2Array (
                                           ISHAPE_SHAPE (IDIM_ISHAPE (type, i))),
                                         CreateWrapperCode (IDIM_ISHAPE (type, i), state,
                                                            lower, funname, arg, args,
                                                            rets, vardecs, new_vardecs),
                                         assigns, new_vardecs);
                }
            }
            assigns = TCappendAssign (tmp_ass, assigns);
        }
        break;

    case TC_ishape:
        DBUG_ASSERT (ISHAPE_GEN (type) != NULL, "ISHAPE_GEN not found!");
        assigns = CreateWrapperCode (ISHAPE_GEN (type), state, 0, funname, arg, args,
                                     rets, vardecs, new_vardecs);
        break;

    case TC_ires:
        if (state == NULL) {
            state = AllocDFT_state (IRES_NUMFUNS (type));
            state = InsertFirstArgDFT_state (state, type, lower);
        } else {
            state = CopyDFT_state (state); /* VERY ugly indeed ... */
            state = InsertNextArgDFT_state (state, type, lower);
        }

        if (state->cnt_funs <= 0) {
            assigns = BuildDispatchErrorAssign (funname, args, rets, vardecs);
        } else if (TYisProd (IRES_TYPE (type))) {
            dft_res *res;
            node *fundef;

            state = FinalizeDFT_state (state);
            res = DFT_state2dft_res (state);
            DBUG_ASSERT (((res->num_partials == 0)
                          && (res->num_deriveable_partials == 0)),
                         "partials found!");
            if (res->def != NULL) {
                DBUG_ASSERT (res->deriveable == NULL, "def and deriveable found!");
                fundef = res->def;
            } else {
                fundef = res->deriveable;
            }

            if (fundef == NULL) {
                assigns = BuildDispatchErrorAssign (funname, args, rets, vardecs);
            } else if (FUNDEF_ISTYPEERROR (fundef)) {
                ntype *bottom = TUcombineBottomsFromRets (FUNDEF_RETS (fundef));

                assigns = BuildTypeErrorAssign (bottom, vardecs);
            } else {
                assigns = BuildApAssign (fundef, args, vardecs, new_vardecs);
            }

            res = TYfreeDft_res (res);
        } else {
            assigns
              = CreateWrapperCode (IRES_TYPE (type), state, lower, funname,
                                   ARG_NEXT (arg), args, rets, vardecs, new_vardecs);
        }

        state = freeDFT_state (state);
        break;

    default:
        DBUG_UNREACHABLE ("illegal ntype constructor found!");
        assigns = NULL;
        break;
    }

    DBUG_RETURN (assigns);
}

node *
TYcreateWrapperCode (node *fundef, node *vardecs, node **new_vardecs)
{
    node *assigns = NULL;
    char *funsig;
    char *tmp;

    DBUG_ENTER ();

    if (FUNDEF_ARGS (fundef) == NULL) {
        DBUG_ASSERT (FUNDEF_IMPL (fundef) != NULL, "FUNDEF_IMPL not found!");
        assigns = BuildApAssign (FUNDEF_IMPL (fundef), FUNDEF_ARGS (fundef), vardecs,
                                 new_vardecs);
    } else {
        DBUG_ASSERT (!FUNDEF_HASDOTRETS (fundef),
                     "wrapper function with ... return type found!");
        DBUG_ASSERT (!FUNDEF_HASDOTARGS (fundef),
                     "wrapper function with ... argument found!");

        tmp = TUtypeSignature2String (fundef);
        funsig = (char *)MEMmalloc (sizeof (char)
                                    * (STRlen (CTIitemName (fundef)) + STRlen (tmp) + 5));
        sprintf (funsig, "%s :: %s", CTIitemName (fundef), tmp);

        assigns = CreateWrapperCode (FUNDEF_WRAPPERTYPE (fundef), NULL, 0, funsig,
                                     FUNDEF_ARGS (fundef), FUNDEF_ARGS (fundef),
                                     FUNDEF_RETS (fundef), vardecs, new_vardecs);

        tmp = MEMfree (tmp);
        funsig = MEMfree (funsig);
    }

    DBUG_RETURN (assigns);
}

/**
 ** Serialization support
 **/

static void
SerializeSimpleType (FILE *file, ntype *type)
{
    char *funname;

    DBUG_ENTER ();

    if (SIMPLE_HIDDEN_UDT (type) != UT_NOT_DEFINED) {
        node *tdef;

        tdef = UTgetTdef (SIMPLE_HIDDEN_UDT (type));

        funname = SERgetSerFunName (tdef);

        fprintf (file, "TYdeserializeType( %d, %d, 1, \"%s\", ", NTYPE_CON (type),
                 SIMPLE_TYPE (type), funname);

        MEMfree (funname);

        NSserializeNamespace (file, UTgetNamespace (SIMPLE_HIDDEN_UDT (type)));

        fprintf (file, ")");
    } else {
        fprintf (file, "TYdeserializeType( %d, %d, 0)", NTYPE_CON (type),
                 SIMPLE_TYPE (type));
    }

    DBUG_RETURN ();
}

static void
SerializeSymbolType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, \"%s\", \"%s\")", NTYPE_CON (type),
             SYMBOL_NAME (type), NSgetName (SYMBOL_NS (type)));

    DBUG_RETURN ();
}

static void
SerializeUserType (FILE *file, ntype *type)
{
    node *tdef;
    char *funname;

    DBUG_ENTER ();

    /*
     * we have to get the symbolname here. One could move
     * this functionality to serialize, but on the other
     * hand I think it is better to have it all in once
     * place for all the ntypes.
     */
    tdef = UTgetTdef (USER_TYPE (type));

    funname = SERgetSerFunName (tdef);

    fprintf (file, "TYdeserializeType( %d, \"%s\", ", NTYPE_CON (type), funname);

    MEMfree (funname);

    NSserializeNamespace (file, UTgetNamespace (USER_TYPE (type)));

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeAKVType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, ", NTYPE_CON (type));

    TYserializeType (file, AKV_BASE (type));

    fprintf (file, ", ");

    COserializeConstant (file, AKV_CONST (type));

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeAKSType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, ", NTYPE_CON (type));

    TYserializeType (file, AKS_BASE (type));

    fprintf (file, ", ");

    SHserializeShape (file, AKS_SHP (type));

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeAKDType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, ", NTYPE_CON (type));

    TYserializeType (file, AKD_BASE (type));

    fprintf (file, ", %zu, ", AKD_DOTS (type));

    SHserializeShape (file, AKD_SHP (type));

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeAUDType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, ", NTYPE_CON (type));

    TYserializeType (file, AUD_BASE (type));

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeAUDGZType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, ", NTYPE_CON (type));

    TYserializeType (file, AUDGZ_BASE (type));

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeProdType (FILE *file, ntype *type)
{
    size_t cnt;

    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, %zu", NTYPE_CON (type), NTYPE_ARITY (type));

    for (cnt = 0; cnt < NTYPE_ARITY (type); cnt++) {
        fprintf (file, ", ");

        TYserializeType (file, PROD_MEMBER (type, cnt));
    }

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeUnionType (FILE *file, ntype *type)
{
    size_t cnt;

    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, %zu", NTYPE_CON (type), NTYPE_ARITY (type));

    for (cnt = 0; cnt < NTYPE_ARITY (type); cnt++) {
        fprintf (file, ", ");

        TYserializeType (file, UNION_MEMBER (type, cnt));
    }

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeFunType (FILE *file, ntype *type)
{
    size_t cnt;

    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, %zu", NTYPE_CON (type), NTYPE_ARITY (type));

    for (cnt = 0; cnt < NTYPE_ARITY (type); cnt++) {
        fprintf (file, ", ");

        TYserializeType (file, NTYPE_SON (type, cnt));
    }

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeIBaseType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, ", NTYPE_CON (type));

    TYserializeType (file, IBASE_BASE (type));

    fprintf (file, ", ");

    TYserializeType (file, IBASE_SCAL (type));

    fprintf (file, ", ");

    TYserializeType (file, IBASE_GEN (type));

    fprintf (file, ", ");

    TYserializeType (file, IBASE_IARR (type));

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeIArrType (FILE *file, ntype *type)
{
    size_t cnt;

    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, %zu, ", NTYPE_CON (type), NTYPE_ARITY (type));

    TYserializeType (file, IARR_GEN (type));
    
    for (cnt = 0; NTYPE_ARITY (type) > 0 && cnt < NTYPE_ARITY (type) - 1; cnt++) {
        fprintf (file, ", ");

        TYserializeType (file, IARR_IDIM (type, cnt));
    }

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeIDimType (FILE *file, ntype *type)
{
    size_t cnt;

    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, %zu, %zu, ", NTYPE_CON (type),
             NTYPE_ARITY (type), IDIM_DIM (type));

    TYserializeType (file, IDIM_GEN (type));

    for (cnt = 0; NTYPE_ARITY (type) > 0 && cnt < NTYPE_ARITY (type) - 1; cnt++) {
        fprintf (file, ", ");

        TYserializeType (file, IDIM_ISHAPE (type, cnt));
    }

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeIShapeType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, ", NTYPE_CON (type));

    SHserializeShape (file, ISHAPE_SHAPE (type));

    fprintf (file, ", ");

    TYserializeType (file, ISHAPE_GEN (type));

    fprintf (file, ")");

    DBUG_RETURN ();
}

static void
SerializeIResType (FILE *file, ntype *type)
{
    size_t cnt;

    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, %zu", NTYPE_CON (type), IRES_NUMFUNS (type));

    for (cnt = 0; cnt < IRES_NUMFUNS (type); cnt++) {
        fprintf (file, ", ");
        SERserializeFundefLink (IRES_FUNDEF (type, cnt), file);
    }

    for (cnt = 0; cnt < IRES_NUMFUNS (type); cnt++) {
        fprintf (file, ", %d", IRES_POS (type, cnt));
    }

    fprintf (file, ", ");

    TYserializeType (file, IRES_TYPE (type));

    fprintf (file, ") ");

    DBUG_RETURN ();
}

static void
SerializeAlphaType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    /*
    DBUG_UNREACHABLE (
        "Cannot handle alpha types");
        */

    fprintf (file, "NULL");

    DBUG_RETURN ();
}

static void
SerializePolyType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, \"%s\")", NTYPE_CON (type), POLY_NAME (type));

    DBUG_RETURN ();
}

static void
SerializePolyUserType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d, \"%s\", \"%s\", \"%s\", %d, %d)",
             NTYPE_CON (type), POLYUSER_OUTER (type), POLYUSER_INNER (type),
             POLYUSER_SHAPE (type), POLYUSER_DENEST (type), POLYUSER_RENEST (type));

    DBUG_RETURN ();
}

static void
SerializeBottomType (FILE *file, ntype *type)
{
    char *tmp;

    DBUG_ENTER ();

    tmp = STRstring2SafeCEncoding (BOTTOM_MSG (type));

    fprintf (file, "TYdeserializeType( %d, \"%s\")", NTYPE_CON (type), tmp);

    tmp = MEMfree (tmp);

    DBUG_RETURN ();
}

static void
SerializeDummyType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    fprintf (file, "TYdeserializeType( %d)", NTYPE_CON (type));

    DBUG_RETURN ();
}

void
TYserializeType (FILE *file, ntype *type)
{
    DBUG_ENTER ();

    if (type == NULL) {
        DBUG_PRINT_TAG ("SET", "Processing type (null)");

        fprintf (file, "NULL");

        DBUG_PRINT_TAG ("SET", "Done processing type (null)");
    } else {
        DBUG_PRINT_TAG ("SET", "Processing type %s", dbug_str[NTYPE_CON (type)]);

        switch (NTYPE_CON (type)) {
        case TC_simple:
            SerializeSimpleType (file, type);
            break;
        case TC_symbol:
            SerializeSymbolType (file, type);
            break;
        case TC_user:
            SerializeUserType (file, type);
            break;
        case TC_akv:
            SerializeAKVType (file, type);
            break;
        case TC_aks:
            SerializeAKSType (file, type);
            break;
        case TC_akd:
            SerializeAKDType (file, type);
            break;
        case TC_aud:
            SerializeAUDType (file, type);
            break;
        case TC_audgz:
            SerializeAUDGZType (file, type);
            break;
        case TC_prod:
            SerializeProdType (file, type);
            break;
        case TC_union:
            SerializeUnionType (file, type);
            break;
        case TC_fun:
            SerializeFunType (file, type);
            break;
        case TC_ibase:
            SerializeIBaseType (file, type);
            break;
        case TC_iarr:
            SerializeIArrType (file, type);
            break;
        case TC_idim:
            SerializeIDimType (file, type);
            break;
        case TC_ishape:
            SerializeIShapeType (file, type);
            break;
        case TC_ires:
            SerializeIResType (file, type);
            break;
        case TC_alpha:
            SerializeAlphaType (file, type);
            break;
        case TC_poly:
            SerializePolyType (file, type);
            break;
        case TC_polyuser:
            SerializePolyUserType (file, type);
            break;
        case TC_bottom:
            SerializeBottomType (file, type);
            break;
        case TC_dummy:
            SerializeDummyType (file, type);
            break;
        }

        DBUG_PRINT_TAG ("SET", "Done processing type %s", dbug_str[NTYPE_CON (type)]);
    }

    DBUG_RETURN ();
}

/**
 ** De-Serialization support
 **/

#if IS_CYGWIN
ntype *
TYdeserializeType (int _con, ...)
{
    ntype *result = NULL;
    va_list Argp;

    DBUG_ENTER ();

    va_start (Argp, _con);
    result = TYdeserializeTypeVa (_con, Argp);
    va_end (Argp);

    DBUG_RETURN (result);
}

ntype *
TYdeserializeTypeVa (int _con, va_list args)
{
#else
ntype *
TYdeserializeType (int _con, ...)
{
    va_list args;
#endif /* IS_CYGWIN */
    ntype *result = NULL;
    size_t cnt;
    typeconstr con = (typeconstr)_con;

    simpletype st;

    DBUG_ENTER ();

    DBUG_PRINT_TAG ("SER", "Deserializing ntype %s", dbug_str[con]);

#if !IS_CYGWIN
    va_start (args, _con);
#endif

    switch (con) {
    case TC_simple: {
        usertype udt;
        char *symid;
        namespace_t *ns;
        bool has_hidden;

        st = (simpletype)va_arg (args, int);
        has_hidden = (bool)va_arg (args, int);

        if (has_hidden) {
            DBUG_ASSERT (st == T_hidden, "Found hidden udt in non hidden type!");

            symid = va_arg (args, char *);
            ns = va_arg (args, namespace_t *);

            udt = DSloadUserType (symid, ns);

            result = TYmakeHiddenSimpleType (udt);
        } else {
            if (st == T_hidden) {
                result = TYmakeHiddenSimpleType (UT_NOT_DEFINED);
            } else {
                result = TYmakeSimpleType (st);
            }
        }
    } break;
    case TC_symbol: {
        char *name;
        namespace_t *ns;

        name = va_arg (args, char *);
        ns = va_arg (args, namespace_t *);

        result = TYmakeSymbType (STRcpy (name), ns);
    } break;
    case TC_user: {
        char *symid;
        namespace_t *ns;
        usertype udt;

        symid = va_arg (args, char *);
        ns = va_arg (args, namespace_t *);

        udt = DSloadUserType (symid, ns);

        result = TYmakeUserType (udt);
    } break;
    case TC_akv: {
        ntype *type;
        constant *cnst;

        type = va_arg (args, ntype *);
        cnst = va_arg (args, constant *);

        result = TYmakeAKV (type, cnst);
    } break;
    case TC_aks: {
        ntype *type;
        shape *shp;

        type = va_arg (args, ntype *);
        shp = va_arg (args, shape *);

        result = TYmakeAKS (type, shp);
    } break;
    case TC_akd: {
        ntype *type;
        ssize_t dim;
        shape *shp;

        type = va_arg (args, ntype *);
        dim = va_arg (args, ssize_t);
        shp = va_arg (args, shape *);
        DBUG_ASSERT (dim >= 0, "TC_akd va_arg was negative, dim must be 0+");
        result = TYmakeAKD (type, (size_t)dim, shp);
    } break;
    case TC_aud: {
        result = TYmakeAUD (va_arg (args, ntype *));
    } break;
    case TC_audgz: {
        result = TYmakeAUDGZ (va_arg (args, ntype *));
    } break;
    case TC_prod: {
        result = MakeNtype (TC_prod, va_arg (args, size_t));

        for (cnt = 0; cnt < NTYPE_ARITY (result); cnt++) {
            PROD_MEMBER (result, cnt) = va_arg (args, ntype *);
        }
    } break;
    case TC_union: {
        result = MakeNtype (TC_union, va_arg (args, size_t));

        for (cnt = 0; cnt < NTYPE_ARITY (result); cnt++) {
            UNION_MEMBER (result, cnt) = va_arg (args, ntype *);
        }
    } break;
    case TC_fun: {
        result = MakeNtype (TC_fun, va_arg (args, size_t));

        for (cnt = 0; cnt < NTYPE_ARITY (result); cnt++) {
            NTYPE_SON (result, cnt) = va_arg (args, ntype *);
        }
    } break;
    case TC_ibase: {
        result = MakeNtype (TC_ibase, 3);

        IBASE_BASE (result) = va_arg (args, ntype *);
        IBASE_SCAL (result) = va_arg (args, ntype *);
        IBASE_GEN (result) = va_arg (args, ntype *);
        IBASE_IARR (result) = va_arg (args, ntype *);
    } break;
    case TC_iarr: {
        result = MakeNtype (TC_iarr, va_arg (args, size_t));

        IARR_GEN (result) = va_arg (args, ntype *);

        for (cnt = 0; cnt < NTYPE_ARITY (result) - 1; cnt++) {
            IARR_IDIM (result, cnt) = va_arg (args, ntype *);
        }
    } break;
    case TC_idim: {
        result = MakeNtype (TC_idim, va_arg (args, size_t));

        IDIM_DIM (result) = va_arg (args, size_t);

        IDIM_GEN (result) = va_arg (args, ntype *);

        for (cnt = 0; cnt < NTYPE_ARITY (result) - 1; cnt++) {
            IDIM_ISHAPE (result, cnt) = va_arg (args, ntype *);
        }
    } break;
    case TC_ishape: {
        result = MakeNtype (TC_ishape, 1);

        ISHAPE_SHAPE (result) = va_arg (args, shape *);
        ISHAPE_GEN (result) = va_arg (args, ntype *);
    } break;
    case TC_ires: {
        result = MakeNtype (TC_ires, 1);
        IRES_NUMFUNS (result) = va_arg (args, size_t);

        if (IRES_NUMFUNS (result) <= 0) {
            IRES_FUNDEFS (result) = NULL;
            IRES_POSS (result) = NULL;
        } else {
            IRES_FUNDEFS (result)
              = (node **)MEMmalloc (sizeof (node *) * IRES_NUMFUNS (result));
            IRES_POSS (result) = (int *)MEMmalloc (sizeof (int) * IRES_NUMFUNS (result));

            for (cnt = 0; cnt < IRES_NUMFUNS (result); cnt++) {
                IRES_FUNDEF (result, cnt) = va_arg (args, node *);
            }

            for (cnt = 0; cnt < IRES_NUMFUNS (result); cnt++) {
                IRES_POS (result, cnt) = va_arg (args, int);
            }
        }

        IRES_TYPE (result) = va_arg (args, ntype *);
    } break;
    case TC_alpha: {
        DBUG_UNREACHABLE ("Cannot deserialize alpha types");

        result = NULL;
    } break;
    case TC_poly: {
        result = TYmakePolyType (STRcpy (va_arg (args, char *)));
    } break;
    case TC_polyuser: {
        char *outer, *inner, *shape;
        bool denest, renest;

        outer = STRcpy (va_arg (args, char *));
        inner = STRcpy (va_arg (args, char *));
        shape = STRcpy (va_arg (args, char *));
        denest = (bool)va_arg (args, int);
        renest = (bool)va_arg (args, int);

        result = TYmakePolyUserType (outer, inner, shape, denest, renest);
    } break;
    case TC_bottom: {
        result = TYmakeBottomType (STRcpy (va_arg (args, char *)));
    } break;
    case TC_dummy: {
        result = MakeNtype (TC_dummy, 0);
    } break;
    }

#if !IS_CYGWIN
    va_end (args);
#endif

    DBUG_RETURN (result);
}

/* @} */ /* defgroup nty */

#undef DBUG_PREFIX
