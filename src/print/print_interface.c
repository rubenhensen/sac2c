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
#define NO_SIMPLE_RETURN -1

/* local functions for internal use */
static int CountFunArgs (node *fundef);
static int CountFunRettypes (node *fundef);
static types *GetFunRettype (node *fundef, int pos);
static int GetReturnPos (node *fundef);
static char *TruncFunName (node *fundef);
static char *TruncArgName (char *argname);
static node *GetWrapper (node *mappingtree, node *fundef);
static node *BuildMappingTree (node *syntax_tree);
static void PrintWrapperSpecializationComment (node *fundef);
static void PrintWrapperPrototype (node *wrapper);
static void PrintWrapperHeaderFile (node *mappingtree);
static void PrintWrapperSwitch (node *specials);
static void PrintWrapperModuleFile (node *mappingtree);

/******************************************************************************
 *
 * function:
 *   node *PIHmodule(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses only in functions of module
 *
 ******************************************************************************/

node *
PIHmodule (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIHmodule");

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
 *
 *
 ******************************************************************************/

node *
PIHfundef (node *arg_node, node *arg_info)
{
    types *rettypes;
    int separator_needed;
    int i;

    DBUG_ENTER ("PIHfundef");

    if (FUNDEF_STATUS (arg_node)
        == ST_regular) { /* export only functions defined in this module */
        if ((FUNDEF_BODY (arg_node) == NULL)
            || ((NULL != FUNDEF_RETURN (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_RETURN (arg_node))))) {
            /* print wrapper-prototype to headerfile */
            /* comment header */
            fprintf (outfile, "/* function declaration for %s */\n",
                     TruncFunName (arg_node));

            fprintf (outfile, "extern ");

            /* simple return type */
            fprintf (outfile, "int ");

            /* function name */
            fprintf (outfile, "%s(", TruncFunName (arg_node));

            /* SAC-return-types */
            rettypes = FUNDEF_TYPES (arg_node);
            i = 0;
            separator_needed = FALSE;
            while (rettypes != NULL) {
                i++;
                fprintf (outfile, "%s **out%d", SACARGTYPE, i);
                rettypes = TYPES_NEXT (rettypes);
                if (rettypes != NULL)
                    fprintf (outfile, ", ");
                separator_needed = TRUE;
            }

            /* args */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                if (separator_needed)
                    fprintf (outfile, ", ");
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
            }

            /* EOL */
            fprintf (outfile, ");\n\n");
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) { /* traverse next function */
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ("PIHArg");

    fprintf (outfile, " %s *%s", SACARGTYPE, TruncArgName (ARG_NAME (arg_node)));

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",");
        arg_node = Trav (ARG_NEXT (arg_node), arg_info);
    }

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
    types *rettypes;
    node *params;
    int separator_needed;
    int i;
    char *typestring;

    DBUG_ENTER ("PIWfundef");

    if (FUNDEF_STATUS (arg_node)
        == ST_regular) { /* export only functions defined in this module */
        if ((FUNDEF_BODY (arg_node) == NULL)
            || ((NULL != FUNDEF_RETURN (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_RETURN (arg_node))))) {

            /*some debug comment*/
            NOTE (("working on function: %s\n", FUNDEF_NAME (arg_node)));

            /*first the returntypes*/
            rettypes = FUNDEF_TYPES (arg_node);
            i = 0;
            while (rettypes != NULL) {
                i++;

                NOTE (("  %d. return type: %s\n", i, Type2String (rettypes, 0 | 4)));
                rettypes = TYPES_NEXT (rettypes);
            }

            /*next the parameters*/
            params = FUNDEF_ARGS (arg_node);
            i = 0;
            while (params != NULL) {
                i++;
                NOTE (("  %d. arg type: %s %s\n", i,
                       Type2String (ARG_TYPE (params), 0 | 4), ARG_NAME (params)));
                params = ARG_NEXT (params);
            }

            NOTE (("\n"));

            /* print wrapper-prototype to headerfile */
            /* comment header */
            fprintf (outfile, "/* wrapper function for %s */\n", FUNDEF_NAME (arg_node));

            /* simple return type */
            fprintf (outfile, "int ");

            /* function name */
            fprintf (outfile, "%s(", TruncFunName (arg_node));

            /* SAC-return-types */
            rettypes = FUNDEF_TYPES (arg_node);
            i = 0;
            separator_needed = FALSE;
            while (rettypes != NULL) {
                i++;
                /*	  typestring=Type2CTypeString(rettypes,0); */
                fprintf (outfile, "%s*out%d", typestring, i);
                FREE (typestring);
                rettypes = TYPES_NEXT (rettypes);
                if (rettypes != NULL)
                    fprintf (outfile, ", ");
                separator_needed = TRUE;
            }

            /* args */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                if (separator_needed)
                    fprintf (outfile, ", ");
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
            }

            /* End Of Header */
            fprintf (outfile, "){\n");

            /* wrapper body */
            if ((NULL != FUNDEF_ICM (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_ICM (arg_node)))
                && (FUNDEF_STATUS (arg_node) != ST_spmdfun)) {
                Trav (FUNDEF_ICM (arg_node), arg_info); /* print N_icm ND_FUN_DEC */
            }

            /* end of wrapper */
            fprintf (outfile, "\n}\n\n");
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) { /* traverse next function */
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

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
    char *typestring;
    DBUG_ENTER ("PIWArg");

    /*  typestring=Type2CTypeString(ARG_TYPE(arg_node),0); */
    fprintf (outfile, " %s%s", typestring, TruncArgName (ARG_NAME (arg_node)));
    FREE (typestring);

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",");
        arg_node = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   int CountFunArgs( node *fundef)
 *
 * description:
 *   counts the number of arguments of this function
 *
 * return:
 *   number of args
 *
 ******************************************************************************/

static int
CountFunArgs (node *fundef)
{

    int count = 0;
    node *args;

    DBUG_ENTER ("CountFunArgs");

    args = FUNDEF_ARGS (fundef);

    while (args != NULL) {
        count++;
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * function:
 *   int CountFunRettypes( node *fundef)
 *
 * description:
 *   counts the number of returntypes of this function
 *
 * return:
 *   number of returntypes
 *
 ******************************************************************************/

static int
CountFunRettypes (node *fundef)
{
    int count = 0;
    types *rettypes;

    DBUG_ENTER ("CountFunRettypes");

    rettypes = FUNDEF_TYPES (fundef);

    while (rettypes != NULL) {
        count++;
        rettypes = TYPES_NEXT (rettypes);
    }

    DBUG_RETURN (count);
}

/******************************************************************************
 *
 * function:
 *   types *GetFunRettype(node *fundef, int pos)
 *
 * description:
 *   fetches the returntype at pos in return list of fundef
 *
 * return:
 *   returntype at pos
 *
 ******************************************************************************/

static types *
GetFunRettype (node *fundef, int pos)
{
    types *rettypes;
    types *result;
    int i;

    DBUG_ENTER ("GetFunRetttpe");

    rettypes = FUNDEF_TYPES (fundef);
    result = rettypes;

    for (i = 1; i < pos; i++) {
        result = rettypes;
        DBUG_ASSERT (result != NULL, ("Access on non existing function returntype!"));
        rettypes = TYPES_NEXT (rettypes);
    }

    DBUG_RETURN (result);
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
 *   char *TruncFunName(node *fundef)
 *
 * description:
 *   truncates the function name after some renaming by removing
 *   SAC-type postfixes.
 *
 * return:
 *   new allocated string
 *
 * remark:
 *   this implementations depends on the current fixed renaming shema
 *   SACf__modulename__functioname__typeinfos!
 *
 ******************************************************************************/

static char *
TruncFunName (node *fundef)
{
    int offset;
    char *funname;
    char *endpos;
    char *result;
    char *tmp_string;
    int count;

    DBUG_ENTER ("TruncFunName");

    funname = FUNDEF_NAME (fundef);
    tmp_string = (char *)MALLOC (sizeof (char) * strlen (funname));
    tmp_string[0] = '\0';

    strcat (tmp_string, "SAC_");     /* set standard Prefix */
    strcat (tmp_string, modulename); /* add modulename*/
    strcat (tmp_string, "_");

    /* behind the modulename in the current funname skip additional inserted "__" */
    offset = strlen (modulename) + strlen ("__");
    result = strstr (funname, modulename) + offset;
    endpos = strstr (result, "__");
    strncat (tmp_string, result, strlen (result) - strlen (endpos)); /* isolate funname */

    /* now add number of arguments */
    count = CountFunArgs (fundef);
    if (count > 0) {
        sprintf (tmp_string + strlen (tmp_string), "_%d", count);
    }

    /* last add the numer of returntypes */
    count = CountFunRettypes (fundef);
    if (count > 0) {
        sprintf (tmp_string + strlen (tmp_string), "_%d", count);
    }

    DBUG_RETURN (tmp_string);
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
 *   node *GetWrapper(node *mappingtree, node *fundef)
 *
 * description:
 *   searches in mappingtree for wrapper matching fundef
 *
 * return:
 *   pointer to wrapper node in mappingtree or NULL if no match
 *
 ******************************************************************************/

static node *
GetWrapper (node *mappingtree, node *fundef)
{
    node *wrapper;
    bool isfound;
    char *wrappername;

    DBUG_ENTER ("GetWrapper");

    wrappername = TruncFunName (fundef); /* get resulting wrappername*/
    wrapper = mappingtree;
    isfound = FALSE;

    while ((wrapper != NULL) && (!isfound)) {
        /*now search for existing wrapper */
        if (strcmp (wrappername, INFO_PIW_WRAPPERNAME (wrapper)) == 0) {
            isfound = TRUE;
        } else {
            wrapper = INFO_PIW_NEXT_WRAPPER (wrapper);
        }
    }

    FREE (wrappername);

    DBUG_RETURN (wrapper);
}

/******************************************************************************
 *
 * function:
 *   node *BuildMappingTree(node *syntaxtree)
 *
 * description:
 *   builds a simple tree of N_info nodes which maps all exportable functions
 *   to a wrapper function in an interface file.
 *
 * return:
 *   pointer root of mappingtree
 *
 * remark:
 *   at this time it searches for function with a fix shape and without user
 *   defined types.
 *
 ******************************************************************************/

static node *
BuildMappingTree (node *syntax_tree)
{
    node *mappingtree = NULL;
    node *funs;
    node *wrappernode;
    node *tempnode;

    DBUG_ENTER ("MapFunctionToWrapper");

    funs = MODUL_FUNS (syntax_tree);
    while (funs != NULL) {
        /* look at all fundefs in tree */
        if (FUNDEF_STATUS (funs) == ST_regular) {
            /* export only functions defined in this module */
            DBUG_ASSERT ((FUNDEF_BODY (funs) != NULL),
                         ("MapFunctionToWrapper: Found fundef ST_regular with empty "
                          "body!"));

            wrappernode = GetWrapper (mappingtree, funs);
            if (wrappernode == NULL) {
                /* new wrapper needed - setup info-node with data for new wrapper */
                wrappernode = MakeInfo ();
                INFO_PIW_FUNDEF (wrappernode) = funs;
                INFO_PIW_NEXT_FUNDEF (wrappernode) = NULL;
                INFO_PIW_NEXT_WRAPPER (wrappernode) = mappingtree;
                mappingtree = wrappernode;
                INFO_PIW_RETPOS (wrappernode) = GetReturnPos (funs);
                INFO_PIW_ARGCOUNT (wrappernode) = CountFunArgs (funs);
                INFO_PIW_RETCOUNT (wrappernode) = CountFunRettypes (funs);
                INFO_PIW_WRAPPERNAME (wrappernode) = TruncFunName (funs);
            } else {
                /* add function to wrapper - setup info-node with additional fundef*/

                while (INFO_PIW_NEXT_FUNDEF (wrappernode) != NULL) {
                    /* search end of fundef list of this wrapper */
                    wrappernode = INFO_PIW_NEXT_FUNDEF (wrappernode);
                }

                /* add new info node for this fundef */
                INFO_PIW_NEXT_FUNDEF (wrappernode) = MakeInfo ();
                tempnode = INFO_PIW_NEXT_FUNDEF (wrappernode);
                INFO_PIW_FUNDEF (tempnode) = funs;
                INFO_PIW_NEXT_FUNDEF (tempnode) = NULL;
                INFO_PIW_NEXT_WRAPPER (tempnode) = INFO_PIW_NEXT_WRAPPER (wrappernode);
                INFO_PIW_RETPOS (tempnode) = GetReturnPos (funs);
                INFO_PIW_ARGCOUNT (tempnode) = CountFunArgs (funs);
                INFO_PIW_RETCOUNT (tempnode) = CountFunRettypes (funs);
                INFO_PIW_WRAPPERNAME (tempnode) = INFO_PIW_WRAPPERNAME (wrappernode);
            }
        }
        funs = FUNDEF_NEXT (funs);
    }

    DBUG_RETURN (mappingtree);
}

/******************************************************************************
 *
 * function:
 *    void PrintWrapperSpecializationComment(node *fundef)
 *
 * description:
 *   Prints one commentline showing the accepted shapes and types
 *
 * return:
 *   nothing
 *
 ******************************************************************************/

static void
PrintWrapperSpecializationComment (node *fundef)
{
    char *typestring;
    node *args;
    types *rettypes;
    bool commaneeded = FALSE;
    int i;

    DBUG_ENTER ("PrintWrapperSpecializationComment");

    args = FUNDEF_ARGS (fundef);
    while (args != NULL) {
        /* print Name and Type/Shape of arguments */
        commaneeded = TRUE;
        typestring = Type2String (ARG_TYPE (args), 0);

        fprintf (outfile, "%s %s", typestring, TruncArgName (ARG_NAME (args)));
        FREE (typestring);

        if (ARG_NEXT (args) != NULL) {
            fprintf (outfile, ", ");
        }

        args = ARG_NEXT (args);
    }

    if (commaneeded) {
        fprintf (outfile, ", ");
    }

    fprintf (outfile, "returning: ");

    rettypes = FUNDEF_TYPES (fundef);
    if (rettypes != NULL) {
        /* print returntypes/shapes */
        i = 0;
        while (rettypes != NULL) {
            i++;
            typestring = Type2String (rettypes, 0 | 4);
            fprintf (outfile, "%s out%d", typestring, i);
            FREE (typestring);

            if (TYPES_NEXT (rettypes) != NULL) {
                fprintf (outfile, ", ");
            }
            rettypes = TYPES_NEXT (rettypes);
        }
    } else {
        fprintf (outfile, "nothing");
    }

    fprintf (outfile, "\n");
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintWrapperPrototype(node *wrapper)
 *
 * description:
 *   Prints one wrapper prototype with SAC_arg parameters and returntypes
 *
 * return:
 *   nothing
 *
 ******************************************************************************/

static void
PrintWrapperPrototype (node *wrapper)
{
    int i;
    int counter;
    types *rettypes;

    DBUG_ENTER ("PrintWrapperPrototype");

    /* print declaration */
    fprintf (outfile, "int %s(", INFO_PIW_WRAPPERNAME (wrapper));

    /* print return reference parameters */
    /* count return parameters */
    counter = 0;
    rettypes = FUNDEF_TYPES (INFO_PIW_FUNDEF (wrapper));
    while (rettypes != NULL) {
        counter++;
        rettypes = TYPES_NEXT (rettypes);
    }
    for (i = 1; i <= counter; i++) {
        fprintf (outfile, "SAC_arg *out%d", i);
        if (i < counter || INFO_PIW_ARGCOUNT (wrapper) > 0)
            fprintf (outfile, ", ");
    }

    /* print arguments */
    for (i = 1; i <= INFO_PIW_ARGCOUNT (wrapper); i++) {
        fprintf (outfile, "SAC_arg in%d", i);
        if (i < INFO_PIW_ARGCOUNT (wrapper)) {
            fprintf (outfile, ", ");
        }
    }

    /* print End of prototype */
    fprintf (outfile, ")");
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintWrapperHeaderFile(node *mappingtree)
 *
 * description:
 *   Prints the whole Interface header file for c library.
 *   gets needed wrapper from mapping tree and generates prototypes and comments
 *
 * return:
 *   nothing
 *
 ******************************************************************************/

static void
PrintWrapperHeaderFile (node *mappingtree)
{
    node *wrapper;
    node *specials;
    node *funs;

    DBUG_ENTER ("PrintWrapperHeaderFile");

    /* open <module>.h in tmpdir for writing wrapper header*/
    outfile = WriteOpen ("%s/%s.h", tmp_dirname, modulename);
    fprintf (outfile, "/* Interface SAC <-> C for %s \n", modulename);
    fprintf (outfile, " * use this %s.h file with lib%s.a */\n", modulename, modulename);
    fprintf (outfile, "#include \"SAC_interface.h\"\n");
    fprintf (outfile, "\n");

    /*Generate fixed manual and usage hints in headerfile*/
    /* to be implemented */

    /* traverse list of wrappers */
    wrapper = mappingtree;
    while (wrapper != NULL) {
        /* print general comment header for this wrapper */
        fprintf (outfile, "\n/* function\n *   %s\n * accepts arguments as follows:\n",
                 INFO_PIW_WRAPPERNAME (wrapper));

        /* print accepted shapes for each spezialized function */
        specials = wrapper;
        while (specials != NULL) {
            funs = INFO_PIW_FUNDEF (specials);
            fprintf (outfile, " * ");
            PrintWrapperSpecializationComment (funs);
            specials = INFO_PIW_NEXT_FUNDEF (specials);
        }
        fprintf (outfile, " */\n");
        fprintf (outfile, "extern ");
        PrintWrapperPrototype (wrapper);
        fprintf (outfile, ";\n\n");
        wrapper = INFO_PIW_NEXT_WRAPPER (wrapper);
    }

    fprintf (outfile,
             "/* generated headerfile, please do not modify function prototypes */\n");
    fclose (outfile);
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintWrapperSwitch(node *specials)
 *
 * description:
 *   Prints part of the wrapper code, which matches the correct specialzed function
 *
 * return:
 *   nothing
 *
 ******************************************************************************/

static void
PrintWrapperSwitch (node *specials)
{
    int i;
    int j;
    types *argtype;
    node *args;

    DBUG_ENTER ("PrintWrapperSwitch");

    /* print local vars with initializers for shapevectors */
    args = FUNDEF_ARGS (INFO_PIW_FUNDEF (specials));
    i = 0;

    while (args != NULL) {
        argtype = ARG_TYPE (args);

        if (TYPES_DIM (argtype) < 0) {
            SYSERROR (("Unknown shapes cannot be exported!\n"));
        }

        if (TYPES_DIM (argtype) == 0) {
            /* no array-type*/
            fprintf (outfile, "int *SAC_local_argshp%d = NULL;\n", i);
        } else {
            /* arraytype with fixed shape */
            fprintf (outfile, "int SAC_local_argshp%d[] = {", i);
            for (j = 0; j < TYPES_DIM (argtype); j++) {
                fprintf (outfile, "%d", TYPES_SHAPE (argtype, j));
                if (j < TYPES_DIM (argtype) - 1) {
                    fprintf (outfile, ", ");
                }
            }
            fprintf (outfile, "};\n");
        }
        i++;
        args = ARG_NEXT (args);
    }

    fprintf (outfile, "if(");

    /* print makros for shape and type checks for all args */
    args = FUNDEF_ARGS (INFO_PIW_FUNDEF (specials));
    i = 0;

    while (args != NULL) {
        argtype = ARG_TYPE (args);

        /*                           var   {x,y} argdim basetype */
        fprintf (outfile, "PIW_CHECK( in%d , SAC_local_argshp%d , %d, %d )", i, i,
                 TYPES_DIM (argtype), TYPES_BASETYPE (argtype));

        if (i < INFO_PIW_ARGCOUNT (specials)) {
            fprintf (outfile, " && ");
        }
        i++;
        args = ARG_NEXT (args);
    }

    fprintf (outfile, ")\n{\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintWrapperModuleFile(node *mappingtree)
 *
 * description:
 *   prints interface wrapper functions to temporary file uses in compilation of
 *   c library.
 *   does implement code fpr runtime checking for shapes,refcounter, argument
 *   type handling and return
 *
 * return:
 *   nothing
 *****************************************************************************/

static void
PrintWrapperModuleFile (node *mappingtree)
{
    node *wrapper;
    node *specials;
    types *rettype;
    int i;
    int j;

    DBUG_ENTER ("PrintWrapperModuleFile");

    /* open <module>_wrapper.c in tmpdir for writing*/
    outfile = WriteOpen ("%s/%s_wrapper.c", tmp_dirname, modulename);
    fprintf (outfile, "/* Interface SAC <-> C for %s\n", modulename);
    fprintf (outfile,
             " * this file is only used when compiling the c-library lib%s.a */\n",
             modulename);
    fprintf (outfile, "#include <stdio.h>\n");
    fprintf (outfile, "#include \"SAC_interface.h\"\n");
    fprintf (outfile, "#include \"SAC_arg.h\"\n");
    fprintf (outfile, "#include \"SAC_interface_makrodefs.h\"\n");
    fprintf (outfile, "\n");

    /* general preload for codefile */
    /* to be implemented */
    fprintf (outfile, "/* <insert some useful things here...> */\n");
    wrapper = mappingtree;
    while (wrapper != NULL) {
        /* print general comment header for this wrapper */
        PrintWrapperPrototype (wrapper);
        fprintf (outfile, "\n{\n");

        /* print checks for refcounts */
        fprintf (outfile, "/* refcount checks */\n");
        for (i = 1; i <= INFO_PIW_ARGCOUNT (wrapper); i++) {
            fprintf (outfile, "SAC_IW_CHECK_RC( in%d );\n", i);
        }

        /* print case switch for specialized functions */
        fprintf (outfile, "/* case switch for specialized functions */\n");
        specials = wrapper;
        while (specials != NULL) {
            fprintf (outfile, "{/* function: %s */\n",
                     FUNDEF_NAME (INFO_PIW_FUNDEF (specials)));
            PrintWrapperSwitch (specials);

            /* print code creating all return SAC_args */
            fprintf (outfile, "  /* create return type structs */\n");
            for (i = 1; i <= INFO_PIW_RETCOUNT (specials); i++) {
                rettype = GetFunRettype (INFO_PIW_FUNDEF (specials), i);
                fprintf (outfile, "  *out%d=SAC_CreateSACArg(%d, %d", i,
                         TYPES_BASETYPE (rettype), TYPES_DIM (rettype));

                /* write shape data*/
                for (j = 0; j < TYPES_DIM (rettype); j++) {
                    fprintf (outfile, "%d", TYPES_SHAPE (rettype, j));
                    if (j < (TYPES_DIM (rettype)) - 1) {
                        fprintf (outfile, ", ");
                    }
                }
                fprintf (outfile, ");\n");
            }

            /* print makros creating function call to specialized function */
            fprintf (outfile, "  /* <call fun()> */\n");

            /* print makros for dec local refcounters and maybe free SAC_arg */
            fprintf (outfile, "  /* <dec & free local refcounts> */\n");

            /* print code  successful return from call */
            fprintf (outfile, "  return(0); /*call successful */\n");

            /* print code end of switch */
            fprintf (outfile, "}\n}\n");
            specials = INFO_PIW_NEXT_FUNDEF (specials);
        }

        /* no speacialized function found matching the args -> error */
        fprintf (outfile,
                 "fprintf(stderr, \"ERROR - no matching specialized function!\\n\");\n");
        fprintf (outfile, "return(1); /* error - no matching specialized function */\n");
        fprintf (outfile, "\n}\n\n");
        wrapper = INFO_PIW_NEXT_WRAPPER (wrapper);
    }

    fprintf (outfile, "/* generated codefile, please do not modify */\n");
    fclose (outfile);
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *PrintInterface( node *syntax_tree)
 *
 * description:
 *   Prints the whole Module-Interface files for c library.
 *   generates <modulename>.h as headerfile with wrapper defs and datatypes
 *             <modulename>_wrapper.c implementing the wrapper functions
 *
 *
 ******************************************************************************/

node *
PrintInterface (node *syntax_tree)
{
    node *mappingtree;

    DBUG_ENTER ("PrintInterface");
    NOTE (("Generating c library interface files\n"));

    /* map specialized functions to needed wrapper functions */
    mappingtree = BuildMappingTree (syntax_tree);

    if (mappingtree != NULL) {
        /* there are some functions to export */

        PrintWrapperHeaderFile (mappingtree);
        PrintWrapperModuleFile (mappingtree);

        /* print all wrapper functions */
        /* do be implemented */

        /* resolving ICMs */
        /* to be implemented */

        /* Systemcall to resolve ICM Makros in a file using the cpp */
        /* SystemCall("%s -E -H %s -o %s/%s.h %s/%s.h",
           config.cc, config.ccdir, tmp_dirname, modulename, tmp_dirname, modulename); */
    }

    DBUG_RETURN (syntax_tree);
}

/*

sac2c -genlib c TestModule.sac

gcc -I ./Interface/ -I ~/sac/sac2c/src/typecheck/ -I ~/sac/sac2c/src/global/ -I
~/sac/sac2c/src/tree/ -c -o TestModule_wrapper.o TestModule_wrapper.c
*/
