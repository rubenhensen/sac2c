/*
 * $Log$
 * Revision 3.4  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker !!
 *
 * Revision 3.3  2002/05/31 14:51:54  sbs
 * intermediate version to ensure compilable overall state.
 *
 * Revision 3.2  2002/03/12 15:14:15  sbs
 * AUDGZ types added functions for handling function types added
 * type comparison as well as Type2String implemented.
 *
 * Revision 3.1  2000/11/20 18:00:10  sacbase
 * new release made
 *
 * Revision 1.2  1999/10/20 15:45:10  sbs
 * some code brushing done.
 *
 * Revision 1.1  1999/10/20 12:52:13  sbs
 * Initial revision
 *
 *
 */

#ifndef _new_types_h
#define _new_types_h

/*
 * The module "new_types" implements a new abstract datatype for representing
 * types. This file describes the interface to that module.
 *
 * In principle, the following type constructs are supported:
 *
 * 1) Array types
 * ==================
 *    Array types are distinguished by two attributes:
 *    - the array shape
 *    - their "element type" (also referred to as "scalar type")
 *
 *    IMPORTANT NOTICE:
 *      Although all "scalar types" are technically treated in the same way
 *      as all other types are, they (for themselves) do NOT constitute legal types in
 *      SAC! They are used as attributes of array types only!!!
 *
 *    At the time being, the following types are legal scalar types:
 *      a) simple types      (these are the built-in types known from C proper)
 *      b) user types        (new types defined by means of  typedef's)
 *      c) symbolic types    (types of unknown structure; basically (void *))
 *
 *    Among array types with identical scalar type, there exists a type hierarchy.
 *    It reflects different levels of shape restrictions:
 *      a) AUD array types   (arrays of unknown dimensionality)
 *      b) AUDGZ array types (arrays of unknown dimensionality greater than zero)
 *      c) AKD array types   (arrays of a fixed (known) dimensionality)
 *      d) AKS array types   (arrays of a fixed (known) shape)
 *    This hierarchy is formalized by a subtype relation which in fact is reflected
 *    by the type comparison function TYCmpTypes (see down below). Wrt. array types,
 *    it returns one of the following values:
 *    - TY_eq   iff the types are identical
 *    - TY_lt   iff the first arg is a non-identical subtype
 *    - TY_gt   iff the first arg is a non-identical supertype
 *    - TY_hcs  iff the types are not directly related but in fact do have a
 *                  common supertype, i.e., they have the same scalar type.
 *    - TY_dis  iff the types are disjoint, i.e., they either have different
 *                  scalar types, or one of them in fact is not an array type
 *
 * 2) Function types
 * ==================
 *
 *    Although, at the time being, in SaC all functions do have a fixed arity,
 *    the function types are curried here. (We hope that this might allow for
 *    future expansion ...). In order to capture the subtype relation between
 *    array types properly, function types in fact are intersection types
 *    which we do represent by a set of types of the form  alpha -> beta.
 *    However, the handling of these sets is kept almost entirely encapsulated
 *    within this module. For example, calling TYMakeFunType (see down below)
 *    with argument type   int[3]   and result type  int[.]  , an internal
 *    representation for the following intersection type will be created:
 *         { int[*] -> int[.], int[.] -> int[.], int[3] -> int[.] }
 *    The reason why we hide the intersection related stuff within this module
 *    rather than a separate one, is, that only this measure allows us to use
 *    a representation for overloaded functions, which can be dispatched more
 *    efficiently. (I hope this was a wise one 8-))
 *    However, as a consequence, this module must provide several more elaborate
 *    functions for handling function types such as TYMakeOverloadedFunType
 *    (see below) or TYDispatchFunType (see below).
 *    Another consequence of this design decision is the need to keep some
 *    function information attached to the individual elements of the intersection types
 *    Therefore,
 *      -  TYMakeFunType            obtains a    node* fun_info         argument, and
 *      -  TYMakeOverloadedFunType  obtains a function pointer argument that merges two
 *                                  such fun_info nodes into a single one.
 *
 *
 * 3) Product types
 * ==================
 *
 *    Although product types are proper types (from a type theoretic POV),
 *    at the time being, the are used internaly only. They serve as vehicle
 *    for handling multiple return values of functions correctly.
 *    To ease the overall implementation, per convention, ALL functions
 *    do return product types, (even void ones!). Nevertheless, the wrapping
 *    and unwrapping of product types does not exist in any stage of the compilation
 *    process. For example, a function
 *
 *    int[*], int[*] foo( int[*] x)     with    foo::{ int[*] -> (int[*], int[*]) }
 *
 *    can be used within an assignment
 *
 *    a, b = foo( [2]);
 *
 *    where a and b are of type int[*]  !!
 */

/*
 * Besides the scalar types, which are of a purely technical nature (see comments
 * above), this module provides another sort of types which do not have a type
 * theoretic correspondence, i.e., TYPE VARIABLES.
 *
 * Again, guided by pragmatical considerations, only a very restricted form
 * of type variables is introduced here, namely type variables for array types
 * only! They are called
 *
 * 1) Alpha types
 * ==============
 *
 */

/*
 * This section is on purely technical matters:
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

/*
 * definitions required for the entire interface:
 */

typedef struct NTYPE ntype;

#include "types.h"
#include "free.h"

#include "shape.h"
#include "ssi.h"

/*
 * basic stuff which should only be used if essential from a performance
 * point of view, as it unleashes part of the implementation...
 */

typedef enum {
#define TCITypeConstr(a) a
#include "type_constructor_info.mac"
} typeconstr;

typeconstr TYGetConstr (ntype *type);

/*
 * Scalar Types: Simple / User / Symbol
 */
extern ntype *TYMakeSimpleType (simpletype base);
extern ntype *TYMakeUserType (usertype base);
extern ntype *TYMakeSymbType (char *name, char *mod);

extern simpletype TYGetSimpleType (ntype *simple);
extern char *TYGetName (ntype *symb);
extern char *TYGetMod (ntype *symb);

/*
 * Array Types: AKS / AKD / AUDGZ / AUD
 */
extern ntype *TYMakeAKS (ntype *scalar, shape *shp);
extern ntype *TYMakeAKD (ntype *scalar, int dots, shape *shp);
extern ntype *TYMakeAUDGZ (ntype *scalar);
extern ntype *TYMakeAUD (ntype *scalar);

extern ntype *TYSetScalar (ntype *array, ntype *scalar);

extern int TYGetDim (ntype *array);
extern shape *TYGetShape (ntype *array);
extern ntype *TYGetScalar (ntype *array);

/*
 * Union Types: (not used at the time being!)
 */
extern ntype *TYMakeUnionType (ntype *t1, ntype *t2);

/*
 * Product Types:
 */
extern ntype *TYMakeProductType (int size, ...);
extern ntype *TYMakeEmptyProductType (int size);
extern ntype *TYSetProductMember (ntype *prod, int pos, ntype *member);

extern int TYGetProductSize (ntype *prod);
extern ntype *TYGetProductMember (ntype *prod, int pos);

/*
 * Function Types:
 */
extern ntype *TYMakeFunType (ntype *arg, ntype *res, node *fun_info);
extern ntype *TYMakeOverloadedFunType (ntype *fun1, ntype *fun2);

typedef struct dft {
    ntype *type;
    node *def;
    node *deriveable;
    int num_partials;
    node **partials;
    int num_deriveable_partials;
    node **deriveable_partials;
} DFT_res;

extern DFT_res *TYDispatchFunType (ntype *fun, ntype *args);

extern DFT_res *TYMakeDFT_res (ntype *type, int max_funs);
extern void TYFreeDFT_res (DFT_res *res);
extern char *TYDFT_res2DebugString (DFT_res *dft);

/*
 * Type Variables:
 */
extern ntype *TYMakeAlphaType (ntype *maxtype);
extern tvar *TYGetAlpha (ntype *type);

/*
 * Some predicates for inspecting types:
 */
extern bool TYIsSimple (ntype *);
extern bool TYIsUser (ntype *);
extern bool TYIsSymb (ntype *);
extern bool TYIsScalar (ntype *);

extern bool TYIsAlpha (ntype *);
extern bool TYIsFixedAlpha (ntype *);
extern bool TYIsNonFixedAlpha (ntype *);
extern bool TYIsAKS (ntype *);
extern bool TYIsAKD (ntype *);
extern bool TYIsAUDGZ (ntype *);
extern bool TYIsAUD (ntype *);
extern bool TYIsArray (ntype *);
extern bool TYIsArrayOrAlpha (ntype *);
extern bool TYIsArrayOrFixedAlpha (ntype *);

extern bool TYIsUnion (ntype *);
extern bool TYIsProd (ntype *);
extern bool TYIsFun (ntype *);

extern bool TYIsAKSSymb (ntype *);
extern bool TYIsProdOfArray (ntype *);
extern bool TYIsProdOfArrayOrFixedAlpha (ntype *);

extern int TYCountNonFixedAlpha (ntype *);
extern int TYCountNoMinAlpha (ntype *);

/*
 * Type-Comparison
 */
typedef enum {
    TY_eq,  /* types are identical */
    TY_lt,  /* first type is subtype of second one */
    TY_gt,  /* first type is supertype of second one */
    TY_hcs, /* types are unrelated but do have a common supertype */
    TY_dis  /* types are disjoint */
} CT_res;

extern CT_res TYCmpTypes (ntype *t1, ntype *t2);
extern bool TYLeTypes (ntype *t1, ntype *t2);
extern bool TYEqTypes (ntype *t1, ntype *t2);

/*
 * Computing types from other types
 */

extern ntype *TYLubOfTypes (ntype *t1, ntype *t2);
extern ntype *TYEliminateAlpha (ntype *t1);
extern ntype *TYFixAndEliminateAlpha (ntype *t1);

/*
 * General Type handling functions
 */
extern void TYFreeTypeConstructor (ntype *type);
extern void TYFreeType (ntype *type);

extern ntype *TYCopyType (ntype *type);
extern ntype *TYCopyTypeConstructor (ntype *type);
extern ntype *TYCopyFixedType (ntype *type);
extern ntype *TYDeriveSubtype (ntype *type);
extern char *TYType2String (ntype *new, bool multiline, int offset);
extern char *TYType2DebugString (ntype *new, bool multiline, int offset);
extern ntype *TYNestTypes (ntype *outer, ntype *inner);

/*
 * Functions for converting from old to new types and back
 */
typedef enum { TY_symb, TY_user } type_conversion_flag;

extern ntype *TYOldType2Type (types *old);
extern types *TYType2OldType (ntype *new);

#endif /* _new_types_h */
