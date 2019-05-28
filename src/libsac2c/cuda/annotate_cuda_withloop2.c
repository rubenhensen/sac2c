/*****************************************************************************
 *
 * @defgroup
 *
 *
 * description:
 *   This module annotates N_with which can be executed on the
 *   CUDA device in parallel. Currently, cudarizable N_withs are
 *   limited as follows:
 *     1) A N_with must not contain any inner N_withs.
 *     2) Fold N_with is currently not supported.
 *     3) Function applications are not allowed in a cudarizable
 *        N_with except primitive mathematical functions.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file annotate_cuda_withloop2.c
 *
 * Prefix: ACUWL
 *
 *****************************************************************************/
#include "annotate_cuda_withloop2.h"

#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "ACUWL"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "namespaces.h"
#include "new_types.h"
#include "DupTree.h"
#include "type_utils.h"
#include "shape.h"
#include "cuda_utils.h"

#include "types.h"
#include "vector.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool inwl;
    bool cudarizable;
    node *letids;
    node *fundef;
    bool from_ap;
    node *code;
};

#define INFO_INWL(n) (n->inwl)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_FROM_AP(n) (n->from_ap)
#define INFO_CODE(n) (n->code)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INWL (result) = FALSE;
    INFO_CUDARIZABLE (result) = TRUE;
    INFO_LETIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_FROM_AP (result) = FALSE;
    INFO_CODE (result) = NULL;

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

static void
InitCudaBlockSizes (void)
{
    DBUG_ENTER ();

    if (STReq (global.config.cuda_arch, "no")) {
        CTIwarn ("Your system might no be setup for CUDA!");
    } else if (!STReq (global.config.cuda_arch,
                       global.cuda_arch_names[global.cuda_arch])) {
        CTIwarn ("You are using CUDA arch `%s', but your system is setup for `%s'.",
                 global.cuda_arch_names[global.cuda_arch], global.config.cuda_arch);
    }

    if (global.cuda_arch == CUDA_SM10 || global.cuda_arch == CUDA_SM11) {
        global.cuda_options.optimal_threads = 256;
        global.cuda_options.optimal_blocks = 3;
        global.cuda_options.cuda_1d_block_large = 256;
        global.cuda_options.cuda_1d_block_small = 64;
        global.cuda_options.cuda_2d_block_x = 16;
        global.cuda_options.cuda_2d_block_y = 16;
        global.cuda_options.cuda_max_x_grid = 65535;
        global.cuda_options.cuda_max_yz_grid = 65535;
        global.cuda_options.cuda_max_xy_block = 512;
        global.cuda_options.cuda_max_z_block = 64;
        global.cuda_options.cuda_max_threads_block = 512;
    } else if (global.cuda_arch == CUDA_SM12 || global.cuda_arch == CUDA_SM13) {
        global.cuda_options.optimal_threads = 256;
        global.cuda_options.optimal_blocks = 4;
        global.cuda_options.cuda_1d_block_large = 256;
        global.cuda_options.cuda_1d_block_small = 64;
        global.cuda_options.cuda_2d_block_x = 16;
        global.cuda_options.cuda_2d_block_y = 16;
        global.cuda_options.cuda_max_x_grid = 65535;
        global.cuda_options.cuda_max_yz_grid = 65535;
        global.cuda_options.cuda_max_xy_block = 512;
        global.cuda_options.cuda_max_z_block = 64;
        global.cuda_options.cuda_max_threads_block = 512;
    } else if (global.cuda_arch == CUDA_SM20) {
        /*
         * global.cuda_options.optimal_threads = 512;
         * global.cuda_options.optimal_blocks = 3;
         * global.cuda_options.cuda_1d_block_large = 512;
         */
        global.cuda_options.optimal_threads = 256;
        global.cuda_options.optimal_blocks = 6;
        global.cuda_options.cuda_1d_block_large = 256;
        /*
         * 1D block size was 512, but to get better performance, it's
         * now set to 64 (See above). We need mechanism to automatically
         * select the best block size
         * global.cuda_options.cuda_1d_block_large = 512;
         */
        global.cuda_options.cuda_1d_block_small = 64;
        global.cuda_options.cuda_2d_block_x = 16;
        global.cuda_options.cuda_2d_block_y = 16;
        global.cuda_options.cuda_max_x_grid = 65535;
        global.cuda_options.cuda_max_yz_grid = 65535;
        global.cuda_options.cuda_max_xy_block = 1024;
        global.cuda_options.cuda_max_z_block = 64;
        global.cuda_options.cuda_max_threads_block = 1024;
    } else if (global.cuda_arch == CUDA_SM35) {
        global.cuda_options.optimal_threads = 512;
        global.cuda_options.optimal_blocks = 3;
        global.cuda_options.cuda_1d_block_large = 1024;
        /*
         * 1D block size was 512, but to get better performance, it's
         * now set to 64 (See above). We need mechanism to automatically
         * select the best block size
         */
        global.cuda_options.cuda_1d_block_small = 64;
        global.cuda_options.cuda_2d_block_x = 16;
        global.cuda_options.cuda_2d_block_y = 16;
        global.cuda_options.cuda_max_x_grid = 2147483647;
        global.cuda_options.cuda_max_yz_grid = 65535;
        global.cuda_options.cuda_max_xy_block = 1024;
        global.cuda_options.cuda_max_z_block = 64;
        global.cuda_options.cuda_max_threads_block = 1024;
    } else if (global.cuda_arch == CUDA_SM50) {
        global.cuda_options.optimal_threads = 512;
        global.cuda_options.optimal_blocks = 3;
        global.cuda_options.cuda_1d_block_large = 1024;
        /*
         * 1D block size was 512, but to get better performance, it's
         * now set to 64 (See above). We need mechanism to automatically
         * select the best block size
         */
        global.cuda_options.cuda_1d_block_small = 64;
        global.cuda_options.cuda_2d_block_x = 32;
        global.cuda_options.cuda_2d_block_y = 32;
        global.cuda_options.cuda_max_x_grid = 2147483647;
        global.cuda_options.cuda_max_yz_grid = 65535;
        global.cuda_options.cuda_max_xy_block = 1024;
        global.cuda_options.cuda_max_z_block = 64;
        global.cuda_options.cuda_max_threads_block = 1024;
    } else if (global.cuda_arch == CUDA_SM60) {
        global.cuda_options.optimal_threads = 512;
        global.cuda_options.optimal_blocks = 3;
        global.cuda_options.cuda_1d_block_large = 1024;
        /*
         * 1D block size was 512, but to get better performance, it's
         * now set to 64 (See above). We need mechanism to automatically
         * select the best block size
         */
        global.cuda_options.cuda_1d_block_small = 64;
        global.cuda_options.cuda_2d_block_x = 32;
        global.cuda_options.cuda_2d_block_y = 32;
        global.cuda_options.cuda_max_x_grid = 2147483647;
        global.cuda_options.cuda_max_yz_grid = 65535;
        global.cuda_options.cuda_max_xy_block = 1024;
        global.cuda_options.cuda_max_z_block = 64;
        global.cuda_options.cuda_max_threads_block = 1024;
    } else if (global.cuda_arch == CUDA_SM61) {
        global.cuda_options.optimal_threads = 1024;
        global.cuda_options.optimal_blocks = 3;
        global.cuda_options.cuda_1d_block_large = 1024;
        /*
         * 1D block size was 512, but to get better performance, it's
         * now set to 64 (See above). We need mechanism to automatically
         * select the best block size
         */
        global.cuda_options.cuda_1d_block_small = 64;
        global.cuda_options.cuda_2d_block_x = 32;
        global.cuda_options.cuda_2d_block_y = 32;
        global.cuda_options.cuda_max_x_grid = 2147483647;
        global.cuda_options.cuda_max_yz_grid = 65535;
        global.cuda_options.cuda_max_xy_block = 1024;
        global.cuda_options.cuda_max_z_block = 64;
        global.cuda_options.cuda_max_threads_block = 1024;
    } else if (global.cuda_arch == CUDA_SM70) {
        global.cuda_options.optimal_threads = 512;
        global.cuda_options.optimal_blocks = 3;
        global.cuda_options.cuda_1d_block_large = 1024;
        /*
         * 1D block size was 512, but to get better performance, it's
         * now set to 64 (See above). We need mechanism to automatically
         * select the best block size
         */
        global.cuda_options.cuda_1d_block_small = 64;
        global.cuda_options.cuda_2d_block_x = 32;
        global.cuda_options.cuda_2d_block_y = 32;
        global.cuda_options.cuda_max_x_grid = 2147483647;
        global.cuda_options.cuda_max_yz_grid = 65535;
        global.cuda_options.cuda_max_xy_block = 1024;
        global.cuda_options.cuda_max_z_block = 64;
        global.cuda_options.cuda_max_threads_block = 1024;
    } else {
        CTIerrorInternal ("Impossible option for `-cuda_arch' flag!");
    }

    // we override the block size at runtime through commandline argument
    if (global.cuda_block_spec[0] != '\0') {
        char * spec = STRtok (global.cuda_block_spec, ",");
        global.cuda_options.cuda_1d_block_large = atoi (spec); 
        spec = MEMfree (spec);
        spec = STRtok (NULL, ",");
        global.cuda_options.cuda_2d_block_x = atoi (spec); 
        spec = MEMfree (spec);
        spec = STRtok (NULL, ",");
        global.cuda_options.cuda_2d_block_y = atoi (spec); 
        spec = MEMfree (spec);
        spec = STRtok (NULL, ",");
        CTIwarn ("Overriding CUDA block spec with %d,%d,%d",
                 global.cuda_options.cuda_1d_block_large,
                 global.cuda_options.cuda_2d_block_x,
                 global.cuda_options.cuda_2d_block_y);
    }


    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravPart(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
static node *
ATravPart (node *arg_node, info *arg_info)
{
    size_t dim;

    DBUG_ENTER ();

    dim = TCcountIds (PART_IDS (arg_node));

    if (dim == 1) {
        PART_THREADBLOCKSHAPE (arg_node)
          = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, dim),
                         TBmakeExprs (TBmakeNum (global.cuda_options.cuda_1d_block_large),
                                      NULL));
    } else if (dim == 2) {
        PART_THREADBLOCKSHAPE (arg_node)
          = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, dim),
                         TBmakeExprs (TBmakeNum (global.cuda_options.cuda_2d_block_y),
                                      TBmakeExprs (TBmakeNum (
                                                     global.cuda_options.cuda_2d_block_x),
                                                   NULL)));
    } else {
        /* For other dimensionalities, since no blocking is needed,
         * we create an array of 0's */
        size_t i = 0;
        node *arr_elems = NULL;
        while (i < dim) {
            arr_elems = TBmakeExprs (TBmakeNum (0), arr_elems);
            i++;
        }
        PART_THREADBLOCKSHAPE (arg_node)
          = TBmakeArray (TYmakeSimpleType (T_int), SHcreateShape (1, dim), arr_elems);
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *ACUWLdoAnnotateCUDAWL( node *syntax_tree)
 *
 *****************************************************************************/
node *
ACUWLdoAnnotateCUDAWL (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ();

    InitCudaBlockSizes ();

    info = MakeInfo ();
    TRAVpush (TR_acuwl);
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
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ACUWLfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ();

    /* During the main traversal, we only look at non-lac functions */
    if (!FUNDEF_ISLACFUN (arg_node) && !FUNDEF_ISSTICKY (arg_node)) {
        /* Functions in prelude are all sticky and we do not want
         * cudarize withloops in those functions */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;

        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }
    /* If the fundef is lac function, we check whether the traversal
     * is initiated from the calling site. */
    else {
        if (INFO_FROM_AP (arg_info)) {
            old_fundef = INFO_FUNDEF (arg_info);
            INFO_FUNDEF (arg_info) = arg_node;
            /* if the traversal is initiated from the calling site,
             * we traverse the body the function */
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_FUNDEF (arg_info) = old_fundef;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLwith (node *arg_node, info *arg_info)
{
    ntype *ty;
    simpletype base_ty;
    bool is_ok_basetype;

    DBUG_ENTER ();

    ty = IDS_NTYPE (INFO_LETIDS (arg_info));
    base_ty = TYgetSimpleType (TYgetScalar (ty));
    is_ok_basetype = CUisSupportedHostSimpletype (base_ty);

    /* The following assignment cauese a bug when a fold wl is inside
     * a fold wl. Usually, in such a case, the outer fold wl should not
     * be cudarized and this is indicated when tranvering the withop.
     * however, before we set this flag to the outer wl, we traverse
     * the body first. And when the inner fold wl is traversed, it will
     * set INFO_CUDARIZABLE to true here. Essentially, this overwrites
     * the flag value set previously. So we need to move the following
     * statment into the "if" branch. */
    // INFO_CUDARIZABLE( arg_info) = TRUE;

    /* If the N_with is a top level withloop */
    if (!INFO_INWL (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = TRUE;

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        INFO_INWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INWL (arg_info) = FALSE;

        if (!INFO_CUDARIZABLE (arg_info)) {
            CTInoteLine (NODE_LINE (arg_node),
                         "Body of With-Loop to complex => no cudarization!");
        }

        /* We only cudarize AKS N_with */
        if (NODE_TYPE (WITH_WITHOP (arg_node)) == N_fold) {
            /* For fold withloop to be cudarized, it *must* be AKS */
            WITH_CUDARIZABLE (arg_node) = TYisAKS (ty) && INFO_CUDARIZABLE (arg_info);
            if (WITH_CUDARIZABLE (arg_node) && !is_ok_basetype) {
                WITH_CUDARIZABLE (arg_node) = FALSE;
                CTIwarnLine (global.linenum,
                             "Cannot cudarize with-loop due to missing base type "
                             "implementation! "
                             "Missing type: \"%s\" for the result of fold!",
                             global.type_string[base_ty]);
            }

            if (WITH_CUDARIZABLE (arg_node)) {
                FOLD_ISPARTIALFOLD (WITH_WITHOP (arg_node)) = TRUE;
            }
        } else {
            WITH_CUDARIZABLE (arg_node)
              = (TYisAKS (ty) || TYisAKD (ty)) && INFO_CUDARIZABLE (arg_info);
            if (WITH_CUDARIZABLE (arg_node) && !is_ok_basetype) {
                WITH_CUDARIZABLE (arg_node) = FALSE;
                CTIwarnLine (global.linenum,
                             "Cannot cudarize with-loop due to missing base type "
                             "implementation! "
                             "Missing type: \"%s\" for the result!",
                             global.type_string[base_ty]);
            }
        }

        if (WITH_CUDARIZABLE (arg_node)) {
            anontrav_t atrav[2] = {{N_part, &ATravPart}, {(nodetype)0, NULL}};

            TRAVpushAnonymous (atrav, &TRAVsons);
            WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), NULL);
            TRAVpop ();
        }
    } else {
        CTInoteLine (NODE_LINE (arg_node), "Inner With-loop => no cudarization!");
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        /* Since we only try to cudarize outermost N_with, any
         * inner N_with is tagged as not cudarizbale */
        WITH_CUDARIZABLE (arg_node) = FALSE;

        INFO_CUDARIZABLE (arg_info)
          = (TYisAKS (ty) || TYisAKD (ty)) && INFO_CUDARIZABLE (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLcode (node *arg_node, info *arg_info)
{
    node *old_code;

    DBUG_ENTER ();

    old_code = INFO_CODE (arg_info);
    INFO_CODE (arg_info) = arg_node;
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    INFO_CODE (arg_info) = old_code;

    CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLfold( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (global.optimize.dopfd) {
        FOLD_NEUTRAL (arg_node) = TRAVopt (FOLD_NEUTRAL (arg_node), arg_info);
        FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);
    } else {
        /* An outermost fold N_with is currently not cudarizable;
         * however, if the fold is within an N_with, we do not signal
         * uncudarizeable to the enclosing N_with. */
        if (!INFO_INWL (arg_info)) {
            // if(!global.optimize.doscuf) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            //}
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLgenarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* If there is a genarray in a withloop, the
     * outer one cannot be cudarized */
    if (INFO_INWL (arg_info)
        && IDS_AVIS (INFO_LETIDS (arg_info))
             != ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (INFO_CODE (arg_info))))) {
        INFO_CUDARIZABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLbreak( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Currently, we do not support break N_with */
    INFO_CUDARIZABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLpropagate( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Currently, we do not support propagate N_with */
    INFO_CUDARIZABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
ACUWLid (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();

    type = ID_NTYPE (arg_node);

    if (INFO_INWL (arg_info)) {
        /* We do not cudarize any N_with which contains arrays
         * other than AKS arrays */
        if (!TUisScalar (type) && !TYisAKV (type) && !TYisAKS (type) && !TYisAKD (type)) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
        } else if (!CUisSupportedHostSimpletype (TYgetSimpleType (TYgetScalar (type)))) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize with-loop due to missing base type "
                         "implementation! "
                         "Missing type: \"%s\" for relatively free variable \"%s\"!",
                         global.type_string[TYgetSimpleType (TYgetScalar (type))],
                         ID_NAME (arg_node));
        }
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
ACUWLap (node *arg_node, info *arg_info)
{
    node *fundef;
    namespace_t *ns;
    bool traverse_lac_fun, old_from_ap;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);

    /* For us to traverse a function from calling site, it must be a
     * condictional function or a loop function and must not be the
     * recursive function call in the loop function. */
    traverse_lac_fun = (FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info));

    old_from_ap = INFO_FROM_AP (arg_info);
    INFO_FROM_AP (arg_info) = TRUE;

    /* If we are in an N_with, we need to distinguish between two different
     * N_ap: 1) Invocation to lac functions and 2) invocation to normal
     * functions. For the former, we traversal into the corresponding N_fundef
     * and for the later we need to check whether it prevents the current
     * N_with from being cudarized. */

    /*
      if( INFO_INWL( arg_info)) {
        if( traverse_lac_fun) {
           AP_FUNDEF( arg_node) = TRAVdo( AP_FUNDEF( arg_node), arg_info);
        }
        else {
          // The only function application allowed in a cudarizbale N_with
          // is mathematical functions. However, the check below is ugly
          // and a better way needs to be found.
          ns = FUNDEF_NS( AP_FUNDEF( arg_node));
          if( ns == NULL || !STReq( NSgetModule( ns), "Math")) {
            INFO_CUDARIZABLE( arg_info) = FALSE;
          }
        }
      }
      else {
        if( traverse_lac_fun) {
          AP_FUNDEF( arg_node) = TRAVdo( AP_FUNDEF( arg_node), arg_info);
        }
      }
    */

    if (traverse_lac_fun) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    } else if (INFO_INWL (arg_info)) {
        /* The only function application allowed in a cudarizbale N_with
         * is mathematical functions. However, the check below is ugly
         * and a better way needs to be found. */
        if (!FUNDEF_ISLACFUN (fundef)) {
            ns = FUNDEF_NS (AP_FUNDEF (arg_node)); /* ns could be NULL */
            if (ns == NULL || !STReq (NSgetModule (ns), "Math")) {
                INFO_CUDARIZABLE (arg_info) = FALSE;
            }
        }
    }

    /* We need to traverse N_ap arguments because they might
     * contain Non-AKS arrays */
    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    INFO_FROM_AP (arg_info) = old_from_ap;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
