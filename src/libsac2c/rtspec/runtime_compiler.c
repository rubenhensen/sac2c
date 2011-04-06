/*
 *
 * $Id$
 *
 */

/** <!--*******************************************************************-->
 *
 * @file runtime_compiler.c
 *
 * @brief This file contains the functions that setup the compiler to be run at
 * runtime (for runtime optimization).
 *
 * @author tvd
 *
 ****************************************************************************/

#include <string.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "namespaces.h"
#include "globals.h"
#include "memory.h"
#include "dbug.h"
#include "str.h"
#include "traverse.h"
#include "filemgr.h"
#include "new_types.h"
#include "shape.h"

#define NAME_LENGTH 100
#define TYPE_DELIM "-"
#define SHAPE_DELIM "-"

/**<!--********************************************************************-->
 *
 * @fn parseType( char *token)
 *
 * @brief Translate a string representation of a type to a simpletype.
 *
 * @param token  A string representing a certain type.
 *
 * @return  A simpletype object.
 *
 ****************************************************************************/
static ntype *
parseType (char *token)
{
    if (STRlen (token) == 0) {
        return NULL;
    }

    if (STReq (token, "float")) {
        return TYmakeSimpleType (T_float);
    }
    if (STReq (token, "bool")) {
        return TYmakeSimpleType (T_bool);
    }
    if (STReq (token, "byte")) {
        return TYmakeSimpleType (T_byte);
    }
    if (STReq (token, "short")) {
        return TYmakeSimpleType (T_short);
    }
    if (STReq (token, "int")) {
        return TYmakeSimpleType (T_int);
    }
    if (STReq (token, "long")) {
        return TYmakeSimpleType (T_long);
    }
    if (STReq (token, "longlong")) {
        return TYmakeSimpleType (T_longlong);
    }
    if (STReq (token, "ubyte")) {
        return TYmakeSimpleType (T_ubyte);
    }
    if (STReq (token, "ushort")) {
        return TYmakeSimpleType (T_ushort);
    }
    if (STReq (token, "uint")) {
        return TYmakeSimpleType (T_uint);
    }
    if (STReq (token, "ulong")) {
        return TYmakeSimpleType (T_ulong);
    }
    if (STReq (token, "ulonglong")) {
        return TYmakeSimpleType (T_ulonglong);
    }
    if (STReq (token, "char")) {
        return TYmakeSimpleType (T_char);
    }
    if (STReq (token, "double")) {
        return TYmakeSimpleType (T_double);
    }

    return NULL;
}

/**<!--********************************************************************-->
 *
 * @fn parseArguments( char *type_info, char *shape_info)
 *
 * @brief This function parses the commandline arguments and creates an ARGnode
 * chain with the information.
 *
 * @param type_info  A list with the types of each argument.
 * @param shape_info A list with the shape information for each argument.
 *
 * @return An ARGnode chain with all the arguments needed for specialization.
 *
 ****************************************************************************/
static node *
parseArguments (char *type_info, char *shape_info)
{
    char *ttoken, *stoken, *type_saveptr = NULL, *shp_saveptr = NULL;

    char var_name[25];

    int num_args, i, j, dims, extent;

    ntype *current_type;

    shape *current_shape;

    node *args = NULL, *current_arg = NULL;

    DBUG_ENTER ("parseArguments");

    /* Get the total number of arguments that is to be parsed. */
    stoken = strtok_r (shape_info, SHAPE_DELIM, &shp_saveptr);
    num_args = atoi (stoken);
    global.rt_num_args = num_args;

    /* Get the first type. */
    ttoken = strtok_r (type_info, TYPE_DELIM, &type_saveptr);

    /* Parse all the arguments. */
    i = 0;
    for (; i < num_args; i++) {
        /* Give the argument a random name. */
        sprintf (var_name, "arg_%d", i);

        /* Get the simpletype of the argumten. */
        current_type = parseType (ttoken);

        /* Get the number of dimensions of the current argument. */

        if ((stoken = strtok_r (NULL, SHAPE_DELIM, &shp_saveptr)) == NULL) {
            DBUG_ASSERT (0, "SHAPE_INFO: format error, missing dimension information!");
        }

        dims = atoi (stoken);

        /* Create the correct shape and set the extent in each dimension. */
        current_shape = SHmakeShape (dims);

        if (dims > 0) {
            j = 0;
            for (; j < dims; j++) {
                stoken = strtok_r (NULL, SHAPE_DELIM, &shp_saveptr);

                DBUG_ASSERT (stoken != NULL, "Missing dimensional extent information!");

                extent = atoi (stoken);
                SHsetExtent (current_shape, j, extent);
            }
        }

        current_type = TYmakeAKS (current_type, current_shape);

        /* Set the first argument or append the next argument.*/
        if (args == NULL) {
            args = TBmakeArg (TBmakeAvis (STRcpy (var_name), current_type), NULL);

            current_arg = args;
        } else {
            ARG_NEXT (current_arg)
              = TBmakeArg (TBmakeAvis (STRcpy (var_name), current_type), NULL);

            current_arg = ARG_NEXT (current_arg);
        }

        AVIS_DECLTYPE (ARG_AVIS (current_arg)) = TYcopyType (ARG_NTYPE (current_arg));

        /* Get the next argument type. */
        ttoken = strtok_r (NULL, TYPE_DELIM, &type_saveptr);
    }

    DBUG_RETURN (args);
}

/**<!--********************************************************************-->
 *
 * @fn  RTsetupRuntimeCompiler( node *syntax_tree)
 *
 * @brief  Setup the compiler for operation in runtime compilation mode.
 *
 * @param  syntax_tree  The abstract syntax tree (Should be NULL in this
 * case!).
 *
 * @return  A freshly made syntax tree for the following code:
 *
 * ----------------------------------------
 *
 *   module <new_module>;
 *
 *   import <old_module>: { <function> };
 *
 *   export all;
 *
 * ----------------------------------------
 *
 ****************************************************************************/
node *
RTsetupRuntimeCompiler (node *syntax_tree)
{
    node *import;
    node *export;
    node *args;

    DBUG_ENTER ("RTdoParseArguments");

    /*
     * Make sure all the necessary information is present.
     */
    DBUG_ASSERT (STRlen (global.rt_type_info), "Missing type info!");
    DBUG_ASSERT (STRlen (global.rt_shape_info), "Missing shape info!");

    DBUG_ASSERT (STRlen (global.rt_old_module) && STRlen (global.rt_new_module),
                 "Missing module info!");

    DBUG_ASSERT (STRlen (global.rt_fun_name), "Missing original function name!");
    DBUG_ASSERT (STRlen (global.rt_new_name), "Missing new function name!");

    args = parseArguments (global.rt_type_info, global.rt_shape_info);

    /*
     * Import the function symbols from the original module or the previously
     * optimized module.
     */
    import = TBmakeImport (STRcpy (global.rt_old_module), NULL,
                           TBmakeSymbol (STRcpy (global.rt_fun_name), NULL));

    IMPORT_ALL (import) = FALSE;

    /*
     * Export all instances.
     */
    export = TBmakeExport (import, NULL);
    EXPORT_ALL (export) = TRUE;

    /*
     * Create a syntax tree with just a module definition, import definition and
     * export all the function instances.
     *
     * The namespace is created according to the name of the module.
     */
    syntax_tree = TBmakeModule (NSgetNamespace (global.rt_new_module), FT_modimp, NULL,
                                NULL, NULL, NULL, NULL);

    MODULE_INTERFACE (syntax_tree) = export;

    /*
     * Make sure the relevant global values are filled in. This is normally done
     * in the first phase, however, that phase is skipped in runtime mode.
     */
    FMGRsetFileNames (syntax_tree);

    global.rt_args = args;

    global.syntax_tree = syntax_tree;

    DBUG_RETURN (syntax_tree);
}
