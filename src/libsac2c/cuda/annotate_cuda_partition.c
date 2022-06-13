/*****************************************************************************
 *
 * @defgroup
 *
 * description:
 *   This seems to be some half-done support for WLs where some partitions 
 *   actually do not require kernels....
 *   However, in its current form, it annotates ALL partitions as 
 *   cudarizable is the WL has been marked cudarizable.
 *
 *   In case code generation does support the cudarization of individual 
 *   partittions only, then this requires some reverse engineering and fixing!
 *
 *   sbs, April 2021
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file annotate_cuda_partition.c
 *
 * Prefix: ACUPTN
 *
 *****************************************************************************/
#include "annotate_cuda_partition.h"

#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"
#include "namespaces.h"
#include "constants.h"

/* This structure stores a list of arrays
 * which are (partly) copied to the destination
 * array together with the total amount of
 * data copied. If more than one array is
 * copied, their sizes will be compared and
 * the array with the largest size will be
 * selected to copied entirely to the destination
 * array before any kernels are called.
 */

typedef struct ARRAYLIST_STRUCT {
    node *avis;
    int size;
    struct ARRAYLIST_STRUCT *next;
} arraylist_struct;

/* Two traversal mode:
 * 1) mark_potential: mark those partitions that are potentially not cudarizable, i.e.
 *                    partitions with only one Assign: idx_sel.
 * 1) unmark_potential: unmark those partitions that are actually cudarizable. Since the
 *                      mark_potential traversal can potentially mark partions that are
 *                      cudarizable as not cudarizable. This can be later discoverred by
 *                      comparing the sizes of different copied arrays.
 */

typedef enum { mark_potential, unmark_potential } travmode_t;

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    travmode_t mode;
    node *arrayavis;
    arraylist_struct *arraylist;
    node *maxarrayavis;
};

#define INFO_MODE(n) (n->mode)
#define INFO_ARRAYAVIS(n) (n->arrayavis)
#define INFO_ARRAYLIST(n) (n->arraylist)
#define INFO_MAXARRAYAVIS(n) (n->maxarrayavis)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ARRAYAVIS (result) = NULL;
    INFO_ARRAYLIST (result) = NULL;
    INFO_MAXARRAYAVIS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *ACUPTNdoAnnotateCUDAPartition( node *syntax_tree)
 *
 *****************************************************************************/
node *
ACUPTNdoAnnotateCUDAPartition (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();
    INFO_MODE (info) = mark_potential;
    TRAVpush (TR_acuptn);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn arraylist_struct *MakeALS( arraylist_struct *als, node *avis, int size)
 *
 * @brief
 *
 *****************************************************************************/
/*
static arraylist_struct *MakeALS( arraylist_struct *als, node *avis, int size)
{
  arraylist_struct *res;

  DBUG_ENTER( "MakeALS");

  res = MEMmalloc( sizeof( arraylist_struct));

  res->avis = avis;
  res->size = size;
  res->next = als;

  DBUG_RETURN( res);
}
*/

/** <!--********************************************************************-->
 *
 * @fn arraylist_struct *FreeALS( arraylist_struct *als)
 *
 * @brief
 *
 *****************************************************************************/
static arraylist_struct *
FreeALS (arraylist_struct *als)
{
    DBUG_ENTER ();

    if (als != NULL) {
        if (als->next != NULL) {
            als->next = FreeALS (als->next);
        }

        als = MEMfree (als);
    }

    DBUG_RETURN (als);
}

/** <!--******************************************************************-->
 *
 * @fn arraylist_struct *ArraylistContains( arraylist_struct *als, node *avis)
 *
 * @brief
 *
 ***************************************************************************/
static arraylist_struct *
ArraylistContains (arraylist_struct *als, node *avis)
{
    arraylist_struct *res;

    DBUG_ENTER ();

    if (als == NULL) {
        res = NULL;
    } else {
        if (als->avis == avis) {
            res = als;
        } else {
            res = ArraylistContains (als->next, avis);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *ArraylistMaxSizeAvis( arraylist_struct *als)
 *
 * @brief
 *
 *****************************************************************************/
static node *
ArraylistMaxSizeAvis (arraylist_struct *als)
{
    node *res;
    arraylist_struct *current_max;

    DBUG_ENTER ();

    if (als == NULL) {
        res = NULL;
    } else {
        current_max = als;
        while (als->next != NULL) {
            if (current_max->size < (als->next)->size) {
                current_max = als->next;
            }
            als = als->next;
        }
        res = current_max->avis;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn arraylist_struct *ArraylistIncSize( arraylist_struct *als, int size)
 *
 * @brief
 *
 *****************************************************************************/
static arraylist_struct *
ArraylistIncSize (arraylist_struct *als, int size)
{
    DBUG_ENTER ();

    als->size = als->size + size;

    DBUG_RETURN (als);
}

/** <!--********************************************************************-->
 *
 * @fn int GetPartitionSize( node *lb_elements, node *ub_elements)
 *
 * @brief
 *
 *****************************************************************************/
static int
GetPartitionSize (node *lb_elements, node *ub_elements)
{
    constant *size_cnst, *lb_cnst, *ub_cnst;

    DBUG_ENTER ();

    size_cnst = COmakeConstantFromInt (0);

    while (lb_elements != NULL && ub_elements != NULL) {
        lb_cnst = COaST2Constant (EXPRS_EXPR (lb_elements));
        ub_cnst = COaST2Constant (EXPRS_EXPR (ub_elements));

        DBUG_ASSERT (lb_cnst != NULL, "Lower bound is not constant!");
        DBUG_ASSERT (ub_cnst != NULL, "Upper bound is not constant!");

        size_cnst = COadd (size_cnst, COsub (ub_cnst, lb_cnst, NULL), NULL);

        lb_elements = EXPRS_NEXT (lb_elements);
        ub_elements = EXPRS_NEXT (ub_elements);
    }
    DBUG_RETURN (COconst2Int (size_cnst));
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ACUPTNwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ACUPTNwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_MODE (arg_info) = mark_potential;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

        INFO_MAXARRAYAVIS (arg_info) = ArraylistMaxSizeAvis (INFO_ARRAYLIST (arg_info));
        INFO_ARRAYLIST (arg_info) = FreeALS (INFO_ARRAYLIST (arg_info));

        INFO_MODE (arg_info) = unmark_potential;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    } else {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUPTNpart (node *arg_node, info *arg_info)
{
    // node *block_instr;

    DBUG_ENTER ();

    /* For now, we mark each partition as true since the implementation
     * is not stable yet */
    PART_CUDARIZABLE (arg_node) = TRUE;

    /*
      block_instr = BLOCK_ASSIGNS( PART_CBLOCK( arg_node));

      if( INFO_MODE( arg_info) == mark_potential) {
        if( NODE_TYPE( block_instr) == N_empty) {
          PART_CUDARIZABLE( arg_node) = TRUE;
        }
        else if( NODE_TYPE( block_instr) == N_assign) {
          int assign_count;
          assign_count = TCcountAssigns( block_instr);
          if( assign_count > 1) {
            PART_CUDARIZABLE( arg_node) = TRUE;
          }
          else {
            if( NODE_TYPE( ASSIGN_STMT( block_instr)) == N_let &&
                NODE_TYPE( ASSIGN_RHS( block_instr)) == N_prf &&
                PRF_PRF( ASSIGN_RHS( block_instr)) == F_idx_sel) {
              PART_CUDARIZABLE( arg_node) = FALSE;
              INFO_ARRAYAVIS( arg_info) = ID_AVIS( PRF_ARG2( ASSIGN_RHS( block_instr)));
              if( ArraylistContains( INFO_ARRAYLIST( arg_info), INFO_ARRAYAVIS( arg_info))
      == NULL) { INFO_ARRAYLIST( arg_info) = MakeALS( INFO_ARRAYLIST( arg_info),
      INFO_ARRAYAVIS( arg_info), 0);
              }
            }
            else {
              PART_CUDARIZABLE( arg_node) = TRUE;
            }
          }
        }

        if( !PART_CUDARIZABLE( arg_node)) {
          PART_GENERATOR( arg_node) = TRAVopt( PART_GENERATOR( arg_node), arg_info);
        }
      }
      else if( INFO_MODE( arg_info) == unmark_potential) {
        if( !PART_CUDARIZABLE( arg_node) && INFO_MAXARRAYAVIS( arg_info) != NULL) {
          if( ID_AVIS( PRF_ARG2( ASSIGN_RHS( block_instr))) != INFO_MAXARRAYAVIS(
      arg_info)) { PART_CUDARIZABLE( arg_node) = TRUE;
          }
        }
      }
    */
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node* ACUPTNgenerator( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUPTNgenerator (node *arg_node, info *arg_info)
{
    node *lower_bound, *upper_bound;
    node *lower_bound_elements, *upper_bound_elements;
    int partition_size;
    arraylist_struct *als;

    DBUG_ENTER ();

    lower_bound = GENERATOR_BOUND1 (arg_node);
    upper_bound = GENERATOR_BOUND2 (arg_node);

    if (NODE_TYPE (lower_bound) == N_id) {
        node *ssaassign = AVIS_SSAASSIGN (ID_AVIS (lower_bound));
        DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (ssaassign)) == N_array,
                     "Lower bound should be an N_array node!");
        lower_bound_elements = ARRAY_AELEMS (ASSIGN_RHS (ssaassign));
    } else {
        lower_bound_elements = ARRAY_AELEMS (lower_bound);
    }

    if (NODE_TYPE (upper_bound) == N_id) {
        node *ssaassign = AVIS_SSAASSIGN (ID_AVIS (upper_bound));
        DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (ssaassign)) == N_array,
                     "Upper bound should be an N_array node!");
        upper_bound_elements = ARRAY_AELEMS (ASSIGN_RHS (ssaassign));
    } else {
        upper_bound_elements = ARRAY_AELEMS (upper_bound);
    }

    partition_size = GetPartitionSize (lower_bound_elements, upper_bound_elements);

    als = ArraylistContains (INFO_ARRAYLIST (arg_info), INFO_ARRAYAVIS (arg_info));

    if (als != NULL) {
        als = ArraylistIncSize (als, partition_size);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
