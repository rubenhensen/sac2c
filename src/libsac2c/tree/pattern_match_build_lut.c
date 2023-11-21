/** <!--********************************************************************-->
 *
 * @defgroup pmbl Pattern Match Build Lut traversal
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  n  |  sah  | 17/03/10
 * can be called on N_fundef               |   -----   |  y  |  sah  | 17/03/10
 * expects LaC funs                        |   -----   |  y  |  sah  | 17/03/10
 * follows N_ap to LaC funs                |   -----   |  n  |  sah  | 17/03/10
 * =============================================================================
 * deals with GLF properly                 |    yes    |  y  |  sah  | 17/03/10
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |  y  |  sah  | 17/03/10
 * utilises SAA annotations                |   -----   |  n  |  sah  | 17/03/10
 * =============================================================================
 * tolerates flattened N_array             |    yes    |  y  |  sah  | 17/03/10
 * tolerates flattened Generators          |    yes    |  y  |  sah  | 17/03/10
 * tolerates flattened operation parts     |    yes    |  y  |  sah  | 17/03/10
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |  y  |  sah  | 17/03/10
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |  y  |  sah  | 17/03/10
 * =============================================================================
 * </pre>
 *
 * This traversal constructs from a given function a mapping between the
 * variables (N_avis) of inner contexts of LaC funs (the called site) to
 * the outer context (the calling site) and vice versa.
 *
 * For conditional functions, all parameters of the cond function are mapped
 * to the arguments of the corresponding application. For loop functions, the
 * situation is more complex. Here, only those parameters that map to
 * themselves in the recursive call are mapped to the corresponding outer
 * application of the loop function.
 *
 * Whether a parameter maps to itself is decided using the pattern matching
 * framework. As the framework supports different notions of equality of
 * N_id and thus N_avis nodes, this traversal is parameterised by such
 * matching mode.
 *
 * @ingroup pmbl
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file trav_template.c
 *
 * Prefix: TEMP
 *
 *****************************************************************************/
#include "pattern_match_build_lut.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "LookUpTable.h"
#include "pattern_match.h"
#include "pattern_match_attribs.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    pm_mode_t *pmmode;
    node *ap;
    node *fundef;
    lut_t *lut;
    node *arguments;
};

/**
 * A template entry in the template info structure
 */
#define INFO_PMMODE(n) ((n)->pmmode)
#define INFO_AP(n) ((n)->ap)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LUT(n) ((n)->lut)
#define INFO_ARGUMENTS(n) ((n)->arguments)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PMMODE (result) = NULL;
    INFO_AP (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_ARGUMENTS (result) = NULL;

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
 * @fn node *PMBLdoBuildPatternMatchingLut( node *fundef, pm_mode_t pmmode)
 *
 *****************************************************************************/
lut_t *
PMBLdoBuildPatternMatchingLut (node *fundef, pm_mode_t *pmmode)
{
    info *info;
    lut_t *lut;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "only fundef nodes can be used as argument here!");

    DBUG_ASSERT (!FUNDEF_ISLACFUN (fundef),
                 "cannot start lut building on a lac function!");

    info = MakeInfo ();

    INFO_LUT (info) = LUTgenerateLut ();
    INFO_PMMODE (info) = pmmode;

    TRAVpush (TR_pmbl);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    lut = INFO_LUT (info);
    info = FreeInfo (info);

    DBUG_RETURN (lut);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * lut_t *TagIdentities( node *args, node *params, lut_t lut, pm_mode_t pmmode)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static lut_t *
TagIdentities (node *args, node *params, lut_t *lut, pm_mode_t *pmmode)
{
    pattern *pat;
    node *avis;

    DBUG_ENTER ();

    pat = PMparam (1, PMAgetAvis (&avis));

    while (args != NULL) {
        DBUG_ASSERT (params != NULL, "no of args does not match no of params");

        if ((PMmatch (pmmode, pat, EXPRS_EXPR (args))) && (avis == ARG_AVIS (params))) {
            /* insert this mapping into lut to remember that it is a pass through */
            lut = LUTinsertIntoLutP (lut, avis, EXPRS_EXPR (args));
        }

        args = EXPRS_NEXT (args);
        params = ARG_NEXT (params);
    }

    DBUG_RETURN (lut);
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
 * @fn node *PMBLfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PMBLfundef (node *arg_node, info *arg_info)
{
    node *lastfun;

    DBUG_ENTER ();

    lastfun = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;

    /*
     * first traverse the body to infer whether loop function
     * arguments are pass-through
     */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (INFO_AP (arg_info) != NULL) {
        /* in inner context */

        INFO_ARGUMENTS (arg_info) = AP_ARGS (INFO_AP (arg_info));
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

        DBUG_ASSERT (INFO_ARGUMENTS (arg_info) == NULL, "left-over arguments found!");
    }

    INFO_FUNDEF (arg_info) = lastfun;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PMBLap(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PMBLap (node *arg_node, info *arg_info)
{
    node *oldap;

    DBUG_ENTER ();

    /*
     * process all lac functions
     */
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        if (INFO_FUNDEF (arg_info) == AP_FUNDEF (arg_node)) {
            /* recursive call in loop function */
            INFO_LUT (arg_info)
              = TagIdentities (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                               INFO_LUT (arg_info), INFO_PMMODE (arg_info));
        } else {
            /* all other lac functions */
            oldap = INFO_AP (arg_info);
            INFO_AP (arg_info) = arg_node;

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            INFO_AP (arg_info) = oldap;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PMBLarg(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
PMBLarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_ARGUMENTS (arg_info) != NULL,
                 "ran out of arguments when processing parameters!");

    if (!FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
        || (LUTsearchInLutP (INFO_LUT (arg_info), ARG_AVIS (arg_node)) != NULL)) {
        /* either it is a cond fun or a pass through argument of a loop fun */

        INFO_LUT (arg_info)
          = LUTupdateLutP (INFO_LUT (arg_info), ARG_AVIS (arg_node),
                           EXPRS_EXPR (INFO_ARGUMENTS (arg_info)), NULL);
    }

    INFO_ARGUMENTS (arg_info) = EXPRS_NEXT (INFO_ARGUMENTS (arg_info));

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
