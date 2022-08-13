/** <!--********************************************************************-->
 *
 * @defgroup esp Traversal
 *
 * Module description goes here.
 *
 * For an example, take a look at src/refcount/explicitcopy.c
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file enforce_specialization.c
 *
 * Prefix: ESP
 *
 *****************************************************************************/
#include "enforce_specialization.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "ESP"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "ssi.h"
#include "specialize.h"
#include "type_utils.h"
#include "new_types.h"
#include "deserialize.h"
#include "sig_deps.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "globals.h"
#include "ctinfo.h"

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ESPdoEnforceSpecialization( node *syntax_tree)
 *
 *****************************************************************************/

node *
ESPdoEnforceSpecialization (node *syntax_tree)
{
    bool ok;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting esp traversal.");

    ok = SSIinitAssumptionSystem (SDhandleContradiction, SDhandleElimination);
    DBUG_ASSERT (ok, "Initialisation of Assumption System went wrong!");

    SPECinitSpecChain ();

    /*
     * Now we have to initialize the deserialisation unit, as
     * specializations may add new functions as dependencies
     * of bodies to the ast
     */
    DSinitDeserialize (syntax_tree);

    TRAVpush (TR_esp);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    CTIabortOnError ();

    /*
     * from here on, no more functions are deserialized, so we can
     * finish the deseralization engine
     */
    DSfinishDeserialize (syntax_tree);

    DBUG_PRINT ("EnforceSpecialization traversal complete.");

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
 * @fn node *ESPmodule(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
ESPmodule (node *arg_node, info *arg_info)
{
    node *specialized_fundefs;

    DBUG_ENTER ();

    if (NULL != MODULE_FUNSPECS (arg_node)) {
        MODULE_FUNSPECS (arg_node) = TRAVdo (MODULE_FUNSPECS (arg_node), arg_info);
        MODULE_FUNSPECS (arg_node) = FREEdoFreeTree (MODULE_FUNSPECS (arg_node));
    }

    specialized_fundefs = SPECresetSpecChain ();
    if (specialized_fundefs != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (MODULE_FUNS (arg_node), specialized_fundefs);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESPfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
ESPfundef (node *arg_node, info *arg_info)
{
    node *wrapper;
    ntype *args, *rets;
    dft_res *disp_res;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    wrapper = FUNDEF_IMPL (arg_node);

    args = TUmakeProductTypeFromArgs (FUNDEF_ARGS (arg_node));
    rets = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));

    DBUG_EXECUTE (tmp_str = TYtype2String (args, 0, 0));
    DBUG_PRINT ("dispatching %s for %s", CTIitemName (wrapper), tmp_str);

    disp_res = TYdispatchFunType (FUNDEF_WRAPPERTYPE (wrapper), args);

    DBUG_EXECUTE (tmp_str = TYdft_res2DebugString (disp_res));
    DBUG_PRINT ("%s", tmp_str);
    DBUG_EXECUTE (MEMfree (tmp_str));

    if (disp_res == NULL) {
        CTIwarn (LINE_TO_LOC (global.linenum), 
                 "Specialization of \"%s\" to arguments () ignored",
                 CTIitemName (arg_node));
    } else {
        /*
         * create specializations (if appropriate), trigger the type
         * check of all potentially involved fundefs and extract the
         * return type from the dft_res structure:
         */
        disp_res = SPEChandleDownProjections (disp_res, wrapper, args, rets);

        if ((disp_res->def == NULL) && (disp_res->num_partials == 0)) {
            /*
             * no match at all!
             */
            CTIerror (LINE_TO_LOC (global.linenum),
                      "No matching definition found for the specialization "
                      " of \"%s\" for arguments %s",
                      CTIitemName (arg_node), TYtype2String (args, FALSE, 0));
        }

        TYfreeDft_res (disp_res);
    }

    args = TYfreeType (args);
    rets = TYfreeType (rets);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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
