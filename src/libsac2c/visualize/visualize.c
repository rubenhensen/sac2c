/**
 * @defgroup graph Graph Functions.
 *
 * Functions needed by graph traversal.
 *
 * @{
 */

#include <stdio.h>
#include <stdlib.h>

#include "visualize.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "type_utils.h"
#include "DupTree.h"

#define DBUG_PREFIX "VISUAL"
#include "debug.h"

#include "traverse.h"
#include "ctinfo.h"
#include "free.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "filemgr.h"
#include "globals.h"
#include "gen_startup_code.h"
#include "SSAWithloopFolding.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "wltransform.h"
#include "multithread_lib.h"
#include "constants.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "shape.h"
#include "vector.h"
#include "dynelem.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "tfprintutils.h"
#include "int_matrix.h"
#include "LookUpTable.h"

/*
 * the INFO structure is shared between all phases that do use functions
 * from visualization.
 */

/* INFO structure */
struct INFO {
    /* print */
    FILE *file;
    lut_t *table;
    int node_number;
    bool draw_attredges; /* enable drawing attribute edges */

    /*The following variables are used for dedicated visualization*/
    char *namespaces;
    bool findmodule_fun;
    bool finmodule_fundec;
    bool findfundeffun;
    bool isfrommodulefun;
    bool isfrommodulefundec;

    node *module_fundec;
    node *module_fun;
    node *fundef_fun;
};

/* access macros print */
#define INFO_NODENUMBER(n) ((n)->node_number)
#define INFO_FILE(n) ((n)->file)
#define INFO_TABLE(n) ((n)->table)
#define INFO_DRAW_ATTREDGES(n) ((n)->draw_attredges)
#define INFO_NAMESPACE(n) ((n)->namespaces)

#define INFO_FINDMODULEFUN(n) ((n)->findmodule_fun)
#define INFO_FINDMODULEFUNDEC(n) ((n)->finmodule_fundec)
#define INFO_FINDFUNDEFFUN(n) ((n)->findfundeffun)
#define INFO_ISFROMMODULEFUN(n) ((n)->findfundeffun)
#define INFO_ISFROMMODULEFUNDEC(n) ((n)->isfrommodulefundec)

#define INFO_MODULE_FUN(n) ((n)->module_fun)
#define INFO_MODULE_FUNDEC(n) ((n)->module_fundec)
#define INFO_FUNDEF_FUN(n) ((n)->fundef_fun)

/******************************************************************************
 *
 * function:
 *   static info *MakeInfo()
 *
 * description:
 *   creates a new info structure.
 *
 ******************************************************************************/
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NODENUMBER (result) = 0;
    INFO_FILE (result) = NULL;
    INFO_TABLE (result) = LUTgenerateLut ();
    /* TODO: maybe a new command-line option to control the drawing of attribute edges? */
    INFO_DRAW_ATTREDGES (result) = TRUE;

    INFO_NAMESPACE (result) = NULL;

    INFO_FINDMODULEFUNDEC (result) = FALSE;
    INFO_FINDMODULEFUN (result) = FALSE;
    INFO_FINDFUNDEFFUN (result) = FALSE;
    INFO_ISFROMMODULEFUN (result) = FALSE;
    INFO_ISFROMMODULEFUNDEC (result) = FALSE;

    INFO_MODULE_FUN (result) = NULL;
    INFO_MODULE_FUNDEC (result) = NULL;
    INFO_FUNDEF_FUN (result) = NULL;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   static info *FreeInfo( info* info)
 *
 * description:
 *   frees a given info structure.
 *
 ******************************************************************************/
static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    LUTremoveLut (INFO_TABLE (info));

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   static char* giveNodeName (node * arg_node, info * arg_info)
 *
 * description:
 *   returns a string name of the node suitable for output to a dot file.
 *
 ******************************************************************************/
static char *
giveNodeName (node *arg_node, info *arg_info)
{
    /* see if it is already in the table */
    void **ndname_p = LUTsearchInLutP (INFO_TABLE (arg_info), arg_node);

    if (ndname_p) {
        /* already generated */
        return (char *)*ndname_p;
    } else {
        /* the node was not seen so far; invent a name for it now */
        char *ndnumber = STRitoa (INFO_NODENUMBER (arg_info)++);
        char *nodename = STRcat ("node", ndnumber);
        ndnumber = MEMfree (ndnumber);

        INFO_TABLE (arg_info)
          = LUTinsertIntoLutP (INFO_TABLE (arg_info), arg_node, nodename);
        /* NOTE: the new memory in nodename is considered owned in the LUT */
        return nodename;
    }
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALids( node *arg_node, info *arg_info)
 *
 *   @brief print N_ids node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (arg_node != NULL) {
        char *node_name = giveNodeName (arg_node, arg_info);

        if (IDS_AVIS (arg_node) != NULL) {
            if (INFO_DRAW_ATTREDGES (arg_info)) {
                fprintf (INFO_FILE (arg_info),
                         "%s[label=\"Ids\" style=filled fillcolor=\"lightblue\"];\n",
                         node_name);
            } else {
                fprintf (INFO_FILE (arg_info),
                         "%s[label=\"Ids \\n%s\" style=filled "
                         "fillcolor=\"lightblue\"];\n",
                         node_name, IDS_NAME (arg_node));
            }

            if (INFO_DRAW_ATTREDGES (arg_info)) {
                fprintf (INFO_FILE (arg_info), "%s -> %s [style=dashed,color=blue];\n",
                         node_name, giveNodeName (IDS_AVIS (arg_node), arg_info));
            }
        }

        // traver son nodes
        TRAVopt (IDS_NEXT (arg_node), arg_info);
        // add edge between two nodes with lable
        if (NULL != IDS_NEXT (arg_node)) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               IDS_NEXT (arg_node)));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALspids( node *arg_node, info *arg_info)
 *
 *   @brief print N_spids node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALspids (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    fprintf (INFO_FILE (arg_info),
             "%s[label=\"%s\" style=filled fillcolor=\"lightblue\"];\n", node_name,
             SPIDS_NAME (arg_node));

    // traver son nodes
    TRAVopt (SPIDS_NEXT (arg_node), arg_info);

    // add edge between two nodes with lable

    if (NULL != SPIDS_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), SPIDS_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *VISUALtypepattern (node *arg_node, info *arg_info)
 *
 * @brief print N_typepattern node
 *
 ******************************************************************************/
node *
VISUALtypepattern (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/** <!--*********************************************************************-->
 *
 * @fn node *VISUALmodule( node *arg_node, info *arg_info)
 *
 * @brief Visualize current node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 *******************************************************************************/
node *
VISUALmodule (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    INFO_NAMESPACE (arg_info) = STRcpy (NSgetName (MODULE_NAMESPACE (arg_node)));

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (MODULE_INTERFACE (arg_node), arg_info);
    TRAVopt (MODULE_TYPEFAMILIES (arg_node), arg_info);
    TRAVopt (MODULE_STRUCTS (arg_node), arg_info);
    TRAVopt (MODULE_TYPES (arg_node), arg_info);
    TRAVopt (MODULE_OBJS (arg_node), arg_info);
    TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);
    TRAVopt (MODULE_FUNSPECS (arg_node), arg_info);
    TRAVopt (MODULE_SPMDSTORE (arg_node), arg_info);
    TRAVopt (MODULE_FPFRAMESTORE (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Module];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != MODULE_INTERFACE (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Interface];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_INTERFACE (arg_node)));
    }

    if (NULL != MODULE_TYPEFAMILIES (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Typefamilies];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_TYPEFAMILIES (arg_node)));
    }

    if (NULL != MODULE_STRUCTS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Structs];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_STRUCTS (arg_node)));
    }

    if (NULL != MODULE_TYPES (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Types];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_TYPES (arg_node)));
    }

    if (NULL != MODULE_OBJS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Objs];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_OBJS (arg_node)));
    }

    if (NULL != MODULE_THREADFUNS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Threadfuns];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_THREADFUNS (arg_node)));
    }

    if (NULL != MODULE_FUNSPECS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Funspecs];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_FUNSPECS (arg_node)));
    }

    if (NULL != MODULE_SPMDSTORE (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=SPMDSTORE];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_SPMDSTORE (arg_node)));
    }

    if (NULL != MODULE_FPFRAMESTORE (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=FPFrameStore];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODULE_FPFRAMESTORE (arg_node)));
    }

    /* choose function to visualize */
    if ((MODULE_FUNDECS (arg_node) != NULL)
        && (global.dovisualizefunsets || global.visualizefunsets.imp
            || global.visualizefunsets.use)) {

        INFO_ISFROMMODULEFUNDEC (arg_info) = TRUE;
        TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
        INFO_ISFROMMODULEFUNDEC (arg_info) = FALSE;

        if (INFO_MODULE_FUNDEC (arg_info) != NULL) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=Fundecs];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               INFO_MODULE_FUNDEC (arg_info)));
        }
    }

    if ((MODULE_FUNS (arg_node) != NULL)
        && (global.dovisualizefunsets || global.visualizefunsets.def
            || global.visualizefunsets.wrp || global.visualizefunsets.pre
            || global.visualizefunsets.use)) {

        INFO_ISFROMMODULEFUN (arg_info) = TRUE;
        TRAVopt (MODULE_FUNS (arg_node), arg_info);
        INFO_ISFROMMODULEFUN (arg_info) = FALSE;

        if (INFO_MODULE_FUN (arg_info) != NULL) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=Funs];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               INFO_MODULE_FUN (arg_info)));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--*********************************************************************-->
 *
 * @fn node * VISUALstructdef (node * arg_node, info * arg_info)
 *
 * @brief Visualize current node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ******************************************************************************/

node *
VISUALstructdef (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (STRUCTDEF_STRUCTELEM (arg_node), arg_info);

    // add edge between two nodes with lable

    fprintf (INFO_FILE (arg_info), "%s[label=StructDef];\n%s -> %s [label=StructElem];\n",
             node_name, node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                       STRUCTDEF_STRUCTELEM (arg_node)));

    TRAVopt (STRUCTDEF_NEXT (arg_node), arg_info);

    if (NULL != STRUCTDEF_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           STRUCTDEF_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALstructelem( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALstructelem (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // output current node
    fprintf (INFO_FILE (arg_info), "%s[label=StructElem];\n", node_name);

    TRAVopt (STRUCTELEM_NEXT (arg_node), arg_info);

    if (NULL != STRUCTELEM_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           STRUCTELEM_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALtypedef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALtypedef (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // output current node
    fprintf (INFO_FILE (arg_info), "%s[label=TypeDef];\n", node_name);

    TRAVopt (TYPEDEF_NEXT (arg_node), arg_info);

    if (NULL != TYPEDEF_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           TYPEDEF_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALobjdef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALobjdef (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (OBJDEF_EXPR (arg_node), arg_info);

    // add edge between two nodes with lable

    if (NULL != OBJDEF_EXPR (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s[label=Object];\n%s -> %s [label=Expr];\n",
                 node_name, node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           OBJDEF_EXPR (arg_node)));
    }

    TRAVopt (OBJDEF_NEXT (arg_node), arg_info);

    if (NULL != OBJDEF_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           OBJDEF_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 * Remark:
 *   If C-code is to be generated, which means that an N_icm node already
 *   hangs on node[3], additional extern declarations for function
 *   definitions are printed.
 *
 ******************************************************************************/

node *
VISUALfundef (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);
    bool is_userdefined_function = FALSE;

    DBUG_ENTER ();

    if (global.dovisualizefunsets) {

        // traver son nodes
        TRAVopt (FUNDEF_RETS (arg_node), arg_info);
        TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        TRAVopt (FUNDEF_OBJECTS (arg_node), arg_info);
        TRAVopt (FUNDEF_AFFECTEDOBJECTS (arg_node), arg_info);
        TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

        // ouput current node
        fprintf (INFO_FILE (arg_info), "%s[label=\"Fundef\\n%s\" \
            style=filled fillcolor=\"lightblue\"];\n", node_name,
                 FUNDEF_NAME (arg_node));

        // add edge between two nodes with lable
        if (NULL != FUNDEF_RETS (arg_node)) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=Rets];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               FUNDEF_RETS (arg_node)));
        }

        // add edge between two nodes with lable
        if (NULL != FUNDEF_ARGS (arg_node)) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=Args];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               FUNDEF_ARGS (arg_node)));
        }
        // add edge between two nodes with lable
        if (NULL != FUNDEF_BODY (arg_node)) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=Body];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               FUNDEF_BODY (arg_node)));
        }

        // add edge between two nodes with lable
        if (NULL != FUNDEF_OBJECTS (arg_node)) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=Objects];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               FUNDEF_OBJECTS (arg_node)));
        }

        // add edge between two nodes with lable
        if (NULL != FUNDEF_AFFECTEDOBJECTS (arg_node)) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=AffectedObjects];\n",
                     node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               FUNDEF_AFFECTEDOBJECTS (arg_node)));
        }
        // add edge between two nodes with lable
        if (NULL != FUNDEF_LOCALFUNS (arg_node)) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=LocalFuns];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               FUNDEF_LOCALFUNS (arg_node)));
        }
        // add edge between two nodes with lable
        if (NULL != FUNDEF_NEXT (arg_node)) {
            fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                     (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                               FUNDEF_NEXT (arg_node)));
        }

    } else {

        is_userdefined_function
          = STReq (NSgetName (FUNDEF_NS (arg_node)), INFO_NAMESPACE (arg_info));

        // check whether user-defined functions
        if (((global.visualizefunsets.def && FUNDEF_ISLOCAL (arg_node)
              && is_userdefined_function)
             || (global.visualizefunsets.wrp && FUNDEF_ISWRAPPERFUN (arg_node)
                 && FUNDEF_BODY (arg_node))
             || (global.visualizefunsets.pre && FUNDEF_ISSTICKY (arg_node))
             || (global.visualizefunsets.imp && FUNDEF_WASIMPORTED (arg_node))
             || (global.visualizefunsets.use && FUNDEF_WASUSED (arg_node)))) {

            // did not find connection yet,now find
            if (!INFO_FINDMODULEFUN (arg_info) && INFO_ISFROMMODULEFUN (arg_info)
                && ((global.visualizefunsets.def && FUNDEF_ISLOCAL (arg_node)
                     && is_userdefined_function)
                    || (global.visualizefunsets.wrp && FUNDEF_ISWRAPPERFUN (arg_node))
                    || (global.visualizefunsets.pre && FUNDEF_ISSTICKY (arg_node))
                    || (global.visualizefunsets.use && FUNDEF_WASUSED (arg_node)))) {
                INFO_MODULE_FUN (arg_info) = arg_node;
                INFO_FINDMODULEFUN (arg_info) = TRUE;
            }

            if (!INFO_FINDMODULEFUNDEC (arg_info) && INFO_ISFROMMODULEFUNDEC (arg_info)
                && ((global.visualizefunsets.imp && FUNDEF_WASIMPORTED (arg_node))
                    || (global.visualizefunsets.use && FUNDEF_WASUSED (arg_node)))) {
                INFO_MODULE_FUNDEC (arg_info) = arg_node;
                INFO_FINDMODULEFUNDEC (arg_info) = TRUE;
            }

            // traver son nodes
            TRAVopt (FUNDEF_RETS (arg_node), arg_info);
            TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
            TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            TRAVopt (FUNDEF_OBJECTS (arg_node), arg_info);
            TRAVopt (FUNDEF_AFFECTEDOBJECTS (arg_node), arg_info);
            TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

            INFO_FINDFUNDEFFUN (arg_info) = FALSE;
            TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

            // ouput current node
            fprintf (INFO_FILE (arg_info), "%s[label=\"Fundef\\n%s\" \
              style=filled fillcolor=\"lightblue\"];\n", node_name,
                     FUNDEF_NAME (arg_node));

            // add edge between two nodes with lable
            if (NULL != FUNDEF_RETS (arg_node)) {
                fprintf (INFO_FILE (arg_info), "%s -> %s [label=Rets];\n", node_name,
                         (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                                   FUNDEF_RETS (arg_node)));
            }

            // add edge between two nodes with lable
            if (NULL != FUNDEF_ARGS (arg_node)) {
                fprintf (INFO_FILE (arg_info), "%s -> %s [label=Args];\n", node_name,
                         (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                                   FUNDEF_ARGS (arg_node)));
            }
            // add edge between two nodes with lable
            if (NULL != FUNDEF_BODY (arg_node)) {
                fprintf (INFO_FILE (arg_info), "%s -> %s [label=Body];\n", node_name,
                         (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                                   FUNDEF_BODY (arg_node)));
            }

            // add edge between two nodes with lable
            if (NULL != FUNDEF_OBJECTS (arg_node)) {
                fprintf (INFO_FILE (arg_info), "%s -> %s [label=Objects];\n", node_name,
                         (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                                   FUNDEF_OBJECTS (arg_node)));
            }

            // add edge between two nodes with lable
            if (NULL != FUNDEF_AFFECTEDOBJECTS (arg_node)) {
                fprintf (INFO_FILE (arg_info), "%s -> %s [label=AffectedObjects];\n",
                         node_name,
                         (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                                   FUNDEF_AFFECTEDOBJECTS (arg_node)));
            }
            // add edge between two nodes with lable
            if (NULL != FUNDEF_LOCALFUNS (arg_node)) {
                fprintf (INFO_FILE (arg_info), "%s -> %s [label=LocalFuns];\n", node_name,
                         (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                                   FUNDEF_LOCALFUNS (arg_node)));
            }
            // add edge between two nodes with lable
            if (NULL != FUNDEF_NEXT (arg_node)) {
                if ((INFO_FUNDEF_FUN (arg_info) != arg_node)
                    && INFO_FINDFUNDEFFUN (arg_info)) {
                    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                                       INFO_FUNDEF_FUN (arg_info)));
                }
            }

            INFO_FUNDEF_FUN (arg_info) = arg_node;
            INFO_FINDFUNDEFFUN (arg_info) = TRUE;
        } else {
            TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALret( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/
node *
VISUALret (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);
    char *type_str;

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (RET_NEXT (arg_node), arg_info);

    // ouput current node
    if (RET_TYPE (arg_node) != NULL) {
        type_str = TYtype2String (RET_TYPE (arg_node), FALSE, 0);
        fprintf (INFO_FILE (arg_info),
                 "%s[label=\"Ret \\n%s\" style=filled fillcolor=\"lightblue\"];\n",
                 node_name, type_str);
    } else {
        fprintf (INFO_FILE (arg_info), "%s[label=Ret];\n", node_name);
    }

    // add edge between two nodes with lable
    if (NULL != RET_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), RET_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALannotate( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALannotate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALarg( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALarg (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (ARG_AVIS (arg_node), arg_info);
    TRAVopt (ARG_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=ARG];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != ARG_AVIS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Avis];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), ARG_AVIS (arg_node)));
    }

    // add edge between two nodes with lable
    if (NULL != ARG_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), ARG_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALvardec( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALvardec (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (VARDEC_AVIS (arg_node), arg_info);
    TRAVopt (VARDEC_INIT (arg_node), arg_info);
    TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Vardec];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Avis];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), VARDEC_AVIS (arg_node)));

    // add edge between two nodes with lable
    if (NULL != VARDEC_INIT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Init];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           VARDEC_INIT (arg_node)));
    }

    // add edge between two nodes with lable
    // Do not print it when drawing AttrEdges to ease dot's task
    if (!INFO_DRAW_ATTREDGES (arg_info) && (NULL != VARDEC_NEXT (arg_node))) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           VARDEC_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALblock( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALblock (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    TRAVopt (BLOCK_SHAREDS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Block];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != BLOCK_ASSIGNS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Instr];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           BLOCK_ASSIGNS (arg_node)));
    }

    // add edge between two nodes with lable
    if (NULL != BLOCK_VARDECS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Vardec];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           BLOCK_VARDECS (arg_node)));
    }

    // add edge between two nodes with lable
    if (NULL != BLOCK_SHAREDS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Shareds];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           BLOCK_SHAREDS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALreturn( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALreturn (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (RETURN_EXPRS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Return];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != RETURN_EXPRS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Exprs];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           RETURN_EXPRS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALassign( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALassign (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Assign];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != ASSIGN_STMT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Inster];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           ASSIGN_STMT (arg_node)));
    }

    // add edge between two nodes with lable
    if (NULL != ASSIGN_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           ASSIGN_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALdo( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALdo (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (DO_COND (arg_node), arg_info);
    TRAVdo (DO_BODY (arg_node), arg_info);
    TRAVopt (DO_SKIP (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Do];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Cond];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), DO_COND (arg_node)));

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Body];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), DO_BODY (arg_node)));

    // add edge between two nodes with lable
    if (NULL != DO_SKIP (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Skip];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), DO_SKIP (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALwhile( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALwhile (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (WHILE_COND (arg_node), arg_info);
    TRAVdo (WHILE_BODY (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=While];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Cond];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WHILE_COND (arg_node)));

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Body];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WHILE_BODY (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALcond( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALcond (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (COND_COND (arg_node), arg_info);
    TRAVdo (COND_THEN (arg_node), arg_info);
    TRAVdo (COND_ELSE (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Cond];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Cond];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), COND_COND (arg_node)));
    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Then];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), COND_THEN (arg_node)));

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Else];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), COND_ELSE (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALcast( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALcast (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (CAST_EXPR (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Cast\\n \"%s\"];\n", node_name,
             TYtype2String (CAST_NTYPE (arg_node), FALSE, 0));

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=expr];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), CAST_EXPR (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALlet( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALlet (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (LET_IDS (arg_node), arg_info);
    TRAVdo (LET_EXPR (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=let];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != LET_IDS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Ids];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), LET_IDS (arg_node)));
    }

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Expr];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), LET_EXPR (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALprf( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALprf (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (PRF_ARGS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Args];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != PRF_ARGS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Ids];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), PRF_ARGS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALfuncond( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALfuncond (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (FUNCOND_IF (arg_node), arg_info);
    TRAVdo (FUNCOND_THEN (arg_node), arg_info);
    TRAVdo (FUNCOND_ELSE (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Funcond];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=If];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), FUNCOND_IF (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Then];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), FUNCOND_THEN (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Else];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), FUNCOND_ELSE (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALap( node  *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALap (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (AP_ARGS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Ap];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != AP_ARGS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Args];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), AP_ARGS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALspap( node  *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALspap (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (SPAP_ID (arg_node), arg_info);
    TRAVopt (SPAP_ARGS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Spap];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Id];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), SPAP_ID (arg_node)));

    if (NULL != SPAP_ARGS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Args];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), SPAP_ARGS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALspmop( node  *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALspmop (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (SPMOP_OPS (arg_node), arg_info);
    TRAVopt (SPMOP_EXPRS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=SPMop];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Ops];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), SPMOP_OPS (arg_node)));

    if (NULL != SPMOP_EXPRS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Exprs];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           SPMOP_EXPRS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALempty( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALempty (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Empty];\n", node_name);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALarray( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALarray (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (ARRAY_AELEMS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Array];\n", node_name);

    // add edge between two nodes with lable
    if (NULL != ARRAY_AELEMS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=AElems];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           ARRAY_AELEMS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALexprs( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALexprs (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (EXPRS_EXPR (arg_node), arg_info);
    TRAVopt (EXPRS_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Exprs];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Expr];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), EXPRS_EXPR (arg_node)));

    if (NULL != EXPRS_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), EXPRS_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALid( node *arg_node, info *arg_info)
 *
 *   @brief print N_id node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALid (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    if (INFO_DRAW_ATTREDGES (arg_info) || (NULL == ID_NAME (arg_node))) {
        fprintf (INFO_FILE (arg_info),
                 "%s[label=Id style=filled fillcolor=\"lightblue\"];\n", node_name);
    } else {
        fprintf (INFO_FILE (arg_info),
                 "%s[label=\"Id\\n%s\" style=filled fillcolor=\"lightblue\"];\n",
                 node_name, ID_NAME (arg_node));
    }

    if (ID_AVIS (arg_node) && INFO_DRAW_ATTREDGES (arg_info)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s[style=dashed,color=blue];\n", node_name,
                 giveNodeName (ID_AVIS (arg_node), arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALspid( node *arg_node, info *arg_info)
 *
 *   @brief print N_spid node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALspid (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=\"%s\" style=filled fillcolor=\"lightblue\"];\n", node_name,
             SPID_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALglobobj( node *arg_node, info *arg_info)
 *
 *   @brief print N_globobj node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALglobobj (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node

    fprintf (INFO_FILE (arg_info), "%s[label=globobj];\n", node_name);

    // not sure about this node
    /*
     fprintf( INFO_FILE( arg_info),
     "%s -> %s [label=Objdef color=blue constraint=false splines=line];\n",
     node_name, (char*) *LUTsearchInLutP( INFO_TABLE(arg_info),
     GLOBOBJ_OBJDEF( arg_node)));

     */

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnum( node *arg_node, info *arg_info)
 *
 *   @brief print N_num node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnum (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%d style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUM_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumbyte( node *arg_node, info *arg_info)
 *
 *   @brief print N_numbyte node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumbyte (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%d style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMBYTE_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumshort( node *arg_node, info *arg_info)
 *
 *   @brief print N_numshort node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumshort (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%hd style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMSHORT_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumint( node *arg_node, info *arg_info)
 *
 *   @brief print N_numint node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumint (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%d style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMINT_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumlong( node *arg_node, info *arg_info)
 *
 *   @brief print N_numlong node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumlong (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%ld style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMLONG_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumlonglong( node *arg_node, info *arg_info)
 *
 *   @brief print N_numlonglong node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumlonglong (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%lldLL style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMLONGLONG_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumubyte( node *arg_node, info *arg_info)
 *
 *   @brief print N_numubyte node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumubyte (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%u style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMUBYTE_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumushort( node *arg_node, info *arg_info)
 *
 *   @brief print N_numushort node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumushort (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%hu style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMUSHORT_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumint( node *arg_node, info *arg_info)
 *
 *   @brief print N_numint node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumuint (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%u style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMUINT_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumulong( node *arg_node, info *arg_info)
 *
 *   @brief print N_numulong node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumulong (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%lu style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMULONG_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnumulonglong( node *arg_node, info *arg_info)
 *
 *   @brief print N_numulonglong node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnumulonglong (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%lluULL style=filled fillcolor=\"lightyellow\"];\n", node_name,
             NUMULONGLONG_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALfloat( node *arg_node, info *arg_info)
 *
 *   @brief print N_float node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALfloat (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);
    char *tmp;

    DBUG_ENTER ();

    tmp = CVfloat2String (FLOAT_VAL (arg_node));

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=\"%s\" style=filled fillcolor=\"lightyellow\"];\n", node_name,
             tmp);

    tmp = MEMfree (tmp);

    DBUG_RETURN (arg_node);
}

/* FIXME implement properly later.  */
node *
VISUALfloatvec (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);
    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=\"%s\" style=filled fillcolor=\"lightyellow\"];\n", node_name,
             "<ho ho ho>");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALdouble( node *arg_node, info *arg_info)
 *
 *   @brief print N_double node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALdouble (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);
    char *tmp;

    DBUG_ENTER ();

    tmp = CVdouble2String (DOUBLE_VAL (arg_node));

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%s style=filled fillcolor=\"lightyellow\"];\n", node_name, tmp);

    tmp = MEMfree (tmp);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALbool( node *arg_node, info *arg_info)
 *
 *   @brief print N_bool node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALbool (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node

    if (0 == BOOL_VAL (arg_node)) {
        fprintf (INFO_FILE (arg_info),
                 "%s[label=false style=filled fillcolor=\"lightyellow\"];\n", node_name);
    } else {
        fprintf (INFO_FILE (arg_info),
                 "%s[label=true style=filled fillcolor=\"lightyellow\"];\n", node_name);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALstr( node *arg_node, info *arg_info)
 *
 *   @brief print N_str node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALstr (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=\"%s\" style=filled fillcolor=\"lightyellow\"];\n", node_name,
             STR_STRING (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * Function:
 *   node *VISUALtype( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALtype (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    if (NULL != TYPE_TYPE (arg_node)) {
        fprintf (INFO_FILE (arg_info),
                 "%s[label=\"%s\" style=filled fillcolor=\"lightyellow\"];\n", node_name,
                 TYtype2String (TYPE_TYPE (arg_node), FALSE, 0));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *  node *VISUALdot( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALdot (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    if (NULL != TYPE_TYPE (arg_node)) {
        fprintf (INFO_FILE (arg_info),
                 "%s[label=dot\\n\"%d\" style=filled fillcolor=\"lightyellow\"];\n",
                 node_name, DOT_NUM (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *  node *VISUALsetwl( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALsetwl (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (SETWL_VEC (arg_node), arg_info);
    TRAVdo (SETWL_EXPR (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=SetWL];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Vec];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), SETWL_VEC (arg_node)));

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Expr];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), SETWL_EXPR (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALchar( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALchar (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=%d style=filled fillcolor=\"lightyellow\"];\n", node_name,
             CHAR_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALicm( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALicm (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVopt (ICM_ARGS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Icm];\n", node_name);

    // ouput current node
    if (NULL != ICM_ARGS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Args];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), ICM_ARGS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALpragma( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALpragma (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVopt (PRAGMA_READONLY (arg_node), arg_info);
    TRAVopt (PRAGMA_REFCOUNTING (arg_node), arg_info);
    TRAVopt (PRAGMA_EFFECT (arg_node), arg_info);
    TRAVopt (PRAGMA_LINKSIGN (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Pragma];\n", node_name);

    // generate lable
    if (NULL != PRAGMA_READONLY (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=ReadOnly];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           PRAGMA_READONLY (arg_node)));
    }

    if (NULL != PRAGMA_REFCOUNTING (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Refcounting];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           PRAGMA_REFCOUNTING (arg_node)));
    }

    if (NULL != PRAGMA_EFFECT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Effect];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           PRAGMA_EFFECT (arg_node)));
    }

    if (NULL != PRAGMA_LINKSIGN (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=LinkSign];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           PRAGMA_LINKSIGN (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALmt(node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALmt (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVopt (MT_REGION (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=MT];\n", node_name);

    // generate lable
    if (NULL != MT_REGION (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Region];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), MT_REGION (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALex(node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALex (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVdo (EX_REGION (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=EX];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Region];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), EX_REGION (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALst(node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALst (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVdo (ST_REGION (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=ST];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Region];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), ST_REGION (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALcudast(node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALcudast (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVdo (CUDAST_REGION (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Cudast];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Region];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), CUDAST_REGION (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALwith( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_with node.
 *
 ******************************************************************************/

//<--check phase to ensure use correct traversal function--!>
node *
VISUALwith (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVdo (WITH_PART (arg_node), arg_info); // phase all, mondatory yes
    TRAVdo (WITH_CODE (arg_node), arg_info); // phase all, mondatory yes
    TRAVopt (WITH_WITHOP (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=With];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Part];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITH_PART (arg_node)));

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Code];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITH_CODE (arg_node)));

    if (NULL != WITH_WITHOP (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=WithOp];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WITH_WITHOP (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALwithid(node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_withid-nodes
 *
 ******************************************************************************/

node *
VISUALwithid (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVopt (WITHID_VEC (arg_node), arg_info);
    TRAVopt (WITHID_IDS (arg_node), arg_info);
    TRAVopt (WITHID_IDXS (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Withid];\n", node_name);

    // generate lable
    if (NULL != WITHID_VEC (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Vec];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITHID_VEC (arg_node)));
    }

    if (NULL != WITHID_IDS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Ids];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITHID_IDS (arg_node)));
    }

    if (NULL != WITHID_IDXS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Idxs];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WITHID_IDXS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALgenerator(node *arg_node, info *arg_info)
 *
 * description:
 *   visualise a generator.
 *
 *   The index variable is found in NPART_WITHID( INFO_NPART( arg_info)).
 *
 ******************************************************************************/

node *
VISUALgenerator (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVopt (GENERATOR_BOUND1 (arg_node), arg_info);
    TRAVopt (GENERATOR_BOUND2 (arg_node), arg_info);
    TRAVopt (GENERATOR_STEP (arg_node), arg_info);
    TRAVopt (GENERATOR_WIDTH (arg_node), arg_info);
    TRAVopt (GENERATOR_GENWIDTH (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Generator];\n", node_name);

    // generate lable
    if (NULL != GENERATOR_BOUND1 (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound1];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENERATOR_BOUND1 (arg_node)));
    }

    if (NULL != GENERATOR_BOUND2 (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound2];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENERATOR_BOUND2 (arg_node)));
    }

    if (NULL != GENERATOR_STEP (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Step];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENERATOR_STEP (arg_node)));
    }

    if (NULL != GENERATOR_WIDTH (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Width];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENERATOR_WIDTH (arg_node)));
    }

    if (NULL != GENERATOR_GENWIDTH (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=GenWidth];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENERATOR_GENWIDTH (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALdefault(node *arg_node, info *arg_info)
 *
 * description:
 *   visualise a default node.
 *
 *   The index variable is found in PART_WITHID( INFO_PART( arg_info)).
 *
 ******************************************************************************/

node *
VISUALdefault (node *arg_node, info *arg_info)
{
    /*char *node_name =*/giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALcode( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_code-nodes
 *
 ******************************************************************************/

node *
VISUALcode (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    TRAVopt (CODE_CEXPRS (arg_node), arg_info);
    TRAVopt (CODE_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Code];\n", node_name);

    // generate lable
    if (NULL != CODE_CBLOCK (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=CBlock];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           CODE_CBLOCK (arg_node)));
    }

    if (NULL != CODE_CEXPRS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=CExprs];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           CODE_CEXPRS (arg_node)));
    }

    if (NULL != CODE_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), CODE_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALpart( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_part nodes
 *
 ******************************************************************************/

node *
VISUALpart (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVdo (PART_WITHID (arg_node), arg_info);
    TRAVdo (PART_GENERATOR (arg_node), arg_info);
    TRAVopt (PART_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Part];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=WithId];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), PART_WITHID (arg_node)));

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Generator];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), PART_GENERATOR (arg_node)));

    if (NULL != PART_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), PART_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALgenarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/

node *
VISUALgenarray (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);
    TRAVopt (GENARRAY_MEM (arg_node), arg_info);
    TRAVopt (GENARRAY_SUB (arg_node), arg_info);
    TRAVopt (GENARRAY_RC (arg_node), arg_info);
    TRAVopt (GENARRAY_PRC (arg_node), arg_info);
    TRAVopt (GENARRAY_DEFSHAPEEXPR (arg_node), arg_info);
    TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Genarray];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Shape];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), GENARRAY_SHAPE (arg_node)));

    if (NULL != GENARRAY_DEFAULT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Default];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENARRAY_DEFAULT (arg_node)));
    }
    if (NULL != GENARRAY_MEM (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Mem];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENARRAY_MEM (arg_node)));
    }
    if (NULL != GENARRAY_SUB (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Sub];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENARRAY_SUB (arg_node)));
    }
    if (NULL != GENARRAY_RC (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Rc];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENARRAY_RC (arg_node)));
    }
    if (NULL != GENARRAY_PRC (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Prc];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENARRAY_PRC (arg_node)));
    }
    if (NULL != GENARRAY_DEFSHAPEEXPR (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=DefShapeExpr];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENARRAY_DEFSHAPEEXPR (arg_node)));
    }
    if (NULL != GENARRAY_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           GENARRAY_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALmodarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/

node *
VISUALmodarray (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
    TRAVopt (MODARRAY_MEM (arg_node), arg_info);
    TRAVopt (MODARRAY_SUB (arg_node), arg_info);
    TRAVopt (MODARRAY_RC (arg_node), arg_info);
    TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Genarray];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Shape];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), MODARRAY_ARRAY (arg_node)));

    if (NULL != MODARRAY_MEM (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Mem];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODARRAY_MEM (arg_node)));
    }
    if (NULL != MODARRAY_SUB (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Sub];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODARRAY_SUB (arg_node)));
    }
    if (NULL != MODARRAY_RC (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Rc];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODARRAY_RC (arg_node)));
    }
    if (NULL != MODARRAY_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           MODARRAY_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALspfold( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/

node *
VISUALspfold (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    TRAVdo (SPFOLD_NEUTRAL (arg_node), arg_info);
    TRAVopt (SPFOLD_GUARD (arg_node), arg_info);
    TRAVopt (SPFOLD_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Genarray];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Neutral];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), SPFOLD_NEUTRAL (arg_node)));

    if (NULL != SPFOLD_GUARD (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Guard];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           SPFOLD_GUARD (arg_node)));
    }
    if (NULL != SPFOLD_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           SPFOLD_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALfold( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/

node *
VISUALfold (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
    TRAVopt (FOLD_INITIAL (arg_node), arg_info);
    TRAVopt (FOLD_PARTIALMEM (arg_node), arg_info);
    TRAVopt (FOLD_GUARD (arg_node), arg_info);
    TRAVopt (FOLD_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Fold];\n", node_name);

    // generate lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Neutral];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), FOLD_NEUTRAL (arg_node)));

    if (NULL != FOLD_INITIAL (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Initial];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           FOLD_INITIAL (arg_node)));
    }
    if (NULL != FOLD_PARTIALMEM (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=PartialMem];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           FOLD_PARTIALMEM (arg_node)));
    }
    if (NULL != FOLD_GUARD (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Guard];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), FOLD_GUARD (arg_node)));
    }
    if (NULL != FOLD_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), FOLD_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALbreak( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/
node *
VISUALbreak (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVopt (BREAK_MEM (arg_node), arg_info);
    TRAVopt (BREAK_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Break];\n", node_name);

    if (NULL != BREAK_MEM (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Mem];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), BREAK_MEM (arg_node)));
    }
    if (NULL != BREAK_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), BREAK_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALpropagate( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/
node *
VISUALpropagate (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVopt (PROPAGATE_DEFAULT (arg_node), arg_info);
    TRAVopt (PROPAGATE_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Propagate];\n", node_name);

    if (NULL != PROPAGATE_DEFAULT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Default];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           PROPAGATE_DEFAULT (arg_node)));
    }
    if (NULL != PROPAGATE_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           PROPAGATE_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_with2-nodes
 *
 ******************************************************************************/

node *
VISUALwith2 (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVdo (WITH2_WITHID (arg_node), arg_info);
    TRAVdo (WITH2_SEGS (arg_node), arg_info);
    TRAVdo (WITH2_CODE (arg_node), arg_info);
    TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=With2];\n", node_name);

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Withid];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITH2_WITHID (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Segs];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITH2_SEGS (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Code];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITH2_CODE (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Withop];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITH2_WITHOP (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALwlseg( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_wlseg- and N_wlsegVar-nodes.
 *
 ******************************************************************************/

node *
VISUALwlseg (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVdo (WLSEG_CONTENTS (arg_node), arg_info);
    TRAVopt (WLSEG_IDXINF (arg_node), arg_info);
    TRAVopt (WLSEG_IDXSUP (arg_node), arg_info);
    TRAVopt (WLSEG_UBV (arg_node), arg_info);
    TRAVopt (WLSEG_BV (arg_node), arg_info);
    TRAVopt (WLSEG_SV (arg_node), arg_info);
    TRAVopt (WLSEG_HOMSV (arg_node), arg_info);
    TRAVopt (WLSEG_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Wlseg];\n", node_name);

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Content];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLSEG_CONTENTS (arg_node)));

    if (NULL != WLSEG_IDXINF (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=IdxInf];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLSEG_IDXINF (arg_node)));
    }
    if (NULL != WLSEG_IDXSUP (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=IdxSup];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLSEG_IDXSUP (arg_node)));
    }

    if (NULL != WLSEG_UBV (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=UBV];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLSEG_UBV (arg_node)));
    }

    if (NULL != WLSEG_BV (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=BV];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLSEG_BV (arg_node)));
    }

    if (NULL != WLSEG_SV (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=SV];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLSEG_SV (arg_node)));
    }

    if (NULL != WLSEG_HOMSV (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=HomSV];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLSEG_HOMSV (arg_node)));
    }

    if (NULL != WLSEG_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLSEG_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALwlblock( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_wlblock node
 *
 ******************************************************************************/

node *
VISUALwlblock (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVdo (WLBLOCK_BOUND1 (arg_node), arg_info);
    TRAVdo (WLBLOCK_BOUND2 (arg_node), arg_info);
    TRAVdo (WLBLOCK_STEP (arg_node), arg_info);
    TRAVopt (WLBLOCK_NEXTDIM (arg_node), arg_info);
    TRAVopt (WLBLOCK_CONTENTS (arg_node), arg_info);
    TRAVopt (WLBLOCK_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Wlblock];\n", node_name);

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound1];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLBLOCK_BOUND1 (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound2];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLBLOCK_BOUND2 (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Step];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLBLOCK_STEP (arg_node)));

    if (NULL != WLBLOCK_NEXTDIM (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=NextDim];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLBLOCK_NEXTDIM (arg_node)));
    }

    if (NULL != WLBLOCK_CONTENTS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Contents];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLBLOCK_CONTENTS (arg_node)));
    }

    if (NULL != WLBLOCK_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLBLOCK_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALwlublock( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_wlublock node
 *
 ******************************************************************************/

node *
VISUALwlublock (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVdo (WLUBLOCK_BOUND1 (arg_node), arg_info);
    TRAVdo (WLUBLOCK_BOUND2 (arg_node), arg_info);
    TRAVdo (WLUBLOCK_STEP (arg_node), arg_info);
    TRAVopt (WLUBLOCK_NEXTDIM (arg_node), arg_info);
    TRAVopt (WLUBLOCK_CONTENTS (arg_node), arg_info);
    TRAVopt (WLUBLOCK_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=WLublock];\n", node_name);

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound1];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                       WLUBLOCK_BOUND1 (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound2];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                       WLUBLOCK_BOUND2 (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Step];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLUBLOCK_STEP (arg_node)));

    if (NULL != WLUBLOCK_NEXTDIM (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=NextDim];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLUBLOCK_NEXTDIM (arg_node)));
    }

    if (NULL != WLUBLOCK_CONTENTS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Contents];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLUBLOCK_CONTENTS (arg_node)));
    }

    if (NULL != WLUBLOCK_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLUBLOCK_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALwlstride( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_wlstride nodes
 *
 * remark:
 *   DYNAMIC wlstride nodes are printed as '=>'
 *
 ******************************************************************************/

node *
VISUALwlstride (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVdo (WLSTRIDE_BOUND1 (arg_node), arg_info);
    TRAVdo (WLSTRIDE_BOUND2 (arg_node), arg_info);
    TRAVdo (WLSTRIDE_STEP (arg_node), arg_info);
    TRAVdo (WLSTRIDE_CONTENTS (arg_node), arg_info);
    TRAVopt (WLSTRIDE_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=WLstride];\n", node_name);

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound1];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                       WLSTRIDE_BOUND1 (arg_node)));

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound2];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                       WLSTRIDE_BOUND2 (arg_node)));

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Step];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLSTRIDE_STEP (arg_node)));

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Contents];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                       WLSTRIDE_CONTENTS (arg_node)));

    if (NULL != WLSTRIDE_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLSTRIDE_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALwlgrid( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_wlgrid nodes
 *
 * remark:
 *   DYNAMIC grids are printed as '==>' (fitted) and '=>>' (unfitted).
 *   STATIC grids are printed as '-->' (fitted) and '->>' (unfitted).
 *
 ******************************************************************************/

node *
VISUALwlgrid (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVdo (WLGRID_BOUND1 (arg_node), arg_info);
    TRAVdo (WLGRID_BOUND2 (arg_node), arg_info);
    TRAVopt (WLGRID_NEXTDIM (arg_node), arg_info);
    TRAVopt (WLGRID_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=WLgrid];\n", node_name);

    // lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound1];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLGRID_BOUND1 (arg_node)));

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Bound2];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WLGRID_BOUND2 (arg_node)));

    if (NULL != WLGRID_NEXTDIM (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=NextDim];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLGRID_NEXTDIM (arg_node)));
    }
    if (NULL != WLGRID_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WLGRID_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALssacnt( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise list of SSA rename counters (for debug only).
 *
 ******************************************************************************/

node *
VISUALssacnt (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVopt (SSACNT_NEXT (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), "%s[label=Ssacnt];\n", node_name);

    if (NULL != SSACNT_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           SSACNT_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALavis( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise elements of avis node connected to vardec or arg.
 *
 ******************************************************************************/

node *
VISUALavis (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVopt (AVIS_DIM (arg_node), arg_info);
    TRAVopt (AVIS_SHAPE (arg_node), arg_info);
    TRAVopt (AVIS_MIN (arg_node), arg_info);
    TRAVopt (AVIS_MAX (arg_node), arg_info);
    TRAVopt (AVIS_SCALARS (arg_node), arg_info);

    // fprintf( INFO_FILE( arg_info), "%s[label=Avis];\n",node_name);
    fprintf (INFO_FILE (arg_info),
             "%s[label=\"Avis\\n%s\", style=filled fillcolor=\"pink\"];\n", node_name,
             AVIS_NAME (arg_node));

    if (NULL != AVIS_DIM (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Dim];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), AVIS_DIM (arg_node)));
    }
    if (NULL != AVIS_SHAPE (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Shape];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), AVIS_SHAPE (arg_node)));
    }
    if (NULL != AVIS_MIN (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Min];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), AVIS_MIN (arg_node)));
    }
    if (NULL != AVIS_MAX (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Max];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), AVIS_MAX (arg_node)));
    }
    if (NULL != AVIS_SCALARS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Scalars];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           AVIS_SCALARS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALconstraint( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise elements of avis node connected to vardec or arg.
 *
 ******************************************************************************/

node *
VISUALconstraint (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVopt (CONSTRAINT_EXPR (arg_node), arg_info);
    TRAVopt (CONSTRAINT_NEXT (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), "%s[label=Contraint];\n", node_name);

    if (NULL != CONSTRAINT_EXPR (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Expr];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           CONSTRAINT_EXPR (arg_node)));
    }
    if (NULL != CONSTRAINT_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           CONSTRAINT_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALwith3( node *arg_node, node *arg_info)
 *
 * @brief visualise the 1-dimensional with3.
 *
 * @param arg_node with3 node
 * @param arg_info info structure
 *
 * @return unchanged with3 node
 ******************************************************************************/
node *
VISUALwith3 (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVopt (WITH3_RANGES (arg_node), arg_info);
    TRAVopt (WITH3_OPERATIONS (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), "%s[label=With3];\n", node_name);

    if (NULL != WITH3_RANGES (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Ranges];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WITH3_RANGES (arg_node)));
    }
    if (NULL != WITH3_OPERATIONS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Operations];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           WITH3_OPERATIONS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *VISUALwiths( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
VISUALwiths (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (WITHS_WITH (arg_node), arg_info);
    TRAVopt (WITHS_NEXT (arg_node), arg_info);

    // ouput current node
    fprintf (INFO_FILE (arg_info), "%s[label=Withs];\n", node_name);

    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=With];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITHS_WITH (arg_node)));
    // add edge between two nodes with lable
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), WITHS_NEXT (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALrange( node *arg_node, node *arg_info)
 *
 * @brief visualise the range component of the with3.
 *
 * @param arg_node range node
 * @param arg_info info structure
 *
 * @return unchanged range node
 ******************************************************************************/

node *
VISUALrange (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traverse son node
    TRAVdo (RANGE_INDEX (arg_node), arg_info);
    TRAVdo (RANGE_LOWERBOUND (arg_node), arg_info);
    TRAVdo (RANGE_UPPERBOUND (arg_node), arg_info);
    TRAVopt (RANGE_CHUNKSIZE (arg_node), arg_info);
    TRAVdo (RANGE_BODY (arg_node), arg_info);
    TRAVopt (RANGE_RESULTS (arg_node), arg_info);
    TRAVopt (RANGE_IDXS (arg_node), arg_info);
    TRAVopt (RANGE_IIRR (arg_node), arg_info);
    TRAVopt (RANGE_NEXT (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), "%s[label=Range];\n", node_name);

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Index];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), RANGE_INDEX (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=LowerBound];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                       RANGE_LOWERBOUND (arg_node)));
    fprintf (INFO_FILE (arg_info), "%s -> %s [label=UpBound];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                       RANGE_UPPERBOUND (arg_node)));
    if (NULL != RANGE_CHUNKSIZE (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=ChunkSize];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           RANGE_CHUNKSIZE (arg_node)));
    }

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Body];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), RANGE_BODY (arg_node)));

    if (NULL != RANGE_RESULTS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Results];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           RANGE_RESULTS (arg_node)));
    }

    if (NULL != RANGE_IDXS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=IDXS];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), RANGE_IDXS (arg_node)));
    }

    if (NULL != RANGE_IIRR (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=IIRR];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), RANGE_IIRR (arg_node)));
    }

    if (NULL != RANGE_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), RANGE_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *VISUALdataflowgraph(node *arg_node, info *arg_info)
 *
 * @brief visualise the dataflowgraph and its dataflownodes
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
VISUALdataflowgraph (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *VISUALdataflownode(node *arg_node, info *arg_info)
 *
 * @brief visualise the dataflownode
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/

node *
VISUALdataflownode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALerror( node *arg_node, info *arg_info)
 *
 * description:
 *   this function print all the errors
 *
 ******************************************************************************/

node *
VISUALerror (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALimport (node * arg_node, info * arg_info)
 *
 * @brief visualise an import node
 *
 * @param arg_node import node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALimport (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (IMPORT_SYMBOL (arg_node), arg_info);

    // add edge between two nodes with lable

    fprintf (INFO_FILE (arg_info), "%s[label=Import];\n", node_name);
    if (NULL != IMPORT_SYMBOL (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Symbol];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           IMPORT_SYMBOL (arg_node)));
    }

    TRAVopt (IMPORT_NEXT (arg_node), arg_info);

    if (NULL != IMPORT_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           IMPORT_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALexport (node * arg_node, info * arg_info)
 *
 * @brief visualise an export node
 *
 * @param arg_node export node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALexport (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (EXPORT_SYMBOL (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), "%s[label=Export];\n", node_name);

    // add edge between two nodes with lable

    if (NULL != EXPORT_SYMBOL (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Symbol];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           EXPORT_SYMBOL (arg_node)));
    }

    TRAVopt (EXPORT_NEXT (arg_node), arg_info);

    if (NULL != EXPORT_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           EXPORT_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALuse (node * arg_node, info * arg_info)
 *
 * @brief visualise an use node
 *
 * @param arg_node use node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALuse (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (USE_SYMBOL (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), "%s[label=USE];\n", node_name);

    // add edge between two nodes with lable

    if (NULL != USE_SYMBOL (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Symbol];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), USE_SYMBOL (arg_node)));
    }

    TRAVopt (USE_NEXT (arg_node), arg_info);

    if (NULL != USE_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), USE_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALprovide (node * arg_node, info * arg_info)
 *
 * @brief visualise an provide node
 *
 * @param arg_node provide node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALprovide (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (PROVIDE_SYMBOL (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), "%s[label=Provide];\n", node_name);
    // add edge between two nodes with lable

    if (NULL != PROVIDE_SYMBOL (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Symbol];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           PROVIDE_SYMBOL (arg_node)));
    }

    TRAVopt (PROVIDE_NEXT (arg_node), arg_info);

    if (NULL != PROVIDE_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           PROVIDE_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALsymbol (node * arg_node, info * arg_info)
 *
 * @brief visualise an symbol node
 *
 * @param arg_node symbol node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALsymbol (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (SYMBOL_NEXT (arg_node), arg_info);

    // add edge between two nodes with lable
    if (NULL != SYMBOL_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s[label=Symbol];\n%s -> %s [label=Next];\n",
                 node_name, node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           SYMBOL_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALset (node * arg_node, info * arg_info)
 *
 * @brief visualise an set node
 *
 * @param arg_node set node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALset (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (SET_NEXT (arg_node), arg_info);

    // add edge between two nodes with lable
    if (NULL != SET_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s[label=Set];\n%s -> %s [label=Next];\n",
                 node_name, node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), SET_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALfunbundle (node *arg_node, info *arg_info)
 *
 * @brief visualise a function bundle
 *
 * @param arg_node funbundle node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALfunbundle (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (FUNBUNDLE_FUNDEF (arg_node), arg_info);
    TRAVopt (FUNBUNDLE_NEXT (arg_node), arg_info);

    // add edge between two nodes with lable

    if (NULL != FUNBUNDLE_FUNDEF (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s[label=FunBundle];\n%s -> %s [label=Fundef];\n",
                 node_name, node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           FUNBUNDLE_FUNDEF (arg_node)));
    }

    if (NULL != FUNBUNDLE_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           FUNBUNDLE_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALtfdag (node *arg_node, info *arg_info)
 *
 * @brief visualise a tfdag
 *
 * @param arg_node Ltfdag node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/

node *
VISUALtfdag (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (TFSPEC_DEFS (arg_node), arg_info);
    TRAVdo (TFSPEC_RELS (arg_node), arg_info);

    // add edge between two nodes with lable

    fprintf (INFO_FILE (arg_info), "%s[label=Tfdag];\n%s -> %s [label=Defs];\n",
             node_name, node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), TFSPEC_DEFS (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALtfspec (node *arg_node, info *arg_info)
 *
 * @brief visualise a tfsepc
 *
 * @param arg_node Ltfsepc node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/

node *
VISUALtfspec (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVdo (TFSPEC_DEFS (arg_node), arg_info);
    TRAVdo (TFSPEC_RELS (arg_node), arg_info);

    // add edge between two nodes with lable

    fprintf (INFO_FILE (arg_info), "%s[label=Tfspec];\n%s -> %s [label=Defs];\n",
             node_name, node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), TFSPEC_DEFS (arg_node)));

    fprintf (INFO_FILE (arg_info), "%s -> %s [label=Rels];\n", node_name,
             (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), TFSPEC_RELS (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALtfvertex (node *arg_node, info *arg_info)
 *
 * @brief visualise a tfvertex
 *
 * @param arg_node vertex node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALtfvertex (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (TFVERTEX_PARENTS (arg_node), arg_info);
    TRAVopt (TFVERTEX_CHILDREN (arg_node), arg_info);
    TRAVopt (TFVERTEX_NEXT (arg_node), arg_info);

    // add edge between two nodes with lable

    if (NULL != TFVERTEX_PARENTS (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Parents];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           TFVERTEX_PARENTS (arg_node)));
    }
    if (NULL != TFVERTEX_CHILDREN (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Children];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           TFVERTEX_CHILDREN (arg_node)));
    }
    if (NULL != TFVERTEX_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           TFVERTEX_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALtfrel (node *arg_node, info *arg_info)
 *
 * @brief visualise a tfrel
 *
 * @param arg_node tfrel node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALtfrel (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (TFREL_COND (arg_node), arg_info);
    TRAVopt (TFREL_NEXT (arg_node), arg_info);

    // add edge between two nodes with lable

    if (NULL != TFREL_COND (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s[label=Tfrel];\n%s -> %s [label=Cond];\n",
                 node_name, node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), TFREL_COND (arg_node)));
    }
    if (NULL != TFREL_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info), TFREL_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALtfedge (node *arg_node, info *arg_info)
 *
 * @brief visualise a tfedge
 *
 * @param arg_node tfedge node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALtfedge (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (TFEDGE_NEXT (arg_node), arg_info);

    // add edge between two nodes with lable

    if (NULL != TFEDGE_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s -> %s [label=Next];\n", node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           TFEDGE_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALtfarg (node *arg_node, info *arg_info)
 *
 * @brief visualise a tfarg
 *
 * @param arg_node tfarg node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALtypecomponentarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#if 0
  char *node_name = giveNodeName(arg_node, arg_info);

  //traver son nodes
  TRAVopt( TFARG_NEXT (arg_node), arg_info);

  //add edge between two nodes with lable

  if (NULL != TFARG_NEXT (arg_node)){
    fprintf( INFO_FILE( arg_info),
            "%s[label=Tfarg];\n%s -> %s [label=Next];\n", node_name,
            node_name, (char*) *LUTsearchInLutP( INFO_TABLE( arg_info),
                                                TFARG_NEXT( arg_node)));
  }
#endif

    fprintf (INFO_FILE (arg_info), "Typcomponent argument printing NOT IMPLEMENTED!\n");

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *VISUALtfexpr (node *arg_node, info *arg_info)
 *
 * @brief visualise a tfexpr
 *
 * @param arg_node tfexpr node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
VISUALtfexpr (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALlivevars( node *arg_node, info *arg_info)
 *
 * description:
 *   visualise N_livevars node.
 *
 ******************************************************************************/
node *
VISUALlivevars (node *arg_node, info *arg_info)
{
    char *node_name = giveNodeName (arg_node, arg_info);

    DBUG_ENTER ();

    // traver son nodes
    TRAVopt (LIVEVARS_NEXT (arg_node), arg_info);

    // add edge between two nodes with lable

    if (NULL != LIVEVARS_NEXT (arg_node)) {
        fprintf (INFO_FILE (arg_info), "%s[label=Livevars];\n%s -> %s [label=Next];\n",
                 node_name, node_name,
                 (char *)*LUTsearchInLutP (INFO_TABLE (arg_info),
                                           LIVEVARS_NEXT (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *VISUALnested_init( node *arg_node, info *arg_info)
 *
 *   @brief print N_nested_init node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
VISUALnested_init (node *arg_node, info *arg_info)
{
    char *number = STRitoa (INFO_NODENUMBER (arg_info)++);
    char *node_name = STRcat ("node", number);

    DBUG_ENTER ();

    // add node into table
    INFO_TABLE (arg_info)
      = LUTinsertIntoLutP (INFO_TABLE (arg_info), arg_node, node_name);

    // ouput current node
    fprintf (INFO_FILE (arg_info),
             "%s[label=false style=filled fillcolor=\"lightyellow\"];\n", node_name);

    number = MEMfree (number);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *VISUALdoVisual( node *syntax_tree)
 *
 * description:
 *   visualise the whole (sub-) tree behind the given node.
 *
 ******************************************************************************/

char *
VISUALdoVisual (node *syntax_tree)
{
    info *arg_info;
    char *filename;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    printf ("\n--------------begin visualizing----------------\n");
    filename = STRcat (global.outfilename, ".dot");
    INFO_FILE (arg_info) = fopen (filename, "w");
    fprintf (INFO_FILE (arg_info), "digraph my_fsm { \n");

    TRAVpush (TR_visual);
    TRAVdo (syntax_tree, arg_info);

    TRAVpop ();

    fprintf (INFO_FILE (arg_info), "} \n");
    fclose (INFO_FILE (arg_info));

    arg_info = FreeInfo (arg_info);

    printf ("\nIt may take few minutes, please wait...........\n");

    DBUG_RETURN (filename);
}
