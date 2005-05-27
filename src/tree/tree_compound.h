/*
 *
 * $Log$
 * Revision 3.190  2005/05/27 20:29:37  ktr
 * Inserted various L_...X macros
 *
 * Revision 3.189  2005/04/22 10:10:26  ktr
 * Removed CreateZeroFromType and code brushing
 *
 * Revision 3.188  2005/04/20 19:19:17  ktr
 * removed TCadjustAvisData and brushed the code
 *
 * Revision 3.187  2005/03/04 21:21:42  cg
 * Serious bug fixed in implementation of IDS_TYPE compound
 * macro. Now, ids referring to N_arg nodes no longer cause
 * segfaults.
 *
 * Revision 3.186  2004/12/19 23:16:37  ktr
 * removed TCcountFunctionParams
 *
 * Revision 3.185  2004/12/16 14:39:07  ktr
 * Cleaned up some WITH_OR_WITH2 macros.
 *
 * Revision 3.184  2004/12/12 08:00:49  ktr
 * removed sons.any, attribs.any, NODE_ISALIVE because they were incompatible
 * with CLEANMEM.
 *
 * Revision 3.183  2004/12/08 18:02:40  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 3.182  2004/12/07 20:35:53  ktr
 * eliminated CONSTVEC which is superseded by ntypes.
 *
 * Revision 3.181  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 3.180  2004/12/01 16:31:18  ktr
 * Some cleanup
 *
 * Revision 3.179  2004/11/29 17:29:49  sah
 * added ID_NAME_OR_SPNAME macro
 *
 * Revision 3.178  2004/11/27 02:03:25  jhb
 * MakeWlSegX changed to TCmakeWlSegX
 *
 * Revision 3.177  2004/11/27 01:35:28  sah
 * fixed it all, yeah!
 *
 * Revision 3.176  2004/11/26 21:52:28  ktr
 * LiftArg removed.
 *
 * Revision 3.175  2004/11/26 18:01:01  sbs
 * PRAGMA macros eliminated.
 *
 * Revision 3.174  2004/11/26 17:44:08  sbs
 * ARRAY_DIM typo fixed
 *
 * Revision 3.173  2004/11/26 17:33:05  skt
 * changed VARDEC_OR_ARG_AVIS
 *
 * Revision 3.172  2004/11/26 16:59:19  skt
 * some includes removed
 *
 * Revision 3.171  2004/11/26 16:53:42  skt
 * killed TCgetArgtabIndexOut/In during SDC2k4
 *
 * Revision 3.170  2004/11/26 14:11:47  skt
 * made it compilable during SACDevCampDK 2k4
 *
 * Revision 3.169  2004/11/26 12:32:40  skt
 * changed header of TCSearchObjectdef - added const
 *
 * Revision 3.168  2004/11/26 12:29:25  skt
 * TCmakeStrCopy re-added
 *
 * Revision 3.167  2004/11/26 12:05:50  sbs
 * *** empty log message ***
 *
 * Revision 3.166  2004/11/26 12:03:34  sbs
 * *** empty log message ***
 *
 * Revision 3.165  2004/11/26 11:59:25  ktr
 * changed *_NOOP into *_ISNOOP
 *
 * Revision 3.164  2004/11/26 11:52:38  sbs
 * changed IDS_VARDEC into IDS_DECL
 *
 * Revision 3.163  2004/11/26 11:30:53  sbs
 * ID_DECL_NAME eliminated
 *
 * Revision 3.162  2004/11/26 11:22:44  cg
 * TCappendRets renamed to TCappendRet to be consistent
 * with other append functions.
 *
 * Revision 3.161  2004/11/26 11:18:13  skt
 * some renaming - exclusive checkin for cg
 *
 * Revision 3.160  2004/11/25 22:18:43  mwe
 * missing brace paranthesis added
 *
 * Revision 3.159  2004/11/25 22:15:01  skt
 * according to some very special change for mwe
 *
 * Revision 3.158  2004/11/25 21:48:21  sbs
 * TCmakeIdsCopyString added
 *
 * Revision 3.157  2004/11/25 21:44:56  skt
 * some debugging
 *
 * Revision 3.156  2004/11/25 21:14:58  skt
 * killed TCmakeIdFromIds
 *
 * Revision 3.155  2004/11/25 21:03:34  skt
 * some parameter change
 *
 * Revision 3.154  2004/11/25 20:56:01  ktr
 * added TCmakeIdCopyString, TCmakeIdCopyStringNt
 *
 * Revision 3.153  2004/11/25 20:47:51  khf
 * additional macros inserted
 *
 * Revision 3.152  2004/11/25 20:15:15  skt
 * some brushing
 *
 * Revision 3.151  2004/11/25 20:10:12  sbs
 * signature of MakeAp<n> functions changed.
 *
 * Revision 3.150  2004/11/25 18:55:55  sbs
 * OBJDEF macros based on types * eliminated.
 *
 * Revision 3.149  2004/11/25 18:29:18  khf
 * + marcros
 *
 * Revision 3.148  2004/11/25 18:15:18  mwe
 * DECL_AVIS fixed
 *
 * Revision 3.147  2004/11/25 18:10:51  skt
 * superflous _ removed of TCgetTypes_Line
 *
 * Revision 3.146  2004/11/25 18:08:38  khf
 * inserted new(old) macros
 *
 * Revision 3.145  2004/11/25 17:20:09  sbs
 * added AP_NAME and AP_MOD
 *
 * Revision 3.144  2004/11/25 17:17:24  khf
 * changed MACROS for wltransform
 *
 * Revision 3.143  2004/11/25 15:41:23  jhb
 * added new macros L_WLSEGX_TASKSEL and L_WLSEGX_SCHEDULING
 *
 * Revision 3.142  2004/11/25 11:49:55  skt
 * OVRLD_FUN bitmask added
 *
 * Revision 3.141  2004/11/25 01:00:21  skt
 * bitmasks for profiling added
 *
 * Revision 3.140  2004/11/24 20:41:23  mwe
 * TCreturnTypes2Ret added
 *
 * Revision 3.139  2004/11/24 20:38:24  khf
 * added OBJDEF_NAME
 *
 * Revision 3.138  2004/11/24 17:28:01  sbs
 * WITH_CEXPR added
 *
 * Revision 3.137  2004/11/24 17:24:46  ktr
 * Added TCcountParts
 *
 * Revision 3.136  2004/11/24 16:48:09  skt
 * VARDEC_OR_ARG_NAME patched & some brushing
 *
 * Revision 3.135  2004/11/24 16:27:42  ktr
 * Changed signature of MakeAssignLet
 *
 * Revision 3.134  2004/11/24 14:12:35  sah
 * added ID_DECL
 *
 * Revision 3.133  2004/11/24 14:04:53  skt
 * ID_VARDEC added
 *
 * Revision 3.132  2004/11/24 13:51:35  mwe
 * DECL_AVIS added
 *
 * Revision 3.131  2004/11/24 13:17:16  skt
 * some old refcnt-macros deleted
 *
 * Revision 3.130  2004/11/24 12:41:05  khf
 * removed macros concerning CODE_CEXPR
 *
 * Revision 3.129  2004/11/24 12:33:33  ktr
 * TCappendRets added.
 *
 * Revision 3.128  2004/11/24 11:45:14  sah
 * added TCgetNthExpr
 *
 * Revision 3.127  2004/11/24 11:35:14  skt
 * N_WLseg -> N_wlseg
 *
 * Revision 3.126  2004/11/24 10:56:11  sah
 * *** empty log message ***
 *
 * Revision 3.125  2004/11/24 10:50:14  sbs
 * TCmakeIdsFromVardecs added
 *
 * Revision 3.124  2004/11/23 22:36:58  sbs
 * TCcountRets added.
 *
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

/* bitmasks for profiling, former defined in tree_basic.h */
#define CALL_FUN 0x0001
#define RETURN_FROM_FUN 0x0002
#define INL_FUN 0x0004
#define OVRLD_FUN 0x0008
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
extern types *TCgetTypesLine (types *type, int line);
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
 ***  N_ids :
 ***/

#define IDS_NAME(n) AVIS_NAME (IDS_AVIS (n))
#define IDS_VARNO(n) VARDEC_OR_ARG_VARNO (IDS_DECL (n))
#define IDS_DECL(n) AVIS_DECL (IDS_AVIS (n))
#define IDS_NTYPE(n) AVIS_TYPE (IDS_AVIS (n))
#define IDS_DIM(n) VARDEC_OR_ARG_DIM (IDS_DECL (n))

/*
 * TODO: remove
 */
#define IDS_TYPE(n) VARDEC_OR_ARG_TYPE (IDS_DECL (n))
#define IDS_SHPSEG(n) TYPES_SHPSEG (VARDEC_OR_ARG_TYPE (IDS_DECL (n)))
#define IDS_SHAPE(n, x) SHPSEG_SHAPE (IDS_SHPSEG (n), x)

extern node *TCappendIds (node *chain, node *item);
extern int TCcountIds (node *ids_arg);
extern node *TCmakeIdsFromVardecs (node *vardecs);

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

extern int TCcountNums (node *nums);
extern bool TCnumsContains (int val, node *nums);

/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
 ***/

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
/*--------------------------------------------------------------------------*/
/*  macros and functions for node structures                                */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/***
 ***  general :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  "N_decl" (N_vardec, N_arg, N_objdef)
 ***/

#define DECL_AVIS(n) ((NODE_TYPE (n) == N_arg) ? ARG_AVIS (n) : VARDEC_AVIS (n))
#define DECL_NAME(n) (AVIS_NAME (DECL_AVIS))

/*--------------------------------------------------------------------------*/

/***
 ***  N_module :
 ***/

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

extern node *TCsearchObjdef (const char *name, char *mod, node *implementations);

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

#define FUNDEF_VARDEC(n) (BLOCK_VARDEC (FUNDEF_BODY (n)))
#define FUNDEF_INSTR(n) (BLOCK_INSTR (FUNDEF_BODY (n)))

#define FUNDEF_ISLACFUN(n) (FUNDEF_ISCONDFUN (n) || FUNDEF_ISDOFUN (n))

/*
 * Ugly macros, don't use
 */
#define FUNDEF_BASETYPE(n) (TYPES_BASETYPE (FUNDEF_TYPES (n)))
#define FUNDEF_DIM(n) (TYPES_DIM (FUNDEF_TYPES (n)))
#define FUNDEF_SHAPE(n, x) (TYPES_SHAPE (FUNDEF_TYPES (n), x))
#define FUNDEF_SHPSEG(n) (TYPES_SHPSEG (FUNDEF_TYPES (n)))
#define FUNDEF_TNAME(n) (TYPES_NAME (FUNDEF_TYPES (n)))
#define FUNDEF_TMOD(n) (TYPES_MOD (FUNDEF_TYPES (n)))
#define FUNDEF_TDEF(n) (TYPES_TDEF (FUNDEF_TYPES (n)))

/*
 *  The following compound access macros are useful whenever a fundef
 *  node is used to represent a function declaration rather than a
 *  function definition.
 */

/*
 * Ugly macros, don't use
 */
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
 *  functionname  : TCsearchFundef
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

/*
 * TODO: REMOVE US CAUSE WE'RE UGLY
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

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***/

/*
 *  compound access macros
 */

#define ARG_NAME(n) (AVIS_NAME (ARG_AVIS (n)))
#define ARG_NTYPE(n) (AVIS_TYPE (ARG_AVIS (n)))

/*
 * TODO: REMOVE US CAUSE WE'RE UGLY
 */
#define ARG_BASETYPE(n) (TYPES_BASETYPE (ARG_TYPE (n)))
#define ARG_DIM(n) (TYPES_DIM (ARG_TYPE (n)))
#define ARG_SHAPE(n, x) (TYPES_SHAPE (ARG_TYPE (n), x))
#define ARG_SHPSEG(n) (TYPES_SHPSEG (ARG_TYPE (n)))
#define ARG_TNAME(n) (TYPES_NAME (ARG_TYPE (n)))
#define ARG_TMOD(n) (TYPES_MOD (ARG_TYPE (n)))

extern int TCcountArgs (node *args);

/*--------------------------------------------------------------------------*/

/***
 ***  N_ret :
 ***/

extern int TCcountRets (node *rets);
extern node *TCappendRet (node *chain, node *item);
extern node *TCreturnTypes2Ret (types *type);

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :  *and*  N_arg :
 ***/

/*
 * TODO: REMOVE US CAUSE WE'RE UGLY
 */

/*
 * CAUTION: Do not use the following macros as l-values!!!
 *          (this is *no* ANSI C style!)
 *          Use the L_VARDEC_OR_... macros instead!!
 */
#define VARDEC_OR_ARG_NAME(n) (AVIS_NAME (DECL_AVIS (n)))
#define VARDEC_OR_ARG_TYPE(n) ((NODE_TYPE (n) == N_arg) ? ARG_TYPE (n) : VARDEC_TYPE (n))
#define VARDEC_OR_ARG_STATUS(n)                                                          \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_STATUS (n)                                                                  \
       : ((NODE_TYPE (n) == N_vardec)                                                    \
            ? VARDEC_STATUS (n)                                                          \
            : DBUG_ASSERT_EXPR (0, "VARDEC_OR_ARG_STATUS on objdef", 0)))
#define VARDEC_OR_ARG_AVIS(n) ((NODE_TYPE (n) == N_arg) ? ARG_AVIS (n) : VARDEC_AVIS (n))
#define VARDEC_OR_ARG_VARNO(n)                                                           \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_VARNO (n)                                                                   \
       : ((NODE_TYPE (n) == N_vardec)                                                    \
            ? VARDEC_VARNO (n)                                                           \
            : DBUG_ASSERT_EXPR (0, "VARDEC_OR_ARG_VARNO on objdef", 0)))
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

#define VARDEC_OR_ARG_DIM(n) ((NODE_TYPE (n) == N_arg) ? ARG_DIM (n) : VARDEC_DIM (n))

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

extern node *TCsearchDecl (const char *name, node *decl_node);

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
 *   node *TCmakeAssignLet( node *avis, node *let_expr);
 *
 * arguments: 1) avis for the LHS variable
 *            2) let-expression
 *            R) assign-node with complete let-subtree
 *
 * description:
 *   returns a assign-node with let-node: var_name = expr;
 *
 * remarks:
 *   ASSIGN_NEXT is set to NULL
 *
 ******************************************************************************/

extern node *TCmakeAssignLet (node *avis, node *let_expr);

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

extern node *TCgetNthExpr (int n, node *exprs);

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

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :  *and*  N_while :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :    (see also N_exprs, Shpseg !!!)
 ***/

/*
 *  compound access macros
 */

#define ARRAY_DIM(n) (SHgetDim (ARRAY_SHAPE (n)))

/*
 *  function declarations
 */
extern node *TCmakeFlatArray (node *aelems);
extern node *TCcreateZeroScalar (simpletype btype);
extern node *TCcreateZeroVector (int length, simpletype btype);

/******************************************************************************
 *
 * Function:
 *   bool TCisConstArray( node *array);
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

extern bool TCisConstArray (node *array);

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

#define ID_DECL(n) (AVIS_DECL (ID_AVIS (n)))
#define ID_NAME(n) AVIS_NAME (ID_AVIS (n))
#define ID_NTYPE(n) AVIS_TYPE (ID_AVIS (n))
#define ID_DIM(n) VARDEC_OR_ARG_DIM (ID_DECL (n))
#define ID_DECL_NEXT(n) VARDEC_OR_ARG_NEXT (ID_DECL (n))
#define ID_PADDED(n) VARDEC_OR_ARG_PADDED (ID_DECL (n))

#define ID_TYPE(n)                                                                       \
    ((NODE_TYPE (AVIS_DECL (ID_AVIS (n))) == N_vardec)                                   \
       ? VARDEC_TYPE (AVIS_DECL (ID_AVIS (n)))                                           \
       : ((NODE_TYPE (AVIS_DECL (ID_AVIS (n))) == N_arg)                                 \
            ? ARG_TYPE (AVIS_DECL (ID_AVIS (n)))                                         \
            : NULL))

#define ID_SHPSEG(n) TYPES_SHPSEG (ID_TYPE (n))
#define ID_SHAPE(n, x) SHPSEG_SHAPE (ID_SHPSEG (n), x)

#define ID_SSAASSIGN(n) (AVIS_SSAASSIGN (ID_AVIS (n)))

#define ID_NAME_OR_ICMTEXT(n) ((ID_AVIS (n) != NULL) ? ID_NAME (n) : ID_ICMTEXT (n))

extern node *TCmakeIdCopyString (const char *str);
extern node *TCmakeIdCopyStringNt (const char *str, types *type);

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

extern bool TCisPhiFun (node *id);

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
 ***  N_spap :
 ***
 ***  watch combined macros for N_ap and N_prf
 ***  (search for "N_ap :" or "N_prf :").
 ***/

/*
 *  compound access macros
 */

#define AP_NAME(n) FUNDEF_NAME (AP_FUNDEF (n))
#define AP_MOD(n) FUNDEF_MOD (AP_FUNDEF (n))

#define SPAP_NAME(n) SPID_NAME (SPAP_ID (n))
#define SPAP_MOD(n) SPID_MOD (SPAP_ID (n))

#define AP_EXPRS1(n) AP_ARGS (n)
#define AP_EXPRS2(n) EXPRS_EXPRS2 (AP_ARGS (n))
#define AP_EXPRS3(n) EXPRS_EXPRS3 (AP_ARGS (n))

#define SPAP_EXPRS1(n) SPAP_ARGS (n)
#define SPAP_EXPRS2(n) EXPRS_EXPRS2 (SPAP_ARGS (n))
#define SPAP_EXPRS3(n) EXPRS_EXPRS3 (SPAP_ARGS (n))

#define AP_ARG1(n) EXPRS_EXPR (AP_EXPRS1 (n))
#define AP_ARG2(n) EXPRS_EXPR (AP_EXPRS2 (n))
#define AP_ARG3(n) EXPRS_EXPR (AP_EXPRS3 (n))

#define SPAP_ARG1(n) EXPRS_EXPR (SPAP_EXPRS1 (n))
#define SPAP_ARG2(n) EXPRS_EXPR (SPAP_EXPRS2 (n))
#define SPAP_ARG3(n) EXPRS_EXPR (SPAP_EXPRS3 (n))

/*
 *  function declarations
 */

extern node *TCmakeAp1 (node *fundef, node *arg1);

extern node *TCmakeAp2 (node *fundef, node *arg1, node *arg2);

extern node *TCmakeAp3 (node *fundef, node *arg1, node *arg2, node *arg3);

extern node *TCmakeSpap1 (char *mod, char *name, node *arg1);

extern node *TCmakeSpap2 (char *mod, char *name, node *arg1, node *arg2);

extern node *TCmakeSpap3 (char *mod, char *name, node *arg1, node *arg2, node *arg3);

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

/*--------------------------------------------------------------------------*/

/***
 ***  N_pragma :
 ***/

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

#define WITH_TYPE(n) (NODE_TYPE (WITH_WITHOP (n)))

/*
 * We only need to inspect the withid of the first part,
 * because the withid is in *all* parts the same!!
 */
#define WITH_WITHID(n) (PART_WITHID (WITH_PART (n)))
#define WITH_IDS(n) (PART_IDS (WITH_PART (n)))
#define WITH_VEC(n) (PART_VEC (WITH_PART (n)))

/*
 * BOUND1, BOUND2, STEP, WIDTH of the *first* N_part-node
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
#define WITH_CEXPRS(n) (CODE_CEXPRS (WITH_CODE (n)))
#define WITH_CEXPR(n) (EXPRS_EXPR (WITH_CEXPRS (n)))

extern node *TCcreateScalarWith (shape *shape, simpletype btype, node *expr,
                                 node *fundef);

extern node *TCcreateZero (shape *shape, simpletype btype, bool unroll, node *fundef);

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

#define PART_CEXPRS(n) (CODE_CEXPRS (PART_CODE (n)))
#define PART_CBLOCK(n) (CODE_CBLOCK (PART_CODE (n)))

extern int TCcountParts (node *parts);

/*--------------------------------------------------------------------------*/

/***
 ***  N_code :
 ***/

#define CODE_CBLOCK_INSTR(n) (BLOCK_INSTR (CODE_CBLOCK (n)))
#define CODE_CEXPR(n) (EXPRS_EXPR1 (CODE_CEXPRS (n)))

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

#define WITH2_TYPE(n) (NODE_TYPE (WITH2_WITHOP (n)))

#define WITH2_IDS(n) (WITHID_IDS (WITH2_WITHID (n)))
#define WITH2_VEC(n) (WITHID_VEC (WITH2_WITHID (n)))

/*
 * CBLOCK, CEXPR of the *first* N_code-node
 * (useful in case of single-generator with-loops only,
 *  e.g. before with-loop-folding)
 */
#define WITH2_CBLOCK(n) (CODE_CBLOCK (WITH2_CODE (n)))
#define WITH2_CEXPRS(n) (CODE_CEXPRS (WITH2_CODE (n)))

#define WITH2_DEFAULT(n) (GENARRAY_DEFAULT (WITH2_WITHOP (n)))
#define WITH2_SHAPE(n) (GENARRAY_SHAPE (WITH2_WITHOP (n)))
#define WITH2_ARRAY(n) (MODARRAY_ARRAY (WITH2_WITHOP (n)))
#define WITH2_NEUTRAL(n) (FOLD_NEUTRAL (WITH2_WITHOP (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_with :  *and*  N_with2 :
 ***/

#define WITH_OR_WITH2_WITHOP(n)                                                          \
    ((NODE_TYPE (n) == N_with) ? WITH_WITHOP (n) : WITH2_WITHOP (n))

#define WITH_OR_WITH2_TYPE(n) ((NODE_TYPE (n) == N_with) ? WITH_TYPE (n) : WITH2_TYPE (n))
#define WITH_OR_WITH2_IDS(n) ((NODE_TYPE (n) == N_with) ? WITH_IDS (n) : WITH2_IDS (n))
#define WITH_OR_WITH2_VEC(n) ((NODE_TYPE (n) == N_with) ? WITH_VEC (n) : WITH2_VEC (n))

#define WITH_OR_WITH2_CEXPR(n)                                                           \
    ((NODE_TYPE (n) == N_with) ? WITH_CEXPR (n) : WITH2_CEXPR (n))

#define WITH_OR_WITH2_IN_MASK(n)                                                         \
    ((NODE_TYPE (n) == N_with) ? WITH_IN_MASK (n) : WITH2_IN_MASK (n))
#define WITH_OR_WITH2_OUT_MASK(n)                                                        \
    ((NODE_TYPE (n) == N_with) ? WITH_OUT_MASK (n) : WITH2_OUT_MASK (n))
#define WITH_OR_WITH2_LOCAL_MASK(n)                                                      \
    ((NODE_TYPE (n) == N_with) ? WITH_LOCAL_MASK (n) : WITH2_LOCAL_MASK (n))

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
 ***  N_wlseg :
 ***/

#define WLSEG_IDX_PRINT(handle, n, field)                                                \
    PRINT_VECT (handle, ((int *)(WLSEG_##field (n))), WLSEG_DIMS (n), "%i");

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlsegVar :
 ***/

#define WLSEGVAR_IDX_PRINT(handle, n, field)                                             \
    {                                                                                    \
        int d;                                                                           \
        node **vect = WLSEGVAR_##field (n);                                              \
        if (vect != NULL) {                                                              \
            fprintf (handle, "[ ");                                                      \
            for (d = 0; d < WLSEGVAR_DIMS (n); d++) {                                    \
                WLBnodeOrIntPrint (handle, N_wlsegvar, &(vect[d]), d);                   \
                fprintf (handle, " ");                                                   \
            }                                                                            \
            fprintf (handle, "]");                                                       \
        } else {                                                                         \
            fprintf (handle, "NULL");                                                    \
        }                                                                                \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlseg :  *and*  N_wlsegVar :
 ***/

#define WLSEGX_IDX_MIN(n)                                                                \
    ((NODE_TYPE (n) == N_wlseg) ? WLSEG_IDX_MIN (n) : WLSEGVAR_IDX_MIN (n))

#define WLSEGX_IDX_MAX(n)                                                                \
    ((NODE_TYPE (n) == N_wlseg) ? WLSEG_IDX_MAX (n) : WLSEGVAR_IDX_MAX (n))

#define WLSEGX_DIMS(n) ((NODE_TYPE (n) == N_wlseg) ? WLSEG_DIMS (n) : WLSEGVAR_DIMS (n))

#define WLSEGX_CONTENTS(n)                                                               \
    ((NODE_TYPE (n) == N_wlseg) ? WLSEG_CONTENTS (n) : WLSEGVAR_CONTENTS (n))

#define WLSEGX_NEXT(n) ((NODE_TYPE (n) == N_wlseg) ? WLSEG_NEXT (n) : WLSEGVAR_NEXT (n))

#define WLSEGX_SCHEDULING(n)                                                             \
    ((NODE_TYPE (n) == N_wlseg) ? WLSEG_SCHEDULING (n) : WLSEGVAR_SCHEDULING (n))

#define WLSEGX_TASKSEL(n)                                                                \
    ((NODE_TYPE (n) == N_wlseg) ? WLSEG_TASKSEL (n) : WLSEGVAR_TASKSEL (n))

#define WLSEGX_IDX_GET_ADDR(n, field, dim)                                               \
    ((NODE_TYPE (n) == N_wlseg) ? (void *)&(((int *)(WLSEG_##field (n)))[dim])           \
                                : ((NODE_TYPE (n) == N_wlsegvar)                         \
                                     ? (void *)&(((node **)(WLSEGVAR_##field (n)))[dim]) \
                                     : NULL))

#define WLSEGX_IDX_PRINT(handle, n, field)                                               \
    if (NODE_TYPE (n) == N_wlseg) {                                                      \
        WLSEG_IDX_PRINT (handle, n, field);                                              \
    } else {                                                                             \
        WLSEGVAR_IDX_PRINT (handle, n, field);                                           \
    }

#define L_WLSEGX_NEXT(n, rhs)                                                            \
    if (NODE_TYPE (n) == N_wlseg) {                                                      \
        WLSEG_NEXT (n) = (rhs);                                                          \
    } else {                                                                             \
        WLSEGVAR_NEXT (n) = (rhs);                                                       \
    }

#define L_WLSEGX_SCHEDULING(n, rhs)                                                      \
    if (NODE_TYPE (n) == N_wlseg) {                                                      \
        WLSEG_SCHEDULING (n) = (rhs);                                                    \
    } else {                                                                             \
        WLSEGVAR_SCHEDULING (n) = (rhs);                                                 \
    }

#define L_WLSEGX_CONTENTS(n, rhs)                                                        \
    if (NODE_TYPE (n) == N_wlseg) {                                                      \
        WLSEG_CONTENTS (n) = (rhs);                                                      \
    } else {                                                                             \
        WLSEGVAR_CONTENTS (n) = (rhs);                                                   \
    }

#define L_WLSEGX_TASKSEL(n, rhs)                                                         \
    if (NODE_TYPE (n) == N_wlseg) {                                                      \
        WLSEG_TASKSEL (n) = (rhs);                                                       \
    } else {                                                                             \
        WLSEGVAR_TASKSEL (n) = (rhs);                                                    \
    }

extern node *TCmakeWlSegX (int dims, node *contents, node *next);

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlblock :
 ***/

#define WLBLOCK_ISNOOP(n)                                                                \
    ((WLBLOCK_NEXTDIM (n) == NULL) && (WLBLOCK_CONTENTS (n) == NULL))

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlublock :
 ***/

#define WLUBLOCK_ISNOOP(n)                                                               \
    ((WLUBLOCK_NEXTDIM (n) == NULL) && (WLUBLOCK_CONTENTS (n) == NULL))

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlblock :  *and*  N_wlublock :
 ***/

#define WLXBLOCK_ISNOOP(n)                                                               \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_ISNOOP (n) : WLUBLOCK_ISNOOP (n))

#define WLXBLOCK_LEVEL(n)                                                                \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_LEVEL (n) : WLUBLOCK_LEVEL (n))

#define WLXBLOCK_DIM(n)                                                                  \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_DIM (n) : WLUBLOCK_DIM (n))

#define WLXBLOCK_BOUND1(n)                                                               \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_BOUND1 (n) : WLUBLOCK_BOUND1 (n))

#define WLXBLOCK_BOUND2(n)                                                               \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_BOUND2 (n) : WLUBLOCK_BOUND2 (n))

#define WLXBLOCK_STEP(n)                                                                 \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_STEP (n) : WLUBLOCK_STEP (n))

#define WLXBLOCK_NEXTDIM(n)                                                              \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_NEXTDIM (n) : WLUBLOCK_NEXTDIM (n))

#define WLXBLOCK_CONTENTS(n)                                                             \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_CONTENTS (n) : WLUBLOCK_CONTENTS (n))

#define WLXBLOCK_NEXT(n)                                                                 \
    ((NODE_TYPE (n) == N_wlblock) ? WLBLOCK_NEXT (n) : WLUBLOCK_NEXT (n))

#define L_WLXBLOCK_CONTENTS(n, rhs)                                                      \
    if (NODE_TYPE (n) == N_wlblock) {                                                    \
        WLBLOCK_CONTENTS (n) = (rhs);                                                    \
    } else {                                                                             \
        WLUBLOCK_CONTENTS (n) = (rhs);                                                   \
    }

#define L_WLXBLOCK_NEXTDIM(n, rhs)                                                       \
    if (NODE_TYPE (n) == N_wlblock) {                                                    \
        WLBLOCK_NEXTDIM (n) = (rhs);                                                     \
    } else {                                                                             \
        WLUBLOCK_NEXTDIM (n) = (rhs);                                                    \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlstride :
 ***/

#define WLSTRIDE_ISNOOP(n) (WLSTRIDE_CONTENTS (n) == NULL)

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlstridevar :
 ***/

#define WLSTRIDEVAR_ISNOOP(n) (WLSTRIDEVAR_CONTENTS (n) == NULL)

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlstride :  *and*  N_wlstridevar :
 ***/

#define WLSTRIDEX_GET_ADDR(n, field)                                                     \
    ((NODE_TYPE (n) == N_wlstride)                                                       \
       ? (void *)&(WLSTRIDE_##field (n))                                                 \
       : ((NODE_TYPE (n) == N_wlstridevar) ? (void *)&(WLSTRIDEVAR_##field (n)) : NULL))

#define WLSTRIDEX_ISNOOP(n)                                                              \
    ((NODE_TYPE (n) == N_wlstride) ? WLSTRIDE_ISNOOP (n) : WLSTRIDEVAR_ISNOOP (n))

#define WLSTRIDEX_LEVEL(n)                                                               \
    ((NODE_TYPE (n) == N_wlstride) ? WLSTRIDE_LEVEL (n) : WLSTRIDEVAR_LEVEL (n))

#define WLSTRIDEX_DIM(n)                                                                 \
    ((NODE_TYPE (n) == N_wlstride) ? WLSTRIDE_DIM (n) : WLSTRIDEVAR_DIM (n))

#define WLSTRIDEX_CONTENTS(n)                                                            \
    ((NODE_TYPE (n) == N_wlstride) ? WLSTRIDE_CONTENTS (n) : WLSTRIDEVAR_CONTENTS (n))

#define WLSTRIDEX_NEXT(n)                                                                \
    ((NODE_TYPE (n) == N_wlstride) ? WLSTRIDE_NEXT (n) : WLSTRIDEVAR_NEXT (n))

#define L_WLSTRIDEX_CONTENTS(n, rhs)                                                     \
    if (NODE_TYPE (n) == N_wlstride) {                                                   \
        WLSTRIDE_CONTENTS (n) = (rhs);                                                   \
    } else {                                                                             \
        WLSTRIDEVAR_CONTENTS (n) = (rhs);                                                \
    }

#define L_WLSTRIDEX_NEXT(n, rhs)                                                         \
    if (NODE_TYPE (n) == N_wlstride) {                                                   \
        WLSTRIDE_NEXT (n) = (rhs);                                                       \
    } else {                                                                             \
        WLSTRIDEVAR_NEXT (n) = (rhs);                                                    \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlgrid :
 ***/

#define WLGRID_CBLOCK(n) (CODE_CBLOCK (WLGRID_CODE (n)))
#define WLGRID_CEXPR(n) (CODE_CEXPR (WLGRID_CODE (n)))

#define WLGRID_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRID_CBLOCK (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlgridvar :
 ***/

#define WLGRIDVAR_CBLOCK(n) (CODE_CBLOCK (WLGRIDVAR_CODE (n)))
#define WLGRIDVAR_CEXPR(n) (CODE_CEXPR (WLGRIDVAR_CODE (n)))

#define WLGRIDVAR_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRIDVAR_CBLOCK (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlgrid :  *and*  N_wlgridvar :
 ***/

#define WLGRIDX_LEVEL(n)                                                                 \
    ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_LEVEL (n) : WLGRIDVAR_LEVEL (n))

#define WLGRIDX_DIM(n) ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_DIM (n) : WLGRIDVAR_DIM (n))

#define WLGRIDX_ISFITTED(n)                                                              \
    ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_ISFITTED (n) : WLGRIDVAR_ISFITTED (n))

#define WLGRIDX_NEXTDIM(n)                                                               \
    ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_NEXTDIM (n) : WLGRIDVAR_NEXTDIM (n))

#define WLGRIDX_NEXT(n)                                                                  \
    ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_NEXT (n) : WLGRIDVAR_NEXT (n))

#define WLGRIDX_CODE(n)                                                                  \
    ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_CODE (n) : WLGRIDVAR_CODE (n))

#define WLGRIDX_ISNOOP(n)                                                                \
    ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_ISNOOP (n) : WLGRIDVAR_ISNOOP (n))

#define WLGRIDX_CBLOCK(n) (CODE_CBLOCK (WLGRIDX_CODE (n)))
#define WLGRIDX_CEXPR(n) (CODE_CEXPR (WLGRIDX_CODE (n)))

#define WLGRIDX_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRIDX_CBLOCK (n)))

#define WLGRIDX_GET_ADDR(n, field)                                                       \
    ((NODE_TYPE (n) == N_wlgrid)                                                         \
       ? (void *)&(WLGRID_##field (n))                                                   \
       : ((NODE_TYPE (n) == N_wlgridvar) ? (void *)&(WLGRIDVAR_##field (n)) : NULL))

#define L_WLGRIDX_ISNOOP(n, rhs)                                                         \
    if (NODE_TYPE (n) == N_wlgrid) {                                                     \
        WLGRID_ISNOOP (n) = (rhs);                                                       \
    } else {                                                                             \
        WLGRIDVAR_ISNOOP (n) = (rhs);                                                    \
    }

#define L_WLGRIDX_NEXTDIM(n, rhs)                                                        \
    if (NODE_TYPE (n) == N_wlgrid) {                                                     \
        WLGRID_NEXTDIM (n) = (rhs);                                                      \
    } else {                                                                             \
        WLGRIDVAR_NEXTDIM (n) = (rhs);                                                   \
    }

#define L_WLGRIDX_CODE(n, rhs)                                                           \
    if (NODE_TYPE (n) == N_wlgrid) {                                                     \
        WLGRID_CODE (n) = (rhs);                                                         \
    } else {                                                                             \
        WLGRIDVAR_CODE (n) = (rhs);                                                      \
    }

#define L_WLGRIDX_ISFITTED(n, rhs)                                                       \
    if (NODE_TYPE (n) == N_wlgrid) {                                                     \
        WLGRID_ISFITTED (n) = (rhs);                                                     \
    } else {                                                                             \
        WLGRIDVAR_ISFITTED (n) = (rhs);                                                  \
    }

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlblock :   *and*  N_wlublock :     *and*
 ***  N_wlstride :  *and*  N_wlstridevar :
 ***/

#define WLBLOCKSTR_GET_ADDR(n, field)                                                    \
    ((NODE_TYPE (n) == N_wlstride)                                                       \
       ? (void *)&(WLSTRIDE_##field (n))                                                 \
       : ((NODE_TYPE (n) == N_wlstridevar)                                               \
            ? (void *)&(WLSTRIDEVAR_##field (n))                                         \
            : ((NODE_TYPE (n) == N_wlblock)                                              \
                 ? (void *)&(WLBLOCK_##field (n))                                        \
                 : ((NODE_TYPE (n) == N_wlublock) ? (void *)&(WLUBLOCK_##field (n))      \
                                                  : NULL))))

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlblock :   *and*  N_wlublock :     *and*
 ***  N_wlstride :  *and*  N_wlstridevar :  *and*
 ***  N_wlgrid :    *and*  N_wlgridvar :
 ***/

#define WLNODE_GET_ADDR(n, field)                                                        \
    ((NODE_TYPE (n) == N_wlstride)                                                       \
       ? (void *)&(WLSTRIDE_##field (n))                                                 \
       : ((NODE_TYPE (n) == N_wlstridevar)                                               \
            ? (void *)&(WLSTRIDEVAR_##field (n))                                         \
            : ((NODE_TYPE (n) == N_wlgrid)                                               \
                 ? (void *)&(WLGRID_##field (n))                                         \
                 : ((NODE_TYPE (n) == N_wlgridvar)                                       \
                      ? (void *)&(WLGRIDVAR_##field (n))                                 \
                      : ((NODE_TYPE (n) == N_wlblock)                                    \
                           ? (void *)&(WLBLOCK_##field (n))                              \
                           : ((NODE_TYPE (n) == N_wlublock)                              \
                                ? (void *)&(WLUBLOCK_##field (n))                        \
                                : NULL))))))

#define WLNODE_ISNOOP(n)                                                                 \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_ISNOOP (n)                                                              \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_ISNOOP (n)                                                        \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_ISNOOP (n)                                                   \
                 : ((NODE_TYPE (n) == N_wlstridevar)                                     \
                      ? WLSTRIDEVAR_ISNOOP (n)                                           \
                      : ((NODE_TYPE (n) == N_wlgrid)                                     \
                           ? WLGRID_ISNOOP (n)                                           \
                           : ((NODE_TYPE (n) == N_wlgridvar) ? WLGRIDVAR_ISNOOP (n)      \
                                                             : FALSE))))))

#define WLNODE_NEXT(n)                                                                   \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_NEXT (n)                                                                \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_NEXT (n)                                                          \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_NEXT (n)                                                     \
                 : ((NODE_TYPE (n) == N_wlstridevar)                                     \
                      ? WLSTRIDEVAR_NEXT (n)                                             \
                      : ((NODE_TYPE (n) == N_wlgrid)                                     \
                           ? WLGRID_NEXT (n)                                             \
                           : ((NODE_TYPE (n) == N_wlgridvar) ? WLGRIDVAR_NEXT (n)        \
                                                             : NULL))))))

#define WLNODE_LEVEL(n)                                                                  \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_LEVEL (n)                                                               \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_LEVEL (n)                                                         \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_LEVEL (n)                                                    \
                 : ((NODE_TYPE (n) == N_wlstridevar)                                     \
                      ? WLSTRIDEVAR_LEVEL (n)                                            \
                      : ((NODE_TYPE (n) == N_wlgrid)                                     \
                           ? WLGRID_LEVEL (n)                                            \
                           : ((NODE_TYPE (n) == N_wlgridvar) ? WLGRIDVAR_LEVEL (n)       \
                                                             : 0))))))

#define WLNODE_DIM(n)                                                                    \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_DIM (n)                                                                 \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_DIM (n)                                                           \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_DIM (n)                                                      \
                 : ((NODE_TYPE (n) == N_wlstridevar)                                     \
                      ? WLSTRIDEVAR_DIM (n)                                              \
                      : ((NODE_TYPE (n) == N_wlgrid)                                     \
                           ? WLGRID_DIM (n)                                              \
                           : ((NODE_TYPE (n) == N_wlgridvar) ? WLGRIDVAR_DIM (n)         \
                                                             : 0))))))

#define WLNODE_BOUND1_INT(n)                                                             \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_BOUND1 (n)                                                              \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_BOUND1 (n)                                                        \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_BOUND1 (n)                                                   \
                 : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_BOUND1 (n) : 0))))

#define WLNODE_BOUND1_NODE(n)                                                            \
    ((NODE_TYPE (n) == N_wlstridevar)                                                    \
       ? WLSTRIDEVAR_BOUND1 (n)                                                          \
       : ((NODE_TYPE (n) == N_wlgridvar) ? WLGRIDVAR_BOUND1 (n) : NULL))

#define WLNODE_BOUND2_INT(n)                                                             \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_BOUND2 (n)                                                              \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_BOUND2 (n)                                                        \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_BOUND2 (n)                                                   \
                 : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_BOUND2 (n) : 0))))

#define WLNODE_BOUND2_NODE(n)                                                            \
    ((NODE_TYPE (n) == N_wlstridevar)                                                    \
       ? WLSTRIDEVAR_BOUND2 (n)                                                          \
       : ((NODE_TYPE (n) == N_wlgridvar) ? WLGRIDVAR_BOUND2 (n) : NULL))

#define WLNODE_STEP_INT(n)                                                               \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_STEP (n)                                                                \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_STEP (n)                                                          \
            : ((NODE_TYPE (n) == N_wlstride) ? WLSTRIDE_STEP (n) : 0)))

#define WLNODE_NEXTDIM(n)                                                                \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_NEXTDIM (n)                                                             \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_NEXTDIM (n)                                                       \
            : ((NODE_TYPE (n) == N_wlgrid)                                               \
                 ? WLGRID_NEXTDIM (n)                                                    \
                 : ((NODE_TYPE (n) == N_wlgridvar) ? WLGRIDVAR_NEXTDIM (n) : NULL))))

#define L_WLNODE_NEXT(n, rhs)                                                            \
    if (NODE_TYPE (n) == N_wlblock)                                                      \
        WLBLOCK_NEXT (n) = rhs;                                                          \
    else if (NODE_TYPE (n) == N_wlublock)                                                \
        WLUBLOCK_NEXT (n) = rhs;                                                         \
    else if (NODE_TYPE (n) == N_wlstride)                                                \
        WLSTRIDE_NEXT (n) = rhs;                                                         \
    else if (NODE_TYPE (n) == N_wlstridevar)                                             \
        WLSTRIDEVAR_NEXT (n) = rhs;                                                      \
    else if (NODE_TYPE (n) == N_wlgrid)                                                  \
        WLGRID_NEXT (n) = rhs;                                                           \
    else if (NODE_TYPE (n) == N_wlgridvar)                                               \
        WLGRIDVAR_NEXT (n) = rhs;                                                        \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_NEXT called on wrong node type");

#define L_WLNODE_BOUND1_INT(n, rhs)                                                      \
    if (NODE_TYPE (n) == N_wlblock)                                                      \
        WLBLOCK_BOUND1 (n) = rhs;                                                        \
    else if (NODE_TYPE (n) == N_wlublock)                                                \
        WLUBLOCK_BOUND1 (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_wlstride)                                                \
        WLSTRIDE_BOUND1 (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_wlgrid)                                                  \
        WLGRID_BOUND1 (n) = rhs;                                                         \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_BOUND1_INT called on wrong node type");

#define L_WLNODE_BOUND2_INT(n, rhs)                                                      \
    if (NODE_TYPE (n) == N_wlblock)                                                      \
        WLBLOCK_BOUND2 (n) = rhs;                                                        \
    else if (NODE_TYPE (n) == N_wlublock)                                                \
        WLUBLOCK_BOUND2 (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_wlstride)                                                \
        WLSTRIDE_BOUND2 (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_wlgrid)                                                  \
        WLGRID_BOUND2 (n) = rhs;                                                         \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_BOUND2_INT called on wrong node type");

#define L_WLNODE_STEP_INT(n, rhs)                                                        \
    if (NODE_TYPE (n) == N_wlblock)                                                      \
        WLBLOCK_STEP (n) = rhs;                                                          \
    else if (NODE_TYPE (n) == N_wlublock)                                                \
        WLUBLOCK_STEP (n) = rhs;                                                         \
    else if (NODE_TYPE (n) == N_wlstride)                                                \
        WLSTRIDE_STEP (n) = rhs;                                                         \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_STEP called on wrong node type");

#define L_WLNODE_NEXTDIM(n, rhs)                                                         \
    if (NODE_TYPE (n) == N_wlblock)                                                      \
        WLBLOCK_NEXTDIM (n) = rhs;                                                       \
    else if (NODE_TYPE (n) == N_wlublock)                                                \
        WLUBLOCK_NEXTDIM (n) = rhs;                                                      \
    else if (NODE_TYPE (n) == N_wlgrid)                                                  \
        WLGRID_NEXTDIM (n) = rhs;                                                        \
    else if (NODE_TYPE (n) == N_wlgridvar)                                               \
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

extern node *TCmakeStrCopy (const char *str);

/*--------------------------------------------------------------------------*/

/***
 ***  N_linklist
 ***/

extern int TCaddLinkToLinks (node **links, node *link);
extern int TCaddLinksToLinks (node **links, node *add);
extern bool TClinklistContains (node *set, node *link);
extern bool TClinklistIsSubset (node *super, node *sub);

#endif /* _SAC_TREE_COMPOUND_H_ */
