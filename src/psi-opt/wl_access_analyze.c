

/*****************************************************************************
 *
 * file:   wl_access_analyze.h
 *
 * prefix: WLAA
 *
 * description:
 *
 *   This compiler module analyzes array accesses within withloops.
 *   It is used by the tilesize inference.
 *
 *   The following access macros are defined for the info-node:
 *
 *   INFO_WLAA_ACCESS(n)     ((access_t*)n->info2)
 *   INFO_WLAA_INDEXVAR(n)              (n->node[0])
 *   INFO_WLAA_FEATURE(n)    ((feature_t)n->lineno)
 *   INFO_WLAA_WOTYPE(n)    ((WithOpType)n->varno)
 *   INFO_WLAA_LASTLETIDS(n)            (n->info.ids)
 *   INFO_WLAA_BELOWAP(n)               (n->flag)
 *   INFO_WLAA_WLLEVEL(n)               (n->counter)
 *   INFO_WLAA_ACCESSVEC(n)    ((shpseg*)n->node[1])
 *   INFO_WLAA_TMPACCESS(n)  ((access_t*)n->node[2])
 *   INFO_WLAA_WLARRAY(n)               (n->node[3])
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
#include "wl_access_analyze.h"

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
 *   node *WLAccessAnalyze(node *arg_node)
 *
 * description:
 *
 *   This function initiates the WL access analyze scheme, i.e.
 *   act_tab is set to WLAA_tab and the traversal mechanism is started.
 *   Just as the other optimization schemes, WL access analyzing is performed
 *   on single function definitions rather than on the entire syntax tree.
 *
 ******************************************************************************/

node *
WLAccessAnalyze (node *arg_node)
{
    funptr *tmp_tab;
    node *arg_info;

    DBUG_ENTER ("WLAccessAnalyze");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "WLAccessAnalyze not initiated on N_fundef level");

    tmp_tab = act_tab;
    act_tab = wlaa_tab;
    arg_info = MakeInfo ();
    INFO_WLAA_WLLEVEL (arg_info) = 0;

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

#if 0

void WLAAprintAccesses(node* arg_node, node* arg_info)
{
  int      i, dim, iv;
  access_t *access;
  shpseg   *offset;
  
  DBUG_ENTER("WLAAprintAccesses");
  
  dim = SHP_SEG_SIZE;
  access = NCODE_ACCESS(arg_node);
  fprintf(outfile,"\n");
  INDENT;
  fprintf(outfile,"/*\n");
  INDENT;
  fprintf(outfile," * WLAA:\n");
  do {
    if (access == NULL) {
      INDENT;
      fprintf(outfile," * No accesses! \n");
    }
    else {
      dim = VARDEC_OR_ARG_DIM(ACCESS_ARRAY(access));
      iv = 0;
      offset = ACCESS_OFFSET(access);
      INDENT;
      fprintf(outfile," * %s : ",ACLT(ACCESS_CLASS(access)));
      switch (ACCESS_CLASS(access)) {
      case ACL_irregular:
        /*
         * here's no break missing !
         */
      case ACL_unknown:
        fprintf(outfile,"\n");
        access = NULL;
        break;
      case ACL_offset:
        iv = 1;
        /*
         * here's no break missing !
         */
      case ACL_const:
        do {
          if (offset == NULL)
            fprintf(outfile,"no offset\n");
          else {
            if (ACCESS_DIR(access) == ADIR_read)
              fprintf(outfile,"read ( %s[ %d",IV(iv),SHPSEG_SHAPE(offset,0));
            else
              /* break; */
              fprintf(outfile,"write( %s[ %d",IV(iv),SHPSEG_SHAPE(offset,0)); 
            for (i=1; i<dim; i++)
              fprintf(outfile,",%d",SHPSEG_SHAPE(offset,i));
            if (VARDEC_NAME(ACCESS_ARRAY(access)) != NULL)
              fprintf(outfile," ], %s)\n", VARDEC_NAME(ACCESS_ARRAY(access)));
            else
              fprintf(outfile," ], ??)\n");
            offset = SHPSEG_NEXT(offset);
          }
        } while (offset != NULL);
        access = ACCESS_NEXT(access);
        break;
      default:
        break;
      }
    }
  } while (access != NULL);
  INDENT;
  fprintf(outfile," */\n");
  INDENT;
  
  DBUG_VOID_RETURN;
}


void WLAAprintFeatures(node* arg_node, node* arg_info)
{
  feature_t feature;
  
  DBUG_ENTER("WLAAprintFeatures");
  
  feature = INFO_WLAA_FEATURE(INFO_PRINT_ACCESS(arg_info));
  fprintf(outfile,"\n");
  INDENT;
  fprintf(outfile,"/*\n");
  INDENT;
  fprintf(outfile," * WITH-LOOP features:\n");
  if (feature == FEATURE_NONE) {
    INDENT; 
    fprintf(outfile," *   no special features\n"); 
  }
  if ((feature & FEATURE_WL) == FEATURE_WL) {
    INDENT; 
    fprintf(outfile," *   with-loop containing array access(es)\n"); 
  }
  if ((feature & FEATURE_LOOP) == FEATURE_LOOP) {
    INDENT; 
    fprintf(outfile," *   while-/do-/for-loop containing array access(es)\n"); 
  }
  if ((feature & FEATURE_TAKE) == FEATURE_TAKE) {
    INDENT; 
    fprintf(outfile," *   primitive function take\n"); 
  }
  if ((feature & FEATURE_DROP) == FEATURE_DROP) {
    INDENT; 
    fprintf(outfile," *   primitive function drop\n"); 
  }
  if ((feature & FEATURE_AP) == FEATURE_AP) {
    INDENT; 
    fprintf(outfile," *   function aplication\n"); 
  }
  if ((feature & FEATURE_APSI) == FEATURE_APSI) {
    INDENT; 
    fprintf(outfile," *   primitive function psi with array return value\n"); 
  }
  if ((feature & FEATURE_MODA) == FEATURE_MODA) {
    INDENT; 
    fprintf(outfile," *   primitive function modarray\n"); 
  }
  if ((feature & FEATURE_CAT) == FEATURE_CAT) {
    INDENT; 
    fprintf(outfile," *   primitive function cat\n"); 
  }
  if ((feature & FEATURE_ROT) == FEATURE_ROT) {
    INDENT; 
    fprintf(outfile," *   primitive function rotate\n"); 
  }
  if ((feature & FEATURE_COND) == FEATURE_COND) {
    INDENT; 
    fprintf(outfile," *   conditional containing array access(es)\n"); 
  }
  if ((feature & FEATURE_AARI) == FEATURE_AARI) {
    INDENT; 
    fprintf(outfile," *   primitive arithmetic operation on arrays "
            "(without index vector access)\n"); 
  }
  INDENT;
  fprintf(outfile," */\n");
  INDENT;
  
  DBUG_VOID_RETURN;
}
#endif

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
        SHPSEG_NEXT (result) = Shpseg2Shpseg (coeff, SHPSEG_NEXT (shp_seg));
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
     *     access == INFO_WLAA_ACCESS(arg_info)
     *  || access == INFO_WLAA_TMPACCESS(arg_info)
     */
    while (access != NULL) {
        if ((ACCESS_IV (access) == IDS_VARDEC (INFO_WLAA_LASTLETIDS (arg_info)))
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
 *   node *WLAAfundef(node *arg_node, node *arg_info)
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
WLAAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAfundef");

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
 *   node *WLAAblock(node *arg_node, node *arg_info)
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
WLAAblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAblock");

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
 *   node *WLAAassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_assign node.
 *
 *   This function just realizes a post-order traversal of the code.
 *
 ******************************************************************************/

node *
WLAAassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAassign");

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
 *   node *WLAAnwith(node *arg_node, node *arg_info)
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
WLAAnwith (node *arg_node, node *arg_info)
{
    node *old_arg_info;

    DBUG_ENTER ("WLAAnwith");

    DBUG_ASSERT ((arg_info != NULL), "WLAAnwith called with empty info node!");

    /*
     * Store old arg_info for the case of nested with-loops.
     */
    old_arg_info = arg_info;
    arg_info = MakeInfo ();

    INFO_WLAA_ACCESS (arg_info) = NULL;
    INFO_WLAA_TMPACCESS (arg_info) = NULL;
    INFO_WLAA_INDEXVAR (arg_info) = IDS_VARDEC (NWITH_VEC (arg_node));
    INFO_WLAA_FEATURE (arg_info) = FEATURE_NONE;
    INFO_WLAA_WOTYPE (arg_info) = NWITH_TYPE (arg_node);
    INFO_WLAA_BELOWAP (arg_info) = 0;
    INFO_WLAA_WLLEVEL (arg_info) = INFO_WLAA_WLLEVEL (old_arg_info) + 1;
    INFO_WLAA_WLARRAY (arg_info) = INFO_WLAA_WLARRAY (old_arg_info);
    /*
     *  WLAAprint(arg_node, arg_info, _ENTER);
     */
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (old_arg_info) | FEATURE_WL;
    /*
     *  WLAAprint(arg_node, arg_info, _EXIT);
     */
    NWITH_WLAA (arg_node) = arg_info;
    arg_info = old_arg_info;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAncode(node *arg_node, node *arg_info)
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
WLAAncode (node *arg_node, node *arg_info)
{
    access_t *old_access;
    feature_t old_feature;

    DBUG_ENTER ("WLAAncode");

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    old_access = INFO_WLAA_ACCESS (arg_info);
    INFO_WLAA_ACCESS (arg_info) = NULL;
    old_feature = INFO_WLAA_FEATURE (arg_info);
    INFO_WLAA_FEATURE (arg_info) = FEATURE_NONE;

    if (NCODE_CBLOCK (arg_node) != NULL) {
        if ((INFO_WLAA_WOTYPE (arg_info) == WO_genarray)
            || (INFO_WLAA_WOTYPE (arg_info) == WO_modarray)) {
            INFO_WLAA_ACCESS (arg_info)
              = MakeAccess (INFO_WLAA_WLARRAY (arg_info),
                            ID_VARDEC (INFO_WLAA_INDEXVAR (arg_info)), ACL_offset, NULL,
                            ADIR_write, INFO_WLAA_ACCESS (arg_info));
            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
              = IntVec2Shpseg (1, 0, NULL, ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
        }

        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        /*
         *    WLAAprint(arg_node, arg_info, _EXIT);
         */
    }

    NCODE_ACCESS (arg_node) = INFO_WLAA_ACCESS (arg_info);
    NCODE_FEATURE (arg_node) = INFO_WLAA_FEATURE (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAwhile(node *arg_node, node *arg_info)
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
WLAAwhile (node *arg_node, node *arg_info)
{
    access_t *old_access;
    feature_t old_feature;

    DBUG_ENTER ("WLAAwhile");

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        old_access = INFO_WLAA_ACCESS (arg_info);
        old_feature = INFO_WLAA_FEATURE (arg_info);
        INFO_WLAA_ACCESS (arg_info) = NULL;
        INFO_WLAA_FEATURE (arg_info) = FEATURE_NONE;
    }

    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        if ((INFO_WLAA_ACCESS (arg_info) == NULL)
            && (INFO_WLAA_FEATURE (arg_info) == FEATURE_NONE)) {
            /*
             * Nothing harmful happened within the loop.
             */
            INFO_WLAA_ACCESS (arg_info) = old_access;
            INFO_WLAA_FEATURE (arg_info) = old_feature;
        } else {
            /*
             * Something harmful happened within the loop.
             */
            FreeAllAccess (INFO_WLAA_ACCESS (arg_info));
            INFO_WLAA_ACCESS (arg_info) = old_access;
            INFO_WLAA_FEATURE (arg_info)
              = INFO_WLAA_FEATURE (arg_info) | FEATURE_LOOP | old_feature;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAdo(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_do node.
 *
 *   This function is equivalent to WLAAwhile for while loops. The kind of loop
 *   makes no difference for the purpose of tile size inference.
 *
 ******************************************************************************/

node *
WLAAdo (node *arg_node, node *arg_info)
{
    access_t *old_access;
    feature_t old_feature;

    DBUG_ENTER ("WLAAdo");

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        old_access = INFO_WLAA_ACCESS (arg_info);
        old_feature = INFO_WLAA_FEATURE (arg_info);
        INFO_WLAA_ACCESS (arg_info) = NULL;
        INFO_WLAA_FEATURE (arg_info) = FEATURE_NONE;
    }

    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        if ((INFO_WLAA_ACCESS (arg_info) == NULL)
            && (INFO_WLAA_FEATURE (arg_info) == FEATURE_NONE)) {
            /*
             * Nothing harmful happened within the loop.
             */
            INFO_WLAA_ACCESS (arg_info) = old_access;
            INFO_WLAA_FEATURE (arg_info) = old_feature;
        } else {
            /*
             * Anything harmful happened within the loop.
             */
            FreeAllAccess (INFO_WLAA_ACCESS (arg_info));
            INFO_WLAA_ACCESS (arg_info) = old_access;
            INFO_WLAA_FEATURE (arg_info)
              = INFO_WLAA_FEATURE (arg_info) | FEATURE_LOOP | old_feature;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
WLAAcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAcond");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAlet(node *arg_node, node *arg_info)
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
WLAAlet (node *arg_node, node *arg_info)
{
    ids *var;
    access_t *access;

    DBUG_ENTER ("WLAAlet");

    INFO_WLAA_LASTLETIDS (arg_info) = LET_IDS (arg_node);

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_Nwith)
        INFO_WLAA_WLARRAY (arg_info) = IDS_VARDEC (LET_IDS (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are within a with-loop.
         */
        INFO_WLAA_LASTLETIDS (arg_info) = NULL;

        var = LET_IDS (arg_node);

        while (var != NULL) {
            access = INFO_WLAA_ACCESS (arg_info);
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
 *   node *WLAAprf(node *arg_node, node *arg_info)
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
WLAAprf (node *arg_node, node *arg_info)
{
    int i;
    int access_flag = CURRENT_A;
    int *offset;
    access_t *access;
    node *arg_node_arg1, *arg_node_arg2;

    DBUG_ENTER ("WLAAprf");

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are within a with-loop.
         */
        arg_node_arg1 = PRF_ARG1 (arg_node);
        arg_node_arg2 = PRF_ARG2 (arg_node);

        switch (PRF_PRF (arg_node)) {
        case F_psi:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_psi"));

            if (IDS_DIM (INFO_WLAA_LASTLETIDS (arg_info)) == SCALAR) {

                DBUG_ASSERT ((NODE_TYPE (arg_node_arg1) == N_id),
                             "1st arg of psi is not variable");
                DBUG_ASSERT ((NODE_TYPE (arg_node_arg2) == N_id),
                             "2nd arg of psi is not variable");

                INFO_WLAA_ACCESS (arg_info)
                  = MakeAccess (ID_VARDEC (arg_node_arg2), ID_VARDEC (arg_node_arg1),
                                ACL_unknown, NULL, ADIR_read,
                                INFO_WLAA_ACCESS (arg_info));

                if (ACCESS_IV (INFO_WLAA_ACCESS (arg_info))
                    == INFO_WLAA_INDEXVAR (arg_info)) {
                    /*
                     * The array is accessed by the index vector of the surrounding
                     * with-loop.
                     */
                    ACCESS_CLASS (INFO_WLAA_ACCESS (arg_info)) = ACL_offset;

                    DBUG_ASSERT ((ID_DIM (arg_node_arg2) > 0),
                                 "Unknown dimension for 2nd arg of psi");

                    ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                      = IntVec2Shpseg (1, 0, NULL,
                                       ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                } else if (ID_INTVEC (arg_node_arg1) != NULL) {
                    ACCESS_CLASS (INFO_WLAA_ACCESS (arg_info)) = ACL_const;

                    DBUG_ASSERT ((ID_VECLEN (arg_node_arg1) > SCALAR),
                                 "propagated constant vector is not of node type "
                                 "N_array");

                    ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                      = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg1),
                                       ID_INTVEC (arg_node_arg1),
                                       ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                } else {
                    /*
                     * The first arg of psi is a variable. The offset of the access
                     * have to be infered later.
                     */
                }
            } else {
                INFO_WLAA_FEATURE (arg_info)
                  = INFO_WLAA_FEATURE (arg_info) | FEATURE_APSI;
            }
            break;

        case F_take:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_take"));
            INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_TAKE;
            break;

        case F_drop:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_drop"));
            INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_DROP;
            break;

        case F_cat:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_cat"));
            INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_CAT;
            break;

        case F_rotate:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_rotate"));
            INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_ROT;
            break;

        case F_modarray:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_modarray"));
            INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_MODA;
            break;

        case F_add_SxA:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_add_SxA"));
            access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_WLAA_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (NODE_TYPE (arg_node_arg1) == N_num) {
                    if (ID_VARDEC (arg_node_arg2) == INFO_WLAA_INDEXVAR (arg_info)) {
                        ACCESS_CLASS (access) = ACL_offset;
                        offset = Malloc (VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access))
                                         * sizeof (int));
                        for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)); i++) {
                            offset[i] = NUM_VAL (arg_node_arg1);
                        }
                        ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                          = IntVec2Shpseg (1, VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)),
                                           offset,
                                           ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                    } else if (ID_INTVEC (arg_node_arg2) != NULL) {
                        ACCESS_CLASS (access) = ACL_const;
                        ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                          = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg2),
                                           ID_INTVEC (arg_node_arg2),
                                           ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                    } else { /* arg2 is neither the indexvar nor a constant */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else { /* NODE_TYPE(arg_node_arg1) == N_id */
                    ACCESS_CLASS (access) = ACL_irregular;
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_add_AxS:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_add_AxS"));
            access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_WLAA_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (NODE_TYPE (arg_node_arg2) == N_num) {
                    if (ID_VARDEC (arg_node_arg1) == INFO_WLAA_INDEXVAR (arg_info)) {
                        ACCESS_CLASS (access) = ACL_offset;
                        offset = Malloc (VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access))
                                         * sizeof (int));
                        for (i = 0; i < VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)); i++) {
                            offset[i] = NUM_VAL (arg_node_arg2);
                        }
                        ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                          = IntVec2Shpseg (1, VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access)),
                                           offset,
                                           ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                    } else if (ID_INTVEC (arg_node_arg1) != NULL) {
                        ACCESS_CLASS (access) = ACL_const;
                        ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                          = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg1),
                                           ID_INTVEC (arg_node_arg1),
                                           ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                    } else { /* arg1 is neither the indexvar nor a constant */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else { /* NODE_TYPE(arg_node_arg2) == N_id */
                    ACCESS_CLASS (access) = ACL_irregular;
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_add_AxA:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_add_AxA"));

            DBUG_ASSERT ((NODE_TYPE (arg_node_arg1) == N_id)
                           && (NODE_TYPE (arg_node_arg2) == N_id),
                         ("WLAA is only possible with constant folding,"
                          " N_id exspected !"));

            access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_WLAA_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (ID_VARDEC (arg_node_arg1) == INFO_WLAA_INDEXVAR (arg_info)) {
                    if (ID_INTVEC (arg_node_arg2) != NULL) {
                        ACCESS_CLASS (access) = ACL_offset;
                        ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                          = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg2),
                                           ID_INTVEC (arg_node_arg2),
                                           ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                    } else { /* arg2 is not constant */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else {
                    if (ID_VARDEC (arg_node_arg2) == INFO_WLAA_INDEXVAR (arg_info)) {
                        if (ID_INTVEC (arg_node_arg1) != NULL) {
                            ACCESS_CLASS (access) = ACL_offset;
                            ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                              = IntVec2Shpseg (1, ID_VECLEN (arg_node_arg1),
                                               ID_INTVEC (arg_node_arg1),
                                               ACCESS_OFFSET (
                                                 INFO_WLAA_ACCESS (arg_info)));
                        } else { /* arg1 is not constant */
                            ACCESS_CLASS (access) = ACL_irregular;
                        }
                    } else { /* None of the arguments is the index vector! */

                        DBUG_ASSERT ((ID_VECLEN (arg_node_arg1) != NULL)
                                       || (ID_VECLEN (arg_node_arg2) != NULL),
                                     ("WLAA is only possible with constant folding,"
                                      " variable exspected !"));

                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_sub_AxS:
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_sub_AxS"));
            access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_WLAA_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (ID_VARDEC (arg_node_arg1) == INFO_WLAA_INDEXVAR (arg_info)) {
                    if (NODE_TYPE (arg_node_arg2) == N_num) {
                        ACCESS_CLASS (access) = ACL_offset;
                        ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                          = IntVec2Shpseg (-1, ID_VECLEN (arg_node_arg2),
                                           ID_INTVEC (arg_node_arg2),
                                           ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                    } else {
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                }
            }
            break;

        case F_sub_AxA:
            DBUG_ASSERT ((NODE_TYPE (arg_node_arg1) == N_id)
                           && (NODE_TYPE (arg_node_arg2) == N_id),
                         ("WLAA is only possible with constant folding,"
                          " N_id exspected !"));

            access = SearchAccess (INFO_WLAA_ACCESS (arg_info), arg_info);
            if (access == NULL) {
                access = SearchAccess (INFO_WLAA_TMPACCESS (arg_info), arg_info);
                access_flag = TEMP_A;
            }
            if (access != NULL) {
                if (ID_VARDEC (arg_node_arg1) == INFO_WLAA_INDEXVAR (arg_info)) {
                    DBUG_PRINT ("WLAA_INFO", ("primitive function F_sub_AxA"));
                    if (ID_INTVEC (arg_node_arg2) != NULL) {
                        ACCESS_CLASS (access) = ACL_offset;
                        ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info))
                          = IntVec2Shpseg (-1, ID_VECLEN (arg_node_arg2),
                                           ID_INTVEC (arg_node_arg2),
                                           ACCESS_OFFSET (INFO_WLAA_ACCESS (arg_info)));
                    } else { /* arg2 is not constant */
                        ACCESS_CLASS (access) = ACL_irregular;
                    }
                } else if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                    DBUG_PRINT ("WLAA_INFO",
                                ("primitive function not inferable or unknown"));
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
                }
            } else { /* access == NULL */
                if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                    DBUG_PRINT ("WLAA_INFO",
                                ("primitive function not inferable or unknown"));
                    INFO_WLAA_FEATURE (arg_info)
                      = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
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
            DBUG_PRINT ("WLAA_INFO", ("primitive function not inferable or unknown"));
            if (!IsIndexVect (IDS_TYPE (INFO_WLAA_LASTLETIDS (arg_info)))) {
                INFO_WLAA_FEATURE (arg_info)
                  = INFO_WLAA_FEATURE (arg_info) | FEATURE_AARI;
            }
            break;

        case F_idx_psi:
        case F_idx_modarray:
            /*
             * These functions are only introduced by index vector elimination,
             * however tile size inference must always be applied before index
             * vector elimination.
             */
            DBUG_PRINT ("WLAA_INFO", ("primitive function F_idx_psi | F_idx_modarray"));
            DBUG_ASSERT (1, "primitive function idx_psi or idx_modarray found during "
                            "tile size selection");
            break;

        default:
            DBUG_PRINT ("WLAA_INFO",
                        ("primitive function which do not deal with arrays"));
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
 *   node *WLAAap(node *arg_node, node *arg_info)
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
WLAAap (node *arg_node, node *arg_info)
{
    ids *ret_var;

    DBUG_ENTER ("WLAAap");

    if (INFO_WLAA_WLLEVEL (arg_info) > 0) {
        /*
         * Here, we are beyond a with-loop.
         */
        ret_var = INFO_WLAA_LASTLETIDS (arg_info);

        while (ret_var != NULL) {
            if (IDS_DIM (ret_var) != SCALAR) {
                INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_AP;
                break;
            }
            ret_var = IDS_NEXT (ret_var);
        }

        if (!(INFO_WLAA_FEATURE (arg_info) & FEATURE_AP)) {
            /*
             * FEATURE_AP has not been set yet.
             */
            if (AP_ARGS (arg_node) != NULL) {
                INFO_WLAA_BELOWAP (arg_info) = 1;
                AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
                INFO_WLAA_BELOWAP (arg_info) = 0;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLAAid(node *arg_node, node *arg_info)
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
WLAAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLAAid");

    if ((INFO_WLAA_WLLEVEL (arg_info) > 0) && (INFO_WLAA_BELOWAP (arg_info))) {
        if (ID_DIM (arg_node) != SCALAR) {
            INFO_WLAA_FEATURE (arg_info) = INFO_WLAA_FEATURE (arg_info) | FEATURE_AP;
        }
    }

    DBUG_RETURN (arg_node);
}
