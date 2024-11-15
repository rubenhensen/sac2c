/**
 * @file gatherdependencies.c
 * @brief gathers the dependencies of a sac file
 * @author Stephan Herhut
 * @date 2005-06-01
 */

#include "gatherdependencies.h"

#include "filemgr.h"
#include "stringset.h"
#include "tree_basic.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "traverse.h"
#include "namespaces.h"
#include "globals.h"
#include "tree_compound.h"

/*
 * INFO structure
 */
struct INFO {
    stringset_t *deps;
    node *module;
};

/*
 * INFO macros
 */
#define INFO_GDP_DEPS(n) ((n)->deps)
#define INFO_GDP_MODULE(n) ((n)->module)

/*
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_GDP_DEPS (result) = NULL;
    INFO_GDP_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_GDP_DEPS (info) = STRSfree (INFO_GDP_DEPS (info));
    INFO_GDP_MODULE (info) = NULL;

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static void
AddNamespaceToDependencies (const namespace_t *ns, info *info)
{
    DBUG_ENTER ();

    if (ns != NULL) {
        if (!NSequals (MODULE_NAMESPACE (INFO_GDP_MODULE (info)), ns)) {
            /*
             * this symbol comes from another namespace
             *  -> add the namespace to the dependency list
             */
            global.dependencies
              = STRSadd (NSgetName (ns), STRS_saclib, global.dependencies);
        }
    }

    DBUG_RETURN ();
}

static void
AddModuleToDependencies (const char *mod, info *info)
{
    DBUG_ENTER ();

    if (mod != NULL) {
        if (!STReq (NSgetName (MODULE_NAMESPACE (INFO_GDP_MODULE (info))), mod)) {
            /*
             * this symbol comes from another module
             *  -> add the module name to the dependency list
             */
            global.dependencies = STRSadd (mod, STRS_saclib, global.dependencies);
        }
    }

    DBUG_RETURN ();
}

static ntype *
GDPntype (ntype *arg_type, info *arg_info)
{
    ntype *scalar;

    DBUG_ENTER ();

    if (arg_type != NULL) {
        if (TYisArray (arg_type)) {
            scalar = TYgetScalar (arg_type);
        } else {
            scalar = arg_type;
        }

        if (TYisSymb (scalar)) {
            AddNamespaceToDependencies (TYgetNamespace (scalar), arg_info);
        }
    }

    DBUG_RETURN (arg_type);
}

node *
GDPspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AddNamespaceToDependencies (SPID_NS (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPspfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AddNamespaceToDependencies (SPFOLD_NS (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    TYPEDEF_NTYPE (arg_node) = GDPntype (TYPEDEF_NTYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RET_TYPE (arg_node) = GDPntype (RET_TYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_TYPE (arg_node) = GDPntype (AVIS_TYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPcast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CAST_NTYPE (arg_node) = GDPntype (CAST_NTYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    OBJDEF_TYPE (arg_node) = GDPntype (OBJDEF_TYPE (arg_node), arg_info);

    AddNamespaceToDependencies (OBJDEF_NS (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AddModuleToDependencies (USE_MOD (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AddModuleToDependencies (IMPORT_MOD (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_GDP_MODULE (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_GDP_MODULE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
GDPdoGatherDependencies (node *tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_gdp);

    tree = TRAVdo (tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (tree);
}

#undef DBUG_PREFIX
