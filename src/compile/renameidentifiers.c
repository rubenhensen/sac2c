/*
 *
 * $Log$
 * Revision 1.13  2005/07/03 17:16:37  ktr
 * Initialized a variable.
 *
 * Revision 1.12  2005/05/31 18:11:51  sah
 * fixed a typoe
 *
 * Revision 1.11  2005/02/02 18:14:17  mwe
 * naming for functions with akv-types changed
 *
 * Revision 1.10  2004/12/11 14:47:26  ktr
 * some bugfixes
 *
 * Revision 1.9  2004/12/08 18:01:17  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.8  2004/11/29 17:34:54  sah
 * fixed a mini bug
 *
 * Revision 1.7  2004/11/29 17:29:49  sah
 * fixed a traversal bug
 *
 * Revision 1.6  2004/11/29 15:03:57  sah
 * made it more obust wrt missing old types.
 *
 * Revision 1.5  2004/11/27 02:31:00  jhb
 * maybe fixed the multiple FreeInfo definition
 *
 * Revision 1.4  2004/11/27 02:15:54  sah
 * ...
 *
 * Revision 1.3  2004/11/27 01:41:11  ktr
 * RID
 *
 * Revision 1.2  2004/11/27 01:40:13  ktr
 * typo
 *
 * Revision 1.1  2004/11/27 01:19:20  sah
 * Initial revision
 *
 *
 * [...]
 *
 */

/******************************************************************************
 *
 * Things done during this traversal:
 *   - All names and identifiers are renamed in order to avoid name clashes.
 *
 ******************************************************************************/

#include <string.h>

#include "renameidentifiers.h"
#include "tree_basic.h"
#include "dbug.h"
#include "traverse.h"
#include "scheduling.h"
#include "internal_lib.h"
#include "user_types.h"
#include "DataFlowMask.h"
#include "new_types.h"
#include "tree_compound.h"
#include "free.h"
#include "convert.h"

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_RID_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

node *
RIDdoRenameIdentifiers (node *arg_node)
{
    info *info;

    DBUG_ENTER ("RIDdoRenameIdentifiers");

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

    DBUG_ENTER ("BuildTypesRenaming");

    result = (char *)ILIBmalloc (sizeof (char) * (strlen (name) + strlen (mod) + 8));
    sprintf (result, "SACt_%s__%s", mod, name);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   types *RenameTypes( types *type)
 *
 * description:
 *   Renames the given type if it is a user-defined SAC-type.
 *   Chains of types structures are considered.
 *
 * remarks:
 *   The complete new name is stored in NAME while MOD is set to NULL.
 *
 ******************************************************************************/

static types *
RenameTypes (types *type)
{
    DBUG_ENTER ("RenameTypes");

    if (TYPES_BASETYPE (type) == T_user) {
        char *newname;

        newname = BuildTypesRenaming (TYPES_MOD (type), TYPES_NAME (type));

        DBUG_PRINT ("PREC", ("renaming type %s:%s to %s", TYPES_MOD (type),
                             TYPES_NAME (type), newname));

        TYPES_NAME (type) = ILIBfree (TYPES_NAME (type));
        TYPES_NAME (type) = newname;
        TYPES_MOD (type) = ILIBfree (TYPES_MOD (type));

        if (TYPES_NEXT (type) != NULL) {
            TYPES_NEXT (type) = RenameTypes (TYPES_NEXT (type));
        }
    }

    DBUG_RETURN (type);
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
    char *prefix;
    char *tmp_name;
    char *new_name;
    char *akv_id = NULL;

    DBUG_ENTER ("RenameFunName");

    tmp_name = ILIBreplaceSpecialCharacters (FUNDEF_NAME (fundef));

    if (FUNDEF_ISSPMDFUN (fundef)) {
        new_name = (char *)ILIBmalloc ((strlen (tmp_name) + 6) * sizeof (char));
        sprintf (new_name, "SACf_%s", tmp_name);
    } else {
        node *arg;
        int length = 0;

        arg = FUNDEF_ARGS (fundef);
        while (arg != NULL) {
            length += strlen (ARG_TYPESTRING (arg)) + 2;
            arg = ARG_NEXT (arg);
        }

        if (FUNDEF_ISWRAPPERFUN (fundef)) {
            prefix = "SACwf";
        } else {
            prefix = "SACf";
        }

        if (FUNDEF_AKVID (fundef) > 0) {
            akv_id = ILIBitoa (FUNDEF_AKVID (fundef));
            length += (strlen (prefix) + strlen (FUNDEF_MOD (fundef)) + strlen (tmp_name)
                       + 4 + 6 + strlen (akv_id));
        } else {
            length
              += (strlen (prefix) + strlen (FUNDEF_MOD (fundef)) + strlen (tmp_name) + 4);
        }

        new_name = (char *)ILIBmalloc (length * sizeof (char));
        sprintf (new_name, "%s_%s__%s", prefix, FUNDEF_MOD (fundef), tmp_name);

        arg = FUNDEF_ARGS (fundef);
        while (arg != NULL) {
            strcat (new_name, "__");
            strcat (new_name, ARG_TYPESTRING (arg));
            ARG_TYPESTRING (arg) = ILIBfree (ARG_TYPESTRING (arg));
            arg = ARG_NEXT (arg);
        }
    }

    if (FUNDEF_AKVID (fundef) > 0) {
        strcat (new_name, "__akv_");
        strcat (new_name, akv_id);
        akv_id = ILIBfree (akv_id);
    }

    tmp_name = ILIBfree (tmp_name);

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

    DBUG_ENTER ("RenameFun");

    if (FUNDEF_ISEXTERN (fun)) {
        if (FUNDEF_LINKNAME (fun) != NULL) {
            /*
             * C functions with additional pragma 'linkname'
             */

            DBUG_PRINT ("PREC", ("renaming C function %s to %s", FUNDEF_NAME (fun),
                                 FUNDEF_LINKNAME (fun)));

            FUNDEF_NAME (fun) = ILIBfree (FUNDEF_NAME (fun));
            FUNDEF_NAME (fun) = ILIBstringCopy (FUNDEF_LINKNAME (fun));
        } else {
            DBUG_PRINT ("PREC",
                        ("C function %s has not been renamed", FUNDEF_NAME (fun)));
        }
    } else {
        /*
         * SAC functions which may be overloaded
         */

        new_name = RenameFunName (fun);

        DBUG_PRINT ("PREC", ("renaming SAC function %s:%s to %s", FUNDEF_MOD (fun),
                             FUNDEF_NAME (fun), new_name));

        FUNDEF_NAME (fun) = ILIBfree (FUNDEF_NAME (fun));
        FUNDEF_NAME (fun) = new_name;
        FUNDEF_MOD (fun) = ILIBfree (FUNDEF_MOD (fun));
    }

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *   char *RIDobjInitFunctionName( bool before_rename)
 *
 * description:
 *   Returns new allocated string with objinitfunction name
 *
 * parameters:
 *   uses global variable modulename!
 *
 ******************************************************************************/

char *
RIDobjInitFunctionName (bool before_rename)
{
    char *name = "GlobalObjInit";
    char *new_name;

    DBUG_ENTER ("RIDobjInitFunctionName");

    if (before_rename) {
        new_name = (char *)ILIBmalloc (strlen (name) + 1);

        strcpy (new_name, name);
    } else {
        new_name = ILIBmalloc (strlen (name) + strlen (MAIN_MOD_NAME) + 8);
        sprintf (new_name, "SACf_%s__%s", MAIN_MOD_NAME, name);
    }

    DBUG_RETURN (new_name);
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
    DBUG_ENTER ("RIDmodul");

    INFO_RID_MODULE (arg_info) = arg_node;

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

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

    DBUG_ENTER ("RIDtypedef");

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
    newname = BuildTypesRenaming (TYPEDEF_MOD (arg_node), TYPEDEF_NAME (arg_node));

    DBUG_PRINT ("PREC", ("renaming type %s:%s to %s", TYPEDEF_MOD (arg_node),
                         TYPEDEF_NAME (arg_node), newname));

    /*
     * now we have to rename the type in the user type database
     * as well.
     */
    type = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_MOD (arg_node));
    UTsetName (type, newname);
    UTsetMod (type, NULL);

    /*
     * and rename the typedef
     */

    TYPEDEF_NAME (arg_node) = ILIBfree (TYPEDEF_NAME (arg_node));
    TYPEDEF_NAME (arg_node) = newname;
    TYPEDEF_MOD (arg_node) = ILIBfree (TYPEDEF_MOD (arg_node));

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

    DBUG_ENTER ("RIDobjdef");

    if (!OBJDEF_ISEXTERN (arg_node)) {
        /*
         * SAC objdef
         */

        OBJDEF_VARNAME (arg_node) = ILIBfree (OBJDEF_VARNAME (arg_node));
        /*
         * OBJDEF_VARNAME is no longer used for the generation of the final C code
         * identifier of a global object.
         */

        new_name = (char *)ILIBmalloc (
          sizeof (char)
          * (strlen (OBJDEF_NAME (arg_node)) + strlen (OBJDEF_MOD (arg_node)) + 8));

        sprintf (new_name, "SACo_%s__%s", OBJDEF_MOD (arg_node), OBJDEF_NAME (arg_node));

        OBJDEF_NAME (arg_node) = ILIBfree (OBJDEF_NAME (arg_node));
        OBJDEF_NAME (arg_node) = new_name;
        OBJDEF_MOD (arg_node) = ILIBfree (OBJDEF_MOD (arg_node));
    } else {
        /*
         * imported C objdef
         */

        /*
         * TODO: why are the external objects not renamed ?!?
         */
        if (OBJDEF_LINKNAME (arg_node) != NULL) {
            OBJDEF_NAME (arg_node) = ILIBfree (OBJDEF_NAME (arg_node));
            OBJDEF_NAME (arg_node) = OBJDEF_LINKNAME (arg_node);
            OBJDEF_PRAGMA (arg_node) = ILIBfree (OBJDEF_PRAGMA (arg_node));
        } else {
        }

        if (OBJDEF_NEXT (arg_node) != NULL) {
            OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
        }
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
    DBUG_ENTER ("RIDfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * Now, the data flow mask base is updated.
     * This is necessary because some local identifiers are removed while all
     * others are renamed.
     */
    if (FUNDEF_DFM_BASE (arg_node) != NULL) {
        DBUG_ASSERT ((FUNDEF_BODY (arg_node) != NULL),
                     "FUNDEF_DFM_BASE without body found!");

        FUNDEF_DFM_BASE (arg_node)
          = DFMupdateMaskBaseAfterRenaming (FUNDEF_DFM_BASE (arg_node),
                                            FUNDEF_ARGS (arg_node),
                                            BLOCK_VARDEC (FUNDEF_BODY ((arg_node))));
    }

    arg_node = RenameFun (arg_node);

    if (FUNDEF_TYPES (arg_node) != NULL) {
        FUNDEF_TYPES (arg_node) = RenameTypes (FUNDEF_TYPES (arg_node));
    }

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
    types *ot;

    DBUG_ENTER ("RIDarg");

    /*
     * Here, a string representation for the argument types is built which
     * is lateron used when renaming the function
     */
    ot = TYtype2OldType (ARG_NTYPE (arg_node));
    ARG_TYPESTRING (arg_node) = CVtype2String (ot, 2, TRUE);
    ot = FREEfreeOneTypes (ot);

    if (ARG_TYPE (arg_node) != NULL) {
        ARG_TYPE (arg_node) = RenameTypes (ARG_TYPE (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *RIDvardec( node *arg_node, info *arg_info)
 *
 * Description:
 *   Renames types of declared variables.
 *
 ******************************************************************************/

node *
RIDvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RIDvardec");

    if (VARDEC_TYPE (arg_node) != NULL) {
        VARDEC_TYPE (arg_node) = RenameTypes (VARDEC_TYPE (arg_node));
    }

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
    DBUG_ENTER ("RIDreturn");

    if (RETURN_REFERENCE (arg_node) != NULL) {
        RETURN_REFERENCE (arg_node) = TRAVdo (RETURN_REFERENCE (arg_node), arg_info);
    }

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
    DBUG_ENTER ("RIDap");

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
    DBUG_ENTER ("RIDicm");

    if (ICM_ARGS (arg_node) != NULL) {
        ICM_ARGS (arg_node) = TRAVdo (ICM_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *RIDwlsegx( node *arg_node, info *arg_info)
 *
 * description:
 *   Since the scheduling specification and WLSEGVAR_IDX_MIN, WLSEGVAR_IDX_MAX
 *   may contain the names of local identifiers, these have to be renamed
 *   according to the general renaming scheme implemented by this compiler
 *   phase.
 *
 ******************************************************************************/

static node *
RIDwlsegx (node *arg_node, info *arg_info)
{
    int d;

    DBUG_ENTER ("RIDwlsegx");

    if (WLSEGX_SCHEDULING (arg_node) != NULL) {
        L_WLSEGX_SCHEDULING (arg_node,
                             SCHprecompileScheduling (WLSEGX_SCHEDULING (arg_node)));

        L_WLSEGX_TASKSEL (arg_node, SCHprecompileTasksel (WLSEGX_TASKSEL (arg_node)));
    }

    if (NODE_TYPE (arg_node) == N_wlsegvar) {
        DBUG_ASSERT ((WLSEGVAR_IDX_MIN (arg_node) != NULL),
                     "WLSEGVAR_IDX_MIN not found!");
        DBUG_ASSERT ((WLSEGVAR_IDX_MAX (arg_node) != NULL),
                     "WLSEGVAR_IDX_MAX not found!");
        for (d = 0; d < WLSEGVAR_DIMS (arg_node); d++) {
            (WLSEGVAR_IDX_MIN (arg_node))[d]
              = TRAVdo ((WLSEGVAR_IDX_MIN (arg_node))[d], arg_info);
            (WLSEGVAR_IDX_MAX (arg_node))[d]
              = TRAVdo ((WLSEGVAR_IDX_MAX (arg_node))[d], arg_info);
        }
    }

    L_WLSEGX_CONTENTS (arg_node, TRAVdo (WLSEGX_CONTENTS (arg_node), arg_info));

    if (WLSEGX_NEXT (arg_node) != NULL) {
        L_WLSEGX_NEXT (arg_node, TRAVdo (WLSEGX_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (arg_node);
}

node *
RIDwlseg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RIDwlseg");

    arg_node = RIDwlsegx (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RIDwlsegvar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RIDwlsegvar");

    arg_node = RIDwlsegx (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RIDavis (node *arg_node, info *arg_info)
{
    char *newname;

    DBUG_ENTER ("RIDavis");

    newname = RIDrenameLocalIdentifier (AVIS_NAME (arg_node));

    AVIS_NAME (arg_node) = newname;

    DBUG_RETURN (arg_node);
}

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

    DBUG_ENTER ("RIDrenameLocalIdentifier");

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
      = (char *)ILIBmalloc (sizeof (char) * (strlen (id) + strlen (name_prefix) + 1));
    sprintf (new_name, "%s%s", name_prefix, id);

    id = ILIBfree (id);

    DBUG_RETURN (new_name);
}
