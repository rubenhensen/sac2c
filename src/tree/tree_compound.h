/*
 *
 * $Log$
 * Revision 3.2  2000/11/29 13:13:29  dkr
 * macros AP_ARG? added
 *
 * Revision 3.1  2000/11/20 18:03:37  sacbase
 * new release made
 *
 * Revision 1.47  2000/10/31 23:31:47  dkr
 * signature of Type2Shpseg, Array2Shpseg modified
 *
 * Revision 1.46  2000/10/27 00:06:00  dkr
 * Type2Shpseg and Type2Exprs added,
 * some code brushing done.
 *
 * Revision 1.45  2000/10/26 13:57:45  dkr
 * CopyShpseg replaced by DupShpseg (DupTree.[ch])
 *
 * Revision 1.44  2000/10/24 14:29:23  dkr
 * some append functions added
 *
 * Revision 1.43  2000/10/24 10:06:08  dkr
 * GetBasetypeSize() added
 *
 * Revision 1.42  2000/10/24 09:43:59  dkr
 * GetSimpletype renamed into GetBasetype
 *
 * Revision 1.41  2000/10/23 18:04:22  dkr
 * syntax error in ICM_EXPRS1 corrected :-(
 *
 * Revision 1.39  2000/10/23 16:38:43  dkr
 * ICM_ARG5, ICM_ARG6 added
 *
 * Revision 1.38  2000/10/20 15:38:40  dkr
 * some functions on types added
 *
 * Revision 1.37  2000/10/17 13:02:13  dkr
 * macro EXPRS_LENGTH added
 *
 * Revision 1.36  2000/10/16 16:01:03  dkr
 * VARDEC_OR_ARG_OBJDEF added
 *
 * Revision 1.35  2000/10/12 18:02:24  dkr
 * return value type of Is...() functions is bool, now
 *
 * Revision 1.34  2000/10/12 15:44:22  dkr
 * WLGRID_CBLOCK_INSTR and WLGRIDVAR_CBLOCK_INSTR added
 *
 * Revision 1.33  2000/10/09 16:33:50  dkr
 * NWITH2_CEXPR, NWITH2_CBLOCK added
 *
 * Revision 1.32  2000/10/02 09:02:05  sbs
 * ID_OR_CAST_TYPE added.
 *
 * Revision 1.31  2000/08/05 13:02:15  dkr
 * some macros for N_Nwith added
 *
 * Revision 1.30  2000/07/21 14:17:53  mab
 * added EqualShpseg
 *
 * Revision 1.29  2000/07/14 09:37:52  dkr
 * CopyNodelist renamed into DupNodelist and moved to DupTree.[ch]
 *
 * Revision 1.28  2000/07/12 15:19:51  dkr
 * function SearchDecl moved from Inline.h to tree_compound.h
 *
 * Revision 1.27  2000/07/11 14:50:40  dkr
 * function IsHidden added
 *
 * Revision 1.26  2000/07/07 15:44:57  bs
 * The following compound macros moved from tree_basic to tree_compound:
 * INFO_WLAA_ARRAYSHP, INFO_WLAA_INDEXDIM, INFO_WLAA_ARRAYDIM
 * INFO_TSI_ARRAYSHP, INFO_TSI_INDEXDIM, INFO_TSI_ARRAYDIM
 *
 * Revision 1.25  2000/07/05 15:18:02  mab
 * added Shpseg2Array
 *
 * Revision 1.24  2000/07/05 12:23:56  dkr
 * comment corrected
 *
 * Revision 1.23  2000/06/30 13:48:44  mab
 * added Array2Shpseg and DiffShpseg
 *
 * Revision 1.22  2000/06/29 16:07:05  mab
 * added Array2Shpseg
 *
 * Revision 1.21  2000/06/23 14:16:59  dkr
 * macros for old with-loop removed
 *
 * Revision 1.20  2000/06/23 13:04:31  mab
 * renamed *_VNAME to *_VARDEC_NAME, *_VNEXT to *_VARDEC_NEXT
 *
 * Revision 1.19  2000/06/21 15:00:49  mab
 * added macros *_PADDED for ARG and VARDEC
 * added INFO_APT_EXPRESSION_PADDED
 *
 * Revision 1.18  2000/06/13 14:18:11  dkr
 * macros L_NWITH_OR_NWITH2_... added
 *
 * Revision 1.17  2000/05/30 14:31:44  dkr
 * redundant macro APPEND_VARDECS removed
 *
 * Revision 1.16  2000/05/30 14:06:18  dkr
 * some helper functions moved from compile.c to tree_compound.c
 *
 * Revision 1.15  2000/05/25 22:54:16  dkr
 * some macros for N_Nwith, N_Nwith2, N_WLgrid and N_WLgridVar added
 *
 * Revision 1.14  2000/05/24 18:57:50  dkr
 * macros for old with-loop separated
 * macro NCODE_CBLOCK_INSTR added
 *
 * Revision 1.13  2000/03/31 14:09:47  dkr
 * NWITH_OR_NWITH2_... macros added
 *
 * Revision 1.12  2000/03/23 21:39:26  dkr
 * macros VARDEC_OR_ARG_ATTRIB and VARDEC_OR_ARG_STATUS added
 *
 * Revision 1.11  2000/03/21 13:12:56  jhs
 * Added macros: [L_]MT_OR_ST_xxx
 *
 * Revision 1.10  2000/03/15 15:59:53  dkr
 * SET_VARDEC_OR_ARG_ACTCHN renamed to L_VARDEC_OR_ARG_ACTCHN, ...
 *
 * [...]
 *
 * Revision 1.1  2000/01/21 15:38:29  dkr
 * Initial revision
 *
 * Revision 2.25  1999/12/01 15:20:55  dkr
 * macro VARDEC_OR_ARG_SHPSEG added
 *
 * [...]
 *
 * Revision 2.1  1999/02/23 12:40:11  sacbase
 * new release made
 *
 * Revision 1.79  1999/02/06 12:50:27  srs
 * declared four functions to handle Nodelist
 *
 * ... [eliminated] ...
 *
 * Revision 1.1  1995/09/27  15:13:12  cg
 * Initial revision
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

All comments in relation to the outward behaviour of functions should be
given in the usual form in this (!) file, not in tree_compound.c.
The reason is to give a quick overview of the provided facilities
(macros and functions) in a single file. Of course, comments to the
specific implementation of a function should remain with the source code.

============================================================================*/

#ifndef _sac_tree_compound_h

#define _sac_tree_compound_h

#include "types.h"
#include "tree_basic.h"
#include "Error.h"

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* general function declarations / Macros                                   */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#define CHECK_NULL(a) ((NULL == a) ? "" : a)

/*
 * compares two module names (name maybe NULL)
 * result: 1 - equal, 0 - not equal
 */
#define CMP_MOD(a, b) ((NULL == a) ? (NULL == b) : ((NULL == b) ? 0 : (!strcmp (a, b))))

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*  macros and functions for non-node structures                            */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/***
 ***  Shpseg :
 ***/

extern int GetShpsegLength (int dims, shpseg *shape);
extern shpseg *DiffShpseg (int dim, shpseg *shape1, shpseg *shape2);
extern bool EqualShpseg (int dim, shpseg *shape2, shpseg *shape1);
extern shpseg *MergeShpseg (shpseg *first, int dim1, shpseg *second, int dim2);

extern shpseg *Array2Shpseg (node *array, int *ret_dim);
extern node *Shpseg2Array (shpseg *shape, int dim);

/*--------------------------------------------------------------------------*/

/***
 ***  Types :
 ***/

/*
 *  compound access macros
 */

#define TYPES_SHAPE(t, x) (SHPSEG_SHAPE (TYPES_SHPSEG (t), x))

/*
 *  macro name    : CMP_TYPE_USER
 *  arg types     : 1) types*
 *                  2) types*
 *  result type   : int
 *  description   : compares two user-defined types (name and module)
 *                  Names and module names must be equal.
 *  remarks       : result: 1 - equal, 0 - not equal
 */

#define CMP_TYPE_USER(a, b)                                                              \
    ((!strcmp (TYPES_NAME (a), TYPES_NAME (b)))                                          \
     && (!strcmp (CHECK_NULL (TYPES_MOD (a)), CHECK_NULL (TYPES_MOD (b)))))

extern types *GetTypes_Line (types *type, int line);
extern types *GetTypes (types *type);
extern int GetDim (types *type);
extern simpletype GetBasetype (types *type);
extern int GetBasetypeSize (types *type);
extern int GetTypesLength (types *type);
extern shpseg *Type2Shpseg (types *type, int *ret_dim);
extern node *Type2Exprs (types *type);

/******************************************************************************
 *
 * function:
 *   int IsBoxed( types *type)
 *   int IsArray( types *type)
 *   int IsUnique( types *type)
 *   int IsHidden( types *type)
 *   int IsNonUniqueHidden( types *type)
 *
 * description:
 *   These functions may be used to check for particular properties
 *   of a given data type.
 *
 ******************************************************************************/

extern bool IsBoxed (types *type);
extern bool IsArray (types *type);
extern bool IsUnique (types *type);
extern bool IsHidden (types *type);
extern bool IsNonUniqueHidden (types *type);

/*--------------------------------------------------------------------------*/

/***
 ***  Ids :
 ***/

#define IDS_VARNO(n) VARDEC_OR_ARG_VARNO (IDS_VARDEC (n))
#define IDS_TYPE(n) VARDEC_OR_ARG_TYPE (IDS_VARDEC (n))
#define IDS_DIM(n) VARDEC_OR_ARG_DIM (IDS_VARDEC (n))
#define IDS_SHAPE(n, x)                                                                  \
    SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_OR_ARG_TYPE (IDS_VARDEC (n))), x)
#define IDS_VARDEC_NAME(n) VARDEC_OR_ARG_NAME (IDS_VARDEC (n))
#define IDS_VARDEC_NEXT(n) VARDEC_OR_ARG_NEXT (IDS_VARDEC (n))
#define IDS_PADDED(n) VARDEC_OR_ARG_PADDED (IDS_VARDEC (n))

extern ids *AppendIds (ids *chain, ids *item);

/******************************************************************************
 *
 * function:
 *   ids *LookupIds(char *name, ids *ids_chain)
 *
 * description:
 *   This function searches for a given identifier name within an ids-chain
 *   of identifiers and returns the ids-structure if found or NULL otherwise.
 *
 ******************************************************************************/

extern ids *LookupIds (char *name, ids *ids_chain);

/*--------------------------------------------------------------------------*/

/***
 ***  Nums :
 ***/

/*
 *
 *  functionname  : CountNums
 *  arguments     : pointer to nums-chain (chained list of integers)
 *  description   : returns the length of th chained list
 *
 */

extern int CountNums (nums *numsp);

/*--------------------------------------------------------------------------*/

/***
 ***  ConstVec :
 ***/

extern void *CopyConstVec (simpletype vectype, int veclen, void *const_vec);
extern void *AllocConstVec (simpletype vectype, int veclen);
extern void *ModConstVec (simpletype vectype, void *const_vec, int idx, node *const_node);
extern node *AnnotateIdWithConstVec (node *expr, node *id);

/*--------------------------------------------------------------------------*/

/***
 ***  Nodelist :
 ***/

/*
 *
 *  functionname  : StoreNeededNode
 *  arguments     : 1) node which has to be inserted
 *                  2) fundef node where 1) has to be inserted
 *                  3) status of the new nodelist entry
 *  description   : inserts the given node at the end of the correct
 *                  nodelist (funlist, objlist, or typelist) of the
 *                  given fundef node and
 *  remarks       : status may be ST_regular | ST_artificial
 *
 */

extern void StoreNeededNode (node *insert, node *fundef, statustype status);

/*
 *
 *  functionname  : StoreNeededNodes
 *  arguments     : 1) list of nodes
 *                  2) fundef node where inserts are to be done
 *                  3) status of the new nodelist entry
 *  description   : inserts each node of the nodelist into the correct
 *                  nodelist of the fundef node
 *  remarks       : status may be ST_regular | ST_artificial
 *
 */

extern void StoreNeededNodes (nodelist *inserts, node *fundef, statustype status);

/*
 *
 *  functionname  : StoreUnresolvedNodes
 *  arguments     : 1) list of nodes
 *                  2) fundef node where inserts are to be done
 *                  3) status of the new nodelist entry
 *  description   : inserts all those nodes of the nodelist into the correct
 *                  nodelist of the fundef node
 *                  which have attribute 'unresolved'.
 *  remarks       : status may be ST_regular | ST_artificial
 *
 */

extern void StoreUnresolvedNodes (nodelist *inserts, node *fundef, statustype status);

/*
 *
 *  functionname  : TidyUpNodelist
 *  arguments     : 1) beginning of nodelist
 *  description   : frees all those entries of a node list which have
 *                  status 'ST_artificial'
 *  remarks       : returns the beginning of the resulting nodelist
 *
 */

extern nodelist *TidyUpNodelist (nodelist *list);

/*
 *
 *  functionname  : ConcatNodelist
 *  arguments     : 1) first node list
 *                  2) second node list
 *  description   : concatenates two node lists without checking double
 *                  occurrences
 *
 */

extern nodelist *ConcatNodelist (nodelist *first, nodelist *second);

/******************************************************************************
 *
 * function:
 *   -
 *
 * description:
 *   the following functions realize basic functions on pure node lists.
 *
 *   Append: appends a node to the given list, returning a new list.
 *           Since the node list has no special order, the new node is
 *           not appended but put in front of the given list to speed
 *           up execution.
 *           Create a list: newlist = Append(NULL, newnode, attrib);
 *   Delete: deletes all elements of the given node. If free_attrib is 0,
 *           the attribut is not set free, else a FREE(attrib) is executed.
 *   Free  : frees whole list. If free_attrib is 0, the attributes are
 *           not set free, else a FREE(attrib) is executed.
 *   Find  : returns the nodelist node of the first found item
 *           with fitting node. If not found, returns NULL.
 *
 ******************************************************************************/

extern nodelist *NodeListAppend (nodelist *nl, node *newnode, void *attrib);
extern nodelist *NodeListDelete (nodelist *nl, node *node, int free_attrib);
extern nodelist *NodeListFree (nodelist *nl, int free_attrib);
extern nodelist *NodeListFind (nodelist *nl, node *node);

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*  macros and functions for node structures                                */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/***
 ***  general :
 ***/

#define NODE_NEXT(n)                                                                     \
                                                                                         \
    (NODE_TYPE (n) == N_fundef                                                           \
       ? FUNDEF_NEXT (n)                                                                 \
       : (NODE_TYPE (n) == N_objdef                                                      \
            ? OBJDEF_NEXT (n)                                                            \
            : (NODE_TYPE (n) == N_typedef                                                \
                 ? TYPEDEF_NEXT (n)                                                      \
                 : (NODE_TYPE (n) == N_implist                                           \
                      ? IMPLIST_NEXT (n)                                                 \
                      : (NODE_TYPE (n) == N_arg                                          \
                           ? ARG_NEXT (n)                                                \
                           : (NODE_TYPE (n) == N_vardec                                  \
                                ? VARDEC_NEXT (n)                                        \
                                : (NODE_TYPE (n) == N_assign                             \
                                     ? ASSIGN_NEXT (n)                                   \
                                     : (NODE_TYPE (n) == N_exprs ? EXPRS_NEXT (n)        \
                                                                 : NULL))))))))

#define BLOCK_INSTR_OR_ASSIGN_NEXT(n)                                                    \
    (NODE_TYPE (n) == N_assign ? ASSIGN_NEXT (n)                                         \
                               : (NODE_TYPE (n) == N_block ? BLOCK_INSTR (n) : NULL))

#define L_BLOCK_INSTR_OR_ASSIGN_NEXT(n, rhs)                                             \
    if (NODE_TYPE (n) == N_assign) {                                                     \
        ASSIGN_NEXT (n) = (rhs);                                                         \
    } else if (NODE_TYPE (n) == N_block) {                                               \
        BLOCK_INSTR (n) = (rhs);                                                         \
    }

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

#define TYPEDEF_BASETYPE(n) (TYPES_BASETYPE (TYPEDEF_TYPE (n)))
#define TYPEDEF_DIM(n) (TYPES_DIM (TYPEDEF_TYPE (n)))
#define TYPEDEF_SHAPE(n, x) (TYPES_SHAPE (TYPEDEF_TYPE (n), x))
#define TYPEDEF_SHPSEG(n) (TYPES_SHPSEG (TYPEDEF_TYPE (n)))
#define TYPEDEF_TNAME(n) (TYPES_NAME (TYPEDEF_TYPE (n)))
#define TYPEDEF_TMOD(n) (TYPES_MOD (TYPEDEF_TYPE (n)))
#define TYPEDEF_TDEF(n) (TYPES_TDEF (TYPEDEF_TYPE (n)))

/*
 *  The following compound access macros are useful whenever a typedef
 *  node is used to represent a type declaration rather than a type
 *  definition.
 */

#define TYPEDEC_TYPE(n) (TYPEDEF_TYPE (TYPEDEC_DEF (n)))
#define TYPEDEC_BASETYPE(n) (TYPEDEF_BASETYPE (TYPEDEC_DEF (n)))
#define TYPEDEC_DIM(n) (TYPEDEF_DIM (TYPEDEC_DEF (n)))
#define TYPEDEC_SHAPE(n, x) (TYPEDEF_SHAPE (TYPEDEC_DEF (n), x))
#define TYPEDEC_SHPSEG(n) (TYPEDEF_SHPSEG (TYPEDEC_DEF (n)))
#define TYPEDEC_TNAME(n) (TYPEDEF_NAME (TYPEDEC_DEF (n)))
#define TYPEDEC_TMOD(n) (TYPEDEF_MOD (TYPEDEC_DEF (n)))

/*
 *
 *  macro name    : CMP_TYPEDEF(a,b)
 *  arg types     : 1) node*  (N_typedef)
 *                  2) node*  (N_typedef)
 *  result type   : int
 *  description   : compares two typedef nodes (name and module)
 *                  result: 1 - equal, 0 - not equal
 *
 */

#define CMP_TYPEDEF(a, b)                                                                \
    ((NULL == TYPEDEF_MOD (a))                                                           \
       ? (!strcmp (TYPEDEF_NAME (a), TYPEDEF_NAME (b)) && (NULL == TYPEDEF_MOD (b)))     \
       : ((NULL == TYPEDEF_MOD (b))                                                      \
            ? 0                                                                          \
            : ((!strcmp (TYPEDEF_NAME (a), TYPEDEF_NAME (b)))                            \
               && (!strcmp (TYPEDEF_MOD (a), TYPEDEF_MOD (b))))))

/*
 *
 *  macro name    : CMP_TYPE_TYPEDEF(name, mod, typedef)
 *  arg types     : 1) char*
 *                  2) char*
 *                  3) node*  (N_typedef)
 *  result type   : int
 *  description   : compares name and module name of a type with the
 *                  defined name and module name of a typedef
 *                  result: 1 - equal, 0 - not equal
 *
 */

#define CMP_TYPE_TYPEDEF(name, mod, tdef)                                                \
    ((!strcmp (name, TYPEDEF_NAME (tdef)))                                               \
     && (!strcmp (CHECK_NULL (mod), CHECK_NULL (TYPEDEF_MOD (tdef)))))

/*
 *
 *  functionname  : SearchTypedef
 *  arguments     : 1) type name to be searched for
 *                  2) module name of type to be searched for
 *                  3) list of type implementations (typedef nodes)
 *  description   : looks for a certain typedef in list of typedefs
 *
 */

extern node *SearchTypedef (char *name, char *mod, node *implementations);

extern node *AppendTypedef (node *tdef_chain, node *tdef);

/*--------------------------------------------------------------------------*/

/***
 ***  N_objdef :
 ***/

/*
 *  compound access macros
 */

#define OBJDEF_BASETYPE(n) (TYPES_BASETYPE (OBJDEF_TYPE (n)))
#define OBJDEF_DIM(n) (TYPES_DIM (OBJDEF_TYPE (n)))
#define OBJDEF_SHAPE(n, x) (TYPES_SHAPE (OBJDEF_TYPE (n), x))
#define OBJDEF_SHPSEG(n) (TYPES_SHPSEG (OBJDEF_TYPE (n)))
#define OBJDEF_TNAME(n) (TYPES_NAME (OBJDEF_TYPE (n)))
#define OBJDEF_TMOD(n) (TYPES_MOD (OBJDEF_TYPE (n)))
#define OBJDEF_TDEF(n) (TYPES_TDEF (OBJDEF_TYPE (n)))

#define OBJDEF_LINKNAME(n)                                                               \
    (OBJDEF_PRAGMA (n) == NULL ? NULL : PRAGMA_LINKNAME (OBJDEF_PRAGMA (n)))

#define OBJDEF_INITFUN(n)                                                                \
    (OBJDEF_PRAGMA (n) == NULL ? NULL : PRAGMA_INITFUN (OBJDEF_PRAGMA (n)))

#define OBJDEF_EFFECT(n)                                                                 \
    (OBJDEF_PRAGMA (n) == NULL ? NULL : PRAGMA_EFFECT (OBJDEF_PRAGMA (n)))

/*
 *
 *  macro name    : CMP_OBJDEF(a,b)
 *  arg types     : 1) node*  (N_objdef)
 *                  2) node*  (N_objdef)
 *  result type   : int
 *  description   : compares two objdef nodes (name and module)
 *                  result: 1 - equal, 0 - not equal
 *
 */

#define CMP_OBJDEF(a, b)                                                                 \
    ((NULL == OBJDEF_MOD (a))                                                            \
       ? (!strcmp (OBJDEF_NAME (a), OBJDEF_NAME (b)) && (NULL == OBJDEF_MOD (b)))        \
       : ((NULL == OBJDEF_MOD (b)) ? 0                                                   \
                                   : ((!strcmp (OBJDEF_NAME (a), OBJDEF_NAME (b)))       \
                                      && (!strcmp (OBJDEF_MOD (a), OBJDEF_MOD (b))))))

/*
 *
 *  macro name    : CMP_OBJ_OBJDEF
 *  arg types     : 1) char*
 *                  2) char*
 *                  3) node*  (N_objdef)
 *  result type   : int
 *  description   : compares name and module name of an object with the
 *                  defined name and module name of an objdef
 *                  result: 1 - equal, 0 - not equal
 *
 */

#define CMP_OBJ_OBJDEF(name, mod, odef)                                                  \
    ((mod == NULL)                                                                       \
       ? (0 == strcmp (name, OBJDEF_NAME (odef)))                                        \
       : ((0 == strcmp (name, OBJDEF_NAME (odef)))                                       \
          && ((OBJDEF_MOD (odef) == NULL) ? (0 == strcmp (mod, OBJDEF_LINKMOD (odef)))   \
                                          : (0 == strcmp (mod, OBJDEF_MOD (odef))))))

/*
 *
 *  functionname  : SearchObjdef
 *  arguments     : 1) global object name to be searched for
 *                  2) module name of global object to be searched for
 *                  3) list of object implementations (objdef nodes)
 *  description   : looks for a certain objdef in list of objdefs
 *
 */

extern node *SearchObjdef (char *name, char *mod, node *implementations);

/*
 *  functionname  : ObjList2ArgList
 *  arguments     : 1) pointer to chain of objdef nodes
 *  description   : makes an argument list from an objdef chain
 *
 */

extern void ObjList2ArgList (node *objdef);

extern node *AppendObjdef (node *objdef_chain, node *objdef);

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
 ***/

/*
 *  compound access macros
 */

#define FUNDEF_BASETYPE(n) (TYPES_BASETYPE (FUNDEF_TYPES (n)))
#define FUNDEF_DIM(n) (TYPES_DIM (FUNDEF_TYPES (n)))
#define FUNDEF_SHAPE(n, x) (TYPES_SHAPE (FUNDEF_TYPES (n), x))
#define FUNDEF_SHPSEG(n) (TYPES_SHPSEG (FUNDEF_TYPES (n)))
#define FUNDEF_TNAME(n) (TYPES_NAME (FUNDEF_TYPES (n)))
#define FUNDEF_TMOD(n) (TYPES_MOD (FUNDEF_TYPES (n)))
#define FUNDEF_TDEF(n) (TYPES_TDEF (FUNDEF_TYPES (n)))

#define FUNDEF_NEEDFUNS(n) (BLOCK_NEEDFUNS (FUNDEF_BODY (n)))
#define FUNDEF_NEEDTYPES(n) (BLOCK_NEEDTYPES (FUNDEF_BODY (n)))
#define FUNDEF_VARDEC(n) (BLOCK_VARDEC (FUNDEF_BODY (n)))
#define FUNDEF_INSTR(n) (BLOCK_INSTR (FUNDEF_BODY (n)))
#define FUNDEF_BODY_VARNO(n) (BLOCK_VARNO (FUNDEF_BODY (n)))

#define FUNDEF_DEFMASK(n) (FUNDEF_MASK (n, 0))
#define FUNDEF_USEMASK(n) (FUNDEF_MASK (n, 1))

#define FUNDEF_LINKNAME(n) (PRAGMA_LINKNAME (FUNDEF_PRAGMA (n)))
#define FUNDEF_LINKSIGN(n) (PRAGMA_LINKSIGN (FUNDEF_PRAGMA (n)))
#define FUNDEF_EFFECT(n) (PRAGMA_EFFECT (FUNDEF_PRAGMA (n)))
#define FUNDEF_TOUCH(n) (PRAGMA_TOUCH (FUNDEF_PRAGMA (n)))
#define FUNDEF_READONLY(n) (PRAGMA_READONLY (FUNDEF_PRAGMA (n)))
#define FUNDEF_REFCOUNTING(n) (PRAGMA_REFCOUNTING (FUNDEF_PRAGMA (n)))
#define FUNDEF_PRATYPES(n) (PRAGMA_NEEDTYPES (FUNDEF_PRAGMA (n)))
#define FUNDEF_PRAFUNS(n) (PRAGMA_NEEDFUNS (FUNDEF_PRAGMA (n)))
#define FUNDEF_PRALINKMOD(n) (PRAGMA_LINKMOD (FUNDEF_PRAGMA (n)))

/*
 *  The following compound access macros are useful whenever a fundef
 *  node is used to represent a function declaration rather than a
 *  function definition.
 */

#define FUNDEC_NEEDFUNS(n) (FUNDEF_NEEDFUNS (FUNDEC_DEF (n)))
#define FUNDEC_NEEDTYPES(n) (FUNDEF_NEEDTYPES (FUNDEC_DEF (n)))
#define FUNDEC_NEEDOBJS(n) (FUNDEF_NEEDOBJS (FUNDEC_DEF (n)))

#define FUNDEC_TYPES(n) (FUNDEF_TYPES (FUNDEC_DEF (n)))
#define FUNDEC_BASETYPE(n) (FUNDEF_BASETYPE (FUNDEC_DEF (n)))
#define FUNDEC_DIM(n) (FUNDEF_DIM (FUNDEC_DEF (n)))
#define FUNDEC_SHAPE(n, x) (FUNDEF_SHAPE (FUNDEC_DEF (n), x))
#define FUNDEC_SHPSEG(n) (FUNDEF_SHPSEG (FUNDEC_DEF (n)))
#define FUNDEC_TNAME(n) (FUNDEF_NAME (FUNDEC_DEF (n)))
#define FUNDEC_TMOD(n) (FUNDEF_MOD (FUNDEC_DEF (n)))

/*
 *  macro name    : CMP_FUN_ID(a,b)
 *  arg types     : 1) node*  (N_objdef)
 *                  2) node*  (N_objdef)
 *  result type   : int
 *  description   : compares two fundef nodes (name and module only)
 *                  result: 1 - equal, 0 - not equal
 */

#define CMP_FUN_ID(a, b)                                                                 \
    ((0 == strcmp (FUNDEF_NAME (a), FUNDEF_NAME (b)))                                    \
     && (0 == strcmp (CHECK_NULL (FUNDEF_MOD (a)), CHECK_NULL (FUNDEF_MOD (b)))))

/*
 *  macro name    : CMP_FUNDEF(a,b)
 *  arg types     : 1) node*  (N_objdef)
 *                  2) node*  (N_objdef)
 *  result type   : int
 *  description   : compares two fundef nodes (name, module, and domain)
 *                  result: 1 - equal, 0 - not equal
 */

#define CMP_FUNDEF(a, b)                                                                 \
    ((CMP_FUN_ID (a, b)) ? CmpDomain (FUNDEF_ARGS (a), FUNDEF_ARGS (b)) : 0)

extern node *FindVardec_Name (char *name, node *fundef);
extern node *FindVardec_Varno (int varno, node *fundef);

/*
 *
 *  functionname  : CountFunctionParams
 *  arguments     : 1) N_fundef node
 *  description   : counts the number of parameters of a function.
 *                  This includes return values AND formal arguments.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       : even the return type 'void' counts !
 *
 */

extern int CountFunctionParams (node *fundef);

/*
 *  functionname  : SearchFundef
 *  arguments     : 1) fundef node of function to search for
 *                  2) ptr to head of fundef chain
 *  description   : returns a ptr to the respective fundef node
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : CMP_FUNDEF
 *
 *  remarks       : This function is used to find the implementation of
 *                  a function which is declared in a module/class
 *                  implementation. For the representation of function
 *                  declarations the fundef node is reused.
 */

extern node *SearchFundef (node *fun, node *allfuns);

extern node *AppendFundef (node *fundef_chain, node *fundef);

/*--------------------------------------------------------------------------*/

/***
 ***  N_block :
 ***/

/*
 *  compound access macros
 */

#define BLOCK_DEFMASK(n) (BLOCK_MASK (n, 0))
#define BLOCK_USEMASK(n) (BLOCK_MASK (n, 1))
#define BLOCK_MRDMASK(n) (BLOCK_MASK (n, 2))

#define MAKE_EMPTY_BLOCK() MakeBlock (MakeEmpty (), NULL)

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :
 ***/

/*
 *  compound access macros
 */

#define VARDEC_BASETYPE(n) (TYPES_BASETYPE (VARDEC_TYPE (n)))
#define VARDEC_DIM(n) (TYPES_DIM (VARDEC_TYPE (n)))
#define VARDEC_SHAPE(n, x) (TYPES_SHAPE (VARDEC_TYPE (n), x))
#define VARDEC_SHPSEG(n) (TYPES_SHPSEG (VARDEC_TYPE (n)))
#define VARDEC_TNAME(n) (TYPES_NAME (VARDEC_TYPE (n)))
#define VARDEC_TMOD(n) (TYPES_MOD (VARDEC_TYPE (n)))
#define VARDEC_TDEF(n) (TYPES_TDEF (VARDEC_TYPE (n)))

/******************************************************************************
 *
 * function:
 *   node *AppendVardec( node *vardec_chain, node *vardec)
 *
 * description:
 *   Appends 'vardec' to 'vardec_chain' and returns the new chain.
 *
 * remark:
 *   In order to use this function in Compile() it can handle mixed chains
 *   containing N_vardec- *and* N_assign-nodes!
 *
 ******************************************************************************/

extern node *AppendVardec (node *vardec_chain, node *vardec);

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***/

/*
 *  compound access macros
 */

#define ARG_BASETYPE(n) (TYPES_BASETYPE (ARG_TYPE (n)))
#define ARG_DIM(n) (TYPES_DIM (ARG_TYPE (n)))
#define ARG_SHAPE(n, x) (TYPES_SHAPE (ARG_TYPE (n), x))
#define ARG_SHPSEG(n) (TYPES_SHPSEG (ARG_TYPE (n)))
#define ARG_TNAME(n) (TYPES_NAME (ARG_TYPE (n)))
#define ARG_TMOD(n) (TYPES_MOD (ARG_TYPE (n)))
#define ARG_TDEF(n) (TYPES_TDEF (ARG_TYPE (n)))

/*
 *  functionname  : CmpDomain
 *  arguments     : 1) N_arg node of one function
 *                  2) N_arg node of another function
 *  description   : checks whether the functions have equal domain
 *                  returns 1 if domain is equal, 0 else
 *  remarks       : similar to function CmpFunParams of typechecker.
 *                  some minor changes to fix appearing segmentation
 *                  faults.
 */

extern int CmpDomain (node *args1, node *args2);

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :  *and*  N_arg :
 ***/

/*
 * CAUTION: Do not use the following macros as l-values!!!
 *          (this is *no* ANSI C style!)
 *          Use the L_VARDEC_OR_... macros instead!!
 */
#define VARDEC_OR_ARG_NAME(n) ((NODE_TYPE (n) == N_arg) ? ARG_NAME (n) : VARDEC_NAME (n))
#define VARDEC_OR_ARG_ACTCHN(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_ACTCHN (n) : VARDEC_ACTCHN (n))
#define VARDEC_OR_ARG_COLCHN(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_COLCHN (n) : VARDEC_COLCHN (n))
#define VARDEC_OR_ARG_TYPE(n) ((NODE_TYPE (n) == N_arg) ? ARG_TYPE (n) : VARDEC_TYPE (n))
#define VARDEC_OR_ARG_DIM(n) ((NODE_TYPE (n) == N_arg) ? ARG_DIM (n) : VARDEC_DIM (n))
#define VARDEC_OR_ARG_SHAPE(n, x)                                                        \
    ((NODE_TYPE (n) == N_arg) ? ARG_SHAPE (n, x) : VARDEC_SHAPE (n, x))
#define VARDEC_OR_ARG_SHPSEG(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_SHPSEG (n) : VARDEC_SHPSEG (n))
#define VARDEC_OR_ARG_VARNO(n)                                                           \
    ((NODE_TYPE (n) == N_arg) ? ARG_VARNO (n) : VARDEC_VARNO (n))
#define VARDEC_OR_ARG_REFCNT(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_REFCNT (n) : VARDEC_REFCNT (n))
#define VARDEC_OR_ARG_NAIVE_REFCNT(n)                                                    \
    ((NODE_TYPE (n) == N_arg) ? ARG_NAIVE_REFCNT (n) : VARDEC_NAIVE_REFCNT (n))
#define VARDEC_OR_ARG_ATTRIB(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_ATTRIB (n) : VARDEC_ATTRIB (n))
#define VARDEC_OR_ARG_STATUS(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_STATUS (n) : VARDEC_STATUS (n))
#define VARDEC_OR_ARG_OBJDEF(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_OBJDEF (n) : VARDEC_OBJDEF (n))
#define VARDEC_OR_ARG_NEXT(n) ((NODE_TYPE (n) == N_arg) ? ARG_NEXT (n) : VARDEC_NEXT (n))
#define VARDEC_OR_ARG_PADDED(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_PADDED (n) : VARDEC_PADDED (n))

#define L_VARDEC_OR_ARG_ACTCHN(n, rhs)                                                   \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_ACTCHN (n) = (rhs);                                                          \
    } else {                                                                             \
        VARDEC_ACTCHN (n) = (rhs);                                                       \
    }

#define L_VARDEC_OR_ARG_COLCHN(n, rhs)                                                   \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_COLCHN (n) = (rhs);                                                          \
    } else {                                                                             \
        VARDEC_COLCHN (n) = (rhs);                                                       \
    }

#define L_VARDEC_OR_ARG_REFCNT(n, rhs)                                                   \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_REFCNT (n) = (rhs);                                                          \
    } else {                                                                             \
        VARDEC_REFCNT (n) = (rhs);                                                       \
    }

#define L_VARDEC_OR_ARG_NAIVE_REFCNT(n, rhs)                                             \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_NAIVE_REFCNT (n) = (rhs);                                                    \
    } else {                                                                             \
        VARDEC_NAIVE_REFCNT (n) = (rhs);                                                 \
    }

#define L_VARDEC_OR_ARG_NEXT(n, rhs)                                                     \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_NEXT (n) = (rhs);                                                            \
    } else {                                                                             \
        VARDEC_NEXT (n) = (rhs);                                                         \
    }

#define L_VARDEC_OR_ARG_PADDED(n, rhs)                                                   \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_PADDED (n) = (rhs);                                                          \
    } else {                                                                             \
        VARDEC_PADDED (n) = (rhs);                                                       \
    }

/*
 * this macro is usefull for traversing the arg- und vardec-list
 */
#define FOREACH_VARDEC_AND_ARG(fundef, vardec, code)                                     \
    vardec = FUNDEF_ARGS (fundef);                                                       \
    while (vardec != NULL) {                                                             \
        code vardec = ARG_NEXT (vardec);                                                 \
    }                                                                                    \
    vardec = FUNDEF_VARDEC (fundef);                                                     \
    while (vardec != NULL) {                                                             \
        code vardec = VARDEC_NEXT (vardec);                                              \
    }

extern node *SearchDecl (char *name, node *decl_node);

/*--------------------------------------------------------------------------*/

/***
 ***  N_assign :
 ***/

/*
 *  compound access macros
 */

#define ASSIGN_DEFMASK(n) (ASSIGN_MASK (n, 0))
#define ASSIGN_USEMASK(n) (ASSIGN_MASK (n, 1))
#define ASSIGN_MRDMASK(n) (ASSIGN_MASK (n, 2))
#define ASSIGN_INSTRTYPE(n) (NODE_TYPE (ASSIGN_INSTR (n)))
#define ASSIGN_NAME(n) (LET_NAME (ASSIGN_INSTR (n)))

extern node *AppendAssign (node *assign_chain, node *assign);

/******************************************************************************
 *
 * function:
 *   node *MakeAssignLet(char *var_name, node *vardec_node, node* let_expr);
 *
 * arguments: 1) name of the variable
 *            2) vardec-node
 *            3) let-expression
 *            R) assign-node with complete let-subtree
 *
 * description:
 *   returns a assign-node with let-node: var_name = expr;
 *
 * remarks:
 *   ASSIGN_NEXT is set to NULL
 *
 ******************************************************************************/

extern node *MakeAssignLet (char *var_name, node *vardec_node, node *let_expr);

/******************************************************************************
 *
 * function:
 *   node *MakeAssignIcm0(char *name)
 *   node *MakeAssignIcm1(char *name, node *arg1)
 *   node *MakeAssignIcm3(char *name, node *arg1, node *arg2)
 *   node *MakeAssignIcm4(char *name, node *arg1, node *arg2, node *arg3,
 *                                    node *arg4)
 *   node *MakeAssignIcm5(char *name, node *arg1, node *arg2, node *arg3,
 *                                    node *arg4, node *arg5)
 *   node *MakeAssignIcm6(char *name, node *arg1, node *arg2, node *arg3,
 *                                    node *arg4, node *arg5, node *arg6)
 *   node *MakeAssignIcm7(char *name, node *arg1, node *arg2, node *arg3,
 *                                    node *arg4, node *arg5, node *arg6,
 *                                    node *arg7)
 *
 * description:
 *   These functions generate an N_assign node with a complete ICM
 *   representations including arguments as body.
 *   Each function argument may be an arbitrary list of single ICM arguments.
 *   These are concatenated correctly.
 *   The ASSIGN_NEXT will be NULL!
 *
 ******************************************************************************/

extern node *MakeAssignIcm0 (char *name);
extern node *MakeAssignIcm1 (char *name, node *arg1);
extern node *MakeAssignIcm2 (char *name, node *arg1, node *arg2);
extern node *MakeAssignIcm3 (char *name, node *arg1, node *arg2, node *arg3);
extern node *MakeAssignIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4);
extern node *MakeAssignIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                             node *arg5);
extern node *MakeAssignIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                             node *arg5, node *arg6);
extern node *MakeAssignIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                             node *arg5, node *arg6, node *arg7);

/******************************************************************************
 *
 * function:
 *   node *GetCompoundNode(node* arg_node);
 *
 * description:
 *   returns the compund_node that is attached to the assign-node
 *
 ******************************************************************************/

extern node *GetCompoundNode (node *arg_node);

/******************************************************************************
 *
 * function:
 *   node *AppendAssignIcm( node *assign, char *name, node *args)
 *
 * description:
 *   Appends an new ICM with name and args given as an assign to the given
 *   chain of assignments assign.
 *
 ******************************************************************************/

extern node *AppendAssignIcm (node *assign, char *name, node *args);

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :        (see also N_array !!!)
 ***/

/******************************************************************************
 *
 * function:
 *   node *ExprsConcat( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_exprs chains of nodes.
 *
 ******************************************************************************/

extern node *ExprsConcat (node *exprs1, node *exprs2);

/******************************************************************************
 *
 * function:
 *   node *MakeExprsNum( int num)
 *
 * description:
 *   Makes an N_exprs with a N_num as EXPR, NEXT is NULL.
 *
 ******************************************************************************/

extern node *MakeExprsNum (int num);

/******************************************************************************
 *
 * function:
 *   node *GetExprsLength( node *exprs)
 *
 * description:
 *   Computes the length of the given N_exprs chain.
 *
 ******************************************************************************/

extern int GetExprsLength (node *exprs);

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :
 ***/

/*
 *  compound access macros
 */

#define LET_NAME(n) (IDS_NAME (LET_IDS (n)))
#define LET_VARDEC(n) (IDS_VARDEC (LET_IDS (n)))
#define LET_MOD(n) (IDS_MOD (LET_IDS (n)))
#define LET_STATUS(n) (IDS_STATUS (LET_IDS (n)))
#define LET_VARNO(n) (VARDEC_OR_ARG_VARNO (LET_VARDEC (n)))
#define LET_TYPE(n) (VARDEC_OR_ARG_TYPE (LET_VARDEC (n)))
#define LET_BASETYPE(n) (TYPES_BASETYPE (LET_TYPE (n)))
#define LET_USE(n) (IDS_USE (LET_IDS (n)))

#define MAKE_OPON_LET(id, expr, op)                                                      \
    MakeLet (MAKE_BIN_PRF (op, MakeId (id, NULL, ST_regular), expr),                     \
             MakeIds (StringCopy (id), NULL, ST_regular))

#define MAKE_INCDEC_LET(id, op)                                                          \
    MakeLet (MAKE_BIN_PRF (op, MakeId (id, NULL, ST_regular), MakeNum (1)),              \
             MakeIds (StringCopy (id), NULL, ST_regular))

/*--------------------------------------------------------------------------*/

/***
 ***  N_cast :
 ***/

/*
 *  compound access macros
 */

#define CAST_BASETYPE(n) (TYPES_BASETYPE (CAST_TYPE (n)))
#define CAST_DIM(n) (TYPES_DIM (CAST_TYPE (n)))
#define CAST_SHAPE(n, x) (TYPES_SHAPE (CAST_TYPE (n), x))
#define CAST_SHPSEG(n) (TYPES_SHPSEG (CAST_TYPE (n)))
#define CAST_TNAME(n) (TYPES_NAME (CAST_TYPE (n)))
#define CAST_TMOD(n) (TYPES_MOD (CAST_TYPE (n)))
#define CAST_TDEF(n) (TYPES_TDEF (CAST_TYPE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_return :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_cond :
 ***/

/*
 *  compound access macros
 */

#define COND_CONDUSEMASK(n) (COND_MASK (n, 1))
#define COND_THENDEFMASK(n) (BLOCK_DEFMASK (COND_THEN (n)))
#define COND_THENUSEMASK(n) (BLOCK_USEMASK (COND_THEN (n)))
#define COND_ELSEDEFMASK(n) (BLOCK_DEFMASK (COND_ELSE (n)))
#define COND_ELSEUSEMASK(n) (BLOCK_USEMASK (COND_ELSE (n)))
#define COND_THENINSTR(n) (BLOCK_INSTR (COND_THEN (n)))
#define COND_ELSEINSTR(n) (BLOCK_INSTR (COND_ELSE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :
 ***/

/*
 *  compound access macros
 */

#define DO_DEFMASK(n) (BLOCK_DEFMASK (DO_BODY (n)))
#define DO_USEMASK(n) (BLOCK_USEMASK (DO_BODY (n)))
#define DO_MRDMASK(n) (BLOCK_MRDMASK (DO_BODY (n)))
#define DO_TERMMASK(n) (DO_MASK (n, 1))
#define DO_INSTR(n) (BLOCK_INSTR (DO_BODY (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_while :
 ***/

/*
 *  compound access macros
 */

#define WHILE_DEFMASK(n) (BLOCK_DEFMASK (WHILE_BODY (n)))
#define WHILE_USEMASK(n) (BLOCK_USEMASK (WHILE_BODY (n)))
#define WHILE_MRDMASK(n) (BLOCK_MRDMASK (WHILE_BODY (n)))
#define WHILE_TERMMASK(n) (WHILE_MASK (n, 1))
#define WHILE_INSTR(n) (BLOCK_INSTR (WHILE_BODY (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :  *and*  N_while :
 ***/

/*
 *  compound access macros
 *  (right hand use only, left hand macros see below)
 */

#define DO_OR_WHILE_COND(n) ((NODE_TYPE (n) == N_do) ? DO_COND (n) : WHILE_COND (n))
#define DO_OR_WHILE_BODY(n) ((NODE_TYPE (n) == N_do) ? DO_BODY (n) : WHILE_BODY (n))
#define DO_OR_WHILE_USEVARS(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_USEVARS (n) : WHILE_USEVARS (n))
#define DO_OR_WHILE_DEFVARS(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_DEFVARS (n) : WHILE_DEFVARS (n))
#define DO_OR_WHILE_NAIVE_USEVARS(n)                                                     \
    ((NODE_TYPE (n) == N_do) ? DO_NAIVE_USEVARS (n) : WHILE_NAIVE_USEVARS (n))
#define DO_OR_WHILE_NAIVE_DEFVARS(n)                                                     \
    ((NODE_TYPE (n) == N_do) ? DO_NAIVE_DEFVARS (n) : WHILE_NAIVE_DEFVARS (n))
#define DO_OR_WHILE_MASK(n, x)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_MASK (n, x) : WHILE_MASK (n, x))
#define DO_OR_WHILE_DEFMASK(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_DEFMASK (n) : WHILE_DEFMASK (n))
#define DO_OR_WHILE_USEMASK(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_USEMASK (n) : WHILE_USEMASK (n))
#define DO_OR_WHILE_TERMMASK(n)                                                          \
    ((NODE_TYPE (n) == N_do) ? DO_TERMMASK (n) : WHILE_TERMMASK (n))
#define DO_OR_WHILE_INSTR(n) ((NODE_TYPE (n) == N_do) ? DO_INSTR (n) : WHILE_INSTR (n))

/*
 *  compound set macros
 *  (left hand use only, right hand macros see above)
 */

#define L_DO_OR_WHILE_COND(n, rhs)                                                       \
    if (NODE_TYPE (n) == N_do) {                                                         \
        DO_COND (n) = (rhs);                                                             \
    } else {                                                                             \
        WHILE_COND (n) = (rhs);                                                          \
    }

#define L_DO_OR_WHILE_BODY(n, rhs)                                                       \
    if (NODE_TYPE (n) == N_do) {                                                         \
        DO_BODY (n) = (rhs);                                                             \
    } else {                                                                             \
        WHILE_BODY (n) = (rhs);                                                          \
    }

#define L_DO_OR_WHILE_USEVARS(n, rhs)                                                    \
    if (NODE_TYPE (n) == N_do) {                                                         \
        DO_USEVARS (n) = (rhs);                                                          \
    } else {                                                                             \
        WHILE_USEVARS (n) = (rhs);                                                       \
    }

#define L_DO_OR_WHILE_DEFVARS(n, rhs)                                                    \
    if (NODE_TYPE (n) == N_do) {                                                         \
        DO_DEFVARS (n) = (rhs);                                                          \
    } else {                                                                             \
        WHILE_DEFVARS (n) = (rhs);                                                       \
    }

#define L_DO_OR_WHILE_NAIVE_USEVARS(n, rhs)                                              \
    if (NODE_TYPE (n) == N_do) {                                                         \
        DO_NAIVE_USEVARS (n) = (rhs);                                                    \
    } else {                                                                             \
        WHILE_NAIVE_USEVARS (n) = (rhs);                                                 \
    }

#define L_DO_OR_WHILE_NAIVE_DEFVARS(n, rhs)                                              \
    if (NODE_TYPE (n) == N_do) {                                                         \
        DO_NAIVE_DEFVARS (n) = (rhs);                                                    \
    } else {                                                                             \
        WHILE_NAIVE_DEFVARS (n) = (rhs);                                                 \
    }

#define L_DO_OR_WHILE_INSTR(n, rhs)                                                      \
    if (NODE_TYPE (n) == N_do) {                                                         \
        DO_INSTR (n) = (rhs);                                                            \
    } else {                                                                             \
        WHILE_INSTR (n) = (rhs);                                                         \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :    (see also N_exprs, Shpseg !!!)
 ***/

/*
 *  compound access macros
 */

#define ARRAY_NODETYPE(n) (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (n))))
#define ARRAY_BASETYPE(n) (TYPES_BASETYPE (ARRAY_TYPE (n)))
#define ARRAY_DIM(n) (TYPES_DIM (ARRAY_TYPE (n)))
#define ARRAY_SHAPE(n, x) (TYPES_SHAPE (ARRAY_TYPE (n), x))
#define ARRAY_SHPSEG(n) (TYPES_SHPSEG (ARRAY_TYPE (n)))
#define ARRAY_TNAME(n) (TYPES_NAME (ARRAY_TYPE (n)))
#define ARRAY_TMOD(n) (TYPES_MOD (ARRAY_TYPE (n)))
#define ARRAY_TDEF(n) (TYPES_TDEF (ARRAY_TYPE (n)))

/*
 *  function declarations
 */

extern node *CreateZeroScalar (simpletype btype);
extern node *CreateZeroVector (int length, simpletype btype);

/******************************************************************************
 *
 * Function:
 *   int IsConstArray( node *array);
 *
 * Description:
 *   Returns number of constant elements if argument is an N_array and all
 *   its elements are N_num, N_char, N_float, N_double, N_bool or otherwise
 *   returns 0.
 *
 *   The parameter type specified the necessary type all elements have to
 *   be of (nodetype, e.g. N_num). If N_ok is given, the type is ignored.
 *
 ******************************************************************************/

extern int IsConstArray (node *array);

/******************************************************************************
 *
 * Function:
 *   node *IntVec2Array(int length, int* intvec);
 *
 * Description:
 *   Returns an N_exprs node containing the elements of intvec.
 *
 ******************************************************************************/

extern node *IntVec2Array (int length, int *intvec);

/******************************************************************************
 *
 * Function:
 *   int    *Array2IntVec(node* aelems, int* length);
 *   int    *Array2BoolVec(node* aelems, int* length);
 *   char   *Array2CharVec(node* aelems, int* length);
 *   float  *Array2FloatVec(node* aelems, int* length);
 *   double *Array2DblVec(node* aelems, int* length);
 *
 * Description:
 *   Returns an iteger (char | float | double) vector and stores the number of
 *   constant integer (char | float | double) elements in *length if first
 *   argument is an N_exprs and all its elements are N_num otherwise the
 *   result is not defined.
 *   If the length of the vector is not of interest, length may be NULL.
 *
 ******************************************************************************/

extern int *Array2IntVec (node *aelems, int *length);
extern int *Array2BoolVec (node *aelems, int *length);
extern char *Array2CharVec (node *aelems, int *length);
extern float *Array2FloatVec (node *aelems, int *length);
extern double *Array2DblVec (node *aelems, int *length);

/*--------------------------------------------------------------------------*/

/***
 ***  N_vinfo :
 ***/

/*
 *  compound access macros
 */

#define VINFO_DIM(n) SHAPES_DIM (VINFO_TYPE (n))
#define VINFO_SHPSEG(n) SHAPES_SHPSEG (VINFO_TYPE (n))
#define VINFO_SELEMS(n) SHAPES_SELEMS (VINFO_TYPE (n))

extern node *MakeVinfoDollar (node *next);

/*--------------------------------------------------------------------------*/

/***
 ***  N_id :
 ***/

/*
 *  compound access macros
 */

#define ID_VARNO(n) VARDEC_OR_ARG_VARNO (ID_VARDEC (n))
#define ID_TYPE(n) VARDEC_OR_ARG_TYPE (ID_VARDEC (n))
#define ID_DIM(n) VARDEC_OR_ARG_DIM (ID_VARDEC (n))
#define ID_SHAPE(n, x) SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_OR_ARG_TYPE (ID_VARDEC (n))), x)
#define ID_VARDEC_NAME(n) VARDEC_OR_ARG_NAME (ID_VARDEC (n))
#define ID_VARDEC_NEXT(n) VARDEC_OR_ARG_NEXT (ID_VARDEC (n))
#define ID_PADDED(n) VARDEC_OR_ARG_PADDED (ID_VARDEC (n))

#define ID_OR_CAST_TYPE(n) ((NODE_TYPE (n) == N_id) ? ID_TYPE (n) : CAST_TYPE (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :
 ***
 ***  watch combined macros for N_ap and N_prf
 ***  (search for "N_ap :" or "N_prf :").
 ***/

/*
 *  compound access macros
 */

#define PRF_ARG1(n) (EXPRS_EXPR (PRF_ARGS (n)))
#define PRF_ARG2(n) (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (n))))
#define PRF_ARG3(n) (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (PRF_ARGS (n)))))

#define MAKE_BIN_PRF(f, arg1, arg2) MakePrf (f, MakeExprs (arg1, MakeExprs (arg2, NULL)))

/*
 *  function declarations
 */

extern node *MakePrf1 (prf prf, node *arg1);
extern node *MakePrf2 (prf prf, node *arg1, node *arg2);
extern node *MakePrf3 (prf prf, node *arg1, node *arg2, node *arg3);

/*--------------------------------------------------------------------------*/

/***
 ***  N_ap :
 ***
 ***  watch combined macros for N_ap and N_prf
 ***  (search for "N_ap :" or "N_prf :").
 ***/

/*
 *  compound access macros
 */

#define AP_ARG1(n) (EXPRS_EXPR (AP_ARGS (n)))
#define AP_ARG2(n) (EXPRS_EXPR (EXPRS_NEXT (AP_ARGS (n))))
#define AP_ARG3(n) (EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (AP_ARGS (n)))))

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :  *and*  N_ap :
 ***
 ***  watch simple macros for N_ap and N_prf
 ***  (search for "N_ap :" or "N_prf :").
 ***/

#define AP_OR_PRF_ARGS(n) ((NODE_TYPE (n) == N_ap) ? AP_ARGS (n) : PRF_ARGS (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_pragma :
 ***/

/*
 *  compound access macros
 */

#define PRAGMA_LS(n, i) PRAGMA_LINKSIGN (n)[i]
#define PRAGMA_RC(n, i) PRAGMA_REFCOUNTING (n)[i]
#define PRAGMA_RO(n, i) PRAGMA_READONLY (n)[i]

/*--------------------------------------------------------------------------*/

/***
 ***  N_icm :
 ***/

/*
 *  compound access macros
 */

#define ICM_EXPRS1(n) ICM_ARGS (n)
#define ICM_EXPRS2(n) EXPRS_NEXT (ICM_ARGS (n))
#define ICM_EXPRS3(n) EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (n)))
#define ICM_EXPRS4(n) EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (n))))
#define ICM_EXPRS5(n) EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (n)))))
#define ICM_EXPRS6(n)                                                                    \
    EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (n))))))

#define ICM_ARG1(n) EXPRS_EXPR (ICM_EXPRS1 (n))
#define ICM_ARG2(n) EXPRS_EXPR (ICM_EXPRS2 (n))
#define ICM_ARG3(n) EXPRS_EXPR (ICM_EXPRS3 (n))
#define ICM_ARG4(n) EXPRS_EXPR (ICM_EXPRS4 (n))
#define ICM_ARG5(n) EXPRS_EXPR (ICM_EXPRS5 (n))
#define ICM_ARG6(n) EXPRS_EXPR (ICM_EXPRS6 (n))

/******************************************************************************
 *
 * function:
 *   node *MakeIcm0(char *name)
 *   node *MakeIcm1(char *name, node *arg1)
 *   node *MakeIcm3(char *name, node *arg1, node *arg2)
 *   node *MakeIcm4(char *name, node *arg1, node *arg2, node *arg3, node *arg4)
 *   node *MakeIcm5(char *name, node *arg1, node *arg2, node *arg3, node *arg4,
 *                              node *arg5)
 *   node *MakeIcm6(char *name, node *arg1, node *arg2, node *arg3, node *arg4,
 *                              node *arg5, node *arg6)
 *   node *MakeIcm7(char *name, node *arg1, node *arg2, node *arg3, node *arg4,
 *                              node *arg5, node *arg6, node *arg7)
 *
 * description:
 *   These functions generate complete ICM representations including arguments.
 *   Each function argument may be an arbitrary list of single ICM arguments.
 *   These are concatenated correctly.
 *
 ******************************************************************************/

extern node *MakeIcm0 (char *name);
extern node *MakeIcm1 (char *name, node *arg1);
extern node *MakeIcm2 (char *name, node *arg1, node *arg2);
extern node *MakeIcm3 (char *name, node *arg1, node *arg2, node *arg3);
extern node *MakeIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4);
extern node *MakeIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                       node *arg5);
extern node *MakeIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                       node *arg5, node *arg6);
extern node *MakeIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                       node *arg5, node *arg6, node *arg7);

/*--------------------------------------------------------------------------*/

/***
 ***  N_mt :   N_st :
 ***/

#define MT_OR_ST_REGION(n) (((NODE_TYPE (n) == N_mt)) ? MT_REGION (n) : ST_REGION (n))

#define MT_OR_ST_USEMASK(n) (((NODE_TYPE (n) == N_mt)) ? MT_USEMASK (n) : ST_USEMASK (n))

#define MT_OR_ST_DEFMASK(n) (((NODE_TYPE (n) == N_mt)) ? MT_DEFMASK (n) : ST_DEFMASK (n))

#define L_MT_OR_ST_REGION(n, region)                                                     \
    if (NODE_TYPE (n) == N_mt) {                                                         \
        MT_REGION (n) = region;                                                          \
    } else {                                                                             \
        ST_REGION (n) = region;                                                          \
    }

#define L_MT_OR_ST_USEMASK(n, region)                                                    \
    if (NODE_TYPE (n) == N_mt) {                                                         \
        MT_USEMASK (n) = region;                                                         \
    } else {                                                                             \
        ST_USEMASK (n) = region;                                                         \
    }

#define L_MT_OR_ST_DEFMASK(n, region)                                                    \
    if (NODE_TYPE (n) == N_mt) {                                                         \
        MT_DEFMASK (n) = region;                                                         \
    } else {                                                                             \
        ST_DEFMASK (n) = region;                                                         \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_spmd :
 ***/

/*
 *  compound access macros
 */

#define SPMD_VARDEC(n) BLOCK_VARDEC (SPMD_REGION (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith :
 ***/

#define NWITH_TYPE(n) (NWITHOP_TYPE (NWITH_WITHOP (n)))

/*
 * We only need to inspect the withid of the first part,
 * because the withid is in *all* parts the same!!
 */
#define NWITH_WITHID(n) (NPART_WITHID (NWITH_PART (n)))
#define NWITH_IDS(n) (NPART_IDS (NWITH_PART (n)))
#define NWITH_VEC(n) (NPART_VEC (NWITH_PART (n)))

/*
 * BOUND1, BOUND2, STEP, WIDTH of the *first* N_Npart-node
 * (useful *before* with-loop-folding only!!!)
 */
#define NWITH_BOUND1(n) (NPART_BOUND1 (NWITH_PART (n)))
#define NWITH_BOUND2(n) (NPART_BOUND2 (NWITH_PART (n)))
#define NWITH_STEP(n) (NPART_STEP (NWITH_PART (n)))
#define NWITH_WIDTH(n) (NPART_WIDTH (NWITH_PART (n)))

/*
 * CBLOCK, CEXPR of the *first* N_Ncode-node
 * (useful in case of single-generator with-loops only,
 *  e.g. before with-loop-folding)
 */
#define NWITH_CBLOCK(n) (NCODE_CBLOCK (NWITH_CODE (n)))
#define NWITH_CEXPR(n) (NCODE_CEXPR (NWITH_CODE (n)))

#define NWITH_FUN(n) (NWITHOP_FUN (NWITH_WITHOP (n)))
#define NWITH_MOD(n) (NWITHOP_MOD (NWITH_WITHOP (n)))
#define NWITH_FUNDEF(n) (NWITHOP_FUNDEF (NWITH_WITHOP (n)))
#define NWITH_SHAPE(n) (NWITHOP_SHAPE (NWITH_WITHOP (n)))
#define NWITH_ARRAY(n) (NWITHOP_ARRAY (NWITH_WITHOP (n)))
#define NWITH_NEUTRAL(n) (NWITHOP_NEUTRAL (NWITH_WITHOP (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Npart :
 ***/

#define NPART_IDS(n) (NWITHID_IDS (NPART_WITHID (n)))
#define NPART_VEC(n) (NWITHID_VEC (NPART_WITHID (n)))

#define NPART_BOUND1(n) (NGEN_BOUND1 (NPART_GEN (n)))
#define NPART_BOUND2(n) (NGEN_BOUND2 (NPART_GEN (n)))
#define NPART_STEP(n) (NGEN_STEP (NPART_GEN (n)))
#define NPART_WIDTH(n) (NGEN_WIDTH (NPART_GEN (n)))

#define NPART_DEFMASK(n) (NPART_MASK (n, 0))
#define NPART_USEMASK(n) (NPART_MASK (n, 1))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ncode :
 ***/

#define NCODE_CBLOCK_INSTR(n) (BLOCK_INSTR (NCODE_CBLOCK (n)))

#define NCODE_DEFMASK(n) (NCODE_MASK (n, 0))
#define NCODE_USEMASK(n) (NCODE_MASK (n, 1))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwithop :
 ***/

#define NWITHOP_OPARG(n)                                                                 \
    (WO_modarray == NWITHOP_TYPE (n)                                                     \
       ? NWITHOP_ARRAY (n)                                                               \
       : WO_genarray == NWITHOP_TYPE (n) ? NWITHOP_SHAPE (n) : NWITHOP_NEUTRAL (n))

#define NWITHOP_DEFMASK(n) (NWITHOP_MASK (n, 0))
#define NWITHOP_USEMASK(n) (NWITHOP_MASK (n, 1))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith2 :
 ***/

#define NWITH2_TYPE(n) (NWITHOP_TYPE (NWITH2_WITHOP (n)))

#define NWITH2_IDS(n) (NWITHID_IDS (NWITH2_WITHID (n)))
#define NWITH2_VEC(n) (NWITHID_VEC (NWITH2_WITHID (n)))

#define NWITH2_FUN(n) (NWITHOP_FUN (NWITH2_WITHOP (n)))
#define NWITH2_MOD(n) (NWITHOP_MOD (NWITH2_WITHOP (n)))
#define NWITH2_FUNDEF(n) (NWITHOP_FUNDEF (NWITH2_WITHOP (n)))
#define NWITH2_SHAPE(n) (NWITHOP_SHAPE (NWITH2_WITHOP (n)))
#define NWITH2_ARRAY(n) (NWITHOP_ARRAY (NWITH2_WITHOP (n)))
#define NWITH2_NEUTRAL(n) (NWITHOP_NEUTRAL (NWITH2_WITHOP (n)))

/*
 * CBLOCK, CEXPR of the *first* N_Ncode-node
 * (useful in case of single-generator with-loops only,
 *  e.g. before with-loop-folding)
 */
#define NWITH2_CBLOCK(n) (NCODE_CBLOCK (NWITH2_CODE (n)))
#define NWITH2_CEXPR(n) (NCODE_CEXPR (NWITH2_CODE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith :  *and*  N_Nwith2 :
 ***/

#define NWITH_OR_NWITH2_TYPE(n)                                                          \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_TYPE (n) : NWITH2_TYPE (n))
#define NWITH_OR_NWITH2_IDS(n)                                                           \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_IDS (n) : NWITH2_IDS (n))
#define NWITH_OR_NWITH2_VEC(n)                                                           \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_VEC (n) : NWITH2_VEC (n))

#define NWITH_OR_NWITH2_IN(n) ((NODE_TYPE (n) == N_Nwith) ? NWITH_IN (n) : NWITH2_IN (n))
#define NWITH_OR_NWITH2_INOUT(n)                                                         \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_INOUT (n) : NWITH2_INOUT (n))
#define NWITH_OR_NWITH2_OUT(n)                                                           \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_OUT (n) : NWITH2_OUT (n))
#define NWITH_OR_NWITH2_LOCAL(n)                                                         \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_LOCAL (n) : NWITH2_LOCAL (n))

#define L_NWITH_OR_NWITH2_IN(n, rhs)                                                     \
    if (NODE_TYPE (n) == N_Nwith) {                                                      \
        NWITH_IN (n) = (rhs);                                                            \
    } else {                                                                             \
        NWITH2_IN (n) = (rhs);                                                           \
    }

#define L_NWITH_OR_NWITH2_INOUT(n, rhs)                                                  \
    if (NODE_TYPE (n) == N_Nwith) {                                                      \
        NWITH_INOUT (n) = (rhs);                                                         \
    } else {                                                                             \
        NWITH2_INOUT (n) = (rhs);                                                        \
    }

#define L_NWITH_OR_NWITH2_OUT(n, rhs)                                                    \
    if (NODE_TYPE (n) == N_Nwith) {                                                      \
        NWITH_OUT (n) = (rhs);                                                           \
    } else {                                                                             \
        NWITH2_OUT (n) = (rhs);                                                          \
    }

#define L_NWITH_OR_NWITH2_LOCAL(n, rhs)                                                  \
    if (NODE_TYPE (n) == N_Nwith) {                                                      \
        NWITH_LOCAL (n) = (rhs);                                                         \
    } else {                                                                             \
        NWITH2_LOCAL (n) = (rhs);                                                        \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLgrid :
 ***/

#define WLGRID_CBLOCK(n) (NCODE_CBLOCK (WLGRID_CODE (n)))
#define WLGRID_CEXPR(n) (NCODE_CEXPR (WLGRID_CODE (n)))

#define WLGRID_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRID_CBLOCK (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLgridVar :
 ***/

#define WLGRIDVAR_CBLOCK(n) (NCODE_CBLOCK (WLGRIDVAR_CODE (n)))
#define WLGRIDVAR_CEXPR(n) (NCODE_CEXPR (WLGRIDVAR_CODE (n)))

#define WLGRIDVAR_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRIDVAR_CBLOCK (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_info :
 ***/

/*
 *  compound access macros
 */

#define INFO_WLAA_ARRAYSHP(n) VARDEC_SHPSEG (INFO_WLAA_WLARRAY (n))
#define INFO_WLAA_INDEXDIM(n) VARDEC_SHAPE (INFO_WLAA_INDEXVAR (n), 0)
#define INFO_WLAA_ARRAYDIM(n) VARDEC_DIM (INFO_WLAA_WLARRAY (n))

#define INFO_TSI_ARRAYSHP(n) VARDEC_SHPSEG (INFO_TSI_WLARRAY (n))
#define INFO_TSI_INDEXDIM(n) VARDEC_SHAPE (INFO_TSI_INDEXVAR (n), 0)
#define INFO_TSI_ARRAYDIM(n) VARDEC_DIM (INFO_TSI_WLARRAY (n))

#endif /* _sac_tree_compound_h */
