/*
 *
 * $Id$
 *
 */

#include "prepareinline.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "add_function_body.h"
#include "deserialize.h"
#include "str.h"
#include "memory.h"
#include "type_utils.h"
#include "new_types.h"
#include "namespaces.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
    int fetched;
    namespace_t *prelude;
};

/*
 * INFO macros
 */
#define INFO_PPI_MODULE(n) ((n)->module)
#define INFO_PPI_FETCHED(n) ((n)->fetched)
#define INFO_PPI_PRELUDE(n) ((n)->prelude)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PPI_MODULE (result) = NULL;
    INFO_PPI_FETCHED (result) = 0;
    INFO_PPI_PRELUDE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Helper functions
 */
static node *
tagFundefAsNeeded (node *fundef, info *info)
{
    DBUG_ENTER ("tagFundefAsNeeded");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "tagFundefAsNeeded applied to non fundef node");

    DBUG_ASSERT ((!FUNDEF_ISWRAPPERFUN (fundef)),
                 "tagFundefAsNeeded called on wrapper fun");

    if (!FUNDEF_ISNEEDED (fundef)) {
        /*
         * mark it as needed first to avoid recursion
         */
        DBUG_PRINT ("PPI", ("marking fundef %s as needed", CTIitemName (fundef)));

        FUNDEF_ISNEEDED (fundef) = TRUE;
#ifndef DBUG_OFF
    } else {
        DBUG_PRINT ("PPI",
                    ("!!! fundef %s already marked as needed...", CTIitemName (fundef)));
#endif /* DBUG_OFF */
    }

    DBUG_RETURN (fundef);
}

static node *
tagWrapperAsNeeded (node *wrapper, info *info)
{
    DBUG_ENTER ("tagWrapperAsNeeded");

    if (!FUNDEF_ISNEEDED (wrapper)) {
        DBUG_PRINT ("PPI", ("marking wrapper %s as needed", CTIitemName (wrapper)));

        FUNDEF_ISNEEDED (wrapper) = TRUE;

        if (FUNDEF_IMPL (wrapper) != NULL) {
            /*
             * we found a wrapper that has FUNDEF_IMPL set
             * this is a dirty hack telling us that the function
             * has no arguments and thus only one instance!
             * so we tag that instance
             */
            FUNDEF_IMPL (wrapper) = tagFundefAsNeeded (FUNDEF_IMPL (wrapper), info);
        } else if (FUNDEF_WRAPPERTYPE (wrapper) != NULL) {
            FUNDEF_WRAPPERTYPE (wrapper)
              = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (wrapper), &tagFundefAsNeeded,
                                        info);
        }
#ifndef DBUG_OFF
    } else {
        DBUG_PRINT ("PPI", ("!!! wrapper %s already marked as needed...",
                            CTIitemName (wrapper)));
#endif /* DBUG_OFF */
    }

    DBUG_RETURN (wrapper);
}

node *
PPIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PPIfundef");

    /*
     * traverse the body to mark functions that are needed.
     * as wrapper functions always and only contain applications
     * of their instances, we skip those. instead, PPIap will mark
     * all instances of a wrapper if it find an application of a
     * wrapper function.
     */

    DBUG_PRINT ("PPI", ("processing down '%s'", CTIitemName (arg_node)));

    if ((!FUNDEF_ISWRAPPERFUN (arg_node)) && (FUNDEF_BODY (arg_node) != NULL)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * tag special prelude funs as needed
     */
    if (NSequals (FUNDEF_NS (arg_node), INFO_PPI_PRELUDE (arg_info))) {
        FUNDEF_ISNEEDED (arg_node) = TRUE;
    }

    /*
     * move down the fundef chain
     */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * On our way up we fetch the body of every inline function
     * that appears at a rhs postion within the ast.
     */

    DBUG_PRINT ("PPI", ("processing up '%s'", CTIitemName (arg_node)));

    if ((FUNDEF_BODY (arg_node) == NULL) && (FUNDEF_ISINLINE (arg_node))
        && (FUNDEF_ISNEEDED (arg_node)) && (FUNDEF_SYMBOLNAME (arg_node) != NULL)) {
        arg_node = AFBdoAddFunctionBody (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            INFO_PPI_FETCHED (arg_info)++;

            DBUG_PRINT ("PPI",
                        ("fetched function body for '%s'", CTIitemName (arg_node)));
        } else {
            char *funsig = TUtypeSignature2String (arg_node);

            CTIerror ("Unable to find body of function '%s' with args '%s' in module.",
                      CTIitemName (arg_node), funsig);

            funsig = MEMfree (funsig);
        }
    }

    /*
     * finally, reset the needed flag
     */
    FUNDEF_ISNEEDED (arg_node) = FALSE;

    DBUG_RETURN (arg_node);
}

node *
PPIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PPIap");

    if (FUNDEF_ISWRAPPERFUN (AP_FUNDEF (arg_node))) {
        AP_FUNDEF (arg_node) = tagWrapperAsNeeded (AP_FUNDEF (arg_node), arg_info);
    } else {
        AP_FUNDEF (arg_node) = tagFundefAsNeeded (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PPIfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PPIfold");

    if (FUNDEF_ISWRAPPERFUN (FOLD_FUNDEF (arg_node))) {
        FOLD_FUNDEF (arg_node) = tagWrapperAsNeeded (FOLD_FUNDEF (arg_node), arg_info);
    } else {
        FOLD_FUNDEF (arg_node) = tagFundefAsNeeded (FOLD_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PPImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PPImodul");

    DSinitDeserialize (arg_node);

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DSfinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

node *
PPIdoPrepareInline (node *syntax_tree)
{
    info *info;
#ifndef DBUG_OFF
    int rounds = 0;
#endif

    DBUG_ENTER ("PPIdoPrepareInline");

    info = MakeInfo ();

    /*
     * we want to fetch the body of our special prelude
     * module always. For performance reasons, we
     * hold a copy of that namespace in the info node.
     */
    INFO_PPI_PRELUDE (info) = NSgetNamespace (global.preludename);

    TRAVpush (TR_ppi);

    if (global.optimize.doinl) {
        do {
            DBUG_PRINT ("PPI", ("Starting round %d.", rounds));

            INFO_PPI_FETCHED (info) = 0;

            syntax_tree = TRAVdo (syntax_tree, info);

            DBUG_PRINT ("PPI", ("Finished round %d, fetched %d bodies.", rounds,
                                INFO_PPI_FETCHED (info)));

#ifndef DBUG_OFF
            rounds++;
#endif

            CTIabortOnError ();
        } while (INFO_PPI_FETCHED (info) != 0);
    } else {
        DBUG_PRINT ("PPI", ("skipping PPI as inlining is disabled..."));
    }

    TRAVpop ();

    INFO_PPI_PRELUDE (info) = NSfreeNamespace (INFO_PPI_PRELUDE (info));
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
