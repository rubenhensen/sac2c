#include "object_init.h"

#include "types.h"
#include "tree_basic.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "new_types.h"
#include "str.h"
#include "namespaces.h"
#include "traverse.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundefs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_FUNDEFS(n) ((n)->fundefs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEFS (result) = NULL;

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
/*
 * helper functions
 */

/** <!-- ****************************************************************** -->
 * @fn node *BuildInitFun( char *name, namespace_t *ns,
 *                         ntype *objtype, node *expr)
 *
 * @brief builds an init function. Consumes ALL its arguments!
 *
 * @param name     name of function to build
 * @param ns       namespace -"-
 * @param objtype  returntype -"-
 * @param expr     expression -"-
 *
 * @return N_fundef node
 ******************************************************************************/
static node *
BuildInitFun (char *name, namespace_t *ns, ntype *objtype, node *expr)
{
    node *result;
    node *assign;
    node *argavis;

    DBUG_ENTER ();

    argavis = TBmakeAvis (STRcpy ("_OI_object"), objtype);
    AVIS_DECLTYPE (argavis) = TYcopyType (AVIS_TYPE (argavis));

    /*
     * return( );
     */
    assign = TBmakeAssign (TBmakeReturn (NULL), NULL);

    /*
     * arg = <expr>
     *
     * we have to use a SPIDS here, as this code is added prior to
     * insert vardec!
     */
    assign = TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy ("_OI_object"), NULL), expr),
                           assign);

    /*
     * void <ns>::<name> (<objtype> &arg)
     */
    result = TBmakeFundef (name, ns, NULL, TBmakeArg (argavis, NULL),
                           TBmakeBlock (assign, NULL), NULL);

    FUNDEF_ISOBJINITFUN (result) = TRUE;
    ARG_ISREFERENCE (FUNDEF_ARGS (result)) = TRUE;

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Traverses all objdefs and creates corresponding init functions.
 *        The init functions are stored in the info field for later insertion
 *        into the tree.
 *
 * @param arg_node objdef node
 * @param arg_info info structure
 *
 * @return objdef node with added initfun
 ******************************************************************************/
node *
OIobjdef (node *arg_node, info *arg_info)
{
    node *initfun;

    DBUG_ENTER ();

    OBJDEF_NEXT (arg_node) = TRAVopt(OBJDEF_NEXT (arg_node), arg_info);

    if (OBJDEF_ISLOCAL (arg_node) && !OBJDEF_ISEXTERN (arg_node)
        && !OBJDEF_ISALIAS (arg_node)) {
        initfun
          = BuildInitFun (STRcat ("init_", OBJDEF_NAME (arg_node)), NSgetInitNamespace (),
                          TYcopyType (OBJDEF_TYPE (arg_node)), OBJDEF_EXPR (arg_node));

        OBJDEF_EXPR (arg_node) = NULL;
        OBJDEF_INITFUN (arg_node) = initfun;

        FUNDEF_NEXT (initfun) = INFO_FUNDEFS (arg_info);
        INFO_FUNDEFS (arg_info) = initfun;
    }

    DBUG_RETURN (arg_node);
}

node *
OImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEFS (arg_info) = MODULE_FUNS (arg_node);

    MODULE_OBJS (arg_node) = TRAVopt(MODULE_OBJS (arg_node), arg_info);

    MODULE_FUNS (arg_node) = INFO_FUNDEFS (arg_info);
    INFO_FUNDEFS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *OIdoObjectInit(node *syntax_tree)
 *
 * @brief starter function of object initialisation.
 *
 * During object initialisation, the init expression of every local objdef is
 * lifted into a special init function to allow for arbitraty expressions in
 * object init expressions.
 *
 * @param syntax_tree the entire syntaxtree, thus the N_module node.
 *
 * @return the transformed tree
 ******************************************************************************/
node *
OIdoObjectInit (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_oi);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
