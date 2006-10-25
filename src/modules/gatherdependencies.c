/**
 * $Id$
 *
 * @file gatherdependencies.c
 * @brief gathers the dependencies of a sac file
 * @author Stephan Herhut
 * @date 2005-06-01
 */

#include "gatherdependencies.h"

#include "filemgr.h"
#include "stringset.h"
#include "tree_basic.h"
#include "dbug.h"
#include "internal_lib.h"
#include "new_types.h"
#include "traverse.h"
#include "namespaces.h"

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
#define INFO_GDP_MODULE(n) ((n)->module)

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_GDP_DEPS (result) = NULL;
    INFO_GDP_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_GDP_DEPS (info) = STRSfree (INFO_GDP_DEPS (info));
    INFO_GDP_MODULE (info) = NULL;

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

static void
AddNamespaceToDependencies (const namespace_t *ns, info *info)
{
    DBUG_ENTER ("AddNamespaceToDependencies");

    if (ns != NULL) {
        if (!NSequals (MODULE_NAMESPACE (INFO_GDP_MODULE (info)), ns)) {
            /*
             * this symbol comes from another namespace
             *  -> add the namespace to the dependency list
             */
            MODULE_DEPENDENCIES (INFO_GDP_MODULE (info))
              = STRSadd (NSgetName (ns), STRS_saclib,
                         MODULE_DEPENDENCIES (INFO_GDP_MODULE (info)));
        }
    }

    DBUG_VOID_RETURN;
}

static void
AddModuleToDependencies (const char *mod, info *info)
{
    DBUG_ENTER ("AddModuleToDependencies");

    if (mod != NULL) {
        if (!ILIBstringCompare (NSgetName (MODULE_NAMESPACE (INFO_GDP_MODULE (info))),
                                mod)) {
            /*
             * this symbol comes from another module
             *  -> add the module name to the dependency list
             */
            MODULE_DEPENDENCIES (INFO_GDP_MODULE (info))
              = STRSadd (mod, STRS_saclib, MODULE_DEPENDENCIES (INFO_GDP_MODULE (info)));
        }
    }

    DBUG_VOID_RETURN;
}

static ntype *
GDPntype (ntype *arg_type, info *arg_info)
{
    ntype *scalar;

    DBUG_ENTER ("GDPntype");

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

static types *
GDPtypes (types *arg_types, info *arg_info)
{
    DBUG_ENTER ("GDPtypes");

    if (arg_types != NULL) {
        AddModuleToDependencies (TYPES_MOD (arg_types), arg_info);

        TYPES_NEXT (arg_types) = GDPtypes (TYPES_NEXT (arg_types), arg_info);
    }

    DBUG_RETURN (arg_types);
}

node *
GDPspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPspid");

    AddNamespaceToDependencies (SPID_NS (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPspfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPspfold");

    AddNamespaceToDependencies (SPFOLD_NS (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPtypedef");

    TYPEDEF_NTYPE (arg_node) = GDPntype (TYPEDEF_NTYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPret");

    RET_TYPE (arg_node) = GDPntype (RET_TYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDParg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDParg");

    ARG_TYPE (arg_node) = GDPtypes (ARG_TYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPavis");

    AVIS_TYPE (arg_node) = GDPntype (AVIS_TYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPcast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPcast");

    CAST_NTYPE (arg_node) = GDPntype (CAST_NTYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPobjdef");

    OBJDEF_TYPE (arg_node) = GDPntype (OBJDEF_TYPE (arg_node), arg_info);

    AddNamespaceToDependencies (OBJDEF_NS (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPfundef");

    FUNDEF_TYPES (arg_node) = GDPtypes (FUNDEF_TYPES (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPvardec");

    VARDEC_TYPE (arg_node) = GDPtypes (VARDEC_TYPE (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPuse");

    AddModuleToDependencies (USE_MOD (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPimport");

    AddModuleToDependencies (IMPORT_MOD (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
GDPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GDPmodule");

    INFO_GDP_MODULE (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_GDP_MODULE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
GDPdoGatherDependencies (node *tree)
{
    info *info;

    DBUG_ENTER ("GDPdoGatherDependencies");

    info = MakeInfo ();

    TRAVpush (TR_gdp);

    tree = TRAVdo (tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (tree);
}
