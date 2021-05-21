/******************************************************************************
 *
 * Things done during this traversal:
 *   - All names and identifiers are renamed in order to avoid name clashes.
 *
 ******************************************************************************/

#include "renameidentifiers.h"
#include "tree_basic.h"

#define DBUG_PREFIX "RID"
#include "debug.h"

#include "traverse.h"
#include "scheduling.h"
#include "str_buffer.h"
#include "str.h"
#include "memory.h"
#include "user_types.h"
#include "DataFlowMask.h"
#include "new_types.h"
#include "tree_compound.h"
#include "free.h"
#include "convert.h"
#include "namespaces.h"
#include "globals.h"
#include "rtspec_modes.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
};

/*
 * INFO macros
 */
#define INFO_RID_MODULE(n) ((n)->module)

/*
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_RID_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
RIDdoRenameIdentifiers (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_rid);

    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

static char *
BuildTypesRenaming (const char *mod, const char *name)
{
    char *result;

    DBUG_ENTER ();

    result = (char *)MEMmalloc (sizeof (char) * (STRlen (name) + STRlen (mod) + 8));
    sprintf (result, "SACt_%s__%s", mod, name);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *RenameFunName( char *mod, char *name,
 *                        statustype status, node *args)
 *
 * description:
 *   Renames the given name of a SAC-function.
 *   A new name is created from the module name, the original name and the
 *   argument's types.
 *
 *   This function depends on the traversal as it needs the
 *   ARG_TYPESTRING strings which are annotated in RIDarg
 *
 ******************************************************************************/

static char *
RenameFunName (node *fundef)
{
    char *tmp_name;
    char *new_name;
    char *ns_name;
    char *akv_id = NULL;
    str_buf *buf;
    node *arg;

    DBUG_ENTER ();

    buf = SBUFcreate (40);

    buf = SBUFprint (buf, "SAC");
    if (FUNDEF_ISSPAWNFUN (fundef)) {
        buf = SBUFprint (buf, "s");
    }
    if (FUNDEF_ISWRAPPERFUN (fundef) || FUNDEF_ISWRAPPERENTRYFUN (fundef)) {
        buf = SBUFprint (buf, "w");
    }
    if (FUNDEF_ISINDIRECTWRAPPERFUN (fundef)) {
        buf = SBUFprint (buf, "iw");
    }
    if (FUNDEF_ISTHREADFUN (fundef)) {
        buf = SBUFprint (buf, "t");
    }
    if (FUNDEF_ISOBJECTWRAPPER (fundef)) {
        buf = SBUFprint (buf, "o");
    }
    buf = SBUFprint (buf, "f_");

    tmp_name = STRreplaceSpecialCharacters (FUNDEF_NAME (fundef));
    ns_name = STRreplaceSpecialCharacters (NSgetName (FUNDEF_NS (fundef)));

    buf = SBUFprintf (buf, "%s__%s", ns_name, tmp_name);

    tmp_name = MEMfree (tmp_name);
    ns_name = MEMfree (ns_name);

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        buf = SBUFprintf (buf, "__%s", ARG_TYPESTRING (arg));
        ARG_TYPESTRING (arg) = MEMfree (ARG_TYPESTRING (arg));
        arg = ARG_NEXT (arg);
    }

    if (FUNDEF_AKVID (fundef) > 0) {
        akv_id = STRitoa (FUNDEF_AKVID (fundef));
        buf = SBUFprintf (buf, "__akv_%s", akv_id);
        akv_id = MEMfree (akv_id);
    }

    new_name = SBUF2str (buf);
    buf = SBUFfree (buf);

    DBUG_RETURN (new_name);
}

/******************************************************************************
 *
 * function:
 *   node *RenameFun( node *fun)
 *
 * description:
 *   Renames the given function.
 *   For SAC-functions, a new name is created from the module name, the
 *   original name and the argument's types.
 *   For C-functions, a new name is taken from the pragma 'linkname' if present.
 *
 *   This function depends on the traversal as it needs the
 *   ARG_TYPESTRING strings which are annotated in RIDarg
 *
 ******************************************************************************/

static node *
RenameFun (node *fun)
{
    char *new_name;
    const char *mod_name;

    DBUG_ENTER ();

    FUNDEF_SOURCENAME (fun) = STRcpy (FUNDEF_NAME (fun));

    if (FUNDEF_LINKNAME (fun) != NULL) {
        /*
         * A name has been preset, so we rename the function
         * accordingly. This happens usually because of
         *  - the LINKNAME pragme for external functions
         *  - sac4c setting the LINKNAME
         */

        DBUG_PRINT ("renaming C function %s to %s", FUNDEF_NAME (fun),
                    FUNDEF_LINKNAME (fun));

        FUNDEF_NAME (fun) = MEMfree (FUNDEF_NAME (fun));
        FUNDEF_NAME (fun) = STRcpy (FUNDEF_LINKNAME (fun));
    } else if (FUNDEF_ISEXTERN (fun)) {
        DBUG_PRINT ("C function %s has not been renamed", FUNDEF_NAME (fun));
    } else {
        /*
         * SAC functions which may be overloaded
         */

        mod_name = NSgetModule (FUNDEF_NS (fun));

        if (global.runtime && global.rtspec_mode == RTSPEC_MODE_SIMPLE
            && STReq (mod_name, global.rt_new_module)
            && STReq (FUNDEF_NAME (fun), global.rt_fun_name)
            && FUNDEF_ISINDIRECTWRAPPERFUN (fun)) {
            // rtspec mode simple
            new_name = STRcpy (global.rt_new_name);
        } else if (global.runtime && global.rtspec_mode != RTSPEC_MODE_SIMPLE
                   && STReq (mod_name, global.rt_new_module)
                   && STReq (FUNDEF_NAME (fun), global.rt_fun_name)
                   && FUNDEF_ISSPECIALISATION (fun) && !FUNDEF_ISWRAPPERFUN (fun)) {
            // rtspec mode uuid/hash
            new_name = STRcpy (global.rt_new_name);
        } else {
            new_name = RenameFunName (fun);
        }

        DBUG_PRINT ("renaming SAC function %s:%s to %s", NSgetName (FUNDEF_NS (fun)),
                    FUNDEF_NAME (fun), new_name);

        FUNDEF_NAME (fun) = MEMfree (FUNDEF_NAME (fun));
        FUNDEF_NAME (fun) = new_name;
    }

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *   node *RIDmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
RIDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RID_MODULE (arg_info) = arg_node;

    MODULE_TYPES (arg_node) = TRAVopt (MODULE_TYPES (arg_node), arg_info);
    MODULE_OBJS (arg_node) = TRAVopt (MODULE_OBJS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_THREADFUNS (arg_node) = TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *RIDtypedef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Renames types. All types defined in SAC get the prefix "SAC_" to avoid
 *   name clashes with C identifiers.
 *
 ******************************************************************************/

node *
RIDtypedef (node *arg_node, info *arg_info)
{
    char *newname;
    usertype type;

    DBUG_ENTER ();

    /*
     * Why are imported C types renamed unlike imported C functions or
     * global objects?
     *
     * Imported C types do not have a real counterpart in the C module/class
     * implementation. So, there must be no coincidence at link time.
     * As the type name actually does only exist for the sake of the SAC world,
     * which maps it directly to either void* or some basic type, its renaming
     * avoids potential name clashes with other external symbols.
     */
    newname
      = BuildTypesRenaming (NSgetName (TYPEDEF_NS (arg_node)), TYPEDEF_NAME (arg_node));

    DBUG_PRINT ("renaming type %s:%s to %s", NSgetName (TYPEDEF_NS (arg_node)),
                TYPEDEF_NAME (arg_node), newname);

    /*
     * now we have to rename the type in the user type database
     * as well.
     */
    type = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));
    UTsetName (type, newname);
    UTsetNamespace (type, NULL);

    /*
     * and rename the typedef
     */

    TYPEDEF_NAME (arg_node) = MEMfree (TYPEDEF_NAME (arg_node));
    TYPEDEF_NAME (arg_node) = newname;
    TYPEDEF_NS (arg_node) = NSfreeNamespace (TYPEDEF_NS (arg_node));

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *RIDobjdef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Renames global objects.
 *   For SAC-functions the VARNAME, a combination of module name and object
 *   name is used, for C-functions the optional 'linkname' is used if present.
 *   Additionally, the object's type is renamed as well.
 *
 ******************************************************************************/

node *
RIDobjdef (node *arg_node, info *arg_info)
{
    char *new_name;

    DBUG_ENTER ();

    if (!OBJDEF_ISEXTERN (arg_node)) {
        /*
         * SAC objdef
         */

        new_name
          = (char *)MEMmalloc (sizeof (char)
                               * (STRlen (OBJDEF_NAME (arg_node))
                                  + STRlen (NSgetName (OBJDEF_NS (arg_node))) + 8));

        sprintf (new_name, "SACo_%s__%s", NSgetName (OBJDEF_NS (arg_node)),
                 OBJDEF_NAME (arg_node));

        OBJDEF_NAME (arg_node) = MEMfree (OBJDEF_NAME (arg_node));
        OBJDEF_NAME (arg_node) = new_name;
        OBJDEF_NS (arg_node) = NSfreeNamespace (OBJDEF_NS (arg_node));
    } else {
        /*
         * imported C objdef
         */

        if (OBJDEF_LINKNAME (arg_node) != NULL) {
            OBJDEF_NAME (arg_node) = MEMfree (OBJDEF_NAME (arg_node));
            OBJDEF_NAME (arg_node) = STRcpy (OBJDEF_LINKNAME (arg_node));
        }
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RIDfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
RIDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("processing function \"%s\"", CTIitemName (arg_node));

    if (FUNDEF_ARGS (arg_node) != NULL) {
        DBUG_PRINT ("   processing args ...");
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("   processing body ...");
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    arg_node = RenameFun (arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RIDarg( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
RIDarg (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();

    /*
     * Here, a string representation for the argument types is built which
     * is later on used when renaming the function. If present, we use
     * the declared type here, as the ntype might have been modified (e.g.
     * the udts might have been replaced) so that the resulting signature
     * would not be unique any more.
     */

    if (AVIS_DECLTYPE (ARG_AVIS (arg_node)) != NULL) {
        type = AVIS_DECLTYPE (ARG_AVIS (arg_node));
    } else {
        type = AVIS_TYPE (ARG_AVIS (arg_node));
    }

    ARG_TYPESTRING (arg_node) = CVtype2String (type, 2, TRUE);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *RIDreturn( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
RIDreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *RIDap( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
RIDap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *RIDicm( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
RIDicm (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ICM_ARGS (arg_node) != NULL) {
        ICM_ARGS (arg_node) = TRAVdo (ICM_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RIDwlseg( node *arg_node, info *arg_info)
 *
 * description:
 *   Since the scheduling specification and WLSEGVAR_IDXINF, WLSEGVAR_IDXSUP
 *   may contain the names of local identifiers, these have to be renamed
 *   according to the general renaming scheme implemented by this compiler
 *   phase.
 *
 ******************************************************************************/

node *
RIDwlseg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (arg_node)
          = SCHprecompileScheduling (WLSEG_SCHEDULING (arg_node));

        WLSEG_TASKSEL (arg_node) = SCHprecompileTasksel (WLSEG_TASKSEL (arg_node));
    }

    WLSEG_IDXINF (arg_node) = TRAVopt (WLSEG_IDXINF (arg_node), arg_info);
    WLSEG_IDXSUP (arg_node) = TRAVopt (WLSEG_IDXSUP (arg_node), arg_info);

    WLSEG_CONTENTS (arg_node) = TRAVdo (WLSEG_CONTENTS (arg_node), arg_info);

    WLSEG_NEXT (arg_node) = TRAVopt (WLSEG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RIDavis (node *arg_node, info *arg_info)
{
    char *newname;

    DBUG_ENTER ();

    newname = RIDrenameLocalIdentifier (AVIS_NAME (arg_node));

    AVIS_NAME (arg_node) = newname;

    DBUG_RETURN (arg_node);
}

/*
node *RIDprf(node * arg_node, info * arg_info)
{
  DBUG_ENTER("RIDprf");

  switch( PRF_PRF( arg_node))
  {
    case F_cuda_wlidxs:
      INFO_RID_WLIDXS(arg_info) = TRUE;
      PRF_ARGS( arg_node) = TRAVdo( PRF_ARGS( arg_node), arg_info);
      INFO_RID_WLIDXS(arg_info) = FALSE;
      break;
    case F_cuda_wlids:
      INFO_RID_WLIDS(arg_info) = TRUE;
      PRF_ARGS( arg_node) = TRAVdo( PRF_ARGS( arg_node), arg_info);
      INFO_RID_WLIDS(arg_info) = FALSE;
      break;
    default:
      if( PRF_ARGS( arg_node) != NULL)
        PRF_ARGS( arg_node) = TRAVdo( PRF_ARGS( arg_node), arg_info);
      break;
  }

  DBUG_RETURN(arg_node);
}
*/

/******************************************************************************
 *
 * function:
 *   char *RIDrenameLocalIdentifier( char *id)
 *
 * description:
 *   This function renames a given local identifier name for precompiling
 *   purposes. If the identifier has been inserted by sac2c, i.e. it starts
 *   with an underscore, it is prefixed by SACp. Otherwise, it is prefixed
 *   by SACl.
 *
 *   It also maps the name into an nt (Name Tuple) for tagged arrays.
 *
 ******************************************************************************/

char *
RIDrenameLocalIdentifier (char *id)
{
    char *name_prefix;
    char *new_name;

    DBUG_ENTER ();

    if (id[0] == '_') {
        /*
         * This local identifier was inserted by sac2c.
         */
        name_prefix = "SACp";
        /*
         * Here, we don't need an underscore after the prefix because the name
         * already starts with one.
         */
    } else {
        /*
         * This local identifier originates from the source code.
         */
        name_prefix = "SACl_";
    }

    new_name
      = (char *)MEMmalloc (sizeof (char) * (STRlen (id) + STRlen (name_prefix) + 1));
    sprintf (new_name, "%s%s", name_prefix, id);

    id = MEMfree (id);

    DBUG_RETURN (new_name);
}

#undef DBUG_PREFIX
