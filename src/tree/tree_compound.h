/*
 *
 * $Log$
 * Revision 1.4  2000/02/02 12:29:09  jhs
 * Added INFO_MUTH_FUNDEF.
 * Added N_mt and N_st.
 * Added ST_xxx-macros, added MT_OR_ST_xxx-macros.
 *
 * Revision 1.3  2000/01/26 13:47:00  jhs
 * L_BLOCK_INSTR_OR_ASSIGN_NEXT added.
 *
 * Revision 1.2  2000/01/25 13:40:34  dkr
 * some rearrangements made
 * all the constvec stuff moved from internal_lib.h
 * function FindVardec_Name and FindVardec_Varno added
 *
 * Revision 1.1  2000/01/21 15:38:29  dkr
 * Initial revision
 *
 * Revision 2.25  1999/12/01 15:20:55  dkr
 * macro VARDEC_OR_ARG_SHPSEG added
 *
 * Revision 2.24  1999/11/11 20:09:13  dkr
 * function IsConstantArray changed:
 *   new name (IsConstArray)
 *   new signature (no type parameter anymore)
 *   new return value (boolean instead of #elements)
 *   NOW THIS FUNCTION (hopefully) works correct :)
 *
 * Revision 2.23  1999/11/11 18:27:15  dkr
 * added NWITH_WITHID
 *
 * Revision 2.22  1999/11/09 21:15:57  dkr
 * Functions MakeIcm7, MakeAssignIcm7 added.
 *
 * Revision 2.21  1999/10/28 17:06:54  dkr
 * compound macros for masks added
 *
 * Revision 2.20  1999/10/27 15:12:41  sbs
 * MakeAssignIcmX(...) added.
 *
 * Revision 2.19  1999/09/20 11:33:32  jhs
 * Added (L_)VARDEC_OR_ARG_BNEXT.
 * Brushed MakeIcm(0..6).
 *
 * Revision 2.18  1999/09/10 14:24:39  jhs
 * Added some DO_OR_WHILE_xxx macros to access both type of loops with one
 * type of macro.
 *
 * Revision 2.17  1999/09/01 12:40:34  sbs
 * typo corrected
 *
 * Revision 2.16  1999/09/01 12:37:56  sbs
 * SET_VARDEC_OR_ARG_COLCHN added.
 *
 * Revision 2.15  1999/09/01 11:49:38  sbs
 * VARDEC_OR_ARG_ACTCHN and SET_VARDEC_OR_ARG_ACTCHN added!
 *
 * Revision 2.14  1999/08/09 15:45:27  dkr
 * L_VARDEC_OR_ARG_REFCNT, ... added
 *
 * Revision 2.13  1999/07/14 12:14:36  sbs
 * proper support for arg_info during IVE added.
 *
 * Revision 2.12  1999/07/13 20:26:28  sbs
 * VARDEC_OR_ARG_MOD added
 *
 * Revision 2.11  1999/07/13 16:41:17  sbs
 * VARDEC_OR_ARG_COLCHN added.
 * ./
 *
 * Revision 2.10  1999/07/13 16:27:16  sbs
 * VARDEC_OR_ARG_SHAPE added.
 *
 * Revision 2.9  1999/07/08 14:59:12  sbs
 * Array2BoolVec added
 *
 * Revision 2.8  1999/07/07 05:59:33  sbs
 * Added function MakeVinfoDollar
 *
 * Revision 2.7  1999/06/17 14:31:34  sbs
 * added new compound-macro TYPES_DIM
 *
 * Revision 2.6  1999/06/15 12:31:13  jhs
 * Added VARDEC_OR_ARG_NAIVE_REFCNT.
 *
 * Revision 2.5  1999/05/12 08:34:07  sbs
 * some logging info eliminated
 *
 * Revision 2.4  1999/03/31 10:51:57  bs
 *  Function Array2CharVec added.
 *
 * Revision 2.3  1999/03/15 14:10:13  bs
 * Access macros renamed (take a look at tree_basic.h).
 * Functions Array2FloatVec and Array2Dblvec added.
 *
 * Revision 2.2  1999/03/04 10:08:02  bs
 * Functions IntVec2Array() and Array2IntVec added.
 *
 * Revision 2.1  1999/02/23 12:40:11  sacbase
 * new release made
 *
 * Revision 1.79  1999/02/06 12:50:27  srs
 * declared four functions to handle Nodelist
 *
 * Revision 1.78  1999/01/26 14:25:46  cg
 * Added functions MakePrf1(), MakePrf2(), and MakePrf3() to
 * create N_prf nodes with 1,2, or 3 arguments, repsectively.
 *
 * Revision 1.77  1999/01/15 15:18:55  cg
 * added compound macro for type access from ids structures and
 * N_id nodes.
 *
 * Revision 1.76  1998/07/16 13:49:30  sbs
 * LET_TYPE inserted
 *
 * Revision 1.75  1998/07/03 10:16:38  cg
 * new functions MakeIcm[012345] added
 * function AppendExpr renamed to AppendExprs
 *
 * Revision 1.74  1998/06/07 18:39:29  dkr
 * addded NWITH2_VEC
 *
 * Revision 1.73  1998/06/05 15:27:49  cg
 * global variable mod_name_con and macros MOD_NAME_CON MOD MOD_NAME MOD_CON removed
 * Now, module name and symbol name are combined correctly by ':'.
 * Only when it really comes to the generation of C code, the ':' is
 * replaced by '__'. This is done by the renaming of all identifiers
 * during the precompilation phase.
 *
 * Revision 1.72  1998/06/04 16:59:20  cg
 * added function LookupIds that looks for a given identifier
 * within an ids-chain.
 *
 * Revision 1.71  1998/05/27 13:17:03  sbs
 * Nwith-loop MACROS BRUSHED
 *
 * Revision 1.70  1998/05/16 19:47:07  dkr
 * added some WLGRID..., WLSTRIDE... macros
 *
 * Revision 1.69  1998/05/12 12:20:19  dkr
 * added AppendAssign, AppendExpr
 *
 * Revision 1.68  1998/05/07 21:38:46  dkr
 * added VARDEC_OR_ARG_TYPE
 *
 * Revision 1.67  1998/05/06 17:20:31  dkr
 * added macros NWITH_IDS, NWITH_VEC
 *
 * Revision 1.66  1998/05/04 17:56:41  sbs
 * LET_VARDEC(n) moved from tree_basic.h to tree_compound.h!!!!
 *
 * Revision 1.65  1998/05/02 17:44:52  dkr
 * added macros VARDEC_OR_ARG_...
 *
 * Revision 1.64  1998/04/28 15:34:51  dkr
 * added compound macros for N_Nwith2
 *
 * Revision 1.63  1998/04/24 17:19:56  dkr
 * added compound macros for N_spmd
 *
 * Revision 1.62  1998/03/25 18:10:40  srs
 * renamed IDS_VARDEV_TYPE to IDS_TYPE
 *
 * Revision 1.61  1998/03/23 18:59:20  srs
 * added macro NWITH_TYPE()
 *
 * Revision 1.60  1998/03/12 14:05:32  srs
 * new macro IDS_SHAPE
 *
 * Revision 1.59  1998/03/10 11:10:47  srs
 * changed parameters of IsConstantArray
 *
 * Revision 1.58  1998/03/07 17:00:55  srs
 * added IsConstantArray() and ID_SHAPE
 *
 * Revision 1.57  1998/03/02 13:55:08  srs
 * new macro MAKE_INCDEC_LET
 *
 * Revision 1.56  1998/02/23 13:05:37  srs
 * added macro for new WL
 *
 * Revision 1.55  1998/02/12 11:06:39  srs
 * added FUNDEF_BODY_VARNO and two compound macros for the new WL
 *
 * Revision 1.54  1998/02/09 15:41:40  sbs
 * forced check in 8-(((
 *  not yet cleaned up!
 *
 * ... [eliminated] ...
 *
 * Revision 1.1  1995/09/27  15:13:12  cg
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
/*  macros and functions for non-node structures                            */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/***
 ***  Shpseg :
 ***/

/*
 *  functionname  : CopyShpseg
 *  arguments     : 1) pointer to old shpseg
 *  description   : returns a copy of the given shpseg
 *  remarks       : returns NULL if old shpseg is NULL
 */

extern shpseg *CopyShpseg (shpseg *old);

/*
 *  functionname  : MergeShpseg
 *  arguments     : 1) pointer to first shpseg
 *                  2) dimension of first shpseg
 *                  3) pointer to second shpseg
 *                  4) dimension of second shpseg
 *  description   : returns a new shpseg that starts with the shapes of
 *                  the first shpseg and ends with the shapes of the
 *                  second shpseg
 */

extern shpseg *MergeShpseg (shpseg *first, int dim1, shpseg *second, int dim2);

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

/*--------------------------------------------------------------------------*/

/***
 ***  Ids :
 ***/

#define IDS_VARNO(n) VARDEC_OR_ARG_VARNO (IDS_VARDEC (n))
#define IDS_TYPE(n) VARDEC_OR_ARG_TYPE (IDS_VARDEC (n))
#define IDS_DIM(n) VARDEC_OR_ARG_DIM (IDS_VARDEC (n))
#define IDS_SHAPE(n, x)                                                                  \
    SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_OR_ARG_TYPE (IDS_VARDEC (n))), x)

/*
 *  macro name    : IDS_IS_UNIQUE
 *  arg types     : 1) ids*
 *  result type   : int (bool)
 *  description   : checks if the given ids is unique or not
 *  remarks       : This macro will only work properly with the VARNO
 *                  settings done in uniquecheck.c
 */

#define IDS_IS_UNIQUE(i)                                                                 \
    ((NODE_TYPE (IDS_VARDEC (i)) == N_vardec) ? (VARDEC_VARNO (IDS_VARDEC (i)) >= 0)     \
                                              : (ARG_VARNO (IDS_VARDEC (i)) >= 0))

/*
 *  functionname  : AppendIdsChain
 *  arguments     : 1) first ids chain
 *                  2) second ids chain to be appended after first one
 *  description   : follows first chain to it's end and
 *                  appends second.
 */

extern ids *AppendIdsChain (ids *first, ids *second);

/******************************************************************************
 *
 * function:
 *   ids *LookupIds(char *name, ids *ids_chain)
 *
 * description:
 *
 *   This function searches for a given identifier name within an ids-chain
 *   of identifiers and returns the ids-structure if found or NULL otherwise.
 *
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

extern int CountNums (nums *numsp);

/*--------------------------------------------------------------------------*/

/***
 ***  Constvec :
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
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
 *  global vars   : ---
 *  internal funs : StoreNeededNode
 *  external funs : ---
 *  macros        :
 *
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
 *  global vars   : ---
 *  internal funs : StoreNeededNode
 *  external funs : ---
 *  macros        :
 *
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : free
 *  macros        : DBUG, TREE
 *
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

extern nodelist *ConcatNodelist (nodelist *first, nodelist *second);

/*
 *
 *  functionname  : CopyNodelist
 *  arguments     : 1) node list
 *  description   : copies an entire node list
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNodelist, CopyNodelist
 *  macros        : ---
 *
 *  remarks       :
 *
 */

extern nodelist *CopyNodelist (nodelist *nl);

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
 *  functionname  : SearchTypedef
 *  arguments     : 1) type name to be searched for
 *                  2) module name of type to be searched for
 *                  3) list of type implementations (typedef nodes)
 *  description   : looks for a certain typedef in list of typedefs
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : CMP_TYPE_TYPEDEF
 *
 *  remarks       :
 *
 */

extern node *SearchTypedef (char *name, char *mod, node *implementations);

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
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       :
 *
 */

#define CMP_TYPE_TYPEDEF(name, mod, tdef)                                                \
    ((!strcmp (name, TYPEDEF_NAME (tdef)))                                               \
     && (!strcmp (CHECK_NULL (mod), CHECK_NULL (TYPEDEF_MOD (tdef)))))

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
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       :
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
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : CMP_OBJ_OBJDEF
 *
 *  remarks       :
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
 *  functionname  : CmpDomain
 *  arguments     : 1) N_arg node of one function
 *                  2) N_arg node of another function
 *  description   : checks whether the functions have equal domain
 *                  returns 1 if domain is equal, 0 else
 *  global vars   : ----
 *  internal funs : ----
 *  external funs : ----
 *  macros        : CMP_TYPE_USER
 *
 *  remarks       : similar to function CmpFunParams of typechecker.
 *                  some minor changes to fix appearing segmentation
 *                  faults.
 */

extern int CmpDomain (node *args1, node *args2);

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

extern node *FindVardec_Name (char *name, node *fundef);
extern node *FindVardec_Varno (int varno, node *fundef);

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

#define APPEND_VARDECS(old, new)                                                         \
    if (NULL != old) {                                                                   \
        VARDEC_NEXT (old) = new;                                                         \
    } else                                                                               \
        old = new

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

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec / N_arg :
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
#define VARDEC_OR_ARG_NEXT(n) ((NODE_TYPE (n) == N_arg) ? ARG_NEXT (n) : VARDEC_NEXT (n))

#define L_VARDEC_OR_ARG_NEXT(n, rhs)                                                     \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_NEXT (n) = (rhs);                                                            \
    } else {                                                                             \
        VARDEC_NEXT (n) = (rhs);                                                         \
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

#define SET_VARDEC_OR_ARG_COLCHN(n, rhs)                                                 \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_COLCHN (n) = (rhs);                                                          \
    } else {                                                                             \
        VARDEC_COLCHN (n) = (rhs);                                                       \
    }

#define SET_VARDEC_OR_ARG_ACTCHN(n, rhs)                                                 \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_ACTCHN (n) = (rhs);                                                          \
    } else {                                                                             \
        VARDEC_ACTCHN (n) = (rhs);                                                       \
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
 *
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
 *   node *AppendAssign( node *assigns, node *assign)
 *
 * description:
 *   appends 'assign' to the N_assign-chain 'assings' and returns the new
 *    chain.
 *
 ******************************************************************************/

extern node *AppendAssign (node *assigns, node *assign);

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
 ***/

/******************************************************************************
 *
 * function:
 *   node *AppendExprs( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_exprs chains of nodes.
 *
 ******************************************************************************/

extern node *AppendExprs (node *exprs1, node *exprs2);

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
#define LET_VARNO(n) (VARDEC_VARNO (LET_VARDEC (n)))
#define LET_TYPE(n) (VARDEC_TYPE (LET_VARDEC (n)))
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
 ***  N_ap :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_generator :
 ***/

/*
 *  compound access macros
 */

#define GEN_DEFMASK(n) (GEN_MASK (n, 0))
#define GEN_USEMASK(n) (GEN_MASK (n, 1))

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

#define WITH_GENDEFMASK(n) (GEN_DEFMASK (WITH_GEN (n)))
#define WITH_GENUSEMASK(n) (GEN_USEMASK (WITH_GEN (n)))

#define WITH_OPERATORDEFMASK(n) (OPERATOR_USEMASK (WITH_OPERATOR (n)))
#define WITH_OPERATORUSEMASK(n) (OPERATOR_USEMASK (WITH_OPERATOR (n)))

#define WITH_BODYDEFMASK(n) (WITH_MASK (n, 0))
#define WITH_BODYUSEMASK(n) (WITH_MASK (n, 0))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith :
 ***/

#define NWITH_TYPE(n) (NWITHOP_TYPE (NWITH_WITHOP (n)))
#define NWITH_WITHID(n) (NPART_WITHID (NWITH_PART (n)))
#define NWITH_IDS(n) (NPART_IDS (NWITH_PART (n)))
#define NWITH_VEC(n) (NPART_VEC (NWITH_PART (n)))
#define NWITH_BOUND2(n) (NPART_BOUND2 (NWITH_PART (n)))
/*
 * remark: We only need to inspect the withid of the first part,
 *         because the withid is in *all* parts the same!!
 */
#define NWITH_CEXPR(n) (NCODE_CEXPR (NWITH_CODE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Npart:
 ***/

#define NPART_IDS(n) (NWITHID_IDS (NPART_WITHID (n)))
#define NPART_VEC(n) (NWITHID_VEC (NPART_WITHID (n)))
#define NPART_BOUND2(n) (NGEN_BOUND2 (NPART_GEN (n)))

#define NPART_DEFMASK(n) (NPART_MASK (n, 0))
#define NPART_USEMASK(n) (NPART_MASK (n, 1))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ncode :
 ***/

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
#define NWITH2_VEC(n) (NWITHID_VEC (NWITH2_WITHID (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WL...:
 ***/

#define WLSTRIANY_CONTENTS(n)                                                            \
    ((NODE_TYPE (n) == N_WLstride) ? (WLSTRIDE_CONTENTS (n)) : (WLSTRIVAR_CONTENTS (n)))

#define WLGRIDANY_NEXTDIM(n)                                                             \
    ((NODE_TYPE (n) == N_WLgrid) ? (WLGRID_NEXTDIM (n)) : (WLGRIDVAR_NEXTDIM (n)))

#define WLGRIDANY_CODE(n)                                                                \
    ((NODE_TYPE (n) == N_WLgrid) ? (WLGRID_CODE (n)) : (WLGRIDVAR_CODE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_genarray :
 ***/

#define OPERATOR_DEFMASK(n) OPERATOR_MASK (n, 0)
#define OPERATOR_USEMASK(n) OPERATOR_MASK (n, 1)

/*--------------------------------------------------------------------------*/

/***
 ***  N_modarray :
 ***/

/* see N_genarray for OPERATOR_DEFMASK and OPERATOR_USEMASK */

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldprf :
 ***/

/* see N_genarray for OPERATOR_DEFMASK and OPERATOR_USEMASK */

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldfun :
 ***/

/* see N_genarray for OPERATOR_DEFMASK and OPERATOR_USEMASK */

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :
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

extern node *Shape2Array (shapes *shp);

extern int IsConstArray (node *array);

/* description:
 *   Returns number of constant elements if argument is an N_array and all
 *   its elements are N_num, N_char, N_float, N_double, N_bool or otherwise
 *   returns 0.
 *
 *   The parameter type specified the necessary type all elements have to
 *   be of (nodetype, e.g. N_num). If N_ok is given, the type is ignored.*/

extern node *IntVec2Array (int length, int *intvec);

/* description:
 *   Returns an N_exprs node containing the elements of intvec. */

extern int *Array2IntVec (node *aelems, int *length);
extern int *Array2BoolVec (node *aelems, int *length);
extern char *Array2CharVec (node *aelems, int *length);
extern float *Array2FloatVec (node *aelems, int *length);
extern double *Array2DblVec (node *aelems, int *length);

/* description:
 *   Returns an iteger (char | float | double) vector and stores the number of
 *   constant integer (char | float | double) elements in *length if first
 *   argument is an N_exprs and all its elements are N_num otherwise the
 *   result is not defined.
 *   If the length of the vector is not of interest, length may be NULL.
 */

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

#define MAKE_BIN_PRF(f, arg1, arg2) MakePrf (f, MakeExprs (arg1, MakeExprs (arg2, NULL)))

/*
 *  function declarations
 */

extern node *MakePrf1 (prf prf, node *arg1);
extern node *MakePrf2 (prf prf, node *arg1, node *arg2);
extern node *MakePrf3 (prf prf, node *arg1, node *arg2, node *arg3);

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
 ***  N_mt :   N_st :
 ***/

#define MT_OR_ST_REGION(n) ((NODE_TYPE (n) == N_mt)) ? MT_REGION (n) : ST_REGION (n)

#define L_MT_OR_ST_REGION(n, region)                                                     \
    if (NODE_TYPE (n) == N_mt) {                                                         \
        MT_REGION (n) = region;                                                          \
    } else {                                                                             \
        ST_REGION (n) = region;                                                          \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_icm :
 ***/

/*
 *  compound access macros
 */

#define ICM_ARG1(n) EXPRS_EXPR (ICM_ARGS (n))
#define ICM_ARG2(n) EXPRS_EXPR (EXPRS_NEXT (ICM_ARGS (n)))
#define ICM_ARG3(n) EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (n))))
#define ICM_ARG4(n) EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (n)))))

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
 *
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

#endif /* _sac_tree_compound_h */
