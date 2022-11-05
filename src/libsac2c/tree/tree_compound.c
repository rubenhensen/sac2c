#include "tree_basic.h"
#include "tree_compound.h"
#include "compare_tree.h"
#include "DupTree.h"
#include "shape.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "free.h"
#include "new_types.h"
#include "user_types.h"
#include "new_typecheck.h"
#include "ctinfo.h"
#include "DataFlowMask.h"
#include "wltransform.h"
#include "globals.h"
#include "NameTuplesUtils.h"
#include "type_utils.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "traverse.h"
#include "math_utils.h"

/*
 * macro template for append functions
 */
#define APPEND(ret, type, atype, chain, item)                                            \
    if (item != NULL) {                                                                  \
        if (chain == NULL) {                                                             \
            chain = item;                                                                \
        } else {                                                                         \
            type __tmp;                                                                  \
            __tmp = chain;                                                               \
            while (atype##_NEXT (__tmp) != NULL) {                                       \
                __tmp = atype##_NEXT (__tmp);                                            \
            }                                                                            \
            atype##_NEXT (__tmp) = item;                                                 \
        }                                                                                \
    }                                                                                    \
    ret = chain

/*--------------------------------------------------------------------------*/
/*  macros and functions for non-node structures                            */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/*****************************************************************************
 *
 * function:
 *   bool TCshapeVarsMatch( node *avis1, node *avis2)
 *
 * description:
 *   Compares the shapes of two arrays, based on their N_avis SAA shapes.
 *
 *
 *****************************************************************************/

bool
TCshapeVarsMatch (node *avis1, node *avis2)
{
    bool res;

    DBUG_ENTER ();

    res = ((TYeqTypes (TYgetScalar (AVIS_TYPE (avis1)), TYgetScalar (AVIS_TYPE (avis2))))
           && (AVIS_DIM (avis1) != NULL) && (AVIS_DIM (avis2) != NULL)
           && (AVIS_SHAPE (avis1) != NULL) && (AVIS_SHAPE (avis2) != NULL)
           && (CMPTdoCompareTree (AVIS_DIM (avis1), AVIS_DIM (avis2)) == CMPT_EQ)
           && (CMPTdoCompareTree (AVIS_SHAPE (avis1), AVIS_SHAPE (avis2)) == CMPT_EQ));

    DBUG_RETURN (res);
}


/*--------------------------------------------------------------------------*/

/***
 ***  IDS :
 ***/

node *
TCappendIds (node *chain, node *item)
{
    node *ret;

    DBUG_ENTER ();

    APPEND (ret, node *, IDS, chain, item);

    DBUG_RETURN (ret);
}

node *
TClookupIds (const char *name, node *ids_chain)
{
    DBUG_ENTER ();

    while ((ids_chain != NULL) && (!STReq (name, IDS_NAME (ids_chain)))) {
        ids_chain = IDS_NEXT (ids_chain);
    }

    DBUG_RETURN (ids_chain);
}

/* isIdsMember - pointer passing back result of search for 
*                whether arg_node is a member of the ids chain */
size_t
TClookupIdsNode (node *ids_chain, node *idsavis, bool *isIdsMember)
{
    size_t z;

    DBUG_ENTER ();
    *isIdsMember = TRUE;
    z = 0;
    while ((ids_chain != NULL) && (IDS_AVIS (ids_chain) != idsavis)) {
        ids_chain = IDS_NEXT (ids_chain);
        z++;
    }

    *isIdsMember = (NULL != ids_chain) ? TRUE : FALSE;

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * Function:
 *   node *TCgetNthIds( int n, node *ids_chain)
 *
 * Description:
 *   Return IDS_AVIS( ids_chain[n])
 *
 ******************************************************************************/
node *
TCgetNthIds (size_t n, node *ids_chain)
{
    DBUG_ENTER ();
    size_t i = 0;

    while ((ids_chain != NULL) && (i != n)) {
        ids_chain = IDS_NEXT (ids_chain);
        i++;
    }
    ids_chain = IDS_AVIS (ids_chain);

    DBUG_RETURN (ids_chain);
}

size_t
TCcountIds (node *ids_arg)
{
    size_t count = 0;

    DBUG_ENTER ();

    while (ids_arg != NULL) {
        count++;
        ids_arg = IDS_NEXT (ids_arg);
    }

    DBUG_RETURN (count);
}

node *
TCmakeIdsFromVardecs (node *vardecs)
{
    node *ids = NULL;
    DBUG_ENTER ();
    while (vardecs != NULL) {
        ids = TBmakeIds (VARDEC_AVIS (vardecs), ids);
        vardecs = VARDEC_NEXT (vardecs);
    }
    DBUG_RETURN (ids);
}

/** <!-- ****************************************************************** -->
 * @fn node *TClastIds( node *ids)
 *
 * @brief Return the last ids in a chain of ids
 *
 * @param ids chain to return last of
 *
 *****************************************************************************/
node *
TClastIds (node *ids)
{
    node *lastIds;
    DBUG_ENTER ();

    if (IDS_NEXT (ids) != NULL) {
        lastIds = TClastIds (IDS_NEXT (ids));
    } else {
        lastIds = ids;
    }

    DBUG_RETURN (lastIds);
}

node *
TCconvertIds2Exprs (node *ids)
{
    node *exprs = NULL;
    DBUG_ENTER ();

    while (ids != NULL) {
        if (exprs == NULL) {
            exprs = TBmakeExprs (TBmakeId (IDS_AVIS (ids)), NULL);
        } else {
            exprs = TCappendExprs (exprs, TBmakeExprs (TBmakeId (IDS_AVIS (ids)), NULL));
        }

        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (exprs);
}

/*--------------------------------------------------------------------------*/

/***
 ***  NUMS :
 ***/

size_t
TCcountNums (node *nums)
{
    size_t cnt = 0;

    DBUG_ENTER ();

    while (nums != NULL) {
        cnt++;
        nums = NUMS_NEXT (nums);
    }

    DBUG_RETURN (cnt);
}

bool
TCnumsContains (int val, node *nums)
{
    bool result = FALSE;

    DBUG_ENTER ();

    while ((nums != NULL) && (!result)) {
        result = (NUMS_VAL (nums) == val);
        nums = NUMS_NEXT (nums);
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
 ***/

/******************************************************************************
 *
 * function:
 *   nodelist *TCnodeListAppend(nodelist *nl, node *newnode, void *attrib)
 *   nodelist *TCnodeListDelete(nodelist *nl, node *node, bool free_attrib)
 *   nodelist *TcnodeListFree(nodelist *nl, bool free_attrib)
 *   nodelist *TCnodeListFind(nodelist *nl, node *node)
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

nodelist *
TCnodeListAppend (nodelist *nl, node *newnode, void *attrib)
{
    DBUG_ENTER ();

    nl = TBmakeNodelistNode (newnode, nl);
    NODELIST_ATTRIB2 (nl) = (node *)attrib;

    DBUG_RETURN (nl);
}

nodelist *
TCnodeListDelete (nodelist *nl, node *node, bool free_attrib)
{
    nodelist *tmpnl, *prevnl;

    DBUG_ENTER ();

    while (nl && NODELIST_NODE (nl) == node) {
        if (free_attrib && NODELIST_ATTRIB2 (nl)) {
            NODELIST_ATTRIB2 (nl) = MEMfree (NODELIST_ATTRIB2 (nl));
        }
        nl = FREEfreeNodelistNode (nl);
    }

    tmpnl = nl;
    prevnl = NULL;
    while (tmpnl) {
        if (NODELIST_NODE (tmpnl) == node) {
            if (free_attrib && NODELIST_ATTRIB2 (tmpnl)) {
                NODELIST_ATTRIB2 (tmpnl) = MEMfree (NODELIST_ATTRIB2 (tmpnl));
            }

            NODELIST_NEXT (prevnl) = FREEfreeNodelistNode (tmpnl);
        } else {
            prevnl = tmpnl;
        }

        tmpnl = NODELIST_NEXT (prevnl);
    }

    DBUG_RETURN (nl);
}

nodelist *
TCnodeListFree (nodelist *nl, bool free_attrib)
{
    DBUG_ENTER ();

    while (nl) {
        if (free_attrib && NODELIST_ATTRIB2 (nl)) {
            NODELIST_ATTRIB2 (nl) = MEMfree (NODELIST_ATTRIB2 (nl));
        }
        nl = FREEfreeNodelistNode (nl);
    }

    DBUG_RETURN (nl);
}

nodelist *
TCnodeListFind (nodelist *nl, node *node)
{
    DBUG_ENTER ();

    while (nl && NODELIST_NODE (nl) != node) {
        nl = NODELIST_NEXT (nl);
    }

    DBUG_RETURN (nl);
}

/*--------------------------------------------------------------------------*/
/*  macros and functions for node structures                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/***
 ***  N_modul :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_moddec :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_classdec :
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

node *
TCsearchTypedef (const char *name, const namespace_t *ns, node *implementations)
{
    node *tmp;

    DBUG_ENTER ();

    tmp = implementations;

    while ((tmp != NULL)
           && (!STReq (name, TYPEDEF_NAME (tmp)) || !NSequals (ns, TYPEDEF_NS (tmp)))) {
        tmp = TYPEDEF_NEXT (tmp);
    }

    DBUG_RETURN (tmp);
}

node *
TCappendTypedef (node *tdef_chain, node *tdef)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((tdef_chain == NULL) || (NODE_TYPE (tdef_chain) == N_typedef)),
                 "First argument of TCappendTypedef() has wrong node type.");
    DBUG_ASSERT (((tdef == NULL) || (NODE_TYPE (tdef) == N_typedef)),
                 "Second argument of TCappendTypedef() has wrong node type.");

    APPEND (ret, node *, TYPEDEF, tdef_chain, tdef);

    DBUG_RETURN (ret);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_objdef :
 ***/

node *
TCappendObjdef (node *objdef_chain, node *objdef)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((objdef_chain == NULL) || (NODE_TYPE (objdef_chain) == N_objdef)),
                 "First argument of AppendObjdef() has wrong node type.");
    DBUG_ASSERT (((objdef == NULL) || (NODE_TYPE (objdef) == N_objdef)),
                 "Second argument of AppendObjdef() has wrong node type.");

    APPEND (ret, node *, OBJDEF, objdef_chain, objdef);

    DBUG_RETURN (ret);
}

node *
TCunAliasObjdef (node *objdef)
{
    node *result;

    DBUG_ENTER ();

    result = objdef;

    while (OBJDEF_ISALIAS (result)) {
        DBUG_ASSERT (NODE_TYPE (OBJDEF_EXPR (result)) == N_globobj,
                     "found objdef alias without proper init expression!");

        result = GLOBOBJ_OBJDEF (OBJDEF_EXPR (result));
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
 ***/

node *
TCappendFundef (node *fundef_chain, node *fundef)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((fundef_chain == NULL) || (NODE_TYPE (fundef_chain) == N_fundef)),
                 "First argument of TCappendFundef() has wrong node type.");
    DBUG_ASSERT (((fundef == NULL) || (NODE_TYPE (fundef) == N_fundef)),
                 "Second argument of TCappendFundef() has wrong node type.");

    APPEND (ret, node *, FUNDEF, fundef_chain, fundef);

    DBUG_RETURN (ret);
}

node *
TCremoveFundef (node *fundef_chain, node *fundef)
{
    node *pos;
    DBUG_ENTER ();

    if (fundef_chain == fundef) {
        fundef_chain = FUNDEF_NEXT (fundef_chain);
    } else {
        pos = fundef_chain;

        while ((FUNDEF_NEXT (pos) != NULL) && (FUNDEF_NEXT (pos) != fundef)) {
            pos = FUNDEF_NEXT (pos);
        }

        if (FUNDEF_NEXT (pos) == fundef) {
            FUNDEF_NEXT (pos) = FUNDEF_NEXT (fundef);
        }
    }

    FUNDEF_NEXT (fundef) = NULL;

    DBUG_RETURN (fundef_chain);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_block :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :
 ***/

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

node *
TCaddVardecs (node *fundef, node *vardecs)
{
    DBUG_ENTER ();

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "no N_fundef node found!");

    /*
     * insert new vardecs into AST
     */
    FUNDEF_VARDECS (fundef) = TCappendVardec (vardecs, FUNDEF_VARDECS (fundef));

    /*
     * we must update FUNDEF_DFM_BASE!!
     */
    if (FUNDEF_DFM_BASE (fundef) != NULL) {
        FUNDEF_DFM_BASE (fundef)
          = DFMupdateMaskBase (FUNDEF_DFM_BASE (fundef), FUNDEF_ARGS (fundef),
                               FUNDEF_VARDECS (fundef));
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *TCappendVardec( node *vardec_chain, node *vardec)
 *
 * description:
 *   Appends 'vardec' to 'vardec_chain' and returns the new chain.
 *
 ******************************************************************************/

node *
TCappendVardec (node *vardec_chain, node *vardec)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((vardec_chain == NULL) || (NODE_TYPE (vardec_chain) == N_vardec)),
                 "First argument of AppendVardec() has wrong node type.");
    DBUG_ASSERT (((vardec == NULL) || (NODE_TYPE (vardec) == N_vardec)),
                 "Second argument of AppendVardec() has wrong node type.");

    APPEND (ret, node *, VARDEC, vardec_chain, vardec);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   size_t TCcountVardecs( node *vardecs)
 *
 * description:
 *   Counts the number of N_vardec nodes.
 *
 ******************************************************************************/

size_t
TCcountVardecs (node *vardecs)
{
    size_t count = 0;

    DBUG_ENTER ();

    while (vardecs != NULL) {
        DBUG_ASSERT (NODE_TYPE (vardecs) == N_vardec, "no N_vardec node found!");
        count++;
        vardecs = VARDEC_NEXT (vardecs);
    }

    DBUG_RETURN (count);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***/

/******************************************************************************
 *
 * function:
 *   size_t TCcountArgs( node *args)
 *
 * description:
 *   Counts the number of N_arg nodes.
 *
 ******************************************************************************/

size_t
TCcountArgs (node *args)
{
    size_t count = 0;

    DBUG_ENTER ();

    while (args != NULL) {
        DBUG_ASSERT (NODE_TYPE (args) == N_arg, "no N_arg node found!");
        count++;
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * function:
 *   size_t TCcountArgsIgnoreArtificials( node *args)
 *
 * description:
 *   Counts the number of N_arg nodes, ignoring those that are marked
 *   as artificial.
 *
 ******************************************************************************/

size_t
TCcountArgsIgnoreArtificials (node *args)
{
    size_t count = 0;

    DBUG_ENTER ();

    while (args != NULL) {
        DBUG_ASSERT (NODE_TYPE (args) == N_arg, "no N_arg node found!");
        if (!ARG_ISARTIFICIAL (args)) {
            count++;
        }
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * function:
 *   node *TCappendArgs( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_arg chains of nodes.
 *
 ******************************************************************************/

node *
TCappendArgs (node *arg_chain, node *arg)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((arg_chain == NULL) || (NODE_TYPE (arg_chain) == N_arg)),
                 "First argument of TCappendArgs() has wrong node type.");
    DBUG_ASSERT (((arg == NULL) || (NODE_TYPE (arg) == N_arg)),
                 "Second argument of TCappendArgs() has wrong node type.");

    APPEND (ret, node *, ARG, arg_chain, arg);

    DBUG_RETURN (ret);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCgetNthArg(int n, node *args)
 *
 * @brief Given an N_arg chain, return the nTH N_arg node
 *        in the chain. If 0==n, return the argument.
 *        If n > chain length, return NULL.
 *
 * @param N_arg chain
 *
 * @return N_arg node
 ******************************************************************************/
node *
TCgetNthArg (size_t n, node *args)
{
    size_t cnt;
    node *result = NULL;

    DBUG_ENTER ();

    for (cnt = 0; cnt < n; cnt++) {
        if (NULL == args) {
            DBUG_UNREACHABLE ("n > N_arg chain length.");
        }

        args = ARG_NEXT (args);
    }

    result = args;
    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_ret :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCappendRet( node *exprs1, node *exprs2)
 *
 * description:
 *   This function concatenates two N_ret chains of nodes.
 *
 ******************************************************************************/

node *
TCappendRet (node *ret_chain, node *item)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((ret_chain == NULL) || (NODE_TYPE (ret_chain) == N_ret)),
                 "First argument of TCappendRet() has wrong node type.");
    DBUG_ASSERT (((item == NULL) || (NODE_TYPE (item) == N_ret)),
                 "Second argument of TCappendRet() has wrong node type.");

    APPEND (ret, node *, RET, ret_chain, item);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   size_t TCcountRets( node *rets)
 *
 * description:
 *   Counts the number of N_arg nodes.
 *
 ******************************************************************************/

size_t
TCcountRets (node *rets)
{
    size_t count = 0;

    DBUG_ENTER ();

    while (rets != NULL) {
        DBUG_ASSERT (NODE_TYPE (rets) == N_ret, "no N_ret node found!");
        count++;
        rets = RET_NEXT (rets);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * function:
 *   size_t TCcountRetsIgnoreArtificials( node *rets)
 *
 * description:
 *   Counts the number of N_arg nodes, ignoring those that are marked as
 *   artificial.
 *
 ******************************************************************************/

size_t
TCcountRetsIgnoreArtificials (node *rets)
{
    size_t count = 0;

    DBUG_ENTER ();

    while (rets != NULL) {
        DBUG_ASSERT (NODE_TYPE (rets) == N_ret, "no N_ret node found!");
        if (!RET_ISARTIFICIAL (rets)) {
            count++;
        }
        rets = RET_NEXT (rets);
    }

    DBUG_RETURN (count);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateIdsFromRets( node *rets, node **vardecs)
 *
 * @brief Creates a N_ids chain with the types given by the N_rets chain.
 *        Vardecs for the created N_ids are appended to the given vardecs
 *        chain.
 *
 * @param rets N_rets chain
 * @param vardecs address of a N_vardec chain
 *
 * @return created N_ids chain
 ******************************************************************************/
node *
TCcreateIdsFromRets (node *rets, node **vardecs)
{
    node *vardec;
    node *result;

    DBUG_ENTER ();

    if (rets != NULL) {
        vardec
          = TBmakeVardec (TBmakeAvis (TRAVtmpVar (), TYcopyType (RET_TYPE (rets))), NULL);
        result = TBmakeIds (VARDEC_AVIS (vardec),
                            TCcreateIdsFromRets (RET_NEXT (rets), vardecs));

        *vardecs = TCappendVardec (vardec, *vardecs);
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :  *and*  N_arg :
 ***/

/*
 *
 *  functionname  : SearchDecl
 *  arguments     : 1) character string
 *                  2) N_vardec - or N_arg - chain
 *                  R) true or false
 *  description   : returns ptr to N_vardec - or N_arg - node  if variable or
 *                  argument with same name as in 1) has been found, else NULL
 *
 */

node *
TCsearchDecl (const char *name, node *decl_node)
{
    node *found = NULL;

    DBUG_ENTER ();

    while (NULL != decl_node) {
        if (N_vardec == NODE_TYPE (decl_node)) {
            if (!STReq (name, VARDEC_NAME (decl_node))) {
                found = decl_node;
                decl_node = NULL;
            } else {
                decl_node = VARDEC_NEXT (decl_node);
            }
        } else {
            if (!STReq (name, ARG_NAME (decl_node))) {
                found = decl_node;
                decl_node = NULL;
            } else {
                decl_node = ARG_NEXT (decl_node);
            }
        }
    }

    DBUG_RETURN (found);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_assign :
 ***/

/******************************************************************************
 *
 * Function:
 *   node *TCmakeAssignInstr( node *instr, node *next)
 *
 * Description:
 *
 *
 ******************************************************************************/
static node *
TCmakeAssignInstr (node *instr, node *next)
{
    node *result;

    DBUG_ENTER ();

    if (instr == NULL) {
        result = next;
    } else if (NODE_TYPE (instr) == N_assign) {
        result = TCappendAssign (instr, next);
    } else {
        result = TBmakeAssign (instr, next);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn size_t TCcountAssigns( node *arg_node)
 *
 *****************************************************************************/
size_t
TCcountAssigns (node *arg_node)
{
    size_t res = 0;

    DBUG_ENTER ();

    while (arg_node != NULL) {
        res += 1;
        arg_node = ASSIGN_NEXT (arg_node);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn int TCgetLastAssign( node *arg_node)
 *
 *****************************************************************************/
node *
TCgetLastAssign (node *arg_node)
{
    DBUG_ENTER ();

    if (arg_node != NULL) {
        while (ASSIGN_NEXT (arg_node) != NULL) {
            arg_node = ASSIGN_NEXT (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *TCmakeAssign?( node *part?, ...)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TCmakeAssigns1 (node *part1)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TCmakeAssignInstr (part1, NULL);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns2 (node *part1, node *part2)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns1 (part2));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns3 (node *part1, node *part2, node *part3)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns2 (part2, part3));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns4 (node *part1, node *part2, node *part3, node *part4)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns3 (part2, part3, part4));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns5 (node *part1, node *part2, node *part3, node *part4, node *part5)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns4 (part2, part3, part4, part5));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns6 (node *part1, node *part2, node *part3, node *part4, node *part5,
                node *part6)
{
    node *assigns;

    DBUG_ENTER ();

    assigns
      = TCmakeAssignInstr (part1, TCmakeAssigns5 (part2, part3, part4, part5, part6));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns7 (node *part1, node *part2, node *part3, node *part4, node *part5,
                node *part6, node *part7)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns6 (part2, part3, part4, part5, part6,
                                                        part7));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns8 (node *part1, node *part2, node *part3, node *part4, node *part5,
                node *part6, node *part7, node *part8)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns7 (part2, part3, part4, part5, part6,
                                                        part7, part8));

    DBUG_RETURN (assigns);
}

node *
TCmakeAssigns9 (node *part1, node *part2, node *part3, node *part4, node *part5,
                node *part6, node *part7, node *part8, node *part9)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TCmakeAssignInstr (part1, TCmakeAssigns8 (part2, part3, part4, part5, part6,
                                                        part7, part8, part9));

    DBUG_RETURN (assigns);
}

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

node *
TCmakeAssignIcm0 (const char *name, node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TBmakeAssign (TCmakeIcm0 (name), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm1 (const char *name, node *arg1, node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TBmakeAssign (TCmakeIcm1 (name, arg1), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm2 (const char *name, node *arg1, node *arg2, node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TBmakeAssign (TCmakeIcm2 (name, arg1, arg2), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm3 (const char *name, node *arg1, node *arg2, node *arg3, node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TBmakeAssign (TCmakeIcm3 (name, arg1, arg2, arg3), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm4 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                  node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TBmakeAssign (TCmakeIcm4 (name, arg1, arg2, arg3, arg4), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm5 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                  node *arg5, node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TBmakeAssign (TCmakeIcm5 (name, arg1, arg2, arg3, arg4, arg5), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm6 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                  node *arg5, node *arg6, node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns = TBmakeAssign (TCmakeIcm6 (name, arg1, arg2, arg3, arg4, arg5, arg6), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm7 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                  node *arg5, node *arg6, node *arg7, node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns
      = TBmakeAssign (TCmakeIcm7 (name, arg1, arg2, arg3, arg4, arg5, arg6, arg7), next);

    DBUG_RETURN (assigns);
}

node *
TCmakeAssignIcm8 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4,
                  node *arg5, node *arg6, node *arg7, node *arg8, node *next)
{
    node *assigns;

    DBUG_ENTER ();

    assigns
      = TBmakeAssign (TCmakeIcm8 (name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8),
                      next);

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *TCgetCompoundNode( node *arg_node)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
TCgetCompoundNode (node *arg_node)
{
    node *compound_node;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign,
                 "TCgetCompoundNode() can handle N_assign nodes only!");

    arg_node = ASSIGN_STMT (arg_node);
    switch (NODE_TYPE (arg_node)) {
    case N_cond:
        /* here is no break missing */
    case N_do:
        /* here is no break missing */
    case N_while:
        compound_node = arg_node;
        break;

    case N_let:
        arg_node = LET_EXPR (arg_node);
        while (N_cast == NODE_TYPE (arg_node)) {
            arg_node = CAST_EXPR (arg_node);
        }
        if (N_with == NODE_TYPE (arg_node)) {
            compound_node = arg_node;
        } else {
            compound_node = NULL;
        }
        break;

    default:
        compound_node = NULL;
    }

    DBUG_RETURN (compound_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCappendAssign( node *assign_chain, node *assign)
 *
 * description:
 *   Appends 'assign' to 'assign_chain' and returns the new chain.
 *
 ******************************************************************************/

node *
TCappendAssign (node *assign_chain, node *assign)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((assign_chain == NULL) || (NODE_TYPE (assign_chain) == N_assign)),
                 "First argument of TCappendAssign() has wrong node type.");
    DBUG_ASSERT (((assign == NULL) || (NODE_TYPE (assign) == N_assign)),
                 "Second argument of TCappendAssign() has wrong node type.");

    APPEND (ret, node *, ASSIGN, assign_chain, assign);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *TCappendAssignIcm( node *assign, char *name, node *args)
 *
 * description:
 *   Appends an new ICM with name and args given as an assign to the given
 *   chain of assignments.
 *
 ******************************************************************************/

node *
TCappendAssignIcm (node *assign, char *name, node *args)
{
    node *result;

    DBUG_ENTER ();

    result = TCappendAssign (assign, TCmakeAssignIcm1 (name, args, NULL));

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
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

node *
TCappendExprs (node *exprs_chain, node *exprs)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((exprs_chain == NULL) || (NODE_TYPE (exprs_chain) == N_exprs)),
                 "First argument of TCappendExprs() has wrong node type.");
    DBUG_ASSERT (((exprs == NULL) || (NODE_TYPE (exprs) == N_exprs)),
                 "Second argument of TCappendExprs() has wrong node type.");

    APPEND (ret, node *, EXPRS, exprs_chain, exprs);

    DBUG_RETURN (ret);
}

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

node *
TCcombineExprs (node *first, node *second)
{
    node *result;

    DBUG_ENTER ();

    if (first != NULL) {
        if (NODE_TYPE (first) != N_exprs) {
            result = TBmakeExprs (first, second);
        } else {
            result = TCappendExprs (first, second);
        }
    } else if (second != NULL) {
        if (NODE_TYPE (second) != N_exprs) {
            result = TBmakeExprs (second, NULL);
        } else {
            result = second;
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateExprsFromVardecs( node *vardec)
 *
 * @brief Returns an N_exprs chain containing N_id nodes with
 *        the avis given by the given vardecs.
 *
 * @param vardec N_vardec chain
 *
 * @return created N_exprs chain
 ******************************************************************************/
node *
TCcreateExprsFromVardecs (node *vardec)
{
    node *result;

    DBUG_ENTER ();

    if (vardec != NULL) {
        result = TBmakeExprs (TBmakeId (VARDEC_AVIS (vardec)),
                              TCcreateExprsFromVardecs (VARDEC_NEXT (vardec)));
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateExprsChainFromAvises( int num_avises, ...)
 *
 * @brief Returns an N_exprs chain containing N_id nodes with
 *        the avis given by as varargs
 *
 * @return created N_exprs chain
 ******************************************************************************/
node *
TCcreateExprsChainFromAvises (int num_avises, ...)
{
    va_list ap;
    int i;
    node *exprs;

    DBUG_ENTER ();
    va_start (ap, num_avises);
    exprs = NULL;
    for (i = 0; i < num_avises; i++) {
        exprs = TCappendExprs (exprs, TBmakeExprs (TBmakeId (va_arg (ap, node *)), NULL));
    }
    va_end (ap);
    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   node *TCmakeExprsNum( int num)
 *
 * description:
 *   Makes an N_exprs with a N_num as EXPR, NEXT is NULL.
 *
 ******************************************************************************/

node *
TCmakeExprsNum (int num)
{
    node *result;

    DBUG_ENTER ();

    result = TBmakeExprs (TBmakeNum (num), NULL);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   size_t TCcountExprs( node *exprs)
 *
 * description:
 *   Computes the length of the given N_exprs chain.
 *
 ******************************************************************************/

size_t
TCcountExprs (node *exprs)
{
    size_t count;

    DBUG_ENTER ();

    count = 0;
    while (exprs != NULL) {
        DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "no N_exprs node found!");
        count++;
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (count);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateExprsFromIds( node *ids)
 *
 * @brief Creates a N_exprs chain containing N_id nodes corresponding to the
 *        given N_ids chain.
 *
 * @param ids N_ids chain
 *
 * @return created N_exprs chain
 ******************************************************************************/
node *
TCcreateExprsFromIds (node *ids)
{
    node *result;

    DBUG_ENTER ();

    if (ids != NULL) {
        result = TBmakeExprs (TBmakeId (IDS_AVIS (ids)),
                              TCcreateExprsFromIds (IDS_NEXT (ids)));
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateArrayFromIds( node *ids)
 *
 * @brief Creates a N_array node containing N_id nodes corresponding to the
 *        given N_ids chain.
 *
 * @param ids N_ids chain
 *
 * @return created N_array node
 ******************************************************************************/
node *
TCcreateArrayFromIds (node *ids)
{
    node *result;

    DBUG_ENTER ();

    result = TCcreateExprsFromIds (ids);
    if (NULL != result) {
        result = TCmakeIntVector (result);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateArrayFromIdsDrop(...)
 *
 * @brief Creates a N_array node containing N_id nodes corresponding to the
 *        given N_ids chain.
 *
 * @param ids N_ids chain
 *        dropcount: number of leading ids entries to ignore
 *
 * @return created N_array node
 ******************************************************************************/
node *
TCcreateArrayFromIdsDrop (int dropcount, node *ids)
{
    node *result;

    DBUG_ENTER ();

    if (dropcount != 0) {
        result = TCcreateArrayFromIdsDrop (dropcount - 1, IDS_NEXT (ids));
    } else {
        result = TCcreateExprsFromIds (ids);
        if (NULL != result) {
            result = TCmakeIntVector (result);
        }
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn int TCcountSpids (node *spids)
 *
 * @brief counts the number of N_spid nodes in the N_spids chain.
 *
 * @param args N_spids chain
 *
 * @return number of N_spid nodes
 ******************************************************************************/
int
TCcountSpids (node *spids)
{
    int count;

    DBUG_ENTER ();

    count = 0;
    while (spids != NULL) {
        DBUG_ASSERT (NODE_TYPE (spids) == N_spids, "no N_spids node found!");
        count++;
        spids = SPIDS_NEXT (spids);
    }

    DBUG_RETURN (count);
}


/** <!-- ****************************************************************** -->
 * @fn node *TCcreateExprsFromArgs( node *args)
 *
 * @brief Creates a N_exprs chain containing N_id nodes corresponding to the
 *        given N_arg chain.
 *
 * @param args N_arg chain
 *
 * @return created N_exprs chain
 ******************************************************************************/
node *
TCcreateExprsFromArgs (node *args)
{
    node *result;

    DBUG_ENTER ();

    if (args != NULL) {
        result = TBmakeExprs (TBmakeId (ARG_AVIS (args)),
                              TCcreateExprsFromArgs (ARG_NEXT (args)));
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCgetNthExprsNext(int n, node *args)
 *
 * @brief Given an N_exprs chain, return the nTH item
 *        in the chain. If 0==n, return the argument.
 *        If n > chain length, return NULL.
 *        This differs from TCgetNthExpr slightly, but
 *        I didn't want to change the semantics of TCgetNthExpr.
 *        It is handy for getting to a specific N_exprs node,
 *        rather than to its EXPRS_EXPRS.
 *
 * @param N_exprs chain
 *
 * @return N_exprs node
 ******************************************************************************/
node *
TCgetNthExprsNext (size_t n, node *exprs)
{
    size_t cnt;
    node *result;

    DBUG_ENTER ();

    for (cnt = 0; cnt < n; cnt++) {
        if (exprs == NULL) {
            break;
        }

        exprs = EXPRS_NEXT (exprs);
    }

    result = exprs;
    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCgetNthExprs(int n, node *args)
 *
 * @brief Given an N_exprs chain, return the nTH N_exprs node
 *        in the chain. If 0==n, return the argument.
 *        If n > chain length, return NULL.
 *
 * @param N_exprs chain
 *
 * @return N_exprs node
 ******************************************************************************/
node *
TCgetNthExprs (size_t n, node *exprs)
{
    size_t cnt;
    node *result = NULL;

    DBUG_ENTER ();

    for (cnt = 0; cnt < n; cnt++) {
        if (exprs == NULL) {
            DBUG_UNREACHABLE ("n > N_exprs chain length.");
        }

        exprs = EXPRS_NEXT (exprs);
    }

    result = exprs;
    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCputNthExprs(int n, node *args, node *val)
 *
 * @brief Given an N_exprs chain, replace the nTH N_exprs node
 *        in the chain by val, IN PLACE.
 *        If n>chain length, abort.
 *
 * @param N_exprs chain
 *
 * @return updated N_exprs chain
 ******************************************************************************/
node *
TCputNthExprs (size_t n, node *oldexprs, node *val)
{
    size_t cnt;
    node *exprs;

    DBUG_ENTER ();

    exprs = oldexprs;

    for (cnt = 0; cnt < n; cnt++) {
        if (exprs == NULL) {
            DBUG_UNREACHABLE ("n > N_exprs chain length.");
            break;
        }

        exprs = EXPRS_NEXT (exprs);
    }

    EXPRS_EXPR (exprs) = FREEdoFreeNode (EXPRS_EXPR (exprs));
    EXPRS_EXPR (exprs) = val;
    DBUG_RETURN (oldexprs);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCgetNthExprsExpr(int n, node *args)
 *
 * @brief Given an N_exprs chain, return the nTH EXPRS_EXPR
 *        in the chain. If 0==n, return the argument.
 *        If n > chain length, return NULL.
 *
 * @param N_exprs chain
 *
 * @return N_node of some sort
 ******************************************************************************/
node *
TCgetNthExprsExpr (size_t n, node *exprs)
{
    node *result = NULL;

    DBUG_ENTER ();

    exprs = TCgetNthExprs (n, exprs);

    if (exprs != NULL) {
        result = EXPRS_EXPR (exprs);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCtakeDropExprs(int takecount, int dropcount offset, node *exprs)
 * @brief Given an N_exprs chain, return items "dropcount + iota takecount",
 *        (or subset thereof, if the chain isn't long enough).
 *        I.e., take(takecount, drop (dropcount, exprs)).
 *        Both offset and cnt are assumed to be >=0, only because I'm lazy.
 *
 * @param dropcount: index into exprs of first element of result
 *        takecount:    number of elements in result
 *        exprs:  N_exprs chain
 *
 * @return: N_exprs chain
 ******************************************************************************/
node *
TCtakeDropExprs (int takecount, size_t dropcount, node *exprs)
{
    node *res = NULL;
    node *tail;

    DBUG_ENTER ();
    DBUG_ASSERT (takecount >= 0,
                 "TCtakeDropExprs take or drop count < 0");
    if (0 != takecount) {
        DBUG_ASSERT ((exprs != NULL) && (N_exprs == NODE_TYPE (exprs)),
                    "TCtakeDropExprs disappointed at not getting N_exprs");
        /* This does too much work, but I'm not sure of a nice way to fix it. */
        res = DUPdoDupTree (TCgetNthExprsNext (dropcount, exprs));  /* do drop */
        tail = TCgetNthExprsNext ((size_t)MATHmax (0, takecount - 1), res); /* do take */
        if ((NULL != tail) && NULL != EXPRS_NEXT (tail)) {
            FREEdoFreeTree (EXPRS_NEXT (tail));
            EXPRS_NEXT (tail) = NULL;
        }
    }
    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCfilterExprs( bool (*pred)( node *), node **exprs)
 * @brief Given an N_exprs chain and a predicate function, pred,
 *        split the N_exprs chain into two pieces, based on
 *        application of the predicate to each element of the
 *        N_exprs chain.
 *
 * @param
 *        exprs:  N_exprs chain
 *
 * @return: res: the N_exprs chain for those elements of exprs
 *               that satisfied the predicate.
 *          SIDE EFFECT: exprs has the satisfied elements REMOVED
 *               from it.
 *
 ******************************************************************************/
node *
TCfilterExprs (bool (*pred) (node *), node **exprs)
{
    node *res = NULL;
    node *tmp;

    DBUG_ENTER ();

    if (*exprs != NULL) {
        if (EXPRS_NEXT (*exprs) != NULL) {
            res = TCfilterExprs (pred, &(EXPRS_NEXT (*exprs)));
        }

        if (pred (EXPRS_EXPR (*exprs))) {
            tmp = EXPRS_NEXT (*exprs);
            EXPRS_NEXT (*exprs) = res;
            res = *exprs;
            *exprs = tmp;
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCfilterExprsArg( bool (*pred)( node *, node *), node **exprs)
 * @brief Given an N_exprs chain, a predicate function, pred,
 *        and an argument node to be passed to pred,
 *        split the N_exprs chain into two pieces, based on
 *        application of the predicate to each element of the
 *        N_exprs chain, using arg as the first argument of
 *        pred.
 *
 * @param
 *        pred: the predicate function
 *        exprs:  N_exprs chain
 *        arg: the first argument to the predicate function
 *
 * @return: res: the N_exprs chain for those elements of exprs
 *               that satisfied the predicate.
 *          SIDE EFFECT: exprs has the satisfied elements REMOVED
 *               from it.
 *
 ******************************************************************************/
node *
TCfilterExprsArg (bool (*pred) (node *, node *), node *arg, node **exprs)
{
    node *res = NULL;
    node *tmp;

    DBUG_ENTER ();

    if (*exprs != NULL) {
        if (EXPRS_NEXT (*exprs) != NULL) {
            res = TCfilterExprsArg (pred, arg, &(EXPRS_NEXT (*exprs)));
        }

        if (pred (arg, EXPRS_EXPR (*exprs))) {
            tmp = EXPRS_NEXT (*exprs);
            EXPRS_NEXT (*exprs) = res;
            res = *exprs;
            *exprs = tmp;
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCfilterAssignArg( bool (*pred)( node *, node *), node **assgn)
 * @brief Given an N_assign chain, a predicate function, pred,
 *        and an argument node to be passed to pred,
 *        split the N_assign chain into two pieces, based on
 *        application of the predicate to each element of the
 *        N_assign chain, using arg as the first argument of
 *        pred.
 *
 * @param
 *        pred: the predicate function, to be called as:
 *           pred( arg, assgn);
 *
 *        assgn:  N_assign chain
 *        arg: the first argument to the predicate function
 *
 * @return: res: the N_assign chain for those elements of assgn
 *               that satisfied the predicate.
 *          SIDE EFFECT: assgn has the satisfied elements REMOVED
 *               from it.
 *
 ******************************************************************************/
node *
TCfilterAssignArg (bool (*pred) (node *, node *), node *arg, node **assgn)
{
    node *res = NULL;
    node *tmp;

    DBUG_ENTER ();

    if (*assgn != NULL) {
        if (ASSIGN_NEXT (*assgn) != NULL) {
            res = TCfilterAssignArg (pred, arg, &(ASSIGN_NEXT (*assgn)));
        }

        if (pred (arg, *assgn)) {
            tmp = ASSIGN_NEXT (*assgn);
            ASSIGN_NEXT (*assgn) = res;
            res = *assgn;
            *assgn = tmp;
        }
    }

    DBUG_RETURN (res);
}

bool
TCfoldPredExprs (bool (*pred) (node *), node *exprs)
{
    bool res;

    DBUG_ENTER ();

    if (exprs == NULL) {
        res = TRUE;
    } else {
        res = (pred (EXPRS_EXPR (exprs)) && TCfoldPredExprs (pred, EXPRS_NEXT (exprs)));
    }

    DBUG_RETURN (res);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :
 ***/

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

node *
TCnodeBehindCast (node *arg_node)
{
    DBUG_ENTER ();
    while (N_cast == NODE_TYPE (arg_node)) {
        arg_node = CAST_EXPR (arg_node);
    }
    DBUG_RETURN (arg_node);
}

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
 ***  N_do :  *and*  N_while :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCmakeVector( ntype *basetype, node *aelems)
 *
 * description:
 *   Returns a vector.
 *
 *****************************************************************************/
node *
TCmakeVector (ntype *elemtype, node *aelems)
{
    DBUG_ENTER ();

    DBUG_RETURN (
      TBmakeArray (elemtype, SHcreateShape (1, TCcountExprs (aelems)), aelems));
}

/******************************************************************************
 *
 * function:
 *   node *TCmakeIntVector( node *aelems)
 *
 * description:
 *   Returns an integer vector.
 *
 *****************************************************************************/
node *
TCmakeIntVector (node *aelems)
{
    DBUG_ENTER ();

    DBUG_RETURN (
      TCmakeVector (TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)), aelems));
}

/******************************************************************************
 *
 * function:
 *   node *TCcreateIntVector( int length, int value, int step)
 *
 * description:
 *   Returns an integer vector of length elements, with starting value
 *   value, and incrementing by inc. Hence, to get the
 *   vector [ 42, 43, 44, 45], invoke this function with:
 *
 *     TCcreateIntVector( 4, 42, 1)
 *
 *****************************************************************************/
node *
TCcreateIntVector (int length, int value, int step)
{
    node *result = NULL;
    int d;
    int v;

    DBUG_ENTER ();

    v = value + ((length - 1) * step);

    for (d = 0; d < length; d++) {
        result = TBmakeExprs (TBmakeNum (v), result);
        v = v - step; /* list is built backwards */
    }

    result = TCmakeIntVector (result);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Given an N_array node that represents an integer vector, this
 *        function returns the value of its pos-th element.
 *
 * @param pos  index of element to return
 * @param vect N_array node
 *
 * @return value at that position
 ******************************************************************************/
int
TCgetIntVectorNthValue (size_t pos, node *vect)
{
    node *elem;

    DBUG_ENTER ();

    DBUG_ASSERT (SHgetDim (ARRAY_FRAMESHAPE (vect)) == 1, "argument not a vector");

    elem = TCgetNthExprsExpr (pos, ARRAY_AELEMS (vect));

    DBUG_ASSERT (elem != NULL, "vector too short!");
    DBUG_ASSERT (NODE_TYPE (elem) == N_num, "element not an int!");

    DBUG_RETURN (NUM_VAL (elem));
}

/******************************************************************************
 *
 * function:
 *   node *TCcreateZeroScalar( simpletype btype)
 *
 * description:
 *   Returns a scalar 0.
 *
 *****************************************************************************/
node *
TCcreateZeroScalar (simpletype btype)
{
    node *ret_node;

    DBUG_ENTER ();

    DBUG_ASSERT (btype != T_user, "unresolved user-type found");
    DBUG_ASSERT (btype != T_hidden, "hidden-type found");

    switch (btype) {
    case T_byte:
        ret_node = TBmakeNumbyte (0);
        break;
    case T_short:
        ret_node = TBmakeNumshort (0);
        break;
    case T_int:
        ret_node = TBmakeNum (0);
        break;
    case T_long:
        ret_node = TBmakeNumlong (0);
        break;
    case T_longlong:
        ret_node = TBmakeNumlonglong (0);
        break;
    case T_ubyte:
        ret_node = TBmakeNumubyte (0);
        break;
    case T_ushort:
        ret_node = TBmakeNumushort (0);
        break;
    case T_uint:
        ret_node = TBmakeNumuint (0);
        break;
    case T_ulong:
        ret_node = TBmakeNumulong (0);
        break;
    case T_ulonglong:
        ret_node = TBmakeNumulonglong (0);
        break;
    case T_float:
        ret_node = TBmakeFloat (0);
        break;
    case T_double:
        ret_node = TBmakeDouble (0);
        break;
    case T_bool:
        ret_node = TBmakeBool (0);
        break;
    case T_char:
        ret_node = TBmakeChar ('\0');
        break;
    default:
        ret_node = NULL;
        DBUG_UNREACHABLE ("unkown basetype found");
    }

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCcreateZeroVector( size_t length, simpletype btype)
 *
 * description:
 *   Returns an N_array node with 'length' components, each 0.
 *
 ******************************************************************************/

node *
TCcreateZeroVector (size_t length, simpletype btype)
{
    node *ret_node, *exprs_node;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (btype != T_user, "unresolved user-type found");
    DBUG_ASSERT (btype != T_hidden, "hidden-type found");

    exprs_node = NULL;
    for (i = 0; i < length; i++) {
        exprs_node = TBmakeExprs (TCcreateZeroScalar (btype), exprs_node);
    }

    ret_node
      = TCmakeVector (TYmakeAKS (TYmakeSimpleType (btype), SHmakeShape (0)), exprs_node);

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCcreateZeroNestedScalar( ntype btype)
 *
 * description:
 *   Returns a scalar 0.
 *
 *****************************************************************************/
node *
TCcreateZeroNestedScalar (ntype *btype)
{
    node *ret_node;

    DBUG_ENTER ();

    ret_node = TBmakeNested_init ();

    DBUG_RETURN (ret_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCcreateZeroNestedVector( int length, simpletype btype)
 *
 * description:
 *   Returns an N_array node with 'length' components, each 0.
 *
 ******************************************************************************/

node *
TCcreateZeroNestedVector (int length, ntype *btype)
{
    node *ret_node, *exprs_node;
    int i;

    DBUG_ENTER ();

    exprs_node = NULL;
    for (i = 0; i < length; i++) {
        exprs_node = TBmakeExprs (TCcreateZeroNestedScalar (btype), exprs_node);
    }

    ret_node = TCmakeVector (TYmakeAKS (btype, SHmakeShape (0)), exprs_node);

    DBUG_RETURN (ret_node);
}

/*****************************************************************************
 *
 * function:
 *   node *TCids2Exprs( ids *ids_arg)
 *
 * description:
 *   convert ids into N_exprs chain
 *
 *****************************************************************************/

node *
TCids2Exprs (node *ids_arg)
{
    node *exprs;

    DBUG_ENTER ();

    if (ids_arg != NULL) {
        exprs = TBmakeExprs (DUPdupIdsId (ids_arg), TCids2Exprs (IDS_NEXT (ids_arg)));
    } else {
        exprs = NULL;
    }

    DBUG_RETURN (exprs);
}

/*****************************************************************************
 *
 * function:
 *   node *TCids2ExprsNT( ids *ids_arg)
 *
 * description:
 *   convert ids into N_exprs chain
 *
 *****************************************************************************/

node *
TCids2ExprsNt (node *ids_arg)
{
    node *exprs;

    DBUG_ENTER ();

    if (ids_arg != NULL) {
        exprs = TBmakeExprs (DUPdupIdsIdNt (ids_arg), TCids2ExprsNt (IDS_NEXT (ids_arg)));
    } else {
        exprs = NULL;
    }

    DBUG_RETURN (exprs);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_id :
 ***/

/*--------------------------------------------------------------------------*/
node *
TCmakeIdCopyString (const char *str)
{
    node *result;
    DBUG_ENTER ();

    if (str == NULL) {
        str = "";
    }

    result = TBmakeId (NULL);

    ID_ICMTEXT (result) = STRcpy (str);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *TCmakeIdCopyStringNtNew( const char *str, ntype *type)
 *
 *   @brief  Create an id with a name tuple derived from str and type.
 *
 *****************************************************************************/
node *
TCmakeIdCopyStringNtNew (const char *str, ntype *type)
{
    node *result;

    DBUG_ENTER ();

    result = TCmakeIdCopyString (str);
    ID_NT_TAG (result) = NTUcreateNtTagFromNType (str, type);

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_ids :
 ***/

/*--------------------------------------------------------------------------*/

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateIdsChainFromExprs()
 *
 * @brief Returns an N_ids chain from, e.g., an ARRAY_AELEMS chain
 *        containing N_id nodes.
 *
 * @return created N_ids chain
 ******************************************************************************/
node *
TCcreateIdsChainFromExprs (node *arg_node)
{
    node *ids = NULL;

    DBUG_ENTER ();

    while (NULL != arg_node) {
        ids = TCappendIds (ids, TBmakeIds (ID_AVIS (EXPRS_EXPR (arg_node)), NULL));
        arg_node = EXPRS_NEXT (arg_node);
    }

    DBUG_RETURN (ids);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCcreateIdsChainFromAvises( int num_avises, ...)
 *
 * @brief Returns an N_ids chain containing nodes with
 *        the avis given by as varargs
 *
 * @return created N_ids chain
 ******************************************************************************/
node *
TCcreateIdsChainFromAvises (int num_avises, ...)
{
    va_list ap;
    int i;
    node *ids;

    DBUG_ENTER ();
    va_start (ap, num_avises);
    ids = NULL;
    for (i = 0; i < num_avises; i++) {
        ids = TCappendIds (ids, TBmakeIds (va_arg (ap, node *), NULL));
    }
    va_end (ap);
    DBUG_RETURN (ids);
}

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

bool
TCisPhiFun (node *id)
{

    bool result;

    DBUG_ENTER ();

    if ((AVIS_SSAASSIGN (ID_AVIS (id)) != NULL)
        && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (id)))) == N_funcond))
        result = TRUE;
    else
        result = FALSE;

    DBUG_RETURN (result);
}

node *
TCsetSSAAssignForIdsChain (node *ids, node *assign)
{
    DBUG_ENTER ();

    if (ids != NULL) {
        DBUG_ASSERT (NODE_TYPE (ids) == N_ids, "N_ids expected!");

        IDS_NEXT (ids) = TCsetSSAAssignForIdsChain (IDS_NEXT (ids), assign);

        AVIS_SSAASSIGN (IDS_AVIS (ids)) = assign;
    }

    DBUG_RETURN (ids);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCmakePrf1( prf prf, node *arg1)
 *   node *TCmakePrf2( prf prf, node *arg1, node *arg2)
 *   node *TCmakePrf3( prf prf, node *arg1, node *arg2, node *arg3)
 *   node *TCmakePrf4( prf prf, node *arg1, node *arg2, node *arg3, node *arg4)
 *
 * description:
 *   create N_prf node for primitive function application with 1, 2, or 3
 *   arguments, respectively.
 *
 ******************************************************************************/

node *
TCmakePrf1 (prf prf, node *arg1)
{
    node *res;

    DBUG_ENTER ();

    res = TBmakePrf (prf, TBmakeExprs (arg1, NULL));

    DBUG_RETURN (res);
}

node *
TCmakePrf2 (prf prf, node *arg1, node *arg2)
{
    node *res;

    DBUG_ENTER ();

    res = TBmakePrf (prf, TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)));

    DBUG_RETURN (res);
}

node *
TCmakePrf3 (prf prf, node *arg1, node *arg2, node *arg3)
{
    node *res;

    DBUG_ENTER ();

    res
      = TBmakePrf (prf, TBmakeExprs (arg1, TBmakeExprs (arg2, TBmakeExprs (arg3, NULL))));

    DBUG_RETURN (res);
}

node *
TCmakePrf4 (prf prf, node *arg1, node *arg2, node *arg3, node *arg4)
{
    node *res;

    DBUG_ENTER ();

    res = TBmakePrf (prf,
                     TBmakeExprs (arg1,
                                  TBmakeExprs (arg2,
                                               TBmakeExprs (arg3,
                                                            TBmakeExprs (arg4, NULL)))));

    DBUG_RETURN (res);
}

node *
TCmakePrf5 (prf prf, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5)
{
    node *res;

    DBUG_ENTER ();

    res = TBmakePrf (
      prf,
      TBmakeExprs (arg1,
                   TBmakeExprs (arg2,
                                TBmakeExprs (arg3,
                                             TBmakeExprs (arg4,
                                                          TBmakeExprs (arg5, NULL))))));

    DBUG_RETURN (res);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_ap :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCmakeAp1( node *fundef, node *arg1)
 *   node *TCmakeAp2( node *fundef, node *arg1, node *arg2)
 *   node *TCmakeAp3( node *fundef, node *arg1, node *arg2, node *arg3)
 *
 * description:
 *   create N_prf node for primitive function application with 1, 2, or 3
 *   arguments, respectively.
 *
 ******************************************************************************/

node *
TCmakeAp1 (node *fundef, node *arg1)
{
    node *res;

    DBUG_ENTER ();

    res = TBmakeAp (fundef, TBmakeExprs (arg1, NULL));

    DBUG_RETURN (res);
}
node *
TCmakeAp2 (node *fundef, node *arg1, node *arg2)
{
    node *res;

    DBUG_ENTER ();

    res = TBmakeAp (fundef, TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)));

    DBUG_RETURN (res);
}

node *
TCmakeAp3 (node *fundef, node *arg1, node *arg2, node *arg3)
{
    node *res;

    DBUG_ENTER ();

    res = TBmakeAp (fundef,
                    TBmakeExprs (arg1, TBmakeExprs (arg2, TBmakeExprs (arg3, NULL))));

    DBUG_RETURN (res);
}

node *
TCmakeSpap1 (namespace_t *ns, char *name, node *arg1)
{
    node *result;

    DBUG_ENTER ();

    result = TBmakeSpap (TBmakeSpid (ns, name), TBmakeExprs (arg1, NULL));

    DBUG_RETURN (result);
}

node *
TCmakeSpap2 (namespace_t *ns, char *name, node *arg1, node *arg2)
{
    node *result;

    DBUG_ENTER ();

    result
      = TBmakeSpap (TBmakeSpid (ns, name), TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)));

    DBUG_RETURN (result);
}

node *
TCmakeSpap3 (namespace_t *ns, char *name, node *arg1, node *arg2, node *arg3)
{
    node *result;

    DBUG_ENTER ();

    result
      = TBmakeSpap (TBmakeSpid (ns, name),
                    TBmakeExprs (arg1, TBmakeExprs (arg2, TBmakeExprs (arg3, NULL))));

    DBUG_RETURN (result);
}
/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :  *and*  N_ap :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_pragma :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_icm :
 ***/

/******************************************************************************
 *
 * function:
 *   node *TCmakeIcm0( char *name)
 *   node *TCmakeIcm?( char *name, node *arg1, ...)
 *
 * description:
 *   These functions generate complete ICM representations including arguments.
 *   Each function argument may be an arbitrary list of ICM arguments (i.e.,
 *   a single expression node (N_id, N_num, ...) or a N_exprs chain!)
 *   These are concatenated correctly.
 *
 ******************************************************************************/

node *
TCmakeIcm0 (const char *name)
{
    node *icm;

    DBUG_ENTER ();

    icm = TBmakeIcm (name, NULL);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm1 (const char *name, node *arg1)
{
    node *icm;

    DBUG_ENTER ();

    arg1 = TCcombineExprs (arg1, NULL);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm2 (const char *name, node *arg1, node *arg2)
{
    node *icm;

    DBUG_ENTER ();

    icm = TBmakeIcm (name, TCcombineExprs (arg1, TCcombineExprs (arg2, NULL)));

    DBUG_RETURN (icm);
}

node *
TCmakeIcm3 (const char *name, node *arg1, node *arg2, node *arg3)
{
    node *icm;

    DBUG_ENTER ();

    arg1 = TCcombineExprs (arg1, TCcombineExprs (arg2, TCcombineExprs (arg3, NULL)));
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm4 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4)
{
    node *icm;

    DBUG_ENTER ();

    arg4 = TCcombineExprs (arg4, NULL);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm5 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5)
{
    node *icm;

    DBUG_ENTER ();

    arg5 = TCcombineExprs (arg5, NULL);
    arg4 = TCcombineExprs (arg4, arg5);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm6 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
            node *arg6)
{
    node *icm;

    DBUG_ENTER ();

    arg6 = TCcombineExprs (arg6, NULL);
    arg5 = TCcombineExprs (arg5, arg6);
    arg4 = TCcombineExprs (arg4, arg5);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm7 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
            node *arg6, node *arg7)
{
    node *icm;

    DBUG_ENTER ();

    arg7 = TCcombineExprs (arg7, NULL);
    arg6 = TCcombineExprs (arg6, arg7);
    arg5 = TCcombineExprs (arg5, arg6);
    arg4 = TCcombineExprs (arg4, arg5);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

node *
TCmakeIcm8 (const char *name, node *arg1, node *arg2, node *arg3, node *arg4, node *arg5,
            node *arg6, node *arg7, node *arg8)
{
    node *icm;

    DBUG_ENTER ();

    arg8 = TCcombineExprs (arg8, NULL);
    arg7 = TCcombineExprs (arg7, arg8);
    arg6 = TCcombineExprs (arg6, arg7);
    arg5 = TCcombineExprs (arg5, arg6);
    arg4 = TCcombineExprs (arg4, arg5);
    arg3 = TCcombineExprs (arg3, arg4);
    arg2 = TCcombineExprs (arg2, arg3);
    arg1 = TCcombineExprs (arg1, arg2);
    icm = TBmakeIcm (name, arg1);

    DBUG_RETURN (icm);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_mt :   N_st :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_spmd :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_with :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_part:
 ***/

/** <!--********************************************************************-->
 *
 * @fn int TCcountParts( node *parts)
 *
 * @brief Counts the number of parts in a given chain of parts
 *
 * @param parts
 *
 * @return number of parts
 *
 *****************************************************************************/
size_t
TCcountParts (node *parts)
{
    size_t counter = 0;

    DBUG_ENTER ();

    DBUG_ASSERT (parts == NULL || NODE_TYPE (parts) == N_part,
                 "TCcountParts called with wrong node type.");

    while (parts != NULL) {
        counter += 1;
        parts = PART_NEXT (parts);
    }

    DBUG_RETURN (counter);
}

/** <!--********************************************************************-->
 *
 * @fn node * TCappendPart( node *parts1, node *parts2)
 *
 * @brief
 *
 * @param parts
 *
 * @return
 *
 *****************************************************************************/
node *
TCappendPart (node *parts1, node *parts2)
{
    node *current;

    DBUG_ENTER ();

    DBUG_ASSERT (parts1 == NULL || NODE_TYPE (parts1) == N_part,
                 "TCappendPart called with wrong node type.");

    DBUG_ASSERT (parts2 == NULL || NODE_TYPE (parts2) == N_part,
                 "TCappendPart called with wrong node type.");

    if (parts1 == NULL) {
        parts1 = parts2;
    } else {
        current = parts1;
        while (PART_NEXT (current) != NULL) {
            current = PART_NEXT (current);
        }
        PART_NEXT (current) = parts2;
    }
    DBUG_RETURN (parts1);
}

/** <!--********************************************************************-->
 *
 * @fn node * TCgetNthPart( node *parts, int n)
 *
 * @brief Return nth N_part of parts. Origin 0, i.e. n==0 --> first N_part
 *
 * @param parts - an N_part
 *
 * @return the Nth N_part of parts
 *
 *****************************************************************************/
node *
TCgetNthPart (node *parts, int n)
{
    node *z = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (parts == NULL || NODE_TYPE (parts) == N_part,
                 "called with wrong node type.");

    z = parts;
    while (n != 0) {
        z = PART_NEXT (z);
        n--;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool TCcontainsDefaultPartition( node *parts)
 *
 * @brief
 *
 * @param parts
 *
 * @return
 *
 *****************************************************************************/
bool
TCcontainsDefaultPartition (node *parts)
{
    DBUG_ENTER ();

    DBUG_ASSERT (parts == NULL || NODE_TYPE (parts) == N_part,
                 "TCcontainsDefaultPartition called with wrong node type.");

    while ((parts != NULL) && (NODE_TYPE (PART_GENERATOR (parts)) != N_default)) {
        parts = PART_NEXT (parts);
    }

    DBUG_RETURN (parts != NULL);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_code :
 ***/

/** <!--********************************************************************-->
 *
 * @fn node * TCappendCode( node *code1, node *code2)
 *
 * @brief Append one list of N_codes to another
 *
 * @param N_codes
 *
 * @return code1 ++ code2
 *
 *****************************************************************************/
node *
TCappendCode (node *code1, node *code2)
{
    node *current;

    DBUG_ENTER ();

    DBUG_ASSERT (code1 == NULL || NODE_TYPE (code1) == N_code, "Expected N_code");

    DBUG_ASSERT (code2 == NULL || NODE_TYPE (code2) == N_code, "Expected N_code");

    if (code1 == NULL) {
        code1 = code2;
    } else {
        current = code1;
        while (CODE_NEXT (current) != NULL) {
            current = CODE_NEXT (current);
        }
        CODE_NEXT (current) = code2;
    }
    DBUG_RETURN (code1);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_withop :
 ***/

size_t
TCcountWithops (node *withop)
{
    size_t counter = 0;

    DBUG_ENTER ();

    while (withop != NULL) {
        counter += 1;
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (counter);
}

size_t
TCcountWithopsEq (node *withop, nodetype eq)
{
    size_t counter = 0;

    DBUG_ENTER ();

    while (withop != NULL) {
        if (NODE_TYPE (withop) == eq) {
            counter += 1;
        }
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (counter);
}

size_t
TCcountWithopsNeq (node *withop, nodetype neq)
{
    size_t counter = 0;

    DBUG_ENTER ();

    while (withop != NULL) {
        if (NODE_TYPE (withop) != neq) {
            counter += 1;
        }
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (counter);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_with2 :
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_wlsegs :
 ***/

size_t
TCcountWlseg (node *wlseg)
{
    size_t counter = 0;

    DBUG_ENTER ();

    while (wlseg != NULL) {
        counter += 1;
        wlseg = WLSEG_NEXT (wlseg);
    }

    DBUG_RETURN (counter);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_str :
 ***/

node *
TCmakeStrCopy (const char *str)
{
    DBUG_ENTER ();

    DBUG_RETURN (TBmakeStr (STRcpy (str)));
}

/*--------------------------------------------------------------------------*/

/***
 *** N_set
 ***/

/** <!-- ****************************************************************** -->
 * @brief Appends the linked lists links1 and links2. Note that this
 *        function does not preserve set properties!
 *
 * @param links1 N_set chain
 * @param links2 N_set chain
 *
 * @return appended N_set chains
 ******************************************************************************/
node *
TCappendSet (node *links1, node *links2)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((links1 == NULL) || (NODE_TYPE (links1) == N_set)),
                 "First argument of TCappendSet() has wrong node type.");
    DBUG_ASSERT (((links2 == NULL) || (NODE_TYPE (links2) == N_set)),
                 "Second argument of TCappendSet() has wrong node type.");

    APPEND (ret, node *, SET, links1, links2);

    DBUG_RETURN (ret);
}

/** <!-- ****************************************************************** -->
 * @fn node *TCdropSet( int drop, node *set)
 *
 * @brief Drops drop elements from the set. If drop is negative, the
 *        elements are dropped from the tail.
 *
 *        NOTE: set is updated destructively!
 *
 * @param drop number of elements to drop
 * @param set  set to drop from
 *
 * @return modified set
 ******************************************************************************/

static node *
DropSetHelper (int *drop, node *set)
{
    bool tagged = FALSE;

    DBUG_ENTER ();

    DBUG_ASSERT (((set != NULL) || (*drop <= 0)),
                 "cannot drop more elements from list than elements in list!");

    if (set != NULL) {
        if (*drop > 0) {
            tagged = TRUE;
            (*drop)--;
        }

        if (*drop != 0) {
            SET_NEXT (set) = DropSetHelper (drop, SET_NEXT (set));
        }

        if (*drop < 0) {
            tagged = TRUE;
            (*drop)++;
        }
    }

    if (tagged) {
        set = FREEdoFreeNode (set);
    }

    DBUG_RETURN (set);
}

node *
TCdropSet (int drop, node *set)
{
    DBUG_ENTER ();

    set = DropSetHelper (&drop, set);

    DBUG_ASSERT (drop == 0, "Cannot drop more elements from end of list than elements in "
                            "list!");

    DBUG_RETURN (set);
}

/** <!-- ****************************************************************** -->
 * @fn int TCsetAdd( node **links, node *link)
 *
 * @brief Adds the element link to links if the set does not yet contain
 *        link.
 *
 *        NOTE: This is implemented as an append to the linked list. This
 *              fact is relied upon in parts of the compiler that were
 *              written when SET nodes were still called LINKEDLIST.
 *
 * @param *links the set
 * @param link   node to insert
 *
 * @return number of new nodes inserted
 ******************************************************************************/
int
TCsetAdd (node **links, node *link)
{
    int result = 0;

    DBUG_ENTER ();

    if (*links == NULL) {
        /*
         * it has not been found so far, so append it
         */
        *links = TBmakeSet (link, NULL);
        result = 1;
    } else if (SET_MEMBER (*links) != link) {
        /*
         * its not the current one, so go on
         */
        result = TCsetAdd (&SET_NEXT (*links), link);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn int TCsetRemove( node **links, node *link)
 *
 * @brief Removes the element link from links if the set does contain link.
 *
 *        NOTE: This is implementation assumes links is indeed a set, i.e.,
 *              link is contained at most once!
 *
 * @param *links the set
 * @param link   node to remove
 *
 * @return number of new nodes removed
 ******************************************************************************/
int
TCsetRemove (node **links, node *link)
{
    int result = 0;

    DBUG_ENTER ();

    if (*links != NULL) {
        if (SET_MEMBER (*links) == link) {
            *links = FREEdoFreeNode (*links);
            result = 1;
        } else {
            result = TCsetRemove (&SET_NEXT (*links), link);
        }
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn int TCsetUnion( node **links, node *add)
 *
 * @brief Merges all elements of add into links. Note that the links structure
 *        is updated destructively.
 *
 * @param *links set to merge and update
 * @param add    set to merge
 *
 * @return Number of elements added to links
 ******************************************************************************/
int
TCsetUnion (node **links, node *add)
{
    int result = 0;

    DBUG_ENTER ();

    while (add != NULL) {
        result += TCsetAdd (links, SET_MEMBER (add));
        add = SET_NEXT (add);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn bool TCsetContains( node *set, node *link)
 *
 * @brief Checks whether link is contained in set
 *
 * @param set   the set to search in
 * @param link  the node to search for
 *
 * @return TRUE iff link is member of set
 ******************************************************************************/
bool
TCsetContains (node *set, node *link)
{
    bool result = FALSE;

    DBUG_ENTER ();

    while ((set != NULL) && (!result)) {
        DBUG_ASSERT (NODE_TYPE (set) == N_set,
                     "called TCsetContains with non N_set node!");

        result = (SET_MEMBER (set) == link);

        set = SET_NEXT (set);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn bool TCsetIsSubset( node *super, node *sub)
 *
 * @brief Checks whether all elements of sub are contained in super
 *
 * @param super set to check against
 * @param sub   set to check for being subset
 *
 * @return TRUE iff sub is subset of super
 ******************************************************************************/
bool
TCsetIsSubset (node *super, node *sub)
{
    bool result = TRUE;

    DBUG_ENTER ();

    while ((sub != NULL) && result) {
        DBUG_ASSERT (NODE_TYPE (sub) == N_set,
                     "called TCsetIsSubset with non N_set node!");

        result = result && TCsetContains (super, SET_MEMBER (sub));

        sub = SET_NEXT (sub);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn void TCprintSet( node *set)
 *
 * @brief Print set members on stderr
 *        This was written for debugging N_set stuff.
 *
 * @param an N_set
 *
 * @return void
 *         side effect is to print each N_ids chain in the set.
 *         E.g.:
 *           [ i, j ]
 *           [ k, ]
 *
 ******************************************************************************/
void
TCprintSet (node *set)
{
    node *ids;

    DBUG_ENTER ();

    while (set != NULL) {
        ids = SET_MEMBER (set);
        fprintf (stderr, "[ ");
        while (NULL != ids) {
            fprintf (stderr, "%s, ", AVIS_NAME (IDS_AVIS (ids)));
            ids = IDS_NEXT (ids);
        }
        fprintf (stderr, " ]\n");
        set = SET_NEXT (set);
    }

    DBUG_RETURN ();
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_error :
 ***/

node *
TCappendError (node *chain, node *item)
{
    node *ret;

    DBUG_ENTER ();

    APPEND (ret, node *, IDS, chain, item);

    DBUG_RETURN (ret);
}

/*--------------------------------------------------------------------------*/

/***
 ***  N_range :
 ***/

/** <!-- ****************************************************************** -->
 * @brief Appends the second argument to the chain of ranges given by the
 *        first argument.
 *
 * @param range_chain chain of range nodes to be appended to
 * @param range range nodes to be appended
 *
 * @return combined chain
 ******************************************************************************/
node *
TCappendRange (node *range_chain, node *range)
{
    node *ret;

    DBUG_ENTER ();

    DBUG_ASSERT (((range_chain == NULL) || (NODE_TYPE (range_chain) == N_range)),
                 "First argument of AppendRange() has wrong node type.");
    DBUG_ASSERT (((range == NULL) || (NODE_TYPE (range) == N_range)),
                 "Second argument of AppendRange() has wrong node type.");

    APPEND (ret, node *, RANGE, range_chain, range);

    DBUG_RETURN (ret);
}

size_t
TCcountRanges (node *range)
{
    size_t counter = 0;

    DBUG_ENTER ();

    while (range != NULL) {
        counter += 1;
        range = RANGE_NEXT (range);
    }

    DBUG_RETURN (counter);
}

/** <!-- ****************************************************************** -->
 * @brief Searches fundef vardec and args chain for a vardec name.
 *
 * @param
 *
 * @return N_vardec or N_args pointer, or NULL
 *
 ******************************************************************************/
node *
TCfindVardec_Name (char *name, node *fundef)
{
    node *v;
    node *curv = NULL;
    bool b = FALSE;

    DBUG_ENTER ();

    /* Search vardec chain */
    v = FUNDEF_VARDECS (fundef);
    if (NULL != v) {
        while ((NULL != v) && (!b)) {
            curv = v;
            b = STReq (name, AVIS_NAME (VARDEC_AVIS (curv)));
            v = b ? v : VARDEC_NEXT (v);
        }
    }

    /* Search args chain */
    if (!b) {
        v = FUNDEF_ARGS (fundef);
        while ((NULL != v) && (!b)) {
            curv = v;
            b = STReq (name, AVIS_NAME (ARG_AVIS (curv)));
            v = b ? v : ARG_NEXT (v);
        }
    }

    curv = b ? curv : NULL;

    DBUG_RETURN (curv);
}

/** <!-- ****************************************************************** -->
 * @brief Return true if argument is Scalar constant node
 *
 * @param
 *
 * @return
 ******************************************************************************/
bool
TCisScalar (node *arg_node)
{
    bool res = FALSE;
    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_num:
    case N_numbyte:
    case N_numint:
    case N_numlong:
    case N_numlonglong:
    case N_nums:
    case N_numshort:
    case N_numubyte:
    case N_numuint:
    case N_numulong:
    case N_numulonglong:
    case N_numushort:
    case N_double:
    case N_float:
        res = TRUE;
        break;
    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @brief Predicate to return true if basetype is signed
 *
 * @param  a simple type
 *
 * @return true if argument is signed
 *
 * @note: Perhaps should allow uchar here, too.
 *
 ******************************************************************************/
bool
TCisSignedType (ntype *typ)
{
    bool res = FALSE;
    simpletype styp;

    DBUG_ENTER ();

    styp = TYgetSimpleType (TYgetScalar (typ));

    switch (styp) {
    case T_int:
    case T_byte:
    case T_long:
    case T_longlong:
    case T_short:
    case T_double:
    case T_float:
        res = TRUE;
        break;
    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
