/*
 *
 * $Log$
 * Revision 3.65  2004/10/28 16:09:51  sah
 * removed a not needed variable
 * andd added some DBUG_PRINTs
 *
 * Revision 3.64  2004/10/26 11:37:40  sah
 * Serialization support now hidden outside of NEW_AST mode
 *
 * Revision 3.63  2004/10/26 11:07:32  sah
 * rewrote TYFixAndEliminateAlpha
 *
 * Revision 3.62  2004/10/25 20:24:15  sah
 * missing , added
 *
 * Revision 3.61  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 3.60  2004/10/01 08:14:36  sah
 * initialised var to avoid compiler warning
 *
 * Revision 3.59  2004/09/30 17:09:57  sah
 * removed TYArgs2FunType
 * added TYArgs2FunTypeString
 *
 * Revision 3.58  2004/09/30 15:12:15  sbs
 * NTY_MEM DBUG_PRINTs added.
 *
 * Revision 3.57  2004/09/29 17:28:44  sbs
 * fixed three DBUG_PRINT memory leaks.
 *
 * Revision 3.56  2004/09/29 13:47:08  sah
 * added TYArgs2FunType
 *
 * Revision 3.55  2004/09/27 19:09:28  sbs
 * 1) sharing of components from FUNDEF_RET_TYPE as introduced
 * while building Applications removed.
 * 2) TYMakeFunType now handles akv types by upgrading them to aks types.
 *
 * Revision 3.54  2004/09/27 13:15:20  sah
 * added serialization support
 *
 * Revision 3.53  2004/07/05 17:26:47  sbs
 * some DBUG output is added.
 *
 * Revision 3.52  2004/03/05 12:08:54  sbs
 * TYOldType2ScalarType added.
 *
 * Revision 3.51  2003/06/17 16:56:17  sbs
 * bug in MakeOverloadedFunType eliminated
 * Now, overloading of functions with disjoint result type specifications
 * (eg. nt and int 8-) is detected and leads to program abortion!
 *
 * Revision 3.50  2003/06/17 13:58:40  sbs
 * inserted DeleteSon. This function is needed in FilterFundefs:
 * if all potential functions are gone, the respective (non-generic)
 * branch has to be taken off the list of sons!!!
 * => bug 17 exploits this requirement!
 *
 * Revision 3.49  2003/06/11 21:45:46  ktr
 * replaced calls of MakeArray with MakeFlatArray
 *
 * Revision 3.48  2003/05/30 14:21:56  dkr
 * TYStaticDispatchWrapper() removed
 *
 * Revision 3.47  2003/05/29 12:30:41  dkr
 * comment for TYStaticDispatchWrapper() added
 *
 * Revision 3.46  2003/04/08 08:14:57  sbs
 * nasty error in CmpTypes eliminated
 *
 * Revision 3.45  2003/04/07 14:31:32  sbs
 * support for AKV types added.
 * functions TYGetValue, TYIsProdOfAKV, and TYIsProdContainingAKV built
 *
 * Revision 3.44  2003/04/01 17:12:57  sbs
 * started integrating TY_akv i.e. constant types ....
 *
 * Revision 3.43  2003/03/17 19:45:03  dkr
 * CreateWrapperCode() modified: [+] is handled correctly now
 *
 * Revision 3.42  2003/03/12 14:50:03  dkr
 * bug in BuildApAssign() fixed:
 * functions with multiple return values are handled correctly now
 *
 * Revision 3.41  2002/11/04 17:39:27  sbs
 * call SHOldTypes2Shape changed into SHOldShpseg2Shape to avoid implicit
 * flattening of user defined types...
 *
 * Revision 3.40  2002/11/04 13:21:22  sbs
 * TYDeNestTypes added !
 *
 * Revision 3.39  2002/10/31 19:46:07  dkr
 * bug in TYCorrectWrapperArgTypes() fixed:
 * old type ARG_TYPE is *not* removed now!!!
 *
 * Revision 3.38  2002/10/30 16:31:30  dkr
 * TYCreateWrapperCode(): DBUG_ASSERT about T_dots added
 *
 * Revision 3.37  2002/10/30 16:10:31  dkr
 * TYStaticDispatchWrapper() added
 *
 * Revision 3.36  2002/10/30 13:23:59  sbs
 * handling of dot args introduced.
 *
 * Revision 3.35  2002/10/30 12:11:34  sbs
 * TYEliminateUser added.
 *
 * Revision 3.34  2002/10/28 10:41:08  sbs
 * wrong ASSERT in TYOldType2Type eliminated again 8-)
 *
 * Revision 3.33  2002/10/28 10:31:28  sbs
 * some DBUG_ASSERTS added in TYOldType2Type
 *
 * Revision 3.32  2002/10/18 15:34:29  sbs
 * Now Type2OldType converts T_classtype thingies into T_hidden ones
 * (for backend compatibility
 *
 * Revision 3.31  2002/10/18 14:35:16  sbs
 * several additions made. Primarily, they concern user defined types
 * hidden types objects and friends.
 *
 * Revision 3.30  2002/09/25 11:37:19  sbs
 * some minor warnings eliminated
 *
 * Revision 3.29  2002/09/17 16:17:44  dkr
 * TYCreateWrapperCode() modified
 *
 * Revision 3.28  2002/09/09 11:57:41  dkr
 * BuildApAssign() modified, BuildErrorAssign() added.
 * Some DBUG_ASSERTs added.
 *
 * Revision 3.27  2002/09/06 17:29:31  sbs
 * TC_poly added.
 *
 * Revision 3.26  2002/09/06 15:16:40  sbs
 * FUNDEF_RETURN now set properly?!
 *
 * Revision 3.25  2002/09/05 16:51:57  sbs
 * 2 bugs eliminated:
 * 1) TYLUB now computes correctly if scalars are involved
 *    (false sharing before....)
 * 2) Type2String now prints scal type instead of generic type GRGRGRGRG
 *
 * Revision 3.24  2002/09/05 11:56:48  dkr
 * fixed a bug in BuildCondAssign()
 *
 * Revision 3.23  2002/09/04 16:19:53  dkr
 * - IsTypeError() added
 * - BuildCondAssign(): optimization added
 *
 * Revision 3.22  2002/09/04 13:55:53  sbs
 * single LUT in Overload replaced by n LUTS....
 *
 * Revision 3.21  2002/09/04 12:59:46  sbs
 * some further StrBufprintf changed into StrBufprint.
 *
 * Revision 3.20  2002/09/04 12:20:37  dkr
 * some comments added...
 *
 * Revision 3.19  2002/09/04 12:16:38  dkr
 * data type DFT_state invented
 *
 * Revision 3.18  2002/09/03 18:53:58  dkr
 * - interface for dispatching functions added
 * - creating wrapper function code is complete now
 *
 * Revision 3.17  2002/09/03 13:17:00  sbs
 * partial derivations added and type2string mechanism
 * changed to StrBuf usage....
 *
 * Revision 3.16  2002/08/30 10:49:42  dkr
 * BuildApAssign modified
 *
 * Revision 3.15  2002/08/15 21:10:57  dkr
 * some errors corrected
 *
 * Revision 3.14  2002/08/13 15:59:30  dkr
 * signature of TYCreateWrapper...() functions modified
 *
 * Revision 3.13  2002/08/13 13:45:32  dkr
 * - SearchInLUT_PP used instead of SearchInLUT_P
 * - functions for creation of wrapper function code added
 *
 * Revision 3.12  2002/08/09 14:52:04  dkr
 * signature of TYType2WrapperCode modified
 *
 * Revision 3.11  2002/08/09 13:01:18  dkr
 * TYType2WrapperCode() added
 *
 * [eliminated] ....
 *
 * Revision 1.1  1999/10/20 12:52:00  sbs
 * Initial revision
 *
 */

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
 *     TYFreeTypeConstructor  for freeing the topmost constructor only, and
 *     TYFreeType             for freeing the entire type.
 *
 * - If the result is an ntype structure, it has been freshly allocated!
 *
 */

#include <stdarg.h>
#include <limits.h>

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "LookUpTable.h"

#include "DupTree.h"
#include "free.h"
#include "convert.h"

#include "user_types.h"
#include "shape.h"
#include "ssi.h"

#include "serialize.h"

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

typedef struct dft_state {
    int max_funs;
    int cnt_funs;
    node **fundefs;
    bool *legal;
    int *ups;
    int *downs;
} DFT_state;

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
    constant *a_akv;
    struct NTYPE *a_ibase;
    int a_idim;
    shape *a_ishape;
    attr_ires a_ires;
    tvar *a_alpha;
    char *a_poly;
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
#define AKV_CONST(n) (n->typeattr.a_akv)
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

#define POLY_NAME(n) (n->typeattr.a_poly)

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
 *    ntype *MakeNtype( typeconstr con, int arity)
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
    int i, arity;

    DBUG_ENTER ("MakeNewSon");

    arity = NTYPE_ARITY (father);
    NTYPE_ARITY (father) = arity + 1;
    new_sons = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (father));
    for (i = 0; i < arity; i++) {
        new_sons[i] = NTYPE_SON (father, i);
    }
    new_sons[i] = son;
    NTYPE_SONS (father) = Free (NTYPE_SONS (father));
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
DeleteSon (ntype *father, int son)
{
    ntype **new_sons;
    int i, arity;

    DBUG_ENTER ("DeleteSon");

    arity = NTYPE_ARITY (father) - 1;
    NTYPE_ARITY (father) = arity;
    new_sons = (ntype **)Malloc (sizeof (ntype *) * arity);
    for (i = 0; i < son; i++) {
        new_sons[i] = NTYPE_SON (father, i);
    }
    for (; i < arity; i++) {
        new_sons[i] = NTYPE_SON (father, i + 1);
    }
    NTYPE_SONS (father) = Free (NTYPE_SONS (father));
    NTYPE_SONS (father) = new_sons;

    DBUG_RETURN (father);
}

/******************************************************************************
 *
 * function:
 *   ntype *IncreaseArity( ntype *type, int amount)
 *
 * description:
 *   Internal function for increasing an ntype's arity by amount new sons.
 *   Like all Makexxx functions it consumes its argument!!
 *
 ******************************************************************************/

ntype *
IncreaseArity (ntype *type, int amount)
{
    ntype **new_sons;
    int i, arity;

    DBUG_ENTER ("IncreaseArity");

    arity = NTYPE_ARITY (type);
    NTYPE_ARITY (type) = arity + amount;
    new_sons = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (type));
    for (i = 0; i < arity; i++) {
        new_sons[i] = NTYPE_SON (type, i);
    }
    for (i = arity; i < NTYPE_ARITY (type); i++) {
        new_sons[i] = NULL;
    }
    NTYPE_SONS (type) = Free (NTYPE_SONS (type));
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
    IRES_FUNDEFS (ires) = Free (IRES_FUNDEFS (ires));
    IRES_POSS (ires) = Free (IRES_POSS (ires));
    fundefs = Free (fundefs);
    poss = Free (poss);
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
 *   ntype * TYMakeSimpleType( simpletype base)
 *   ntype * TYMakeSymbType( char *name, char *mod)
 *   ntype * TYMakeUserType( usertype udt)
 *
 *   ntype * TYSetSimpleType( ntype *simple, simpletype base)
 *
 * description:
 *   Several functions for creating scalar types
 *   (type-constructors with arity 0).
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

ntype *
TYSetSimpleType (ntype *simple, simpletype base)
{
    DBUG_ENTER ("TYSetSimpleType");

    SIMPLE_TYPE (simple) = base;

    DBUG_RETURN (simple);
}

/******************************************************************************
 *
 * function:
 *   simpletype TYGetSimpleType( ntype *simple)
 *   usertype   TYGetUserType( ntype *user)
 *   char     * TYGetName( ntype *symb)
 *   char     * TYGetMod( ntype *symb)
 *
 * description:
 *   Several functions for extracting the attributes from scalar types.
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

usertype
TYGetUserType (ntype *user)
{
    DBUG_ENTER ("TYGetUserType");
    DBUG_ASSERT ((NTYPE_CON (user) == TC_user),
                 "TYGetUserType applied to non-user-type!");
    DBUG_RETURN (USER_TYPE (user));
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
 *   ntype * TYMakeAKV( ntype *scalar, constant *val)
 *   ntype * TYMakeAKS( ntype *scalar, shape *shp)
 *   ntype * TYMakeAKD( ntype *scalar, int dots, shape *shp)
 *   ntype * TYMakeAUD( ntype *scalar)
 *   ntype * TYMakeAUDGZ( ntype *scalar)
 *
 *   ntype * TYSetScalar( ntype *array, ntype *scalar)
 *
 * description:
 *   Several functions for creating array-types.
 *
 ******************************************************************************/

ntype *
TYMakeAKV (ntype *scalar, constant *val)
{
    ntype *res;

    DBUG_ENTER ("TYMakeAKV");

    res = MakeNtype (TC_akv, 1);
    AKV_CONST (res) = val;
    AKV_BASE (res) = scalar;

    DBUG_RETURN (res);
}

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

    NTYPE_SON (array, 0) = TYFreeType (NTYPE_SON (array, 0));
    NTYPE_SON (array, 0) = scalar;

    DBUG_RETURN (array);
}

/******************************************************************************
 *
 * function:
 *   int        TYGetDim( ntype *array)
 *   shape *    TYGetShape( ntype *array)
 *   constant * TYGetValue( ntype *array)
 *   ntype *    TYGetScalar( ntype *array)
 *
 * description:
 *   Several functions for extracting the attributes / sons of array types.
 *
 ******************************************************************************/

int
TYGetDim (ntype *array)
{
    shape *shp;
    int res;

    DBUG_ENTER ("TYGetDim");
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akv)
                   || (NTYPE_CON (array) == TC_akd),
                 "TYGetDim applied to ther than AKV, AKS or AKD type!");
    if (NTYPE_CON (array) == TC_akv) {
        res = COGetDim (AKV_CONST (array));
    } else if (NTYPE_CON (array) == TC_aks) {
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
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akv)
                   || (NTYPE_CON (array) == TC_akd),
                 "TYGetShape applied to ther than AKV, AKS or AKD type!");
    if (NTYPE_CON (array) == TC_akv) {
        res = COGetShape (AKV_CONST (array));
    } else if (NTYPE_CON (array) == TC_aks) {
        res = AKS_SHP (array);
    } else {
        res = AKD_SHP (array);
    }

    DBUG_RETURN (res);
}

constant *
TYGetValue (ntype *array)
{
    constant *res;

    DBUG_ENTER ("TYGetValue");
    DBUG_ASSERT ((NTYPE_CON (array) == TC_akv),
                 "TYGetValue applied to ther than AKV type!");
    res = AKV_CONST (array);

    DBUG_RETURN (res);
}

ntype *
TYGetScalar (ntype *array)
{
    DBUG_ENTER ("TYGetScalar");
    DBUG_ASSERT ((NTYPE_CON (array) == TC_aks) || (NTYPE_CON (array) == TC_akv)
                   || (NTYPE_CON (array) == TC_akd) || (NTYPE_CON (array) == TC_audgz)
                   || (NTYPE_CON (array) == TC_aud),
                 "TYGetScalar applied to other than array type!");
    DBUG_RETURN (NTYPE_SON (array, 0));
}

/******************************************************************************
 *
 * function:
 *   ntype * TYMakeUnionType( ntype *t1, ntype *t2)
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
        t1 = TYFreeTypeConstructor (t1);
    } else {
        UNION_MEMBER (res, pos++) = t1;
    }
    if (NTYPE_CON (t2) == TC_union) {
        for (i = 0; i < NTYPE_ARITY (t2); i++) {
            UNION_MEMBER (res, pos++) = UNION_MEMBER (t2, i);
        }
        t2 = TYFreeTypeConstructor (t2);
    } else {
        UNION_MEMBER (res, pos++) = t2;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   ntype * TYMakeProductType( int size, ...)
 *
 *   ntype * TYMakeEmptyProductType( int size)
 *   ntype * TYSetProductMember( ntype *prod, int pos, ntype *member)
 *
 * description:
 *   Functions for creating product types. Note here, that this function, like
 *   all MakeXYZ and GetXYZ functions consumes its arguments!!
 *   At the time being, only array types or type variables may be given as
 *   arguments.
 *   The first version is useful in all situations where the number of
 *   components is statically known. However, in many situations this is not
 *   the case. In these situations, the latter two functions are to be used.
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
                         "non array type / type var components of product types"
                         " are not yet supported!");
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
                 "TYSetProductMember applied to product type with"
                 " too few elements");

    NTYPE_SON (prod, pos) = member;

    DBUG_RETURN (prod);
}

/******************************************************************************
 *
 * function:
 *   int       TYGetProductSize( ntype *prod)
 *   ntype  *  TYGetProductMember( ntype *prod, int pos)
 *
 * description:
 *   functions inspecting or extracting components of product types!
 *   Note here, that TYGetProductMember does not copy the member to be
 *   extracted!!
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
 *   ntype * TYMakePolyType( char *name)
 *
 * description:
 *   Function for creating poly types.
 *
 ******************************************************************************/

ntype *
TYMakePolyType (char *name)
{
    ntype *res;

    DBUG_ENTER ("TYMakePolyType");

    res = MakeNtype (TC_poly, 0);
    POLY_NAME (res) = name;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   char * TYGetPolyName( ntype *type)
 *
 * description:
 *  function for extracting the name of a polymorphic type.
 *
 ******************************************************************************/

char *
TYGetPolyName (ntype *type)
{
    DBUG_ENTER ("TYGetPolyName");

    DBUG_ASSERT ((NTYPE_CON (type) == TC_poly),
                 "TYGetPolyName applied to non poly type!");

    DBUG_RETURN (POLY_NAME (type));
}

/******************************************************************************
 *
 * function:
 *   ntype * TYMakeFunType( ntype *arg, ntype *res, node *fun_info)
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
 *                                            /[3,4]
 *
 *  All "open ends" of this structure point to TC_ires nodes which hold the
 *  return types of the given function. The dots right of some edges indicate
 *  that there may be multiple of the lower nodes attached, i.e., ther may be
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
TYMakeFunType (ntype *arg, ntype *res_type, node *fundef)
{
    ntype *fun = NULL;
    ntype *base = NULL;
    ntype *arr = NULL;
    ntype *dim = NULL;
    ntype *shape = NULL;
    ntype *res = NULL;
    ntype *aks = NULL;

#ifndef DBUG_OFF
    char *tmp;
#endif

    DBUG_ENTER ("TYMakeFunType");
    DBUG_PRINT ("NTY_MEM",
                ("Allocated mem on entering TYMakeFunType: %u", current_allocated_mem));

    res = MakeNtype (TC_ires, 1);

    IRES_TYPE (res) = res_type;

    IRES_NUMFUNS (res) = 1;
    IRES_FUNDEFS (res) = (node **)Malloc (sizeof (node *));
    IRES_FUNDEF (res, 0) = fundef;
    IRES_POSS (res) = (int *)Malloc (sizeof (int));
    IRES_POS (res, 0) = 0;

    base = MakeNtype (TC_ibase, 3);

    switch (NTYPE_CON (arg)) {
    case TC_akv:
        aks = TYEliminateAKV (arg);
        arg = TYFreeType (arg);
        arg = aks;
        aks = NULL;
        /**
         * there is no break here, as we wish to re-use the AKS implementation.
         * At a later stage, this has to be replaced by proper code suporting
         * value specialized function instances.
         */

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
    arg = TYFreeTypeConstructor (arg);

    DBUG_EXECUTE ("NTY", tmp = TYType2DebugString (fun, TRUE, 0););
    DBUG_PRINT ("NTY", ("fun type built: %s\n", tmp));
    DBUG_EXECUTE ("NTY", tmp = Free (tmp););

    DBUG_PRINT ("NTY_MEM",
                ("Allocated mem on leaving  TYMakeFunType: %u", current_allocated_mem));

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *   ntype * TYMakeOverloadedFunType( ntype *fun1, ntype *fun2)
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
            IBASE_GEN (fun) = FilterFundefs (IBASE_GEN (fun), num_kills, kill_list);
            if (IBASE_GEN (fun) == NULL) {
                fun = TYFreeType (fun);
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
                fun = TYFreeType (fun);
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
                fun = TYFreeType (fun);
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
ProjDown (ntype *ires, ntype *template)
{
    int i;
    int new_numfuns = 0;
    int num_kills = 0;
    ntype *res = NULL;
    ntype *tmp = NULL;
    node **kill_list;

    DBUG_ENTER ("ProjDown");

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

    DBUG_RETURN (res);
}

typedef bool (*cmp_ntype_fun_t) (ntype *, ntype *);

static bool
CmpIbase (ntype *ibase1, ntype *ibase2)
{
    DBUG_ASSERT (((NTYPE_CON (ibase1) == TC_ibase) && (NTYPE_CON (ibase1) == TC_ibase)),
                 "CmpIbase called with non TC_ibase arg!");

    return (TYEqTypes (IBASE_BASE (ibase1), IBASE_BASE (ibase2)));
}

static bool
CmpIdim (ntype *idim1, ntype *idim2)
{
    DBUG_ASSERT (((NTYPE_CON (idim1) == TC_idim) && (NTYPE_CON (idim1) == TC_idim)),
                 "CmpIdim called with non TC_idim arg!");

    return (IDIM_DIM (idim1) == IDIM_DIM (idim2));
}

static bool
CmpIshape (ntype *ishape1, ntype *ishape2)
{
    DBUG_ASSERT (((NTYPE_CON (ishape1) == TC_ishape)
                  && (NTYPE_CON (ishape1) == TC_ishape)),
                 "CmpIshape called with non TC_ishape arg!");

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

    DBUG_ENTER ("MergeSons");

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

    DBUG_ENTER ("AdjustSons");

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

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * NOW, the implementation of   TYMakeOverloadedFunType    itself!!
 *
 ******************************************************************************/

#ifndef DBUG_OFF
static tvar **overload_fun1_alphas;
#endif
static int overload_num_luts = 0;
static int overload_pos = 0;
static LUT_t *overload_luts;

ntype *
TYMakeOverloadedFunType (ntype *fun1, ntype *fun2)
{
    ntype *res;
    int i;
#ifndef DBUG_OFF
    char *tmp, *tmp2;
#endif

    DBUG_ENTER ("TYMakeOverloadedFunType");

    DBUG_EXECUTE ("NTY", tmp = TYType2DebugString (fun1, TRUE, 0);
                  tmp2 = TYType2DebugString (fun2, TRUE, 0););
    DBUG_PRINT ("NTY", ("functions:        %s", tmp));
    DBUG_PRINT ("NTY", ("and               %s", tmp2));
    DBUG_EXECUTE ("NTY", tmp = Free (tmp); tmp2 = Free (tmp2););

    /*
     * iff this is the very first call, instantiate rel. free vars 8-))
     *
     * we need max num rets many LUTs here.
     * Since we do not statically now, we start with 5 LUTs. If it turns out during
     * overloading that these are not enough, we simply allocate further ones...
     */
    if (overload_num_luts == 0) {
        overload_num_luts = 5;
#ifndef DBUG_OFF
        overload_fun1_alphas = (tvar **)Malloc (overload_num_luts * sizeof (tvar *));
        for (i = 0; i < overload_num_luts; i++) {
            overload_fun1_alphas[i] = NULL;
        }
#endif
        overload_luts = (LUT_t *)Malloc (overload_num_luts * sizeof (LUT_t));
        for (i = 0; i < overload_num_luts; i++) {
            overload_luts[i] = GenerateLUT ();
        }
    }

    if ((fun1 != NULL) && (NTYPE_CON (fun1) != TC_fun) && (fun2 != NULL)
        && (NTYPE_CON (fun2) != TC_fun)) {
        ABORT (linenum, ("cannot overload functions of arity 0"));
    }

    res = MakeOverloadedFunType (fun1, fun2);

    /*
     * reset the rel free vars for next usage!
     */
    for (i = 0; i < overload_num_luts; i++) {
#ifndef DBUG_OFF
        overload_fun1_alphas[i] = NULL;
#endif
        overload_luts[i] = RemoveContentLUT (overload_luts[i]);
    }

    DBUG_EXECUTE ("NTY", tmp = TYType2DebugString (res, TRUE, 0););
    DBUG_PRINT ("NTY", ("overloaded into : %s", tmp));
    DBUG_EXECUTE ("NTY", tmp = Free (tmp););

    DBUG_RETURN (res);
}

static ntype *
MakeOverloadedFunType (ntype *fun1, ntype *fun2)
{
    ntype *lub, *res;
    tvar *old_alpha;
    bool ok;
    int i;
    int new_num_luts;
#ifndef DBUG_OFF
    tvar **new_alphas;
    char *tmpstring;
#endif
    LUT_t *new_luts;

    DBUG_ENTER ("MakeOverloadedFunType");

    if (fun1 == NULL) {
        res = fun2;
    } else if (fun2 == NULL) {
        res = fun1;
    } else {
        DBUG_EXECUTE ("NTY", tmpstring = TYType2DebugString (fun1, TRUE, 0););
        DBUG_PRINT ("NTY", ("fun1: %s", tmpstring));

        DBUG_EXECUTE ("NTY", tmpstring = TYType2DebugString (fun2, TRUE, 0););
        DBUG_PRINT ("NTY", ("fun2: %s", tmpstring));

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
                         "trying to overload function types with different number"
                         " of return types");
            if (NTYPE_ARITY (fun1) > overload_num_luts) {
                new_num_luts = overload_num_luts + NTYPE_ARITY (fun1);
#ifndef DBUG_OFF
                new_alphas = (tvar **)Malloc (new_num_luts * sizeof (tvar *));
                for (i = 0; i < overload_num_luts; i++) {
                    new_alphas[i] = overload_fun1_alphas[i];
                }
                for (; i < new_num_luts; i++) {
                    new_alphas[i] = NULL;
                }
                overload_fun1_alphas = Free (overload_fun1_alphas);
                overload_fun1_alphas = new_alphas;
#endif
                new_luts = (LUT_t *)Malloc (new_num_luts * sizeof (LUT_t));
                for (i = 0; i < overload_num_luts; i++) {
                    new_luts[i] = overload_luts[i];
                }
                for (; i < new_num_luts; i++) {
                    new_luts[i] = GenerateLUT ();
                }
                overload_luts = Free (overload_luts);
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
                DBUG_ASSERT ((overload_fun1_alphas[overload_pos] == ALPHA_SSI (fun1)),
                             "TYMakeOverloadedFunType called with overloaded fun1!");
            }
#endif
            if (SSIIsLe (ALPHA_SSI (fun1), ALPHA_SSI (fun2))) {
                res = fun2;
            } else if (SSIIsLe (ALPHA_SSI (fun2), ALPHA_SSI (fun1))) {
                res = TYCopyType (fun1);
                fun2 = TYFreeTypeConstructor (fun2);
            } else {
                old_alpha
                  = SearchInLUT_PP (overload_luts[overload_pos], ALPHA_SSI (fun2));
                if (old_alpha != ALPHA_SSI (fun2)) { /* found! */
                    res = MakeNtype (TC_alpha, 0);
                    ALPHA_SSI (res) = old_alpha;
                } else {
                    lub = TYLubOfTypes (SSIGetMax (ALPHA_SSI (fun1)),
                                        SSIGetMax (ALPHA_SSI (fun2)));
                    if (lub == NULL) {
                        ABORT (linenum,
                               ("cannot overload functions with disjoint result type;"
                                " types found: \"%s\" and \"%s\"",
                                TYType2String (SSIGetMax (ALPHA_SSI (fun1)), FALSE, 0),
                                TYType2String (SSIGetMax (ALPHA_SSI (fun2)), FALSE, 0)));
                    } else {
                        res = TYMakeAlphaType (lub);
                        ok = SSINewRel (ALPHA_SSI (fun1), ALPHA_SSI (res));
                        DBUG_ASSERT (ok,
                                     "SSINewRel did not work in TYMakeOverloadFunType");
                        ok = SSINewRel (ALPHA_SSI (fun2), ALPHA_SSI (res));
                        DBUG_ASSERT (ok,
                                     "SSINewRel did not work in TYMakeOverloadFunType");
                        overload_luts[overload_pos]
                          = InsertIntoLUT_P (overload_luts[overload_pos],
                                             ALPHA_SSI (fun2), ALPHA_SSI (res));
                    }
                }
            }
            overload_pos++;
            break;
        default:
            DBUG_ASSERT ((0), "TYMakeOverloadFunType called with illegal funtype!");
        }
        fun1 = TYFreeTypeConstructor (fun1);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   DFT_res * TYMakeDFT_res( ntype *type, int max_funs)
 *
 * Description:
 *
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
 * Function:
 *   DFT_res * TYFreeDFT_res( DFT_res *res)
 *
 * Description:
 *
 *
 ******************************************************************************/

DFT_res *
TYFreeDFT_res (DFT_res *res)
{
    DBUG_ENTER ("TYFreeDFT_res");

    DBUG_ASSERT ((res != NULL), "argument is NULL");

    if (res->partials != NULL) {
        res->partials = Free (res->partials);
    }
    if (res->deriveable_partials != NULL) {
        res->deriveable_partials = Free (res->partials);
    }

    res = Free (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   DFT_state *AllocDFT_state( int max_funs)
 *
 * Description:
 *
 *
 ******************************************************************************/

static DFT_state *
AllocDFT_state (int max_funs)
{
    DFT_state *state;

    DBUG_ENTER ("AllocDFT_state");

    state = (DFT_state *)Malloc (sizeof (DFT_state));

    state->max_funs = max_funs;
    state->cnt_funs = 0;
    state->fundefs = (node **)Malloc (max_funs * sizeof (node *));
    state->legal = (bool *)Malloc (max_funs * sizeof (bool));
    state->ups = (int *)Malloc (max_funs * sizeof (int));
    state->downs = (int *)Malloc (max_funs * sizeof (int));

    DBUG_RETURN (state);
}

/******************************************************************************
 *
 * Function:
 *   DFT_state *FreeDFT_state( DFT_state *state)
 *
 * Description:
 *
 *
 ******************************************************************************/

static DFT_state *
FreeDFT_state (DFT_state *state)
{
    DBUG_ENTER ("FreeDFT_state");

    state->fundefs = Free (state->fundefs);
    state->legal = Free (state->legal);
    state->ups = Free (state->ups);
    state->downs = Free (state->downs);

    state = Free (state);

    DBUG_RETURN (state);
}

/******************************************************************************
 *
 * Function:
 *   DFT_state *CopyDFT_state( DFT_state *state)
 *
 * Description:
 *
 *
 ******************************************************************************/

static DFT_state *
CopyDFT_state (DFT_state *state)
{
    DFT_state *new_state;
    int i;

    DBUG_ENTER ("CopyDFT_state");

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
 *   DFT_state *InsertFirstArgDFT_state( DFT_state *state,
 *                                       ntype *ires, int lower)
 *
 * Description:
 *
 *
 ******************************************************************************/

static DFT_state *
InsertFirstArgDFT_state (DFT_state *state, ntype *ires, int lower)
{
    int cnt, i;

    DBUG_ENTER ("InsertFirstArgDFT_state");

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
 *   DFT_state *InsertNextArgDFT_state( DFT_state *state,
 *                                      ntype *ires, int lower)
 *
 * Description:
 *
 *
 ******************************************************************************/

static DFT_state *
InsertNextArgDFT_state (DFT_state *state, ntype *ires, int lower)
{
    int cnt, i, j;

    DBUG_ENTER ("InsertNextArgDFT_state");

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
 *   DFT_state *FinalizeDFT_state( DFT_state *state)
 *
 * Description:
 *
 *
 ******************************************************************************/

static DFT_state *
FinalizeDFT_state (DFT_state *state)
{
    int i;

    DBUG_ENTER ("FinalizeDFT_state");

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
 *   DFT_res *DFT_state2DFT_res( DFT_state *state)
 *
 * Description:
 *
 *
 ******************************************************************************/

static bool
AllArgTypesLe (node *fundef, node *fundef2)
{
    node *arg, *arg2;

    DBUG_ENTER ("AllArgTypesLe");

    arg = FUNDEF_ARGS (fundef);
    arg2 = FUNDEF_ARGS (fundef2);
    while ((arg != NULL)
           && TYLeTypes (AVIS_TYPE (ARG_AVIS (arg)), AVIS_TYPE (ARG_AVIS (arg2)))) {
        arg = ARG_NEXT (arg);
        arg2 = ARG_NEXT (arg2);
    }

    DBUG_RETURN ((arg == NULL));
}

static void
EliminateDeriveablePartial (node **dp_list, int *dp2ud, int length, int pos)
{
    int i;

    DBUG_ENTER ("EliminateDeriveablePartial");

    for (i = pos; i < length - 1; i++) {
        dp_list[i] = dp_list[i + 1];
        dp2ud[i] = dp2ud[i + 1];
    }

    DBUG_VOID_RETURN;
}

static DFT_res *
DFT_state2DFT_res (DFT_state *state)
{
    DFT_res *res;
    int max_deriveable;
    bool exact_found = FALSE;
    int i, j;
    int dom, irr;
    int *dp2ud;
    int *p2ud;

    DBUG_ENTER ("DFT_state2DFT_res");

    res = TYMakeDFT_res (NULL, state->cnt_funs);
    dp2ud = (int *)Malloc (state->cnt_funs * sizeof (int));
    p2ud = (int *)Malloc (state->cnt_funs * sizeof (int));

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
    for (i = 0; i < state->max_funs; i++) {
        if (state->fundefs[i] != NULL) {
            if (state->ups[i] == 0) {
                if (state->downs[i] == 0) {
                    res->def = state->fundefs[i];
                    /* no down projections in case of an exact definition! */
                    max_deriveable = 0;
                    res->deriveable = NULL;
                    /* no deriveable partials in case of an exact definition! */
                    exact_found = TRUE;
                    res->num_deriveable_partials = 0;
                } else {
                    if (state->downs[i] > max_deriveable) {
                        res->deriveable = state->fundefs[i];
                        max_deriveable = state->downs[i];
                    }
                }
            } else {
                if (state->downs[i] == 0) {
                    res->partials[res->num_partials] = state->fundefs[i];
                    p2ud[res->num_partials] = i;
                    res->num_partials++;
                } else {
                    if (!exact_found) {
                        res->deriveable_partials[res->num_deriveable_partials]
                          = state->fundefs[i];
                        dp2ud[res->num_deriveable_partials] = i;
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

    DBUG_PRINT ("NTDIS", ("filtering derived partials:"));

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
                DBUG_PRINT ("NTDIS",
                            ("  %p might shadow %p here", state->fundefs[dp2ud[dom]],
                             state->fundefs[dp2ud[irr]]));
                if (AllArgTypesLe (state->fundefs[dp2ud[dom]],
                                   state->fundefs[dp2ud[irr]])) {
                    DBUG_PRINT ("NTDIS",
                                ("  eliminating %p", state->fundefs[dp2ud[irr]]));
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
                DBUG_PRINT ("NTDIS",
                            ("  %p might shadow %p here", state->fundefs[p2ud[dom]],
                             state->fundefs[dp2ud[irr]]));
                if (AllArgTypesLe (state->fundefs[p2ud[dom]],
                                   state->fundefs[dp2ud[irr]])) {
                    DBUG_PRINT ("NTDIS",
                                ("  eliminating %p", state->fundefs[dp2ud[irr]]));
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
 *   ntype * TYDispatchFunType( ntype *fun, ntype *args)
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
        /*   new default:   <base>[*]   */
        res = IBASE_GEN (fun);

        if (((NTYPE_CON (arg) == TC_akv) || (NTYPE_CON (arg) == TC_aks)
             || (NTYPE_CON (arg) == TC_akd))
            && (TYGetDim (arg) == 0)) {
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
                        fun = FindIdim (fun, TYGetDim (arg));
                        if (fun == NULL) {
                            lower = ((NTYPE_CON (arg) == TC_akv)
                                       ? 3
                                       : (NTYPE_CON (arg) == TC_aks ? 2 : 1));
                        } else {

                            /*   new default:   <base>[...]   */
                            res = IDIM_GEN (fun);

                            if (NTYPE_CON (arg) != TC_akd) {
                                fun = FindIshape (fun, TYGetShape (arg));
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
    int i;

    DBUG_ENTER ("DebugPrintDispatchInfo");

    for (i = 0; i < IRES_NUMFUNS (ires); i++) {
        DBUG_PRINT (dbug_str,
                    ("  fundef %8p: %d", IRES_FUNDEF (ires, i), IRES_POS (ires, i)));
    }

    DBUG_VOID_RETURN;
}

static void
DebugPrintDFT_state (DFT_state *state)
{
    int i;

    DBUG_ENTER ("DebugPrintDFT_state");

    for (i = 0; i < state->max_funs; i++) {
        DBUG_PRINT ("NTDIS", ("  fundef %8p: ups %2d | downs %2d", state->fundefs[i],
                              state->ups[i], state->downs[i]));
    }

    DBUG_VOID_RETURN;
}

#endif /* DBUG_OFF */

DFT_res *
TYDispatchFunType (ntype *fun, ntype *args)
{
    int lower;
    int i, n;
    ntype *arg, *ires;
    node *fundef;
    DFT_res *res;
    DFT_state *state = NULL;
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
                       ("no definition found for a function \"%s\" that"
                        " accepts an argument of type \"%s\" as parameter"
                        " no %d",
                        FUNDEF_NAME (fundef), TYType2String (arg, FALSE, 0), i + 1));
            }

            DBUG_EXECUTE ("NTDIS", tmp_str = TYType2String (arg, FALSE, 0););
            DBUG_PRINT ("NTDIS",
                        ("arg #%d: %s yields (lifted by %d):", i, tmp_str, lower));
            DBUG_EXECUTE ("NTDIS", tmp_str = Free (tmp_str););
            DBUG_EXECUTE ("NTDIS", DebugPrintDispatchInfo ("NTDIS", ires););

            /*
             * Now, we accumulate the ups and downs:
             */
            if (i == 0) {
                state = AllocDFT_state (IRES_NUMFUNS (ires));
                state = InsertFirstArgDFT_state (state, ires, lower);
            } else {
                state = InsertNextArgDFT_state (state, ires, lower);
            }

            DBUG_PRINT ("NTDIS", ("accumulated ups and downs:"));
            DBUG_EXECUTE ("NTDIS", DebugPrintDFT_state (state););

            fun = IRES_TYPE (ires);
            if (NTYPE_CON (fun) != TC_fun) {
                i = n;
            }
        }

        state = FinalizeDFT_state (state);

        DBUG_PRINT ("NTDIS", ("final ups and downs:"));
        DBUG_EXECUTE ("NTDIS", DebugPrintDFT_state (state););

        /*
         * Finally, we export our findings via a DFT_res structure.
         * However, in case of 0 args (n==0), no dispatch has to be made
         * (since no overloading is allowed) so we return NULL!!
         */

        res = DFT_state2DFT_res (state);

        res->type = fun; /* insert the result type */

        state = FreeDFT_state (state);
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
    static str_buf *buf = NULL;
    int i;
    char *tmp_str;

    DBUG_ENTER ("TYDFT_res2DebugString");

    if (buf == NULL) {
        buf = StrBufCreate (100);
    }
    if (dft == NULL) {
        buf = StrBufprintf (buf, "--");
    } else {
        if (dft->def) {
            tmp_str = OldTypeSignature2String (dft->def);
            buf = StrBufprintf (buf, "exact : (%s) ", tmp_str);
            tmp_str = Free (tmp_str);
        }
        if (dft->deriveable) {
            tmp_str = OldTypeSignature2String (dft->deriveable);
            buf = StrBufprintf (buf, "deriveable : (%s) ", tmp_str);
            tmp_str = Free (tmp_str);
        }
        if (dft->num_partials > 0) {
            buf = StrBufprintf (buf, "partials : ");
            for (i = 0; i < dft->num_partials; i++) {
                tmp_str = OldTypeSignature2String (dft->partials[i]);
                buf = StrBufprintf (buf, "%s ", tmp_str);
                tmp_str = Free (tmp_str);
            }
        }
        if (dft->num_deriveable_partials > 0) {
            buf = StrBufprintf (buf, "deriveable_partials : ");
            for (i = 0; i < dft->num_deriveable_partials; i++) {
                tmp_str = OldTypeSignature2String (dft->deriveable_partials[i]);
                buf = StrBufprintf (buf, "%s ", tmp_str);
                tmp_str = Free (tmp_str);
            }
        }

        if (StrBufIsEmpty (buf)) {
            buf = StrBufprintf (buf, "no match!");
        }
    }

    tmp_str = StrBuf2String (buf);
    StrBufFlush (buf);

    DBUG_RETURN (tmp_str);
}

/******************************************************************************
 *
 * function:
 *   ntype * TYMakeAlphaType( ntype *maxtype)
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
 *   tvar * TYGetAlpha( ntype *type)
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
                 "TYGetAlpha applied to non type variable!");

    DBUG_RETURN (ALPHA_SSI (type));
}

/***
 *** Functions inspecting types / matching on specific types:
 ***/

/******************************************************************************
 *
 * function:
 *    bool TYIsSimple( ntype *type)
 *    bool TYIsUser( ntype *type)
 *    bool TYIsSymb( ntype *type)
 *    bool TYIsScalar( ntype *type)
 *    bool TYIsAlpha( ntype *type)
 *    bool TYIsFixedAlpha( ntype *type)
 *    bool TYIsNonFixedAlpha( ntype *type)
 *    bool TYIsAKV( ntype *type)
 *    bool TYIsAKS( ntype *type)
 *    bool TYIsAKD( ntype *type)
 *    bool TYIsAUDGZ( ntype *type)
 *    bool TYIsAUD( ntype *type)
 *    bool TYIsArray( ntype *type)
 *    bool TYIsArrayOrAlpha( ntype *type)
 *    bool TYIsArrayOrFixedAlpha( ntype *type)
 *    bool TYIsUnion( ntype *type)
 *    bool TYIsProd( ntype *type)
 *    bool TYIsFun( ntype *type)
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
TYIsAKV (ntype *type)
{
    DBUG_ENTER ("TYIsAKV");
    DBUG_RETURN (NTYPE_CON (type) == TC_akv);
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
                 || (NTYPE_CON (type) == TC_akd) || (NTYPE_CON (type) == TC_aks)
                 || (NTYPE_CON (type) == TC_akv));
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
 *    bool TYIsAKSSymb( ntype *type)
 *    bool TYIsProdOfArrayOrFixedAlpha( ntype *type)
 *    bool TYIsProdOfAKV( ntype *type)
 *    bool TYIsProdContainingAKV( ntype *type)
 *    bool TYIsProdOfArray( ntype *type)
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
    DBUG_RETURN (
      ((NTYPE_CON (type) == TC_aks) && (NTYPE_CON (AKS_BASE (type)) == TC_symbol)));
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

bool
TYIsProdOfAKV (ntype *args)
{
    bool res = TRUE;
    ntype *arg;
    int i;

    DBUG_ENTER ("TYIsProdContainingAKV");

    if (TYIsProd (args)) {
        for (i = 0; i < TYGetProductSize (args); i++) {
            arg = TYGetProductMember (args, i);
            res = res && TYIsAKV (arg);
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

bool
TYIsProdContainingAKV (ntype *args)
{
    bool res = FALSE;
    ntype *arg;
    int i;

    DBUG_ENTER ("TYIsProdContainingAKV");

    if (TYIsProd (args)) {
        for (i = 0; i < TYGetProductSize (args); i++) {
            arg = TYGetProductMember (args, i);
            res = res || TYIsAKV (arg);
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
 *    CT_res TYCmpTypes( ntype *t1, ntype *t2)
 *    bool   TYLeTypes( ntype *t1, ntype *t2)
 *    bool   TYEqTypes( ntype *t1, ntype *t2)
 *
 * description:
 *
 *
 ******************************************************************************/

CT_res
TYCmpTypes (ntype *t1, ntype *t2)
{
#ifndef DBUG_OFF
    char *tmp_str, *tmp_str2;
#endif

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
    case TC_akv:
        switch (NTYPE_CON (t2)) {
        case TC_akv:
            if (TYCmpTypes (AKV_BASE (t1), AKV_BASE (t2)) == TY_eq) {
                if (COCompareConstants (AKV_CONST (t1), AKV_CONST (t2))) {
                    res = TY_eq;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aks:
            if (TYCmpTypes (AKV_BASE (t1), AKS_BASE (t2)) == TY_eq) {
                if (SHCompareShapes (COGetShape (AKV_CONST (t1)), AKS_SHP (t2))) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_akd:
            if (TYCmpTypes (AKV_BASE (t1), AKD_BASE (t2)) == TY_eq) {
                if (TYGetDim (t1) == TYGetDim (t2)) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_audgz:
            if (TYCmpTypes (AKV_BASE (t1), AUDGZ_BASE (t2)) == TY_eq) {
                if (TYGetDim (t1) > 0) {
                    res = TY_lt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
        case TC_aud:
            if (TYCmpTypes (AKV_BASE (t1), AUD_BASE (t2)) == TY_eq) {
                res = TY_lt;
            }
            break;
        default:
            break;
        }
        break;
    case TC_aks:
        switch (NTYPE_CON (t2)) {
        case TC_akv:
            if (TYCmpTypes (AKS_BASE (t1), AKV_BASE (t2)) == TY_eq) {
                if (SHCompareShapes (AKS_SHP (t1), COGetShape (AKV_CONST (t2)))) {
                    res = TY_gt;
                } else {
                    res = TY_hcs;
                }
            }
            break;
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
        case TC_akv:
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
        case TC_akv:
        case TC_aks:
        case TC_akd:
            if (TYCmpTypes (AUDGZ_BASE (t1), TYGetScalar (t2)) == TY_eq) {
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
        case TC_akv:
        case TC_aks:
        case TC_akd:
        case TC_audgz:
            if (TYCmpTypes (AUD_BASE (t1), TYGetScalar (t2)) == TY_eq) {
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
        DBUG_EXECUTE ("NTY", tmp_str = TYType2String (t1, FALSE, 0);
                      tmp_str2 = TYType2String (t2, FALSE, 0););
        DBUG_PRINT ("NTY", ("trying to compare %s and %s", tmp_str, tmp_str2));

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
 *    ntype * TYLubOfTypes( ntype *t1, ntype *t2)
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
        case TC_akv:
            new_t1 = TYMakeAKS (TYCopyType (AKV_BASE (t1)),
                                SHCopyShape (COGetShape (AKV_CONST (t1))));
            res = TYLubOfTypes (new_t1, t2);
            new_t1 = TYFreeType (new_t1);
            break;
        case TC_aks:
            if (SHGetDim (AKS_SHP (t1)) == 0) {
                new_t1 = TYMakeAUD (TYCopyType (AKS_BASE (t1)));
            } else {
                new_t1 = TYMakeAKD (TYCopyType (AKS_BASE (t1)), SHGetDim (AKS_SHP (t1)),
                                    SHCreateShape (0));
            }
            res = TYLubOfTypes (new_t1, t2);
            new_t1 = TYFreeType (new_t1);
            break;
        case TC_akd:
            new_t1 = TYMakeAUDGZ (AKD_BASE (t1));
            res = TYLubOfTypes (new_t1, t2);
            new_t1 = TYFreeTypeConstructor (new_t1);
            break;
        case TC_audgz:
            new_t1 = TYMakeAUD (AUDGZ_BASE (t1));
            res = TYLubOfTypes (new_t1, t2);
            new_t1 = TYFreeTypeConstructor (new_t1);
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
 *    ntype * TYEliminateAlpha( ntype *t1)
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
 *    ntype * TYFixAndEliminateAlpha( ntype *t1)
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

    DBUG_ENTER ("TYFixAndEliminateAlpha");

    if (t1 == NULL) {
        res = t1;
    } else if (TYIsAlpha (t1)) {
        if (SSIGetMin (TYGetAlpha (t1)) != NULL) {
            res = TYCopyType (SSIGetMin (ALPHA_SSI (t1)));
        } else {
            res = TYCopyType (t1);
        }
    } else {
        int cnt;

        res = TYCopyTypeConstructor (t1);

        res = IncreaseArity (res, NTYPE_ARITY (t1));

        for (cnt = 0; cnt < NTYPE_ARITY (t1); cnt++) {
            NTYPE_SON (res, cnt) = TYFixAndEliminateAlpha (NTYPE_SON (t1, cnt));
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYEliminateUser( ntype *t1)
 *
 * description:
 *    if t1 is a user defined type, its base type is returned;
 *    otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYEliminateUser (ntype *t1)
{
    ntype *res;
    int i;

    DBUG_ENTER ("TYEliminateUser");

    if (TYIsProd (t1)) {
        res = MakeNtype (TC_prod, NTYPE_ARITY (t1));
        for (i = 0; i < NTYPE_ARITY (t1); i++) {
            PROD_MEMBER (res, i) = TYEliminateUser (PROD_MEMBER (t1, i));
        }
    } else {
        if (TYIsArray (t1) && TYIsUser (TYGetScalar (t1))) {
            res = TYNestTypes (t1, UTGetBaseType (USER_TYPE (TYGetScalar (t1))));
        } else {
            res = TYCopyType (t1);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype * TYEliminateAKV( ntype *t1)
 *
 *   @brief if t1 is a AKV type, the respective AKS type is returned;
 *          otherwise, a copy of t1 is returned.
 *
 ******************************************************************************/

ntype *
TYEliminateAKV (ntype *t1)
{
    ntype *res;
    int i;

    DBUG_ENTER ("TYEliminateAKV");

    if (TYIsProd (t1)) {
        res = MakeNtype (TC_prod, NTYPE_ARITY (t1));
        for (i = 0; i < NTYPE_ARITY (t1); i++) {
            PROD_MEMBER (res, i) = TYEliminateAKV (PROD_MEMBER (t1, i));
        }
    } else {
        if (TYIsAKV (t1)) {
            res = TYMakeAKS (TYCopyType (TYGetScalar (t1)),
                             SHCopyShape (COGetShape (AKV_CONST (t1))));
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
 *    ntype * TYFreeTypeConstructor( ntype *type)
 *    ntype * TYFreeType( ntype *type)
 *
 * description:
 *   functions for freeing types. While TYFreeTypeConstructor only frees the
 *   topmost type constructor, TYFreeType frees the entire type including all
 *   sons!
 *
 ******************************************************************************/

ntype *
TYFreeTypeConstructor (ntype *type)
{
    DBUG_ENTER ("TYFreeTypeConstructor");

    DBUG_ASSERT ((type != NULL), "argument is NULL");

    switch (NTYPE_CON (type)) {
    case TC_symbol:
        SYMBOL_MOD (type) = Free (SYMBOL_MOD (type));
        SYMBOL_NAME (type) = Free (SYMBOL_NAME (type));
        break;
    case TC_akv:
        AKV_CONST (type) = COFreeConstant (AKV_CONST (type));
        break;
    case TC_aks:
        AKS_SHP (type) = SHFreeShape (AKS_SHP (type));
        break;
    case TC_akd:
        AKD_SHP (type) = SHFreeShape (AKD_SHP (type));
        break;
    case TC_ibase:
        IBASE_BASE (type) = TYFreeType (IBASE_BASE (type));
        break;
    case TC_ishape:
        ISHAPE_SHAPE (type) = SHFreeShape (ISHAPE_SHAPE (type));
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
        DBUG_ASSERT ((0), "trying to free illegal type constructor!");
    }
    type = Free (type);

    DBUG_RETURN (type);
}

ntype *
TYFreeType (ntype *type)
{
    int i;

    DBUG_ENTER ("TYFreeType");

    DBUG_ASSERT ((type != NULL), "argument is NULL");

    for (i = 0; i < NTYPE_ARITY (type); i++) {
        if (NTYPE_SON (type, i) != NULL) {
            NTYPE_SON (type, i) = TYFreeType (NTYPE_SON (type, i));
        }
    }
    type = TYFreeTypeConstructor (type);

    DBUG_RETURN (type);
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
    int i;
    bool ok;

    DBUG_ENTER ("CopyTypeConstructor");

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
        case TC_poly:
            POLY_NAME (res) = StringCopy (POLY_NAME (type));
            break;
        case TC_user:
            USER_TYPE (res) = USER_TYPE (type);
            break;
        case TC_akv:
            AKV_CONST (res) = COCopyConstant (AKV_CONST (type));
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
#ifndef DBUG_OFF
    int mem_entry;
#endif

    DBUG_ENTER ("TYCopyType");

    DBUG_EXECUTE ("NTY_MEM", mem_entry = current_allocated_mem;);

    res = CopyTypeConstructor (type, tv_id);
    if (res != NULL) {
        NTYPE_ARITY (res) = NTYPE_ARITY (type);
        NTYPE_SONS (res) = (ntype **)Malloc (sizeof (ntype *) * NTYPE_ARITY (res));
        for (i = 0; i < NTYPE_ARITY (res); i++) {
            NTYPE_SON (res, i) = TYCopyType (NTYPE_SON (type, i));
        }
    }

    DBUG_PRINT ("NTY_MEM", ("size of type copied by TYCopyType: %u",
                            current_allocated_mem - mem_entry));

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
    static str_buf *buf = NULL;
    char *res;

    DBUG_ENTER ("ScalarType2String");

    if (buf == NULL) {
        buf = StrBufCreate (64);
    }

    switch (NTYPE_CON (type)) {
    case TC_simple:
        buf = StrBufprintf (buf, "%s", mdb_type[SIMPLE_TYPE (type)]);
        break;
    case TC_symbol:
        if (SYMBOL_MOD (type) == NULL) {
            buf = StrBufprintf (buf, "%s", SYMBOL_NAME (type));
        } else {
            buf = StrBufprintf (buf, "%s:%s", SYMBOL_MOD (type), SYMBOL_NAME (type));
        }
        break;
    case TC_user:
        buf = StrBufprintf (buf, "%s", UTGetName (USER_TYPE (type)));
        break;
    case TC_poly:
        buf = StrBufprintf (buf, "<%s>", POLY_NAME (type));
        break;
    default:
        DBUG_ASSERT (0, "ScalarType2String called with non-scalar type!");
    }

    res = StrBuf2String (buf);
    StrBufFlush (buf);

    DBUG_RETURN (res);
}

static char *
ArrayType2String (ntype *type)
{
    static str_buf *buf = NULL;
    char *tmp_str;

    DBUG_ENTER ("ArrayType2String");

    if (buf == NULL) {
        buf = StrBufCreate (128);
    }

    DBUG_ASSERT (type, "ArrayType2String called with NULL!");
    DBUG_ASSERT (TYIsArray (type), "ArrayType2String called with non-array type!");

    tmp_str = ScalarType2String (AKS_BASE (type));
    buf = StrBufprint (buf, tmp_str);
    tmp_str = Free (tmp_str);

    switch (NTYPE_CON (type)) {
    case TC_akv:
        if (TYGetDim (type) > 0) {
            tmp_str = SHShape2String (0, COGetShape (AKV_CONST (type)));
            buf = StrBufprint (buf, tmp_str);
            tmp_str = Free (tmp_str);
        }
        tmp_str = COConstantData2String (3, AKV_CONST (type));
        buf = StrBufprintf (buf, "{%s}", tmp_str);
        tmp_str = Free (tmp_str);
        break;
    case TC_aks:
        if (TYGetDim (type) > 0) {
            tmp_str = SHShape2String (0, AKS_SHP (type));
            buf = StrBufprint (buf, tmp_str);
            tmp_str = Free (tmp_str);
        }
        break;
    case TC_akd:
        tmp_str = SHShape2String (AKD_DOTS (type), AKD_SHP (type));
        buf = StrBufprint (buf, tmp_str);
        tmp_str = Free (tmp_str);
        break;
    case TC_audgz:
        buf = StrBufprintf (buf, "[+]");
        break;
    case TC_aud:
        buf = StrBufprintf (buf, "[*]");
        break;
    default:
        DBUG_ASSERT (0, "ArrayType2String called with non-array type!");
    }

    tmp_str = StrBuf2String (buf);
    StrBufFlush (buf);

    DBUG_RETURN (tmp_str);
}

static str_buf *
PrintFunSep (str_buf *buf, bool multiline, int offset)
{
    DBUG_ENTER ("FunType2String");
    if (multiline) {
        buf = StrBufprintf (buf, ",\n%*s", offset, "");
    } else {
        buf = StrBufprintf (buf, ", ");
    }
    DBUG_RETURN (buf);
}

static char *
FunType2String (ntype *type, char *scal_str, bool multiline, int offset)
{
    str_buf *buf;
    char *tmp_str, *shp_str;
    shape *empty_shape;
    int i;
    int scal_len = 0;

    DBUG_ENTER ("FunType2String");

    buf = StrBufCreate (4096);
    switch (NTYPE_CON (type)) {
    case TC_fun:
        buf = StrBufprintf (buf, "{ ");
        offset += 2;
        for (i = 0; i < NTYPE_ARITY (type); i++) {
            tmp_str = FunType2String (NTYPE_SON (type, i), scal_str, multiline, offset);
            if (i > 0) {
                buf = PrintFunSep (buf, multiline, offset);
            }
            buf = StrBufprint (buf, tmp_str);
            tmp_str = Free (tmp_str);
        }
        buf = StrBufprintf (buf, "}");
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
        buf = StrBufprintf (buf, "%s[*]", scal_str);
        tmp_str
          = FunType2String (IBASE_GEN (type), scal_str, multiline, offset + scal_len + 3);
        buf = StrBufprint (buf, tmp_str);
        tmp_str = Free (tmp_str);

        /*
         * print "<scal_str>[]" instance:
         */
        if (IBASE_SCAL (type)) {
            tmp_str = FunType2String (IBASE_SCAL (type), scal_str, multiline,
                                      offset + scal_len);
            buf = PrintFunSep (buf, multiline, offset);
            buf = StrBufprint (buf, scal_str);
            buf = StrBufprint (buf, tmp_str);
            tmp_str = Free (tmp_str);
        }

        /*
         * Print TC_iarr
         */
        if (IBASE_IARR (type)) {
            tmp_str = FunType2String (IBASE_IARR (type), scal_str, multiline, offset);
            buf = StrBufprint (buf, tmp_str);
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
            buf = PrintFunSep (buf, multiline, offset);
            buf = StrBufprintf (buf, "%s[+]", scal_str);
            buf = StrBufprint (buf, tmp_str);
            tmp_str = Free (tmp_str);
        }

        /*
         * Print TC_idims
         */

        for (i = 0; i < (NTYPE_ARITY (type) - 1); i++) {
            tmp_str = FunType2String (IARR_IDIM (type, i), scal_str, multiline, offset);
            buf = StrBufprint (buf, tmp_str);
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
            buf = PrintFunSep (buf, multiline, offset);
            buf = StrBufprintf (buf, "%s%s", scal_str, shp_str);
            buf = StrBufprint (buf, tmp_str);
            shp_str = Free (shp_str);
            tmp_str = Free (tmp_str);
        }

        /*
         * Print TC_ishapes
         */

        for (i = 0; i < (NTYPE_ARITY (type) - 1); i++) {
            tmp_str = FunType2String (IDIM_ISHAPE (type, i), scal_str, multiline, offset);
            buf = StrBufprint (buf, tmp_str);
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
            buf = PrintFunSep (buf, multiline, offset);
            buf = StrBufprintf (buf, "%s%s", scal_str, shp_str);
            buf = StrBufprint (buf, tmp_str);
            shp_str = Free (shp_str);
            tmp_str = Free (tmp_str);
        }

        break;

    case TC_ires:
        offset += 4;
        tmp_str = TYType2String (IRES_TYPE (type), multiline, offset);
        buf = StrBufprintf (buf, " -> ");
        buf = StrBufprint (buf, tmp_str);
        tmp_str = Free (tmp_str);
        break;
    default:
        DBUG_ASSERT (0, "FunType2String called with non-legal type!");
        break;
    }

    tmp_str = StrBuf2String (buf);
    buf = StrBufFree (buf);

    DBUG_RETURN (tmp_str);
}

char *
TYType2String (ntype *type, bool multiline, int offset)
{
    str_buf *buf;
    char *tmp_str, *res;
    int i;

    DBUG_ENTER ("TYType2String");

    if (type == NULL) {
        res = StringCopy ("--");
    } else {

        switch (NTYPE_CON (type)) {
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
            buf = StrBufCreate (256);
            buf = StrBufprintf (buf, "(");
            if (NTYPE_ARITY (type) > 0) {
                tmp_str = TYType2String (NTYPE_SON (type, 0), multiline, offset);
                buf = StrBufprintf (buf, " %s", tmp_str);
                tmp_str = Free (tmp_str);
                for (i = 1; i < NTYPE_ARITY (type); i++) {
                    tmp_str = TYType2String (NTYPE_SON (type, i), multiline, offset);
                    buf = StrBufprintf (buf, ", %s", tmp_str);
                    tmp_str = Free (tmp_str);
                }
            }
            buf = StrBufprintf (buf, ")");
            res = StrBuf2String (buf);
            buf = StrBufFree (buf);
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
    str_buf *buf;
    char *tmp_str;
    int i, n;

    DBUG_ENTER ("TYType2DebugString");

    buf = StrBufCreate (8192);
    if (type == NULL) {
        buf = StrBufprintf (buf, "--");
    } else {
        buf = StrBufprintf (buf, "%s{ ", dbug_str[NTYPE_CON (type)]);

        switch (NTYPE_CON (type)) {
        case TC_akv:
            multiline = FALSE;
            tmp_str = COConstant2String (AKV_CONST (type));
            buf = StrBufprintf (buf, "%s, ", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        case TC_aks:
            multiline = FALSE;
            tmp_str = SHShape2String (0, AKS_SHP (type));
            buf = StrBufprintf (buf, "%s, ", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        case TC_akd:
            multiline = FALSE;
            tmp_str = SHShape2String (AKD_DOTS (type), AKD_SHP (type));
            buf = StrBufprintf (buf, "%s, ", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        case TC_aud:
            multiline = FALSE;
            break;
        case TC_simple:
            multiline = FALSE;
            buf = StrBufprintf (buf, "%s", mdb_type[SIMPLE_TYPE (type)]);
            break;
        case TC_symbol:
            multiline = FALSE;
            if (SYMBOL_MOD (type) == NULL) {
                buf = StrBufprint (buf, SYMBOL_NAME (type));
            } else {
                buf = StrBufprintf (buf, "%s:%s", SYMBOL_MOD (type), SYMBOL_NAME (type));
            }
            break;
        case TC_user:
            multiline = FALSE;
            buf = StrBufprintf (buf, "%d", USER_TYPE (type));
            break;
        case TC_poly:
            multiline = FALSE;
            buf = StrBufprint (buf, POLY_NAME (type));
            break;
        case TC_ibase:
            tmp_str = TYType2DebugString (IBASE_BASE (type), FALSE, offset);
            buf = StrBufprint (buf, tmp_str);
            buf = StrBufprint (buf, ",");
            tmp_str = Free (tmp_str);
            break;
        case TC_idim:
            buf = StrBufprintf (buf, "%d,", IDIM_DIM (type));
            break;
        case TC_ishape:
            tmp_str = SHShape2String (0, ISHAPE_SHAPE (type));
            buf = StrBufprintf (buf, "%s,", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        case TC_ires:
            if (IRES_NUMFUNS (type) > 0) {
                buf = StrBufprintf (buf, "poss: {");
                for (i = 0; i < IRES_NUMFUNS (type); i++) {
                    buf = StrBufprintf (buf, "%d ", IRES_POS (type, i));
                }
                buf = StrBufprintf (buf, "} ");
            }
            if (IRES_NUMFUNS (type) > 0) {
                buf = StrBufprintf (buf, "fundefs: {");
                for (i = 0; i < IRES_NUMFUNS (type); i++) {
                    buf = StrBufprintf (buf, F_PTR " ", IRES_FUNDEF (type, i));
                }
                buf = StrBufprintf (buf, "} ");
            }
            break;
        case TC_alpha:
            multiline = FALSE;
            tmp_str = SSIVariable2DebugString (ALPHA_SSI (type));
            buf = StrBufprintf (buf, "%s", tmp_str);
            tmp_str = Free (tmp_str);
            break;
        default:
            break;
        }

        if (variable_arity[NTYPE_CON (type)]) {
            buf = StrBufprintf (buf, " <");
        }
        n = NTYPE_ARITY (type);
        offset += 3;
        for (i = 0; i < n; i++) {
            tmp_str = TYType2DebugString (NTYPE_SON (type, i), multiline, offset);
            if (i == 0) {
                if (multiline) {
                    buf = StrBufprintf (buf, "\n%*s", offset - 1, "");
                }
                buf = StrBufprint (buf, tmp_str);
            } else {
                buf = PrintFunSep (buf, multiline, offset);
                buf = StrBufprint (buf, tmp_str);
            }
            tmp_str = Free (tmp_str);
        }
        offset -= 3;
        if (variable_arity[NTYPE_CON (type)]) {
            buf = StrBufprintf (buf, ">");
        }
        buf = StrBufprintf (buf, "}");
    }

    tmp_str = StrBuf2String (buf);
    buf = StrBufFree (buf);

    DBUG_RETURN (tmp_str);
}

char *
TYArgs2FunTypeString (node *args, ntype *rettype)
{
    str_buf *buf;
    char *tmp;

    DBUG_ENTER ("TYArgs2FunTypeString");

    buf = StrBufCreate (4096);

    StrBufprintf (buf, "PROJ { ");

    while (args != NULL) {
        ntype *atype = AVIS_TYPE (ARG_AVIS (args));

        if (atype != NULL) {
            tmp = TYType2String (atype, 0, 0);

            StrBufprintf (buf, "%s -> ", tmp);

            tmp = Free (tmp);
        }

        args = ARG_NEXT (args);
    }

    tmp = TYType2String (rettype, 0, 0);

    StrBufprintf (buf, "%s }", tmp);

    tmp = StrBuf2String (buf);

    buf = StrBufFree (buf);

    DBUG_RETURN (tmp);
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
            res = TYMakeAKS (TYCopyType (AKV_BASE (inner)),
                             SHAppendShapes (AKS_SHP (outer), TYGetShape (inner)));
            break;
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
         * AUDGZ{ a}, AKV{ b, s2, v2}  => AUDGZ{ b}
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
 *    ntype * TYDeNestTypes( ntype *nested, ntype *inner)
 *
 * description:
 *    de-nests (array) types. Since this function is NOT considered a type
 *    constructing (MakeXYZ) function, it does NOT re-use the argument types
 *    but inspects them only!
 *
 ******************************************************************************/

ntype *
TYDeNestTypes (ntype *nested, ntype *inner)
{
    ntype *res;

    DBUG_ENTER ("TYDeNestTypes");

    DBUG_ASSERT (TYIsAKS (inner),
                 "TYDeNestTypes with non AKS inner type not yet implemented!");

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
            res = TYMakeAKS (TYCopyType (AKS_BASE (nested)),
                             SHDropFromShape (-TYGetDim (inner), AKS_SHP (nested)));
            break;
        case TC_audgz:
            if (TYGetDim (nested) == 1) {
                res = TYMakeAKS (TYCopyType (AKS_BASE (nested)), SHMakeShape (0));
            } else {
                res = TYMakeAUD (TYCopyType (AKD_BASE (nested)));
            }
            break;
        case TC_aud:
            if (TYGetDim (nested) == 0) {
                res = TYMakeAKS (TYCopyType (AKS_BASE (nested)), SHMakeShape (0));
            } else {
                res = TYMakeAUD (TYCopyType (AKD_BASE (nested)));
            }
            break;
        default:
            res = TYCopyType (nested);
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
            if ((TYGetDim (nested) - TYGetDim (inner)) == 0) {
                res = TYMakeAKS (TYCopyType (AKS_BASE (nested)), SHMakeShape (0));
            } else {
                res = TYMakeAKD (TYCopyType (AKD_BASE (nested)),
                                 TYGetDim (nested) - TYGetDim (inner), SHMakeShape (0));
            }
            break;
        case TC_audgz:
            if (TYGetDim (nested) == 1) {
                res = TYMakeAKS (TYCopyType (AKS_BASE (nested)), SHMakeShape (0));
            } else {
                res = TYMakeAUD (TYCopyType (AKD_BASE (nested)));
            }
            break;
        case TC_aud:
            if (TYGetDim (nested) == 0) {
                res = TYMakeAKS (TYCopyType (AKS_BASE (nested)), SHMakeShape (0));
            } else {
                res = TYMakeAUD (TYCopyType (AKD_BASE (nested)));
            }
            break;
        default:
            res = TYCopyType (nested);
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
            if (TYGetDim (inner) == 0) {
                res = TYMakeAUDGZ (TYCopyType (AKD_BASE (nested)));
            } else {
                res = TYMakeAUD (TYCopyType (AKD_BASE (nested)));
            }
            break;
        case TC_audgz:
        case TC_aud:
            res = TYMakeAUD (TYCopyType (AKD_BASE (nested)));
            break;
        default:
            res = TYCopyType (nested);
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
        res = TYCopyType (nested);
        break;

    default:
        /*
         * a, b => a
         *
         */
        res = TYCopyType (nested);
        break;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYOldType2ScalarType( types *old)
 *
 * description:
 *    converts an old TYPES node into an ntype node for the base type.
 *
 ******************************************************************************/

ntype *
TYOldType2ScalarType (types *old)
{
    ntype *res;
    usertype udt;

#ifndef DBUG_OFF
    char *tmp, *tmp2;
#endif

    DBUG_ENTER ("TYOldType2ScalarType");

    switch (TYPES_BASETYPE (old)) {
    case T_user:
        if (TYPES_POLY (old)) {
            res = TYMakePolyType (TYPES_NAME (old));
        } else {
            udt = UTFindUserType (TYPES_NAME (old), TYPES_MOD (old));
            if (udt == UT_NOT_DEFINED) {
                res = TYMakeSymbType (StringCopy (TYPES_NAME (old)),
                                      StringCopy (TYPES_MOD (old)));
            } else {
                res = TYMakeUserType (udt);
            }
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
    case T_hidden:
        res = TYMakeSimpleType (TYPES_BASETYPE (old));
        break;
    case T_void:
    case T_unknown:
    case T_nothing:
        res = NULL;
        break;
    case T_dots:
        res = NULL;
        DBUG_ASSERT (0, "TYOldType2Type applied to T_dots");
        break;
    default:
        res = NULL;
        DBUG_ASSERT (0, "TYOldType2Type applied to illegal type");
    }

    DBUG_EXECUTE ("NTY", tmp = Type2String (old, 3, TRUE);
                  tmp2 = TYType2DebugString (res, TRUE, 0););
    DBUG_PRINT ("NTY", ("base type of %s converted into : %s\n", tmp, tmp2));
    DBUG_EXECUTE ("NTY", tmp = Free (tmp); tmp2 = Free (tmp2););

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYOldType2Type( types *old)
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

#ifndef DBUG_OFF
    char *tmp, *tmp2;
#endif

    DBUG_ENTER ("TYOldType2Type");

    res = TYOldType2ScalarType (old);

    if (res != NULL) {
        if (TYPES_DIM (old) > SCALAR) {
            res
              = TYMakeAKS (res, SHOldShpseg2Shape (TYPES_DIM (old), TYPES_SHPSEG (old)));
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

    DBUG_EXECUTE ("NTY", tmp = Type2String (old, 3, TRUE);
                  tmp2 = TYType2DebugString (res, TRUE, 0););
    DBUG_PRINT ("NTY", ("%s converted into : %s\n", tmp, tmp2));
    DBUG_EXECUTE ("NTY", tmp = Free (tmp); tmp2 = Free (tmp2););

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype * TYOldTypes2ProdType( types *old)
 *
 * description:
 *    converts a (linked list of) old TYPES node(s) into a product type of ntypes.
 *
 ******************************************************************************/

ntype *
TYOldTypes2ProdType (types *old)
{
    int i, num_types;
    ntype *res;

    num_types = (HasDotTypes (old) ? CountTypes (old) - 1 : CountTypes (old));
    res = TYMakeEmptyProductType (num_types);
    for (i = 0; i < num_types; i++) {
        res = TYSetProductMember (res, i, TYOldType2Type (old));
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
        if (NTYPE_ARITY (new) == 0) {
            res = MakeTypes1 (T_void);
        } else {
            for (i = NTYPE_ARITY (new) - 1; i >= 0; i--) {
                res = Type2OldType (PROD_MEMBER (new, i));
                TYPES_NEXT (res) = tmp;
                tmp = res;
            }
        }
        break;
    case TC_akv:
        res = Type2OldType (AKS_BASE (new));
        TYPES_DIM (res) = TYGetDim (new);
        TYPES_SHPSEG (res) = SHShape2OldShpseg (TYGetShape (new));
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
        if (SIMPLE_TYPE (new) == T_classtype) {
            res = MakeTypes (T_hidden, 0, NULL, NULL, NULL);
        } else {
            res = MakeTypes (SIMPLE_TYPE (new), 0, NULL, NULL, NULL);
        }
        break;
    case TC_user:
        res = MakeTypes (T_user, 0, NULL, StringCopy (UTGetName (USER_TYPE (new))),
                         StringCopy (UTGetMod (USER_TYPE (new))));
        TYPES_TDEF (res) = UTGetTdef (USER_TYPE (new));
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

/**
 **  functions for creating wrapper function code
 **/

/******************************************************************************
 *
 * Function:
 *   ntype *TYSplitWrapperType( ntype *type, bool *finished)
 *
 * Description:
 *   Extracts a single FUN_IBASE path from the given type 'type'.
 *   (The found path is removed from 'type'!!)
 *   Iff 'type' contains a single FUN_IBASE path only, '*finished' is set to
 *   TRUE.
 *
 ******************************************************************************/

static ntype *
SplitWrapperType (ntype *type, bool *finished)
{
    ntype *new_type;
    int n, i;
    bool old_finished, once_finished;

    DBUG_ENTER ("SplitWrapperType");

    if (type == NULL) {
        new_type = NULL;
    } else {
        n = NTYPE_ARITY (type);
        new_type = CopyTypeConstructor (type, tv_id);

        if (TYIsFun (type)) {
            DBUG_ASSERT ((n >= 1), "TC_fun with (ARITY < 1) found!");

            NTYPE_ARITY (new_type) = 1;
            NTYPE_SONS (new_type) = (ntype **)Malloc (sizeof (ntype *));

            DBUG_ASSERT ((NTYPE_SON (type, (n - 1)) != NULL),
                         "TC_fun with (NTYPE_SON == NULL) found!");

            NTYPE_SON (new_type, 0)
              = SplitWrapperType (NTYPE_SON (type, (n - 1)), finished);

            if (*finished) {
                NTYPE_ARITY (type) = n - 1;
                NTYPE_SON (type, (n - 1)) = TYFreeType (NTYPE_SON (type, (n - 1)));
                if (n > 1) {
                    (*finished) = FALSE;
                }
            }
        } else {
            NTYPE_ARITY (new_type) = n;
            NTYPE_SONS (new_type) = (ntype **)Malloc (n * sizeof (ntype *));
            old_finished = *finished;
            once_finished = old_finished;
            for (i = 0; i < n; i++) {
                *finished = old_finished;
                NTYPE_SON (new_type, i)
                  = SplitWrapperType (NTYPE_SON (type, i), finished);
                once_finished = once_finished && (*finished);
            }
            *finished = once_finished;
        }
    }

    DBUG_RETURN (new_type);
}

ntype *
TYSplitWrapperType (ntype *type, bool *finished)
{
    DBUG_ENTER ("TYSplitWrapperType");

    *finished = TRUE;
    type = SplitWrapperType (type, finished);

    DBUG_RETURN (type);
}

/******************************************************************************
 *
 * Function:
 *   ntype *TYGetWrapperRetType( ntype *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

ntype *
TYGetWrapperRetType (ntype *type)
{
    ntype *ret_type;

    DBUG_ENTER ("TYGetWrapperRetType");

    DBUG_ASSERT ((type != NULL), "no type found!");

    if (TYIsFun (type)) {
        DBUG_ASSERT ((NTYPE_ARITY (type) == 1), "multiple FUN_IBASE found!");

        type = IRES_TYPE (IBASE_GEN (FUN_IBASE (type, 0)));
        DBUG_ASSERT ((type != NULL), "IBASE_GEN not found!");
        ret_type = TYGetWrapperRetType (type);
    } else {
        DBUG_ASSERT ((TYIsProd (type)), "neither TC_fun nor TC_prod found!");
        ret_type = type;
    }

    DBUG_RETURN (ret_type);
}

/******************************************************************************
 *
 * Function:
 *   node *TYCorrectWrapperArgTypes( node *args, ntype *type)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TYCorrectWrapperArgTypes (node *args, ntype *type)
{
    DBUG_ENTER ("TYCorrectWrapperArgTypes");

    if ((args != NULL) && (TYPES_BASETYPE (ARG_TYPE (args)) != T_dots)) {
        DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "no N_exprs node found!");
        DBUG_ASSERT ((TYIsFun (type)), "no TC_fun found!");
        DBUG_ASSERT ((NTYPE_ARITY (type) == 1), "multiple FUN_IBASE found!");

        if (ARG_ATTRIB (args) != ST_regular) {
            AVIS_TYPE (ARG_AVIS (args))
              = TYMakeAKS (TYCopyType (IBASE_BASE (FUN_IBASE (type, 0))),
                           SHCreateShape (0));
        } else {
            AVIS_TYPE (ARG_AVIS (args))
              = TYMakeAUD (TYCopyType (IBASE_BASE (FUN_IBASE (type, 0))));
        }

        type = IRES_TYPE (IBASE_GEN (FUN_IBASE (type, 0)));
        ARG_NEXT (args) = TYCorrectWrapperArgTypes (ARG_NEXT (args), type);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * Function:
 *   node *TYCreateWrapperVardecs( node *fundef)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TYCreateWrapperVardecs (node *fundef)
{
    ntype *ret_type;
    int i;
    node *vardecs = NULL;

    DBUG_ENTER ("TYCreateWrapperVardecs");

    ret_type = FUNDEF_RET_TYPE (fundef);

    if (ret_type != NULL) {
        DBUG_ASSERT ((TYIsProd (ret_type)), "no TC_prod found");

        for (i = 0; i < NTYPE_ARITY (ret_type); i++) {
            vardecs = MakeVardec (TmpVar (), NULL, vardecs);
            AVIS_TYPE (VARDEC_AVIS (vardecs)) = TYCopyType (PROD_MEMBER (ret_type, i));
        }
    }

    DBUG_RETURN (vardecs);
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
Arg2Id (node *arg)
{
    node *id;

    DBUG_ENTER ("Arg2Id");

    DBUG_ASSERT (((arg != NULL) && (NODE_TYPE (arg) == N_arg)), "no N_arg found!");

    id = MakeId_Copy (ARG_NAME (arg));
    ID_VARDEC (id) = arg;
    SET_FLAG (ID, id, IS_GLOBAL, FALSE);
    SET_FLAG (ID, id, IS_REFERENCE,
              ((ARG_ATTRIB (arg) == ST_reference)
               || (ARG_ATTRIB (arg) == ST_readonly_reference)));
    SET_FLAG (ID, id, IS_READ_ONLY, (ARG_ATTRIB (arg) == ST_readonly_reference));
    ID_AVIS (id) = ARG_AVIS (arg);

    DBUG_RETURN (id);
}

static ids *
VardecOrArg2Ids (node *vardec)
{
    ids *ret_ids;

    DBUG_ENTER ("VardecOrArg2Ids");

    DBUG_ASSERT ((vardec != NULL), "no vardec found!");

    ret_ids = MakeIds_Copy (VARDEC_OR_ARG_NAME (vardec));
    IDS_VARDEC (ret_ids) = vardec;
    IDS_AVIS (ret_ids) = VARDEC_OR_ARG_AVIS (vardec);

    DBUG_RETURN (ret_ids);
}

static ids *
Vardecs2Ids (node *vardecs)
{
    ids *tmp_ids;
    ids *ret_ids = NULL;

    DBUG_ENTER ("Vardecs2Ids");

    while (vardecs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (vardecs) == N_vardec), "no N_vardec found!");

        tmp_ids = VardecOrArg2Ids (vardecs);
        IDS_NEXT (tmp_ids) = ret_ids;
        ret_ids = tmp_ids;
        vardecs = VARDEC_NEXT (vardecs);
    }

    DBUG_RETURN (ret_ids);
}

static node *
Args2Exprs (node *args)
{
    node *exprs;

    DBUG_ENTER ("Args2Exprs");

    if (args != NULL) {
        DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "no N_arg found!");

        exprs = MakeExprs (Arg2Id (args), Args2Exprs (ARG_NEXT (args)));
    } else {
        exprs = NULL;
    }

    DBUG_RETURN (exprs);
}

static ids *
ReferenceArgs2Ids (node *args)
{
    ids *ret_ids;

    DBUG_ENTER ("ReferenceArgs2Ids");

    if (args != NULL) {
        DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "no N_arg found!");

        if (ARG_ATTRIB (args) == ST_reference) {
            ret_ids = VardecOrArg2Ids (args);
            IDS_NEXT (ret_ids) = ReferenceArgs2Ids (ARG_NEXT (args));
        } else {
            ret_ids = ReferenceArgs2Ids (ARG_NEXT (args));
        }
    } else {
        ret_ids = NULL;
    }

    DBUG_RETURN (ret_ids);
}

static node *
BuildTmpId (ntype *type, node **new_vardecs)
{
    node *id;
    char *name;

    DBUG_ENTER ("BuildTmpId");

    name = TmpVar ();
    id = MakeId (name, NULL, ST_regular);
    /*
     * All these tmp_id are used for flattening code that checks shapes / dims;
     * Hence, we are sure that we are not dealing with unique objects here!!
     */
    SET_FLAG (ID, id, IS_GLOBAL, FALSE);
    SET_FLAG (ID, id, IS_REFERENCE, FALSE);
    *new_vardecs = ID_VARDEC (id) = MakeVardec (StringCopy (name), NULL, *new_vardecs);
    ID_AVIS (id) = VARDEC_AVIS (ID_VARDEC (id));
    AVIS_TYPE (ID_AVIS (id)) = type;

    DBUG_RETURN (id);
}

static ids *
BuildTmpIds (ntype *type, node **new_vardecs)
{
    ids *ids;
    char *name;

    DBUG_ENTER ("BuildTmpIds");

    name = TmpVar ();
    ids = MakeIds (name, NULL, ST_regular);
    *new_vardecs = IDS_VARDEC (ids) = MakeVardec (StringCopy (name), NULL, *new_vardecs);
    IDS_AVIS (ids) = VARDEC_AVIS (IDS_VARDEC (ids));
    AVIS_TYPE (IDS_AVIS (ids)) = type;

    DBUG_RETURN (ids);
}

static node *
GetPrfOrFundef (node *assigns)
{
    node *res;

    DBUG_ENTER ("GetApOrPrf");

    DBUG_ASSERT (((assigns != NULL) && (NODE_TYPE (assigns) == N_assign)),
                 "no assignment found!");

    if ((ASSIGN_NEXT (assigns) == NULL)
        && (NODE_TYPE (ASSIGN_INSTR (assigns)) == N_let)) {
        if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assigns))) == N_prf) {
            res = LET_EXPR (ASSIGN_INSTR (assigns));
        } else if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assigns))) == N_ap) {
            res = AP_FUNDEF (LET_EXPR (ASSIGN_INSTR (assigns)));
            DBUG_ASSERT ((res != NULL), "AP_FUNDEF not found!");
            DBUG_ASSERT ((NODE_TYPE (res) == N_fundef), "no N_fundef node found!");
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

    DBUG_ENTER ("BranchesAreEquivalent");

    prf_or_fundef1 = GetPrfOrFundef (assigns1);
    prf_or_fundef2 = GetPrfOrFundef (assigns2);

    if ((prf_or_fundef1 != NULL) && (prf_or_fundef2 != NULL)) {
        if ((NODE_TYPE (prf_or_fundef1) == N_prf)
            && (NODE_TYPE (prf_or_fundef2) == N_prf)) {
            DBUG_ASSERT (((PRF_PRF (prf_or_fundef1) == F_type_error)
                          && (PRF_PRF (prf_or_fundef2) == F_type_error)),
                         "illegal prf found!");
            res = TRUE;
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

    DBUG_ENTER ("BuildDimAssign");

    assign = MakeAssign (MakeLet (MakePrf (F_dim, MakeExprs (Arg2Id (arg), NULL)),
                                  BuildTmpIds (TYMakeAKS (TYMakeSimpleType (T_int),
                                                          SHCreateShape (0)),
                                               new_vardecs)),
                         NULL);

    DBUG_RETURN (assign);
}

static node *
BuildShapeAssign (node *arg, node **new_vardecs)
{
    node *assign;

    DBUG_ENTER ("BuildShapeAssign");

    assign = MakeAssign (MakeLet (MakePrf (F_shape, MakeExprs (Arg2Id (arg), NULL)),
                                  BuildTmpIds (TYMakeAUDGZ (TYMakeSimpleType (T_int)),
                                               new_vardecs)),
                         NULL);

    DBUG_RETURN (assign);
}

static node *
BuildCondAssign (node *prf_ass, prf rel_prf, node *expr, node *then_ass, node *else_ass,
                 node **new_vardecs)
{
    ids *prf_ids;
    prf prf;
    node *assigns;
    node *id;

    DBUG_ENTER ("BuildCondAssign");

    if (BranchesAreEquivalent (then_ass, else_ass)) {
        /*
         * both parts of the conditional contain the same code
         *   -> no need to build the conditional!
         */
        assigns = then_ass;
        else_ass = FreeTree (else_ass);
    } else {
        prf_ids = ASSIGN_LHS (prf_ass);
        prf = PRF_PRF (ASSIGN_RHS (prf_ass));

        switch (prf) {
        case F_dim: {
            node *prf2;
            ids *prf_ids2;

            DBUG_ASSERT ((NODE_TYPE (expr) == N_num), "illegal expression found!");

            id = DupIds_Id (prf_ids);
            SET_FLAG (ID, id, IS_GLOBAL, FALSE);
            SET_FLAG (ID, id, IS_REFERENCE, FALSE);

            prf2 = MakePrf (rel_prf, MakeExprs (id, MakeExprs (expr, NULL)));
            prf_ids2
              = BuildTmpIds (TYMakeAKS (TYMakeSimpleType (T_bool), SHCreateShape (0)),
                             new_vardecs);

            id = DupIds_Id (prf_ids2);
            SET_FLAG (ID, id, IS_GLOBAL, FALSE);
            SET_FLAG (ID, id, IS_REFERENCE, FALSE);
            assigns = MakeAssign (MakeLet (prf2, prf_ids2),
                                  MakeAssign (MakeCond (id, MakeBlock (then_ass, NULL),
                                                        MakeBlock (else_ass, NULL)),
                                              NULL));
        } break;

        case F_shape: {
            node *prf2, *prf3, *prf4;
            node *flt_prf2, *flt_prf3, *flt_prf4;
            node *aexprs, *last_ass;
            int dim;

            DBUG_ASSERT ((NODE_TYPE (expr) == N_array), "illegal expression found!");

            last_ass = assigns = MakeAssign (NULL, NULL); /* dummy assignment */
            aexprs = ARRAY_AELEMS (expr);
            dim = 0;
            flt_prf4 = MakeBool (TRUE);
            while (aexprs != NULL) {
                id = DupIds_Id (prf_ids);
                SET_FLAG (ID, id, IS_GLOBAL, FALSE);
                SET_FLAG (ID, id, IS_REFERENCE, FALSE);
                prf2
                  = MakePrf (F_sel,
                             MakeExprs (MakeFlatArray (MakeExprs (MakeNum (dim), NULL)),
                                        MakeExprs (id, NULL)));
                flt_prf2
                  = BuildTmpId (TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (0)),
                                new_vardecs);

                prf3
                  = MakePrf (rel_prf,
                             MakeExprs (flt_prf2, MakeExprs (EXPRS_EXPR (aexprs), NULL)));
                flt_prf3
                  = BuildTmpId (TYMakeAKS (TYMakeSimpleType (T_bool), SHCreateShape (0)),
                                new_vardecs);

                prf4 = MakePrf (F_and, MakeExprs (flt_prf3, MakeExprs (flt_prf4, NULL)));
                flt_prf4
                  = BuildTmpId (TYMakeAKS (TYMakeSimpleType (T_bool), SHCreateShape (0)),
                                new_vardecs);

                ASSIGN_NEXT (last_ass)
                  = MakeAssign (MakeLet (prf2, DupId_Ids (flt_prf2)),
                                MakeAssign (MakeLet (prf3, DupId_Ids (flt_prf3)),
                                            MakeAssign (MakeLet (prf4,
                                                                 DupId_Ids (flt_prf4)),
                                                        NULL)));
                last_ass = ASSIGN_NEXT (ASSIGN_NEXT (ASSIGN_NEXT (last_ass)));

                aexprs = EXPRS_NEXT (aexprs);
                dim++;
            }
            ASSIGN_NEXT (last_ass)
              = MakeAssign (MakeCond (flt_prf4, MakeBlock (then_ass, NULL),
                                      MakeBlock (else_ass, NULL)),
                            NULL);
            assigns = FreeNode (assigns); /* free dummy assignment */

            ARRAY_AELEMS (expr) = NULL;
            expr = FreeNode (expr);
        } break;

        default:
            DBUG_ASSERT ((0), "illegal prf found!");
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
    ids *lhs;
    ids *tmp_ids;
    ntype *ret_type;
    int i;

    DBUG_ENTER ("BuildApAssign");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no fundef found!");

    assigns = NULL;
    lhs = NULL;
    ret_type = FUNDEF_RET_TYPE (fundef);
    i = NTYPE_ARITY (ret_type) - 1;
    while (i >= 0) {
        DBUG_ASSERT ((vardecs != NULL), "inconsistant application found");

        tmp_id = BuildTmpId (TYCopyType (PROD_MEMBER (ret_type, i)), new_vardecs);
        assigns = MakeAssign (MakeLet (tmp_id, VardecOrArg2Ids (vardecs)), assigns);

        tmp_ids = DupId_Ids (tmp_id);
        IDS_NEXT (tmp_ids) = lhs;
        lhs = tmp_ids;

        i--;
        vardecs = VARDEC_NEXT (vardecs);
    }
    DBUG_ASSERT ((vardecs == NULL), "inconsistant application found");

    ap = MakeAp (StringCopy (FUNDEF_NAME (fundef)), NULL, Args2Exprs (args));
    AP_FUNDEF (ap) = fundef;

    assigns = MakeAssign (MakeLet (ap, lhs), assigns);

    DBUG_RETURN (assigns);
}

static node *
BuildErrorAssign (char *funname, node *args, node *vardecs)
{
    node *assigns;

    DBUG_ENTER ("BuildErrorAssign");

    assigns
      = MakeAssign (MakeLet (MakePrf (F_type_error, MakeExprs (MakeStr_Copy (funname),
                                                               Args2Exprs (args))),
                             Vardecs2Ids (vardecs)),
                    NULL);

    DBUG_RETURN (assigns);
}

static bool
IsRelevant (ntype *type)
{
    bool ret;
    ntype *ires;
    int i;

    DBUG_ENTER ("IsRelevant");

    DBUG_ASSERT ((type != NULL), "no type found!");

    switch (TYGetConstr (type)) {
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
        DBUG_ASSERT ((0), "illegal ntype constructor found!");
        ires = NULL;
        break;
    }
    DBUG_ASSERT ((ires != NULL), "I..._GEN not found!");

    ret = FALSE;
    for (i = 0; i < IRES_NUMFUNS (ires); i++) {
        if (IRES_POS (ires, i) == 0) {
            ret = TRUE;
        }
    }

    DBUG_RETURN (ret);
}

static node *
CreateWrapperCode (ntype *type, DFT_state *state, int lower, char *funname, node *arg,
                   node *args, node *vardecs, node **new_vardecs)
{
    node *assigns;
    node *tmp_ass;
    int i;

    DBUG_ENTER ("CreateWrapperCode");

    DBUG_ASSERT ((type != NULL), "no type found!");

    switch (TYGetConstr (type)) {
    case TC_fun:
        DBUG_ASSERT ((NTYPE_ARITY (type) == 1), "multipe FUN_IBASE found!");
        assigns = CreateWrapperCode (FUN_IBASE (type, 0), state, lower, funname, arg,
                                     args, vardecs, new_vardecs);
        break;

    case TC_ibase:
        if (ARG_ATTRIB (arg) != ST_regular) {
            /*
             * this should identify all legal unique argument types!
             */
            DBUG_ASSERT ((IBASE_SCAL (type) != NULL),
                         "fun without instance for scalar unique argument found");
            assigns = CreateWrapperCode (IBASE_SCAL (type), state, 0, funname, arg, args,
                                         vardecs, new_vardecs);
        } else {

            DBUG_ASSERT ((IBASE_GEN (type) != NULL), "IBASE_GEN not found!");
            if (IBASE_IARR (type) != NULL) {
                assigns = CreateWrapperCode (IBASE_IARR (type), state, lower, funname,
                                             arg, args, vardecs, new_vardecs);
                if (IsRelevant (IBASE_IARR (type))) {
                    /*
                     * this conditional is needed only iff an instance with type [+]
                     * exists
                     */
                    tmp_ass = BuildDimAssign (arg, new_vardecs);
                    assigns = BuildCondAssign (tmp_ass, F_gt, MakeNum (0), assigns,
                                               CreateWrapperCode (IBASE_GEN (type), state,
                                                                  3, funname, arg, args,
                                                                  vardecs, new_vardecs),
                                               new_vardecs);
                    assigns = AppendAssign (tmp_ass, assigns);
                }
            } else {
                assigns = CreateWrapperCode (IBASE_GEN (type), state, 3, funname, arg,
                                             args, vardecs, new_vardecs);
            }
            if (IBASE_SCAL (type) != NULL) {
                tmp_ass = BuildDimAssign (arg, new_vardecs);
                assigns = BuildCondAssign (tmp_ass, F_eq, MakeNum (0),
                                           CreateWrapperCode (IBASE_SCAL (type), state, 0,
                                                              funname, arg, args, vardecs,
                                                              new_vardecs),
                                           assigns, new_vardecs);
                assigns = AppendAssign (tmp_ass, assigns);
            }
        }
        break;

    case TC_iarr:
        DBUG_ASSERT ((IARR_GEN (type) != NULL), "IARR_GEN not found!");
        assigns = CreateWrapperCode (IARR_GEN (type), state, 2, funname, arg, args,
                                     vardecs, new_vardecs);

        if (NTYPE_ARITY (type) >= 2) {
            tmp_ass = BuildDimAssign (arg, new_vardecs);
            for (i = NTYPE_ARITY (type) - 2; i >= 0; i--) {
                if (IARR_IDIM (type, i) != NULL) {
                    assigns
                      = BuildCondAssign (tmp_ass, F_eq,
                                         MakeNum (IDIM_DIM (IARR_IDIM (type, i))),
                                         CreateWrapperCode (IARR_IDIM (type, i), state,
                                                            lower, funname, arg, args,
                                                            vardecs, new_vardecs),
                                         assigns, new_vardecs);
                }
            }
            assigns = AppendAssign (tmp_ass, assigns);
        }
        break;

    case TC_idim:
        DBUG_ASSERT ((IDIM_GEN (type) != NULL), "IDIM_GEN not found!");
        assigns = CreateWrapperCode (IDIM_GEN (type), state, 1, funname, arg, args,
                                     vardecs, new_vardecs);

        if (NTYPE_ARITY (type) >= 2) {
            tmp_ass = BuildShapeAssign (arg, new_vardecs);
            for (i = NTYPE_ARITY (type) - 2; i >= 0; i--) {
                if (IDIM_ISHAPE (type, i) != NULL) {
                    assigns
                      = BuildCondAssign (tmp_ass, F_eq,
                                         SHShape2Array (
                                           ISHAPE_SHAPE (IDIM_ISHAPE (type, i))),
                                         CreateWrapperCode (IDIM_ISHAPE (type, i), state,
                                                            lower, funname, arg, args,
                                                            vardecs, new_vardecs),
                                         assigns, new_vardecs);
                }
            }
            assigns = AppendAssign (tmp_ass, assigns);
        }
        break;

    case TC_ishape:
        DBUG_ASSERT ((ISHAPE_GEN (type) != NULL), "ISHAPE_GEN not found!");
        assigns = CreateWrapperCode (ISHAPE_GEN (type), state, 0, funname, arg, args,
                                     vardecs, new_vardecs);
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
            assigns = BuildErrorAssign (funname, args, vardecs);
        } else if (TYIsProd (IRES_TYPE (type))) {
            DFT_res *res;
            node *fundef;

            state = FinalizeDFT_state (state);
            res = DFT_state2DFT_res (state);
            DBUG_ASSERT (((res->num_partials == 0)
                          && (res->num_deriveable_partials == 0)),
                         "partials found!");
            if (res->def != NULL) {
                DBUG_ASSERT ((res->deriveable == NULL), "def and deriveable found!");
                fundef = res->def;
            } else {
                fundef = res->deriveable;
            }

            if (fundef == NULL) {
                assigns = BuildErrorAssign (funname, args, vardecs);
            } else {
                assigns = BuildApAssign (fundef, args, vardecs, new_vardecs);
            }

            res = TYFreeDFT_res (res);
        } else {
            assigns = CreateWrapperCode (IRES_TYPE (type), state, lower, funname,
                                         ARG_NEXT (arg), args, vardecs, new_vardecs);
        }

        state = FreeDFT_state (state);
        break;

    default:
        DBUG_ASSERT ((0), "illegal ntype constructor found!");
        assigns = NULL;
        break;
    }

    DBUG_RETURN (assigns);
}

node *
TYCreateWrapperCode (node *fundef, node *vardecs, node **new_vardecs)
{
    node *assigns;
    ntype *type = FUNDEF_TYPE (fundef);

    DBUG_ENTER ("TYCreateWrapperCode");

    if (type == NULL) {
        assigns = NULL;
    } else if (TYGetConstr (type) == TC_prod) {
        /*
         * pure TC_prod type (function with no arguments)!!
         *   -> fundef can be found in FUNDEF_IMPL (dirty hack!)
         */
        DBUG_ASSERT ((FUNDEF_IMPL (fundef) != NULL), "FUNDEF_IMPL not found!");
        assigns = BuildApAssign (FUNDEF_IMPL (fundef), FUNDEF_ARGS (fundef), vardecs,
                                 new_vardecs);
    } else {
        DBUG_ASSERT ((!HasDotTypes (FUNDEF_TYPES (fundef))),
                     "wrapper function with ... return type found!");
        DBUG_ASSERT ((!HasDotArgs (FUNDEF_ARGS (fundef))),
                     "wrapper function with ... argument found!");
        assigns
          = CreateWrapperCode (type, NULL, 0, FUNDEF_NAME (fundef), FUNDEF_ARGS (fundef),
                               FUNDEF_ARGS (fundef), vardecs, new_vardecs);
    }

    DBUG_RETURN (assigns);
}

/**
 ** Serialization support
 **/

#ifdef NEW_AST

static void
SerializeSimpleType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeSimpleType");

    fprintf (file, "TYDeserializeType( %d, %d)", NTYPE_CON (type), SIMPLE_TYPE (type));

    DBUG_VOID_RETURN;
}

static void
SerializeSymbolType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeSymbolType");

    fprintf (file, "TYDeserializeType( %d, \"%s\", \"%s\")", NTYPE_CON (type),
             SYMBOL_NAME (type), SYMBOL_MOD (type));

    DBUG_VOID_RETURN;
}

static void
SerializeUserType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeUserType");

    fprintf (file, "TYDeserializeType( %d, \"%s\", \"%s\")", NTYPE_CON (type),
             UTGetName (USER_TYPE (type)), UTGetMod (USER_TYPE (type)));

    DBUG_VOID_RETURN;
}

static void
SerializeAKVType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeAKVType");

    fprintf (file, "TYDeserializeType( %d, ", NTYPE_CON (type));

    TYSerializeType (file, AKV_BASE (type));

    fprintf (file, ", ");

    COSerializeConstant (file, AKV_CONST (type));

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeAKSType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeAKSType");

    fprintf (file, "TYDeserializeType( %d, ", NTYPE_CON (type));

    TYSerializeType (file, AKS_BASE (type));

    fprintf (file, ", ");

    SHSerializeShape (file, AKS_SHP (type));

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeAKDType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeAKDType");

    fprintf (file, "TYDeserializeType( %d, ", NTYPE_CON (type));

    TYSerializeType (file, AKD_BASE (type));

    fprintf (file, ", %d, ", AKD_DOTS (type));

    SHSerializeShape (file, AKD_SHP (type));

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeAUDType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeAUDType");

    fprintf (file, "TYDeserializeType( %d, ", NTYPE_CON (type));

    TYSerializeType (file, AUD_BASE (type));

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeAUDGZType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeAUDGZType");

    fprintf (file, "TYDeserializeType( %d, ", NTYPE_CON (type));

    TYSerializeType (file, AUDGZ_BASE (type));

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeProdType (FILE *file, ntype *type)
{
    int cnt;

    DBUG_ENTER ("SerializeProdType");

    fprintf (file, "TYDeserializeType( %d, %d", NTYPE_CON (type), NTYPE_ARITY (type));

    for (cnt = 0; cnt < NTYPE_ARITY (type); cnt++) {
        fprintf (file, ", ");

        TYSerializeType (file, PROD_MEMBER (type, cnt));
    }

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeUnionType (FILE *file, ntype *type)
{
    int cnt;

    DBUG_ENTER ("SerializeUnionType");

    fprintf (file, "TYDeserializeType( %d, %d", NTYPE_CON (type), NTYPE_ARITY (type));

    for (cnt = 0; cnt < NTYPE_ARITY (type); cnt++) {
        fprintf (file, ", ");

        TYSerializeType (file, UNION_MEMBER (type, cnt));
    }

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeFunType (FILE *file, ntype *type)
{
    int cnt;

    DBUG_ENTER ("SerializeFunType");

    fprintf (file, "TYDeserializeType( %d, %d", NTYPE_CON (type), NTYPE_ARITY (type));

    for (cnt = 0; cnt < NTYPE_ARITY (type); cnt++) {
        fprintf (file, ", ");

        TYSerializeType (file, FUN_IBASE (type, cnt));
    }

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeIBaseType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeIBaseType");

    fprintf (file, "TYDeserializeType( %d, ", NTYPE_CON (type));

    TYSerializeType (file, IBASE_BASE (type));

    fprintf (file, ", ");

    TYSerializeType (file, IBASE_SCAL (type));

    fprintf (file, ", ");

    TYSerializeType (file, IBASE_GEN (type));

    fprintf (file, ", ");

    TYSerializeType (file, IBASE_IARR (type));

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeIArrType (FILE *file, ntype *type)
{
    int cnt;

    DBUG_ENTER ("SerializeIArrType");

    fprintf (file, "TYDeserializeType( %d, %d, ", NTYPE_CON (type), NTYPE_ARITY (type));

    TYSerializeType (file, IARR_GEN (type));

    for (cnt = 0; cnt < NTYPE_ARITY (type) - 1; cnt++) {
        fprintf (file, ", ");

        TYSerializeType (file, IARR_IDIM (type, cnt));
    }

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeIDimType (FILE *file, ntype *type)
{
    int cnt;

    DBUG_ENTER ("SerializeIDimType");

    fprintf (file, "TYDeserializeType( %d, %d, %d, ", NTYPE_CON (type),
             NTYPE_ARITY (type), IDIM_DIM (type));

    TYSerializeType (file, IDIM_GEN (type));

    for (cnt = 0; cnt < NTYPE_ARITY (type) - 1; cnt++) {
        fprintf (file, ", ");

        TYSerializeType (file, IDIM_ISHAPE (type, cnt));
    }

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeIShapeType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeIShapeType");

    fprintf (file, "TYDeserializeType( %d, ", NTYPE_CON (type));

    SHSerializeShape (file, ISHAPE_SHAPE (type));

    fprintf (file, ", ");

    TYSerializeType (file, ISHAPE_GEN (type));

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

static void
SerializeIResType (FILE *file, ntype *type)
{
    int cnt;

    DBUG_ENTER ("SerializeIResType");

    fprintf (file, "TYDeserializeType( %d, %d", NTYPE_CON (type), IRES_NUMFUNS (type));

    for (cnt = 0; cnt < IRES_NUMFUNS (type); cnt++) {
        fprintf (file, ", ");
        SerializeFundefLink (IRES_FUNDEF (type, cnt), file);
    }

    for (cnt = 0; cnt < IRES_NUMFUNS (type); cnt++) {
        fprintf (file, ", %d", IRES_POS (type, cnt));
    }

    fprintf (file, ", ");

    TYSerializeType (file, IRES_TYPE (type));

    fprintf (file, ") ");

    DBUG_VOID_RETURN;
}

static void
SerializeAlphaType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeAlphaType");

    /*
    DBUG_ASSERT( 0,
        "Cannot handle alpha types");
        */

    fprintf (file, "NULL");

    DBUG_VOID_RETURN;
}

static void
SerializePolyType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializePolyType");

    fprintf (file, "TYDeserializeType( %d, \"%s\")", NTYPE_CON (type), POLY_NAME (type));

    DBUG_VOID_RETURN;
}

static void
SerializeDummyType (FILE *file, ntype *type)
{
    DBUG_ENTER ("SerializeDummyType");

    fprintf (file, "TYDeserializeType( %d)", NTYPE_CON (type));

    DBUG_VOID_RETURN;
}

void
TYSerializeType (FILE *file, ntype *type)
{
    DBUG_ENTER ("TYSerializeType");

    if (type == NULL) {
        DBUG_PRINT ("SET", ("Processing type (null)"));

        fprintf (file, "NULL");

        DBUG_PRINT ("SET", ("Done processing type (null)"));
    } else {
        DBUG_PRINT ("SET", ("Processing type %s", dbug_str[NTYPE_CON (type)]));

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
        case TC_dummy:
            SerializeDummyType (file, type);
            break;
        }

        DBUG_PRINT ("SET", ("Done processing type %s", dbug_str[NTYPE_CON (type)]));
    }

    DBUG_VOID_RETURN;
}

/**
 ** De-Serialization support
 **/

ntype *
TYDeserializeType (typeconstr con, ...)
{
    ntype *result = NULL;
    int cnt;
    va_list args;

    DBUG_ENTER ("TYDeserializeType");

    DBUG_PRINT ("SER", ("Deserializing ntype %s", dbug_str[con]));

    switch (con) {
    case TC_simple:
        va_start (args, con);

        result = TYMakeSimpleType (va_arg (args, simpletype));

        va_end (args);
        break;
    case TC_symbol:
        va_start (args, con);

        result = TYMakeSymbType (va_arg (args, char *), va_arg (args, char *));

        va_end (args);
        break;
    case TC_user:
        va_start (args, con);

        /* TODO: lookup usertype, if found, set value, otherwise
                 load type and set value then */

        result = TYMakeUserType (0);

        va_end (args);
        break;
    case TC_akv:
        va_start (args, con);

        result = TYMakeAKV (va_arg (args, ntype *), va_arg (args, constant *));

        va_end (args);
        break;
    case TC_aks:
        va_start (args, con);

        result = TYMakeAKS (va_arg (args, ntype *), va_arg (args, shape *));

        va_end (args);
        break;
    case TC_akd:
        va_start (args, con);

        result = TYMakeAKD (va_arg (args, ntype *), va_arg (args, int),
                            va_arg (args, shape *));

        va_end (args);
        break;
    case TC_aud:
        va_start (args, con);

        result = TYMakeAUD (va_arg (args, ntype *));

        va_end (args);
        break;
    case TC_audgz:
        va_start (args, con);

        result = TYMakeAUDGZ (va_arg (args, ntype *));
        break;
    case TC_prod:
        va_start (args, con);

        result = MakeNtype (TC_prod, va_arg (args, int));

        for (cnt = 0; cnt < NTYPE_ARITY (result); cnt++) {
            PROD_MEMBER (result, cnt) = va_arg (args, ntype *);
        }

        va_end (args);
        break;
    case TC_union:
        va_start (args, con);

        result = MakeNtype (TC_union, va_arg (args, int));

        for (cnt = 0; cnt < NTYPE_ARITY (result); cnt++) {
            UNION_MEMBER (result, cnt) = va_arg (args, ntype *);
        }

        va_end (args);
        break;
    case TC_fun:
        va_start (args, con);

        result = MakeNtype (TC_fun, va_arg (args, int));

        for (cnt = 0; cnt < NTYPE_ARITY (result); cnt++) {
            FUN_IBASE (result, cnt) = va_arg (args, ntype *);
        }

        va_end (args);
        break;
    case TC_ibase:
        va_start (args, con);

        result = MakeNtype (TC_ibase, 3);

        IBASE_BASE (result) = va_arg (args, ntype *);
        IBASE_SCAL (result) = va_arg (args, ntype *);
        IBASE_GEN (result) = va_arg (args, ntype *);
        IBASE_IARR (result) = va_arg (args, ntype *);

        va_end (args);
        break;
    case TC_iarr:
        va_start (args, con);

        result = MakeNtype (TC_iarr, va_arg (args, int));

        IARR_GEN (result) = va_arg (args, ntype *);

        for (cnt = 0; cnt < NTYPE_ARITY (result) - 1; cnt++) {
            IARR_IDIM (result, cnt) = va_arg (args, ntype *);
        }

        va_end (args);
        break;
    case TC_idim:
        va_start (args, con);

        result = MakeNtype (TC_idim, va_arg (args, int));

        IDIM_DIM (result) = va_arg (args, int);

        IDIM_GEN (result) = va_arg (args, ntype *);

        for (cnt = 0; cnt < NTYPE_ARITY (result) - 1; cnt++) {
            IDIM_ISHAPE (result, cnt) = va_arg (args, ntype *);
        }

        va_end (args);
        break;
    case TC_ishape:
        va_start (args, con);

        result = MakeNtype (TC_ishape, 1);

        ISHAPE_SHAPE (result) = va_arg (args, shape *);
        ISHAPE_GEN (result) = va_arg (args, ntype *);

        va_end (args);
        break;
    case TC_ires:
        va_start (args, con);

        result = MakeNtype (TC_ires, 1);
        IRES_NUMFUNS (result) = va_arg (args, int);

        if (IRES_NUMFUNS (result) <= 0) {
            IRES_FUNDEFS (result) = NULL;
            IRES_POSS (result) = NULL;
        } else {
            IRES_FUNDEFS (result) = Malloc (sizeof (node *) * IRES_NUMFUNS (result));
            IRES_POSS (result) = Malloc (sizeof (int) * IRES_NUMFUNS (result));

            for (cnt = 0; cnt < IRES_NUMFUNS (result); cnt++) {
                IRES_FUNDEF (result, cnt) = va_arg (args, node *);
            }

            for (cnt = 0; cnt < IRES_NUMFUNS (result); cnt++) {
                IRES_POS (result, cnt) = va_arg (args, int);
            }
        }

        IRES_TYPE (result) = va_arg (args, ntype *);

        va_end (args);
        break;
    case TC_alpha:
        DBUG_ASSERT (0, "Cannot deserialize alpha types");

        result = NULL;
        break;
    case TC_poly:
        va_start (args, con);

        result = TYMakePolyType (StringCopy (va_arg (args, char *)));

        va_end (args);
        break;
    case TC_dummy:
        result = MakeNtype (TC_dummy, 0);

        break;
    }

    DBUG_RETURN (result);
}

#endif

/* @} */ /* defgroup nty */
