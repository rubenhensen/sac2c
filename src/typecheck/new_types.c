/*
 * $Log$
 * Revision 1.2  1999/10/20 15:45:10  sbs
 * some code brushing done.
 *
 * Revision 1.1  1999/10/20 12:52:00  sbs
 * Initial revision
 *
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
 *   There are EXACTLY TWO CLASSES OF FUNCTIONS that are an EXEPTION OF
 *   THIS RULE: - the MAKExyz - functions for generating ntype structures
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

#include "dbug.h"

#include "free.h"
#include "tree.h"
#include "convert.h"
#include "types.h"

#include "shape.h"

/*
 * enum type for type constructors:
 */

typedef enum {
#define TCITypeConstr(a) a
#include "type_constructor_info.mac"
} typeconstr;

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
} typeattr;

/*
 * Finally, the new type structure "ntype" can be defined:
 *  it consists of - a type constructor tag
 *                 - the arity of the constructor
 *                 - its respective attribute
 *                 - a pointer to a list of (arity-many) son-constructors
 */

typedef struct NTYPE {
    typeconstr typeconstr;
    int arity;
    typeattr typeattr;
    struct NTYPE **sons;
} ntype;

/*
 * Now, we include the own interface! The reason fot this is twofold:
 * First, it ensures consistency betweeen the interface and the
 * implementation and second, it serves as a forward declaration for all
 * functions.
 * The only problem this technique has is that we either have to "export"
 * the definition of the the type "ntype" (which we want to avoid for
 * software engeneering reasons) or we have to include all but the
 *   typedef void ntype;
 * We achieve the later by using the compilation flag "SELF"
 */

#define SELF
#include "new_types.h"

#include "user_types.h" /* has to be included here since it includes new_types.h! */

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

/*
 * Macros for accessing the sons...
 */
#define AKS_BASE(n) (n->sons[0])
#define AKD_BASE(n) (n->sons[0])
#define AUD_BASE(n) (n->sons[0])
#define UNION_MEMBER(n, i) (n->sons[i])

/*
 * For dbug-output purposes we keep an array of strings for the individual
 * type constructors:
 */

static char *dbug_str[] = {
#define TCIDbugString(a) a
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

    DBUG_ENTER ("MakeNtype");

    res = (ntype *)MALLOC (sizeof (ntype));
    NTYPE_CON (res) = con;
    NTYPE_ARITY (res) = arity;
    if (NTYPE_ARITY (res) > 0) {
        NTYPE_SONS (res) = (ntype **)MALLOC (sizeof (ntype *) * NTYPE_ARITY (res));
    } else {
        NTYPE_SONS (res) = NULL;
    }

    DBUG_RETURN (res);
}

/***
 *** Functions for creating and inspecting ntypes (MakeXYZ, GetXYZ):
 ***   these functions are the sole functions that use arg-pointers as
 ***   part of their result!
 ***/

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
 *    char * TYGetName( ntype *symb)
 *    char * TYGetMod( ntype *symb)
 *
 * description:
 *  several functions for extracting the attributes from scalar types.
 *
 ******************************************************************************/

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
TYMakeAUD (ntype *scalar)
{
    ntype *res;

    DBUG_ENTER ("TYMakeAUD");

    res = MakeNtype (TC_aud, 1);
    AUD_BASE (res) = scalar;

    DBUG_RETURN (res);
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
                   || (NTYPE_CON (array) == TC_aud),
                 "TYGetScalar applied to ther than array type!");
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
 *    ntype * TYMakeFunType( ntype *arg, ntype *res)
 *    ntype * TYMakeProdType( ntype *res)
 *    void    TYInsertIntoProdType( ntype *prod, ntype *res)
 *
 * description:
 *  functions for creating function and product types.
 *
 ******************************************************************************/

#if 0
ntype *  TYMakeFunType( ntype *arg, ntype *res)
ntype *  TYMakeProdType( ntype *res)
void     TYInsertIntoProdType( ntype *prod, ntype *res)
#endif

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
 *    bool TYIsAKS( ntype * type)
 *    bool TYIsAKD( ntype * type)
 *    bool TYIsAUD( ntype * type)
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

/***
 *** functions that check for the relationship of types:
 ***/

/******************************************************************************
 *
 * function:
 *    CT_res TYCmpTypes( ntype * t1, ntype * t2)
 *
 * description:
 *    NOT YET IMPLEMENTED!
 *
 ******************************************************************************/

CT_res
TYCmpTypes (ntype *t1, ntype *t2)
{
    DBUG_ENTER ("TYCmpTypes");
    DBUG_RETURN (TY_unrel);
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
        FREE (SYMBOL_MOD (type));
        FREE (SYMBOL_NAME (type));
        break;
    case TC_aks:
        SHFreeShape (AKS_SHP (type));
        break;
    case TC_akd:
        SHFreeShape (AKD_SHP (type));
        break;
    case TC_aud:
    case TC_union:
    case TC_prod:
    case TC_fun:
    case TC_simple:
    case TC_user:
        break;
    default:
        DBUG_ASSERT ((0 == 1), "illegal type constructor!");
    }
    FREE (type);

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
 *
 * description:
 *    copies type entirely! That does not only include copying the attributes
 *    but it includes copying the sons as well!
 *
 ******************************************************************************/

ntype *
TYCopyType (ntype *type)
{
    ntype *res;
    int i, n;

    DBUG_ENTER ("TYCopyType");

    /*
     * First, we copy the type node itself!
     */
    n = NTYPE_ARITY (type);
    res = MakeNtype (NTYPE_CON (type), n);

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
    default:
        break;
    }

    /*
     * Now, we recursively copy all son nodes:
     */
    for (i = 0; i < n; i++) {
        NTYPE_SON (res, i) = TYCopyType (NTYPE_SON (type, i));
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    char * TYType2String( ntype *type)
 *
 * description:
 *    NOT YET IMPLEMENTED!
 *
 ******************************************************************************/

char *
TYType2String (ntype *type)
{
    DBUG_ENTER ("TYType2String");
    DBUG_RETURN ("not yet done");
}

/******************************************************************************
 *
 * function:
 *    char * TYType2DebugString( ntype *type)
 *
 * description:
 *   constructs a string that represents the internal type constructor
 *   structure of the type!
 *
 ******************************************************************************/

char *
TYType2DebugString (ntype *type)
{
    char buf[256];
    char *tmp = &buf[0];
    char *tmp_str;
    int i, n;

    DBUG_ENTER ("TYType2DebugString");

    tmp += sprintf (tmp, "%s{ ", dbug_str[NTYPE_CON (type)]);

    switch (NTYPE_CON (type)) {
    case TC_aks:
        tmp_str = SHShape2String (0, AKS_SHP (type));
        tmp += sprintf (tmp, "%s,", tmp_str);
        FREE (tmp_str);
        break;
    case TC_akd:
        tmp_str = SHShape2String (AKD_DOTS (type), AKD_SHP (type));
        tmp += sprintf (tmp, "%s,", tmp_str);
        FREE (tmp_str);
        break;
    case TC_aud:
        break;
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
        tmp += sprintf (tmp, "%d", USER_TYPE (type));
        break;
    default:
        break;
    }

    n = NTYPE_ARITY (type);
    for (i = 0; i < n; i++) {
        tmp_str = TYType2DebugString (NTYPE_SON (type, i));
        if (i == 0) {
            tmp += sprintf (tmp, " %s", tmp_str);
        } else {
            tmp += sprintf (tmp, ", %s", tmp_str);
        }
        FREE (tmp_str);
    }
    tmp += sprintf (tmp, "}");

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

/******************************************************************************
 *
 * function:
 *    ntype * TYOldType2Type( types * old, type_conversion_flag flag)
 *
 * description:
 *    converts an old TYPES node into an ntype node (or - if neccessary -
 *    a nesting of ntype nodes).
 *
 ******************************************************************************/

ntype *
TYOldType2Type (types *old, type_conversion_flag flag)
{
    ntype *res;

#ifndef DBUG_OFF
    char *tmp, *tmp2;
#endif

    DBUG_ENTER ("TYOldType2Type");

    switch (TYPES_BASETYPE (old)) {
    case T_user:
        if (flag == TY_symb) {
            res = TYMakeSymbType (TYPES_NAME (old), TYPES_MOD (old));
        } else {
            res = TYMakeUserType (UTFindUserType (TYPES_NAME (old), TYPES_MOD (old)));
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

    if (TYPES_DIM (old) > SCALAR) {
        res = TYMakeAKS (res, SHOldTypes2Shape (old));
    } else if (TYPES_DIM (old) < KNOWN_DIM_OFFSET) {
        res = TYMakeAKD (res, KNOWN_DIM_OFFSET - TYPES_DIM (old), NULL);
    } else if (TYPES_DIM (old) == UNKNOWN_SHAPE) {
        res = TYMakeAUD (res);
    } else if (TYPES_DIM (old) == ARRAY_OR_SCALAR) {
        res = TYMakeUnionType (TYMakeAUD (res), res);
    } else { /* TYPES_DIM( old) == SCALAR */
    }

    DBUG_EXECUTE ("NTY", (tmp = Type2String (old, 3), tmp2 = TYType2DebugString (res)););
    DBUG_PRINT ("NTY", ("%s converted into : %s\n", tmp, tmp2));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    types * TYType2OldType( ntype * new)
 *
 * description:
 *   NOT YET IMPLEMENTED!
 *
 ******************************************************************************/

types *
TYType2OldType (ntype *new)
{

    DBUG_ENTER ("TYType2OldType");
    DBUG_RETURN (MakeType (T_int, 0, NULL, "no yet", "implemented"));
}
