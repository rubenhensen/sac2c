#ifndef _SAC_NEW_TYPES_H_
#define _SAC_NEW_TYPES_H_

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
 *      as all other types are, they (for themselves) do NOT constitute legal
 *      types in SAC! They are used as attributes of array types only!!!
 *
 *    At the time being, the following types are legal scalar types:
 *      a) simple types      (these are the built-in types known from C proper)
 *      b) user types        (new types defined by means of  typedef's)
 *      c) symbolic types    (types of unknown structure; basically (void *))
 *
 *    Among array types with identical scalar type, there exists a type
 *    hierarchy. It reflects different levels of shape restrictions:
 *      a) AUD array types   (arrays of unknown dimensionality)
 *      b) AUDGZ array types (arrays of unknown dimensionality greater than 0)
 *      c) AKD array types   (arrays of a fixed (known) dimensionality)
 *      d) AKS array types   (arrays of a fixed (known) shape)
 *    This hierarchy is formalized by a subtype relation which in fact is
 *    reflected by the type comparison function TYCmpTypes (see down below).
      Wrt. array types, it returns one of the following values:
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
 *    function information attached to the individual elements of the
 *    intersection types. Therefore,
 *      -  TYMakeFunType            obtains a   node* fun_info   argument, and
 *      -  TYMakeOverloadedFunType  obtains a function pointer argument that
 *                                  merges two such fun_info nodes into a single
 *                                  one.
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
 *    and unwrapping of product types does not exist in any stage of the
 *    compilation process. For example, a function
 *
 *    int[*], int[*] foo( int[*] x)   with   foo::{ int[*] -> (int[*], int[*]) }
 *
 *    can be used within an assignment
 *
 *    a, b = foo( [2]);
 *
 *    where a and b are of type int[*]  !!
 *
 * 4) Polymorphic types
 * =====================
 *
 *    These are type variables that can be specified by the user in order to define
 *    polymorphic functions. However, their usage is restricted to function definitions
 *    AND they have to be scalar types....  ( comment is to be improved....)
 */

/*
 * Besides the scalar types, which are of a purely technical nature (see
 * comments above), this module provides another sort of types which do not
 * have a type theoretic correspondence, i.e., TYPE VARIABLES.
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
 *     TYfreeTypeConstructor  for freeing the topmost constructor only, and
 *     TYfreeType             for freeing the entire type.
 *
 * - If the result is an ntype structure, it has been freshly allocated!
 *
 *
 */

/*
 * definitions required for the entire interface:
 */

#include "types.h"

typeconstr TYgetConstr (ntype *type);
mutcScope TYgetMutcScope (ntype *type);
mutcUsage TYgetMutcUsage (ntype *type);
/*
 * Scalar Types: Simple / User / Symbol
 */
extern ntype *TYmakeSimpleType (simpletype base);
extern ntype *TYmakeHiddenSimpleType (usertype udt);
extern ntype *TYmakeUserType (usertype base);
extern ntype *TYmakeSymbType (char *name, namespace_t *mod);

extern ntype *TYsetSimpleType (ntype *simple, simpletype base);
extern ntype *TYsetHiddenUserType (ntype *simple, usertype udt);

extern ntype *TYsetMutcUsage (ntype *type, mutcUsage usage);
extern ntype *TYsetMutcScope (ntype *type, mutcScope scope);
extern ntype *TYsetUnique (ntype *type, bool val);
extern ntype *TYsetDistributed (ntype *type, distmem_dis val);

extern simpletype TYgetSimpleType (ntype *simple);
extern usertype TYgetHiddenUserType (ntype *simple);
extern usertype TYgetUserType (ntype *user);
extern char *TYgetName (ntype *symb);
extern const namespace_t *TYgetNamespace (ntype *symb);
extern ntype *TYsetNamespace (ntype *symb, namespace_t *ns);

/*
 * Array Types: AKS / AKD / AUDGZ / AUD
 */
extern ntype *TYmakeAKV (ntype *scalar, constant *val);
extern ntype *TYmakeAKS (ntype *scalar, shape *shp);
extern ntype *TYmakeAKD (ntype *scalar, size_t dots, shape *shp);
extern ntype *TYmakeAUDGZ (ntype *scalar);
extern ntype *TYmakeAUD (ntype *scalar);

extern ntype *TYsetScalar (ntype *array, ntype *scalar);

extern int TYgetDim (ntype *array);
extern shape *TYgetShape (ntype *array);
extern constant *TYgetValue (ntype *array);
extern ntype *TYgetScalar (ntype *array);

/*
 * Union Types: (not used at the time being!)
 */
extern ntype *TYmakeUnionType (ntype *t1, ntype *t2);

/*
 * Product Types:
 */
extern ntype *TYmakeProductType (size_t size, ...);
extern ntype *TYmakeEmptyProductType (size_t size);
extern ntype *TYsetProductMember (ntype *prod, size_t pos, ntype *member);

extern size_t TYgetProductSize (ntype *prod);
extern ntype *TYgetProductMember (ntype *prod, size_t pos);

/*
 * Bottom Types:
 */
extern ntype *TYmakeBottomType (char *err_msg);
extern void TYextendBottomError (ntype *type, char *err_msg);
extern char *TYgetBottomError (ntype *type);

/*
 * Polymorphic Types:
 */
extern ntype *TYmakePolyType (char *name);
extern char *TYgetPolyName (ntype *poly);

extern ntype *TYmakePolyUserType (char *outer, char *inner, char *shape, bool denest,
                                  bool renest);
extern char *TYgetPolyUserOuter (ntype *poly);
extern char *TYgetPolyUserInner (ntype *poly);
extern char *TYgetPolyUserShape (ntype *poly);
extern bool TYgetPolyUserDeNest (ntype *poly);
extern bool TYgetPolyUserReNest (ntype *poly);

/*
 * Function Types:
 */
extern ntype *TYmakeFunType (ntype *arg, ntype *res, node *fun_info);
extern ntype *TYmakeOverloadedFunType (ntype *fun1, ntype *fun2);

extern ntype *TYmapFunctionInstances (ntype *funtype, node *(*mapfun) (node *, info *),
                                      info *info);

extern void *TYfoldFunctionInstances (ntype *funtype, void *(*foldfun) (node *, void *),
                                      void *initial);

extern size_t TYgetArity (ntype *fun);

extern dft_res *TYdispatchFunType (ntype *fun, ntype *args);

extern dft_res *TYmakeDft_res (ntype *type, int max_funs);
extern dft_res *TYfreeDft_res (dft_res *res);
extern char *TYdft_res2DebugString (dft_res *dft);

/*
 * Type Variables:
 */
extern ntype *TYmakeAlphaType (ntype *maxtype);
extern tvar *TYgetAlpha (ntype *type);
extern bool TYcontainsAlpha (ntype *type);

/*
 * Some predicates for inspecting types:
 */
extern bool TYisSimple (ntype *);
extern bool TYisUser (ntype *);
extern bool TYisPoly (ntype *);
extern bool TYisPolyUser (ntype *);
extern bool TYisSymb (ntype *);
extern bool TYisRecord (ntype *);
extern bool TYisScalar (ntype *);

extern bool TYisBottom (ntype *);

extern bool TYisAlpha (ntype *);
extern bool TYisFixedAlpha (ntype *);
extern bool TYisNonFixedAlpha (ntype *);
extern bool TYisAKV (ntype *);
extern bool TYisAKS (ntype *);
extern bool TYisAKD (ntype *);
extern bool TYisAUDGZ (ntype *);
extern bool TYisAUD (ntype *);
extern bool TYisArray (ntype *);
extern bool TYisArrayOrAlpha (ntype *);
extern bool TYisArrayOrFixedAlpha (ntype *);

extern bool TYisUnion (ntype *);
extern bool TYisProd (ntype *);
extern bool TYisFun (ntype *);

extern bool TYisAKSSymb (ntype *);
extern bool TYisAKSUdt (ntype *);
extern bool TYisProdOfArray (ntype *);
extern bool TYisProdOfArrayOrFixedAlpha (ntype *);
extern bool TYisProdOfAKV (ntype *);
extern bool TYisProdOfAKVafter (ntype *, size_t);
extern bool TYisProdContainingAKV (ntype *);

extern bool TYgetUnique (ntype *type);
extern distmem_dis TYgetDistributed (ntype *type);

extern int TYcountNonFixedAlpha (ntype *);
extern int TYcountNoMinAlpha (ntype *);

extern ntype *TYgetBottom (ntype *);

extern ct_res TYcmpTypes (ntype *t1, ntype *t2);
extern bool TYleTypes (ntype *t1, ntype *t2);
extern bool TYeqTypes (ntype *t1, ntype *t2);

/*
 * Computing types from other types
 */

extern ntype *TYlubOfTypes (ntype *t1, ntype *t2);
extern ntype *TYeliminateAlpha (ntype *t1);
extern ntype *TYfixAndEliminateAlpha (ntype *t1);
extern ntype *TYliftBottomFixAndEliminateAlpha (ntype *t1);
extern ntype *TYeliminateUser (ntype *t1);
extern ntype *TYeliminateAKV (ntype *t1);

/*
 * General Type handling functions
 */
extern ntype *TYfreeTypeConstructor (ntype *type);
extern ntype *TYfreeType (ntype *type);
extern void TYtouchTypeConstructor (ntype *type, info *arg_info);
extern void TYtouchType (ntype *type, info *arg_info);

extern ntype *TYcopyType (ntype *type);
extern ntype *TYcopyTypeConstructor (ntype *type);
extern ntype *TYcopyFixedType (ntype *type);
extern ntype *TYderiveSubtype (ntype *type);
extern char *TYScalarType2String (ntype *mnew);
extern char *TYtype2String (ntype *mnew, bool multiline, size_t offset);
extern char *TYtype2DebugString (ntype *mnew, bool multiline, size_t offset);
extern char *TYargs2FunTypeString (node *args, ntype *rettype);
extern ntype *TYnestTypes (ntype *outer, ntype *inner);
extern ntype *TYdeNestTypeFromInner (ntype *nested, ntype *inner);
extern ntype *TYdeNestTypeFromOuter (ntype *nested, ntype *outer);

/*
 * Functions for converting types into SAC code for wrapper functions
 */
extern ntype *TYsplitWrapperType (ntype *type, int *pathes_remaining);
extern ntype *TYgetWrapperRetType (ntype *type);
extern node *TYcorrectWrapperArgTypes (node *args, ntype *type);
extern node *TYcreateWrapperCode (node *fundef, node *vardecs, node **new_vardecs);

/*
 * Serialization and Deserialization support
 */

extern void TYserializeType (FILE *file, ntype *type);
extern ntype *TYdeserializeType (int _con, ...);
#if IS_CYGWIN
extern ntype *TYdeserializeTypeVa (int _con, va_list Argp);
#endif

#endif /* _SAC_NEW_TYPES_H_ */
