/*****************************************************************************
 *
 * @defgroup
 *
 *
 * description:
 *   This module annotates N_with which can be executed on the
 *   CUDA device in parallel. It set for *all* N_with nodes the
 *   attribute WITH_CUDARIZABLE to either true or false!
 *
 *   Currently (April 2021), a WL is annotated as CUDARIZABLE, iff:
 *
 *     (1) It is an outermost WL within a function body               &&
 *
 *     (2) The simpletype of the result of the WL is a
 *         SupportedHostSimpletype.                                   &&
 *
 *     (3) There is no #pragma nocuda at the WL                       &&
 *
 *     (4) all referenced variables within that WL are at least AKD
 *         && their simpletype is a SupportedHostSimpletype!          &&
 *
 *     (5) The only function applications that are found in the code
 *         are LaC funs or functions from the module "Math". NB: we 
 *         do traverse into the LaC fun bodies here!                  &&
 *
 *     (6) The N_withops supported are *single operator* WLs of the kind
 *         N_genarray, N_modarray, or N_fold; for these, we have the
 *         following constraints:
 *           N_genarray and N_modarray:
 *             The WL is at least AKD, and any potential inner WL 
 *             can only be N_genarray, N_modarray, or N_fold as well;
 *             An inner N_genarray or N_modarray can only be in 
 *             perfect nesting position, i.e., its LHS is the CEXPR
 *           N_fold:
 *             is only admissible in case partial Folds (PFD) is turned on
 *             and the result type is AKS.
 *
 *
 * implementation:
 *   The implementation is driven through ACUWLwith. It keeps track whether
 *   WL is the outermost one ( !INFO_INWL ). Once inside the outermost WL, 
 *   INFO_CUDARIZABLE is keeping track of the permissibility of *that* very WL.
 *   Even when traversing inner WLs, INFO_CUDARIZABLE always relates to the
 *   outermost one.
 *   The overall scheme is that INFO_CUDARIZABLE is initialised with TRUE and
 *   during the traversal of all parts of the WL, including potentially nested
 *   WLs, we check for exclusion criteria according to the above list.
 *
 *   ACUWLwith checks (1), (2), and (3).
 *   ACUWLid checks for (4), and
 *   ACUWLap checks for (5).
 *   The criteria from (6) are checked within the individual withop functions
 *   ACUWLgenarray, ACUWLmodarray, ACUWLfold, ACUWLbreak, ACUWLpropagate
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

#define INFO_INWL(n) (n->inwl)                // signals that we are within a
                                              // candidate WL (outer one)
#define INFO_CUDARIZABLE(n) (n->cudarizable)  // signals whether the WL under
                                              // consideration is still permissible
#define INFO_LETIDS(n) (n->letids)            // current LHS N_ids
#define INFO_FUNDEF(n) (n->fundef)            // current N_fundef
#define INFO_FROM_AP(n) (n->from_ap)          // did we enter via LaC-fun application?
#define INFO_CODE(n) (n->code)                // current N_code

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
    } else {
        /* If the fundef is lac function, we check whether the traversal
         * is initiated from the calling site. */
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
 * @brief just injecting actual lhs N_ids into INFO_LETIDS!
 *
 *
 *****************************************************************************/
node *
ACUWLlet (node *arg_node, info *arg_info)
{
    node *old_letids;
    DBUG_ENTER ();

    old_letids = INFO_LETIDS (arg_info);
    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = old_letids;

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

    DBUG_PRINT ("inspecting N_with that defines \"%s\"",
                IDS_NAME (INFO_LETIDS (arg_info)));

    ty = IDS_NTYPE (INFO_LETIDS (arg_info));
    base_ty = TYgetSimpleType (TYgetScalar (ty));
    is_ok_basetype = CUisSupportedHostSimpletype (base_ty);

    /* If the N_with is a top level withloop */
    if (!INFO_INWL (arg_info)) {        // checking condition (1)
        DBUG_PRINT ("  outer WL => candidate!");
        INFO_CUDARIZABLE (arg_info) = TRUE;

        DBUG_PRINT ("  checking base type!");
        if (!is_ok_basetype) {          // checking condition  (2)
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize with-loop due to missing base type "
                         "implementation! "
                         "Missing type: \"%s\" for the result!",
                         global.type_string[base_ty]);
        }
        DBUG_PRINT ("  %s!",
                    (INFO_CUDARIZABLE (arg_info) ? "ok" : "not ok"));

        DBUG_PRINT ("  checking #pragma nocuda!");
        if (WITH_PRAGMA (arg_node) != NULL
            && PRAGMA_NOCUDA (WITH_PRAGMA (arg_node))) { // checking (3)
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarn (LINE_TO_LOC (global.linenum), "Cudarization of with-loop blocked "
                         "by pragma!");
        }
        DBUG_PRINT ("  %s!",
                    (INFO_CUDARIZABLE (arg_info) ? "ok" : "not ok"));

        // checking conditions (4) and (5)
        DBUG_PRINT ("  checking code!");
        INFO_INWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INWL (arg_info) = FALSE;
        DBUG_PRINT ("  %s!",
                    (INFO_CUDARIZABLE (arg_info) ? "ok" : "not ok"));

        // checking condition (6)
        DBUG_PRINT ("  checking withop!");
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        DBUG_PRINT ("  %s!",
                    (INFO_CUDARIZABLE (arg_info) ? "ok" : "not ok"));

        WITH_CUDARIZABLE (arg_node) = INFO_CUDARIZABLE (arg_info);
        DBUG_PRINT ("  result: %s!",
                    (WITH_CUDARIZABLE (arg_node) ?
                     "cudarizable" : "not cudarizable"));

        INFO_CUDARIZABLE (arg_info) = FALSE;

    } else {
        /*
         * we are dealing with an inner WL here. Consequently, we dismiss it 
         * straightaway. Nevertheless, we still have to traverse through it
         * as we are still in the outermost WL!
         */
        DBUG_PRINT ("  inner WL => NO candidate!");
        WITH_CUDARIZABLE (arg_node) = FALSE;
        CTInoteLine (NODE_LINE (arg_node), "Inner With-loop => no cudarization!");

        DBUG_PRINT ("  checking body of inner WL!");
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        /*
         * For the outer WL, we still ensure that the inner WL is at least
         * AKD and of suitable simpletype.
         */
        INFO_CUDARIZABLE (arg_info)
          = is_ok_basetype && TUdimKnown (ty) && INFO_CUDARIZABLE (arg_info);
    }
    DBUG_PRINT ("done with N_with that defines \"%s\"",
                IDS_NAME (INFO_LETIDS (arg_info)));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLcode( node *arg_node, info *arg_info)
 *
 * @brief just injecting actual N_code into INFO_CODE!
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
 * @brief checking condition (6) for N_fold
 *
 *
 *****************************************************************************/
node *
ACUWLfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("    checking N_fold!");
    if (!INFO_INWL (arg_info)) {
        if (global.optimize.dopfd) {
            FOLD_NEUTRAL (arg_node) = TRAVopt (FOLD_NEUTRAL (arg_node), arg_info);
            if (FOLD_NEXT (arg_node) != NULL) {
                INFO_CUDARIZABLE (arg_info) = FALSE;
                CTIwarnLine (global.linenum,
                             "Cannot cudarize with-loop due to"
                             " multiple operators!");
            }
            if (!TUshapeKnown (IDS_NTYPE (INFO_LETIDS (arg_info)))) {
                INFO_CUDARIZABLE (arg_info) = FALSE;
                CTIwarnLine (global.linenum,
                             "Cannot cudarize with-loop as partial folds"
                             " require statically known result shapes!");
            }
            if (INFO_CUDARIZABLE (arg_info)) {
                FOLD_ISPARTIALFOLD (arg_node) = TRUE;
            }
        } else {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize with-loop as partial"
                         " folds are turned off! Use -doPFD to enable"
                         " partial folds.");
        }
    } else {
        /* Inner with-loops themselves are dismissed in ACUWLwith; here we
         * only check (4) of the neutral element!
         */
        FOLD_NEUTRAL (arg_node) = TRAVopt (FOLD_NEUTRAL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLgenarray( node *arg_node, info *arg_info)
 *
 * @brief check conditon (6) for N_genarray
 *
 *
 *****************************************************************************/
node *
ACUWLgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("    checking N_genarray!");
    if (!INFO_INWL (arg_info)) {
        DBUG_PRINT ("      checking single operator!");
        if (GENARRAY_NEXT (arg_node) != NULL) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize with-loop due to"
                         " multiple operators!");
        }
        DBUG_PRINT ("      %s!",
                    (INFO_CUDARIZABLE (arg_info) ? "ok" : "not ok"));

        DBUG_PRINT ("      checking AKD!");
        if (!TUdimKnown (IDS_NTYPE (INFO_LETIDS (arg_info)))) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize genarray-with-loop as genarray-"
                         "with-loops require statically known result dimensions!");
        }
        DBUG_PRINT ("      %s!",
                    (INFO_CUDARIZABLE (arg_info) ? "ok" : "not ok"));
    } else {
        if (IDS_AVIS (INFO_LETIDS (arg_info))
             != ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (INFO_CODE (arg_info))))) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize with-loop as inner"
                         " genarray-with-loop is not in perfect nesting"
                         " position!");
        }
    }
    DBUG_PRINT ("    %s!", (INFO_CUDARIZABLE (arg_info) ? "ok" : "not ok"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ACUWLmodarray( node *arg_node, info *arg_info)
 *
 * @brief check conditon (6) for N_modarray
 *
 *
 *****************************************************************************/
node *
ACUWLmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("    checking N_modarray!");
    if (!INFO_INWL (arg_info)) {
        if (MODARRAY_NEXT (arg_node) != NULL) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize with-loop due to"
                         " multiple operators!");
        }
        if (!TUdimKnown (IDS_NTYPE (INFO_LETIDS (arg_info)))) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize modarray-with-loop as modarray-"
                         "with-loops require statically known result dimensions!");
        }
    } else {
        if (IDS_AVIS (INFO_LETIDS (arg_info))
             != ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (INFO_CODE (arg_info))))) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize with-loop as inner"
                         " modarray-with-loop is not in perfect nesting"
                         " position!");
        }
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

    INFO_CUDARIZABLE (arg_info) = FALSE;
    CTIwarnLine (global.linenum,
                 "Cannot cudarize break-with-loop!");

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

    INFO_CUDARIZABLE (arg_info) = FALSE;
    CTIwarnLine (global.linenum,
                 "Cannot cudarize propagate-with-loop!");

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
        if (!TUdimKnown( type)) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarnLine (global.linenum,
                         "Cannot cudarize with-loop as the variable \"%s\""
                         "is of statically unknown dimensionality.",
                         ID_NAME (arg_node));
        } else if (!CUisSupportedHostSimpletype (TYgetSimpleType (TYgetScalar (type)))) {
            INFO_CUDARIZABLE (arg_info) = FALSE;
            CTIwarn (LINE_TO_LOC (global.linenum),
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
