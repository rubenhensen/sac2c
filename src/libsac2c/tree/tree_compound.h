/*
 * $Id$
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
#define LIB_FUN 0x0008
#define OVRLD_FUN 0x0010

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***/

extern int TCgetShpsegLength (int dims, shpseg *shape);
extern shpseg *TCdiffShpseg (int dim, shpseg *shape1, shpseg *shape2);
extern bool TCshapeVarsMatch (node *avis1, node *avis2);
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
extern shpseg *TCtype2Shpseg (types *type, int *ret_dim);
extern shape *TCtype2Shape (types *type);
extern node *TCtype2Exprs (types *type);

extern bool TCisUnique (types *type);
extern bool TCisHidden (types *type);

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

extern node *TCcreateIdsChainFromAvises (int num_avises, ...);
extern node *TCappendIds (node *chain, node *item);
extern int TCcountIds (node *ids_arg);
extern node *TCmakeIdsFromVardecs (node *vardecs);
extern node *TCsetSSAAssignForIdsChain (node *ids, node *assign);

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
 ***  "N_decl" (N_vardec, N_arg, N_argtemplate)
 ***/

#define DECL_AVIS(n)                                                                     \
    ((NODE_TYPE (n) == N_arg)                                                            \
       ? ARG_AVIS (n)                                                                    \
       : ((NODE_TYPE (n) == N_vardec) ? VARDEC_AVIS (n) : (node *)0x0))
#define DECL_NAME(n) (AVIS_NAME (DECL_AVIS (n)))

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
 *                  2) namespace of type to be searched for
 *                  3) list of type implementations (typedef nodes)
 *  description   : looks for a certain typedef in list of typedefs
 *
 */

extern node *TCsearchTypedef (const char *name, const namespace_t *ns,
                              node *implementations);

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

extern node *TCunAliasObjdef (node *objdef);

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
#define VARDEC_DIM(n) (TYPES_DIM (VARDEC_TYPE (n)))
#define VARDEC_SHAPE(n, x) (TYPES_SHAPE (VARDEC_TYPE (n), x))
#define VARDEC_SHPSEG(n) (TYPES_SHPSEG (VARDEC_TYPE (n)))

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

extern int TCcountVardecs (node *vardecs);

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
#define ARG_DIM(n) (TYPES_DIM (ARG_TYPE (n)))
#define ARG_TNAME(n) (TYPES_NAME (ARG_TYPE (n)))

extern int TCcountArgs (node *args);
extern int TCcountArgsIgnoreArtificials (node *args);
extern node *TCappendArgs (node *arg_chain, node *arg);

/*--------------------------------------------------------------------------*/

/***
 ***  N_argtemplate :
 ***/

/*
 *  compound access macros
 */

#define ARGTEMPLATE_NAME(n) (AVIS_NAME (ARGTEMPLATE_AVIS (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_ret :
 ***/

extern int TCcountRets (node *rets);
extern int TCcountRetsIgnoreArtificials (node *rets);
extern node *TCappendRet (node *chain, node *item);
extern node *TCcreateIdsFromRets (node *rets, node **vardecs);
extern node *TCcreateExprsFromArgs (node *args);

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
 * Function:
 *   node *TCmakeAssigns?( node *part?, ...)
 *
 * Description:
 *   * DEPRECATED * USED ONLY IN compile.c
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

extern int TCcountAssigns (node *assigns);
extern node *TCgetLastAssign (node *arg_node);

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

extern node *TCcreateExprsFromVardecs (node *vardecs);

extern node *TCcreateExprsChainFromAvises (int num_avises, ...);

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

extern node *TCgetNthExprs (int n, node *exprs);
extern node *TCgetNthExprsExpr (int n, node *exprs);
extern node *TCtakeDropExprs (int takecount, int dropcount, node *exprs);

extern node *TCcreateExprsFromIds (node *ids);

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

/******************************************************************************
 *
 * function:
 *   node *TCfilterExprs( bool (*pred)( node *), node **exprs)
 *
 * description:
 *   Filters the given exprs chain
 *
 *****************************************************************************/
extern node *TCfilterExprs (bool (*pred) (node *), node **exprs);

/******************************************************************************
 *
 * function:
 *   bool TCfoldPredExprs( bool (*pred)( node *), node *exprs)
 *
 * description:
 *   Evaluates whether a given predicate holds for all elements of an exprs
 *   chain
 *
 *****************************************************************************/
extern bool TCfoldPredExprs (bool (*pred) (node *), node *exprs);

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :
 ***/

/*
 *  compound access macros
 */

#define LET_NAME(n) (IDS_NAME (LET_IDS (n)))
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

#define ARRAY_FRAMEDIM(n) (SHgetDim (ARRAY_FRAMESHAPE (n)))

/*
 *  function declarations
 */
extern node *TCmakeVector (ntype *basetype, node *aelems);
extern node *TCmakeIntVector (node *aelems);
extern node *TCcreateIntVector (int length, int value);
extern int TCgetIntVectorNthValue (int pos, node *vect);

extern node *TCcreateZeroScalar (simpletype btype);
extern node *TCcreateZeroVector (int length, simpletype btype);

extern node *TCids2Exprs (node *ids_arg);
extern node *TCids2ExprsNt (node *ids_arg);

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

#define ID_SSAASSIGN(n) (AVIS_SSAASSIGN (ID_AVIS (n)))

#define ID_NAME_OR_ICMTEXT(n) ((ID_AVIS (n) != NULL) ? ID_NAME (n) : ID_ICMTEXT (n))

extern node *TCmakeIdCopyString (const char *str);
extern node *TCmakeIdCopyStringNt (const char *str, types *type);
extern node *TCmakeIdCopyStringNtNew (const char *str, ntype *type);

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

/* PRF ARGUMENT ENCODING */
#define PRF_ARGENCODING(prf, arg) global.prf_arg_encoding[3 * prf + arg]

/* PRF NAME from globals */
#define PRF_NAME(prf) global.prf_name[prf]

/*
 *  function declarations
 */

extern node *TCmakePrf1 (prf prf, node *arg1);
extern node *TCmakePrf2 (prf prf, node *arg1, node *arg2);
extern node *TCmakePrf3 (prf prf, node *arg1, node *arg2, node *arg3);
extern node *TCmakePrf4 (prf prf, node *arg1, node *arg2, node *arg3, node *arg4);
extern node *TCmakePrf5 (prf prf, node *arg1, node *arg2, node *arg3, node *arg4,
                         node *arg5);

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
#define AP_NS(n) FUNDEF_NS (AP_FUNDEF (n))

#define SPAP_NAME(n) SPID_NAME (SPAP_ID (n))
#define SPAP_NS(n) SPID_NS (SPAP_ID (n))

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

extern node *TCmakeSpap1 (namespace_t *ns, char *name, node *arg1);

extern node *TCmakeSpap2 (namespace_t *ns, char *name, node *arg1, node *arg2);

extern node *TCmakeSpap3 (namespace_t *ns, char *name, node *arg1, node *arg2,
                          node *arg3);

extern node *TCmakeSpap5 (namespace_t *ns, char *name, node *arg1, node *arg2, node *arg3,
                          node *arg4, node *arg5);

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
extern node *TCappendPart (node *parts1, node *parts2);
extern bool TCcontainsDefaultPartition (node *parts);

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

extern int TCcountWithops (node *withop);
extern int TCcountWithopsEq (node *withop, nodetype eq);
extern int TCcountWithopsNeq (node *withop, nodetype neq);

/*
 * DON'T USE THE FOLLOWING MACROS
 */
#define WITHOP_NEXT(n)                                                                   \
    ((NODE_TYPE (n) == N_genarray)                                                       \
       ? GENARRAY_NEXT (n)                                                               \
       : (NODE_TYPE (n) == N_modarray)                                                   \
           ? MODARRAY_NEXT (n)                                                           \
           : (NODE_TYPE (n) == N_spfold)                                                 \
               ? SPFOLD_NEXT (n)                                                         \
               : (NODE_TYPE (n) == N_break)                                              \
                   ? BREAK_NEXT (n)                                                      \
                   : (NODE_TYPE (n) == N_propagate) ? PROPAGATE_NEXT (n)                 \
                                                    : (FOLD_NEXT (n)))

#define L_WITHOP_NEXT(n, rhs)                                                            \
    switch                                                                               \
        NODE_TYPE (n)                                                                    \
        {                                                                                \
        case N_genarray:                                                                 \
            GENARRAY_NEXT (n) = rhs;                                                     \
            break;                                                                       \
        case N_modarray:                                                                 \
            MODARRAY_NEXT (n) = rhs;                                                     \
            break;                                                                       \
        case N_fold:                                                                     \
            FOLD_NEXT (n) = rhs;                                                         \
            break;                                                                       \
        case N_break:                                                                    \
            BREAK_NEXT (n) = rhs;                                                        \
            break;                                                                       \
        case N_propagate:                                                                \
            PROPAGATE_NEXT (n) = rhs;                                                    \
            break;                                                                       \
        case N_spfold:                                                                   \
            SPFOLD_NEXT (n) = rhs;                                                       \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT (FALSE, "Illegal node type");                                    \
        }

#define WITHOP_MEM(n)                                                                    \
    ((NODE_TYPE (n) == N_genarray)                                                       \
       ? GENARRAY_MEM (n)                                                                \
       : (NODE_TYPE (n) == N_modarray)                                                   \
           ? MODARRAY_MEM (n)                                                            \
           : (NODE_TYPE (n) == N_break) ? BREAK_MEM (n) : (NULL))

#define WITHOP_IDX(n)                                                                    \
    ((NODE_TYPE (n) == N_genarray)                                                       \
       ? GENARRAY_IDX (n)                                                                \
       : (NODE_TYPE (n) == N_modarray) ? MODARRAY_IDX (n) : NULL)

#define L_WITHOP_IDX(n, rhs)                                                             \
    switch                                                                               \
        NODE_TYPE (n)                                                                    \
        {                                                                                \
        case N_genarray:                                                                 \
            GENARRAY_IDX (n) = rhs;                                                      \
            break;                                                                       \
        case N_modarray:                                                                 \
            MODARRAY_IDX (n) = rhs;                                                      \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT (FALSE, "Illegal node type");                                    \
        }

#define WITHOP_SUB(n)                                                                    \
    ((NODE_TYPE (n) == N_genarray)                                                       \
       ? GENARRAY_SUB (n)                                                                \
       : (NODE_TYPE (n) == N_modarray) ? MODARRAY_SUB (n) : NULL)

#define L_WITHOP_SUB(n, rhs)                                                             \
    switch                                                                               \
        NODE_TYPE (n)                                                                    \
        {                                                                                \
        case N_genarray:                                                                 \
            GENARRAY_SUB (n) = rhs;                                                      \
            break;                                                                       \
        case N_modarray:                                                                 \
            MODARRAY_SUB (n) = rhs;                                                      \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT (FALSE, "Illegal node type");                                    \
        }

#define WITHOP_RC(n)                                                                     \
    ((NODE_TYPE (n) == N_genarray)                                                       \
       ? GENARRAY_RC (n)                                                                 \
       : (NODE_TYPE (n) == N_modarray) ? MODARRAY_RC (n) : NULL)

#define L_WITHOP_RC(n, rhs)                                                              \
    switch                                                                               \
        NODE_TYPE (n)                                                                    \
        {                                                                                \
        case N_genarray:                                                                 \
            GENARRAY_RC (n) = rhs;                                                       \
            break;                                                                       \
        case N_modarray:                                                                 \
            MODARRAY_RC (n) = rhs;                                                       \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT (FALSE, "Illegal node type");                                    \
        }

/*--------------------------------------------------------------------------*/

/***
 ***  N_with2 :
 ***/

#define WITH2_TYPE(n) (NODE_TYPE (WITH2_WITHOP (n)))

#define WITH2_IDS(n) (WITHID_IDS (WITH2_WITHID (n)))
#define WITH2_VEC(n) (WITHID_VEC (WITH2_WITHID (n)))
#define WITH2_IDXS(n) (WITHID_IDXS (WITH2_WITHID (n)))

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

extern int TCcountWlseg (node *withop);
/*--------------------------------------------------------------------------*/

/***
 ***  N_with :  *and*  N_with2 :
 ***/

#define WITH_OR_WITH2_WITHOP(n)                                                          \
    ((NODE_TYPE (n) == N_with) ? WITH_WITHOP (n) : WITH2_WITHOP (n))

#define WITH_OR_WITH2_TYPE(n) ((NODE_TYPE (n) == N_with) ? WITH_TYPE (n) : WITH2_TYPE (n))
#define WITH_OR_WITH2_IDS(n) ((NODE_TYPE (n) == N_with) ? WITH_IDS (n) : WITH2_IDS (n))
#define WITH_OR_WITH2_VEC(n) ((NODE_TYPE (n) == N_with) ? WITH_VEC (n) : WITH2_VEC (n))

#define WITH_OR_WITH2_CODE(n) ((NODE_TYPE (n) == N_with) ? WITH_CODE (n) : WITH2_CODE (n))
#define WITH_OR_WITH2_CEXPR(n)                                                           \
    ((NODE_TYPE (n) == N_with) ? WITH_CEXPR (n) : WITH2_CEXPRS (n))

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
 ***  N_with :  *and*  N_with2 *and* N_with3 :
 ***/

#define WITH_OR_WITH2_OR_WITH3_WITHOP(n)                                                 \
    ((NODE_TYPE (n) == N_with3) ? WITH3_OPERATIONS (n) : WITH_OR_WITH2_WITHOP (n))

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlseg :
 ***/

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
 ***  N_wlgrid :
 ***/

#define WLGRID_CBLOCK(n) (CODE_CBLOCK (WLGRID_CODE (n)))
#define WLGRID_CEXPR(n) (CODE_CEXPR (WLGRID_CODE (n)))

#define WLGRID_CBLOCK_INSTR(n) (BLOCK_INSTR (WLGRID_CBLOCK (n)))

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlblock :   *and*  N_wlublock :     *and*
 ***  N_wlstride :  *and*  N_wlgrid :
 ***/

#define WLNODE_ISNOOP(n)                                                                 \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_ISNOOP (n)                                                              \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_ISNOOP (n)                                                        \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_ISNOOP (n)                                                   \
                 : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_ISNOOP (n) : FALSE))))

#define WLNODE_ISDYNAMIC(n)                                                              \
    ((NODE_TYPE (n) == N_wlseg)                                                          \
       ? WLSEG_ISDYNAMIC (n)                                                             \
       : ((NODE_TYPE (n) == N_wlstride)                                                  \
            ? WLSTRIDE_ISDYNAMIC (n)                                                     \
            : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_ISDYNAMIC (n) : FALSE)))

#define WLNODE_NEXT(n)                                                                   \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_NEXT (n)                                                                \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_NEXT (n)                                                          \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_NEXT (n)                                                     \
                 : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_NEXT (n) : NULL))))

#define WLNODE_LEVEL(n)                                                                  \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_LEVEL (n)                                                               \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_LEVEL (n)                                                         \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_LEVEL (n)                                                    \
                 : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_LEVEL (n) : 0))))

#define WLNODE_DIM(n)                                                                    \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_DIM (n)                                                                 \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_DIM (n)                                                           \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_DIM (n)                                                      \
                 : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_DIM (n) : 0))))

#define WLNODE_BOUND1(n)                                                                 \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_BOUND1 (n)                                                              \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_BOUND1 (n)                                                        \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_BOUND1 (n)                                                   \
                 : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_BOUND1 (n) : NULL))))

#define WLNODE_BOUND2(n)                                                                 \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_BOUND2 (n)                                                              \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_BOUND2 (n)                                                        \
            : ((NODE_TYPE (n) == N_wlstride)                                             \
                 ? WLSTRIDE_BOUND2 (n)                                                   \
                 : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_BOUND2 (n) : NULL))))

#define WLNODE_STEP(n)                                                                   \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_STEP (n)                                                                \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_STEP (n)                                                          \
            : ((NODE_TYPE (n) == N_wlstride) ? WLSTRIDE_STEP (n) : NULL)))

#define WLNODE_NEXTDIM(n)                                                                \
    ((NODE_TYPE (n) == N_wlblock)                                                        \
       ? WLBLOCK_NEXTDIM (n)                                                             \
       : ((NODE_TYPE (n) == N_wlublock)                                                  \
            ? WLUBLOCK_NEXTDIM (n)                                                       \
            : ((NODE_TYPE (n) == N_wlgrid) ? WLGRID_NEXTDIM (n) : NULL)))

#define L_WLNODE_NEXT(n, rhs)                                                            \
    if (NODE_TYPE (n) == N_wlblock)                                                      \
        WLBLOCK_NEXT (n) = rhs;                                                          \
    else if (NODE_TYPE (n) == N_wlublock)                                                \
        WLUBLOCK_NEXT (n) = rhs;                                                         \
    else if (NODE_TYPE (n) == N_wlstride)                                                \
        WLSTRIDE_NEXT (n) = rhs;                                                         \
    else if (NODE_TYPE (n) == N_wlgrid)                                                  \
        WLGRID_NEXT (n) = rhs;                                                           \
    else                                                                                 \
        DBUG_ASSERT (0, "L_WLNODE_NEXT called on wrong node type");

#define L_WLNODE_BOUND1(n, rhs)                                                          \
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

#define L_WLNODE_BOUND2(n, rhs)                                                          \
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

#define L_WLNODE_STEP(n, rhs)                                                            \
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
 ***  N_set :
 ***/

extern node *TCappendSet (node *links1, node *links2);
extern node *TCdropSet (int num, node *set);
extern int TCsetAdd (node **links, node *link);
extern int TCsetDel (node **links, node *link);
extern int TCsetUnion (node **links, node *add);
extern bool TCsetContains (node *set, node *link);
extern bool TCsetIsSubset (node *super, node *sub);

/*--------------------------------------------------------------------------*/

/***
 ***  N_ids :
 ***/

extern node *TCappendError (node *chain, node *item);

/*--------------------------------------------------------------------------*/

/***
 ***  N_range :
 ***/

extern node *TCappendRange (node *range_chain, node *range);

#endif /* _SAC_TREE_COMPOUND_H_ */
