/*
 * $Log$
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
 * The central idea of the ntype is that all types are represented by recursively
 * nested type-constructors with varying arity.
 * All "scalar types", e.g., pre-defined types or user-defined types, are
 * represented by typeconstructors of arity 0, e.g. TC_simple or TC_user.
 * Further information concerning type constructors can be found in
 *     type_constructor_info.mac    .
 *
 * For avoiding un-intended pointer sharing and for avoiding memory leaks
 * we establish the following rules:
 * - whenever a ntype is given as argument, neither the pointer to it nor
 *   any potential sub structure will be copied in any data structure
 *   that serves as a result!
 *   THE SOLE EXCEPTION OF THIS RULE are the "MakeXYZ" functions for type
 *   constructors XYZ, which take other ntype nodes as son nodes. These
 *   will use the argument pointers directly. Nevertheless, any other
 *   pointer arguments given to these functions, e.g. strings, will NOT
 *   be used directly, but the structures behind them will be copied instead!
 * - The only functions for freeing an ntype constructor are
 *     TYFreeTypeConstructor  for freeing the topmost constructor only, and
 *     TYFreeType             for freeing the entire type.
 * - If the result is a ntype structure, it has been dynamically allocated!
 *
 */

#ifdef SELF
#undef SELF
#else
typedef void ntype;
#endif

#include "types.h"
#include "free.h"

#include "shape.h"

/*
 * Scalar Types: Simple / User / Symbol
 */
extern ntype *TYMakeSimpleType (simpletype base);
extern ntype *TYMakeUserType (usertype base);
extern ntype *TYMakeSymbType (char *name, char *mod);

extern char *TYGetName (ntype *symb);
extern char *TYGetMod (ntype *symb);

/*
 * Array Types: AKS / AKD / AUD
 */
extern ntype *TYMakeAKS (ntype *scalar, shape *shp);
extern ntype *TYMakeAKD (ntype *scalar, int dots, shape *shp);
extern ntype *TYMakeAUD (ntype *scalar);

extern int TYGetDim (ntype *array);
extern shape *TYGetShape (ntype *array);
extern ntype *TYGetScalar (ntype *array);

/*
 * Union Types: Union
 */
extern ntype *TYMakeUnion (ntype *t1, ntype *t2);

#if 0
/*
 * Types for handling functions: Fun / Prod
 */
extern ntype *  TYMakeFunType( ntype *arg, ntype *res);
extern ntype *  TYMakeProdType( ntype *res);
extern void     TYInsertIntoProdType( ntype *prod, ntype *res);
#endif

/*
 * Some predicates for matching types:
 */
extern bool TYIsSimple (ntype *);
extern bool TYIsUser (ntype *);
extern bool TYIsSymb (ntype *);
extern bool TYIsScalar (ntype *);
extern bool TYIsAKS (ntype *);
extern bool TYIsAKD (ntype *);
extern bool TYIsAUD (ntype *);
extern bool TYIsUnion (ntype *);
extern bool TYIsProd (ntype *);
extern bool TYIsFun (ntype *);

extern bool TYIsAKSSymb (ntype *);

/*
 * Type-Comparison
 */
typedef enum {
    TY_eq,    /* types are identical */
    TY_lt,    /* first type is subtype of second one */
    TY_gt,    /* first type is supertype of second one */
    TY_undec, /* although both types are compatible, i.e., they have a common
               * instance, they are neither CT_lt, CT_eq, nor CT_gt. This may
               * only happen for product or function types!!
               */
    TY_unrel  /* types are unrelated */
} CT_res;

extern CT_res TYCmpTypes (ntype *t1, ntype *t2);

/*
 * General Type handling functions
 */
extern void TYFreeTypeConstructor (ntype *type);
extern void TYFreeType (ntype *type);

extern ntype *TYCopyType (ntype *type);
extern char *TYType2String (ntype *new);
extern char *TYType2DebugString (ntype *new);
extern ntype *TYNestTypes (ntype *outer, ntype *inner);

/*
 * Functions for converting from old to new types and back
 */
typedef enum { TY_symb, TY_user } type_conversion_flag;

extern ntype *TYOldType2Type (types *old, type_conversion_flag flag);
extern types *TYType2OldType (ntype *new);

#endif /* _new_types_h */
