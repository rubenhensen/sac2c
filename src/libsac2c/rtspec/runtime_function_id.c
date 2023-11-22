/**<!--*********************************************************************-->
 *
 * @file runtime_function_id.c
 *
 * @brief Traversal for setting unique function ids to generic functions.
 *
 * @author hmw
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "memory.h"

#define DBUG_PREFIX "UID"
#include "debug.h"

#include "rtspec_modes.h"
#include "globals.h"
#include "traverse.h"
#include "new_types.h"
#include "tree_compound.h"
#include "str.h"

#if ENABLE_UUID
#include <uuid/uuid.h>
#endif /* ENABLE_UUID */

#if ENABLE_HASH
#include <time.h>
#include <unistd.h>
#ifndef __APPLE__
#include <crypt.h>
#endif /* __APPLE__ */
#endif /* ENABLE_HASH */

struct INFO {
    node *module;
    bool isgeneric;
    bool isuser;
};

#define INFO_MODULE(n) ((n)->module)
#define INFO_ISGENERIC(n) ((n)->isgeneric)
#define INFO_ISUSER(n) ((n)->isuser)

static info *
MakeInfo (info *arg_info)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *arg_info)
{
    DBUG_ENTER ();

    arg_info = MEMfree (arg_info);

    DBUG_RETURN (arg_info);
}

/** <!--********************************************************************-->
 *
 * @fn UIDarg (node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param  arg_node  The current node of the syntax tree.
 * @param  arg_info  Info object, unused.
 *
 * @return  The updated arg node.
 *
 *****************************************************************************/
node *
UIDarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ntype *argtype = ARG_NTYPE (arg_node);

    if (TYisArray (argtype) && TYisUser (TYgetScalar (argtype))
        && !ARG_ISARTIFICIAL (arg_node)) {
        INFO_ISUSER (arg_info) = TRUE;
    }

    if (!TYisAKS (argtype)) {
        INFO_ISGENERIC (arg_info) = TRUE;
    }

    ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn UIDfundef (node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param  arg_node  The current node of the syntax tree.
 * @param  arg_info  Info object, unused.
 *
 * @return  The updated fundef node.
 *
 *****************************************************************************/
node *
UIDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#if ENABLE_UUID
    uuid_t uuid;
#endif /* ENABLE_UUID */
#if ENABLE_HASH
    time_t seconds;
    char hostname[1024];
    hostname[1023] = '\0';
    char *str_id;
    char *str_seconds;
#endif /* ENABLE_HASH */

    if (!FUNDEF_HASDOTARGS (arg_node) && !FUNDEF_HASDOTRETS (arg_node)
        && !FUNDEF_ISWRAPPERFUN (arg_node) && !FUNDEF_ISCONDFUN (arg_node)
        && !FUNDEF_ISLOOPFUN (arg_node) && FUNDEF_ARGS (arg_node) != NULL) {
        INFO_ISGENERIC (arg_info) = FALSE;
        INFO_ISUSER (arg_info) = FALSE;

        FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

        if (INFO_ISGENERIC (arg_info) && !INFO_ISUSER (arg_info)) {
            FUNDEF_RTSPECID (arg_node) = (char *)MEMmalloc (sizeof (char) * 37);

#if ENABLE_UUID
            if (global.rtspec_mode == RTSPEC_MODE_UUID) {
                uuid_generate (uuid);
                uuid_unparse_lower (uuid, FUNDEF_RTSPECID (arg_node));
            }
#endif /* ENABLE_UUID */

#if ENABLE_HASH
            if (global.rtspec_mode == RTSPEC_MODE_HASH) {
                gethostname (hostname, 1023);
                seconds = time (NULL);

                str_seconds = (char *)MEMmalloc (sizeof (char) * 11);
                snprintf (str_seconds, 11, "%ld", (long)seconds);

                str_id = STRcatn (3, FUNDEF_NAME (arg_node), hostname, str_seconds);

                FUNDEF_RTSPECID (arg_node) = STRcpy (crypt (str_id, "$1$RTspec$"));

                MEMfree (str_id);
                MEMfree (str_seconds);
            }
#endif /* ENABLE_HASH */
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*
         * Traverse all the functions in the fundef chain.
         */
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn UIDmodule (node *arg_node, info *arg_info)
 *
 * @brief Go over modules and traverse only the ones that actually have functions
 *
 * @param arg_node  the syntax tree.
 *
 * @return  the updated and extended syntax tree.
 *
 * ***************************************************************************/
node *
UIDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (MODULE_FUNS (arg_node) != NULL) {
        INFO_MODULE (arg_info) = arg_node;
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn UIDdoSetFunctionIDs( node *arg_node)
 *
 * @brief Start the traversal for setting unique function ids.
 *
 * @param arg_node  the syntax tree.
 *
 * @return  the updated and extended syntax tree.
 *
 * ***************************************************************************/
node *
UIDdoSetFunctionIDs (node *arg_node)
{
    DBUG_ENTER ();

    info *info = NULL;

    info = MakeInfo (info);

    TRAVpush (TR_uid);

    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
