/*
 *
 * $Log$
 * Revision 1.54  1998/02/09 15:41:40  sbs
 * forced check in 8-(((
 *  not yet cleaned up!
 *
 * Revision 1.53  1997/12/10 14:24:02  sbs
 * MAKE_BIN_PRF macro inserted !
 *
 * Revision 1.52  1997/11/05 16:30:40  dkr
 * moved nnode[] from tree_compound.[ch] to traverse.[ch]
 *
 * Revision 1.51  1997/11/05 09:38:49  dkr
 * export of nnode[] added
 *
 * Revision 1.50  1997/11/02 13:58:28  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.49  1997/10/29 17:18:32  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.48  1997/05/02 13:53:00  sbs
 * VINFO_SHPSEG inserted
 *
 * Revision 1.47  1997/04/25  13:21:59  sbs
 * CHECK_NULL inserted.
 *
 * Revision 1.46  1997/04/25  09:35:40  sbs
 * () inserted in OBJDEF_LINKNAME & friends
 *
 * Revision 1.45  1997/03/11  16:33:57  cg
 * macro CMP_OBJ_OBJDEF rewritten. . Now, it should be possible to specify
 * >> a module name even for external modules when using a global object.
 * >> .
 *
 * Revision 1.44  1996/08/29  17:46:51  sbs
 * LET_BASETYPE inserted
 *
 * Revision 1.43  1996/05/28  11:30:51  sbs
 * BLOCK_INSTR_OR_ASSIGN_NEXT inserted.
 *
 * Revision 1.42  1996/04/02  19:35:10  cg
 * function string2array moved to sac.y
 *
 * Revision 1.41  1996/03/21  18:00:20  cg
 * added function string2array for new-fashioned string handling
 *
 * Revision 1.40  1996/03/12  17:02:44  hw
 * added macro APPEND_VARDECS
 *
 * Revision 1.39  1996/02/27  15:23:13  hw
 * added macro IDS_VARDEC_TYPE
 *
 * Revision 1.38  1996/02/21  15:03:13  cg
 * added new function CopyNodelist
 *
 * Revision 1.37  1996/02/21  10:56:08  cg
 * macro CMP_FUN_ID drastically simplified
 *
 * Revision 1.36  1996/02/11  20:19:01  sbs
 * some minor corrections on stuff concerning N_vinfo,
 *
 * Revision 1.35  1996/01/22  17:27:50  cg
 * modified OBJDEF_LINKNAME(n), OBJDEF_INITFUN(n), OBJDEF_EFFECT(n)
 *
 * Revision 1.34  1996/01/21  13:56:08  cg
 * added compound access macros for N_icm node
 *
 * Revision 1.33  1996/01/12  15:53:28  asi
 * added LET_VARNO
 *
 * Revision 1.32  1996/01/07  16:55:09  cg
 * function CountFunctionParams now counts return type void
 *
 * Revision 1.31  1996/01/02  12:48:41  cg
 * added function StringsLength
 *
 * Revision 1.30  1995/12/29  16:04:12  asi
 * added ASSIGN_MRDMASK
 *
 * Revision 1.29  1995/12/29  14:49:00  asi
 * added ID_VARNO
 *
 * Revision 1.28  1995/12/29  12:53:23  cg
 * added function StoreString
 *
 * Revision 1.27  1995/12/29  10:34:19  cg
 * added ConcatNodelist
 * added TDEF compound access macros for fundef, typedef, etc
 *
 * Revision 1.26  1995/12/21  13:25:55  asi
 * added WITH_OPERATORDEFMASK, WITH_OPERATORUSEMASK, OPERATOR_DEFMASK and OPERATOR_USEMASK
 *
 * Revision 1.25  1995/12/21  10:36:55  cg
 * added function CountFunctionParams
 *
 * Revision 1.24  1995/12/20  08:16:50  cg
 * added compound access macros for N_pragma node
 *
 * Revision 1.23  1995/12/18  14:41:51  asi
 * added DO_DEFMASK, DO_USEMASK, DO_TERMMASK, DO_INSTR,
 *       WHILE_DEFMASK, WHILE_USEMASK, WHILE_TERMMASK, WHILE_INSTR
 *
 * Revision 1.22  1995/12/15  14:13:56  asi
 * added GetCompoundNode
 *
 * Revision 1.21  1995/12/15  13:22:12  asi
 * added COND_THENINSTR and COND_ELSEINSTR
 *
 * Revision 1.20  1995/12/13  17:32:37  asi
 * added ASSIGN_INSTRTYPE
 *
 * Revision 1.19  1995/12/12  15:49:08  hw
 * added macros LET_NAME, LET_MOD, LET_STATUS
 *
 * Revision 1.18  1995/12/07  16:24:46  asi
 * added function MakeAssignLet
 *
 * Revision 1.17  1995/12/01  17:10:45  cg
 * new compound access macros for pragmas of functions,
 * objects and types.
 *
 * Revision 1.16  1995/11/16  19:41:43  cg
 * new compound access macros for masks.
 * Function FreeNodelist moved to free.c
 *
 * Revision 1.15  1995/11/03  16:04:48  cg
 * no changes
 *
 * Revision 1.14  1995/11/02  16:26:28  cg
 * new compound access macros FUNDEF_VARDEC and FUNDEF_INSTR.
 *
 * Revision 1.13  1995/11/01  16:25:01  cg
 * new function AppendIdsChain from tree.c and converted to new macros
 *
 * Revision 1.12  1995/10/31  08:54:43  cg
 * added new functions FreeNodelist and TidyUpNodelist.
 *
 * Revision 1.11  1995/10/26  16:01:44  cg
 *  macro MOD_NAME_CON replaced by new global variable mod_name_con
 *  Now, different strings can be used for combining module name and
 * item name with respect to the compilation phase.
 * Macro CMP_TYPE_USER_NONSTRICT removed.
 * New macro NODE_NEXT(n)
 *
 * Revision 1.10  1995/10/24  13:13:31  cg
 * new macro CMP_TYPE_USER_NONSTRICT
 *
 * Revision 1.9  1995/10/22  17:29:59  cg
 * new function SearchObjdef
 * new compound access macros for fundec and typedec
 * macro CMP_TYPE_USER now tests if argument actually is T_user.
 *
 * Revision 1.8  1995/10/20  16:52:35  cg
 * functions InsertNode, InsertNodes, and InsertUnresolvedNodes
 * transformed into void functions and renamed into
 * StoreNeededNode, StoreNeededNodes, and StoreUnresolvedNodes
 * respectively.
 *
 * Revision 1.7  1995/10/20  13:46:47  cg
 * added additional parameter in functions InsertNode, InsertNodes,
 * and InsertUnresolvedNodes.
 * Now the status of the nodelist entry can be given as well.
 *
 * Revision 1.6  1995/10/19  10:07:51  cg
 * functions InsertNode, InsertNodes and InsertUnresolvedNodes
 * modified in signature.
 *
 * Revision 1.5  1995/10/16  12:28:56  cg
 * Bugs in macros FUNDEF_NEEDFUNS and FUNDEF_NEEDTYPES fixed.
 *
 * Revision 1.4  1995/10/06  17:19:36  cg
 * functions InsertNode InsertNodes and InsertUnresolvedNodes for dealing with
 * type nodelist added.
 *
 * Revision 1.3  1995/10/01  16:40:31  cg
 * function SearchFundef added.
 *
 * Revision 1.2  1995/10/01  13:04:32  cg
 * added external declarations for SearchTypedef, CountNums, CopyShpseg,
 *  MergeShpseg
 * added new compound macros for Shape-Access.
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

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* macros for module name access                                            */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#define MOD_NAME_CON "__"

#define MOD(a) ((NULL == a) ? "" : a)
#define MOD_CON(a) ((NULL == a) ? "" : MOD_NAME_CON)
#define MOD_NAME(a) MOD (a), MOD_CON (a)

extern char *mod_name_con;
extern char mod_name_con_1[];
extern char mod_name_con_2[];

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
 ***  SHAPES :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***/

/*
 *
 *  functionname  : CopyShpseg
 *  arguments     : 1) pointer to old shpseg
 *  description   : returns a copy of the given shpseg
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       : returns NULL if old shpseg is NULL
 *
 */

extern shpseg *CopyShpseg (shpseg *old);

/*
 *
 *  functionname  : MergeShpseg
 *  arguments     : 1) pointer to first shpseg
 *                  2) dimension of first shpseg
 *                  3) pointer to second shpseg
 *                  4) dimension of second shpseg
 *  description   : returns a new shpseg that starts with the shapes of
 *                  the first shpseg and ends with the shapes of the
 *                  second shpseg
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

extern shpseg *MergeShpseg (shpseg *first, int dim1, shpseg *second, int dim2);

/*--------------------------------------------------------------------------*/

/***
 ***  TYPES :
 ***/

/*
 *  compound access macros
 */

#define TYPES_SHAPE(t, x) (SHPSEG_SHAPE (TYPES_SHPSEG (t), x))

/*
 *
 *  macro name    : CMP_TYPE_USER
 *  arg types     : 1) types*
 *                  2) types*
 *  result type   : int
 *  description   : compares two user-defined types (name and module)
 *                  Names and module names must be equal.
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       : result: 1 - equal, 0 - not equal
 *
 */

#define CMP_TYPE_USER(a, b)                                                              \
                                                                                         \
    ((!strcmp (TYPES_NAME (a), TYPES_NAME (b)))                                          \
     && (!strcmp (MOD (TYPES_MOD (a)), MOD (TYPES_MOD (b)))))

/*--------------------------------------------------------------------------*/

/***
 ***  IDS :
 ***/

#define IDS_VARNO(n) VARDEC_VARNO (IDS_VARDEC (n))

#define IDS_VARDEC_TYPE(n) VARDEC_TYPE (IDS_VARDEC (n))

/*
 *
 *  macro name    : IDS_IS_UNIQUE
 *  arg types     : 1) ids*
 *  result type   : int (bool)
 *  description   : checks if the given ids is unique or not
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       : This macro will only work properly with the VARNO
 *                  settings done in uniquecheck.c
 *
 *
 *
 */

#define IDS_IS_UNIQUE(i)                                                                 \
    ((NODE_TYPE (IDS_VARDEC (i)) == N_vardec) ? (VARDEC_VARNO (IDS_VARDEC (i)) >= 0)     \
                                              : (ARG_VARNO (IDS_VARDEC (i)) >= 0))

/*
 *
 *  functionname  : AppendIdsChain
 *  arguments     : 1) first ids chain
 *                  2) second ids chain to be appended after first one
 *  description   : follows first chain to it's end and
 *                  appends second.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

extern ids *AppendIdsChain (ids *first, ids *second);

/*--------------------------------------------------------------------------*/

/***
 ***  NUMS :
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
 ***  STRINGS :
 ***/

/*
 *
 *  functionname  : StoreString
 *  arguments     : 1) list of strings
 *                  2) string to store in 1)
 *  description   : adds 2) to 1) if it's not yet in the list
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcmp
 *  macros        :
 *
 *  remarks       : The strings are not copied !!
 *
 */

extern strings *AddToLinkList (strings *list, char *str);

/*
 *
 *  functionname  : StringsLength
 *  arguments     : 1) list of strings
 *  description   : counts the total length of all strings
 *                  in the given list including the terminating 0.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strlen
 *  macros        : ---
 *
 *  remarks       :
 *
 *
 */

extern int StringsLength (strings *list);

/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
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

/*--------------------------------------------------------------------------*/

/***
 ***  OTHERS :
 ***/

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
     && (!strcmp (MOD (mod), MOD (TYPEDEF_MOD (tdef)))))

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
 *
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
 *
 */

extern int CmpDomain (node *args1, node *args2);

/*
 *
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
 *
 */

extern node *SearchFundef (node *fun, node *allfuns);

/*
 *
 *  functionname  : ObjList2ArgList
 *  arguments     : 1) pointer to chain of objdef nodes
 *  description   : makes an argument list from an objdef chain
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

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

#if 0
#define CMP_FUN_ID(a, b)                                                                 \
                                                                                         \
    ((NULL == FUNDEF_MOD (a))                                                            \
       ? (!strcmp (FUNDEF_NAME (a), FUNDEF_NAME (b)) && (NULL == FUNDEF_MOD (b)))        \
       : ((NULL == FUNDEF_MOD (b)) ? 0                                                   \
                                   : ((!strcmp (FUNDEF_NAME (a), FUNDEF_NAME (b)))       \
                                      && (!strcmp (FUNDEF_MOD (a), FUNDEF_MOD (b))))))
#endif

#define CMP_FUN_ID(a, b)                                                                 \
                                                                                         \
    ((0 == strcmp (FUNDEF_NAME (a), FUNDEF_NAME (b)))                                    \
     && (0 == strcmp (MOD (FUNDEF_MOD (a)), MOD (FUNDEF_MOD (b)))))

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

#define ARG_BASETYPE(n) (TYPES_BASETYPE (ARG_TYPE (n)))
#define ARG_DIM(n) (TYPES_DIM (ARG_TYPE (n)))
#define ARG_SHAPE(n, x) (TYPES_SHAPE (ARG_TYPE (n), x))
#define ARG_SHPSEG(n) (TYPES_SHPSEG (ARG_TYPE (n)))
#define ARG_TNAME(n) (TYPES_NAME (ARG_TYPE (n)))
#define ARG_TMOD(n) (TYPES_MOD (ARG_TYPE (n)))
#define ARG_TDEF(n) (TYPES_TDEF (ARG_TYPE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_block :
 ***/

/*
 *  compound access macros
 */

#define BLOCK_DEFMASK(n) (BLOCK_MASK (n, 0))
#define BLOCK_USEMASK(n) (BLOCK_MASK (n, 1))

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
 ***  N_assign :
 ***/

/*
 *  compound access macros
 */

#define ASSIGN_DEFMASK(n) (ASSIGN_MASK (n, 0))
#define ASSIGN_USEMASK(n) (ASSIGN_MASK (n, 1))
#define ASSIGN_MRDMASK(n) (ASSIGN_MASK (n, 2))
#define ASSIGN_INSTRTYPE(n) (NODE_TYPE (ASSIGN_INSTR (n)))

/*
 *
 *  functionname  : MakeAssignLet
 *  arguments     : 1) name of the variable
 *                  2) vardec-node
 *                  3) let-expression
 *                  R) assign-node with complete let-subtree
 *  description   : returns a assign-node with let-node : var_name = expr;
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeIds, MakeLet, MakeAssign
 *  macros        : IDS_VARDEC, NULL
 *
 *  remarks       : the nnode's will get the new value !!!
 *                  ASSIGN_NEXT is set to NULL
 *
 */
extern node *MakeAssignLet (char *var_name, node *vardec_node, node *let_expr);

/*
 *
 *  functionname  : GetCompoundNode
 *  arguments     : 1) assign-node
 *                  R) compund_node attached to the assign-node
 *  description   : returns the compund_node that is attached to the assign-node
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ASSIGN_INSTR, LET_EXPR, NODE_TYPE, CAST_EXPR
 *
 *  remarks       : ---
 *
 */
extern node *GetCompoundNode (node *arg_node);

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :
 ***/
/*
 *  compound access macros
 */

#define LET_NAME(n) (IDS_NAME (LET_IDS (n)))
#define LET_MOD(n) (IDS_MOD (LET_IDS (n)))
#define LET_STATUS(n) (IDS_STATUS (LET_IDS (n)))
#define LET_VARNO(n) (VARDEC_VARNO (LET_VARDEC (n)))
#define LET_BASETYPE(n) (TYPES_BASETYPE (VARDEC_TYPE (LET_VARDEC (n))))
#define LET_USE(n) (IDS_USE (LET_IDS (n)))

#define MAKE_OPON_LET(id, expr, op)                                                      \
    MakeLet (MAKE_BIN_PRF (op, MakeId (id, NULL, ST_regular), expr),                     \
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
#define WHILE_TERMMASK(n) (WHILE_MASK (n, 1))
#define WHILE_INSTR(n) (BLOCK_INSTR (WHILE_BODY (n)))

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
 ***  N_exprs :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :
 ***/

/*
 *  compound access macros
 */

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

/*--------------------------------------------------------------------------*/

/***
 ***  N_id :
 ***/

/*
 *  compound access macros
 */

#define ID_VARNO(n) VARDEC_VARNO (ID_VARDEC (n))
#define ID_TYPE(n) VARDEC_TYPE (ID_VARDEC (n))

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

#define ICM_ARG1(n) EXPRS_EXPR (ICM_ARGS (n))
#define ICM_ARG2(n) EXPRS_EXPR (EXPRS_NEXT (ICM_ARGS (n)))
#define ICM_ARG3(n) EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (n))))
#define ICM_ARG4(n) EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (n)))))

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

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

#endif /* _sac_tree_compound_h */
