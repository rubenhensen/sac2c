/*
 *
 * $Log$
 * Revision 1.3  2001/04/02 16:01:56  dkr
 * NodeOrInt_MakeIndex modified
 * NameOrVal_MakeIndex removed
 * NodeOrInt_Print added
 *
 * Revision 1.2  2001/04/02 11:41:14  dkr
 * includes added
 *
 * Revision 1.1  2001/03/30 14:07:28  dkr
 * Initial revision
 *
 */

#include "wl_bounds.h"

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

#include "free.h"
#include "DupTree.h"

/******************************************************************************
 *
 * Function:
 *   bool NameOrVal_CheckConsistency( char *name, int val)
 *
 * Description:
 *   Checks whether  (name != NULL) <-> (val == IDX_OTHER)  is hold.
 *
 ******************************************************************************/

bool
NameOrVal_CheckConsistency (char *name, int val)
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
 *   bool NameOrVal_IsInt( char *name, int val)
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
NameOrVal_IsInt (char *name, int val)
{
    bool ret;

    DBUG_ENTER ("NodeOrVal_IsInt");

    NameOrVal_CheckConsistency (name, val);

    ret = (val != IDX_OTHER);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool NodeOrInt_IsInt( nodetype nt, void *node_or_int)
 *
 * Description:
 *   Returns TRUE iff  *node_or_int  has the type 'int'.
 *
 ******************************************************************************/

bool
NodeOrInt_IsInt (nodetype nt, void *node_or_int)
{
    bool ret;

    DBUG_ENTER ("NodeOrInt_IsInt");

    switch (nt) {
    case N_WLsegVar:
    case N_WLstrideVar:
    case N_WLgridVar:
        ret = FALSE;
        break;

    case N_WLseg:
    case N_WLblock:
    case N_WLublock:
    case N_WLstride:
    case N_WLgrid:
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
 *   void NodeOrInt_GetNameOrVal( char **ret_name, int *ret_val,
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
NodeOrInt_GetNameOrVal (char **ret_name, int *ret_val, nodetype nt, void *node_or_int)
{
    DBUG_ENTER ("NodeOrInt_GetNameOrVal");

    DBUG_ASSERT ((node_or_int != NULL), "no address found!");

    if (NodeOrInt_IsInt (nt, node_or_int)) {
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
 *   void NameOrVal_SetNodeOrInt( nodetype ret_nt, void *ret_node_or_int,
 *                                char *name, int val)
 *
 * Description:
 *   Stores the given name or value at the given node-or-int pointer.
 *
 ******************************************************************************/

void
NameOrVal_SetNodeOrInt (nodetype ret_nt, void *ret_node_or_int, char *name, int val)
{
    DBUG_ENTER ("NameOrVal_SetNodeOrInt");

    DBUG_ASSERT ((ret_node_or_int != NULL), "no return address found!");

    if (NodeOrInt_IsInt (ret_nt, ret_node_or_int)) {
        (*((int *)ret_node_or_int)) = val;
    } else {
        FreeTree ((*((node **)ret_node_or_int)));
        (*((node **)ret_node_or_int)) = NameOrVal_MakeNode (name, val);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void NodeOrInt_SetNodeOrInt( nodetype ret_nt, void *ret_node_or_int,
 *                                nodetype nt, void *node_or_int)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
NodeOrInt_SetNodeOrInt (nodetype ret_nt, void *ret_node_or_int, nodetype nt,
                        void *node_or_int)
{
    int val;

    DBUG_ENTER ("NodeOrInt_SetNodeOrInt");

    DBUG_ASSERT ((ret_node_or_int != NULL), "no return address found!");

    if (NodeOrInt_IsInt (ret_nt, ret_node_or_int)) {
        NodeOrInt_GetNameOrVal (NULL, &val, nt, node_or_int);
        (*((int *)ret_node_or_int)) = val;
    } else {
        FreeTree ((*((node **)ret_node_or_int)));
        (*((node **)ret_node_or_int)) = NodeOrInt_MakeNode (nt, node_or_int);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *NameOrVal_MakeNode( char *name, int val)
 *
 * Description:
 *   Generates a node containing the parameter of a N_WLstride(Var) or
 *   N_WLgrid(Var) node.
 *
 * CAUTION:
 *   This function creates a *fresh* node without any back-references!
 *   You might want to use NodeOrInt_MakeNode() instead!
 *
 ******************************************************************************/

node *
NameOrVal_MakeNode (char *name, int val)
{
    node *ret;

    DBUG_ENTER ("NameOrVal_MakeNode");

    if (NameOrVal_IsInt (name, val)) {
        ret = MakeNum (val);
    } else {
        ret = MakeId_Copy (name);
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   node *NodeOrInt_MakeNode( nodetype nt, void *node_or_int)
 *
 * Description:
 *   Generates a node containing the parameter of a N_WLstride(Var) or
 *   N_WLgrid(Var) node.
 *
 * Remark:
 *   If 'node_or_int' represents a node it is duplicated in order to get
 *   correct back-references!
 *
 ******************************************************************************/

node *
NodeOrInt_MakeNode (nodetype nt, void *node_or_int)
{
    node *ret;

    DBUG_ENTER ("NodeOrInt_MakeNode");

    if (NodeOrInt_IsInt (nt, node_or_int)) {
        ret = MakeNum (*((int *)node_or_int));
    } else {
        ret = DupNode (*((node **)node_or_int));
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   node *NodeOrInt_MakeIndex( nodetype nt, void *node_or_int,
 *                              int dim, char *wl_name,
 *                              bool no_num, bool no_icm)
 *
 * Description:
 *   Converts the parameter of a N_WLstride(Var) or N_WLgrid(Var) node into
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
NodeOrInt_MakeIndex (nodetype nt, void *node_or_int, int dim, char *wl_name, bool no_num,
                     bool no_icm)
{
    node *index;
    char *str;
    char *name;
    int val;

    DBUG_ENTER ("NodeOrInt_MakeIndex");

    NodeOrInt_GetNameOrVal (&name, &val, nt, node_or_int);

    if (NameOrVal_IsInt (name, val)) {
        if (val == IDX_SHAPE) {
            if (no_icm) {
                str = (char *)MALLOC ((strlen (wl_name) + 40) * sizeof (char));
                sprintf (str, "SAC_ND_A_SHAPE( %s, %d)", wl_name, dim);
                index = MakeId (str, NULL, ST_regular);
            } else {
                index = MakeIcm2 ("ND_A_SHAPE", MakeId_Copy (wl_name), MakeNum (dim));
            }
        } else {
            if (no_num) {
                index = MakeId_Num (val);
            } else {
                index = MakeNum (val);
            }
        }
    } else {
        DBUG_ASSERT ((ID_VARDEC ((*((node **)node_or_int))) != NULL), "no vardec found!");

        if (ID_DIM ((*((node **)node_or_int))) == SCALAR) {
            index = DupNode (*((node **)node_or_int));
        } else {
            if (no_icm) {
                str = (char *)MALLOC ((strlen (name) + 43) * sizeof (char));
                sprintf (str, "SAC_ND_READ_ARRAY( %s, %d)", name, dim);
                index = MakeId (str, NULL, ST_regular);
            } else {
                index = MakeIcm2 ("ND_READ_ARRAY", MakeId_Copy (name), MakeNum (dim));
            }
        }
    }

    DBUG_RETURN (index);
}

/******************************************************************************
 *
 * Function:
 *   bool NameOrVal_Eq( char *name1, int val1,
 *                      char *name2, int val2,
 *                      int shape)
 *
 * Description:
 *   Compares two parameters of N_WLstride(Var) or N_WLgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) 'shape' must equal IDX_SHAPE.
 *
 ******************************************************************************/

bool
NameOrVal_Eq (char *name1, int val1, char *name2, int val2, int shape)
{
    bool ret;

    DBUG_ENTER ("NameOrVal_Eq");

    DBUG_ASSERT (((shape >= 0) || (shape == IDX_SHAPE)), "illegal shape found!");

    if (NameOrVal_IsInt (name1, val1) && NameOrVal_IsInt (name2, val2)) {
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
        ret = (!strcmp (name1, name2));
    } else {
        ret = FALSE;
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool NodeOrInt_Eq( nodetype nt1, void *node_or_int1,
 *                      nodetype nt2, void *node_or_int2,
 *                      int shape)
 *
 * Description:
 *   Compares two parameters of N_WLstride(Var) or N_WLgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) it must be set to IDX_SHAPE.
 *
 ******************************************************************************/

bool
NodeOrInt_Eq (nodetype nt1, void *node_or_int1, nodetype nt2, void *node_or_int2,
              int shape)
{
    char *name1, *name2;
    int val1, val2;
    bool ret;

    DBUG_ENTER ("NodeOrInt_Eq");

    NodeOrInt_GetNameOrVal (&name1, &val1, nt1, node_or_int1);
    NodeOrInt_GetNameOrVal (&name2, &val2, nt2, node_or_int2);
    ret = NameOrVal_Eq (name1, val1, name2, val2, shape);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool NodeOrInt_IntEq( nodetype nt1, void *node_or_int1,
 *                         int val2,
 *                         int shape)
 *
 * Description:
 *   Compares a parameter of a N_WLstride(Var) or N_WLgrid(Var) node with
 *   an integer value.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) it must be set to IDX_SHAPE.
 *
 ******************************************************************************/

bool
NodeOrInt_IntEq (nodetype nt1, void *node_or_int1, int val2, int shape)
{
    char *name1;
    int val1;
    bool ret;

    DBUG_ENTER ("NodeOrInt_IntEq");

    NodeOrInt_GetNameOrVal (&name1, &val1, nt1, node_or_int1);
    ret = NameOrVal_Eq (name1, val1, NULL, val2, shape);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool NodeOrInt_StrEq( nodetype nt1, void *node_or_int1,
 *                         char *name2,
 *                         int shape)
 *
 * Description:
 *   Compares a parameter of a N_WLstride(Var) or N_WLgrid(Var) node with
 *   a string.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) it must be set to IDX_SHAPE.
 *
 ******************************************************************************/

bool
NodeOrInt_StrEq (nodetype nt1, void *node_or_int1, char *name2, int shape)
{
    char *name1;
    int val1;
    bool ret;

    DBUG_ENTER ("NodeOrInt_StrEq");

    NodeOrInt_GetNameOrVal (&name1, &val1, nt1, node_or_int1);
    ret = NameOrVal_Eq (name1, val1, name2, IDX_OTHER, shape);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool NameOrVal_Le( char *name1, int val1,
 *                      char *name2, int val2,
 *                      int shape)
 *
 * Description:
 *   Compares two parameters of N_WLstride(Var) or N_WLgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) 'shape' must equal IDX_SHAPE.
 *
 ******************************************************************************/

bool
NameOrVal_Le (char *name1, int val1, char *name2, int val2, int shape)
{
    bool ret;

    DBUG_ENTER ("NameOrVal_Le");

    ret = NameOrVal_Eq (name1, val1, name2, val2, shape);
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
 *   bool NodeOrInt_Le( nodetype nt1, void *node_or_int1,
 *                      nodetype nt2, void *node_or_int2,
 *                      int shape)
 *
 * Description:
 *   Compares two parameters of N_WLstride(Var) or N_WLgrid(Var) nodes.
 *
 *   'shape' denotes the shape component of the current dimension.
 *   If the concrete value of the shape is unknown (i.e. dynamically shaped
 *   or fold with-loop) 'shape' must equal IDX_SHAPE.
 *
 ******************************************************************************/

bool
NodeOrInt_Le (nodetype nt1, void *node_or_int1, nodetype nt2, void *node_or_int2,
              int shape)
{
    char *name1, *name2;
    int val1, val2;
    bool ret;

    DBUG_ENTER ("NodeOrInt_Le");

    NodeOrInt_GetNameOrVal (&name1, &val1, nt1, node_or_int1);
    NodeOrInt_GetNameOrVal (&name2, &val2, nt2, node_or_int2);
    ret = NameOrVal_Le (name1, val1, name2, val2, shape);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   void NodeOrInt_Print( FILE *handle,
 *                         nodetype nt, void *node_or_int,
 *                         int dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
NodeOrInt_Print (FILE *handle, nodetype nt, void *node_or_int, int dim)
{
    char *name;
    int val;

    DBUG_ENTER ("NodeOrInt_Print");

    NodeOrInt_GetNameOrVal (&name, &val, nt, node_or_int);

    if (NameOrVal_IsInt (name, val)) {
        if (val == IDX_OTHER) {
            fprintf (outfile, "?");
        } else if (val == IDX_SHAPE) {
            fprintf (outfile, ".");
        } else {
            fprintf (handle, "%i", val);
        }
    } else {
        DBUG_ASSERT ((ID_VARDEC ((*((node **)node_or_int))) != NULL), "no vardec found!");

        if (ID_DIM ((*((node **)node_or_int))) == SCALAR) {
            fprintf (handle, "%s", name);
        } else {
            fprintf (handle, "%s[%i]", name, dim);
        }
    }

    DBUG_VOID_RETURN;
}
