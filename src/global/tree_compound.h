/*
 *
 * $Log$
 * Revision 1.1  1995/09/27 15:13:12  cg
 * Initial revision
 *
 *
 *
 */

/*============================================================================

How to use this file ?

While tree_basic.h contains the basic structure of the new virtual syntax
tree, this file is dedicated to more elaborate macros and function
declarations.

These are sorted towards the structures they are used for similar to
tree_basic.h. Please observe that all macros in this file and all functions
in tree_compound.c should exclusively use the new virtual syntax tree.
They must not (!!) contain direct accesses to the underlying data structure.

============================================================================*/

#ifndef _sac_tree_compound_h

#define _sac_tree_compound_h

#include "types.h"
#include "tree_basic.h"

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* general function declarations                                            */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* macros for module name access                                            */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#define MOD_NAME_CON "__"
#define MOD(a) (NULL == a) ? "" : a
#define MOD_CON(a) (NULL == a) ? "" : MOD_NAME_CON
#define MOD_NAME(a) MOD (a), MOD_CON (a)

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*  compound macros for non-node structures                                 */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/*
 *
 *  macro name    : CMP_TYPE_USER
 *  arg types     : 1) types*
 *                  2) types*
 *  result type   : int
 *  description   : compares two user-defined types (name and module)
 *                  result: 1 - equal, 0 - not equal
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       :
 *
 */

#define CMP_TYPE_USER(a, b)                                                              \
                                                                                         \
    ((NULL == TYPES_MOD (a))                                                             \
       ? (!strcmp (TYPES_NAME (a), TYPES_NAME (b)) && (NULL == TYPES_MOD (b)))           \
       : ((NULL == TYPES_MOD (b)) ? 0                                                    \
                                  : ((!strcmp (TYPES_NAME (a), TYPES_NAME (b)))          \
                                     && (!strcmp (TYPES_MOD (a), TYPES_MOD (b))))))

/*
 *
 *  macro name    : CMP_MOD(a,b)
 *  arg types     : 1) char*
 *                  2) char*
 *  result type   : int
 *  description   : compares two module names (name maybe NULL)
 *                  result: 1 - equal, 0 - not equal
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       :
 *
 */

#define CMP_MOD(a, b) ((NULL == a) ? (NULL == b) : ((NULL == b) ? 0 : (!strcmp (a, b))))

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*  compound macros for node structures                                     */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/***
 ***  N_modul :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_moddec :
 ***/

/*
 *  compound access macros
 */

#define MODDEC_ITYPES(n) EXPLIST_ITYPES (MODDEC_OWN (n))
#define MODDEC_ETYPES(n) EXPLIST_ETYPES (MODDEC_OWN (n))
#define MODDEC_OBJS(n) EXPLIST_OBJS (MODDEC_OWN (n))
#define MODDEC_FUNS(n) EXPLIST_FUNS (MODDEC_OWN (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_classdec :
 ***/

/*
 *  compound access macros
 */

#define CLASSDEC_ITYPES(n) EXPLIST_ITYPES (CLASSDEC_OWN (n))
#define CLASSDEC_ETYPES(n) EXPLIST_ETYPES (CLASSDEC_OWN (n))
#define CLASSDEC_OBJS(n) EXPLIST_OBJS (CLASSDEC_OWN (n))
#define CLASSDEC_FUNS(n) EXPLIST_FUNS (CLASSDEC_OWN (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_sib :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_implist :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_explist :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_typedef :
 ***/

/*
 *  compound access macros
 */

#define TYPEDEF_BASETYPE(n) TYPES_BASETYPE (TYPEDEF_TYPE (n))
#define TYPEDEF_DIM(n) TYPES_DIM (TYPEDEF_TYPE (n))
#define TYPEDEF_SHAPE(n, x) TYPES_SHAPE (TYPEDEF_TYPE (n), x)
#define TYPEDEF_TNAME(n) TYPES_NAME (TYPEDEF_TYPE (n))
#define TYPEDEF_TMOD(n) TYPES_MOD (TYPEDEF_TYPE (n))

/*
 *
 *  macro name    : CMP_TYPEDEF(a,b)
 *  arg types     : 1) node*  (N_typedef)
 *                  2) node*  (N_typedef)
 *  result type   : int
 *  description   : compares two typedef nodes (name and module)
 *                  result: 1 - equal, 0 - not equal
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       :
 *
 */

#define CMP_TYPEDEF(a, b)                                                                \
                                                                                         \
    ((NULL == TYPEDEF_MOD (a))                                                           \
       ? (!strcmp (TYPEDEF_NAME (a), TYPEDEF_NAME (b)) && (NULL == TYPEDEF_MOD (b)))     \
       : ((NULL == TYPEDEF_MOD (b))                                                      \
            ? 0                                                                          \
            : ((!strcmp (TYPEDEF_NAME (a), TYPEDEF_NAME (b)))                            \
               && (!strcmp (TYPEDEF_MOD (a), TYPEDEF_MOD (b))))))

/*--------------------------------------------------------------------------*/

/***
 ***  N_objdef :
 ***/

/*
 *  compound access macros
 */

#define OBJDEF_BASETYPE(n) TYPES_BASETYPE (OBJDEF_TYPE (n))
#define OBJDEF_DIM(n) TYPES_DIM (OBJDEF_TYPE (n))
#define OBJDEF_SHAPE(n, x) TYPES_SHAPE (OBJDEF_TYPE (n), x)
#define OBJDEF_TNAME(n) TYPES_NAME (OBJDEF_TYPE (n))
#define OBJDEF_TMOD(n) TYPES_MOD (OBJDEF_TYPE (n))

/*
 *
 *  macro name    : CMP_OBJDEF(a,b)
 *  arg types     : 1) node*  (N_objdef)
 *                  2) node*  (N_objdef)
 *  result type   : int
 *  description   : compares two objdef nodes (name and module)
 *                  result: 1 - equal, 0 - not equal
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       :
 *
 */

#define CMP_OBJDEF(a, b)                                                                 \
                                                                                         \
    ((NULL == OBJDEF_MOD (a))                                                            \
       ? (!strcmp (OBJDEF_NAME (a), OBJDEF_NAME (b)) && (NULL == OBJDEF_MOD (b)))        \
       : ((NULL == OBJDEF_MOD (b)) ? 0                                                   \
                                   : ((!strcmp (OBJDEF_NAME (a), OBJDEF_NAME (b)))       \
                                      && (!strcmp (OBJDEF_MOD (a), OBJDEF_MOD (b))))))

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
 ***/

/*
 *  compound access macros
 */

#define FUNDEF_BASETYPE(n) TYPES_BASETYPE (FUNDEF_TYPES (n))
#define FUNDEF_DIM(n) TYPES_DIM (FUNDEF_TYPES (n))
#define FUNDEF_SHAPE(n, x) TYPES_SHAPE (FUNDEF_TYPES (n), x)
#define FUNDEF_TNAME(n) TYPES_NAME (FUNDEF_TYPES (n))
#define FUNDEF_TMOD(n) TYPES_MOD (FUNDEF_TYPES (n))

/*
 *  function declarations
 */

extern int CmpDomain (node *args1, node *args2);
extern void ObjList2ArgList (node *objdef);

/*
 *
 *  macro name    : CMP_FUN_ID(a,b)
 *  arg types     : 1) node*  (N_objdef)
 *                  2) node*  (N_objdef)
 *  result type   : int
 *  description   : compares two fundef nodes (name and module only)
 *                  result: 1 - equal, 0 - not equal
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       :
 *
 */

#define CMP_FUN_ID(a, b)                                                                 \
                                                                                         \
    ((NULL == FUNDEF_MOD (a))                                                            \
       ? (!strcmp (FUNDEF_NAME (a), FUNDEF_NAME (b)) && (NULL == FUNDEF_MOD (b)))        \
       : ((NULL == FUNDEF_MOD (b)) ? 0                                                   \
                                   : ((!strcmp (FUNDEF_NAME (a), FUNDEF_NAME (b)))       \
                                      && (!strcmp (FUNDEF_MOD (a), FUNDEF_MOD (b))))))

/*
 *
 *  macro name    : CMP_FUNDEF(a,b)
 *  arg types     : 1) node*  (N_objdef)
 *                  2) node*  (N_objdef)
 *  result type   : int
 *  description   : compares two fundef nodes (name, module, and domain)
 *                  result: 1 - equal, 0 - not equal
 *  global vars   : ---
 *  funs          : CmpDomain
 *
 *  remarks       :
 *
 */

#define CMP_FUNDEF(a, b)                                                                 \
                                                                                         \
    ((CMP_FUN_ID (a, b)) ? CmpDomain (FUNDEF_ARGS (a), FUNDEF_ARGS (b)) : 0)

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***/

/*
 *  compound access macros
 */

#define ARG_BASETYPE(n) TYPES_BASETYPE (ARG_TYPE (n))
#define ARG_DIM(n) TYPES_DIM (ARG_TYPE (n))
#define ARG_SHAPE(n, x) TYPES_SHAPE (ARG_TYPE (n), x)
#define ARG_TNAME(n) TYPES_NAME (ARG_TYPE (n))
#define ARG_TMOD(n) TYPES_MOD (ARG_TYPE (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_block :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :
 ***/

/*
 *  compound access macros
 */

#define VARDEC_BASETYPE(n) TYPES_BASETYPE (VARDEC_TYPE (n))
#define VARDEC_DIM(n) TYPES_DIM (VARDEC_TYPE (n))
#define VARDEC_SHAPE(n, x) TYPES_SHAPE (VARDEC_TYPE (n), x)
#define VARDEC_TNAME(n) TYPES_NAME (VARDEC_TYPE (n))
#define VARDEC_TMOD(n) TYPES_MOD (VARDEC_TYPE (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_assign :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_cast :
 ***/

/*
 *  compound access macros
 */

#define CAST_BASETYPE(n) TYPES_BASETYPE (CAST_TYPE (n))
#define CAST_DIM(n) TYPES_DIM (CAST_TYPE (n))
#define CAST_SHAPE(n, x) TYPES_SHAPE (CAST_TYPE (n), x)
#define CAST_TNAME(n) TYPES_NAME (CAST_TYPE (n))
#define CAST_TMOD(n) TYPES_MOD (CAST_TYPE (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_return :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_cond :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_while :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_ap :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_with :
 ***/

/*
 *  compound access macros
 */

#define WITH_LEFT(n) (GEN_LEFT (WITH_GEN (n)))
#define WITH_RIGHT(n) (GEN_RIGHT (WITH_GEN (n)))
#define WITH_ID(n) (GEN_ID (WITH_GEN (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_generator :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_genarray :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_modarray :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldprf :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldfun :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :
 ***/

/*
 *  compound access macros
 */

#define ARRAY_BASETYPE(n) TYPES_BASETYPE (ARRAY_TYPE (n))
#define ARRAY_DIM(n) TYPES_DIM (ARRAY_TYPE (n))
#define ARRAY_SHAPE(n, x) TYPES_SHAPE (ARRAY_TYPE (n), x)
#define ARRAY_TNAME(n) TYPES_NAME (ARRAY_TYPE (n))
#define ARRAY_TMOD(n) TYPES_MOD (ARRAY_TYPE (n))

/*
 *  function declarations
 */

node *Shape2Array (shapes *shp);

/*--------------------------------------------------------------------------*/

/***
 ***  N_vinfo :
 ***/

/*
 *  compound access macros
 */

#define VINFO_DIM(n) SHAPES_DIM (VINFO_SHP (n))
#define VINFO_SELEMS(n) SHAPES_SELEMS (VINFO_SHP (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :
 ***/

/*
 *  compound access macros
 */

#define PRF_ARG1(n) (EXPRS_EXPR (PRF_ARGS (n)))
#define PRF_ARG2(n) (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (n))))
#define PRF_ARG3(n) (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (PRF_ARGS (n)))))

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/*
 *
 *  macro name    :
 *  arg types     :
 *  result type   :
 *  description   :
 *  global vars   :
 *  funs          :
 *
 *  remarks       :
 *
 */

#endif /* _sac_tree_compound_h */
