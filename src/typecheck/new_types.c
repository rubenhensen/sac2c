/*
 * $Log$
 * Revision 1.1  1999/10/20 12:52:00  sbs
 * Initial revision
 *
 *
 */

#include "dbug.h"

#include "free.h"
#include "tree.h"
#include "convert.h"
#include "types.h"

#include "shape.h"

typedef enum {
#define TCITypeConstr(a) a
#include "type_constructor_info.mac"
} typeconstr;

typedef struct AKD {
    shape *shp;
    int dots;
} akd;

typedef struct SYMBOL {
    char *mod;
    char *name;
} symbol;

typedef union {
    simpletype a_simple;
    symbol a_symbol;
    usertype a_user;
    shape *a_aks;
    akd a_akd;
} typeattr;

typedef struct NTYPE {
    typeconstr typeconstr;
    int arity;
    typeattr typeattr;
    struct NTYPE **sons;
} ntype;

#define SELF
#include "new_types.h"

#include "user_types.h" /* has to be included here since it includes new_types.h! */

static char *dbug_str[] = {
#define TCIDbugString(a) a
#include "type_constructor_info.mac"
};

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

    res = (ntype *)MALLOC (sizeof (ntype));
    NTYPE_CON (res) = con;
    NTYPE_ARITY (res) = arity;
    if (NTYPE_ARITY (res) > 0) {
        NTYPE_SONS (res) = (ntype **)MALLOC (sizeof (ntype *) * NTYPE_ARITY (res));
    }
    return (res);
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

    res = MakeNtype (TC_simple, 0);
    SIMPLE_TYPE (res) = base;
    return (res);
}

ntype *
TYMakeSymbType (char *name, char *mod)
{
    ntype *res;

    res = MakeNtype (TC_symbol, 0);
    SYMBOL_MOD (res) = mod;
    SYMBOL_NAME (res) = name;
    return (res);
}

ntype *
TYMakeUserType (usertype udt)
{
    ntype *res;

    res = MakeNtype (TC_user, 0);
    USER_TYPE (res) = udt;
    return (res);
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
    DBUG_ASSERT ((NTYPE_CON (symb) == TC_symbol), "TYGetName applied to nonsymbol-type!");
    return (SYMBOL_NAME (symb));
}

char *
TYGetMod (ntype *symb)
{
    DBUG_ASSERT ((NTYPE_CON (symb) == TC_symbol), "TYGetMod applied to nonsymbol-type!");
    return (SYMBOL_MOD (symb));
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

    res = MakeNtype (TC_aks, 1);
    AKS_SHP (res) = shp;
    AKS_BASE (res) = scalar;
    return (res);
}

ntype *
TYMakeAKD (ntype *scalar, int dots, shape *shp)
{
    ntype *res;

    res = MakeNtype (TC_akd, 1);
    AKD_DOTS (res) = dots;
    AKD_SHP (res) = shp;
    AKD_BASE (res) = scalar;
    return (res);
}

ntype *
TYMakeAUD (ntype *scalar)
{
    ntype *res;

    res = MakeNtype (TC_aud, 1);
    AUD_BASE (res) = scalar;
    return (res);
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
    return (res);
}

shape *
TYGetShape (ntype *array)
{
    shape *res;

    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akd),
                 "TYGetShape applied to ther than AKS or AKD type!");
    if (NTYPE_CON (array) == TC_aks) {
        res = AKS_SHP (array);
    } else {
        res = AKD_SHP (array);
    }
    return (res);
}

ntype *
TYGetScalar (ntype *array)
{
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akd)
                   || (NTYPE_CON (array) == TC_aud),
                 "TYGetScalar applied to ther than array type!");

    return (NTYPE_SON (array, 0));
}

/******************************************************************************
 *
 * function:
 *    ntype * TYMakeUnion( ntype *t1, ntype *t2)
 *
 * description:
 *  functions for creating union-types.
 *
 ******************************************************************************/

ntype *
TYMakeUnion (ntype *t1, ntype *t2)
{
    ntype *res;
    int arity = 2, pos = 0;
    int i;

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

    return (res);
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
    return (NTYPE_CON (type) == TC_simple);
}

bool
TYIsUser (ntype *type)
{
    return (NTYPE_CON (type) == TC_user);
}

bool
TYIsSymb (ntype *type)
{
    return (NTYPE_CON (type) == TC_symbol);
}

bool
TYIsScalar (ntype *type)
{
    return (NTYPE_ARITY (type) == 0);
}

bool
TYIsAKS (ntype *type)
{
    return (NTYPE_CON (type) == TC_aks);
}

bool
TYIsAKD (ntype *type)
{
    return (NTYPE_CON (type) == TC_akd);
}

bool
TYIsAUD (ntype *type)
{
    return (NTYPE_CON (type) == TC_aud);
}

bool
TYIsUnion (ntype *type)
{
    return (NTYPE_CON (type) == TC_union);
}

bool
TYIsProd (ntype *type)
{
    return (NTYPE_CON (type) == TC_prod);
}

bool
TYIsFun (ntype *type)
{
    return (NTYPE_CON (type) == TC_fun);
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
    return ((NTYPE_CON (type) == TC_aks) && (NTYPE_CON (AKS_BASE (type)) == TC_symbol));
}

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
    return (TY_unrel);
}

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
}

void
TYFreeType (ntype *type)
{
    int i;

    for (i = 0; i < NTYPE_ARITY (type); i++) {
        TYFreeType (NTYPE_SON (type, i));
    }
    TYFreeTypeConstructor (type);
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

    return (res);
}

/******************************************************************************
 *
 * function:
 *    char * TYType2String( ntype *type)
 *
 * description:
 *
 ******************************************************************************/

char *
TYType2String (ntype *type)
{
    return ("not yet done");
}

/******************************************************************************
 *
 * function:
 *    char * TYType2DebugString( ntype *type)
 *
 * description:
 *
 ******************************************************************************/

char *
TYType2DebugString (ntype *type)
{
    char buf[256];
    char *tmp = &buf[0];
    char *tmp_str;
    int i, n;

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

    return (StringCopy (buf));
}

/******************************************************************************
 *
 * function:
 *    ntype * TYNestTypes( ntype *outer, ntype *inner)
 *
 * description:
 *    nests (array) types.
 *
 ******************************************************************************/

ntype *
TYNestTypes (ntype *outer, ntype *inner)
{
    ntype *res;

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
    return (res);
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
        res = TYMakeUnion (TYMakeAUD (res), res);
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
 *
 ******************************************************************************/

types *
TYType2OldType (ntype *new)
{

    return (MakeType (T_int, 0, NULL, "no yet", "implemented"));
}
