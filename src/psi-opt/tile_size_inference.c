/*
 *
 * $Log$
 * Revision 2.4  1999/04/12 18:00:54  bs
 * Two functions added: TSIprintAccesses and TSIprintFestures.
 *
 * Revision 2.3  1999/04/08 12:49:37  bs
 * The TSI is analysing withloops now.
 *
 * Revision 2.2  1999/03/15 15:49:54  bs
 * access macros changed
 *
 * Revision 2.1  1999/02/23 12:43:17  sacbase
 * new release made
 *
 * Revision 1.1  1999/01/15 15:31:06  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   tile_size_inference.c
 *
 * prefix: TSI
 *
 * description:
 *
 *   This compiler module realizes an inference scheme for the selection
 *   of appropriate tile sizes. This is used by the code generation in
 *   order to create tiled target code.
 *
 *   The following access macros are defined for the info-node:
 *
 *   INFO_TSI_ACCESS(n)     ((access_t*)n->info2)
 *   INFO_TSI_INDEXVAR(n)   (n->node[0])
 *   INFO_TSI_FEATURE(n)    ((feature_t)n->lineno)
 *   INFO_TSI_WOTYPE(n)     ((WithOpType)n->varno)
 *   INFO_TSI_LASTLETIDS(n) (n->info.ids)
 *   INFO_TSI_BELOWAP(n)    (n->flag)
 *   INFO_TSI_WLLEVEL(n)    (n->counter)
 *   INFO_TSI_ACCESSVEC(n)  ((shpseg*)n->node[1])
 *   INFO_TSI_TMPACCESS(n)  ((access_t*)n->node[2])
 *
 *****************************************************************************/

#include <stdlib.h>
#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "print.h"
#include "tile_size_inference.h"

#define ACLT(arg)                                                                        \
    (arg == ACL_unknown)                                                                 \
      ? ("ACL_unknown")                                                                  \
      : ((arg == ACL_irregular)                                                          \
           ? ("ACL_irregular")                                                           \
           : ((arg == ACL_offset) ? ("ACL_offset")                                       \
                                  : ((arg == ACL_const) ? ("ACL_const") : (""))))

#define IV(a) ((a) == 0) ? ("") : ("iv + ")

#define CURRENT_A 0
#define TEMP_A 1
#define _EXIT 0
#define _ENTER 1

/******************************************************************************
 *
 * function:
 *   node *TileSizeInference(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   This function initiates the tile size inference scheme, i.e.
 *   act_tab is set to tsi_tab and the traversal mechanism is started.
 *   Just as the other optimization schemes, tile size selection is performed
 *   on single function definitions rather than on the entire syntax tree.
 *
 ******************************************************************************/

node *
TileSizeInference (node *arg_node)
{
    funptr *tmp_tab;
    node *arg_info;

    DBUG_ENTER ("TileSizeInference");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "Tile size selection not initiated on N_fundef level");

    tmp_tab = act_tab;
    act_tab = tsi_tab;
    outfile = stdout;
    indent = 2;
    arg_info = MakeInfo ();
    INFO_TSI_WLLEVEL (arg_info) = 0;

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

static void
TSIprint (node *arg_node, node *arg_info, int status)
{
    int i, dim;
    access_t *access;
    shpseg *offset;

    DBUG_ENTER ("TSIprint");

    switch (NODE_TYPE (arg_node)) {
    case N_Nwith:
        if (status == _ENTER) {
            indent = indent + 2;
            INDENT;
            fprintf (outfile, "WL_LEVEL: %d \n", INFO_TSI_WLLEVEL (arg_info));
        } else
            indent = indent - 2;
        break;
    case N_Ncode:
        dim = SHP_SEG_SIZE;
        if (status == _EXIT) {
            access = INFO_TSI_ACCESS (arg_info);
            do {
                if (access == NULL) {
                    INDENT;
                    fprintf (outfile, "No accesses! \n");
                } else {
                    dim = VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access));
                    offset = ACCESS_OFFSET (access);
                    INDENT;
                    fprintf (outfile, "Access-class: ");
                    fprintf (outfile, "%s\n", ACLT (ACCESS_CLASS (access)));
                    INDENT;
                    do {
                        INDENT;
                        if (offset == NULL)
                            fprintf (outfile, "no offset\n");
                        else {
                            fprintf (outfile, "read( [ %d", SHPSEG_SHAPE (offset, 0));
                            for (i = 1; i < dim; i++)
                                fprintf (outfile, ",%d", SHPSEG_SHAPE (offset, i));
                            fprintf (outfile, " ], %s)\n",
                                     VARDEC_NAME (ACCESS_ARRAY (access)));
                            offset = SHPSEG_NEXT (offset);
                        }
                    } while (offset != NULL);
                    access = ACCESS_NEXT (access);
                }
                fprintf (outfile, "\n");
            } while (access != NULL);
        }
        break;
    default:
        /*
         * Nothing to do!
         */
        break;
    }

    DBUG_VOID_RETURN;
}

void
TSIprintAccesses (node *arg_node, node *arg_info)
{
    int i, dim, iv;
    access_t *access;
    shpseg *offset;

    DBUG_ENTER ("TSIprintAccesses");

    dim = SHP_SEG_SIZE;
    access = NCODE_ACCESS (arg_node);
    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "/*\n");
    INDENT;
    fprintf (outfile, " * TSI:\n");
    do {
        if (access == NULL) {
            INDENT;
            fprintf (outfile, " * No accesses! \n");
        } else {
            dim = VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access));
            iv = 0;
            offset = ACCESS_OFFSET (access);
            INDENT;
            fprintf (outfile, " * %s : ", ACLT (ACCESS_CLASS (access)));
            switch (ACCESS_CLASS (access)) {
            case ACL_irregular:
                /*
                 * here's no break missing !
                 */
            case ACL_unknown:
                fprintf (outfile, "\n");
                break;
            case ACL_offset:
                iv = 1;
                /*
                 * here's no break missing !
                 */
            case ACL_const:
                do {
                    if (offset == NULL)
                        fprintf (outfile, "no offset\n");
                    else {
                        fprintf (outfile, "read( %s[ %d", IV (iv),
                                 SHPSEG_SHAPE (offset, 0));
                        for (i = 1; i < dim; i++)
                            fprintf (outfile, ",%d", SHPSEG_SHAPE (offset, i));
                        fprintf (outfile, " ], %s)\n",
                                 VARDEC_NAME (ACCESS_ARRAY (access)));
                        offset = SHPSEG_NEXT (offset);
                    }
                } while (offset != NULL);
                access = ACCESS_NEXT (access);
                break;
            default:
                break;
            }
        }
    } while (access != NULL);
    INDENT;
    fprintf (outfile, " */\n");
    INDENT;

    DBUG_VOID_RETURN;
}

void
TSIprintFeatures (node *arg_node, node *arg_info)
{
    feature_t feature;

    DBUG_ENTER ("TSIprintFeatures");

    feature = INFO_TSI_FEATURE (INFO_PRINT_ACCESS (arg_info));
    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "/*\n");
    INDENT;
    fprintf (outfile, " * WITH-LOOP features:\n");
    if (feature == FEATURE_NONE) {
        INDENT;
        fprintf (outfile, " *   no special features\n");
    }
    if ((feature & FEATURE_WL) == FEATURE_WL) {
        INDENT;
        fprintf (outfile, " *   with-loop containing array access(es)\n");
    }
    if ((feature & FEATURE_LOOP) == FEATURE_LOOP) {
        INDENT;
        fprintf (outfile, " *   while-/do-/for-loop containing array access(es)\n");
    }
    if ((feature & FEATURE_TAKE) == FEATURE_TAKE) {
        INDENT;
        fprintf (outfile, " *   primitive function take\n");
    }
    if ((feature & FEATURE_DROP) == FEATURE_DROP) {
        INDENT;
        fprintf (outfile, " *   primitive function drop\n");
    }
    if ((feature & FEATURE_AP) == FEATURE_AP) {
        INDENT;
        fprintf (outfile, " *   function aplication\n");
    }
    if ((feature & FEATURE_APSI) == FEATURE_APSI) {
        INDENT;
        fprintf (outfile, " *   primitive function psi with array return value\n");
    }
    if ((feature & FEATURE_MODA) == FEATURE_MODA) {
        INDENT;
        fprintf (outfile, " *   primitive function modarray\n");
    }
    if ((feature & FEATURE_CAT) == FEATURE_CAT) {
        INDENT;
        fprintf (outfile, " *   primitive function cat\n");
    }
    if ((feature & FEATURE_ROT) == FEATURE_ROT) {
        INDENT;
        fprintf (outfile, " *   primitive function rotate\n");
    }
    if ((feature & FEATURE_COND) == FEATURE_COND) {
        INDENT;
        fprintf (outfile, " *   conditional containing array access(es)\n");
    }
    if ((feature & FEATURE_AARI) == FEATURE_AARI) {
        INDENT;
        fprintf (outfile, " *   primitive arithmetic operation on arrays "
                          "(without index vector access)\n");
    }
    INDENT;
    fprintf (outfile, " */\n");
    INDENT;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   nums   *NumVect2NumsList(int coeff, node *exprs)
 *
 * description:
 *   This function converts an N_exprs list from an N_array node into a list
 *   of type nums, i.e. the elements of a constant int vector are transformed
 *   into a format suitable to build a shape segment. During the format
 *   conversion each element is multiplied by <coeff>.
 *
 * remark:
 *   Here, a recursive implementation is used although recrsion is not without
 *   problems concerning the stack size and the performance. However, a
 *   recursive implementation where the order of the list is untouched is much
 *   easier than an iterative one and this function is only used for index
 *   vectors, i.e. the lists are quite short.
 *
 ******************************************************************************/
static nums *
NumVect2NumsList (int coeff, node *exprs)
{
    nums *tmp;

    DBUG_ENTER ("NumVect2NumsList");

    DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs),
                 "Illegal node type in call to function NumVect2NumsList()");

    if (exprs == NULL) {
        tmp = NULL;
    } else {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_num),
                     "Illegal expression in call to function NumVect2NumsList()");

        tmp = MakeNums (coeff * NUM_VAL (EXPRS_EXPR (exprs)),
                        NumVect2NumsList (coeff, EXPRS_NEXT (exprs)));
    }

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *   shpseg *Shpseg2Shpseg(int coeff, shpseg *shp_seg)
 *
 * description:
 *   This function creates a new shape vector. The new shape vector
 *   is made of the shp_seg multiplied by <coeff>.
 *
 * remark:
 *   Here, a recursive implementation is used although recrsion is not without
 *   problems concerning the stack size and the performance. However, a
 *   recursive implementation where the order of the list is untouched is much
 *   easier than an iterative one and this function is only used for index
 *   vectors, i.e. the lists are quite short.
 *
 ******************************************************************************/

static shpseg *
Shpseg2Shpseg (int coeff, shpseg *shp_seg)
{
    int i;
    shpseg *result;

    DBUG_ENTER ("Shpseg2Shpseg");

    if (shp_seg == NULL)
        result = NULL;
    else {
        result = Malloc (sizeof (shpseg));
        for (i = 0; i < SHP_SEG_SIZE; i++)
            SHPSEG_SHAPE (result, i) = coeff * SHPSEG_SHAPE (shp_seg, i);
#if 0
    SHPSEG_NEXT(result) = Shpseg2Shpseg(coeff, SHPSEG_NEXT(shp_seg));
#endif
        result->next = Shpseg2Shpseg (coeff, result->next);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   shpseg *IntVec2Shpseg(int coeff, int length, int *intvec)
 *
 * description:
 *   This functions convert a vector of constant integers into a shape vector.
 *   During the format conversion each element is multiplied by <coeff>.
 *
 ******************************************************************************/

static shpseg *
IntVec2Shpseg (int coeff, int length, int *intvec, shpseg *next)
{
    int i;
    shpseg *result;

    DBUG_ENTER ("IntVec2Shpseg");

    result = Malloc (sizeof (shpseg));

    if (length == 0) {
        for (i = 0; i < SHP_SEG_SIZE; i++)
            SHPSEG_SHAPE (result, i) = 0;
    } else {
        for (i = 0; i < length; i++)
            SHPSEG_SHAPE (result, i) = coeff * intvec[i];
        for (i = length; i < SHP_SEG_SIZE; i++)
            SHPSEG_SHAPE (result, i) = 0;
    }
    SHPSEG_NEXT (result) = next;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int IsIndexVect(types *type)
 *
 * description:
 *
 *   This function classifies certain types as "index vectors", i.e. small
 *   integer vectors on which primitive arithmetic operations shall not harm
 *   tile size inference.
 *
 ******************************************************************************/

static int
IsIndexVect (types *type)
{
    int res = FALSE;

    DBUG_ENTER ("IsIndexVect");

    if ((TYPES_BASETYPE (type) == T_int) && (TYPES_DIM (type) == 1)
        && (TYPES_SHAPE (type, 0) < SHP_SEG_SIZE)) {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   access_t *SearchAccess(node *arg_info)
 *
 * description:
 *
 *   This function traverses all accesses stored in the given arg_info node.
 *   It returns the first one whose index vector is exactly the same variable
 *   that is instantiated in the last let-expression. Additionally, the access
 *   class still must be ACL_unknown.
 *
 ******************************************************************************/

static access_t *
SearchAccess (access_t *access, node *arg_info)
{
    DBUG_ENTER ("SearchAccess");
    /*
     *     access == INFO_TSI_ACCESS(arg_info)
     *  || access == INFO_TSI_TMPACCESS(arg_info)
     */
    while (access != NULL) {
        if ((ACCESS_IV (access) == IDS_VARDEC (INFO_TSI_LASTLETIDS (arg_info)))
            && (ACCESS_CLASS (access) == ACL_unknown)) {
            break;
        }
        access = ACCESS_NEXT (access);
    }

    DBUG_RETURN (access);
}

/******************************************************************************
 *
 * function:
 *   node *TSIfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_fundef node.
 *
 *   The traversal is limited to the function body, arguments and remaining
 *   functions are not traversed.
 *
 ******************************************************************************/

node *
TSIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        /*
         * Nodetype of FUNDEF_BODY(arg_node) is N_block.
         */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIblock(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_block node.
 *
 *   The traversal is limited to the assignments chain, variable declarations
 *   are not traversed.
 *
 *
 ******************************************************************************/

node *
TSIblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
        /*
         * Nodetype of BLOCK_INSTR(arg_node) is N_assign or N_empty.
         */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_assign node.
 *
 *   This function just realizes a post-order traversal of the code.
 *
 ******************************************************************************/

node *
TSIassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "N_assign node without instruction.");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSInwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Nwith node.
 *
 *   Here, the arg_info node is created and initialized. Everywhere beyond
 *   an N_Nwith node, arg_info is not NULL. See general remarks about usage
 *   of arg_info in this compiler module.
 *
 *   In the case of a nested with-loop, the old arg_info is stored and the
 *   whole game is restarted. Afterwards, the outer with-loops features bit
 *   mask is set accordingly.
 *
 ******************************************************************************/

node *
TSInwith (node *arg_node, node *arg_info)
{
    node *old_arg_info;

    DBUG_ENTER ("TSInwith");

    DBUG_ASSERT ((arg_info != NULL), "TSInwith called with empty info node!");

    /*
     * Store old arg_info for the case of nested with-loops.
     */
    old_arg_info = arg_info;
    arg_info = MakeInfo ();

    INFO_TSI_ACCESS (arg_info) = NULL;
    INFO_TSI_TMPACCESS (arg_info) = NULL;
    INFO_TSI_INDEXVAR (arg_info) = IDS_VARDEC (NWITH_VEC (arg_node));
    INFO_TSI_FEATURE (arg_info) = FEATURE_NONE;
    INFO_TSI_WOTYPE (arg_info) = NWITH_TYPE (arg_node);
    INFO_TSI_BELOWAP (arg_info) = 0;
    INFO_TSI_WLLEVEL (arg_info) = INFO_TSI_WLLEVEL (old_arg_info) + 1;

    TSIprint (arg_node, arg_info, _ENTER);

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (old_arg_info) | FEATURE_WL;

    TSIprint (arg_node, arg_info, _EXIT);

    NWITH_TSI (arg_node) = arg_info;
    arg_info = old_arg_info;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIncode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Ncode node.
 *
 *   The code block of the first operator is traversed, the information
 *   gathered is stored in this node, and the traversal continues with the
 *   following operator.
 *
 ******************************************************************************/

node *
TSIncode (node *arg_node, node *arg_info)
{
    access_t *old_access;
    feature_t old_feature;

    DBUG_ENTER ("TSIncode");

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    old_access = INFO_TSI_ACCESS (arg_info);
    INFO_TSI_ACCESS (arg_info) = NULL;
    old_feature = INFO_TSI_FEATURE (arg_info);
    INFO_TSI_FEATURE (arg_info) = FEATURE_NONE;

    if (NCODE_CBLOCK (arg_node) != NULL) {
        TSIprint (arg_node, arg_info, _ENTER);
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        TSIprint (arg_node, arg_info, _EXIT);
    }

    NCODE_ACCESS (arg_node) = INFO_TSI_ACCESS (arg_info);
    NCODE_FEATURE (arg_node) = INFO_TSI_FEATURE (arg_info);
    INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | old_feature;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIwhile(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_while node.
 *
 *   If a with-loop operator contains a sequential loop (for/do/while), then
 *   tile size inference has lost. The only exception is the case where the
 *   loop does not contain any array operations. The function detects exactly
 *   this and sets the features bit mask accordingly.
 *
 ******************************************************************************/

node *
TSIwhile (node *arg_node, node *arg_info)
{
    access_t *old_access;
    feature_t old_feature;

    DBUG_ENTER ("TSIwhile");

    if (INFO_TSI_WLLEVEL (arg_info) > 0) {
        old_access = INFO_TSI_ACCESS (arg_info);
        old_feature = INFO_TSI_FEATURE (arg_info);
        INFO_TSI_ACCESS (arg_info) = NULL;
        INFO_TSI_FEATURE (arg_info) = FEATURE_NONE;
    }

    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    if (INFO_TSI_WLLEVEL (arg_info) > 0) {
        if ((INFO_TSI_ACCESS (arg_info) == NULL)
            && (INFO_TSI_FEATURE (arg_info) == FEATURE_NONE)) {
            /*
             * Nothing harmful happened within the loop.
             */
            INFO_TSI_ACCESS (arg_info) = old_access;
            INFO_TSI_FEATURE (arg_info) = old_feature;
        } else {
            /*
             * Something harmful happened within the loop.
             */
            FreeAllAccess (INFO_TSI_ACCESS (arg_info));
            INFO_TSI_ACCESS (arg_info) = old_access;
            INFO_TSI_FEATURE (arg_info)
              = INFO_TSI_FEATURE (arg_info) | FEATURE_LOOP | old_feature;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIdo(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_do node.
 *
 *   This function is equivalent to TSIwhile for while loops. The kind of loop
 *   makes no difference for the purpose of tile size inference.
 *
 ******************************************************************************/

node *
TSIdo (node *arg_node, node *arg_info)
{
    access_t *old_access;
    feature_t old_feature;

    DBUG_ENTER ("TSIdo");

    if (INFO_TSI_WLLEVEL (arg_info) > 0) {
        old_access = INFO_TSI_ACCESS (arg_info);
        old_feature = INFO_TSI_FEATURE (arg_info);
        INFO_TSI_ACCESS (arg_info) = NULL;
        INFO_TSI_FEATURE (arg_info) = FEATURE_NONE;
    }

    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    if (INFO_TSI_WLLEVEL (arg_info) > 0) {
        if ((INFO_TSI_ACCESS (arg_info) == NULL)
            && (INFO_TSI_FEATURE (arg_info) == FEATURE_NONE)) {
            /*
             * Nothing harmful happened within the loop.
             */
            INFO_TSI_ACCESS (arg_info) = old_access;
            INFO_TSI_FEATURE (arg_info) = old_feature;
        } else {
            /*
             * Anything harmful happened within the loop.
             */
            FreeAllAccess (INFO_TSI_ACCESS (arg_info));
            INFO_TSI_ACCESS (arg_info) = old_access;
            INFO_TSI_FEATURE (arg_info)
              = INFO_TSI_FEATURE (arg_info) | FEATURE_LOOP | old_feature;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
TSIcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIcond");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_let node.
 *
 *   This function mainly sets the LASTLETIDS entry of the arg_info node
 *   correctly and traverses the left hand side of the let.
 *   Afterwards, it is checked whether any of the variables set by this let,
 *   has been used as an index vector in a psi operation and the respective
 *   access is still classified ACL_unknown. In this case, the access is
 *   re-classified as ACL_irregular. Any "regular" access would have already
 *   been handled during the traversal of the left hand side.
 *
 ******************************************************************************/

node *
TSIlet (node *arg_node, node *arg_info)
{
    ids *var;
    access_t *access;

    DBUG_ENTER ("TSIlet");

    if (INFO_TSI_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are within a with-loop.
         */
        INFO_TSI_LASTLETIDS (arg_info) = LET_IDS (arg_node);
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (INFO_TSI_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are within a with-loop.
         */
        INFO_TSI_LASTLETIDS (arg_info) = NULL;

        var = LET_IDS (arg_node);

        while (var != NULL) {
            access = INFO_TSI_ACCESS (arg_info);
            while (access != NULL) {
                if ((IDS_VARDEC (var) == ACCESS_IV (access))
                    && (ACCESS_CLASS (access) == ACL_unknown)) {
                    ACCESS_CLASS (access) = ACL_irregular;
                }
                access = ACCESS_NEXT (access);
            }
            var = IDS_NEXT (var);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIprf(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_prf node.
 *
 *   This function does most of the code analysis. See specific functions
 *   for more information.
 *
 ******************************************************************************/

node *
TSIprf (node *arg_node, node *arg_info)
{
    int i;
    int access_flag = CURRENT_A;
    int *offset;
    access_t *access;
    node *arg_node_arg1, *arg_node_arg2;

    DBUG_ENTER ("TSIprf");

    if (INFO_TSI_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are within a with-loop.
         */
        arg_node_arg1 = PRF_ARG1 (arg_node);
        arg_node_arg2 = PRF_ARG2 (arg_node);

        switch (PRF_PRF (arg_node)) {
        case F_psi:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_psi"));

            if (IDS_DIM (INFO_TSI_LASTLETIDS (arg_info)) == SCALAR) {

                DBUG_ASSERT ((NODE_TYPE (arg_node_arg1) == N_id),
                             "1st arg of psi is not variable");
                DBUG_ASSERT ((NODE_TYPE (arg_node_arg2) == N_id),
                             "2nd arg of psi is not variable");

                INFO_TSI_ACCESS (arg_info)
                  = MakeAccess (ID_VARDEC (arg_node_arg2), ID_VARDEC (arg_node_arg1),
                                ACL_unknown, NULL, INFO_TSI_ACCESS (arg_info));

                if (ACCESS_IV (INFO_TSI_ACCESS (arg_info))
                    == INFO_TSI_INDEXVAR (arg_info)) {
                    /*
                     * The array is accessed by the index vector of the surrounding
                     * with-loop.
                     */
                    ACCESS_CLASS (INFO_TSI_ACCESS (arg_info)) = ACL_offset;

                    DBUG_ASSERT ((ID_DIM (arg_node_arg2) > 0),
                                 "Unknown dimension for 2nd arg of psi");

                    ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                      = IntVec2Shpseg (1, 0, NULL,
                                       ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                } else if (ID_INTVEC (arg_node_arg1) != NULL) {
                    ACCESS_CLASS (INFO_TSI_ACCESS (arg_info)) = ACL_const;

                    DBUG_ASSERT ((ID_VECLEN (arg_node_arg1) > SCALAR),
                                 "propagated constant vector is not of node type "
                                 "N_array");

                    ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                      = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg1),
                                       ID_INTVEC (arg_node_arg1),
                                       ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                } else {
                    /*
                     * The first arg of psi is a variable. The offset of the access
                     * have to be infered later.
                     */
                }
            } else {
                INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_APSI;
            }
            break;

        case F_take:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_take"));
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_TAKE;
            break;

        case F_drop:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_drop"));
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_DROP;
            break;

        case F_cat:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_cat"));
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_CAT;
            break;

        case F_rotate:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_rotate"));
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_ROT;
            break;

        case F_modarray:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_modarray"));
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_MODA;
            break;

        case F_add_SxA:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_add_SxA"));
            access = SearchAccess (INFO_TSI_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_TSI_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (NODE_TYPE (arg_node_arg1) == N_num) {
                    if (ID_VARDEC (arg_node_arg2) == INFO_TSI_INDEXVAR (arg_info)) {
                        ACCESS_CLASS (access) = ACL_offset;
                        offset = Malloc (VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access))
                                         * sizeof (int));
                        for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)); i++) {
                            offset[i] = NUM_VAL (arg_node_arg1);
                        }
                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                          = IntVec2Shpseg (1, VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)),
                                           offset,
                                           ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                    } else if (ID_INTVEC (arg_node_arg2) != NULL) {
                        ACCESS_CLASS (access) = ACL_const;
                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                          = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg2),
                                           ID_INTVEC (arg_node_arg2),
                                           ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                    } else { /* arg2 is neither the indexvar nor a constant */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else { /* NODE_TYPE(arg_node_arg1) == N_id */
                    ACCESS_CLASS (access) = ACL_irregular;
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_add_AxS:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_add_AxS"));
            access = SearchAccess (INFO_TSI_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_TSI_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (NODE_TYPE (arg_node_arg2) == N_num) {
                    if (ID_VARDEC (arg_node_arg1) == INFO_TSI_INDEXVAR (arg_info)) {
                        ACCESS_CLASS (access) = ACL_offset;
                        offset = Malloc (VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access))
                                         * sizeof (int));
                        for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)); i++) {
                            offset[i] = NUM_VAL (arg_node_arg2);
                        }
                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                          = IntVec2Shpseg (1, VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)),
                                           offset,
                                           ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                    } else if (ID_INTVEC (arg_node_arg1) != NULL) {
                        ACCESS_CLASS (access) = ACL_const;
                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                          = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg1),
                                           ID_INTVEC (arg_node_arg1),
                                           ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                    } else { /* arg1 is neither the indexvar nor a constant */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else { /* NODE_TYPE(arg_node_arg2) == N_id */
                    ACCESS_CLASS (access) = ACL_irregular;
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_add_AxA:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_add_AxA"));

            DBUG_ASSERT ((NODE_TYPE (arg_node_arg1) == N_id)
                           && (NODE_TYPE (arg_node_arg2) == N_id),
                         ("TSI is only possible with constant folding,"
                          " N_id exspected !"));

            access = SearchAccess (INFO_TSI_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_TSI_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (ID_VARDEC (arg_node_arg1) == INFO_TSI_INDEXVAR (arg_info)) {
                    if (ID_INTVEC (arg_node_arg2) != NULL) {
                        ACCESS_CLASS (access) = ACL_offset;
                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                          = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg2),
                                           ID_INTVEC (arg_node_arg2),
                                           ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                    } else { /* arg2 is not constant */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else {
                    if (ID_VARDEC (arg_node_arg2) == INFO_TSI_INDEXVAR (arg_info)) {
                        if (ID_INTVEC (arg_node_arg1) != NULL) {
                            ACCESS_CLASS (access) = ACL_offset;
                            ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                              = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg1),
                                               ID_INTVEC (arg_node_arg1),
                                               ACCESS_OFFSET (
                                                 INFO_TSI_ACCESS (arg_info)));
                        } else { /* arg1 is not constant */
                            ACCESS_CLASS (access) = ACL_irregular;
                        }
                    } else { /* None of the arguments is the index vector! */

                        DBUG_ASSERT ((ID_VECLEN (arg_node_arg1) != NULL)
                                       || (ID_VECLEN (arg_node_arg2) != NULL),
                                     ("TSI is only possible with constant folding,"
                                      " variable exspected !"));

                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_sub_AxS:
            DBUG_PRINT ("TSI_INFO", ("primitive function F_sub_AxS"));
            access = SearchAccess (INFO_TSI_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_TSI_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (ID_VARDEC (arg_node_arg1) == INFO_TSI_INDEXVAR (arg_info)) {
                    if (NODE_TYPE (arg_node_arg2) == N_num) {
                        ACCESS_CLASS (access) = ACL_offset;
                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                          = IntVec2Shpseg (-1, ID_VECLEN (arg_node_arg2),
                                           ID_INTVEC (arg_node_arg2),
                                           ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                    } else {
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_sub_AxA:
            DBUG_ASSERT ((NODE_TYPE (arg_node_arg1) == N_id)
                           && (NODE_TYPE (arg_node_arg2) == N_id),
                         ("TSI is only possible with constant folding,"
                          " N_id exspected !"));

            access = SearchAccess (INFO_TSI_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_TSI_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (ID_VARDEC (arg_node_arg1) == INFO_TSI_INDEXVAR (arg_info)) {
                    DBUG_PRINT ("TSI_INFO", ("primitive function F_sub_AxA"));
                    if (ID_INTVEC (arg_node_arg2) != NULL) {
                        ACCESS_CLASS (access) = ACL_offset;
                        ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info))
                          = IntVec2Shpseg (-1, ID_VECLEN (arg_node_arg2),
                                           ID_INTVEC (arg_node_arg2),
                                           ACCESS_OFFSET (INFO_TSI_ACCESS (arg_info)));
                    } else { /* arg2 is not constant */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    DBUG_PRINT ("TSI_INFO",
                                ("primitive function not inferable or unknown"));
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                    DBUG_PRINT ("TSI_INFO",
                                ("primitive function not inferable or unknown"));
                    INFO_TSI_FEATURE (arg_info)
                      = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_sub_SxA:
        case F_mul_SxA:
        case F_mul_AxS:
        case F_mul_AxA:
        case F_div_SxA:
        case F_div_AxS:
        case F_div_AxA:
            DBUG_PRINT ("TSI_INFO", ("primitive function not inferable or unknown"));
            if (!IsIndexVect (IDS_TYPE (INFO_TSI_LASTLETIDS (arg_info)))) {
                INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_AARI;
            }
            break;

        case F_idx_psi:
        case F_idx_modarray:
            /*
             * These functions are only introduced by index vector elimination,
             * however tile size inference must always be applied before index
             * vector elimination.
             */
            DBUG_PRINT ("TSI_INFO", ("primitive function F_idx_psi | F_idx_modarray"));
            DBUG_ASSERT (1, "primitive function idx_psi or idx_modarray found during "
                            "tile size selection");
            break;

        default:
            DBUG_PRINT ("TSI_INFO", ("primitive function which do not deal with arrays"));
            /*
             * Do nothing !
             *
             * All other primitive functions do not deal with arrays.
             */
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIap(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_ap node.
 *
 *   This function detects function applications within with-loop operators
 *   and sets the features bit mask accordingly. However, functions which do
 *   not deal with arrays are no problem for tile size inference. We could
 *   investigate this by an inter-functional analysis. But for now, we only
 *   check whether one of the arguments or one of the return values is of an
 *   array type.
 *
 ******************************************************************************/

node *
TSIap (node *arg_node, node *arg_info)
{
    ids *ret_var;

    DBUG_ENTER ("TSIap");

    if (INFO_TSI_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are beyond a with-loop.
         */
        ret_var = INFO_TSI_LASTLETIDS (arg_info);

        while (ret_var != NULL) {
            if (IDS_DIM (ret_var) != SCALAR) {
                INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_AP;
                break;
            }
            ret_var = IDS_NEXT (ret_var);
        }

        if (!(INFO_TSI_FEATURE (arg_info) & FEATURE_AP)) {
            /*
             * FEATURE_AP has not been set yet.
             */
            if (AP_ARGS (arg_node) != NULL) {
                INFO_TSI_BELOWAP (arg_info) = 1;
                AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
                INFO_TSI_BELOWAP (arg_info) = 0;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_id node.
 *
 *   This function applies only beyond an N_ap node within a with-loop
 *   operator. If the function argument is not of scalar type the AP bit
 *   of the features bit mask is set.
 *
 *
 ******************************************************************************/

node *
TSIid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIid");

    if ((INFO_TSI_WLLEVEL (arg_info) > 0) && (INFO_TSI_BELOWAP (arg_info))) {
        if (ID_DIM (arg_node) != SCALAR) {
            INFO_TSI_FEATURE (arg_info) = INFO_TSI_FEATURE (arg_info) | FEATURE_AP;
        }
    }

    DBUG_RETURN (arg_node);
}
