/*
 *
 * $Log$
 * Revision 3.123  2004/11/23 22:27:57  khf
 * added TCmakeFlatArray
 *
 * Revision 3.122  2004/11/23 22:18:10  ktr
 * take it!!!
 *
 * Revision 3.121  2004/11/23 20:14:47  skt
 * WITHOP_MEM added
 *
 * Revision 3.120  2004/11/23 20:09:51  sbs
 * VARDEC_NTYPE added.
 *
 * Revision 3.119  2004/11/23 20:06:58  sbs
 * ARG_NTYPE added.
 *
 * Revision 3.118  2004/11/23 19:32:48  khf
 * ID_TYPE modified
 *
 * Revision 3.117  2004/11/23 17:25:53  ktr
 * changed some GENs into GENERATORs
 *
 * Revision 3.116  2004/11/23 16:15:02  khf
 * IDS_TYPE, IDS_VARDEC added
 *
 * Revision 3.115  2004/11/23 15:53:20  ktr
 * Corrected WITHOP_NEXT
 *
 * Revision 3.114  2004/11/23 15:24:11  mwe
 * *** empty log message ***
 *
 * Revision 3.113  2004/11/23 14:55:11  ktr
 * Added IDS_NAME, VARDEC_NAME
 *
 * Revision 3.112  2004/11/23 14:49:38  skt
 * brushing during SDC 2k4
 *
 * Revision 3.111  2004/11/23 13:52:52  skt
 * ismop in SACDEvCampDK 2k4
 *
 * Revision 3.110  2004/11/23 13:18:28  skt
 * SacDevCampDK 2k4
 *
 * Revision 3.109  2004/11/23 13:14:05  skt
 * SacDevCampDK 2k4
 *
 * Revision 3.108  2004/11/22 21:10:09  ktr
 * IMSOP 04
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

#ifndef _SAC_TREE_COMPOUND_H_
#define _SAC_TREE_COMPOUND_H_

#include "types.h"
/*#include "shape.h"
#include "tree_basic.h"
#include "Error.h"
#include "free.h"
*/

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*  macros and functions for non-node structures                            */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/***
 ***  RC: moved here temporarily for compatibility reasons from refcount.h
 ***/

/* value, representing an undefined reference counter */
#define RC_UNDEF (-2)
/* value, representing an inactive reference counter */
#define RC_INACTIVE (-1)

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***/

extern int TCgetShpsegLength (int dims, shpseg *shape);
extern shpseg *TCdiffShpseg (int dim, shpseg *shape1, shpseg *shape2);
extern bool TCequalShpseg (int dim, shpseg *shape2, shpseg *shape1);
extern shpseg *TCmergeShpseg (shpseg *first, int dim1, shpseg *second, int dim2);

extern shpseg *TCarray2Shpseg (node *array, int *ret_dim);
extern node *TCshpseg2Array (shpseg *shape, int dim);

/*--------------------------------------------------------------------------*/

/***
 ***  TYPES :
 ***/

/*
 *  compound access macros
 */

#define TYPES_SHAPE(t, x) (SHPSEG_SHAPE (TYPES_SHPSEG (t), x))

extern types *TCappendTypes (types *chain, types *item);
extern int TCcountTypes (types *type);
extern types *TCgetTypes_Line (types *type, int line);
extern types *TCgetTypes (types *type);
extern int TCgetShapeDim (types *type);
extern int TCgetDim (types *type);
extern simpletype TCgetBasetype (types *type);
extern int TCgetBasetypeSize (types *type);
extern int TCgetTypesLength (types *type);
extern int TCcompareTypesImplementation (types *t1, types *t2);
extern shpseg *TCtype2Shpseg (types *type, int *ret_dim);
extern shape *TCtype2Shape (types *type);
extern node *TCtype2Exprs (types *type);
extern node *TCcreateZeroFromType (types *type, bool unroll, node *fundef);

/******************************************************************************
 *
 * function:
 *   bool TCisBoxed( types *type)
 *   bool TCisArray( types *type)
 *   bool TCisUnique( types *type)
 *   bool TCisHidden( types *type)
 *   bool TCisNonUniqueHidden( types *type)
 *
 * description:
 *   These functions may be used to check for particular properties
 *   of a given data type.
 *
 ******************************************************************************/

extern bool TCisBoxed (types *type);
extern bool TCisArray (types *type);
extern bool TCisUnique (types *type);
extern bool TCisHidden (types *type);
extern bool TCisNonUniqueHidden (types *type);

/*--------------------------------------------------------------------------*/

/***
 ***  IDS :
 ***/

#define IDS_NAME(n) AVIS_NAME (IDS_AVIS (n))
#define IDS_VARNO(n) VARDEC_OR_ARG_VARNO (IDS_VARDEC (n))
#define IDS_VARDEC(n) AVIS_DECL (IDS_AVIS (n))
#define IDS_TYPE(n) VARDEC_TYPE (IDS_VARDEC (n))
#define IDS_NTYPE(n) AVIS_TYPE (IDS_AVIS (n))
#define IDS_DIM(n) VARDEC_OR_ARG_DIM (IDS_VARDEC (n))
#define IDS_SHPSEG(n) TYPES_SHPSEG (VARDEC_OR_ARG_TYPE (IDS_VARDEC (n)))
#define IDS_SHAPE(n, x) SHPSEG_SHAPE (IDS_SHPSEG (n), x)
#define IDS_VARDEC_NAME(n) VARDEC_OR_ARG_NAME (IDS_VARDEC (n))
#define IDS_VARDEC_NEXT(n) VARDEC_OR_ARG_NEXT (IDS_VARDEC (n))
#define IDS_PADDED(n) VARDEC_OR_ARG_PADDED (IDS_VARDEC (n))

extern node *TCappendIds (node *chain, node *item);
extern int CountIds (node *ids_arg);

/******************************************************************************
 *
 * function:
 *   node *LookupIds(char *name, node *ids_chain)
 *
 * description:
 *   This function searches for a given identifier name within an ids-chain
 *   of identifiers and returns the ids-structure if found or NULL otherwise.
 *
 ******************************************************************************/

extern node *TClookupIds (const char *name, node *ids_chain);

extern int TCcountNums (node *numsp);

/*--------------------------------------------------------------------------*/

/***
 ***  ConstVec :
 ***/

/* TODO  - eleminate the void* */
extern void *TCcopyConstVec (simpletype vectype, int veclen, void *const_vec);

extern void *TCallocConstVec (simpletype vectype, int veclen);

extern void *TCmodConstVec (simpletype vectype, void *const_vec, int idx,
                            node *const_node);

extern node *TCannotateIdWithConstVec (node *expr, node *id);

/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
 ***/

#if 0 /* TODO - to be deleted after SACDevCampDK 2k4 */
/*
 *
 *  functionname  : TCtidyUpNodelist
 *  arguments     : 1) beginning of nodelist
 *  description   : frees all those entries of a node list which have
 *                  status 'ST_artificial'
 *  remarks       : returns the beginning of the resulting nodelist
 *
 */

extern nodelist *TCtidyUpNodelist(nodelist *list);


/*
 *
 *  functionname  : TCconcatNodelist
 *  arguments     : 1) first node list
 *                  2) second node list
 *  description   : concatenates two node lists without checking double
 *                  occurrences
 *
 */

extern nodelist *TCconcatNodelist(nodelist *first, nodelist *second);
#endif

/******************************************************************************
 *
 * function:
 *   nodelist *TCnodeListAppend( nodelist *nl, node *newnode, void *attrib)
 *   nodelist *TCnodeListDelete( nodelist *nl, node *node, bool free_attrib)
 *   nodelist *TCnodeListFree( nodelist *nl, bool free_attrib)
 *   nodelist *TCnodeListFind( nodelist *nl, node *node)
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
/* TODO - void* be deleted after SACDevCampDK 2k4 */
extern nodelist *TCnodeListAppend (nodelist *nl, node *newnode, void *attrib);
extern nodelist *TCnodeListDelete (nodelist *nl, node *node, bool free_attrib);
extern nodelist *TCnodeListFree (nodelist *nl, bool free_attrib);
extern nodelist *TCnodeListFind (nodelist *nl, node *node);

/*--------------------------------------------------------------------------*/

/***
 ***  ARGTAB :
 ***/

extern int TCgetArgtabIndexOut (types *type, argtab_t *argtab);
extern int TCgetArgtabIndexIn (types *type, argtab_t *argtab);

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

#define NODE_ISALIVE(n) (n->attribs.any != NULL)

/*****************************************************************************
 *
 * function:
 *   bool TCisImported( node* symbol)
 *   bool TCisExternal( node* symbol)
 *   bool TCisFromModule( node* symbol)
 *   bool TCisFromClass( node* symbol)
 *
 * description:
 *
 *   These test functions are applicable to any kind of symbol,
 *   more precisely to N_typedef, N_objdef, and N_fundef nodes.
 *
 *****************************************************************************/

extern bool TCisImported (node *symbol);
extern bool TCisExternal (node *symbol);
extern bool TCisFromModule (node *symbol);
extern bool TCisFromClass (node *symbol);

/*--------------------------------------------------------------------------*/

/***
 ***  N_module :
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
 *  functionname  : TCsearchTypedef
 *  arguments     : 1) type name to be searched for
 *                  2) module name of type to be searched for
 *                  3) list of type implementations (typedef nodes)
 *  description   : looks for a certain typedef in list of typedefs
 *
 */

extern node *TCSearchTypedef (char *name, char *mod, node *implementations);

/*****************************************************************************
 *
 * function:
 *   node *TCappendTypedef( node *tdef_chain, node *tdef)
 *
 * description:
 *
 *   This function concatenates two chains of N_typedef nodes.
 *
 *****************************************************************************/

extern node *TCappendTypedef (node *tdef_chain, node *tdef);

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
 *  functionname  : TCsearchObjdef
 *  arguments     : 1) global object name to be searched for
 *                  2) module name of global object to be searched for
 *                  3) list of object implementations (objdef nodes)
 *  description   : looks for a certain objdef in list of objdefs
 *
 */

extern node *TCsearchObjdef (char *name, char *mod, node *implementations);

/*
 *  functionname  : TCobjList2ArgList
 *  arguments     : 1) pointer to chain of objdef nodes
 *  description   : makes an argument list from an objdef chain
 *
 */

extern void TCobjList2ArgList (node *objdef);

/*****************************************************************************
 *
 * function:
 *   node *TCappendObjdef( node *objdef_chain, node *objdef)
 *
 * description:
 *
 *   This function concatenates two chains of N_objdef nodes.
 *
 *****************************************************************************/

extern node *TCappendObjdef (node *objdef_chain, node *objdef);

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

#define FUNDEF_LINKNAME(n) (PRAGMA_LINKNAME (FUNDEF_PRAGMA (n)))
#define FUNDEF_LINKSIGN(n) (PRAGMA_LINKSIGN (FUNDEF_PRAGMA (n)))
#define FUNDEF_REFCOUNTING(n) (PRAGMA_REFCOUNTING (FUNDEF_PRAGMA (n)))

#define FUNDEF_ISLACFUN(n) (FUNDEF_ISCONDFUN (n) || FUNDEF_ISDOFUN (n))
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

extern node *TCfindVardec_Name (char *name, node *fundef);
extern node *TCfindVardec_Varno (int varno, node *fundef);

/*
 *
 *  functionname  : TCcountFunctionParams
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

extern int TCcountFunctionParams (node *fundef);

/*
 *  functionname  : SearchFundef
 *  arguments     : 1) fundef node of function to search for
 *                  2) ptr to head of fundef chain
 *  description   : returns a ptr to the respective fundef node
 *  remarks       : This function is used to find the implementation of
 *                  a function which is declared in a module/class
 *                  implementation. For the representation of function
 *                  declarations the fundef node is reused.
 */

extern node *TCsearchFundef (node *fun, node *allfuns);

/*****************************************************************************
 *
 * function:
 *   node *TCappendFundef( node *fundef_chain, node *fundef)
 *
 * description:
 *
 *   This function concatenates two chains of N_fundef nodes.
 *
 *****************************************************************************/

extern node *TCappendFundef (node *fundef_chain, node *fundef);

extern node *TCremoveFundef (node *fundef_chain, node *fundef);

/*--------------------------------------------------------------------------*/

/***
 ***  N_block :
 ***/

/*
 *  compound access macros
 */
#define MAKE_EMPTY_BLOCK() TBmakeBlock (TBmakeEmpty (), NULL)

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :
 ***/

/*
 *  compound access macros
 */

#define VARDEC_NTYPE(n) (AVIS_TYPE (VARDEC_AVIS (n)))
#define VARDEC_NAME(n) (AVIS_NAME (VARDEC_AVIS (n)))
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
 *   node *TCaddVardecs( node *fundef, node *vardecs)
 *
 * Description:
 *   Inserts new declarations into the AST, updates the DFMbase and returns
 *   the modified N_fundef node.
 *
 ******************************************************************************/

extern node *TCaddVardecs (node *fundef, node *vardecs);

/******************************************************************************
 *
 * function:
 *   node *TCappendVardec( node *vardec_chain, node *vardec)
 *
 * description:
 *   Appends 'vardec' to 'vardec_chain' and returns the new chain.
 *
 ******************************************************************************/

extern node *TCappendVardec (node *vardec_chain, node *vardec);

/******************************************************************************
 *
 * function:
 *   node *TCmakeVardecFromArg( node *arg)
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

extern node *TCmakeVardecFromArg (node *arg_node);

/******************************************************************************
 *
 * function:
 *   node *TCmakeArgFromVardec( node *vardec_node)
 *
 * description:
 *   copies all attributes from an vardec node to a new allocated arg node.
 *
 ******************************************************************************/

extern node *TCmakeArgFromVardec (node *vardec_node);

/******************************************************************************
 *
 * function:
 *   node *TCadjustAvisData( node *new_vardec, node *fundef)
 *
 * description:
 *   when a vardec is duplicated via DupTree all dependend infomation in the
 *   corresponding avis node is duplicated, too. when this vardec is used in
 *   the same fundef as the original one everything is good, but if the
 *   duplicated vardec should be used in a different fundef the fundef related
 *   attributes have to be adjusted by this function:
 *     AVIS_SSACOUNT = (new fresh ssacnt node)
 *     AVIS_SSALPINV = FALSE
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

extern node *TCadjustAvisData (node *new_vardec, node *fundef);

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***/

/*
 *  compound access macros
 */

#define ARG_NAME(n) (AVIS_NAME (ARG_AVIS (n)))
#define ARG_NTYPE(n) (AVIS_TYPE (ARG_AVIS (n)))
#define ARG_BASETYPE(n) (TYPES_BASETYPE (ARG_TYPE (n)))
#define ARG_DIM(n) (TYPES_DIM (ARG_TYPE (n)))
#define ARG_SHAPE(n, x) (TYPES_SHAPE (ARG_TYPE (n), x))
#define ARG_SHPSEG(n) (TYPES_SHPSEG (ARG_TYPE (n)))
#define ARG_TNAME(n) (TYPES_NAME (ARG_TYPE (n)))
#define ARG_TMOD(n) (TYPES_MOD (ARG_TYPE (n)))
#define ARG_TDEF(n) (TYPES_TDEF (ARG_TYPE (n)))

extern int TCcountArgs (node *args);
extern int TCcmpDomain (node *args1, node *args2);

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :  *and*  N_arg :
 ***/

/*
 * CAUTION: Do not use the following macros as l-values!!!
 *          (this is *no* ANSI C style!)
 *          Use the L_VARDEC_OR_... macros instead!!
 */
#define VARDEC_OR_ARG_NAME(n)                                                            \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_NAME (n)                                                                    \
       : ((NODE_TYPE (n) == N_vardec) ? VARDEC_NAME (n) : OBJDEF_NAME (n)))
#define VARDEC_OR_ARG_TYPE(n)                                                            \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_TYPE (n)                                                                    \
       : ((NODE_TYPE (n) == N_vardec) ? VARDEC_TYPE (n) : OBJDEF_TYPE (n)))
#define VARDEC_OR_ARG_STATUS(n)                                                          \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_STATUS (n)                                                                  \
       : ((NODE_TYPE (n) == N_vardec)                                                    \
            ? VARDEC_STATUS (n)                                                          \
            : DBUG_ASSERT_EXPR (0, "VARDEC_OR_ARG_STATUS on objdef", 0)))
#define VARDEC_OR_ARG_ATTRIB(n)                                                          \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_ATTRIB (n)                                                                  \
       : ((NODE_TYPE (n) == N_vardec)                                                    \
            ? VARDEC_ATTRIB (n)                                                          \
            : DBUG_ASSERT_EXPR (0, "VARDEC_OR_ARG_ATTRIB on objdef", 0)))
#define VARDEC_OR_ARG_AVIS(n)                                                            \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_AVIS (n)                                                                    \
       : ((NODE_TYPE (n) == N_vardec) ? VARDEC_AVIS (n) : OBJDEF_AVIS (n)))
#define VARDEC_OR_ARG_VARNO(n)                                                           \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_VARNO (n)                                                                   \
       : ((NODE_TYPE (n) == N_vardec)                                                    \
            ? VARDEC_VARNO (n)                                                           \
            : DBUG_ASSERT_EXPR (0, "VARDEC_OR_ARG_VARNO on objdef", 0)))
#define VARDEC_OR_ARG_REFCNT(n)                                                          \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_REFCNT (n)                                                                  \
       : ((NODE_TYPE (n) == N_vardec)                                                    \
            ? VARDEC_REFCNT (n)                                                          \
            : DBUG_ASSERT_EXPR (0, "VARDEC_OR_ARG_REFCNT on objdef", 0)))
#define VARDEC_OR_ARG_NAIVE_REFCNT(n)                                                    \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_NAIVE_REFCNT (n)                                                            \
       : ((NODE_TYPE (n) == N_vardec)                                                    \
            ? VARDEC_NAIVE_REFCNT (n)                                                    \
            : DBUG_ASSERT_EXPR (0, "VARDEC_OR_ARG_NAIVE_REFCNT on objdef", 0)))
#define VARDEC_OR_ARG_NEXT(n)                                                            \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_NEXT (n)                                                                    \
       : ((NODE_TYPE (n) == N_vardec) ? VARDEC_NEXT (n) : OBJDEF_NEXT (n)))

#define VARDEC_OR_ARG_PADDED(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_PADDED (n) : VARDEC_PADDED (n))
#define VARDEC_OR_ARG_ACTCHN(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_ACTCHN (n) : VARDEC_ACTCHN (n))
#define VARDEC_OR_ARG_COLCHN(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_COLCHN (n) : VARDEC_COLCHN (n))
#define VARDEC_OR_ARG_OBJDEF(n)                                                          \
    ((NODE_TYPE (n) == N_arg) ? ARG_OBJDEF (n) : VARDEC_OBJDEF (n))

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
    } else if (NODE_TYPE (n) == N_vardec) {                                              \
        VARDEC_AVIS (n) = (rhs);                                                         \
    } else {                                                                             \
        OBJDEF_AVIS (n) = (rhs);                                                         \
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

extern node *TCsearchDecl (char *name, node *decl_node);

/*--------------------------------------------------------------------------*/

/***
 ***  N_assign :
 ***/

/*
 *  compound access macros
 */

#define ASSIGN_INSTRTYPE(n) NODE_TYPE (ASSIGN_INSTR (n))
#define ASSIGN_NAME(n) IDS_NAME (ASSIGN_LHS (n))
#define ASSIGN_LHS(n) LET_IDS (ASSIGN_INSTR (n))
#define ASSIGN_RHS(n) LET_EXPR (ASSIGN_INSTR (n))

/******************************************************************************
 *
 * function:
 *   node *TCmakeAssignLet(char *var_name, node *vardec_node, node *let_expr);
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

extern node *TCmakeAssignLet (char *var_name, node *vardec_node, node *let_expr);

/******************************************************************************
 *
 * Function:
 *   node *TCmakeAssignInstr( node *instr, node *next)
 *
 * Description:
 *
 *
 ******************************************************************************/

extern node *TCmakeAssignInstr (node *instr, node *next);

/******************************************************************************
 *
 * Function:
 *   node *TCmakeAssign?( node *part?, ...)
 *
 * Description:
 *
 *
 ******************************************************************************/

extern node *TCmakeAssigns1 (node *part1);
extern node *TCmakeAssigns2 (node *part1, node *part2);
extern node *TCmakeAssigns3 (node *part1, node *part2, node *part3);
extern node *TCmakeAssigns4 (node *part1, node *part2, node *part3, node *part4);
extern node *TCmakeAssigns5 (node *part1, node *part2, node *part3, node *part4,
                             node *part5);
extern node *TCmakeAssigns6 (node *part1, node *part2, node *part3, node *part4,
                             node *part5, node *part6);
extern node *TCmakeAssigns7 (node *part1, node *part2, node *part3, node *part4,
                             node *part5, node *part6, node *part7);
extern node *TCmakeAssigns8 (node *part1, node *part2, node *part3, node *part4,
                             node *part5, node *part6, node *part7, node *part8);
extern node *TCmakeAssigns9 (node *part1, node *part2, node *part3, node *part4,
                             node *part5, node *part6, node *part7, node *part8,
                             node *part9);

/******************************************************************************
 *
 * function:
 *   node *TCmakeAssignIcm0( char *name, node *next)
 *   node *TCmakeAssignIcm?( char *name, node *arg?, ..., node *next)
 *
 * description:
 *   These functions generate an N_assign node with a complete ICM
 *   representations including arguments as body.
 *   Each function argument may be an arbitrary list of single ICM arguments.
 *   These are concatenated correctly.
 *   The ASSIGN_NEXT will be NULL!
 *
 ******************************************************************************/

extern node *TCmakeAssignIcm0 (char *name, node *next);
extern node *TCmakeAssignIcm1 (char *name, node *arg1, node *next);
extern node *TCmakeAssignIcm2 (char *name, node *arg1, node *arg2, node *next);
extern node *TCmakeAssignIcm3 (char *name, node *arg1, node *arg2, node *arg3,
                               node *next);
extern node *TCmakeAssignIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                               node *next);
extern node *TCmakeAssignIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                               node *arg5, node *next);
extern node *TCmakeAssignIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                               node *arg5, node *arg6, node *next);
extern node *TCmakeAssignIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                               node *arg5, node *arg6, node *arg7, node *next);

/******************************************************************************
 *
 * function:
 *   node *TCgetCompoundNode(node *arg_node);
 *
 * description:
 *   returns the compund_node that is attached to the assign-node
 *
 ******************************************************************************/

extern node *TCgetCompoundNode (node *arg_node);

/*****************************************************************************
 *
 * function:
 *   TCappendAssign( node *assign_chain, node *assign)
 *
 * description:
 *   This function concatenates two chains of N_assign nodes.
 *   However, the first one may simply be a single N_empty node.
 *
 *****************************************************************************/

extern node *TCappendAssign (node *assign_chain, node *assign);

/******************************************************************************
 *
 * function:
 *   node *TCappendAssignIcm( node *assign, char *name, node *args)
 *
 * description:
 *   Appends a new ICM with name and args given as an assign to the given
 *   chain of assignments.
 *
 ******************************************************************************/

extern node *TCappendAssignIcm (node *assign, char *name, node *args);

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :        (see also N_array !!!)
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCappendExprs( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_exprs chains of nodes.
 *
 ******************************************************************************/

extern node *TCappendExprs (node *exprs1, node *exprs2);

/******************************************************************************
 *
 * function:
 *   node *TCcombineExprs( node *first, node *second)
 *
 * description:
 *   'first' and 'second' are N_exprs chains or expression nodes (N_id, N_num,
 *   ...) that will be conactenated to a single N_exprs chain.
 *
 ******************************************************************************/

extern node *TCcombineExprs (node *first, node *second);

/******************************************************************************
 *
 * function:
 *   node *TCmakeExprsNum( int num)
 *
 * description:
 *   Makes an N_exprs with a N_num as EXPR, NEXT is NULL.
 *
 ******************************************************************************/

extern node *TCmakeExprsNum (int num);

/******************************************************************************
 *
 * function:
 *   int TCcountExprs( node *exprs)
 *
 * description:
 *   Computes the length of the given N_exprs chain.
 *
 ******************************************************************************/

extern int TCcountExprs (node *exprs);

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
#define LET_MOD(n) (IDS_MOD (LET_IDS (n)))
#define LET_STATUS(n) (IDS_STATUS (LET_IDS (n)))
#define LET_BASETYPE(n) (TYPES_BASETYPE (LET_TYPE (n)))
#define LET_USE(n) (IDS_USE (LET_IDS (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_cast :
 ***/

/*
 *
 *  functionname  : TCnodeBehindCast
 *  arguments     : 1) expression-node of a let-node
 *                  R) node behind various cast's
 *  description   : determine what node is hidden behind the cast-nodes
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..
 *
 *  remarks       :
 *
 */

extern node *TCnodeBehindCast (node *arg_node);

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
#define COND_THENINSTR(n) (BLOCK_INSTR (COND_THEN (n)))
#define COND_ELSEINSTR(n) (BLOCK_INSTR (COND_ELSE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :
 ***/

/*
 *  compound access macros
 */
#define DO_INSTR(n) (BLOCK_INSTR (DO_BODY (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_while :
 ***/

/*
 *  compound access macros
 */
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

#define DO_OR_WHILE_INSTR(n) ((NODE_TYPE (n) == N_do) ? DO_INSTR (n) : WHILE_INSTR (n))

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
#define ARRAY_DIM(n) (SHGetDim (ARRAY_SHAPE (n)))
#define ARRAY_SHPSEG(n) (TYPES_SHPSEG (ARRAY_TYPE (n)))
#define ARRAY_TNAME(n) (TYPES_NAME (ARRAY_TYPE (n)))
#define ARRAY_TMOD(n) (TYPES_MOD (ARRAY_TYPE (n)))
#define ARRAY_TDEF(n) (TYPES_TDEF (ARRAY_TYPE (n)))

/*
 *  function declarations
 */
extern node *TCmakeFlatArray (node *aelems);
extern node *TCcreateZeroScalar (simpletype btype);
extern node *TCcreateZeroVector (int length, simpletype btype);

/******************************************************************************
 *
 * Function:
 *   int TCisConstArray( node *array);
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

extern int TCisConstArray (node *array);

extern node *TCids2Exprs (node *ids_arg);
extern node *TCids2Array (node *ids_arg);

/******************************************************************************
 *
 * Function:
 *   node *TCintVec2Array( int length, int *intvec);
 *
 * Description:
 *   Returns an N_exprs node containing the elements of intvec.
 *
 ******************************************************************************/

extern node *TCintVec2Array (int length, int *intvec);

/*****************************************************************************
 *
 * Function:
 *   node *TCadjustVectorShape( node *array)
 *
 * Description:
 *   adjusts ARRAY_SHAPE according to the number of elements.
 *   Note that the array will always be one-dimensional
 *
 *****************************************************************************/

extern node *TCadjustVectorShape (node *array);

/*****************************************************************************
 *
 * Function:
 *   node *TCconcatVecs( node* vec1, node *vec2 )
 *
 * Description:
 *   concatenates two vectors.
 *
 *****************************************************************************/

extern node *TCconcatVecs (node *vec1, node *vec2);

/******************************************************************************
 *
 * Function:
 *   int    *TCarray2IntVec( node *aelems, int *length);
 *   int    *TCarray2BoolVec( node *aelems, int *length);
 *   char   *TCarray2CharVec( node *aelems, int *length);
 *   float  *TCarray2FloatVec( node *aelems, int *length);
 *   double *TCarray2DblVec( node *aelems, int *length);
 *
 *   void   *TCarray2Vec( simpletype t, node *aelems, int *length);
 *
 * Description:
 *   Returns an iteger (char | float | double) vector and stores the number of
 *   constant integer (char | float | double) elements in *length if first
 *   argument is an N_exprs and all its elements are N_num otherwise the
 *   result is not defined.
 *   If the length of the vector is not of interest, length may be NULL.
 *   The TCarray2Vec function is a dispatching wrapper for all above.
 *
 ******************************************************************************/

extern int *TCarray2IntVec (node *aelems, int *length);
extern int *TCarray2BoolVec (node *aelems, int *length);
extern char *TCarray2CharVec (node *aelems, int *length);
extern float *TCarray2FloatVec (node *aelems, int *length);
extern double *TCarray2DblVec (node *aelems, int *length);
extern void *TCarray2Vec (simpletype t, node *aelems, int *length);

/*--------------------------------------------------------------------------*/

/***
 ***  N_vinfo :
 ***/

extern node *TCmakeVinfoDollar (node *next);

/*--------------------------------------------------------------------------*/

/***
 ***  N_id :
 ***/

/*
 *  compound access macros
 */

#define ID_VARNO(n) VARDEC_OR_ARG_VARNO (ID_VARDEC (n))
#define ID_NAME(n) AVIS_NAME (ID_AVIS (n))
#define ID_NTYPE(n) AVIS_TYPE (ID_AVIS (n))
#define ID_DIM(n) VARDEC_OR_ARG_DIM (ID_VARDEC (n))
#define ID_SHPSEG(n) TYPES_SHPSEG (VARDEC_OR_ARG_TYPE (ID_VARDEC (n)))
#define ID_SHAPE(n, x) SHPSEG_SHAPE (ID_SHPSEG (n), x)
#define ID_VARDEC_NAME(n) VARDEC_OR_ARG_NAME (ID_VARDEC (n))
#define ID_VARDEC_NEXT(n) VARDEC_OR_ARG_NEXT (ID_VARDEC (n))
#define ID_PADDED(n) VARDEC_OR_ARG_PADDED (ID_VARDEC (n))

#define ID_TYPE(n)                                                                       \
    ((NODE_TYPE (AVIS_DECL (ID_AVIS (n))) == N_vardec)                                   \
       ? VARDEC_TYPE (AVIS_DECL (ID_AVIS (n)))                                           \
       : ((NODE_TYPE (AVIS_DECL (ID_AVIS (n))) == N_arg)                                 \
            ? ARG_TYPE (AVIS_DECL (ID_AVIS (n)))                                         \
            : NULL))

#define ID_SSAASSIGN(n) (AVIS_SSAASSIGN (ID_AVIS (n)))

#define ID_OR_CAST_TYPE(n) ((NODE_TYPE (n) == N_id) ? ID_TYPE (n) : CAST_TYPE (n))
#define ID_OR_ARRAY_TYPE(n) ((NODE_TYPE (n) == N_id) ? ID_TYPE (n) : ARRAY_TYPE (n))

extern node *TCmakeId_Copy_NT (char *str, types *type);

extern node *TCmakeId_Num (int val);

extern node *TCmakeIdFromIds (node *idss);

/***************************************************************************
 *
 * function:
 *   bool TCisPhiFun(node *id)
 *
 * description:
 *   this function returns TRUE if the defining assignment of 'id'
 *   uses the primitive phi function 'F_phi'. In all other cases id
 *   returns FALSE.
 *   This function replaces the PHITARGET macro used to identify phi functions.
 *
 ****************************************************************************/

extern bool PHisPhiFun (node *id);

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
#define PRF_EXPRS4(n) EXPRS_EXPRS4 (PRF_ARGS (n))

#define PRF_ARG1(n) EXPRS_EXPR (PRF_EXPRS1 (n))
#define PRF_ARG2(n) EXPRS_EXPR (PRF_EXPRS2 (n))
#define PRF_ARG3(n) EXPRS_EXPR (PRF_EXPRS3 (n))
#define PRF_ARG4(n) EXPRS_EXPR (PRF_EXPRS4 (n))

#define MAKE_BIN_PRF(f, arg1, arg2)                                                      \
    TBmakePrf (f, TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)))

/*
 *  function declarations
 */

extern node *TCmakePrf1 (prf prf, node *arg1);
extern node *TCmakePrf2 (prf prf, node *arg1, node *arg2);
extern node *TCmakePrf3 (prf prf, node *arg1, node *arg2, node *arg3);

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

extern node *TCmakeAp1 (char *name, char *mod, node *arg1);

extern node *TCmakeAp2 (char *name, char *mod, node *arg1, node *arg2);

extern node *TCmakeAp3 (char *name, char *mod, node *arg1, node *arg2, node *arg3);

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :  *and*  N_ap :
 ***
 ***  watch simple macros for N_ap and N_prf
 ***  (search for "N_ap :" or "N_prf :").
 ***/

#define L_AP_OR_PRF_ARGS(n, rhs)                                                         \
    if (NODE_TYPE (n) == N_ap) {                                                         \
        AP_ARGS (n) = (rhs);                                                             \
    } else {                                                                             \
        PRF_ARGS (n) = (rhs);                                                            \
    }

#define AP_OR_PRF_ARGS(n) ((NODE_TYPE (n) == N_ap) ? AP_ARGS (n) : PRF_ARGS (n))

/******************************************************************************
 *
 * Function:
 *   node *TCliftArg( node *arg, node *fundef, types *new_type, bool do_rc,
 *                    node **new_assigns)
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

node *TCliftArg (node *arg, node *fundef, types *new_type, bool do_rc,
                 node **new_assigns);

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
 *   node *TCmakeIcm0(char *name)
 *   node *TCmakeIcm?(char *name, node *arg1, ...)
 *
 * description:
 *   These functions generate complete ICM representations including arguments.
 *   Each function argument may be an arbitrary list of single ICM arguments.
 *   These are concatenated correctly.
 *
 ******************************************************************************/

extern node *TCmakeIcm0 (char *name);
extern node *TCmakeIcm1 (char *name, node *arg1);
extern node *TCmakeIcm2 (char *name, node *arg1, node *arg2);
extern node *TCmakeIcm3 (char *name, node *arg1, node *arg2, node *arg3);
extern node *TCmakeIcm4 (char *name, node *arg1, node *arg2, node *arg3, node *arg4);
extern node *TCmakeIcm5 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                         node *arg5);
extern node *TCmakeIcm6 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                         node *arg5, node *arg6);
extern node *TCmakeIcm7 (char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                         node *arg5, node *arg6, node *arg7);

/*--------------------------------------------------------------------------*/

/***
 ***  N_mt :   N_st :   N_ex :
 ***/

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
 ***  N_with :
 ***/

#define WITH_TYPE(n) (WITHOP_TYPE (WITH_WITHOP (n)))

/*
 * We only need to inspect the withid of the first part,
 * because the withid is in *all* parts the same!!
 */
#define WITH_WITHID(n) (PART_WITHID (WITH_PART (n)))
#define WITH_IDS(n) (PART_IDS (WITH_PART (n)))
#define WITH_VEC(n) (PART_VEC (WITH_PART (n)))

/*
 * BOUND1, BOUND2, STEP, WIDTH of the *first* N_Npart-node
 * (useful *before* with-loop-folding only!!!)
 */
#define WITH_BOUND1(n) (PART_BOUND1 (WITH_PART (n)))
#define WITH_BOUND2(n) (PART_BOUND2 (WITH_PART (n)))
#define WITH_STEP(n) (PART_STEP (WITH_PART (n)))
#define WITH_WIDTH(n) (PART_WIDTH (WITH_PART (n)))

/*
 * CBLOCK, CEXPR of the *first* N_code-node
 * (useful in case of single-generator with-loops only,
 *  e.g. before with-loop-folding)
 */
#define WITH_CBLOCK(n) (CODE_CBLOCK (WITH_CODE (n)))
#define WITH_CEXPR(n) (CODE_CEXPR (WITH_CODE (n)))

extern node *TCcreateScalarWith (int dim, shpseg *shape, simpletype btype, node *expr,
                                 node *fundef);

extern node *TCcreateZero (int dim, shpseg *shape, simpletype btype, bool unroll,
                           node *fundef);

extern node *TCcreateSel (node *sel_vec, node *sel_ids, node *sel_array, bool no_wl,
                          node *fundef);

/*--------------------------------------------------------------------------*/

/***
 ***  N_part :
 ***/

#define PART_IDS(n) (WITHID_IDS (PART_WITHID (n)))
#define PART_VEC(n) (WITHID_VEC (PART_WITHID (n)))

#define PART_BOUND1(n) (GENERATOR_BOUND1 (PART_GENERATOR (n)))
#define PART_BOUND2(n) (GENERATOR_BOUND2 (PART_GENERATOR (n)))
#define PART_STEP(n) (GENERATOR_STEP (PART_GENERATOR (n)))
#define PART_WIDTH(n) (GENERATOR_WIDTH (PART_GENERATOR (n)))

#define PART_CEXPR(n) (CODE_CEXPR (PART_CODE (n)))
#define PART_CBLOCK(n) (CODE_CBLOCK (PART_CODE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_code :
 ***/

#define CODE_CBLOCK_INSTR(n) (BLOCK_INSTR (CODE_CBLOCK (n)))
#define CODE_CEXPR(n) EXPRS_EXPR (CODE_CEXPRS (n))

#define CODE_WLAA_ACCESS(n) (CODE_WLAA_INFO (n)->access)
#define CODE_WLAA_ACCESSCNT(n) (CODE_WLAA_INFO (n)->accesscnt)
#define CODE_WLAA_FEATURE(n) (CODE_WLAA_INFO (n)->feature)
#define CODE_WLAA_INDEXVAR(n) (CODE_WLAA_INFO (n)->indexvar)
#define CODE_WLAA_WLARRAY(n) (CODE_WLAA_INFO (n)->wlarray)

#define CODE_WLAA_ARRAYSHP(n) VARDEC_SHPSEG (CODE_WLAA_WLARRAY (n))
#define CODE_WLAA_INDEXDIM(n) VARDEC_SHAPE (CODE_WLAA_INDEXVAR (n), 0)
#define CODE_WLAA_ARRAYDIM(n) VARDEC_DIM (CODE_WLAA_WLARRAY (n))

#define CODE_INC_USED(n) CODE_USED (n) = CODE_USED (n) + 1
#define CODE_DEC_USED(n)                                                                 \
    if (CODE_USED (n) == 0) {                                                            \
        DBUG_ASSERT (0, "CODE_USED dropped below 0");                                    \
    } else {                                                                             \
        CODE_USED (n) = CODE_USED (n) - 1;                                               \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  withop :
 ***/

#define WITHOP_NEXT(n)                                                                   \
    ((NODE_TYPE (n) == N_genarray)                                                       \
       ? GENARRAY_NEXT (n)                                                               \
       : ((NODE_TYPE (n) == N_modarray) ? MODARRAY_NEXT (n) : FOLD_NEXT (n)))

#define WITHOP_MEM(n)                                                                    \
    ((NODE_TYPE (n) == N_genarray) ? GENARRAY_MEM (n) : MODARRAY_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_with2 :
 ***/

#define WITH2_TYPE(n) (WITHOP_TYPE (WITH2_WITHOP (n)))

#define WITH2_IDS(n) (WITHID_IDS (WITH2_WITHID (n)))
#define WITH2_VEC(n) (WITHID_VEC (WITH2_WITHID (n)))

/*
 * CBLOCK, CEXPR of the *first* N_code-node
 * (useful in case of single-generator with-loops only,
 *  e.g. before with-loop-folding)
 */
#define WITH2_CBLOCK(n) (CODE_CBLOCK (WITH2_CODE (n)))
#define WITH2_CEXPR(n) (CODE_CEXPR (WITH2_CODE (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_with :  *and*  N_with2 :
 ***/

#define WITH_OR_WITH2_TYPE(n) ((NODE_TYPE (n) == N_with) ? WITH_TYPE (n) : WITH2_TYPE (n))
#define WITH_OR_WITH2_IDS(n) ((NODE_TYPE (n) == N_with) ? WITH_IDS (n) : WITH2_IDS (n))
#define WITH_OR_WITH2_VEC(n) ((NODE_TYPE (n) == N_with) ? WITH_VEC (n) : WITH2_VEC (n))

#define WITH_OR_WITH2_CEXPR(n)                                                           \
    ((NODE_TYPE (n) == N_with) ? WITH_CEXPR (n) : WITH2_CEXPR (n))

#define WITH_OR_WITH2_DEC_RC_IDS(n)                                                      \
    ((NODE_TYPE (n) == N_with) ? WITH_DEC_RC_IDS (n) : WITH2_DEC_RC_IDS (n))

#define WITH_OR_WITH2_IN_MASK(n)                                                         \
    ((NODE_TYPE (n) == N_with) ? WITH_IN_MASK (n) : WITH2_IN_MASK (n))
#define WITH_OR_WITH2_OUT_MASK(n)                                                        \
    ((NODE_TYPE (n) == N_with) ? WITH_OUT_MASK (n) : WITH2_OUT_MASK (n))
#define WITH_OR_WITH2_LOCAL_MASK(n)                                                      \
    ((NODE_TYPE (n) == N_with) ? WITH_LOCAL_MASK (n) : WITH2_LOCAL_MASK (n))

#define L_WITH_OR_WITH2_DEC_RC_IDS(n, rhs)                                               \
    if (NODE_TYPE (n) == N_with) {                                                       \
        WITH_DEC_RC_IDS (n) = (rhs);                                                     \
    } else {                                                                             \
        WITH2_DEC_RC_IDS (n) = (rhs);                                                    \
    }

#define L_WITH_OR_WITH2_IN_MASK(n, rhs)                                                  \
    if (NODE_TYPE (n) == N_with) {                                                       \
        WITH_IN_MASK (n) = (rhs);                                                        \
    } else {                                                                             \
        WITH2_IN_MASK (n) = (rhs);                                                       \
    }

#define L_WITH_OR_WITH2_OUT_MASK(n, rhs)                                                 \
    if (NODE_TYPE (n) == N_with) {                                                       \
        WITH_OUT_MASK (n) = (rhs);                                                       \
    } else {                                                                             \
        WITH2_OUT_MASK (n) = (rhs);                                                      \
    }

#define L_WITH_OR_WITH2_LOCAL_MASK(n, rhs)                                               \
    if (NODE_TYPE (n) == N_with) {                                                       \
        WITH_LOCAL_MASK (n) = (rhs);                                                     \
    } else {                                                                             \
        WITH2_LOCAL_MASK (n) = (rhs);                                                    \
    }

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

#define WLSEGX_DIMS(n) ((NODE_TYPE (n) == N_WLseg) ? WLSEG_DIMS (n) : WLSEGVAR_DIMS (n))

#define WLSEGX_CONTENTS(n)                                                               \
    ((NODE_TYPE (n) == N_WLseg) ? WLSEG_CONTENTS (n) : WLSEGVAR_CONTENTS (n))

#define WLSEGX_NEXT(n) ((NODE_TYPE (n) == N_WLseg) ? WLSEG_NEXT (n) : WLSEGVAR_NEXT (n))

#define WLSEGX_SCHEDULING(n)                                                             \
    ((NODE_TYPE (n) == N_WLseg) ? WLSEG_SCHEDULING (n) : WLSEGVAR_SCHEDULING (n))

#define WLSEGX_TASKSEL(n)                                                                \
    ((NODE_TYPE (n) == N_WLseg) ? WLSEG_TASKSEL (n) : WLSEGVAR_TASKSEL (n))

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

#define WLXBLOCK_LEVEL(n)                                                                \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_LEVEL (n) : WLUBLOCK_LEVEL (n))

#define WLXBLOCK_DIM(n)                                                                  \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_DIM (n) : WLUBLOCK_DIM (n))

#define WLXBLOCK_BOUND1(n)                                                               \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_BOUND1 (n) : WLUBLOCK_BOUND1 (n))

#define WLXBLOCK_BOUND2(n)                                                               \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_BOUND2 (n) : WLUBLOCK_BOUND2 (n))

#define WLXBLOCK_STEP(n)                                                                 \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_STEP (n) : WLUBLOCK_STEP (n))

#define WLXBLOCK_NEXTDIM(n)                                                              \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_NEXTDIM (n) : WLUBLOCK_NEXTDIM (n))

#define WLXBLOCK_CONTENTS(n)                                                             \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_CONTENTS (n) : WLUBLOCK_CONTENTS (n))

#define WLXBLOCK_NEXT(n)                                                                 \
    ((NODE_TYPE (n) == N_WLblock) ? WLBLOCK_NEXT (n) : WLUBLOCK_NEXT (n))

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

#define WLSTRIDEX_LEVEL(n)                                                               \
    ((NODE_TYPE (n) == N_WLstride) ? WLSTRIDE_LEVEL (n) : WLSTRIDEVAR_LEVEL (n))

#define WLSTRIDEX_DIM(n)                                                                 \
    ((NODE_TYPE (n) == N_WLstride) ? WLSTRIDE_DIM (n) : WLSTRIDEVAR_DIM (n))

#define WLSTRIDEX_CONTENTS(n)                                                            \
    ((NODE_TYPE (n) == N_WLstride) ? WLSTRIDE_CONTENTS (n) : WLSTRIDEVAR_CONTENTS (n))

#define WLSTRIDEX_NEXT(n)                                                                \
    ((NODE_TYPE (n) == N_WLstride) ? WLSTRIDE_NEXT (n) : WLSTRIDEVAR_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLgrid :
 ***/

#define WLGRID_CBLOCK(n) (CODE_CBLOCK (WLGRID_CODE (n)))
#define WLGRID_CEXPR(n) (CODE_CEXPR (WLGRID_CODE (n)))

#define WLGRID_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRID_CBLOCK (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLgridVar :
 ***/

#define WLGRIDVAR_CBLOCK(n) (CODE_CBLOCK (WLGRIDVAR_CODE (n)))
#define WLGRIDVAR_CEXPR(n) (CODE_CEXPR (WLGRIDVAR_CODE (n)))

#define WLGRIDVAR_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRIDVAR_CBLOCK (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_WLgrid :  *and*  N_WLgridVar :
 ***/

#define WLGRIDX_LEVEL(n)                                                                 \
    ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_LEVEL (n) : WLGRIDVAR_LEVEL (n))

#define WLGRIDX_DIM(n) ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_DIM (n) : WLGRIDVAR_DIM (n))

#define WLGRIDX_FITTED(n)                                                                \
    ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_FITTED (n) : WLGRIDVAR_FITTED (n))

#define WLGRIDX_NEXTDIM(n)                                                               \
    ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_NEXTDIM (n) : WLGRIDVAR_NEXTDIM (n))

#define WLGRIDX_NEXT(n)                                                                  \
    ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_NEXT (n) : WLGRIDVAR_NEXT (n))

#define WLGRIDX_CODE(n)                                                                  \
    ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_CODE (n) : WLGRIDVAR_CODE (n))

#define WLGRIDX_NOOP(n)                                                                  \
    ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_NOOP (n) : WLGRIDVAR_NOOP (n))

#define WLGRIDX_CBLOCK(n) (CODE_CBLOCK (WLGRIDX_CODE (n)))
#define WLGRIDX_CEXPR(n) (CODE_CEXPR (WLGRIDX_CODE (n)))

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

#define WLNODE_NEXT(n)                                                                   \
    ((NODE_TYPE (n) == N_WLblock)                                                        \
       ? WLBLOCK_NEXT (n)                                                                \
       : ((NODE_TYPE (n) == N_WLublock)                                                  \
            ? WLUBLOCK_NEXT (n)                                                          \
            : ((NODE_TYPE (n) == N_WLstride)                                             \
                 ? WLSTRIDE_NEXT (n)                                                     \
                 : ((NODE_TYPE (n) == N_WLstrideVar)                                     \
                      ? WLSTRIDEVAR_NEXT (n)                                             \
                      : ((NODE_TYPE (n) == N_WLgrid)                                     \
                           ? WLGRID_NEXT (n)                                             \
                           : ((NODE_TYPE (n) == N_WLgridVar) ? WLGRIDVAR_NEXT (n)        \
                                                             : NULL))))))

#define WLNODE_LEVEL(n)                                                                  \
    ((NODE_TYPE (n) == N_WLblock)                                                        \
       ? WLBLOCK_LEVEL (n)                                                               \
       : ((NODE_TYPE (n) == N_WLublock)                                                  \
            ? WLUBLOCK_LEVEL (n)                                                         \
            : ((NODE_TYPE (n) == N_WLstride)                                             \
                 ? WLSTRIDE_LEVEL (n)                                                    \
                 : ((NODE_TYPE (n) == N_WLstrideVar)                                     \
                      ? WLSTRIDEVAR_LEVEL (n)                                            \
                      : ((NODE_TYPE (n) == N_WLgrid)                                     \
                           ? WLGRID_LEVEL (n)                                            \
                           : ((NODE_TYPE (n) == N_WLgridVar) ? WLGRIDVAR_LEVEL (n)       \
                                                             : 0))))))

#define WLNODE_DIM(n)                                                                    \
    ((NODE_TYPE (n) == N_WLblock)                                                        \
       ? WLBLOCK_DIM (n)                                                                 \
       : ((NODE_TYPE (n) == N_WLublock)                                                  \
            ? WLUBLOCK_DIM (n)                                                           \
            : ((NODE_TYPE (n) == N_WLstride)                                             \
                 ? WLSTRIDE_DIM (n)                                                      \
                 : ((NODE_TYPE (n) == N_WLstrideVar)                                     \
                      ? WLSTRIDEVAR_DIM (n)                                              \
                      : ((NODE_TYPE (n) == N_WLgrid)                                     \
                           ? WLGRID_DIM (n)                                              \
                           : ((NODE_TYPE (n) == N_WLgridVar) ? WLGRIDVAR_DIM (n)         \
                                                             : 0))))))

#define WLNODE_BOUND1_INT(n)                                                             \
    ((NODE_TYPE (n) == N_WLblock)                                                        \
       ? WLBLOCK_BOUND1 (n)                                                              \
       : ((NODE_TYPE (n) == N_WLublock)                                                  \
            ? WLUBLOCK_BOUND1 (n)                                                        \
            : ((NODE_TYPE (n) == N_WLstride)                                             \
                 ? WLSTRIDE_BOUND1 (n)                                                   \
                 : ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_BOUND1 (n) : 0))))

#define WLNODE_BOUND1_NODE(n)                                                            \
    ((NODE_TYPE (n) == N_WLstrideVar)                                                    \
       ? WLSTRIDEVAR_BOUND1 (n)                                                          \
       : ((NODE_TYPE (n) == N_WLgridVar) ? WLGRIDVAR_BOUND1 (n) : NULL))

#define WLNODE_BOUND2_INT(n)                                                             \
    ((NODE_TYPE (n) == N_WLblock)                                                        \
       ? WLBLOCK_BOUND2 (n)                                                              \
       : ((NODE_TYPE (n) == N_WLublock)                                                  \
            ? WLUBLOCK_BOUND2 (n)                                                        \
            : ((NODE_TYPE (n) == N_WLstride)                                             \
                 ? WLSTRIDE_BOUND2 (n)                                                   \
                 : ((NODE_TYPE (n) == N_WLgrid) ? WLGRID_BOUND2 (n) : 0))))

#define WLNODE_BOUND2_NODE(n)                                                            \
    ((NODE_TYPE (n) == N_WLstrideVar)                                                    \
       ? WLSTRIDEVAR_BOUND2 (n)                                                          \
       : ((NODE_TYPE (n) == N_WLgridVar) ? WLGRIDVAR_BOUND2 (n) : NULL))

#define WLNODE_STEP_INT(n)                                                               \
    ((NODE_TYPE (n) == N_WLblock)                                                        \
       ? WLBLOCK_STEP (n)                                                                \
       : ((NODE_TYPE (n) == N_WLublock)                                                  \
            ? WLUBLOCK_STEP (n)                                                          \
            : ((NODE_TYPE (n) == N_WLstride) ? WLSTRIDE_STEP (n) : 0)))

#define WLNODE_NEXTDIM(n)                                                                \
    ((NODE_TYPE (n) == N_WLblock)                                                        \
       ? WLBLOCK_NEXTDIM (n)                                                             \
       : ((NODE_TYPE (n) == N_WLublock)                                                  \
            ? WLUBLOCK_NEXTDIM (n)                                                       \
            : ((NODE_TYPE (n) == N_WLgrid)                                               \
                 ? WLGRID_NEXTDIM (n)                                                    \
                 : ((NODE_TYPE (n) == N_WLgridVar) ? WLGRIDVAR_NEXTDIM (n) : NULL))))

#define L_WLNODE_NEXT(n, rhs)                                                            \
    if (NODE_TYPE (n) == N_WLblock)                                                      \
        WLBLOCK_NEXT (n) = rhs;                                                          \
    else if (NODE_TYPE (n) == N_WLublock)                                                \
        WLUBLOCK_NEXT (n) = rhs;                                                         \
    else if (NODE_TYPE (n) == N_WLstride)                                                \
        WLSTRIDE_NEXT (n) = rhs;                                                         \
    else if (NODE_TYPE (n) == N_WLstrideVar)                                             \
        WLSTRIDEVAR_NEXT (n) = rhs;                                                      \
    else if (NODE_TYPE (n) == N_WLgrid)                                                  \
        WLGRID_NEXT (n) = rhs;                                                           \
    else if (NODE_TYPE (n) == N_WLgridVar)                                               \
        WLGRIDVAR_NEXT (n) = rhs;                                                        \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_NEXT called on wrong node type");

#define L_WLNODE_BOUND1_INT(n, rhs)                                                      \
    if (NODE_TYPE (n) == N_WLblock)                                                      \
        WLBLOCK_BOUND1 (n) = rhs;                                                        \
    else if (NODE_TYPE (n) == N_WLublock)                                                \
        WLUBLOCK_BOUND1 (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_WLstride)                                                \
        WLSTRIDE_BOUND1 (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_WLgrid)                                                  \
        WLGRID_BOUND1 (n) = rhs;                                                         \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_BOUND1_INT called on wrong node type");

#define L_WLNODE_BOUND2_INT(n, rhs)                                                      \
    if (NODE_TYPE (n) == N_WLblock)                                                      \
        WLBLOCK_BOUND2 (n) = rhs;                                                        \
    else if (NODE_TYPE (n) == N_WLublock)                                                \
        WLUBLOCK_BOUND2 (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_WLstride)                                                \
        WLSTRIDE_BOUND2 (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_WLgrid)                                                  \
        WLGRID_BOUND2 (n) = rhs;                                                         \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_BOUND2_INT called on wrong node type");

#define L_WLNODE_STEP_INT(n, rhs)                                                        \
    if (NODE_TYPE (n) == N_WLblock)                                                      \
        WLBLOCK_STEP (n) = rhs;                                                          \
    else if (NODE_TYPE (n) == N_WLublock)                                                \
        WLUBLOCK_STEP (n) = rhs;                                                         \
    else if (NODE_TYPE (n) == N_WLstride)                                                \
        WLSTRIDE_STEP (n) = rhs;                                                         \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_STEP called on wrong node type");

#define L_WLNODE_NEXTDIM(n, rhs)                                                         \
    if (NODE_TYPE (n) == N_WLblock)                                                      \
        WLBLOCK_NEXTDIM (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_WLublock)                                                \
        WLUBLOCK_NEXTDIM (n) = rhs;                                                      \
    else if (NODE_TYPE (n) == N_WLgrid)                                                  \
        WLGRID_NEXTDIM (n) = rhs;                                                        \
    else if (NODE_TYPE (n) == N_WLgridVar)                                               \
        WLGRIDVAR_NEXTDIM (n) = rhs;                                                     \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_NEXTDIM called on wrong node type");

/*--------------------------------------------------------------------------*/

/***
 ***  N_dot :
 ***/

#define DOT_ISSINGLE(n) ((NODE_TYPE (n) == N_dot) && (DOT_NUM (n) == 1))

/*--------------------------------------------------------------------------*/

/***
 ***  N_avis :
 ***/

#define AVIS_SSASTACK_TOP(n) SSASTACK_AVIS (AVIS_SSASTACK (n))
#define AVIS_SSASTACK_INUSE(n) SSASTACK_INUSE (AVIS_SSASTACK (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_str :
 ***/

extern node *TCmakeStr_Copy (const char *str);

/*--------------------------------------------------------------------------*/

/***
 ***  N_linklist
 ***/

extern int AddLinkToLinks (node **links, node *link);
extern int AddLinksToLinks (node **links, node *add);
extern bool LinklistContains (node *set, node *link);
extern bool LinklistIsSubset (node *super, node *sub);

#endif /* _SAC_TREE_COMPOUND_H_ */
