#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "print_interface.h"
#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "filemgr.h"
#include "globals.h"
#include "free.h"
#include "resource.h"
#include "shape.h"

/* interface identifier */
#define SACARGTYPE "SAC_arg"
#define SACPREFIX "SAC_"
#define NO_SIMPLE_RETURN -1
#define PIH_PRINT_COMMENT 1
#define PIH_PRINT_PROTOTYPE 2

/* function for only local usage */
static types *TravT (types *type, node *arg_info);
static node *PIHcwrapperPrototype (node *wrapper, node *arg_info);

/******************************************************************************
 *
 * function:
 *   node *PIHmodule(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses only in wrappers of module, generate c headerfile for module
 *
 ******************************************************************************/

node *
PIHmodul (node *arg_node, node *arg_info)
{
    FILE *old_outfile;

    DBUG_ENTER ("PIHmodul");

    old_outfile = outfile; /* save, might be in use */

    /* open <module>.h in tmpdir for writing wrapper header*/
    outfile = WriteOpen ("%s/%s.h", tmp_dirname, MODUL_NAME (arg_node));
    fprintf (outfile, "/* Interface SAC <-> C for %s \n", MODUL_NAME (arg_node));
    fprintf (outfile, " * use this %s.h file with lib%s.a */\n", MODUL_NAME (arg_node),
             MODUL_NAME (arg_node));
    fprintf (outfile, "#include \"SAC_interface.h\"\n");
    fprintf (outfile, "\n");

    /*Generate fixed manual and usage hints in headerfile*/
    /* to be implemented */

    if (MODUL_CWRAPPER (arg_node) != NULL) {
        /* traverse list of wrappers */
        MODUL_CWRAPPER (arg_node) = Trav (MODUL_CWRAPPER (arg_node), arg_info);
    }

    fprintf (outfile,
             "\n/* generated headerfile, please do not modify function prototypes */\n");
    fclose (outfile);

    outfile = old_outfile; /* restore old filehandle */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIHcwrapper(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses and print cwrapper interface header and protoypes
 *
 ******************************************************************************/

node *
PIHcwrapper (node *arg_node, node *arg_info)
{
    nodelist *funlist;

    DBUG_ENTER ("PIHcwrapper");

    /* print general comment header for this wrapper */
    fprintf (outfile, "\n/* function\n *   %s in %s\n * accepts arguments as follows:\n",
             CWRAPPER_NAME (arg_node), CWRAPPER_MOD (arg_node));

    /* print accepted shapes for each spezialized function */
    funlist = CWRAPPER_FUNS (arg_node);
    DBUG_ASSERT (funlist != NULL, ("PIHcwrapper: wrapper node without fundef\n"));

    while (funlist != NULL) {
        /* go for all fundefs in nodelist */
        INFO_PIH_FLAG (arg_info) = PIH_PRINT_COMMENT;

        NODELIST_NODE (funlist) = Trav (NODELIST_NODE (funlist), arg_info);

        funlist = NODELIST_NEXT (funlist);
    }

    fprintf (outfile, " */\n");
    fprintf (outfile, "extern ");
    arg_node = PIHcwrapperPrototype (arg_node, arg_info);
    fprintf (outfile, ";\n\n");

    if (CWRAPPER_NEXT (arg_node) != NULL) {
        CWRAPPER_NEXT (arg_node) = Trav (CWRAPPER_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIHfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints Interface Header for Wrapper-function c->SAC
 *   also prints the return types
 * flag PIH_PRINT_COMMENT: prints comment line for specialized fundef
 *
 *
 ******************************************************************************/

node *
PIHfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIHfundef");

    if (INFO_PIH_FLAG (arg_info) == PIH_PRINT_COMMENT) {
        /* print comment line in headerfile */
        fprintf (outfile, " * ");

        /* first print accepted arguments */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        } else {
            fprintf (outfile, "void");
        }

        fprintf (outfile, " -> ");

        /* then print resulting types */
        if (FUNDEF_TYPES (arg_node) != NULL) {
            FUNDEF_TYPES (arg_node) = TravT (FUNDEF_TYPES (arg_node), arg_info);
        } else {
            fprintf (outfile, "void");
        }
        fprintf (outfile, "\n");
    } else {
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIHarg(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints function arguments for Interface Header for Wrapper-function c->SAC
 *
 *
 ******************************************************************************/

node *
PIHarg (node *arg_node, node *arg_info)
{
    char *typestring;

    DBUG_ENTER ("PIHArg");

    if (INFO_PIH_FLAG (arg_info) == PIH_PRINT_COMMENT) {
        /* print internal accepted types of argument */

        typestring = Type2String (ARG_TYPE (arg_node), 0);

        fprintf (outfile, "%s %s", typestring, ARG_NAME (arg_node));
        FREE (typestring);

        if (ARG_NEXT (arg_node) != NULL) {
            fprintf (outfile, ", ");
            ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   types *PIHtypes(types *arg_type, node *arg_info)
 *
 * description:
 *   Prints results of functions
 *
 * remark: simulation of the syntax of the Trav technology
 *
 ******************************************************************************/

types *
PIHtypes (types *arg_type, node *arg_info)
{
    char *typestring;
    DBUG_ENTER ("PIHtypes");

    if (INFO_PIH_FLAG (arg_info) == PIH_PRINT_COMMENT) {
        /* print resulting types */

        typestring = Type2String (arg_type, 0);

        fprintf (outfile, "%s", typestring);
        FREE (typestring);

        if (TYPES_NEXT (arg_type) != NULL) {
            fprintf (outfile, ", ");
            TYPES_NEXT (arg_type) = TravT (TYPES_NEXT (arg_type), arg_info);
        }
    }
    DBUG_RETURN (arg_type);
}

/******************************************************************************
 *
 * function:
 *   node *PIHcwrapperPrototype(node *wrapper, node *arg_info)
 *
 * description:
 *   Prints one cwrapper prototype with SAC_arg parameters and returntypes
 *
 ******************************************************************************/

static node *
PIHcwrapperPrototype (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("PIHcwrapperPrototype");

    /* print declaration */
    fprintf (outfile, "int %s%s_%s(", SACPREFIX, CWRAPPER_MOD (arg_node),
             CWRAPPER_NAME (arg_node));

    /* print return reference parameters */
    for (i = 1; i <= CWRAPPER_RESCOUNT (arg_node); i++) {
        fprintf (outfile, "%s *out%d", SACARGTYPE, i);
        if (i < CWRAPPER_RESCOUNT (arg_info) || CWRAPPER_ARGCOUNT (arg_node) > 0) {
            fprintf (outfile, ", ");
        }
    }

    /* print arguments */
    for (i = 1; i <= CWRAPPER_ARGCOUNT (arg_node); i++) {
        fprintf (outfile, "%s in%d", SACARGTYPE, i);
        if (i < CWRAPPER_ARGCOUNT (arg_node)) {
            fprintf (outfile, ", ");
        }
    }

    /* print End of prototype */
    fprintf (outfile, ")");
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWmodul(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses only in functions of module
 *
 ******************************************************************************/

node *
PIWmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIWmodul");

    NOTE (("PIWmodul reached...\n"));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWcwrapper(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses and print cwrapper interface implementation
 *
 ******************************************************************************/

node *
PIWcwrapper (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIWcwrapper");

    NOTE (("PIWcwrapper reached...\n"));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints Wrapper-function c->SAC
 *   with argument checks, return values, refcounter handling
 *
 *
 ******************************************************************************/

node *
PIWfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIWfundef");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWarg(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints function arguments for use in Wrapper-function c->SAC
 *
 *
 ******************************************************************************/
node *
PIWarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIWArg");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   int GetReturnPos( node *fundef)
 *
 * description:
 *   Looks for the one returntype that can be passed by the function
 *   return value. This must be a simple type without refcounting.
 *
 * return:
 *   the position (1 to n) in the functions return list or NO_SIMPLE_RETURN
 *
 * remark:
 *   this implemenatation depends on compile.h!
 *   should be changed to a flag based variant
 *
 ******************************************************************************/

static int
GetReturnPos (node *fundef)
{
    int count = 0;
    types *rettypes;

    DBUG_ENTER ("GetReturnPos");

    rettypes = FUNDEF_TYPES (fundef);

    while (rettypes != NULL) {
        count++;
        if (TYPES_DIM (rettypes) == 0) {
            /* found a simple type */
            DBUG_RETURN (count);
        }
        rettypes = TYPES_NEXT (rettypes);
    }

    /* no simple return type found */
    DBUG_RETURN (NO_SIMPLE_RETURN);
}

/******************************************************************************
 *
 * function:
 *   char *TruncArgName( char *argname)
 *
 * description:
 *   truncates the actual argument name after some renaming by removing
 *   SAC-prefix. returns the offset-pointer in the funname string
 *
 * return:
 *   Pointer to string in old string, NOT a new copy of that string!
 *
 * remark:
 *   this implementation depends on the current renaming shama
 *   SACl_argname
 *
 ******************************************************************************/

static char *
TruncArgName (char *argname)
{
    int offset;
    char *result;

    DBUG_ENTER ("truncArgName");

    /* skip additional inserted "SACl_" */
    offset = strlen ("SACl_");

    result = argname + offset;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *PrintInterface( node *syntax_tree)
 *
 * description:
 *   Prints the whole Module-Interface files for c library.
 *
 *
 ******************************************************************************/

node *
PrintInterface (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("PrintInterface");
    NOTE (("Generating c library interface files\n"));

    arg_info = MakeInfo ();
    old_tab = act_tab;

    /* do PIH traversal */
    act_tab = pih_tab;
    syntax_tree = Trav (syntax_tree, arg_info);

    /* do PIW traversal */
    act_tab = piw_tab;
    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;
    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   types *TravT(types *arg_type, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *
 *
 ******************************************************************************/

static types *
TravT (types *arg_type, node *arg_info)
{
    DBUG_ENTER ("TravT");

    DBUG_ASSERT (arg_type != NULL, "TravT: traversal in NULL type\n");
    arg_type = PIHtypes (arg_type, arg_info);

    DBUG_RETURN (arg_type);
}

/*

sac2c -genlib c TestModule.sac

gcc -I ./Interface/ -I ~/sac/sac2c/src/typecheck/ -I ~/sac/sac2c/src/global/ -I
~/sac/sac2c/src/tree/ -c -o TestModule_wrapper.o TestModule_wrapper.c
*/
