/*
 *
 * $Log$
 * Revision 3.10  2002/08/06 08:41:48  sbs
 * just to please gcc...
 *
 * Revision 3.9  2002/08/06 08:26:49  sbs
 * some vars initialized to please gcc for the product version.
 *
 * Revision 3.8  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker !!
 *
 * Revision 3.7  2002/05/31 14:51:54  sbs
 * intermediate version to ensure compilable overall state.
 *
 * Revision 3.6  2002/03/12 15:14:15  sbs
 * AUDGZ types added functions for handling function types added
 * type comparison as well as Type2String implemented.
 *
 * Revision 3.5  2001/05/17 11:34:07  sbs
 * return value of Free now used ...
 *
 * Revision 3.4  2001/05/17 09:20:42  sbs
 * MALLOC FREE aliminated
 *
 * Revision 3.3  2001/03/22 20:38:52  dkr
 * include of tre.h eliminated
 *
 * Revision 3.2  2001/03/15 15:17:47  dkr
 * signature of Type2String modified
 *
 * Revision 3.1  2000/11/20 18:00:09  sacbase
 * new release made
 *
 * Revision 1.4  2000/11/14 13:19:29  dkr
 * no '... might be used uninitialized' warnings anymore
 *
 * Revision 1.3  2000/10/24 11:46:10  dkr
 * MakeType renamed into MakeTypes
 *
 * Revision 1.2  1999/10/20 15:45:10  sbs
 * some code brushing done.
 *
 * Revision 1.1  1999/10/20 12:52:00  sbs
 * Initial revision
 *
 */

/*
 * This module implements a new abstract data type for representing
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
 * The central idea of the ntype is that all types are represented by recursively
 * nested type-constructors with varying arity.
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
 *     TYFreeTypeConstructor  for freeing the topmost constructor only, and
 *     TYFreeType             for freeing the entire type.
 *
 * - If the result is an ntype structure, it has been freshly allocated!
 *
 *
 */

#include <stdarg.h>
#include <limits.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "LookUpTable.h"

#include "dbug.h"

#include "free.h"
#include "convert.h"

#include "user_types.h"
#include "shape.h"
#include "ssi.h"

/*
 * Since all type constructors may have different attributes,
 * for each type constructor TC_xx that requires at least two attributes
 * a new type attr_xx is defined:
 */

typedef struct ATTR_AKD {
    shape *shp;
    int dots;
} attr_akd;

typedef struct ATTR_SYMBOL {
    char *mod;
    char *name;
} attr_symbol;

typedef struct ATTR_IRES {
    int num_funs;
    node **fundefs;
    int *poss;
} attr_ires;

/*
 * In order to have a uniform type for ALL type constructors, we define
 * a union type over all potential attributes:
 */

typedef union {
    simpletype a_simple;
    attr_symbol a_symbol;
    usertype a_user;
    shape *a_aks;
    attr_akd a_akd;
    struct NTYPE *a_ibase;
    int a_idim;
    shape *a_ishape;
    attr_ires a_ires;
    tvar *a_alpha;
} typeattr;

/*
 * Finally, the new type structure "ntype" can be defined:
 *  it consists of - a type constructor tag
 *                 - the arity of the constructor
 *                 - its respective attribute
 *                 - a pointer to a list of (arity-many) son-constructors
 */

struct NTYPE {
    typeconstr typeconstr;
    int arity;
    typeattr typeattr;
    struct NTYPE **sons;
};

/*
 * Now, we include the own interface! The reason for this is twofold:
 * First, it ensures consistency betweeen the interface and the
 * implementation and second, it serves as a forward declaration for all
 * functions.
 */

#include "new_types.h"

/*
 * For internal usage within this module only, we define the following
 * shape access macros:
 *
 * First, we define some basic ntype-access-macros:
 */

#define NTYPE_CON(n) (n->typeconstr)
#define NTYPE_ARITY(n) (n->arity)
#define NTYPE_SONS(n) (n->sons)
#define NTYPE_SON(n, i) (n->sons[i])

/*
 * Macros for accessing the attributes...
 */
#define SIMPLE_TYPE(n) (n->typeattr.a_simple)
#define SYMBOL_MOD(n) (n->typeattr.a_symbol.mod)
#define SYMBOL_NAME(n) (n->typeattr.a_symbol.name)
#define USER_TYPE(n) (n->typeattr.a_user)
#define AKS_SHP(n) (n->typeattr.a_aks)
#define AKD_SHP(n) (n->typeattr.a_akd.shp)
#define AKD_DOTS(n) (n->typeattr.a_akd.dots)

#define IBASE_BASE(n) (n->typeattr.a_ibase)
#define IDIM_DIM(n) (n->typeattr.a_idim)
#define ISHAPE_SHAPE(n) (n->typeattr.a_ishape)
#define IRES_NUMFUNS(n) (n->typeattr.a_ires.num_funs)
#define IRES_FUNDEFS(n) (n->typeattr.a_ires.fundefs)
#define IRES_FUNDEF(n, i) (n->typeattr.a_ires.fundefs[i])
#define IRES_POSS(n) (n->typeattr.a_ires.poss)
#define IRES_POS(n, i) (n->typeattr.a_ires.poss[i])

#define ALPHA_SSI(n) (n->typeattr.a_alpha)

/*
 * Macros for accessing the sons...
 */
#define AKS_BASE(n) (n->sons[0])
#define AKD_BASE(n) (n->sons[0])
#define AUDGZ_BASE(n) (n->sons[0])
#define AUD_BASE(n) (n->sons[0])
#define UNION_MEMBER(n, i) (n->sons[i])
#define PROD_MEMBER(n, i) (n->sons[i])

#define FUN_IBASE(n, i) (n->sons[i])

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
 *    ntype * MakeNtype( typeconstr con, int arity)
 *
 * description:
 *    internal function for allocating an ntype-node. According to arity,
 *    an array of pointers to subtypes is allocated as well (iff arity>0).
 *
 ******************************************************************************/

ntype *
MakeNtype (typeconstr con, int arity)
{
    ntype *res;
    int i;

    DBUG_ENTER ("MakeNtype");

    res = (ntype *)Malloc (sizeof (ntype));
    NTYPE_CON (res) = con;
    NTYPE_ARITY (res) = arity;
    if (NTYPE_ARITY (res) > 0) {
        NTYPE_SONS (res) = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < arity; i++) {
            NTYPE_SON (res, i) = NULL;
        }
    } else {
        NTYPE_SONS (res) = NULL;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * MakeNewSon( ntype *father, ntype *son)
 *
 * description:
 *    internal function for increasing an ntype's arity.
 *    Like all Makexxx functions it consumes (reuses) both its arguments!!
 *
 ******************************************************************************/

ntype *
MakeNewSon (ntype *father, ntype *son)
{
    ntype **new_sons;
    int i, arity;

    DBUG_ENTER ("MakeNewSon");

    arity = NTYPE_ARITY (father);
    NTYPE_ARITY (father) = arity + 1;
    new_sons = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (father));
    for (i = 0; i < arity; i++) {
        new_sons[i] = NTYPE_SON (father, i);
    }
    new_sons[i] = son;
    Free (NTYPE_SONS (father));
    NTYPE_SONS (father) = new_sons;

    DBUG_RETURN (father);
}

/******************************************************************************
 *
 * function:
 *    ntype * MakeNewFundefsPoss( ntype *ires, int num, node **fundefs, int *poss)
 *
 * description:
 *    internal function for adding <num> fundefs and <num> poss to an ires's
 *    fundefs and poss list.
 *    Like all Makexxx functions it consumes (reuses) both its arguments!!
 *
 ******************************************************************************/

ntype *
MakeNewFundefsPoss (ntype *ires, int num, node **fundefs, int *poss)
{
    node **new_fundefs;
    int *new_poss;
    int i, arity;

    DBUG_ENTER ("MakeNewFundefsPoss");

    arity = IRES_NUMFUNS (ires);
    IRES_NUMFUNS (ires) = arity + num;
    new_fundefs = (node **)Malloc (sizeof (node *) * IRES_NUMFUNS (ires));
    new_poss = (int *)Malloc (sizeof (int) * IRES_NUMFUNS (ires));
    for (i = 0; i < arity; i++) {
        new_fundefs[i] = IRES_FUNDEF (ires, i);
        new_poss[i] = IRES_POS (ires, i);
    }
    for (; i < IRES_NUMFUNS (ires); i++) {
        new_fundefs[i] = fundefs[i - arity];
        new_poss[i] = poss[i - arity];
    }
    Free (IRES_FUNDEFS (ires));
    Free (IRES_POSS (ires));
    Free (fundefs);
    Free (poss);
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
 *    typeconstr TYGetConstr( ntype *type)
 *
 * description:
 *
 ******************************************************************************/

typeconstr
TYGetConstr (ntype *type)
{
    DBUG_ENTER ("TYGetConstr");

    DBUG_RETURN (NTYPE_CON (type));
}

/******************************************************************************
 *
 * function:
 *    ntype * TYMakeSimpleType( simpletype base)
 *    ntype * TYMakeSymbType( char *name, char *mod)
 *    ntype * TYMakeUserType( usertype udt)
 *
 * description:
 *  several functions for creating scalar types (type-constructors with arity 0).
 *
 ******************************************************************************/

ntype *
TYMakeSimpleType (simpletype base)
{
    ntype *res;

    DBUG_ENTER ("TYMakeSimpleType");

    res = MakeNtype (TC_simple, 0);
    SIMPLE_TYPE (res) = base;

    DBUG_RETURN (res);
}

ntype *
TYMakeSymbType (char *name, char *mod)
{
    ntype *res;

    DBUG_ENTER ("TYMakeSymbType");

    DBUG_ASSERT ((name != NULL), ("TYMakeSymbType called with NULL name!"));

    res = MakeNtype (TC_symbol, 0);
    SYMBOL_MOD (res) = mod;
    SYMBOL_NAME (res) = name;

    DBUG_RETURN (res);
}

ntype *
TYMakeUserType (usertype udt)
{
    ntype *res;

    DBUG_ENTER ("TYMakeUserType");

    res = MakeNtype (TC_user, 0);
    USER_TYPE (res) = udt;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    simpletype   TYGetSimpleType( ntype *simple)
 *    char       * TYGetName( ntype *symb)
 *    char       * TYGetMod( ntype *symb)
 *
 * description:
 *  several functions for extracting the attributes from scalar types.
 *
 ******************************************************************************/

simpletype
TYGetSimpleType (ntype *simple)
{
    DBUG_ENTER ("TYGetSimpleType");
    DBUG_ASSERT ((NTYPE_CON (simple) == TC_simple),
                 "TYGetSimpleType applied to nonsimple-type!");
    DBUG_RETURN (SIMPLE_TYPE (simple));
}

char *
TYGetName (ntype *symb)
{
    DBUG_ENTER ("TYGetName");
    DBUG_ASSERT ((NTYPE_CON (symb) == TC_symbol), "TYGetName applied to nonsymbol-type!");
    DBUG_RETURN (SYMBOL_NAME (symb));
}

char *
TYGetMod (ntype *symb)
{
    DBUG_ENTER ("TYGetMod");
    DBUG_ASSERT ((NTYPE_CON (symb) == TC_symbol), "TYGetMod applied to nonsymbol-type!");
    DBUG_RETURN (SYMBOL_MOD (symb));
}

/******************************************************************************
 *
 * function:
 *    ntype * TYMakeAKS( ntype *scalar, shape *shp)
 *    ntype * TYMakeAKD( ntype *scalar, int dots, shape *shp)
 *    ntype * TYMakeAUD( ntype *scalar)
 *    ntype * TYMakeAUDGZ( ntype *scalar)
 *
 *    ntype * TYSetScalar( ntype *array, ntype *scalar)
 *
 * description:
 *  several functions for creating array-types.
 *
 ******************************************************************************/

ntype *
TYMakeAKS (ntype *scalar, shape *shp)
{
    ntype *res;

    DBUG_ENTER ("TYMakeAKS");

    res = MakeNtype (TC_aks, 1);
    AKS_SHP (res) = shp;
    AKS_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYMakeAKD (ntype *scalar, int dots, shape *shp)
{
    ntype *res;

    DBUG_ENTER ("TYMakeAKD");

    res = MakeNtype (TC_akd, 1);
    AKD_DOTS (res) = dots;
    AKD_SHP (res) = shp;
    AKD_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYMakeAUDGZ (ntype *scalar)
{
    ntype *res;

    DBUG_ENTER ("TYMakeAUDGZ");

    res = MakeNtype (TC_audgz, 1);
    AUDGZ_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYMakeAUD (ntype *scalar)
{
    ntype *res;

    DBUG_ENTER ("TYMakeAUD");

    res = MakeNtype (TC_aud, 1);
    AUD_BASE (res) = scalar;

    DBUG_RETURN (res);
}

ntype *
TYSetScalar (ntype *array, ntype *scalar)
{
    DBUG_ENTER ("TYSetScalar");

    TYFreeType (NTYPE_SON (array, 0));
    NTYPE_SON (array, 0) = scalar;

    DBUG_RETURN (array);
}

/******************************************************************************
 *
 * function:
 *    int     TYGetDim( ntype *array)
 *    shape * TYGetShape( ntype *array)
 *    ntype * TYGetScalar( ntype *array)
 *
 * description:
 *  several functions for extracting the attributes / sons of array types.
 *
 ******************************************************************************/

int
TYGetDim (ntype *array)
{
    shape *shp;
    int res;

    DBUG_ENTER ("TYGetDim");
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akd),
                 "TYGetDim applied to ther than AKS or AKD type!");
    if (NTYPE_CON (array) == TC_aks) {
        res = SHGetDim (AKS_SHP (array));
    } else {
        shp = AKD_SHP (array);
        if (shp != NULL) {
            res = SHGetDim (shp) + AKD_DOTS (array);
        } else {
            res = AKD_DOTS (array);
        }
    }

    DBUG_RETURN (res);
}

shape *
TYGetShape (ntype *array)
{
    shape *res;

    DBUG_ENTER ("TYGetShape");
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akd),
                 "TYGetShape applied to ther than AKS or AKD type!");
    if (NTYPE_CON (array) == TC_aks) {
        res = AKS_SHP (array);
    } else {
        res = AKD_SHP (array);
    }

    DBUG_RETURN (res);
}

ntype *
TYGetScalar (ntype *array)
{
    DBUG_ENTER ("TYGetScalar");
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akd)
                   || (NTYPE_CON (array) == TC_audgz) || (NTYPE_CON (array) == TC_aud),
                 "TYGetScalar applied to other than array type!");
    DBUG_RETURN (NTYPE_SON (array, 0));
}

/******************************************************************************
 *
 * function:
 *    ntype * TYMakeUnionType( ntype *t1, ntype *t2)
 *
 * description:
 *  functions for creating union-types. Note here, that this function, like
 *  all MakeXYZ and GetXYZ functions consumes its arguments!!
 *  Since the representation of the union type constructor does not allow the
 *  two argument constructors to be re-used iff the are already union-constructors,
 *  those constructors are freed!!
 *
 ******************************************************************************/

ntype *
TYMakeUnionType (ntype *t1, ntype *t2)
{
    ntype *res;
    int arity = 2, pos = 0;
    int i;

    DBUG_ENTER ("TYMakeUnionType");

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
        TYFreeTypeConstructor (t1);
    } else {
        UNION_MEMBER (res, pos++) = t1;
    }
    if (NTYPE_CON (t2) == TC_union) {
        for (i = 0; i < NTYPE_ARITY (t2); i++) {
            UNION_MEMBER (res, pos++) = UNION_MEMBER (t2, i);
        }
        TYFreeTypeConstructor (t2);
    } else {
        UNION_MEMBER (res, pos++) = t2;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYMakeProductType( int size, ...)
 *
 *    ntype * TYMakeEmptyProductType( int size)
 *    ntype * TYSetProductMember( ntype *prod, int pos, ntype *member)
 *
 * description:
 *  functions for creating product types. Note here, that this function, like
 *  all MakeXYZ and GetXYZ functions consumes its arguments!!
 *  At the time being, only array types or type variables may be given as arguments.
 *  The first version is useful in all situations where the number of components
 *  is statically known. However, in many situations this is not the case.
 *  In these situations, the latter two functions are to be used.
 *
 ******************************************************************************/

ntype *
TYMakeProductType (int size, ...)
{
    va_list Argp;
    int i;
    ntype *res, *arg;

    DBUG_ENTER ("TYMakeProductType");

    res = MakeNtype (TC_prod, size);

    if (size > 0) {
        va_start (Argp, size);
        for (i = 0; i < size; i++) {
            arg = va_arg (Argp, ntype *);
            DBUG_ASSERT ((TYIsArray (arg) || TYIsAlpha (arg)),
                         "non array type / type var components of product types are not "
                         "yet supported!");
            PROD_MEMBER (res, i) = arg;
        }
    }

    DBUG_RETURN (res);
}

ntype *
TYMakeEmptyProductType (int size)
{
    ntype *res;

    DBUG_ENTER ("TYMakeEmptyProductType");

    res = MakeNtype (TC_prod, size);

    DBUG_RETURN (res);
}

ntype *
TYSetProductMember (ntype *prod, int pos, ntype *member)
{
    DBUG_ENTER ("TYSetProductMember");

    DBUG_ASSERT ((NTYPE_CON (prod) == TC_prod),
                 "TYSetProductMember applied to non-product type");
    DBUG_ASSERT ((pos < NTYPE_ARITY (prod)),
                 "TYSetProductMember applied to product type with too few elements");

    NTYPE_SON (prod, pos) = member;
    DBUG_RETURN (prod);
}

/******************************************************************************
 *
 * function:
 *    int       TYGetProductSize( ntype *prod)
 *    ntype  *  TYGetProductMember( ntype *prod, int pos)
 *
 * description:
 *  functions inspecting or extracting components of product types!
 *  Note here, that TYGetProductMember does not copy the member to be extracted!!
 *
 ******************************************************************************/

int
TYGetProductSize (ntype *prod)
{
    DBUG_ENTER ("TYGetProductSize");
    DBUG_ASSERT ((NTYPE_CON (prod) == TC_prod),
                 "TYGetProductSize applied to non-product type");
    DBUG_RETURN (NTYPE_ARITY (prod));
}

ntype *
TYGetProductMember (ntype *prod, int pos)
{
    DBUG_ENTER ("TYGetProductMember");
    DBUG_ASSERT ((NTYPE_CON (prod) == TC_prod),
                 "TYGetProductMember applied to non-product type");
    DBUG_ASSERT ((NTYPE_ARITY (prod) > pos),
                 "TYGetProductMember applied with illegal index");
    DBUG_RETURN (PROD_MEMBER (prod, pos));
}

/******************************************************************************
 *
 * function:
 *   ntype *  TYMakeFunType( ntype *arg, ntype *res, node *fun_info)
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
 *                           TC_fun
 *                          /  ...
 *                         /
 *                  TC_ibase -- scalar type (e.g. INT)
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
 *                                            / [3,4]
 *
 *  All "open ends" of this structure point to TC_ires nodes which hold the return
 *  types of the given function. The dots right of some edges indicate that there may
 *  be multiple of the lower nodes attached, i.e., ther may be several TC_ibase
 *  nodes under a single TC_fun node, and there may also be several TC_idim and
 *  several TC_ishape nodes.
 *  However, since this function only creates the type for a single non-overloaded
 *  function, it does only create single ones. In case the parameter type of the
 *  function is a generic one, e.g. int[.,.], only that part of the tree is
 *  constructed that is necessary to accomodate it. In case of int[.,.], simply
 *  the lowest TC_ishape node would be missing.
 *
 ******************************************************************************/

ntype *
TYMakeFunType (ntype *arg, ntype *res_type, node *fundef)
{
    ntype *fun = NULL;
    ntype *base = NULL;
    ntype *arr = NULL;
    ntype *dim = NULL;
    ntype *shape = NULL;
    ntype *res = NULL;

#ifndef DBUG_OFF
    char *tmp;
#endif

    DBUG_ENTER ("TYMakeFunType");

    res = MakeNtype (TC_ires, 1);

    IRES_TYPE (res) = res_type;

    IRES_NUMFUNS (res) = 1;
    IRES_FUNDEFS (res) = (node **)Malloc (sizeof (node *));
    IRES_FUNDEF (res, 0) = fundef;
    IRES_POSS (res) = (int *)Malloc (sizeof (int));
    IRES_POS (res, 0) = 0;

    base = MakeNtype (TC_ibase, 3);

    switch (NTYPE_CON (arg)) {
    case TC_aks:
        if (TYGetDim (arg) == 0) {
            IBASE_SCAL (base) = TYCopyType (res); /* scalar: definition case */
            /* finally, we make res an up-projection as res will be used in AUD! */
            IRES_POS (res, 0) = 1;
        } else {
            shape = MakeNtype (TC_ishape, 1);
            ISHAPE_SHAPE (shape) = SHCopyShape (AKS_SHP (arg));
            ISHAPE_GEN (shape) = TYCopyType (res); /* array AKS: definition case */

            dim = MakeNtype (TC_idim, 2);
            IDIM_DIM (dim) = TYGetDim (arg);
            IDIM_ISHAPE (dim, 0) = shape;
            IDIM_GEN (dim) = TYCopyType (res);
            IRES_POS (IDIM_GEN (dim), 0) = 1; /* projecting AKS to AKD */

            arr = MakeNtype (TC_iarr, 2);
            IARR_IDIM (arr, 0) = dim;
            IARR_GEN (arr) = TYCopyType (res);
            IRES_POS (IARR_GEN (arr), 0) = 2; /* projecting AKS to AUDGZ */
            /* finally, we make res an up-projection as res will be used in AUD! */
            IRES_POS (res, 0) = 3;
        }
        break;

    case TC_akd:
        if (TYGetDim (arg) == 0) {
            IBASE_SCAL (base) = TYCopyType (res); /* scalar: definition case */
        } else {
            dim = MakeNtype (TC_idim, 1);
            IDIM_DIM (dim) = TYGetDim (arg);
            IDIM_GEN (dim) = TYCopyType (res); /* array AKD: definition case */

            arr = MakeNtype (TC_iarr, 2);
            IARR_IDIM (arr, 0) = dim;
            IARR_GEN (arr) = TYCopyType (res);
            IRES_POS (IARR_GEN (arr), 0) = 1; /* projecting AKD to AUDGZ */
        }
        /* finally, we make res an up-projection as res will be used in AUD! */
        IRES_POS (res, 0) = 2;
        break;

    case TC_audgz:
        arr = MakeNtype (TC_iarr, 1);
        IARR_GEN (arr) = TYCopyType (res); /* AUDGZ definition case */

        /* finally, we make res an up-projection as res will be used in AUD! */
        IRES_POS (res, 0) = 1;
        break;

    case TC_aud:
        break;

    default:
        DBUG_ASSERT (0, "argument type not yet supported");
    }

    IBASE_GEN (base) = res;
    IBASE_BASE (base) = TYGetScalar (arg);
    IBASE_IARR (base) = arr;

    fun = MakeNtype (TC_fun, 1);
    FUN_IBASE (fun, 0) = base;

    /*
     * the only son of the arg type has been reused, now we free its constructor!
     */
    TYFreeTypeConstructor (arg);

    DBUG_EXECUTE ("NTY", (tmp = TYType2DebugString (fun, TRUE, 0)););
    DBUG_PRINT ("NTY", ("fun type built: %s\n", tmp));

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *   ntype *  TYMakeOverloadedFunType( ntype *fun1, ntype *fun2)
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
    int i, j;
    int new_numfuns = 0;
    node **new_fundefs;
    int *new_poss;

    DBUG_ENTER ("FilterFundefs");

    if (fun != NULL) {
        switch (NTYPE_CON (fun)) {
        case TC_fun:
            for (i = 0; i < NTYPE_ARITY (fun); i++) {
                NTYPE_SON (fun, i)
                  = FilterFundefs (NTYPE_SON (fun, i), num_kills, kill_list);
            }
            break;
        case TC_ibase:
        case TC_iarr:
        case TC_idim:
        case TC_ishape:
            NTYPE_SON (fun, 0) = FilterFundefs (NTYPE_SON (fun, 0), num_kills, kill_list);
            if (NTYPE_SON (fun, 0) == NULL) {
                TYFreeType (fun);
                fun = NULL;
            } else {
                for (i = 1; i < NTYPE_ARITY (fun); i++) {
                    NTYPE_SON (fun, i)
                      = FilterFundefs (NTYPE_SON (fun, i), num_kills, kill_list);
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
                TYFreeType (fun);
                fun = NULL;
            } else {
                new_fundefs = (node **)Malloc (sizeof (node *) * new_numfuns);
                new_poss = (int *)Malloc (sizeof (int) * new_numfuns);
                j = 0;
                for (i = 0; i < IRES_NUMFUNS (fun); i++) {
                    if (IRES_FUNDEF (fun, i) != NULL) {
                        new_fundefs[j] = IRES_FUNDEF (fun, i);
                        new_poss[j] = IRES_POS (fun, i);
                        j++;
                    }
                }
                IRES_FUNDEFS (fun) = Free (IRES_FUNDEFS (fun));
                IRES_POSS (fun) = Free (IRES_POSS (fun));
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
            DBUG_ASSERT (0, "FilterFundefs called with illegal funtype!");
        }
    }

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *    ntype *ProjDown( ntype *ires, ntype *template)
 *
 * description:
 *    copies ires and inspects the positioning of the associated fundefs.
 *    Those that are upward projections are NOT projected down!
 *    (If all fundefs turn out to be upward projections, NULL is returned!)
 *    If the template ntype-node is not an ires node, a copy of it is put
 *    in front of the freshly generated ires node and returned.
 *
 ******************************************************************************/

static ntype *
ProjDown (ntype *ires, ntype *template)
{
    int i;
    int new_numfuns = 0;
    int num_kills = 0;
    ntype *res = NULL;
    ntype *tmp = NULL;
    node **kill_list;

    kill_list = (node **)Malloc (sizeof (node *) * IRES_NUMFUNS (ires));

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
        res = TYCopyFixedType (ires);
        res = FilterFundefs (res, num_kills, kill_list);
        for (i = 0; i < IRES_NUMFUNS (res); i++) {
            IRES_POS (res, i) = IRES_POS (res, i) - 1;
        }
        if (NTYPE_CON (template) != TC_ires) {
            tmp = res;
            res = TYCopyTypeConstructor (template);
            NTYPE_ARITY (res) = 1;
            NTYPE_SONS (res) = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (res));
            NTYPE_SON (res, 0) = tmp;
        }
    }

    kill_list = Free (kill_list);

    return (res);
}

typedef bool (*cmp_ntype_fun_t) (ntype *, ntype *);

static bool
CmpIbase (ntype *ibase1, ntype *ibase2)
{
    DBUG_ASSERT (((NTYPE_CON (ibase1) == TC_ibase) && (NTYPE_CON (ibase1) == TC_ibase)),
                 ("CmpIbase called with non TC_ibase arg!"));
    return (TYEqTypes (IBASE_BASE (ibase1), IBASE_BASE (ibase2)));
}

static bool
CmpIdim (ntype *idim1, ntype *idim2)
{
    DBUG_ASSERT (((NTYPE_CON (idim1) == TC_idim) && (NTYPE_CON (idim1) == TC_idim)),
                 ("CmpIdim called with non TC_idim arg!"));
    return (IDIM_DIM (idim1) == IDIM_DIM (idim2));
}

static bool
CmpIshape (ntype *ishape1, ntype *ishape2)
{
    DBUG_ASSERT (((NTYPE_CON (ishape1) == TC_ishape)
                  && (NTYPE_CON (ishape1) == TC_ishape)),
                 ("CmpIshape called with non TC_ishape arg!"));
    return (SHCompareShapes (ISHAPE_SHAPE (ishape1), ISHAPE_SHAPE (ishape2)));
}

static ntype *
FindAndMergeSons (ntype *fun1, ntype *fun2, int start, cmp_ntype_fun_t CmpFun)
{
    int i, j;
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
FindOrAdjustSons (ntype **fun1_p, ntype **fun2_p, int start, cmp_ntype_fun_t CmpFun)
{
    int i, j;
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
MergeSons (ntype *fun1, ntype *fun2, int start, int stop)
{
    int i;

    for (i = start; i < (stop); i++) {
        NTYPE_SON (fun2, i)
          = MakeOverloadedFunType (NTYPE_SON (fun1, i), NTYPE_SON (fun2, i));
    }
    return (fun2);
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
}

/******************************************************************************
 *
 * NOW, the implementation of   TYMakeOverloadedFunType    itself!!
 *
 ******************************************************************************/

#ifndef DBUG_OFF
static tvar *overload_fun1_alpha;
#endif
static LUT_t overload_lut;

ntype *
TYMakeOverloadedFunType (ntype *fun1, ntype *fun2)
{
    ntype *res;
#ifndef DBUG_OFF
    char *tmp, *tmp2;
#endif

    DBUG_ENTER ("TYMakeOverloadedFunType");

    DBUG_EXECUTE ("NTY", (tmp = TYType2DebugString (fun1, TRUE, 0),
                          tmp2 = TYType2DebugString (fun2, TRUE, 0)););
    DBUG_PRINT ("NTY", ("functions:        %s", tmp));
    DBUG_PRINT ("NTY", ("and               %s", tmp2));
    DBUG_EXECUTE ("NTY", (tmp = Free (tmp), tmp2 = Free (tmp2)););

    /*
     * instantiate rel. free vars 8-))
     */
#ifndef DBUG_OFF
    overload_fun1_alpha = NULL;
#endif
    overload_lut = GenerateLUT ();

    if ((fun1 != NULL) && (NTYPE_CON (fun1) != TC_fun) && (fun2 != NULL)
        && (NTYPE_CON (fun2) != TC_fun)) {
        ABORT (linenum, ("cannot overload functions of arity 0"));
    }

    res = MakeOverloadedFunType (fun1, fun2);

    overload_lut = RemoveLUT (overload_lut);

    DBUG_EXECUTE ("NTY", (tmp = TYType2DebugString (res, TRUE, 0)););
    DBUG_PRINT ("NTY", ("overloaded into : %s", tmp));
    DBUG_EXECUTE ("NTY", tmp = Free (tmp););

    DBUG_RETURN (res);
}

static ntype *
MakeOverloadedFunType (ntype *fun1, ntype *fun2)
{
    ntype *res;
    tvar *old_alpha;
    bool ok;

    DBUG_ENTER ("MakeOverloadedFunType");
    if (fun1 == NULL) {
        res = fun2;
    } else if (fun2 == NULL) {
        res = fun1;
    } else {
        DBUG_ASSERT ((NTYPE_CON (fun1) == NTYPE_CON (fun2)),
                     "TYOverloadFunType called with incompatible types!");

        res = fun2;
        switch (NTYPE_CON (fun1)) {
        case TC_fun:
            fun2 = FindAndMergeSons (fun1, fun2, 0, CmpIbase);
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
            DBUG_ASSERT (((TYIsProd (IRES_TYPE (fun1)) && TYIsProd (IRES_TYPE (fun2)))
                          || (TYIsFun (IRES_TYPE (fun1)) && TYIsFun (IRES_TYPE (fun2)))),
                         "trying to overload incompatible function types");
            res = MakeNewFundefsPoss (fun2, IRES_NUMFUNS (fun1), IRES_FUNDEFS (fun1),
                                      IRES_POSS (fun1));

            DBUG_PRINT ("NTOVLD", ("new ires:"));
            DBUG_EXECUTE ("NTOVLD", DebugPrintDispatchInfo ("NTOVLD", res););

            MakeOverloadedFunType (IRES_TYPE (fun1), IRES_TYPE (fun2));
            break;
        case TC_prod:
            DBUG_ASSERT ((NTYPE_ARITY (fun1) == NTYPE_ARITY (fun2)),
                         "trying to overload function types with different number of "
                         "return types");
            fun2 = MergeSons (fun1, fun2, 0, NTYPE_ARITY (fun1));
            break;
        case TC_alpha:
#ifndef DBUG_OFF
            /*
             * check whether fun1 is not yet overloaded!
             */
            if (overload_fun1_alpha == NULL) {
                overload_fun1_alpha = ALPHA_SSI (fun1);
            } else {
                DBUG_ASSERT ((overload_fun1_alpha == ALPHA_SSI (fun1)),
                             ("TYMakeOverloadedFunType called with overloaded fun1!"));
            }
#endif
            if (SSIIsLe (ALPHA_SSI (fun1), ALPHA_SSI (fun2))) {
                res = fun2;
            } else if (SSIIsLe (ALPHA_SSI (fun2), ALPHA_SSI (fun1))) {
                res = TYCopyType (fun1);
                TYFreeTypeConstructor (fun2);
            } else {
                old_alpha = SearchInLUT_P (overload_lut, ALPHA_SSI (fun2));
                if (old_alpha != ALPHA_SSI (fun2)) { /* found! */
                    res = MakeNtype (TC_alpha, 0);
                    ALPHA_SSI (res) = old_alpha;
                } else {
                    res = TYMakeAlphaType (TYLubOfTypes (SSIGetMax (ALPHA_SSI (fun1)),
                                                         SSIGetMax (ALPHA_SSI (fun2))));
                    ok = SSINewRel (ALPHA_SSI (fun1), ALPHA_SSI (res));
                    DBUG_ASSERT (ok, "SSINewRel did not work in TYMakeOverloadFunType");
                    ok = SSINewRel (ALPHA_SSI (fun2), ALPHA_SSI (res));
                    DBUG_ASSERT (ok, "SSINewRel did not work in TYMakeOverloadFunType");
                    overload_lut
                      = InsertIntoLUT_P (overload_lut, ALPHA_SSI (fun2), ALPHA_SSI (res));
                }
            }
            break;
        default:
            DBUG_ASSERT ((0), "TYMakeOverloadFunType called with illegal funtype!");
        }
        TYFreeTypeConstructor (fun1);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   ntype *  TYDispatchFunType( ntype *fun, ntype *args )
 *
 * description:
 *
 ******************************************************************************/

static ntype *
FindIbase (ntype *fun, ntype *scalar)
{
    ntype *res = NULL;
    int i = 0;

    DBUG_ENTER ("FindIbase");
    while ((i < NTYPE_ARITY (fun))
           && !TYEqTypes (IBASE_BASE (FUN_IBASE (fun, i)), scalar)) {
        i++;
    }
    if (i < NTYPE_ARITY (fun)) {
        res = FUN_IBASE (fun, i);
    }
    DBUG_RETURN (res);
}

static ntype *
FindIdim (ntype *iarr, int dim)
{
    ntype *res = NULL;
    int i = 0;

    DBUG_ENTER ("FindIdim");
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
    int i = 0;

    DBUG_ENTER ("FindIshape");
    while ((i < (NTYPE_ARITY (idim) - 1))
           && !SHCompareShapes (ISHAPE_SHAPE (IDIM_ISHAPE (idim, i)), shp)) {
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
    fun = FindIbase (fun, TYGetScalar (arg));

    if (fun != NULL) {
        /*   new default:  <base>[*]  */
        res = IBASE_GEN (fun);

        if (((NTYPE_CON (arg) == TC_aks) || (NTYPE_CON (arg) == TC_akd))
            && (TYGetDim (arg) == 0)) {
            /* argument is a scalar ! */
            if (IBASE_SCAL (fun) == NULL) {
                lower = 1;
            } else {
                res = IBASE_SCAL (fun);
            }
        } else {
            if (NTYPE_CON (arg) != TC_aud) {
                fun = IBASE_IARR (fun);
                if (fun == NULL) {
                    lower
                      = (NTYPE_CON (arg) == TC_aks ? 3
                                                   : (NTYPE_CON (arg) == TC_akd ? 2 : 1));
                } else {

                    /*   new default:   <base>[+]   */
                    res = IARR_GEN (fun);

                    if (NTYPE_CON (arg) != TC_audgz) {
                        fun = FindIdim (fun, TYGetDim (arg));
                        if (fun == NULL) {
                            lower = (NTYPE_CON (arg) == TC_aks ? 2 : 1);
                        } else {

                            /*   new default:   <base>[...]   */
                            res = IDIM_GEN (fun);

                            if (NTYPE_CON (arg) != TC_akd) {
                                fun = FindIshape (fun, TYGetShape (arg));
                                if (fun == NULL) {
                                    lower = 1;
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
    int i;

    DBUG_ENTER ("DebugPrintDispatchInfo");

    for (i = 0; i < IRES_NUMFUNS (ires); i++) {
        DBUG_PRINT (dbug_str,
                    ("  fundef %8p: %d", IRES_FUNDEF (ires, i), IRES_POS (ires, i)));
    }

    DBUG_VOID_RETURN;
}

static void
DebugPrintUpsAndDowns (int max_funs, node **fundefs, int *ups, int *downs)
{
    int i;

    DBUG_ENTER ("DebugPrintUpsAndDowns");

    for (i = 0; i < max_funs; i++) {
        DBUG_PRINT ("NTDIS",
                    ("  fundef %8p: ups %2d | downs %2d", fundefs[i], ups[i], downs[i]));
    }

    DBUG_VOID_RETURN;
}
#endif

/******************************************************************************
 *
 * function:
 *   DFT_res * TYMakeDFT_res( ntype *type, int max_funs)
 *
 * description:
 *
 ******************************************************************************/

DFT_res *
TYMakeDFT_res (ntype *type, int max_funs)
{
    DFT_res *res;

    DBUG_ENTER ("TYMakeDFT_res");

    res = (DFT_res *)Malloc (sizeof (DFT_res));

    res->type = type;
    res->def = NULL;
    res->deriveable = NULL;
    res->num_partials = 0;
    res->partials = (node **)Malloc (sizeof (node *) * max_funs);
    res->num_deriveable_partials = 0;
    res->deriveable_partials = (node **)Malloc (sizeof (node *) * max_funs);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   void TYFreeDFT_res( DFT_res *res)
 *
 * description:
 *
 ******************************************************************************/

void
TYFreeDFT_res (DFT_res *res)
{
    DBUG_ENTER ("TYFreeDFT_res");

    if (res->partials != NULL) {
        res->partials = Free (res->partials);
    }
    if (res->deriveable_partials != NULL) {
        res->deriveable_partials = Free (res->partials);
    }

    res = Free (res);

    DBUG_VOID_RETURN;
}

DFT_res *
TYDispatchFunType (ntype *fun, ntype *args)
{
    int lower;
    int i, j, n;
    int k = 0;
    int max_funs = 0;
    ntype *arg, *ires;
    node *fundef;
    DFT_res *res;
    int *ups = NULL;
    int *downs = NULL;
    node **fundefs = NULL;
    int max_deriveable;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("TYDispatchFunType");

    DBUG_ASSERT ((NTYPE_CON (args) = TC_prod),
                 ("second arg of TYDispatchFunType non-product type!"));

    n = NTYPE_ARITY (args);

    if (n == 0) {
        res = NULL;

    } else {

        for (i = 0; i < n; i++) {
            arg = PROD_MEMBER (args, i);
            ires = DispatchOneArg (&lower, fun, arg);
            if (ires == NULL) {
                fundef = IRES_FUNDEF (IBASE_GEN (FUN_IBASE (fun, 0)), 0);
                ABORT (linenum,
                       ("no definition found for a function \"%s\" that accepts"
                        " an argument of type \"%s\" as parameter no %d",
                        FUNDEF_NAME (fundef), TYType2String (arg, FALSE, 0), i + 1));
            }

            DBUG_EXECUTE ("NTDIS", tmp_str = TYType2String (arg, FALSE, 0););
            DBUG_PRINT ("NTDIS", ("arg #%d: %s yields:", i, tmp_str));
            DBUG_EXECUTE ("NTDIS", tmp_str = Free (tmp_str););
            DBUG_EXECUTE ("NTDIS", DebugPrintDispatchInfo ("NTDIS", ires););

            /*
             * Now, we accumulate the ups and downs:
             */
            if (i == 0) {
                max_funs = IRES_NUMFUNS (ires);
                ups = (int *)Malloc (sizeof (int) * max_funs);
                downs = (int *)Malloc (sizeof (int) * max_funs);
                fundefs = (node **)Malloc (sizeof (node *) * max_funs);

                for (j = 0; j < max_funs; j++) {
                    if ((IRES_POS (ires, j) <= 0) || (lower == 0)) {
                        fundefs[j] = IRES_FUNDEF (ires, j);
                        if (IRES_POS (ires, j) > 0) {
                            ups[j] = IRES_POS (ires, j);
                            downs[j] = 0;
                        } else {
                            ups[j] = 0;
                            downs[j] = IRES_POS (ires, j) - lower;
                        }
                    } else {
                        fundefs[j] = NULL;
                    }
                }
                k = max_funs; /* in case we have only one arg (cf. comment below!) */
            } else {
                for (j = 0, k = 0; j < max_funs; j++) {
                    if ((k < IRES_NUMFUNS (ires))
                        && (IRES_FUNDEF (ires, k) == fundefs[j])) {
                        if (IRES_POS (ires, k) > 0) {
                            if (lower > 0) {
                                fundefs[j] = NULL;
                            } else {
                                ups[j] += IRES_POS (ires, k);
                            }
                        } else {
                            downs[j] += IRES_POS (ires, k) - lower;
                        }
                        k++;
                    } else {
                        fundefs[j] = NULL;
                    }
                }
            }

            DBUG_PRINT ("NTDIS", ("accumulated ups and downs:"));
            DBUG_EXECUTE ("NTDIS",
                          DebugPrintUpsAndDowns (max_funs, fundefs, ups, downs););

            fun = IRES_TYPE (ires);
        }

        /*
         * Finally, we export our findings via a DFT_res structure.
         *   in order to avoid multiple allocations we allocate
         *   space for a maximum of "k" fundefs. This is ok as
         *   k denotes the number of fundefs found at the last ires
         *   node!
         * However, in case of 0 args (n==0), no dispatch has to be made
         * (since no overloading is allowed) so we return NULL!!
         */

        res = TYMakeDFT_res (fun, k);

        /* Due to a bug in limits.h (!!!), we have to use INT_MAX here!!! */
        max_deriveable = 1 - INT_MAX;
        for (i = 0; i < max_funs; i++) {
            if (fundefs[i] != NULL) {
                if (ups[i] == 0) {
                    if (downs[i] == 0) {
                        res->def = fundefs[i];
                        /* no down projections in case of an exact definition! */
                        max_deriveable = 0;
                        res->deriveable = NULL;
                    } else {
                        if (downs[i] > max_deriveable) {
                            res->deriveable = fundefs[i];
                            max_deriveable = downs[i];
                        }
                    }
                } else {
                    if (downs[i] == 0) {
                        res->partials[res->num_partials] = fundefs[i];
                        res->num_partials++;
                    } else {
                        res->deriveable_partials[res->num_deriveable_partials]
                          = fundefs[i];
                        res->num_deriveable_partials++;
                    }
                }
            }
        }
        ups = Free (ups);
        downs = Free (downs);
        fundefs = Free (fundefs);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   char * TYDFT_res2DebugString( DFT_res *dft)
 *
 * description:
 *
 ******************************************************************************/

char *
TYDFT_res2DebugString (DFT_res *dft)
{
    static char buf[256];
    char *tmp = &buf[0];
    int i;
    char *tmp_str;

    DBUG_ENTER ("TYDFT_res2DebugString");

    if (dft == NULL) {
        tmp += sprintf (tmp, "--");
    } else {
        if (dft->def) {
            tmp_str = OldTypeSignature2String (dft->def);
            tmp += sprintf (tmp, "exact : (%s) ", tmp_str);
            tmp_str = Free (tmp_str);
        }
        if (dft->deriveable) {
            tmp_str = OldTypeSignature2String (dft->deriveable);
            tmp += sprintf (tmp, "deriveable : (%s) ", tmp_str);
            tmp_str = Free (tmp_str);
        }
        if (dft->num_partials > 0) {
            tmp += sprintf (tmp, "partials : ");
            for (i = 0; i < dft->num_partials; i++) {
                tmp_str = OldTypeSignature2String (dft->partials[i]);
                tmp += sprintf (tmp, "%s ", tmp_str);
                tmp_str = Free (tmp_str);
            }
        }
        if (dft->num_deriveable_partials > 0) {
            tmp += sprintf (tmp, "deriveable_partials : ");
            for (i = 0; i < dft->num_deriveable_partials; i++) {
                tmp_str = OldTypeSignature2String (dft->deriveable_partials[i]);
                tmp += sprintf (tmp, "%s ", tmp_str);
                tmp_str = Free (tmp_str);
            }
        }

        if (tmp == &buf[0]) {
            tmp += sprintf (tmp, "no match!");
        }
    }

    DBUG_RETURN (StringCopy (buf));
}

/******************************************************************************
 *
 * function:
 *   ntype *  TYMakeAlphaType( ntype *maxtype )
 *
 * description:
 *  function for creating a not yet determined subtype of maxtype.
 *
 ******************************************************************************/

ntype *
TYMakeAlphaType (ntype *maxtype)
{
    ntype *res;
    tvar *alpha;

    DBUG_ENTER ("TYMakeAlphaType");
    res = MakeNtype (TC_alpha, 0);
    alpha = SSIMakeVariable ();
    if (maxtype != NULL) {
        SSINewMax (alpha, maxtype);
    }
    ALPHA_SSI (res) = alpha;
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   tvar *  TYGetAlpha( ntype *type )
 *
 * description:
 *  function for extracting the ssa variable from a type variable.
 *
 ******************************************************************************/

tvar *
TYGetAlpha (ntype *type)
{
    DBUG_ENTER ("TYGetAlpha");
    DBUG_ASSERT ((NTYPE_CON (type) == TC_alpha),
                 ("TYGetAlpha applied to non type variable!"));

    DBUG_RETURN (ALPHA_SSI (type));
}

/***
 *** Functions inspecting types / matching on specific types:
 ***/

/******************************************************************************
 *
 * function:
 *    bool TYIsSimple( ntype * type)
 *    bool TYIsUser( ntype * type)
 *    bool TYIsSymb( ntype * type)
 *    bool TYIsScalar( ntype * type)
 *    bool TYIsAlpha( ntype * type)
 *    bool TYIsFixedAlpha( ntype * type)
 *    bool TYIsNonFixedAlpha( ntype * type)
 *    bool TYIsAKS( ntype * type)
 *    bool TYIsAKD( ntype * type)
 *    bool TYIsAUDGZ( ntype * type)
 *    bool TYIsAUD( ntype * type)
 *    bool TYIsArray( ntype * type)
 *    bool TYIsArrayOrAlpha( ntype * type)
 *    bool TYIsArrayOrFixedAlpha( ntype * type)
 *    bool TYIsUnion( ntype * type)
 *    bool TYIsProd( ntype * type)
 *    bool TYIsFun( ntype * type)
 *
 * description:
 *  several predicate functions for inspecting the top level ntype!
 *
 ******************************************************************************/

bool
TYIsSimple (ntype *type)
{
    DBUG_ENTER ("TYIsSimple");
    DBUG_RETURN (NTYPE_CON (type) == TC_simple);
}

bool
TYIsUser (ntype *type)
{
    DBUG_ENTER ("TYIsUser");
    DBUG_RETURN (NTYPE_CON (type) == TC_user);
}

bool
TYIsSymb (ntype *type)
{
    DBUG_ENTER ("TYIsSmb");
    DBUG_RETURN (NTYPE_CON (type) == TC_symbol);
}

bool
TYIsScalar (ntype *type)
{
    DBUG_ENTER ("TYIsScalar");
    DBUG_RETURN (NTYPE_ARITY (type) == 0);
}

bool
TYIsAlpha (ntype *type)
{
    DBUG_ENTER ("TYIsAlpha");
    DBUG_RETURN (NTYPE_CON (type) == TC_alpha);
}

bool
TYIsFixedAlpha (ntype *type)
{
    DBUG_ENTER ("TYIsAlpha");
    DBUG_RETURN ((NTYPE_CON (type) == TC_alpha) && SSIIsFix (ALPHA_SSI (type)));
}

bool
TYIsNonFixedAlpha (ntype *type)
{
    DBUG_ENTER ("TYIsAlpha");
    DBUG_RETURN ((NTYPE_CON (type) == TC_alpha) && !SSIIsFix (ALPHA_SSI (type)));
}

bool
TYIsAKS (ntype *type)
{
    DBUG_ENTER ("TYIsAKS");
    DBUG_RETURN (NTYPE_CON (type) == TC_aks);
}

bool
TYIsAKD (ntype *type)
{
    DBUG_ENTER ("TYIsAKD");
    DBUG_RETURN (NTYPE_CON (type) == TC_akd);
}

bool
TYIsAUD (ntype *type)
{
    DBUG_ENTER ("TYIsAUD");
    DBUG_RETURN (NTYPE_CON (type) == TC_aud);
}

bool
TYIsAUDGZ (ntype *type)
{
    DBUG_ENTER ("TYIsAUDGZ");
    DBUG_RETURN (NTYPE_CON (type) == TC_audgz);
}

bool
TYIsArray (ntype *type)
{
    DBUG_ENTER ("TYIsArray");
    DBUG_RETURN ((NTYPE_CON (type) == TC_aud) || (NTYPE_CON (type) == TC_audgz)
                 || (NTYPE_CON (type) == TC_akd) || (NTYPE_CON (type) == TC_aks));
}

bool
TYIsArrayOrAlpha (ntype *type)
{
    DBUG_ENTER ("TYIsArray");
    DBUG_RETURN (TYIsArray (type) || (NTYPE_CON (type) == TC_alpha));
}

bool
TYIsArrayOrFixedAlpha (ntype *type)
{
    DBUG_ENTER ("TYIsArray");
    DBUG_RETURN (TYIsArray (type)
                 || ((NTYPE_CON (type) == TC_alpha) && SSIIsFix (ALPHA_SSI (type))));
}

bool
TYIsUnion (ntype *type)
{
    DBUG_ENTER ("TYIsUnion");
    DBUG_RETURN (NTYPE_CON (type) == TC_union);
}

bool
TYIsProd (ntype *type)
{
    DBUG_ENTER ("TYIsProd");
    DBUG_RETURN (NTYPE_CON (type) == TC_prod);
}

bool
TYIsFun (ntype *type)
{
    DBUG_ENTER ("TYIsFun");
    DBUG_RETURN (NTYPE_CON (type) == TC_fun);
}

/******************************************************************************
 *
 * function:
 *    bool TYIsAKSSymb( ntype * type)
 *    bool TYIsProdOfArrayOrFixedAlpha( ntype * type)
 *    bool TYIsProdOfArray( ntype * type)
 *
 * description:
 *   several predicate functions for checking particular nestings of
 *   type constructors
 *
 ******************************************************************************/

bool
TYIsAKSSymb (ntype *type)
{
    DBUG_ENTER ("TYIsAKSSymb");
    DBUG_RETURN ((NTYPE_CON (type) == TC_aks)
                 && (NTYPE_CON (AKS_BASE (type)) == TC_symbol));
}

bool
TYIsProdOfArrayOrFixedAlpha (ntype *args)
{
    bool res = TRUE;
    ntype *arg;
    int i;

    DBUG_ENTER ("TYIsProdOfArrayOrFixedAlpha");
    if (TYIsProd (args)) {
        for (i = 0; i < TYGetProductSize (args); i++) {
            arg = TYGetProductMember (args, i);
            res = res && TYIsArrayOrFixedAlpha (arg);
        }
    } else {
        res = FALSE;
    }
    DBUG_RETURN (res);
}

bool
TYIsProdOfArray (ntype *args)
{
    bool res = TRUE;
    ntype *arg;
    int i;

    DBUG_ENTER ("TYIsProdOfArray");
    if (TYIsProd (args)) {
        for (i = 0; i < TYGetProductSize (args); i++) {
            arg = TYGetProductMember (args, i);
            res = res && TYIsArray (arg);
        }
    } else {
        res = FALSE;
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    int TYCountNonFixedAlpha( ntype *type)
 *
 * description:
 *   counts the number of non-fixed type variables contained in the type given.
 *
 ******************************************************************************/

int
TYCountNonFixedAlpha (ntype *type)
{
    int res = 0;
    int i, n;

    DBUG_ENTER ("TYCountNonFixedAlpha");

    if (TYIsProd (type)) {
        n = TYGetProductSize (type);
        for (i = 0; i < n; i++) {
            res += TYCountNonFixedAlpha (TYGetProductMember (type, i));
        }
    } else {
        res += (TYIsNonFixedAlpha (type) ? 1 : 0);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    int TYCountNoMinAlpha( ntype *type)
 *
 * description:
 *   counts the number of type variables that do not have a lower limit.
 *
 ******************************************************************************/

int
TYCountNoMinAlpha (ntype *type)
{
    int res = 0;
    int i, n;

    DBUG_ENTER ("TYCountNoMinAlpha");

    if (TYIsProd (type)) {
        n = TYGetProductSize (type);
        for (i = 0; i < n; i++) {
            res += TYCountNoMinAlpha (TYGetProductMember (type, i));
        }
    } else {
        res += (TYIsAlpha (type) && (SSIGetMin (TYGetAlpha (type)) == NULL) ? 1 : 0);
    }

    DBUG_RETURN (res);
}

/***
 *** functions that check for the relationship of types:
 ***/

/******************************************************************************
 *
 * function:
 *    CT_res TYCmpTypes( ntype * t1, ntype * t2)
 *    bool     TYLeTypes( ntype * t1, ntype * t2)
 *    bool     TYEqTypes( ntype * t1, ntype * t2)
 *
 * description:
 *
 *
 ******************************************************************************/

CT_res
TYCmpTypes (ntype *t1, ntype *t2)
{
    CT_res res = TY_dis;

    DBUG_ENTER ("TYCmpTypes");

    switch (NTYPE_CON (t1)) {
    case TC_simple:
        if ((NTYPE_CON (t2) == TC_simple) && (SIMPLE_TYPE (t1) == SIMPLE_TYPE (t2))) {
            res = TY_eq;
        }
        break;
    case TC_symbol:
        if ((NTYPE_CON (t2) == TC_symbol)
            && (((SYMBOL_MOD (t1) == NULL) && (SYMBOL_MOD (t2) == NULL))
                || ((SYMBOL_MOD (t1) != NULL) && (SYMBOL_MOD (t2) != NULL)
                    && strcmp (SYMBOL_MOD (t1), SYMBOL_MOD (t2)) == 0))
            && (strcmp (SYMBOL_NAME (t1), SYMBOL_NAME (t2)) == 0)) {
            res = TY_eq;
        }
        break;
    case TC_user:
        if ((NTYPE_CON (t2) == TC_user) && (USER_TYPE (t1) == USER_TYPE (t2))) {
            res = TY_eq;
        }
        break;
    case TC_aks:
        switch (NTYPE_CON (t2)) {
        case TC_aks:
            if (TYCmpTypes (AKS_BASE (t1), AKS_BASE (t2)) == TY_eq) {
                if (SHCompareShapes (AKS_SHP (t1), AKS_SHP (t2))) {
                    res = TY_eq;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_akd:
            if (TYCmpTypes (AKS_BASE (t1), AKD_BASE (t2)) == TY_eq) {
                if (TYGetDim (t1) == TYGetDim (t2)) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_audgz:
            if (TYCmpTypes (AKS_BASE (t1), AUDGZ_BASE (t2)) == TY_eq) {
                if (TYGetDim (t1) > 0) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aud:
            if (TYCmpTypes (AKS_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_lt;
            }
            break;
        default:
            break;
        }
        break;
    case TC_akd:
        switch (NTYPE_CON (t2)) {
        case TC_aks:
            if (TYCmpTypes (AKD_BASE (t1), AKS_BASE (t2)) == TY_eq) {
                if (TYGetDim (t1) == TYGetDim (t2)) {
                    res = TY_gt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_akd:
            if (TYCmpTypes (AKD_BASE (t1), AKD_BASE (t2)) == TY_eq) {
                if (TYGetDim (t1) == TYGetDim (t2)) {
                    res = TY_eq;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_audgz:
            if (TYCmpTypes (AKD_BASE (t1), AUDGZ_BASE (t2)) == TY_eq) {
                if (TYGetDim (t1) > 0) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aud:
            if (TYCmpTypes (AKD_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_lt;
            }
            break;
        default:
            break;
        }
        break;
    case TC_audgz:
        switch (NTYPE_CON (t2)) {
        case TC_aks:
            if (TYCmpTypes (AUDGZ_BASE (t1), AKS_BASE (t2)) == TY_eq) {
                if (TYGetDim (t2) > 0) {
                    res = TY_gt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_akd:
            if (TYCmpTypes (AUDGZ_BASE (t1), AKD_BASE (t2)) == TY_eq) {
                if (TYGetDim (t2) > 0) {
                    res = TY_gt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_audgz:
            if ((TYCmpTypes (AUDGZ_BASE (t1), AUDGZ_BASE (t2)) == TY_eq)) {
                res = TY_eq;
            }
            break;
        case TC_aud:
            if (TYCmpTypes (AUDGZ_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_lt;
            }
            break;
        default:
            break;
        }
        break;
    case TC_aud:
        switch (NTYPE_CON (t2)) {
        case TC_aks:
            if (TYCmpTypes (AUD_BASE (t1), AKS_BASE (t2)) == TY_eq) {
                res = TY_gt;
            }
            break;
        case TC_akd:
            if (TYCmpTypes (AUD_BASE (t1), AKD_BASE (t2)) == TY_eq) {
                res = TY_gt;
            }
            break;
        case TC_audgz:
            if (TYCmpTypes (AUD_BASE (t1), AUDGZ_BASE (t2)) == TY_eq) {
                res = TY_gt;
            }
            break;
        case TC_aud:
            if (TYCmpTypes (AUD_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_eq;
            }
            break;
        default:
            break;
        }
        break;
    default:
        DBUG_ASSERT ((0), "Type comparison for non-array types not yet implemented!");
    }

    DBUG_RETURN (res);
}

bool
TYEqTypes (ntype *t1, ntype *t2)
{
    DBUG_ENTER ("TYCmpTypes");
    DBUG_RETURN (TYCmpTypes (t1, t2) == TY_eq);
}

bool
TYLeTypes (ntype *t1, ntype *t2)
{
    CT_res cmp;

    DBUG_ENTER ("TYLeTypes");
    cmp = TYCmpTypes (t1, t2);
    DBUG_RETURN ((cmp == TY_eq) || (cmp == TY_lt));
}

/******************************************************************************
 *
 * function:
 *    ntype * TYLubOfTypes( ntype * t1, ntype * t2)
 *
 * description:
 *    computes the least upper bound of types t1 and t2. In case it does not
 *    exist, NULL is returned.
 *
 ******************************************************************************/

ntype *
TYLubOfTypes (ntype *t1, ntype *t2)
{
    ntype *res, *new_t1;

    DBUG_ENTER ("TYLubOfTypes");

    switch (TYCmpTypes (t1, t2)) {
    case TY_eq:
        res = TYCopyType (t1);
        break;
    case TY_lt:
        res = TYCopyType (t2);
        break;
    case TY_gt:
        res = TYCopyType (t1);
        break;
    case TY_hcs:
        switch (NTYPE_CON (t1)) {
        case TC_aks:
            if (SHGetDim (AKS_SHP (t1)) == 0) {
                new_t1 = TYMakeAUD (AKS_BASE (t1));
            } else {
                new_t1 = TYMakeAKD (TYCopyType (AKS_BASE (t1)), SHGetDim (AKS_SHP (t1)),
                                    SHCreateShape (0));
            }
            res = TYLubOfTypes (new_t1, t2);
            TYFreeType (new_t1);
            break;
        case TC_akd:
            new_t1 = TYMakeAUDGZ (AKD_BASE (t1));
            res = TYLubOfTypes (new_t1, t2);
            TYFreeTypeConstructor (new_t1);
            break;
        case TC_audgz:
            new_t1 = TYMakeAUD (AUDGZ_BASE (t1));
            res = TYLubOfTypes (new_t1, t2);
            TYFreeTypeConstructor (new_t1);
            break;
        case TC_aud:
            DBUG_ASSERT ((0), "Cannot compute LUB!");
            res = NULL;
            break;
        default:
            DBUG_ASSERT ((0), "Cannot compute LUB!");
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
 *    ntype * TYEliminateAlpha( ntype * t1)
 *
 * description:
 *    if t1 is a type variable with identical upper and lower bound the
 *    boundary type is returned, otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYEliminateAlpha (ntype *t1)
{
    ntype *res;
    int i;

    DBUG_ENTER ("TYEliminateAlpha");

    if (TYIsProd (t1)) {
        res = MakeNtype (TC_prod, NTYPE_ARITY (t1));
        for (i = 0; i < NTYPE_ARITY (t1); i++) {
            PROD_MEMBER (res, i) = TYEliminateAlpha (PROD_MEMBER (t1, i));
        }
    } else {
        if (TYIsFixedAlpha (t1)) {
            res = TYCopyType (SSIGetMin (ALPHA_SSI (t1)));
        } else {
            res = TYCopyType (t1);
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYFixAndEliminateAlpha( ntype * t1)
 *
 * description:
 *    if t1 is a type variable with a lower bound the lower bound
 *    is returned, otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYFixAndEliminateAlpha (ntype *t1)
{
    ntype *res;
    int i;

    DBUG_ENTER ("TYFixAndEliminateAlpha");

    if (TYIsProd (t1)) {
        res = MakeNtype (TC_prod, NTYPE_ARITY (t1));
        for (i = 0; i < NTYPE_ARITY (t1); i++) {
            PROD_MEMBER (res, i) = TYFixAndEliminateAlpha (PROD_MEMBER (t1, i));
        }
    } else {
        if (TYIsAlpha (t1) && (SSIGetMin (TYGetAlpha (t1)) != NULL)) {
            res = TYCopyType (SSIGetMin (ALPHA_SSI (t1)));
        } else {
            res = TYCopyType (t1);
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
 *    void TYFreeTypeConstructor( ntype *type)
 *    void TYFreeType( ntype *type)
 *
 * description:
 *   functions for freeing types. While TYFreeTypeConstructor only frees the
 *   topmost type constructor, TYFreeType frees the entire type including all
 *   sons!
 *
 ******************************************************************************/

void
TYFreeTypeConstructor (ntype *type)
{
    DBUG_ENTER ("TYFreeTypeConstructor");

    switch (NTYPE_CON (type)) {
    case TC_symbol:
        SYMBOL_MOD (type) = Free (SYMBOL_MOD (type));
        SYMBOL_NAME (type) = Free (SYMBOL_NAME (type));
        break;
    case TC_aks:
        SHFreeShape (AKS_SHP (type));
        break;
    case TC_akd:
        SHFreeShape (AKD_SHP (type));
        break;
    case TC_ibase:
        TYFreeType (IBASE_BASE (type));
        break;
    case TC_ishape:
        SHFreeShape (ISHAPE_SHAPE (type));
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
        DBUG_ASSERT ((0 == 1), "trying to free illegal type constructor!");
    }
    type = Free (type);

    DBUG_VOID_RETURN;
}

void
TYFreeType (ntype *type)
{
    int i;

    DBUG_ENTER ("TYFreeType");

    for (i = 0; i < NTYPE_ARITY (type); i++) {
        TYFreeType (NTYPE_SON (type, i));
    }
    TYFreeTypeConstructor (type);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    ntype * TYCopyType( ntype *type)
 *    ntype * TYCopyFixedType( ntype *type)
 *    ntype * TYDeriveSubtype( ntype *type)
 *
 * description:
 *    all these function copy types entirely! That does not only include
 *    copying the attributes but it includes copying the sons as well!
 *    The only difference of them is the way type variables are treated.
 *    - TYCopyType copies type variables as they are, i.e., they point to
 *      the same tvar structure, whereas
 *    - TYCopyFixedType does not copy type variables at all but inserts
 *      NULL pointers instead, and
 *    - TYDeriveSubtype creates a new tvar structure with the same upper
 *      bound!
 *    Therefore, all these functions are just wrappers for a static function
 *
 *          ntype * CopyType( ntype * type, TV_treatment new_tvars)
 *
 ******************************************************************************/

typedef enum {
    tv_id,  /* make a 1:1 copy  */
    tv_sub, /* create a subtype   */
    tv_none /* do not copy at all */
} TV_treatment;

static ntype *
CopyTypeConstructor (ntype *type, TV_treatment new_tvars)
{
    ntype *res;
    tvar *alpha;
    int i;
    bool ok;

    DBUG_ENTER ("CopyType");

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
        case TC_simple:
            SIMPLE_TYPE (res) = SIMPLE_TYPE (type);
            break;
        case TC_symbol:
            SYMBOL_MOD (res) = StringCopy (SYMBOL_MOD (type));
            SYMBOL_NAME (res) = StringCopy (SYMBOL_NAME (type));
            break;
        case TC_user:
            USER_TYPE (res) = USER_TYPE (type);
            break;
        case TC_aks:
            AKS_SHP (res) = SHCopyShape (AKS_SHP (type));
            break;
        case TC_akd:
            AKD_SHP (res) = SHCopyShape (AKD_SHP (type));
            AKD_DOTS (res) = AKD_DOTS (type);
            break;
        case TC_ibase:
            IBASE_BASE (res) = TYCopyType (IBASE_BASE (type));
            break;
        case TC_idim:
            IDIM_DIM (res) = IDIM_DIM (type);
            break;
        case TC_ishape:
            ISHAPE_SHAPE (res) = SHCopyShape (ISHAPE_SHAPE (type));
            break;
        case TC_ires:
            IRES_NUMFUNS (res) = IRES_NUMFUNS (type);
            if (IRES_NUMFUNS (type) != 0) {
                IRES_FUNDEFS (res)
                  = (node **)Malloc (IRES_NUMFUNS (type) * sizeof (node *));
                IRES_POSS (res) = (int *)Malloc (IRES_NUMFUNS (type) * sizeof (int));
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
                alpha = SSIMakeVariable ();
                SSINewMax (alpha, TYCopyType (SSIGetMax (ALPHA_SSI (type))));
                ALPHA_SSI (res) = alpha;
                ok = SSINewRel (alpha, ALPHA_SSI (type));
                DBUG_ASSERT (ok, "SSINewRel did not work in TYDeriveSubtype");
                break;
            case tv_none:
                res = Free (res);
                break;
            }
            break;
        default:
            break;
        }
    }

    DBUG_RETURN (res);
}

ntype *
TYCopyTypeConstructor (ntype *type)
{
    ntype *res;

    DBUG_ENTER ("TYCopyTypeConstructor");

    res = CopyTypeConstructor (type, tv_id);

    DBUG_RETURN (res);
}

ntype *
TYCopyType (ntype *type)
{
    ntype *res;
    int i;

    DBUG_ENTER ("TYCopyType");

    res = CopyTypeConstructor (type, tv_id);
    if (res != NULL) {
        NTYPE_ARITY (res) = NTYPE_ARITY (type);
        NTYPE_SONS (res) = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < NTYPE_ARITY (res); i++) {
            NTYPE_SON (res, i) = TYCopyType (NTYPE_SON (type, i));
        }
    }

    DBUG_RETURN (res);
}

ntype *
TYCopyFixedType (ntype *type)
{
    ntype *res;
    int i;

    DBUG_ENTER ("TYCopyFixedType");

    res = CopyTypeConstructor (type, tv_none);
    if (res != NULL) {
        NTYPE_ARITY (res) = NTYPE_ARITY (type);
        NTYPE_SONS (res) = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < NTYPE_ARITY (res); i++) {
            NTYPE_SON (res, i) = TYCopyFixedType (NTYPE_SON (type, i));
        }
    }

    DBUG_RETURN (res);
}

ntype *
TYDeriveSubtype (ntype *type)
{
    ntype *res;
    int i;

    DBUG_ENTER ("TYDeriveSubtype");

    res = CopyTypeConstructor (type, tv_sub);
    if (res != NULL) {
        NTYPE_ARITY (res) = NTYPE_ARITY (type);
        NTYPE_SONS (res) = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < NTYPE_ARITY (res); i++) {
            NTYPE_SON (res, i) = TYDeriveSubtype (NTYPE_SON (type, i));
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    char * TYType2String( ntype *type, bool multiline, int offset)
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
    static char buf[256];
    char *tmp = &buf[0];

    DBUG_ENTER ("ScalarType2String");

    switch (NTYPE_CON (type)) {
    case TC_simple:
        tmp += sprintf (tmp, "%s", mdb_type[SIMPLE_TYPE (type)]);
        break;
    case TC_symbol:
        if (SYMBOL_MOD (type) == NULL) {
            tmp += sprintf (tmp, "%s", SYMBOL_NAME (type));
        } else {
            tmp += sprintf (tmp, "%s:%s", SYMBOL_MOD (type), SYMBOL_NAME (type));
        }
        break;
    case TC_user:
        tmp += sprintf (tmp, "%s", UTGetName (USER_TYPE (type)));
        break;
    default:
        DBUG_ASSERT (0, "ScalarType2String called with non-scalar type!");
    }

    DBUG_RETURN (StringCopy (buf));
}

static char *
ArrayType2String (ntype *type)
{
    static char buf[512];
    char *tmp = &buf[0];
    char *tmp_str;

    DBUG_ENTER ("ArrayType2String");

    DBUG_ASSERT (type, "ArrayType2String called with NULL!");
    DBUG_ASSERT (TYIsArray (type), "ArrayType2String called with non-array type!");

    tmp_str = ScalarType2String (AKS_BASE (type));
    tmp += sprintf (tmp, "%s", tmp_str);
    tmp_str = Free (tmp_str);

    switch (NTYPE_CON (type)) {
    case TC_aks:
        if (TYGetDim (type) > 0) {
            tmp_str = SHShape2String (0, AKS_SHP (type));
            tmp += sprintf (tmp, "%s", tmp_str);
            tmp_str = Free (tmp_str);
        }
        break;
    case TC_akd:
        tmp_str = SHShape2String (AKD_DOTS (type), AKD_SHP (type));
        tmp += sprintf (tmp, "%s", tmp_str);
        tmp_str = Free (tmp_str);
        break;
    case TC_audgz:
        tmp += sprintf (tmp, "[+]");
        break;
    case TC_aud:
        tmp += sprintf (tmp, "[*]");
        break;
    default:
        DBUG_ASSERT (0, "ArrayType2String called with non-array type!");
    }

    DBUG_RETURN (StringCopy (buf));
}

static char *
PrintFunSep (char *tmp, bool multiline, int offset)
{
    DBUG_ENTER ("FunType2String");
    if (multiline) {
        tmp += sprintf (tmp, ",\n%*s", offset, "");
    } else {
        tmp += sprintf (tmp, ", ");
    }
    DBUG_RETURN (tmp);
}

static char *
FunType2String (ntype *type, char *scal_str, bool multiline, int offset)
{
    char buf[8192];
    char *tmp = &buf[0];
    char *tmp_str, *shp_str;
    shape *empty_shape;
    int i;
    int scal_len = 0;

    DBUG_ENTER ("FunType2String");

    switch (NTYPE_CON (type)) {
    case TC_fun:
        tmp += sprintf (tmp, "{ ");
        offset += 2;
        for (i = 0; i < NTYPE_ARITY (type); i++) {
            tmp_str = FunType2String (NTYPE_SON (type, i), scal_str, multiline, offset);
            if (i > 0) {
                tmp = PrintFunSep (tmp, multiline, offset);
            }
            tmp += sprintf (tmp, "%s", tmp_str);
            tmp_str = Free (tmp_str);
        }
        tmp += sprintf (tmp, "}");
        break;

    case TC_ibase:
        DBUG_ASSERT (IBASE_GEN (type), "fun type without generic instance!");
        DBUG_ASSERT ((scal_str == NULL),
                     "FunType2String called on ibase with non NULL scal_str!");

        scal_str = ScalarType2String (IBASE_BASE (type));
        scal_len = strlen (scal_str);

        /*
         * print "<scal_str>[*]" instance:
         */
        tmp_str
          = FunType2String (IBASE_GEN (type), scal_str, multiline, offset + scal_len + 3);
        tmp += sprintf (tmp, "%s[*]%s", scal_str, tmp_str);
        tmp_str = Free (tmp_str);

        /*
         * print "<scal_str>[]" instance:
         */
        if (IBASE_SCAL (type)) {
            tmp_str
              = FunType2String (IBASE_GEN (type), scal_str, multiline, offset + scal_len);
            tmp = PrintFunSep (tmp, multiline, offset);
            tmp += sprintf (tmp, "%s%s", scal_str, tmp_str);
            tmp_str = Free (tmp_str);
        }

        /*
         * Print TC_iarr
         */
        if (IBASE_IARR (type)) {
            tmp_str = FunType2String (IBASE_IARR (type), scal_str, multiline, offset);
            tmp += sprintf (tmp, "%s", tmp_str);
            tmp_str = Free (tmp_str);
        }

        scal_str = Free (scal_str);
        break;

    case TC_iarr:
        /*
         * print "<scal_str>[+]" instance:
         */
        if (IARR_GEN (type)) {
            scal_len = strlen (scal_str);
            tmp_str = FunType2String (IBASE_GEN (type), scal_str, multiline,
                                      offset + scal_len + 3);
            tmp = PrintFunSep (tmp, multiline, offset);
            tmp += sprintf (tmp, "%s[+]%s", scal_str, tmp_str);
            tmp_str = Free (tmp_str);
        }

        /*
         * Print TC_idims
         */

        for (i = 0; i < (NTYPE_ARITY (type) - 1); i++) {
            tmp_str = FunType2String (IARR_IDIM (type, i), scal_str, multiline, offset);
            tmp += sprintf (tmp, "%s", tmp_str);
            tmp_str = Free (tmp_str);
        }

        break;

    case TC_idim:
        /*
         * print "<scal_str>[<dots>]" instance:
         */
        if (IDIM_GEN (type)) {
            empty_shape = SHMakeShape (0);
            shp_str = SHShape2String (IDIM_DIM (type), empty_shape);
            empty_shape = SHFreeShape (empty_shape);

            tmp_str = FunType2String (IDIM_GEN (type), scal_str, multiline,
                                      offset + strlen (scal_str) + strlen (shp_str));
            tmp = PrintFunSep (tmp, multiline, offset);
            tmp += sprintf (tmp, "%s%s%s", scal_str, shp_str, tmp_str);
            shp_str = Free (shp_str);
            tmp_str = Free (tmp_str);
        }

        /*
         * Print TC_ishapes
         */

        for (i = 0; i < (NTYPE_ARITY (type) - 1); i++) {
            tmp_str = FunType2String (IDIM_ISHAPE (type, i), scal_str, multiline, offset);
            tmp += sprintf (tmp, "%s", tmp_str);
            tmp_str = Free (tmp_str);
        }

        break;

    case TC_ishape:
        /*
         * print "<scal_str>[<shp>]" instance:
         */
        if (ISHAPE_GEN (type)) {
            shp_str = SHShape2String (0, ISHAPE_SHAPE (type));

            tmp_str = FunType2String (IDIM_GEN (type), scal_str, multiline,
                                      offset + strlen (scal_str) + strlen (shp_str));
            tmp = PrintFunSep (tmp, multiline, offset);
            tmp += sprintf (tmp, "%s%s%s", scal_str, shp_str, tmp_str);
            shp_str = Free (shp_str);
            tmp_str = Free (tmp_str);
        }

        break;

    case TC_ires:
        offset += 4;
        tmp_str = TYType2String (IRES_TYPE (type), multiline, offset);
        tmp += sprintf (tmp, " -> %s", tmp_str);
        tmp_str = Free (tmp_str);
        break;
    default:
        DBUG_ASSERT (0, "FunType2String called with non-legal type!");
        break;
    }

    DBUG_RETURN (StringCopy (buf));
}

char *
TYType2String (ntype *type, bool multiline, int offset)
{
    char buf[4096];
    char *tmp = &buf[0];
    char *tmp_str, *res;
    int i;

    DBUG_ENTER ("TYType2String");

    if (type == NULL) {
        tmp += sprintf (tmp, "--");
        res = StringCopy (buf);
    } else {

        switch (NTYPE_CON (type)) {
        case TC_aud:
        case TC_audgz:
        case TC_akd:
        case TC_aks:
            res = ArrayType2String (type);
            break;
        case TC_fun:
            res = FunType2String (type, NULL, multiline, offset);
            break;
        case TC_prod:
            tmp += sprintf (tmp, "(");
            if (NTYPE_ARITY (type) > 0) {
                tmp_str = TYType2String (NTYPE_SON (type, 0), multiline, offset);
                tmp += sprintf (tmp, " %s", tmp_str);
                tmp_str = Free (tmp_str);
                for (i = 1; i < NTYPE_ARITY (type); i++) {
                    tmp_str = TYType2String (NTYPE_SON (type, i), multiline, offset);
                    tmp += sprintf (tmp, ", %s", tmp_str);
                    tmp_str = Free (tmp_str);
                }
            }
            tmp += sprintf (tmp, ")");
            res = StringCopy (buf);
            break;
        case TC_alpha:
            res = SSIVariable2DebugString (ALPHA_SSI (type));
            break;
        default:
            DBUG_ASSERT (0, "TYType2String applied to non-SAC type!");
            res = NULL;
            break;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    char * TYType2DebugString( ntype *type, bool multiline, int offset)
 *
 * description:
 *   constructs a string that represents the internal type constructor
 *   structure of the type!
 *   iff "multiline" is TRUE, strings for function types contain new line
 *   symbols. Each new line is followed by "offset" preceeding blanks.
 *
 ******************************************************************************/

char *
TYType2DebugString (ntype *type, bool multiline, int offset)
{
    char buf[32768];
    char *tmp = &buf[0];
    char *tmp_str;
    int i, n;

    DBUG_ENTER ("TYType2DebugString");

    if (type == NULL) {
        tmp += sprintf (tmp, "--");
    } else {
        tmp += sprintf (tmp, "%s{ ", dbug_str[NTYPE_CON (type)]);

        switch (NTYPE_CON (type)) {
        case TC_aks:
            multiline = FALSE;
            tmp_str = SHShape2String (0, AKS_SHP (type));
            tmp += sprintf (tmp, "%s, ", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        case TC_akd:
            multiline = FALSE;
            tmp_str = SHShape2String (AKD_DOTS (type), AKD_SHP (type));
            tmp += sprintf (tmp, "%s, ", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        case TC_aud:
            multiline = FALSE;
            break;
        case TC_simple:
            multiline = FALSE;
            tmp += sprintf (tmp, "%s", mdb_type[SIMPLE_TYPE (type)]);
            break;
        case TC_symbol:
            multiline = FALSE;
            if (SYMBOL_MOD (type) == NULL) {
                tmp += sprintf (tmp, "%s", SYMBOL_NAME (type));
            } else {
                tmp += sprintf (tmp, "%s:%s", SYMBOL_MOD (type), SYMBOL_NAME (type));
            }
            break;
        case TC_user:
            multiline = FALSE;
            tmp += sprintf (tmp, "%d", USER_TYPE (type));
            break;
        case TC_ibase:
            tmp_str = TYType2DebugString (IBASE_BASE (type), FALSE, offset);
            tmp += sprintf (tmp, "%s,", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        case TC_idim:
            tmp += sprintf (tmp, "%d,", IDIM_DIM (type));
            break;
        case TC_ishape:
            tmp_str = SHShape2String (0, ISHAPE_SHAPE (type));
            tmp += sprintf (tmp, "%s,", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        case TC_ires:
            if (IRES_NUMFUNS (type) > 0) {
                tmp += sprintf (tmp, "poss: {");
                for (i = 0; i < IRES_NUMFUNS (type); i++) {
                    tmp += sprintf (tmp, "%d ", IRES_POS (type, i));
                }
                tmp += sprintf (tmp, "} ");
            }
            if (IRES_NUMFUNS (type) > 0) {
                tmp += sprintf (tmp, "fundefs: {");
                for (i = 0; i < IRES_NUMFUNS (type); i++) {
                    tmp += sprintf (tmp, "%p ", IRES_FUNDEF (type, i));
                }
                tmp += sprintf (tmp, "} ");
            }
            break;
        case TC_alpha:
            multiline = FALSE;
            tmp_str = SSIVariable2DebugString (ALPHA_SSI (type));
            tmp += sprintf (tmp, "%s", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        default:
            break;
        }

        if (variable_arity[NTYPE_CON (type)]) {
            tmp += sprintf (tmp, " <");
        }
        n = NTYPE_ARITY (type);
        offset += 3;
        for (i = 0; i < n; i++) {
            tmp_str = TYType2DebugString (NTYPE_SON (type, i), multiline, offset);
            if (i == 0) {
                if (multiline) {
                    tmp += sprintf (tmp, "\n%*s", offset - 1, "");
                }
                tmp += sprintf (tmp, " %s", tmp_str);
            } else {
                tmp = PrintFunSep (tmp, multiline, offset);
                tmp += sprintf (tmp, "%s", tmp_str);
            }
            tmp_str = Free (tmp_str);
        }
        offset -= 3;
        if (variable_arity[NTYPE_CON (type)]) {
            tmp += sprintf (tmp, ">");
        }
        tmp += sprintf (tmp, "}");
    }

    DBUG_RETURN (StringCopy (buf));
}

/******************************************************************************
 *
 * function:
 *    ntype * TYNestTypes( ntype *outer, ntype *inner)
 *
 * description:
 *    nests (array) types. Since this function is NOT considered a type
 *    constructing (MakeXYZ) function, it does NOT re-use the argument types
 *    but inspects them only!
 *
 ******************************************************************************/

#if AKD_MAY_CONTAIN_SHAPE

ntype *
TYNestTypes (ntype *outer, ntype *inner)
{
    ntype *res;

    DBUG_ENTER ("TYNestTypes");

    if (NTYPE_CON (outer) == TC_aks) {
        /*
         * AKS{ a, s1}, AKS{ b, s2}      => AKS{ b, s1++s2}}
         * AKS{ a, s1}, AKD{ b, do2, s2} => AKD{ b, d1+do2, s2}
         * AKS{ a, s1}, AUD{ b}          => AUD{ b}
         * AKS{ a, s1}, b                => AKS{ b, s1}
         *
         */
        if (NTYPE_CON (inner) == TC_aks) {
            res = TYMakeAKS (TYCopyType (AKS_BASE (inner)),
                             SHAppendShapes (AKS_SHP (outer), AKS_SHP (inner)));
        } else if (NTYPE_CON (inner) == TC_akd) {
            res = TYMakeAKD (TYCopyType (AKD_BASE (inner)),
                             TYGetDim (outer) + AKD_DOTS (inner),
                             SHCopyShape (AKD_SHP (inner)));
        } else if (NTYPE_CON (inner) == TC_aud) {
            res = TYMakeAUD (TYCopyType (AKS_BASE (inner)));
        } else {
            res = TYMakeAKS (TYCopyType (inner), SHCopyShape (AKS_SHP (outer)));
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
            res = TYMakeAKD (TYCopyType (AKS_BASE (inner)), AKD_DOTS (outer),
                             SHAppendShapes (AKD_SHP (outer), AKS_SHP (inner)));
            res = TYMakeAKD (TYCopyType (AKD_BASE (inner)),
                             TYGetDim (outer) + AKD_DOTS (inner),
                             SHCopyShape (AKD_SHP (inner)));
        } else if (NTYPE_CON (inner) == TC_aud) {
            res = TYMakeAUD (TYCopyType (AKS_BASE (inner)));
        } else {
            res = TYMakeAKD (TYCopyType (inner), AKD_DOTS (inner),
                             SHCopyShape (AKD_SHP (outer)));
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
            res = TYMakeAUD (TYCopyType (AKS_BASE (inner)));
        } else if (NTYPE_CON (inner) == TC_akd) {
            res = TYMakeAUD (TYCopyType (AKD_BASE (inner)));
        } else if (NTYPE_CON (inner) == TC_aud) {
            res = TYMakeAUD (TYCopyType (AUD_BASE (inner)));
        } else {
            res = TYMakeAUD (TYCopyType (inner));
        }
    } else {
        /*
         * a, b => b
         *
         */
        res = TYCopyType (inner);
    }

    DBUG_RETURN (res);
}

#else /* AKD_MAY_CONTAIN_SHAPE */

ntype *
TYNestTypes (ntype *outer, ntype *inner)
{
    ntype *res;

    DBUG_ENTER ("TYNestTypes");

    switch (NTYPE_CON (outer)) {
    case TC_aks:
        /*
         * AKS{ a, s1}, AKS{ b, s2}      => AKS{ b, s1++s2}}
         * AKS{ a, s1}, AKD{ b, do2, --} => AKD{ b, d1+do2, --}
         * AKS{ a, s1}, AUDGZ{ b}        => AUDGZ{ b}
         * AKS{ a, s1}, AUD{ b}          => AUDGZ{ b} / AUD{ b}
         * AKS{ a, s1}, b                => AKS{ b, s1}
         *
         */
        switch (NTYPE_CON (inner)) {
        case TC_aks:
            res = TYMakeAKS (TYCopyType (AKS_BASE (inner)),
                             SHAppendShapes (AKS_SHP (outer), AKS_SHP (inner)));
            break;
        case TC_akd:
            res = TYMakeAKD (TYCopyType (AKD_BASE (inner)),
                             TYGetDim (outer) + AKD_DOTS (inner), SHMakeShape (0));
            break;
        case TC_audgz:
            res = TYCopyType (inner);
            break;
        case TC_aud:
            if (TYGetDim (outer) > 0) {
                res = TYMakeAUDGZ (TYCopyType (AKD_BASE (inner)));
            } else {
                res = TYCopyType (inner);
            }
            break;
        default:
            res = TYMakeAKS (TYCopyType (inner), SHCopyShape (AKS_SHP (outer)));
        }
        break;

    case TC_akd:
        /*
         * AKD{ a, do1, --}, AKS{ b, s2}      => AKD{ b, do1+d2, --}}
         * AKD{ a, do1, --}, AKD{ b, do2, --} => AKD{ b, do1+do2, --}
         * AKD{ a, do1, --}, AUDGZ{ b}        => AUDGZ{ b}
         * AKD{ a, do1, --}, AUD{ b}          => AUDGZ{ b} / AUD{ b}
         * AKD{ a, do1, --}, b                => AKD{ b, do1, --}
         *
         */
        switch (NTYPE_CON (inner)) {
        case TC_aks:
        case TC_akd:
            res = TYMakeAKD (TYCopyType (AKD_BASE (inner)),
                             TYGetDim (outer) + TYGetDim (inner), SHMakeShape (0));
            break;
        case TC_audgz:
            res = TYCopyType (inner);
            break;
        case TC_aud:
            if (TYGetDim (outer) > 0) {
                res = TYMakeAUDGZ (TYCopyType (AKD_BASE (inner)));
            } else {
                res = TYCopyType (inner);
            }
            break;
        default:
            res = TYMakeAKD (TYCopyType (inner), AKD_DOTS (inner),
                             SHCopyShape (AKD_SHP (outer)));
        }
        break;

    case TC_audgz:
        /*
         * AUDGZ{ a}, AKS{ b, s2}      => AUDGZ{ b}
         * AUDGZ{ a}, AKD{ b, do2, s2} => AUDGZ{ b}
         * AUDGZ{ a}, AUDGZ{ b}        => AUDGZ{ b}
         * AUDGZ{ a}, AUD{ b}          => AUDGZ{ b}
         * AUDGZ{ a}, b                => AUDGZ{ b}
         *
         */
        res = TYMakeAUDGZ (TYCopyType (AKD_BASE (inner)));
        break;

    case TC_aud:
        /*
         * AUD{ a}, AKS{ b, s2}      => AUDGZ{ b} / AUD{ b}
         * AUD{ a}, AKD{ b, do2, s2} => AUDGZ{ b} / AUD{ b}
         * AUD{ a}, AUDGZ{ b}        => AUDGZ{ b}
         * AUD{ a}, AUD{ b}          => AUD{ b}
         * AUD{ a}, b                => AUD{ b}
         *
         */
        switch (NTYPE_CON (inner)) {
        case TC_aks:
        case TC_akd:
            if (TYGetDim (inner) > 0) {
                res = TYMakeAUDGZ (TYCopyType (AKD_BASE (inner)));
            } else {
                res = TYMakeAUD (TYCopyType (AKD_BASE (inner)));
            }
            break;
        case TC_audgz:
        case TC_aud:
            res = TYCopyType (inner);
            break;
        default:
            res = TYMakeAUD (TYCopyType (inner));
            break;
        }
        break;

    default:
        /*
         * a, b => b
         *
         */
        res = TYCopyType (inner);
        break;
    }

    DBUG_RETURN (res);
}

#endif /* AKD_MAY_CONTAIN_SHAPE */

/******************************************************************************
 *
 * function:
 *    ntype * TYOldType2Type( types * old)
 *
 * description:
 *    converts an old TYPES node into an ntype node (or - if neccessary -
 *    a nesting of ntype nodes).
 *
 ******************************************************************************/

ntype *
TYOldType2Type (types *old)
{
    ntype *res;
    usertype udt;

#ifndef DBUG_OFF
    char *tmp, *tmp2;
#endif

    DBUG_ENTER ("TYOldType2Type");

    switch (TYPES_BASETYPE (old)) {
    case T_user:
        udt = UTFindUserType (TYPES_NAME (old), TYPES_MOD (old));
        if (udt == UT_NOT_DEFINED) {
            res = TYMakeSymbType (StringCopy (TYPES_NAME (old)),
                                  StringCopy (TYPES_MOD (old)));
        } else {
            res = TYMakeUserType (udt);
        }
        break;
    case T_int:
    case T_short:
    case T_long:
    case T_uint:
    case T_ushort:
    case T_ulong:
    case T_float:
    case T_double:
    case T_longdbl:
    case T_bool:
    case T_str:
    case T_char:
        res = TYMakeSimpleType (TYPES_BASETYPE (old));
        break;
    case T_hidden:
    case T_void:
    case T_dots:
    case T_nothing:
    case T_unknown:
    default:
        res = NULL;
    }

    if (res != NULL) {
        if (TYPES_DIM (old) > SCALAR) {
            res = TYMakeAKS (res, SHOldTypes2Shape (old));
        } else if (TYPES_DIM (old) < KNOWN_DIM_OFFSET) {
            res = TYMakeAKD (res, KNOWN_DIM_OFFSET - TYPES_DIM (old), SHMakeShape (0));
        } else if (TYPES_DIM (old) == UNKNOWN_SHAPE) {
            res = TYMakeAUDGZ (res);
        } else if (TYPES_DIM (old) == ARRAY_OR_SCALAR) {
            res = TYMakeAUD (res);
        } else { /* TYPES_DIM( old) == SCALAR */
            res = TYMakeAKS (res, SHCreateShape (0));
        }
    }

    DBUG_EXECUTE ("NTY", (tmp = Type2String (old, 3, TRUE),
                          tmp2 = TYType2DebugString (res, TRUE, 0)););
    DBUG_PRINT ("NTY", ("%s converted into : %s\n", tmp, tmp2));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    types * TYType2OldType( ntype * new)
 *
 * description:
 *
 *
 ******************************************************************************/

static types *
Type2OldType (ntype *new)
{
    types *res = NULL;
    types *tmp = NULL;
    int i;

    DBUG_ENTER ("Type2OldType");

    switch (NTYPE_CON (new)) {
    case TC_alpha:
        DBUG_ASSERT ((TYCmpTypes (SSIGetMin (TYGetAlpha (new)),
                                  SSIGetMax (TYGetAlpha (new)))
                      == TY_eq),
                     "Type2OldType applied to non-unique alpha type");
        res = Type2OldType (SSIGetMin (TYGetAlpha (new)));
        break;
    case TC_prod:
        for (i = NTYPE_ARITY (new) - 1; i >= 0; i--) {
            res = Type2OldType (PROD_MEMBER (new, i));
            TYPES_NEXT (res) = tmp;
            tmp = res;
        }
        break;
    case TC_aks:
        res = Type2OldType (AKS_BASE (new));
        TYPES_DIM (res) = SHGetDim (AKS_SHP (new));
        TYPES_SHPSEG (res) = SHShape2OldShpseg (AKS_SHP (new));
        break;
    case TC_akd:
        res = Type2OldType (AKD_BASE (new));
        TYPES_DIM (res) = KNOWN_DIM_OFFSET - AKD_DOTS (new);
        break;
    case TC_audgz:
        res = Type2OldType (AUDGZ_BASE (new));
        TYPES_DIM (res) = UNKNOWN_SHAPE;
        break;
    case TC_aud:
        res = Type2OldType (AUD_BASE (new));
        TYPES_DIM (res) = ARRAY_OR_SCALAR;
        break;
    case TC_simple:
        res = MakeTypes (SIMPLE_TYPE (new), 0, NULL, NULL, NULL);
        break;
    default:
        DBUG_ASSERT ((0), "Type2OldType not yet entirely implemented!");
        res = NULL;
        break;
    }

    DBUG_RETURN (res);
}

types *
TYType2OldType (ntype *new)
{
    types *res;
#ifndef DBUG_OFF
    char *tmp_str, *tmp_str2;
#endif

    DBUG_ENTER ("Type2OldType");

    res = Type2OldType (new);

    DBUG_EXECUTE ("NTY", tmp_str = TYType2DebugString (new, FALSE, 0););
    DBUG_EXECUTE ("NTY", tmp_str2 = Type2String (res, 0, TRUE););
    DBUG_PRINT ("NTY", ("converting %s into %s", tmp_str, tmp_str2));
    DBUG_EXECUTE ("NTY", tmp_str = Free (tmp_str););
    DBUG_EXECUTE ("NTY", tmp_str2 = Free (tmp_str2););

    DBUG_RETURN (res);
}
