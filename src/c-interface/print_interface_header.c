/*
 * $Log$
 * Revision 3.1  2000/11/20 18:03:43  sacbase
 * new release made
 *
 * Revision 1.1  2000/08/02 14:24:07  nmw
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "print_interface_header.h"
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
#include "gen_startup_code.h"

/* interface identifier */
#define SACARGTYPE "SAC_arg"
#define SACPREFIX "SAC_"

#define PIH_PRINT_COMMENT 1
#define PIH_PRINT_PROTOTYPE 2

/* function for only local usage */
static types *TravTH (types *arg_type, node *arg_info);
static types *PIHtypes (types *arg_type, node *arg_info);
static strings *PrintDepEntry (deps *depends, statustype stat, strings *done);

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
    strings *liblist[2];
    strings *item;
    int i;
    char *libsac_string;

    DBUG_ENTER ("PIHmodul");

    old_outfile = outfile; /* save, might be in use */

    NOTE (("Writing c-library headerfile \"%s%s.h\"", targetdir, MODUL_NAME (arg_node)));

    /* open <module>.h in tmpdir for writing wrapper header*/
    outfile = WriteOpen ("%s/%s.h", targetdir, MODUL_NAME (arg_node));
    fprintf (outfile, "/* Interface SAC <-> C for %s \n", MODUL_NAME (arg_node));
    fprintf (outfile, " * use this %s.h file with lib%s.a\n", MODUL_NAME (arg_node),
             MODUL_NAME (arg_node));

    /*Generate list of modules to link with */
    fprintf (outfile,
             " * For compiling with this module you need to set up\n"
             " * your c compiler with searchpaths to:\n"
             " * -L$SACBASE/runtime (for sac runtime-system)\n"
             " * -L.                (or where you place 'lib%s.a')\n"
             " *\n"
             " * -I$SACBASE/runtime (for include of 'sac_cinterface.h')\n"
             " *\n"
             " * when compiling your code add the following files to link with:\n"
             " * -l%s\n",
             MODUL_NAME (arg_node), MODUL_NAME (arg_node));

    liblist[0] = PrintDepEntry (dependencies, ST_external, NULL);
    liblist[1] = PrintDepEntry (dependencies, ST_system, NULL);

    /* determine the needed files of the sac-runtime system */
    if ((gen_mt_code == GEN_MT_OLD) || (gen_mt_code == GEN_MT_NEW)) { /* MT */
        if (optimize & OPT_PHM) {                                     /* PHM */
            if (runtimecheck & RUNTIMECHECK_HEAP) {                   /* diag */
                fprintf (outfile, " * -lsac_heapmgr_mt_diag\n"
                                  " * -lsac_mt\n"
                                  " * -lpthread\n");
                libsac_string = "-lsac_heapmgr_mt_diag -lsac_mt -lpthread";
            } else { /* no diag */
                fprintf (outfile, " * -lsac_heapmgr_mt\n"
                                  " * -lsac_mt\n"
                                  " * -lpthread\n");
                libsac_string = "-lsac_heapmgr_mt -lsac_mt -lpthread";
            }
        } else { /* no PHM */
            fprintf (outfile, " * -lsac_mt\n"
                              " * -lpthread\n");
            libsac_string = "-lsac_mt -lpthread";
        }
    } else {                      /* no MT */
        if (optimize & OPT_PHM) { /* diag */
            if (runtimecheck & RUNTIMECHECK_HEAP) {
                fprintf (outfile, " * -lsac_heapmgr_diag\n"
                                  " * -lsac\n");
                libsac_string = "-lsac_heapmgr_diag -lsac";
            } else { /* no diag */
                fprintf (outfile, " * -lsac_heapmgr\n"
                                  " * -lsac\n");
                libsac_string = "-lsac_heapmgr -lsac";
            }
        } else { /* no PHM */
            fprintf (outfile, " * -lsac\n");
            libsac_string = "-lsac";
        }
    }

    /* add a simple copy&past linkline */
    fprintf (outfile,
             " *\n"
             " * simply copy&past the following line to your makefile:\n"
             " * -L$SACBASE/runtime -L. -I$SACBASE/runtime -l%s",
             MODUL_NAME (arg_node));

    for (i = 0; i <= 1; i++) {
        item = liblist[i];
        while (item != NULL) {
            fprintf (outfile, " %s", STRINGS_STRING (item));
            item = STRINGS_NEXT (item);
        }
        liblist[i] = FreeAllStrings (liblist[i]);
    }

    fprintf (outfile, " %s\n", libsac_string);
    fprintf (outfile, " *\n */\n\n");

    fprintf (outfile, "#include \"sac_cinterface.h\"\n");

    if (MODUL_CWRAPPER (arg_node) != NULL) {
        /* traverse list of wrappers */
        MODUL_CWRAPPER (arg_node) = Trav (MODUL_CWRAPPER (arg_node), arg_info);
    }

    fprintf (outfile, "\n/* generated headerfile, please do not modify"
                      " function prototypes */\n");
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
    fprintf (outfile,
             "\n"
             "/* function %s\n"
             " * defined in module %s\n"
             " * accepts arguments as follows:\n",
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
        INFO_PIH_COUNTER (arg_info) = 0;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        } else {
            fprintf (outfile, " () ");
        }

        fprintf (outfile, " -> ");

        INFO_PIH_COUNTER (arg_info) = 0;
        /* then print resulting types */
        if (FUNDEF_TYPES (arg_node) != NULL) {
            FUNDEF_TYPES (arg_node) = TravTH (FUNDEF_TYPES (arg_node), arg_info);
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

    switch (INFO_PIH_FLAG (arg_info)) {
    case PIH_PRINT_COMMENT:
        /* print internal accepted types of argument */
        typestring = Type2String (ARG_TYPE (arg_node), 0);

        fprintf (outfile, "%s %s", typestring, ARG_NAME (arg_node));
        FREE (typestring);
        break;

    default:
        SYSERROR (("undefined case in PIWtypes!\n"));
    }

    /* traverse to next arg */
    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ", ");
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
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

static types *
PIHtypes (types *arg_type, node *arg_info)
{
    char *typestring;

    DBUG_ENTER ("PIHtypes");

    INFO_PIH_COUNTER (arg_info) = INFO_PIH_COUNTER (arg_info) + 1;

    switch (INFO_PIH_FLAG (arg_info)) {
    case PIH_PRINT_COMMENT:
        if (TYPES_BASETYPE (arg_type) == T_void) {
            /* if void, do not print varname */
            fprintf (outfile, "void");
        } else {
            typestring = Type2String (arg_type, 0 | 4);

            fprintf (outfile, "%s out%d", typestring, INFO_PIH_COUNTER (arg_info));
            FREE (typestring);
        }

        if (TYPES_NEXT (arg_type) != NULL) {
            fprintf (outfile, ", ");
        }
        break;

    default:
        SYSERROR (("undefined case in PIWtypes!\n"));
    }

    /* traverse to next returntype */
    if (TYPES_NEXT (arg_type) != NULL) {
        TYPES_NEXT (arg_type) = TravTH (TYPES_NEXT (arg_type), arg_info);
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

node *
PIHcwrapperPrototype (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("PIHcwrapperPrototype");

    /* print declaration */
    fprintf (outfile, "int %s%s_%s_%d_%d(", SACPREFIX, CWRAPPER_MOD (arg_node),
             CWRAPPER_NAME (arg_node), CWRAPPER_RESCOUNT (arg_node),
             CWRAPPER_ARGCOUNT (arg_node));

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
 *   types *TravTH(types *arg_type, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for PIH
 *
 *
 ******************************************************************************/

static types *
TravTH (types *arg_type, node *arg_info)
{
    DBUG_ENTER ("TravTH");

    DBUG_ASSERT (arg_type != NULL, "TravTH: traversal in NULL type\n");
    arg_type = PIHtypes (arg_type, arg_info);

    DBUG_RETURN (arg_type);
}

/******************************************************************************
 *
 * function:
 *   strings *PrintDepEntry(deps *depends, statustype stat, strings *done)
 *
 * description:
 *   prints the dependencies to library files in the header comment.
 *
 ******************************************************************************/

static strings *
PrintDepEntry (deps *depends, statustype stat, strings *done)
{
    strings *tmp_done;
    deps *tmp;

    DBUG_ENTER ("PrintDepEntry");

    tmp = depends;

    while (tmp != NULL) {
        if (DEPS_STATUS (tmp) == stat) {
            tmp_done = done;

            while ((tmp_done != NULL)
                   && (0 != strcmp (DEPS_LIBNAME (tmp), STRINGS_STRING (tmp_done)))) {
                tmp_done = STRINGS_NEXT (tmp_done);
            }
            if (tmp_done == NULL) {
                done = MakeStrings (DEPS_LIBNAME (tmp), done);
                fprintf (outfile, " * %s\n", DEPS_LIBNAME (tmp));
            }
        }
        tmp = DEPS_NEXT (tmp);
    }

    tmp = depends;
    while (tmp != NULL) {
        if (DEPS_SUB (tmp) != NULL) {
            done = PrintDepEntry (DEPS_SUB (tmp), stat, done);
        }
        tmp = DEPS_NEXT (tmp);
    }

    DBUG_RETURN (done);
}
