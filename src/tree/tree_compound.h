/*
 *
 * $Log$
 * Revision 3.66  2002/09/03 11:09:07  dkr
 * signature of CompareTypesImplementation() modified
 *
 * Revision 3.65  2002/08/05 17:03:45  sbs
 * several extensions required for the alpha version of the new type checker
 *
 * Revision 3.64  2002/07/15 17:25:02  dkr
 * LiftArg() moved from precompile.c to tree_compound.[ch]
 *
 * Revision 3.63  2002/06/27 16:56:26  dkr
 * signature of CreateSel() modified
 *
 * Revision 3.62  2002/06/27 13:31:47  dkr
 * Ids2Array() added
 *
 * Revision 3.61  2002/06/27 12:51:29  dkr
 * signature of CreateSel() modified
 *
 * Revision 3.60  2002/06/27 10:59:52  dkr
 * - CreateScalarWith() and CreateSel() added
 * - bug in CreateScalarWith() fixed
 *
 * Revision 3.59  2002/06/25 23:52:06  ktr
 * NPART_CEXPR and NPART_CBLOCK added.
 *
 * Revision 3.58  2002/06/20 15:24:56  dkr
 * CreateZeroFromType(), CreateZero() added
 * AddVardecs() added
 *
 * Revision 3.57  2002/04/29 15:59:34  sbs
 * function HasDotTypes added.
 *
 * Revision 3.56  2002/03/07 16:42:03  sbs
 * HasDotArgs added.
 *
 * Revision 3.55  2002/03/01 02:35:14  dkr
 * type ARGTAB added
 *
 * Revision 3.54  2002/02/22 13:56:09  dkr
 * L_NWITH_OR_NWITH2_DEC_RC_IDS added
 *
 * Revision 3.53  2002/02/21 14:38:00  dkr
 * some VARDEC_OR_ARG_... macros added
 *
 * Revision 3.52  2002/02/20 17:03:54  dkr
 * some FUNDEF_... macros added or removed
 *
 * Revision 3.51  2001/12/13 15:15:44  dkr
 * signature of MakeAssignIcm?() modified
 *
 * Revision 3.50  2001/12/12 14:33:14  dkr
 * function CombineExprs() added
 *
 * Revision 3.49  2001/12/11 15:58:21  dkr
 * GetDim() renamed into GetShapeDim()
 * GetDim() added
 *
 * Revision 3.48  2001/12/10 13:45:58  dkr
 * function MakeAssignInstr() added
 * functions MakeAssigns?() added
 *
 * Revision 3.47  2001/07/19 16:19:57  cg
 * Added new inquiery functions IsImported, IsExternal, IsFromModule
 * and IsFromClass to avoid clumsy direct checks for import status
 * of types, objects, and functions.
 *
 * Revision 3.46  2001/07/18 12:57:45  cg
 * Function ExprsConcat renamed to AppendExprs.
 *
 * Revision 3.45  2001/07/17 15:12:12  cg
 * Some compound macros moved from tree_basic.h to tree_compound.h
 *
 * Revision 3.44  2001/06/01 14:46:51  dkr
 * macro NWITH_OR_NWITH2_DEC_RC_IDS added
 *
 * Revision 3.43  2001/05/31 14:50:43  nmw
 * CompareTypesImplementation() added
 *
 * Revision 3.42  2001/04/26 21:06:45  dkr
 * L_VARDEC_OR_ARG_TYPE added
 *
 * Revision 3.41  2001/04/26 13:10:16  dkr
 * CountIds() added
 *
 * Revision 3.40  2001/04/26 12:18:42  dkr
 * GetExprsLength() renamed into CountExprs()
 *
 * Revision 3.39  2001/04/24 20:08:08  dkr
 * macros ASSIGN_LHS, ASSIGN_RHS added
 *
 * Revision 3.38  2001/04/24 09:35:06  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 *
 * Revision 3.37  2001/04/17 15:26:37  nmw
 * AppendTypes added
 *
 * Revision 3.36  2001/04/09 15:56:33  nmw
 * MakeArgFromVardec added
 *
 * Revision 3.35  2001/04/06 16:05:10  dkr
 * CountTypes(), CountArgs() added
 *
 * Revision 3.34  2001/04/04 19:41:45  dkr
 * FUNDEF_DOES_REFCOUNT and FUNDEF_WANTS_REFCOUNT added
 *
 * Revision 3.33  2001/04/04 09:58:30  nmw
 * AdjustAvisData added
 *
 * Revision 3.32  2001/04/02 15:20:03  dkr
 * macros FUNDEF_IS_LACFUN, FUNDEF_IS_CONDFUN, FUNDEF_IS_LOOPFUN added
 *
 * Revision 3.31  2001/04/02 14:42:57  dkr
 * WLSEGVAR_IDX_PRINT modified
 * WLSEGX_IDX_MIN, WLSEGX_IDX_MAX added
 *
 * Revision 3.30  2001/04/02 11:44:53  dkr
 * functions NodeOrInt...(), NameOrVal...() moved to wl_bounds.[ch]
 *
 * Revision 3.29  2001/03/29 09:18:49  nmw
 * tabs2spaces done
 *
 * Revision 3.28  2001/03/29 01:33:43  dkr
 * functions for NodeOrInt, NameOrVal recoded
 *
 * Revision 3.26  2001/03/28 14:53:49  dkr
 * CHECK_NULL moved from tree_compound.h to internal_lib.h
 *
 * Revision 3.25  2001/03/27 15:40:17  nmw
 * Array2Vec as wrapper for different Array2<XYZ>Vec added
 *
 * Revision 3.22  2001/03/05 16:43:13  dkr
 * new macros NWITH???_IS_FOLD added
 *
 * Revision 3.21  2001/02/16 08:42:14  nmw
 * macro AVIS_SSASTACK_INUSE added
 *
 * Revision 3.20  2001/02/15 16:59:43  nmw
 * access macro for SSAstack added
 *
 * Revision 3.19  2001/02/13 15:17:39  nmw
 * compound macro VARDEC_OR_ARG_AVIS added
 *
 * Revision 3.17  2001/02/09 10:47:07  dkr
 * macros PRF_EXPRS?, AP_EXPRS? added
 *
 * Revision 3.16  2001/02/07 21:14:04  dkr
 * some WL... macros corrected: all ?-expressions in brackets now
 *
 * Revision 3.15  2001/02/07 20:16:56  dkr
 * N_WL?block, N_WLstride?: NOOP not an attribute but a macro now
 *
 * Revision 3.14  2001/02/06 01:48:06  dkr
 * WLBLOCKSTR_GET_ADDR added
 *
 * Revision 3.12  2001/01/29 16:08:19  dkr
 * WL_GET_ADDRESS replaced by WLNODE_GET_ADDR, WLSTRIDEX_GET_ADDR,
 * WLGRIDX_GET_ADDR
 *
 * Revision 3.11  2001/01/24 23:34:24  dkr
 * NameOrVal_MakeIndex, NodeOrInt_MakeIndex added
 *
 * Revision 3.10  2001/01/19 11:54:43  dkr
 * signature of NameOrVal_MakeNode modified
 *
 * Revision 3.9  2001/01/17 14:17:27  dkr
 * functions NameOrVal_... and NodeOrInt_... added
 *
 * Revision 3.8  2001/01/10 14:27:32  dkr
 * function MakeWLsegX added
 *
 * Revision 3.7  2001/01/10 11:30:18  dkr
 * some WLGRIDX_... macros added
 *
 * Revision 3.6  2001/01/09 17:26:49  dkr
 * N_WLstriVar renamed into N_WLstrideVar
 *
 * Revision 3.5  2001/01/09 16:55:13  dkr
 * some redundant macros for WL-nodes removed
 *
 * Revision 3.3  2000/12/12 15:34:07  dkr
 * some macros renamed
 *
 * Revision 3.2  2000/11/29 13:13:29  dkr
 * macros AP_ARG? added
 *
 * [...]
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
 ***  SHAPE :
 ***/

#define SHAPES_SELEMS(s) (SHPSEG_ELEMS (SHAPES_SHPSEG (s)))
#define SHAPES_SNEXT(s) (SHPSEG_NEXT (SHAPES_SHPSEG (s)))

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***/

extern int GetShpsegLength (int dims, shpseg *shape);
extern shpseg *DiffShpseg (int dim, shpseg *shape1, shpseg *shape2);
extern bool EqualShpseg (int dim, shpseg *shape2, shpseg *shape1);
extern shpseg *MergeShpseg (shpseg *first, int dim1, shpseg *second, int dim2);

extern shpseg *Array2Shpseg (node *array, int *ret_dim);
extern node *Shpseg2Array (shpseg *shape, int dim);

/*--------------------------------------------------------------------------*/

/***
 ***  TYPES :
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
     && (!strcmp (STR_OR_EMPTY (TYPES_MOD (a)), STR_OR_EMPTY (TYPES_MOD (b)))))

extern types *AppendTypes (types *chain, types *item);
extern int CountTypes (types *type);
extern int HasDotTypes (types *type);
extern types *GetTypes_Line (types *type, int line);
extern types *GetTypes (types *type);
extern int GetShapeDim (types *type);
extern int GetDim (types *type);
extern simpletype GetBasetype (types *type);
extern int GetBasetypeSize (types *type);
extern int GetTypesLength (types *type);
extern int CompareTypesImplementation (types *t1, types *t2);
extern shpseg *Type2Shpseg (types *type, int *ret_dim);
extern node *Type2Exprs (types *type);
extern node *CreateZeroFromType (types *type, bool unroll, node *fundef);

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
 ***  IDS :
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
extern int CountIds (ids *ids_arg);

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
 ***  NUMS :
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
 *   nodelist *NodeListAppend( nodelist *nl, node *newnode, void *attrib)
 *   nodelist *NodeListDelete( nodelist *nl, node *node, bool free_attrib)
 *   nodelist *NodeListFree( nodelist *nl, bool free_attrib)
 *   nodelist *NodeListFind( nodelist *nl, node *node)
 *
 * description:
 *   the following functions realize basic functions on pure node lists.
 *
 *   Append: appends a node to the given list, returning a new list.
 *           Since the node list has no special order, the new node is
 *           not appended but put in front of the given list to speed
 *           up execution.
 *           Create a list: newlist = Append(NULL, newnode, attrib);
 *   Delete: deletes all elements of the given node. If free_attrib is FALSE,
 *           the attribut is not set free, else a FREE(attrib) is executed.
 *   Free  : frees whole list. If free_attrib is FALSE, the attributes are
 *           not set free, else a FREE(attrib) is executed.
 *   Find  : returns the nodelist node of the first found item
 *           with fitting node. If not found, returns NULL.
 *
 ******************************************************************************/

extern nodelist *NodeListAppend (nodelist *nl, node *newnode, void *attrib);
extern nodelist *NodeListDelete (nodelist *nl, node *node, bool free_attrib);
extern nodelist *NodeListFree (nodelist *nl, bool free_attrib);
extern nodelist *NodeListFind (nodelist *nl, node *node);

/*--------------------------------------------------------------------------*/

/***
 ***  ARGTAB :
 ***/

extern int GetArgtabIndexOut (types *type, argtab_t *argtab);

extern int GetArgtabIndexIn (types *type, argtab_t *argtab);

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

/*****************************************************************************
 *
 * function:
 *   bool IsImported( node* symbol)
 *   bool IsExternal( node* symbol)
 *   bool IsFromModule( node* symbol)
 *   bool IsFromClass( node* symbol)
 *
 * description:
 *
 *   These test functions are applicable to any kind of symbol,
 *   more precisely to N_typedef, N_objdef, and N_fundef nodes.
 *
 *****************************************************************************/

extern bool IsImported (node *symbol);
extern bool IsExternal (node *symbol);
extern bool IsFromModule (node *symbol);
extern bool IsFromClass (node *symbol);

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
     && (!strcmp (STR_OR_EMPTY (mod), STR_OR_EMPTY (TYPEDEF_MOD (tdef)))))

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

/*****************************************************************************
 *
 * function:
 *   node *AppendTypedef( node *tdef_chain, node *tdef)
 *
 * description:
 *
 *   This function concatenates two chains of N_typedef nodes.
 *
 *****************************************************************************/

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

/*****************************************************************************
 *
 * function:
 *   node *AppendObjdef( node *objdef_chain, node *objdef)
 *
 * description:
 *
 *   This function concatenates two chains of N_objdef nodes.
 *
 *****************************************************************************/

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
#define FUNDEF_RC_ICMS(n) (BLOCK_RC_ICMS (FUNDEF_BODY (n)))

#define FUNDEF_DEFMASK(n) (FUNDEF_MASK (n, 0))
#define FUNDEF_USEMASK(n) (FUNDEF_MASK (n, 1))

#define FUNDEF_LINKNAME(n) (PRAGMA_LINKNAME (FUNDEF_PRAGMA (n)))
#define FUNDEF_LINKSIGN(n) (PRAGMA_LINKSIGN (FUNDEF_PRAGMA (n)))
#define FUNDEF_REFCOUNTING(n) (PRAGMA_REFCOUNTING (FUNDEF_PRAGMA (n)))

#define FUNDEF_IS_LACFUN(n)                                                              \
    ((FUNDEF_STATUS (n) == ST_condfun) || (FUNDEF_STATUS (n) == ST_dofun)                \
     || (FUNDEF_STATUS (n) == ST_whilefun))
#define FUNDEF_IS_CONDFUN(n) (FUNDEF_STATUS (n) == ST_condfun)
#define FUNDEF_IS_LOOPFUN(n)                                                             \
    ((FUNDEF_STATUS (n) == ST_dofun) || (FUNDEF_STATUS (n) == ST_whilefun))

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
     && (0 == strcmp (STR_OR_EMPTY (FUNDEF_MOD (a)), STR_OR_EMPTY (FUNDEF_MOD (b)))))

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

/*****************************************************************************
 *
 * function:
 *   node *AppendFundef( node *fundef_chain, node *fundef)
 *
 * description:
 *
 *   This function concatenates two chains of N_fundef nodes.
 *
 *****************************************************************************/

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
 * Function:
 *   node *AddVardecs( node *fundef, node *vardecs)
 *
 * Description:
 *   Inserts new declarations into the AST, updates the DFMbase and returns
 *   the modified N_fundef node.
 *
 ******************************************************************************/

extern node *AddVardecs (node *fundef, node *vardecs);

/******************************************************************************
 *
 * function:
 *   node *AppendVardec( node *vardec_chain, node *vardec)
 *
 * description:
 *   Appends 'vardec' to 'vardec_chain' and returns the new chain.
 *
 ******************************************************************************/

extern node *AppendVardec (node *vardec_chain, node *vardec);

/******************************************************************************
 *
 * function:
 *   node *MakeVardecFromArg( node *arg)
 *
 * description:
 *   copies all attributes from an arg node to a new allocated vardec node.
 *
 * remark:
 *   This function is used by ssa-transformation to rename a redefinition
 *   of an fundef argument. The Next pointer is set to NULL.
 *
 *
 ******************************************************************************/

extern node *MakeVardecFromArg (node *arg_node);

/******************************************************************************
 *
 * function:
 *   node *MakeArgFromVardec( node *vardec_node)
 *
 * description:
 *   copies all attributes from an vardec node to a new allocated arg node.
 *
 ******************************************************************************/

extern node *MakeArgFromVardec (node *vardec_node);

/******************************************************************************
 *
 * function:
 *   node *AdjustAvisData( node *new_vardec, node *fundef)
 *
 * description:
 *   when a vardec is duplicated via DupTree all dependend infomation in the
 *   corresponding avis node is duplicated, too. when this vardec is used in
 *   the same fundef as the original one everything is good, but if the
 *   duplicated vardec should be used in a different fundef the fundef related
 *   attributes have to be adjusted by this function:
 *     AVIS_SSACOUNT = (new fresh ssacnt node)
 *     AVIS_SSALPINV = FALSE
 *     AVIS_SSAPHITARGET = FALSE
 *     AVIS_SSADEFINED = FALSE
 *     AVIS_SSATHEN = FALSE
 *     AVIS_SSAELSE = FALSE
 *     AVIS_NEEDCOUNT = 0
 *     AVIS_SUBST = NULL
 *     AVIS_SUBSTUSSA = NULL
 *
 * remark:
 *   when creating a new ssacounter node this node is stored in the toplevel
 *   block of the given fundef (sideeffekt!!!)
 *
 ******************************************************************************/

extern node *AdjustAvisData (node *new_vardec, node *fundef);

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

extern int CountArgs (node *args);

extern int HasDotArgs (node *args);

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
#define VARDEC_OR_ARG_TYPE(n) ((NODE_TYPE (n) == N_arg) ? ARG_TYPE (n) : VARDEC_TYPE (n))
#define VARDEC_OR_ARG_STATUS(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_STATUS (n) : VARDEC_STATUS (n))
#define VARDEC_OR_ARG_ATTRIB(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_ATTRIB (n) : VARDEC_ATTRIB (n))
#define VARDEC_OR_ARG_AVIS(n) ((NODE_TYPE (n) == N_arg) ? ARG_AVIS (n) : VARDEC_AVIS (n))
#define VARDEC_OR_ARG_VARNO(n)                                                           \
    ((NODE_TYPE (n) == N_arg) ? ARG_VARNO (n) : VARDEC_VARNO (n))
#define VARDEC_OR_ARG_REFCNT(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_REFCNT (n) : VARDEC_REFCNT (n))
#define VARDEC_OR_ARG_NAIVE_REFCNT(n)                                                    \
    ((NODE_TYPE (n) == N_arg) ? ARG_NAIVE_REFCNT (n) : VARDEC_NAIVE_REFCNT (n))
#define VARDEC_OR_ARG_PADDED(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_PADDED (n) : VARDEC_PADDED (n))
#define VARDEC_OR_ARG_ACTCHN(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_ACTCHN (n) : VARDEC_ACTCHN (n))
#define VARDEC_OR_ARG_COLCHN(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_COLCHN (n) : VARDEC_COLCHN (n))
#define VARDEC_OR_ARG_OBJDEF(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_OBJDEF (n) : VARDEC_OBJDEF (n))
#define VARDEC_OR_ARG_NEXT(n) ((NODE_TYPE (n) == N_arg) ? ARG_NEXT (n) : VARDEC_NEXT (n))

#define VARDEC_OR_ARG_BASETYPE(n)                                                        \
    ((NODE_TYPE (n) == N_arg) ? ARG_BASETYPE (n) : VARDEC_BASETYPE (n))
#define VARDEC_OR_ARG_DIM(n) ((NODE_TYPE (n) == N_arg) ? ARG_DIM (n) : VARDEC_DIM (n))
#define VARDEC_OR_ARG_SHAPE(n, x)                                                        \
    ((NODE_TYPE (n) == N_arg) ? ARG_SHAPE (n, x) : VARDEC_SHAPE (n, x))
#define VARDEC_OR_ARG_SHPSEG(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_SHPSEG (n) : VARDEC_SHPSEG (n))
#define VARDEC_OR_ARG_TNAME(n)                                                           \
    ((NODE_TYPE (n) == N_arg) ? ARG_TNAME (n) : VARDEC_TNAME (n))
#define VARDEC_OR_ARG_TMOD(n) ((NODE_TYPE (n) == N_arg) ? ARG_TMOD (n) : VARDEC_TMOD (n))
#define VARDEC_OR_ARG_TDEF(n) ((NODE_TYPE (n) == N_arg) ? ARG_TDEF (n) : VARDEC_TDEF (n))

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

#define L_VARDEC_OR_ARG_AVIS(n, rhs)                                                     \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_AVIS (n) = (rhs);                                                            \
    } else {                                                                             \
        VARDEC_AVIS (n) = (rhs);                                                         \
    }

#define L_VARDEC_OR_ARG_TYPE(n, rhs)                                                     \
    if (NODE_TYPE (n) == N_arg) {                                                        \
        ARG_TYPE (n) = (rhs);                                                            \
    } else {                                                                             \
        VARDEC_TYPE (n) = (rhs);                                                         \
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

#define ASSIGN_DEFMASK(n) ASSIGN_MASK (n, 0)
#define ASSIGN_USEMASK(n) ASSIGN_MASK (n, 1)
#define ASSIGN_MRDMASK(n) ASSIGN_MASK (n, 2)
#define ASSIGN_INSTRTYPE(n) NODE_TYPE (ASSIGN_INSTR (n))
#define ASSIGN_NAME(n) IDS_NAME (ASSIGN_LHS (n))
#define ASSIGN_LHS(n) LET_IDS (ASSIGN_INSTR (n))
#define ASSIGN_RHS(n) LET_EXPR (ASSIGN_INSTR (n))

/******************************************************************************
 *
 * function:
 *   node *MakeAssignLet(char *var_name, node *vardec_node, node *let_expr);
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
 * Function:
 *   node *MakeAssignInstr( node *instr, node *next)
 *
 * Description:
 *
 *
 ******************************************************************************/

extern node *MakeAssignInstr (node *instr, node *next);

/******************************************************************************
 *
 * Function:
 *   node *MakeAssign?( node *part?, ...)
 *
 * Description:
 *
 *
 ******************************************************************************/

extern node *MakeAssigns1 (node *part1);
extern node *MakeAssigns2 (node *part1, node *part2);
extern node *MakeAssigns3 (node *part1, node *part2, node *part3);
extern node *MakeAssigns4 (node *part1, node *part2, node *part3, node *part4);
extern node *MakeAssigns5 (node *part1, node *part2, node *part3, node *part4,
                           node *part5);
extern node *MakeAssigns6 (node *part1, node *part2, node *part3, node *part4,
                           node *part5, node *part6);
extern node *MakeAssigns7 (node *part1, node *part2, node *part3, node *part4,
                           node *part5, node *part6, node *part7);
extern node *MakeAssigns8 (node *part1, node *part2, node *part3, node *part4,
                           node *part5, node *part6, node *part7, node *part8);
extern node *MakeAssigns9 (node *part1, node *part2, node *part3, node *part4,
                           node *part5, node *part6, node *part7, node *part8,
                           node *part9);

/******************************************************************************
 *
 * function:
 *   node *MakeAssignIcm0( char *name, node *next)
 *   node *MakeAssignIcm?( char *name, node *arg?, ..., node *next)
 *
 * description:
 *   These functions generate an N_assign node with a complete ICM
 *   representations including arguments as body.
 *   Each function argument may be an arbitrary list of single ICM arguments.
 *   These are concatenated correctly.
 *   The ASSIGN_NEXT will be NULL!
 *
 ******************************************************************************/

extern node *MakeAssignIcm0 (char *name, node *next);
extern node *MakeAssignIcm1 (char *name, node *arg1, node *next);
extern node *MakeAssignIcm2 (char *name, node *arg1, node *arg2, node *next);
extern node *MakeAssignIcm3 (char *name, node *arg1, node *arg2, node *arg3, node *next);
extern node *MakeAssignIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                             node *next);
extern node *MakeAssignIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                             node *arg5, node *next);
extern node *MakeAssignIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                             node *arg5, node *arg6, node *next);
extern node *MakeAssignIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                             node *arg5, node *arg6, node *arg7, node *next);

/******************************************************************************
 *
 * function:
 *   node *GetCompoundNode(node *arg_node);
 *
 * description:
 *   returns the compund_node that is attached to the assign-node
 *
 ******************************************************************************/

extern node *GetCompoundNode (node *arg_node);

/*****************************************************************************
 *
 * function:
 *   AppendAssign( node *assign_chain, node *assign)
 *
 * description:
 *   This function concatenates two chains of N_assign nodes.
 *   However, the first one may simply be a single N_empty node.
 *
 *****************************************************************************/

extern node *AppendAssign (node *assign_chain, node *assign);

/******************************************************************************
 *
 * function:
 *   node *AppendAssignIcm( node *assign, char *name, node *args)
 *
 * description:
 *   Appends a new ICM with name and args given as an assign to the given
 *   chain of assignments.
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
 *   node *AppendExprs( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_exprs chains of nodes.
 *
 ******************************************************************************/

extern node *AppendExprs (node *exprs1, node *exprs2);

/******************************************************************************
 *
 * function:
 *   node *CombineExprs( node *first, node *second)
 *
 * description:
 *   'first' and 'second' are N_exprs chains or expression nodes (N_id, N_num,
 *   ...) that will be conactenated to a single N_exprs chain.
 *
 ******************************************************************************/

extern node *CombineExprs (node *first, node *second);

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
 *   int CountExprs( node *exprs)
 *
 * description:
 *   Computes the length of the given N_exprs chain.
 *
 ******************************************************************************/

extern int CountExprs (node *exprs);

#define EXPRS_EXPRS1(n) (n)
#define EXPRS_EXPRS2(n) EXPRS_NEXT (n)
#define EXPRS_EXPRS3(n) EXPRS_NEXT (EXPRS_NEXT (n))
#define EXPRS_EXPRS4(n) EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (n)))
#define EXPRS_EXPRS5(n) EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (n))))
#define EXPRS_EXPRS6(n) EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (n)))))

#define EXPRS_EXPR1(n) EXPRS_EXPR (n)
#define EXPRS_EXPR2(n) EXPRS_EXPR (EXPRS_EXPRS2 (n))
#define EXPRS_EXPR3(n) EXPRS_EXPR (EXPRS_EXPRS3 (n))
#define EXPRS_EXPR4(n) EXPRS_EXPR (EXPRS_EXPRS4 (n))
#define EXPRS_EXPR5(n) EXPRS_EXPR (EXPRS_EXPRS5 (n))
#define EXPRS_EXPR6(n) EXPRS_EXPR (EXPRS_EXPRS6 (n))

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
#define DO_INSTR(n) (BLOCK_INSTR (DO_BODY (n)))
#define DO_TERMMASK(n) (DO_MASK (n, 1))

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
#define WHILE_INSTR(n) (BLOCK_INSTR (WHILE_BODY (n)))
#define WHILE_TERMMASK(n) (WHILE_MASK (n, 1))

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

#define DO_OR_WHILE_INSTR(n) ((NODE_TYPE (n) == N_do) ? DO_INSTR (n) : WHILE_INSTR (n))

#define DO_OR_WHILE_MASK(n, x)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_MASK (n, x) : WHILE_MASK (n, x))
#define DO_OR_WHILE_DEFMASK(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_DEFMASK (n) : WHILE_DEFMASK (n))
#define DO_OR_WHILE_USEMASK(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_USEMASK (n) : WHILE_USEMASK (n))
#define DO_OR_WHILE_TERMMASK(n)                                                          \
    ((NODE_TYPE (n) == N_do) ? DO_TERMMASK (n) : WHILE_TERMMASK (n))

#define DO_OR_WHILE_IN_MASK(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_IN_MASK (n) : WHILE_IN_MASK (n))
#define DO_OR_WHILE_OUT_MASK(n)                                                          \
    ((NODE_TYPE (n) == N_do) ? DO_OUT_MASK (n) : WHILE_OUT_MASK (n))
#define DO_OR_WHILE_LOCAL_MASK(n)                                                        \
    ((NODE_TYPE (n) == N_do) ? DO_LOCAL_MASK (n) : WHILE_LOCAL_MASK (n))

#define DO_OR_WHILE_USEVARS(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_USEVARS (n) : WHILE_USEVARS (n))
#define DO_OR_WHILE_DEFVARS(n)                                                           \
    ((NODE_TYPE (n) == N_do) ? DO_DEFVARS (n) : WHILE_DEFVARS (n))
#define DO_OR_WHILE_NAIVE_USEVARS(n)                                                     \
    ((NODE_TYPE (n) == N_do) ? DO_NAIVE_USEVARS (n) : WHILE_NAIVE_USEVARS (n))
#define DO_OR_WHILE_NAIVE_DEFVARS(n)                                                     \
    ((NODE_TYPE (n) == N_do) ? DO_NAIVE_DEFVARS (n) : WHILE_NAIVE_DEFVARS (n))

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

#define L_DO_OR_WHILE_INSTR(n, rhs)                                                      \
    if (NODE_TYPE (n) == N_do) {                                                         \
        DO_INSTR (n) = (rhs);                                                            \
    } else {                                                                             \
        WHILE_INSTR (n) = (rhs);                                                         \
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

extern node *Ids2Array (ids *ids_arg);

/******************************************************************************
 *
 * Function:
 *   node *IntVec2Array( int length, int *intvec);
 *
 * Description:
 *   Returns an N_exprs node containing the elements of intvec.
 *
 ******************************************************************************/

extern node *IntVec2Array (int length, int *intvec);

/******************************************************************************
 *
 * Function:
 *   int    *Array2IntVec( node *aelems, int *length);
 *   int    *Array2BoolVec( node *aelems, int *length);
 *   char   *Array2CharVec( node *aelems, int *length);
 *   float  *Array2FloatVec( node *aelems, int *length);
 *   double *Array2DblVec( node *aelems, int *length);
 *
 *   void   *Array2Vec( simpletype t, node *aelems, int *length);
 *
 * Description:
 *   Returns an iteger (char | float | double) vector and stores the number of
 *   constant integer (char | float | double) elements in *length if first
 *   argument is an N_exprs and all its elements are N_num otherwise the
 *   result is not defined.
 *   If the length of the vector is not of interest, length may be NULL.
 *   The Array2Vec function is a dispatching wrapper for all above.
 *
 ******************************************************************************/

extern int *Array2IntVec (node *aelems, int *length);
extern int *Array2BoolVec (node *aelems, int *length);
extern char *Array2CharVec (node *aelems, int *length);
extern float *Array2FloatVec (node *aelems, int *length);
extern double *Array2DblVec (node *aelems, int *length);
extern void *Array2Vec (simpletype t, node *aelems, int *length);

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

#define ID_SSAASSIGN(n) (AVIS_SSAASSIGN (ID_AVIS (n)))

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

#define PRF_EXPRS1(n) PRF_ARGS (n)
#define PRF_EXPRS2(n) EXPRS_EXPRS2 (PRF_ARGS (n))
#define PRF_EXPRS3(n) EXPRS_EXPRS3 (PRF_ARGS (n))

#define PRF_ARG1(n) EXPRS_EXPR (PRF_EXPRS1 (n))
#define PRF_ARG2(n) EXPRS_EXPR (PRF_EXPRS2 (n))
#define PRF_ARG3(n) EXPRS_EXPR (PRF_EXPRS3 (n))

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

#define AP_EXPRS1(n) AP_ARGS (n)
#define AP_EXPRS2(n) EXPRS_EXPRS2 (AP_ARGS (n))
#define AP_EXPRS3(n) EXPRS_EXPRS3 (AP_ARGS (n))

#define AP_ARG1(n) EXPRS_EXPR (AP_EXPRS1 (n))
#define AP_ARG2(n) EXPRS_EXPR (AP_EXPRS2 (n))
#define AP_ARG3(n) EXPRS_EXPR (AP_EXPRS3 (n))

/*
 *  function declarations
 */

extern node *MakeAp1 (char *name, char *mod, node *arg1);

extern node *MakeAp2 (char *name, char *mod, node *arg1, node *arg2);

extern node *MakeAp3 (char *name, char *mod, node *arg1, node *arg2, node *arg3);

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :  *and*  N_ap :
 ***
 ***  watch simple macros for N_ap and N_prf
 ***  (search for "N_ap :" or "N_prf :").
 ***/

#define AP_OR_PRF_ARGS(n) ((NODE_TYPE (n) == N_ap) ? AP_ARGS (n) : PRF_ARGS (n))

/******************************************************************************
 *
 * Function:
 *   node *LiftArg( node *arg, node *fundef, types *new_type, bool do_rc,
 *                  node **new_assigns)
 *
 * Description:
 *   Lifts the given argument of a function application:
 *    - Generates a new and fresh varname.
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of ID_TYPE(arg).
 *    - Builds a new assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *    - Returns the new argument.
 *
 ******************************************************************************/

node *LiftArg (node *arg, node *fundef, types *new_type, bool do_rc, node **new_assigns);

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
#define ICM_EXPRS2(n) EXPRS_EXPRS2 (ICM_ARGS (n))
#define ICM_EXPRS3(n) EXPRS_EXPRS3 (ICM_ARGS (n))
#define ICM_EXPRS4(n) EXPRS_EXPRS4 (ICM_ARGS (n))
#define ICM_EXPRS5(n) EXPRS_EXPRS5 (ICM_ARGS (n))
#define ICM_EXPRS6(n) EXPRS_EXPRS6 (ICM_ARGS (n))

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
 *   node *MakeIcm?(char *name, node *arg1, ...)
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

#define NWITH_IS_FOLD(n)                                                                 \
    ((NWITH_TYPE (n) == WO_foldprf) || (NWITH_TYPE (n) == WO_foldfun))

extern node *CreateScalarWith (int dim, shpseg *shape, simpletype btype, node *expr,
                               node *fundef);

extern node *CreateZero (int dim, shpseg *shape, simpletype btype, bool unroll,
                         node *fundef);

extern node *CreateSel (ids *sel_vec, ids *sel_ids, node *sel_array, bool no_wl,
                        node *fundef);

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

#define NPART_CEXPR(n) (NCODE_CEXPR (NPART_CODE (n)))
#define NPART_CBLOCK(n) (NCODE_CBLOCK (NPART_CODE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ncode :
 ***/

#define NCODE_CBLOCK_INSTR(n) (BLOCK_INSTR (NCODE_CBLOCK (n)))

#define NCODE_DEFMASK(n) (NCODE_MASK (n, 0))
#define NCODE_USEMASK(n) (NCODE_MASK (n, 1))

#define NCODE_WLAA_ARRAYSHP(n) VARDEC_SHPSEG (NCODE_WLAA_WLARRAY (n))
#define NCODE_WLAA_INDEXDIM(n) VARDEC_SHAPE (NCODE_WLAA_INDEXVAR (n), 0)
#define NCODE_WLAA_ARRAYDIM(n) VARDEC_DIM (NCODE_WLAA_WLARRAY (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwithop :
 ***/

#define NWITHOP_OPARG(n)                                                                 \
    ((WO_modarray == NWITHOP_TYPE (n))                                                   \
       ? NWITHOP_ARRAY (n)                                                               \
       : (WO_genarray == NWITHOP_TYPE (n)) ? NWITHOP_SHAPE (n) : NWITHOP_NEUTRAL (n))

#define NWITHOP_DEFMASK(n) (NWITHOP_MASK (n, 0))
#define NWITHOP_USEMASK(n) (NWITHOP_MASK (n, 1))

#define NWITHOP_IS_FOLD(n)                                                               \
    ((NWITHOP_TYPE (n) == WO_foldprf) || (NWITHOP_TYPE (n) == WO_foldfun))

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

#define NWITH2_IS_FOLD(n)                                                                \
    ((NWITH2_TYPE (n) == WO_foldprf) || (NWITH2_TYPE (n) == WO_foldfun))

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

#define NWITH_OR_NWITH2_DEC_RC_IDS(n)                                                    \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_DEC_RC_IDS (n) : NWITH2_DEC_RC_IDS (n))

#define NWITH_OR_NWITH2_IN_MASK(n)                                                       \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_IN_MASK (n) : NWITH2_IN_MASK (n))
#define NWITH_OR_NWITH2_OUT_MASK(n)                                                      \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_OUT_MASK (n) : NWITH2_OUT_MASK (n))
#define NWITH_OR_NWITH2_LOCAL_MASK(n)                                                    \
    ((NODE_TYPE (n) == N_Nwith) ? NWITH_LOCAL_MASK (n) : NWITH2_LOCAL_MASK (n))

#define L_NWITH_OR_NWITH2_DEC_RC_IDS(n, rhs)                                             \
    if (NODE_TYPE (n) == N_Nwith) {                                                      \
        NWITH_DEC_RC_IDS (n) = (rhs);                                                    \
    } else {                                                                             \
        NWITH2_DEC_RC_IDS (n) = (rhs);                                                   \
    }

#define L_NWITH_OR_NWITH2_IN_MASK(n, rhs)                                                \
    if (NODE_TYPE (n) == N_Nwith) {                                                      \
        NWITH_IN_MASK (n) = (rhs);                                                       \
    } else {                                                                             \
        NWITH2_IN_MASK (n) = (rhs);                                                      \
    }

#define L_NWITH_OR_NWITH2_OUT_MASK(n, rhs)                                               \
    if (NODE_TYPE (n) == N_Nwith) {                                                      \
        NWITH_OUT_MASK (n) = (rhs);                                                      \
    } else {                                                                             \
        NWITH2_OUT_MASK (n) = (rhs);                                                     \
    }

#define L_NWITH_OR_NWITH2_LOCAL_MASK(n, rhs)                                             \
    if (NODE_TYPE (n) == N_Nwith) {                                                      \
        NWITH_LOCAL_MASK (n) = (rhs);                                                    \
    } else {                                                                             \
        NWITH2_LOCAL_MASK (n) = (rhs);                                                   \
    }

#define NWITH_OR_NWITH2_IS_FOLD(n)                                                       \
    ((NWITH_OR_NWITH2_TYPE (n) == WO_foldprf) || (NWITH_OR_NWITH2_TYPE (n) == WO_foldfun))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLseg :
 ***/

#define WLSEG_IDX_PRINT(handle, n, field)                                                \
    PRINT_VECT (handle, ((int *)(WLSEG_##field (n))), WLSEG_DIMS (n), "%i");

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLsegVar :
 ***/

#define WLSEGVAR_IDX_PRINT(handle, n, field)                                             \
    {                                                                                    \
        int d;                                                                           \
        node **vect = WLSEGVAR_##field (n);                                              \
        if (vect != NULL) {                                                              \
            fprintf (handle, "[ ");                                                      \
            for (d = 0; d < WLSEGVAR_DIMS (n); d++) {                                    \
                NodeOrInt_Print (handle, N_WLsegVar, &(vect[d]), d);                     \
                fprintf (handle, " ");                                                   \
            }                                                                            \
            fprintf (handle, "]");                                                       \
        } else {                                                                         \
            fprintf (handle, "NULL");                                                    \
        }                                                                                \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLseg :  *and*  N_WLsegVar :
 ***/

#define WLSEGX_IDX_MIN(n)                                                                \
    ((NODE_TYPE (n) == N_WLseg) ? WLSEG_IDX_MIN (n) : WLSEGVAR_IDX_MIN (n))

#define WLSEGX_IDX_MAX(n)                                                                \
    ((NODE_TYPE (n) == N_WLseg) ? WLSEG_IDX_MAX (n) : WLSEGVAR_IDX_MAX (n))

#define WLSEGX_IDX_GET_ADDR(n, field, dim)                                               \
    ((NODE_TYPE (n) == N_WLseg) ? (void *)&(((int *)(WLSEG_##field (n)))[dim])           \
                                : ((NODE_TYPE (n) == N_WLsegVar)                         \
                                     ? (void *)&(((node **)(WLSEGVAR_##field (n)))[dim]) \
                                     : NULL))

#define WLSEGX_IDX_PRINT(handle, n, field)                                               \
    if (NODE_TYPE (n) == N_WLseg) {                                                      \
        WLSEG_IDX_PRINT (handle, n, field);                                              \
    } else {                                                                             \
        WLSEGVAR_IDX_PRINT (handle, n, field);                                           \
    }

extern node *MakeWLsegX (int dims, node *contents, node *next);

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLblock :
 ***/

#define WLBLOCK_NOOP(n) ((WLBLOCK_NEXTDIM (n) == NULL) && (WLBLOCK_CONTENTS (n) == NULL))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLublock :
 ***/

#define WLUBLOCK_NOOP(n)                                                                 \
    ((WLUBLOCK_NEXTDIM (n) == NULL) && (WLUBLOCK_CONTENTS (n) == NULL))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLblock :  *and*  N_WLublock :
 ***/

#define WLXBLOCK_NOOP(n)                                                                 \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_NOOP (n) : WLUBLOCK_NOOP (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLstride :
 ***/

#define WLSTRIDE_NOOP(n) (WLSTRIDE_CONTENTS (n) == NULL)

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLstrideVar :
 ***/

#define WLSTRIDEVAR_NOOP(n) (WLSTRIDEVAR_CONTENTS (n) == NULL)

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLstride :  *and*  N_WLstrideVar :
 ***/

#define WLSTRIDEX_GET_ADDR(n, field)                                                     \
    ((NODE_TYPE (n) == N_WLstride)                                                       \
       ? (void *)&(WLSTRIDE_##field (n))                                                 \
       : ((NODE_TYPE (n) == N_WLstrideVar) ? (void *)&(WLSTRIDEVAR_##field (n)) : NULL))

#define WLSTRIDEX_NOOP(n)                                                                \
    ((NODE_TYPE (n) == N_WLstride) ? WLSTRIDE_NOOP (n) : WLSTRIDEVAR_NOOP (n))

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
 ***  N_WLgrid :  *and*  N_WLgridVar :
 ***/

#define WLGRIDX_CBLOCK(n) (NCODE_CBLOCK (WLGRIDX_CODE (n)))
#define WLGRIDX_CEXPR(n) (NCODE_CEXPR (WLGRIDX_CODE (n)))

#define WLGRIDX_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRIDX_CBLOCK (n)))

#define WLGRIDX_GET_ADDR(n, field)                                                       \
    ((NODE_TYPE (n) == N_WLgrid)                                                         \
       ? (void *)&(WLGRID_##field (n))                                                   \
       : ((NODE_TYPE (n) == N_WLgridVar) ? (void *)&(WLGRIDVAR_##field (n)) : NULL))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLblock :   *and*  N_WLublock :     *and*
 ***  N_WLstride :  *and*  N_WLstrideVar :
 ***/

#define WLBLOCKSTR_GET_ADDR(n, field)                                                    \
    ((NODE_TYPE (n) == N_WLstride)                                                       \
       ? (void *)&(WLSTRIDE_##field (n))                                                 \
       : ((NODE_TYPE (n) == N_WLstrideVar)                                               \
            ? (void *)&(WLSTRIDEVAR_##field (n))                                         \
            : ((NODE_TYPE (n) == N_WLblock)                                              \
                 ? (void *)&(WLBLOCK_##field (n))                                        \
                 : ((NODE_TYPE (n) == N_WLublock) ? (void *)&(WLUBLOCK_##field (n))      \
                                                  : NULL))))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLblock :   *and*  N_WLublock :     *and*
 ***  N_WLstride :  *and*  N_WLstrideVar :  *and*
 ***  N_WLgrid :    *and*  N_WLgridVar :
 ***/

#define WLNODE_GET_ADDR(n, field)                                                        \
    ((NODE_TYPE (n) == N_WLstride)                                                       \
       ? (void *)&(WLSTRIDE_##field (n))                                                 \
       : ((NODE_TYPE (n) == N_WLstrideVar)                                               \
            ? (void *)&(WLSTRIDEVAR_##field (n))                                         \
            : ((NODE_TYPE (n) == N_WLgrid)                                               \
                 ? (void *)&(WLGRID_##field (n))                                         \
                 : ((NODE_TYPE (n) == N_WLgridVar)                                       \
                      ? (void *)&(WLGRIDVAR_##field (n))                                 \
                      : ((NODE_TYPE (n) == N_WLblock)                                    \
                           ? (void *)&(WLBLOCK_##field (n))                              \
                           : ((NODE_TYPE (n) == N_WLublock)                              \
                                ? (void *)&(WLUBLOCK_##field (n))                        \
                                : NULL))))))

#define WLNODE_NOOP(n)                                                                   \
    ((NODE_TYPE (n) == N_WLblock)                                                        \
       ? WLBLOCK_NOOP (n)                                                                \
       : ((NODE_TYPE (n) == N_WLublock)                                                  \
            ? WLUBLOCK_NOOP (n)                                                          \
            : ((NODE_TYPE (n) == N_WLstride)                                             \
                 ? WLSTRIDE_NOOP (n)                                                     \
                 : ((NODE_TYPE (n) == N_WLstrideVar)                                     \
                      ? WLSTRIDEVAR_NOOP (n)                                             \
                      : ((NODE_TYPE (n) == N_WLgrid)                                     \
                           ? WLGRID_NOOP (n)                                             \
                           : ((NODE_TYPE (n) == N_WLgridVar) ? WLGRIDVAR_NOOP (n)        \
                                                             : FALSE))))))

/*--------------------------------------------------------------------------*/

/***
 ***  N_avis :
 ***/

#define AVIS_SSASTACK_TOP(n) SSASTACK_AVIS (AVIS_SSASTACK (n))
#define AVIS_SSASTACK_INUSE(n) SSASTACK_INUSE (AVIS_SSASTACK (n))

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
