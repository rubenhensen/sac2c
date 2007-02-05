/*
 *
 * $Id$
 *
 */

#include "wl_bounds.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "DupTree.h"
#include "NameTuplesUtils.h"
#include "internal_lib.h"
#include "str.h"
#include "new_types.h"

/******************************************************************************
 *
 *  Modul: wl_bounds
 *
 *  Description:
 *
 *    The index bounds for a with-loop in internal representation (N_Nwith2)
 *    are stored in the AST as values of type 'int' (N_WLseg, N_WLblock,
 *    N_WLublock, N_WLstride, N_WLgrid) or as N_id/N_num nodes (N_WLsegVar,
 *    N_WLstrideVar, N_WLgridVar).
 *
 *    This module provides functions for handling all the different
 *    respresentations of index bounds uniformly.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * Function:
 *   bool WLBnameOrValCheckConsistency( const char *name, int val)
 *
 * Description:
 *   Checks whether  (name != NULL) <-> (val == IDX_OTHER)  is hold.
 *
 ******************************************************************************/

bool
WLBnameOrValCheckConsistency (const char *name, int val)
{
    bool res;

    DBUG_ENTER ("NameOrVal");

    res = (((name != NULL) && (val == IDX_OTHER))
           || ((name == NULL) && (val != IDX_OTHER)));

    DBUG_ASSERT ((res), "Consistency check for NameOrVal failed!");

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   bool WLBnameOrValIsInt( char *name, int val)
 *
 * Description:
 *   Returns TRUE iff  (val != IDX_OTHER).
 *
 * Note:
 *   (NodeOrInt_IsInt => NameOrVal_IsInt)  is hold
 *   (NameOrVal_IsInt <= NodeOrInt_IsInt)  is *not* hold!!
 *
 ******************************************************************************/

bool
WLBnameOrValIsInt (const char *name, int val)
{
    bool ret;

    DBUG_ENTER ("NodeOrVal_IsInt");

    WLBnameOrValCheckConsistency (name, val);

    ret = (val != IDX_OTHER);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool WLBnodeOrIntIsInt( nodetype nt, void *node_or_int)
 *
 * Description:
 *   Returns TRUE iff  *node_or_int  has the type 'int'.
 *
 ******************************************************************************/

bool
WLBnodeOrIntIsInt (nodetype nt, void *node_or_int)
{
    bool ret;

    DBUG_ENTER ("NodeOrInt_IsInt");

    switch (nt) {
    case N_wlsegvar:
    case N_wlstridevar:
    case N_wlgridvar:
        ret = FALSE;
        break;

    case N_wlseg:
    case N_wlblock:
    case N_wlublock:
    case N_wlstride:
    case N_wlgrid:
        ret = TRUE;
        break;

    default:
        DBUG_ASSERT ((0), "wrong node type found!");
        ret = FALSE;
        break;
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   void WLBnodeOrIntGetNameOrVal( char **ret_name, int *ret_val,
 *                                nodetype nt, void *node_or_int)
 *
 * Description:
 *   Extracts the name or value from the given node-or-int data.
 *   If 'node_or_int' represents an 'int' value or N_id node, *ret_name is
 *   set to NULL and *ret_val is set to the found 'int' value.
 *   If 'node_or_int' represents a N_id node, *ret_name is set to the found
 *   name and *ret_val is set to IDX_OTHER.
 *
 ******************************************************************************/

void
WLBnodeOrIntGetNameOrVal (char **ret_name, int *ret_val, nodetype nt, void *node_or_int)
{
    DBUG_ENTER ("NodeOrInt_GetNameOrVal");

    DBUG_ASSERT ((node_or_int != NULL), "no address found!");

    if (WLBnodeOrIntIsInt (nt, node_or_int)) {
        if (ret_name != NULL) {
            (*ret_name) = NULL;
        }
        if (ret_val != NULL) {
            (*ret_val) = *((int *)node_or_int);
        }
    } else {
        if (NODE_TYPE ((*((node **)node_or_int))) == N_id) {
            if (ret_name != NULL) {
                (*ret_name) = ID_NAME ((*((node **)node_or_int)));
            }
            if (ret_val != NULL) {
                (*ret_val) = IDX_OTHER;
            }
        } else {
            if (ret_name != NULL) {
                (*ret_name) = NULL;
            }
            if (ret_val != NULL) {
                (*ret_val) = NUM_VAL ((*((node **)node_or_int)));
            }
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void WLBnodeOrIntSetNodeOrInt( nodetype ret_nt, void *ret_node_or_int,
 *                                  nodetype nt, void *node_or_int)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
WLBnodeOrIntSetNodeOrInt (nodetype ret_nt, void *ret_node_or_int, nodetype nt,
                          void *node_or_int)
{
    int val;

    DBUG_ENTER ("WLBnNodeOrIntSetNodeOrInt");

    DBUG_ASSERT ((ret_node_or_int != NULL), "no return address found!");

    if (WLBnodeOrIntIsInt (ret_nt, ret_node_or_int)) {
        WLBnodeOrIntGetNameOrVal (NULL, &val, nt, node_or_int);
        (*((int *)ret_node_or_int)) = val;
    } else {
        FREEdoFreeTree ((*((node **)ret_node_or_int)));
        (*((node **)ret_node_or_int)) = WLBnodeOrIntMakeNode (nt, node_or_int);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *WLBnodeOrIntMakeNode( nodetype nt, void *node_or_int)
 *
 * Description:
 *   Generates a node containing the parameter of a N_wlstride(Var) or
 *   N_wlgrid(Var) node.
 *
 * Remark:
 *   If 'node_or_int' represents a node it is duplicated in order to get
 *   correct back-references!
 *
 ******************************************************************************/

node *
WLBnodeOrIntMakeNode (nodetype nt, void *node_or_int)
{
    node *ret;

    DBUG_ENTER ("WLBnodeOrIntMakeNode");

    if (WLBnodeOrIntIsInt (nt, node_or_int)) {
        ret = TBmakeNum (*((int *)node_or_int));
    } else {
        ret = DUPdoDupNode (*((node **)node_or_int));
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   node *WLBnodeOrIntMakeIndex( nodetype nt, void *node_or_int,
 *                                int dim, ids *wl_ids)
 *
 * Description:
 *   Converts the parameter of a N_wlstride(Var) or N_wlgrid(Var) node into
 *   an index.
 *   If ('name' != NULL) the function returns a new N_id/N_icm node containing
 *   the correct index selection ('name' if scalar, 'name[dim]' otherwise).
 *   If ('name' == NULL) and ('val' == IDX_SHAPE) the functions returns a new
 *   N_id/N_icm node containing the shape of the current with-loop.
 *   If ('name' == NULL) and ('val' != IDX_OTHER) the function returns a new
 *   N_num/N_id node containing the value of 'val'.
 *
 ******************************************************************************/

node *
WLBnodeOrIntMakeIndex (nodetype nt, void *node_or_int, int dim, node *wl_ids)
{
    node *index;
    char *name;
    int val;
    int sel_dim;

    DBUG_ENTER ("WLBnodeOrIntMakeIndex");

    WLBnodeOrIntGetNameOrVal (&name, &val, nt, node_or_int);

    if (WLBnameOrValIsInt (name, val)) {
        if (val == IDX_SHAPE) {
            index = TCmakeIcm2 ("ND_A_SHAPE", DUPdupIdsIdNt (wl_ids), TBmakeNum (dim));
        } else {
            index = TBmakeNum (val);
        }
    } else {

        DBUG_ASSERT ((ID_DECL ((*((node **)node_or_int))) != NULL),
                     "no vardec/decl found!");

        /**
         * As we assume that this code is relevant for with2 only(!!),
         * we can now rely on WLPG to make bounds structural constants
         * As a consequence of that, we know, that we just need to select
         * the scalar value!
         * However, I left the original commented out in case
         * this is used in a different context too.
         */
#if 0
    sel_dim = (ID_DIM( (*((node **) node_or_int))) == SCALAR) ? 0 : dim;
#else
        sel_dim = 0;
#endif

        index = TCmakeIcm2 ("ND_READ", DUPdupIdNt ((*((node **)node_or_int))),
                            TBmakeNum (sel_dim));
    }

    DBUG_RETURN (index);
}

/******************************************************************************
 *
 * Function:
 *   bool WLBNameOrValEq( const char *name1, int val1,
 *                        const char *name2, int val2,
 *                        int shape)
 *
 * Description:
 *   Compares two parameters of N_wlstride(Var) or N_wlgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) 'shape' must equal IDX_SHAPE.
 *
 ******************************************************************************/

bool
WLBnameOrValEq (const char *name1, int val1, const char *name2, int val2, int shape)
{
    bool ret;

    DBUG_ENTER ("WLBnameOrValEq");

    DBUG_ASSERT (((shape >= 0) || (shape == IDX_SHAPE)), "illegal shape found!");

    if (WLBnameOrValIsInt (name1, val1) && WLBnameOrValIsInt (name2, val2)) {
        if (val1 == IDX_SHAPE) {
            /*
             * here we deal with the situation
             *   val1 == shape == 16, val == IDX_SHAPE
             */
            val1 = IDX_SHAPE;
        }
        if (val2 == IDX_SHAPE) {
            val2 = IDX_SHAPE;
        }

        ret = (val1 == val2);
    } else if ((name1 != NULL) && (name2 != NULL)) {
        ret = (!STReq (name1, name2));
    } else {
        ret = FALSE;
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool WLBnodeOrIntEq( nodetype nt1, void *node_or_int1,
 *                        nodetype nt2, void *node_or_int2,
 *                        int shape)
 *
 * Description:
 *   Compares two parameters of N_wlstride(Var) or N_wlgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) it must be set to IDX_SHAPE.
 *
 ******************************************************************************/

bool
WLBnodeOrIntEq (nodetype nt1, void *node_or_int1, nodetype nt2, void *node_or_int2,
                int shape)
{
    char *name1, *name2;
    int val1, val2;
    bool ret;

    DBUG_ENTER ("WLBnodeOrIntEq");

    WLBnodeOrIntGetNameOrVal (&name1, &val1, nt1, node_or_int1);
    WLBnodeOrIntGetNameOrVal (&name2, &val2, nt2, node_or_int2);
    ret = WLBnameOrValEq (name1, val1, name2, val2, shape);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool WLBnodeOrIntIntEq( nodetype nt1, void *node_or_int1,
 *                           int val2,
 *                           int shape)
 *
 * Description:
 *   Compares a parameter of a N_wlstride(Var) or N_wlgrid(Var) node with
 *   an integer value.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) it must be set to IDX_SHAPE.
 *
 ******************************************************************************/

bool
WLBnodeOrIntIntEq (nodetype nt1, void *node_or_int1, int val2, int shape)
{
    char *name1;
    int val1;
    bool ret;

    DBUG_ENTER ("WLBnodeOrIntIntEq");

    WLBnodeOrIntGetNameOrVal (&name1, &val1, nt1, node_or_int1);
    ret = WLBnameOrValEq (name1, val1, NULL, val2, shape);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool WLBnodeOrIntStrEq( nodetype nt1, void *node_or_int1,
 *                           const char *name2,
 *                           int shape)
 *
 * Description:
 *   Compares a parameter of a N_wlstride(Var) or N_wlgrid(Var) node with
 *   a string.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) it must be set to IDX_SHAPE.
 *
 ******************************************************************************/

bool
WLBnodeOrIntStrEq (nodetype nt1, void *node_or_int1, const char *name2, int shape)
{
    char *name1;
    int val1;
    bool ret;

    DBUG_ENTER ("WLBnodeOrIntStrEq");

    WLBnodeOrIntGetNameOrVal (&name1, &val1, nt1, node_or_int1);
    ret = WLBnameOrValEq (name1, val1, name2, IDX_OTHER, shape);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool WLBnameOrValLe( char *name1, int val1,
 *                        char *name2, int val2,
 *                        int shape)
 *
 * Description:
 *   Compares two parameters of N_wlstride(Var) or N_wlgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) 'shape' must equal IDX_SHAPE.
 *
 ******************************************************************************/

bool
WLBnameOrValLe (const char *name1, int val1, const char *name2, int val2, int shape)
{
    bool ret;

    DBUG_ENTER ("WLBnameOrValLe");

    ret = WLBnameOrValEq (name1, val1, name2, val2, shape);
    if ((val1 == 0) || (val2 == IDX_SHAPE) || (val2 == shape)
        || ((val2 > 0) && (val1 > 0) && (val1 < val2))) {
        ret = TRUE;
    }

    DBUG_PRINT ("NodeOrInt",
                ("[ %s, %i ] %s<= [ %s, %i ]", (name1 == NULL) ? "NULL" : name1, val1,
                 ret ? "" : "!", (name2 == NULL) ? "NULL" : name2, val2));

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool WLBnodeOrIntLe( nodetype nt1, void *node_or_int1,
 *                        nodetype nt2, void *node_or_int2,
 *                        int shape)
 *
 * Description:
 *   Compares two parameters of N_wlstride(Var) or N_wlgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) 'shape' must equal IDX_SHAPE.
 *
 ******************************************************************************/

bool
WLBnodeOrIntLe (nodetype nt1, void *node_or_int1, nodetype nt2, void *node_or_int2,
                int shape)
{
    char *name1, *name2;
    int val1, val2;
    bool ret;

    DBUG_ENTER ("WLBnodeOrIntLe");

    WLBnodeOrIntGetNameOrVal (&name1, &val1, nt1, node_or_int1);
    WLBnodeOrIntGetNameOrVal (&name2, &val2, nt2, node_or_int2);
    ret = WLBnameOrValLe (name1, val1, name2, val2, shape);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   void WLBnodeOrIntPrint( FILE *handle,
 *                           nodetype nt, void *node_or_int,
 *                           int dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
WLBnodeOrIntPrint (FILE *handle, nodetype nt, void *node_or_int, int dim)
{
    char *name;
    int val;

    DBUG_ENTER ("WLBnodeOrIntPrint");

    WLBnodeOrIntGetNameOrVal (&name, &val, nt, node_or_int);

    if (WLBnameOrValIsInt (name, val)) {
        if (val == IDX_OTHER) {
            fprintf (handle, "?");
        } else if (val == IDX_SHAPE) {
            fprintf (handle, ".");
        } else {
            /**
             * As we assume that this code is relevant for with2 only(!!),
             * we can now rely on WLPG to make bounds structural constants
             * As a consequence of that, we know, that we just need to select
             * the scalar value!
             */
            fprintf (handle, "%i", val);
        }
    } else {
        node *n = *((node **)node_or_int);

        if (AVIS_TYPE (ID_AVIS (n)) != NULL) {
            fprintf (handle, "%s", name);
        } else {
            DBUG_ASSERT ((ID_DECL (n) != NULL), "no vardec found!");

            /**
             * As we assume that this code is relevant for with2 only(!!),
             * we can now rely on WLPG to make bounds structural constants
             * As a consequence of that, we know, that we just need to select
             * the scalar value!
             */
            fprintf (handle, "%s", name);
        }
    }

    DBUG_VOID_RETURN;
}
