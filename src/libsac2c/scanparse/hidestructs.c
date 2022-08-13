#include "hidestructs.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "DupTree.h"
#include "free.h"
#include "traverse.h"
#include "types.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"

#define DBUG_PREFIX "HS"
#include "debug.h"

#include "memory.h"
#include "str.h"
#include "ctinfo.h"
#include "globals.h"

/**
 *
 * @file hidestructs.c
 *
 * Hide struct definitions behind typedefs and accessors.
 *
 * This leverages existing functionality to deal with the structs. The accessors
 * and structdefs are resolved (i.e.: any traces of structs are gone) in a
 * seperate phase after the typechecker (NOT IMPLEMENTED).
 */

/**
 * INFO structure
 * - module: Pointer to the N_module.
 * - structtype: pointer to a ntype object that represents the userntype of the
 *   structure that is currently being traversed. NULL if there is no structdef
 *   in the current traversal stack.
 * - init_args: List of N_arg nodes that is rebuilt for every structdef. After
 *   deep traversal it is used to create a constructor fundec.
 * - elemcount: Keep track of the number of elements during traversal.
 */
struct INFO {
    node *module;
    node *init_args;
    node *structdef;
    ntype *structtype;
    int elemcount;
};

/**
 * INFO macros
 */
#define INFO_MODULE(n) ((n)->module)
#define INFO_INIT_ARGS(n) ((n)->init_args)
#define INFO_STRUCTDEF(n) ((n)->structdef)
#define INFO_STRUCTTYPE(n) ((n)->structtype)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MODULE (result) = NULL;
    INFO_INIT_ARGS (result) = NULL;
    INFO_STRUCTDEF (result) = NULL;
    INFO_STRUCTTYPE (result) = NULL;

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
 *
 * @fn node *HSdoHideStructs( node *syntax_tree)
 *
 *   @brief  Prepare, initiate and clean up the HideStructs phase.
 *
 ******************************************************************************/
node *
HSdoHideStructs (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting struct hiding.");

    info = MakeInfo ();

    TRAVpush (TR_hs);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Done hiding all structs.");

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *HSmodule( node *arg_node, info *arg_info)
 *
 *   @brief  Store a pointer to the N_module in the arg_info struct.
 *
 ******************************************************************************/
node *
HSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MODULE (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_MODULE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HSstructdef( node *arg_node, info *arg_info)
 *
 *   @brief  Create an appropriate typedef, constructor and identity function (=
 *           overloaded constructor) for this structdef.
 *
 ******************************************************************************/
node *
HSstructdef (node *arg_node, info *arg_info)
{
    node *module;
    node *typedef_;
    node *fundec;
    node *ret;
    node *arg;
    ntype *structtype;
    char *sname;

    DBUG_ENTER ();

    if (!global.enable_structs) {
        CTIabort (EMPTY_LOC,
          "Structs are a novel feature of SAC that is only partially implemented. "
          "Struct support needs to be enabled through the -enable_structs option. "
          "Beware that code with structs may arbitrarily misbehave for the time being.");
    }

    module = INFO_MODULE (arg_info);
    sname = STRcat (STRUCT_TYPE, STRUCTDEF_NAME (arg_node));
    /* TODO: should 2nd arg be MODULE_NAMESPACE( arg_node)? */
    /* Leaving the last arg as NULL for now is important because this node is
     * also copied by the getter/setter fundecs.
     */
    typedef_
      = TBmakeTypedef (STRcpy (sname), NULL, STRcpy (global.default_component_name),
                       TYmakeAKS (TYmakeHiddenSimpleType (UT_NOT_DEFINED),
                                  SHmakeShape (0)),
                       NULL, NULL);
    /* Store a reference to this structdef in the typedef. */
    TYPEDEF_STRUCTDEF (typedef_) = arg_node;
    /* Push the typedef on the module's typedef stack. */
    TYPEDEF_NEXT (typedef_) = MODULE_TYPES (module);
    MODULE_TYPES (module) = typedef_;
    /* Prepare the argument list for the N_structelem handlers. */
    DBUG_ASSERT (INFO_INIT_ARGS (arg_info) == NULL,
                 "Garbage constructor arguments lying around in arg_info.");
    INFO_STRUCTDEF (arg_info) = arg_node;
    /* Create a new userntype for this struct (typechecker will link above typedef
     * to this userntype). */
    structtype = TYmakeAKS (TYmakeSymbType (STRcpy (sname), NULL), SHmakeShape (0));
    INFO_STRUCTTYPE (arg_info) = structtype;
    sname = MEMfree (sname);

    STRUCTDEF_STRUCTELEM (arg_node) = TRAVopt (STRUCTDEF_STRUCTELEM (arg_node), arg_info);

    /* Create two constructors: one with every element as a seperate argument, one
     * with another struct as the sole argument. */
    ret = TBmakeRet (TYcopyType (structtype), NULL);
    /* First constructor. */
    fundec = TBmakeFundef (STRcpy (STRUCTDEF_NAME (arg_node)), NULL, ret,
                           INFO_INIT_ARGS (arg_info), NULL, MODULE_FUNDECS (module));
    FUNDEF_ISEXTERN (fundec) = TRUE;
    FUNDEF_ISSTRUCTCONSTR (fundec) = TRUE;
    MODULE_FUNDECS (module) = fundec;
    /* Second constructor (replace only the argument list). */
    fundec = DUPdoDupNode (fundec);
    arg = TBmakeArg (TBmakeAvis (STRcpy ("s"), TYcopyType (structtype)), NULL);
    AVIS_DECLTYPE (ARG_AVIS (arg)) = TYcopyType (structtype);
    if (FUNDEF_ARGS (fundec) != NULL) {
        FREEdoFreeTree (FUNDEF_ARGS (fundec));
    }
    FUNDEF_ARGS (fundec) = arg;
    FUNDEF_NEXT (fundec) = MODULE_FUNDECS (module);
    /* TODO: Instead of sticky, recreate on-demand in 9:des. */
    FUNDEF_ISSTICKY (fundec) = TRUE;
    MODULE_FUNDECS (module) = fundec;
    STRUCTDEF_COPYCONSTRUCTOR (arg_node) = fundec;

    INFO_INIT_ARGS (arg_info) = NULL;
    INFO_STRUCTDEF (arg_info) = NULL;
    INFO_STRUCTTYPE (arg_info) = TYfreeType (INFO_STRUCTTYPE (arg_info));

    STRUCTDEF_NEXT (arg_node) = TRAVopt (STRUCTDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HSstructelem( node *arg_node, info *arg_info)
 *
 *   @brief  Create getters and setters for every element of the struct (=
 *           N_structelem).
 *
 ******************************************************************************/
node *
HSstructelem (node *arg_node, info *arg_info)
{
    node *fundec;
    node *arg;
    node *ret;
    node *module;
    node *structdef;
    ntype *structtype;
    node *avis;

    DBUG_ENTER ();

    module = INFO_MODULE (arg_info);
    DBUG_ASSERT (module != NULL, "No module set for this struct element.");
    structdef = INFO_STRUCTDEF (arg_info);
    DBUG_ASSERT (structdef != NULL, "No structdef for this struct element.");
    structtype = INFO_STRUCTTYPE (arg_info);
    DBUG_ASSERT (structtype != NULL, "No struct set for this struct element.");

    /* Create getter as new external fundec. */
    arg = TBmakeArg (TBmakeAvis (STRcpy ("s"), TYcopyType (structtype)), NULL);
    AVIS_DECLTYPE (ARG_AVIS (arg)) = TYcopyType (structtype);
    ret = TBmakeRet (TYcopyType (STRUCTELEM_TYPE (arg_node)), NULL);
    fundec = TBmakeFundef (STRcat (STRUCT_GET, STRUCTELEM_NAME (arg_node)),
                           NULL, /* TODO: namespace? */
                           ret, arg, NULL, MODULE_FUNDECS (module));
    FUNDEF_ISEXTERN (fundec) = TRUE;
    FUNDEF_STRUCTGETTER (fundec) = arg_node;
    /* Push the getter on the module's fundec stack. */
    MODULE_FUNDECS (module) = fundec;
    /* Setter. */
    arg = TBmakeArg (TBmakeAvis (STRcpy ("s"), TYcopyType (structtype)), NULL);
    AVIS_DECLTYPE (ARG_AVIS (arg)) = TYcopyType (structtype);
    arg = TBmakeArg (TBmakeAvis (STRcpy ("e"), TYcopyType (STRUCTELEM_TYPE (arg_node))),
                     arg);
    AVIS_DECLTYPE (ARG_AVIS (arg)) = TYcopyType (STRUCTELEM_TYPE (arg_node));
    ret = TBmakeRet (TYcopyType (structtype), NULL);
    fundec = TBmakeFundef (STRcat (STRUCT_SET, STRUCTELEM_NAME (arg_node)), NULL, ret,
                           arg, NULL, MODULE_FUNDECS (module));
    FUNDEF_ISEXTERN (fundec) = TRUE;
    FUNDEF_STRUCTSETTER (fundec) = arg_node;
    /* Setter on the stack. */
    MODULE_FUNDECS (module) = fundec;
    /* Create a typedef for this struct element. */
    STRUCTELEM_TYPEDEF (arg_node)
      = TBmakeTypedef (STRcatn (4, STRUCT_ELEM, STRUCTDEF_NAME (structdef), "_",
                                STRUCTELEM_NAME (arg_node)),
                       NULL, STRcpy (global.default_component_name),
                       TYcopyType (STRUCTELEM_TYPE (arg_node)), NULL,
                       MODULE_TYPES (module));
    /* Store a pointer to this elem's structdef. */
    TYPEDEF_STRUCTDEF (STRUCTELEM_TYPEDEF (arg_node)) = structdef;
    MODULE_TYPES (module) = STRUCTELEM_TYPEDEF (arg_node);

    /* Continue with the next structelem. */
    STRUCTELEM_NEXT (arg_node) = TRAVopt (STRUCTELEM_NEXT (arg_node), arg_info);

    /* Add the argument for this element to the constructor's args list. This must
     * be done bottom-up to honour the order of the arg declarations in the
     * structdef.
     */

    avis = TBmakeAvis (STRcpy (STRUCTELEM_NAME (arg_node)),
                       TYcopyType (STRUCTELEM_TYPE (arg_node)));

    arg = TBmakeArg (avis, INFO_INIT_ARGS (arg_info));
    AVIS_DECL (avis) = arg;
    AVIS_DECLTYPE (avis) = TYcopyType (STRUCTELEM_TYPE (arg_node));

    INFO_INIT_ARGS (arg_info) = arg;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
