/*
 *
 * $Log$
 * Revision 3.7  2001/05/17 12:52:48  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 3.6  2001/03/22 18:03:00  dkr
 * tree.h no longer included
 *
 * Revision 3.5  2001/02/13 12:58:38  dkr
 * AddSpecializedFundef: access macros used
 *
 * Revision 3.4  2000/11/29 16:21:59  nmw
 * handling of mixed variable identifiers in specialization
 * file for one generic fundef added
 *
 * Revision 3.3  2000/11/23 16:19:14  nmw
 * function RemoveGenericTemplates() removed
 *
 * Revision 3.2  2000/11/22 16:25:14  nmw
 * when specializing generic functions the generic function
 * itself will no longer be removed
 *
 * Revision 3.1  2000/11/20 18:03:39  sacbase
 * new release made
 *
 * Revision 1.8  2000/11/16 13:49:00  nmw
 * removal of generic functions after specialization reenabled
 *
 * Revision 1.7  2000/10/31 18:19:13  cg
 * New fundef nodes introduced via specialization file are tagged
 * ST_exported to prevent them from subsequent elimination by
 * dead function removal.
 *
 * Revision 1.6  2000/10/31 15:07:41  sbs
 * ST_gen_remove not set anymore --- I'm not sure what implications
 * this change has therefore it's marked SBS
 *
 * Revision 1.5  2000/08/03 10:23:06  nmw
 * removal of generic fundefs after specialization added
 *
 * Revision 1.4  2000/07/28 14:45:56  nmw
 * minor bugfixes
 *
 * Revision 1.3  2000/07/24 14:59:25  nmw
 * analysing and generating of function specializations implemented
 *
 * Revision 1.2  2000/07/21 15:13:06  nmw
 * specfile parsing integrated
 *
 * Revision 1.1  2000/07/21 08:18:37  nmw
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "filemgr.h"
#include "globals.h"
#include "free.h"
#include "DupTree.h"
#include "resource.h"
#include "scnprs.h"
#include "import_specialization.h"

/* datatype for local usage */
typedef enum { SPECTYPE_NO, SPECTYPE_YES, SPECTYPE_EQUAL } spectype;

/* functions for local usage */
static node *ScanParseSpecializationFile (char *modname);
static node *MapSpecialized2Generic (node *spec_fundef, node *arg_info);
static node *AddSpecializedFundef (node *fundefs, node *spec_fundef, node *gen_fundef);
static bool isSpecialization (char *spec_name, node *spec_args, types *spec_types,
                              char *gen_name, node *gen_args, types *gen_types);
static spectype isSpecializationType (types *spec_type, types *gen_type);

/******************************************************************************
 *
 * function:
 *   node *IMPSPECmodspec(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal of specialized fundefs declared in .spec file
 *
 *
 ******************************************************************************/

node *
IMPSPECmodspec (node *arg_node, node *arg_info)
{
    node *fundefs;

    DBUG_ENTER ("IMPSPECmodspec");

    if (MODSPEC_OWN (arg_node)) {
        fundefs = EXPLIST_FUNS (MODSPEC_OWN (arg_node));
        if (fundefs) {
            EXPLIST_FUNS (MODSPEC_OWN (arg_node)) = Trav (fundefs, arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *IMPSPECfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses all specialized fundefs an looks for a matching generic fundef
 *
 *
 ******************************************************************************/

node *
IMPSPECfundef (node *arg_node, node *arg_info)
{
    node *generic_fundef;

    DBUG_ENTER ("IMPSPECfundef");

    generic_fundef = MapSpecialized2Generic (arg_node, arg_info);

    /*
     * this generic fundef is not needed anymore after specialization
     * because generic functions are not exported when using c libraries.
     * to avoid errors when parsing the modules declaration file these
     * fundefs are marked in the typechecker to be ignored in the
     * further compiling steps.
     */

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *MapSpecialized2Generic( node *spec_fundef , node *arg_info )
 *
 * description:
 *   traverses all fundefs in module and looks for a matching generic fundef
 *   returns fundef of matching generic fundef or NULL if no match
 *
 ******************************************************************************/

static node *
MapSpecialized2Generic (node *spec_fundef, node *arg_info)
{
    node *gen_fundef;
    node *fundef;

    DBUG_ENTER ("MapSpecialized2Generic");

    gen_fundef = NULL;
    fundef = MODUL_FUNS (INFO_IMPSPEC_MODUL (arg_info));
    while (fundef) {
        if (isSpecialization (FUNDEF_NAME (spec_fundef), FUNDEF_ARGS (spec_fundef),
                              FUNDEF_TYPES (spec_fundef), FUNDEF_NAME (fundef),
                              FUNDEF_ARGS (fundef), FUNDEF_TYPES (fundef))) {
            MODUL_FUNS (INFO_IMPSPEC_MODUL (arg_info))
              = AddSpecializedFundef (MODUL_FUNS (INFO_IMPSPEC_MODUL (arg_info)),
                                      spec_fundef, fundef);
            gen_fundef = fundef;
            fundef = NULL; /* break */
        } else {
            fundef = FUNDEF_NEXT (fundef); /* traverse to next */
        }
    }

    DBUG_RETURN (gen_fundef);
}

/******************************************************************************
 *
 * function:
 *   bool isSpecialization(char *spec_name, node *spec_args, types *spec_types,
 *                         char gen_name,   node *gen_args,  types *gen_types)
 *
 * description:
 *   checks args and types of to fundefs, if first is specialization of a generic
 *   second one
 *   checks: fundef name, arguments and results
 ******************************************************************************/

static bool
isSpecialization (char *spec_name, node *spec_args, types *spec_types, char *gen_name,
                  node *gen_args, types *gen_types)
{
    bool result_flag;
    spectype spec_result;
    spectype check_args;
    node *s_arg;
    node *g_arg;
    types *s_type;
    types *g_type;

    DBUG_ENTER ("IsSpecialization");
    result_flag = TRUE;

    /* check name */
    if (strcmp (spec_name, gen_name) != 0)
        result_flag = FALSE;

    /* check args */
    if (result_flag) {
        s_arg = spec_args;
        g_arg = gen_args;
        check_args = SPECTYPE_EQUAL;

        while ((s_arg != NULL) && (g_arg != NULL)) {
            /* compare types */
            s_type = ARG_TYPE (s_arg);
            g_type = ARG_TYPE (g_arg);
            DBUG_ASSERT (s_type != NULL, "arg without type");
            DBUG_ASSERT (g_type != NULL, "arg without type");

            spec_result = isSpecializationType (s_type, g_type);
            if (spec_result == SPECTYPE_NO) {
                check_args = SPECTYPE_NO; /* this arg does not match */
            } else if ((spec_result == SPECTYPE_YES) && (check_args != SPECTYPE_NO)) {
                check_args = SPECTYPE_YES;
            }

            s_arg = ARG_NEXT (s_arg);
            g_arg = ARG_NEXT (g_arg);
        }
        result_flag = result_flag && (check_args == SPECTYPE_YES);

        /* after the while loop both pointer have to be NULL */
        if ((s_arg != NULL) || (g_arg != NULL)) {
            /* number of args are different */
            result_flag = FALSE;
        }
    }

    /* check result types */
    if (result_flag) {
        s_type = spec_types;
        g_type = gen_types;

        while ((s_type != NULL) && (g_type != NULL)) {
            /* compare types */
            result_flag
              = result_flag && (isSpecializationType (s_type, g_type) != SPECTYPE_NO);

            s_type = TYPES_NEXT (s_type);
            g_type = TYPES_NEXT (g_type);
        }

        /* after the while loop both pointer have to be NULL */
        if ((s_type != NULL) || (g_type != NULL)) {
            /* number of result are different */
            result_flag = FALSE;
        }
    }

    DBUG_RETURN (result_flag);
}

/******************************************************************************
 *
 * function:
 *   spectype isSpecializationType(types *spec_type, types *gen_type)
 *
 * description:
 *   checks: simpletype, relation of dimension and relation of specification in
 *           shape, e.g. []<[.,.,.]<[1,2,3]
 *
 * result:
 *   SPECTYPE_NO    spec_type is no specialization of gen_type
 *   SPECTYPE_YES   spec_type is specialization of gen_type
 *   SPECTYPE_EQUAL spec_type is equal to gen_type
 ******************************************************************************/

static spectype
isSpecializationType (types *s_type, types *g_type)
{
    int i;
    spectype spec_flag;

    DBUG_ENTER ("isSpecializationType");

    spec_flag = SPECTYPE_YES;

    /* check basetypes */
    if (TYPES_BASETYPE (s_type) != TYPES_BASETYPE (g_type)) {
        spec_flag = SPECTYPE_NO;
    }

    /* check dimensions */
    else if ((TYPES_DIM (g_type) >= 0) && (TYPES_DIM (g_type) != TYPES_DIM (s_type))) {
        /* different known dimensions */
        spec_flag = SPECTYPE_NO;
    } else if ((TYPES_DIM (g_type) >= 0) && (TYPES_DIM (g_type) == TYPES_DIM (s_type))) {
        /* same known dimensions, check shape */
        spec_flag = SPECTYPE_EQUAL;
        for (i = 0; i < TYPES_DIM (g_type); i++) {
            if (TYPES_SHAPE (g_type, i) != TYPES_SHAPE (s_type, i)) {
                /* difference in shape */
                spec_flag = SPECTYPE_NO;
            }
        }
    } else if ((TYPES_DIM (g_type) <= KNOWN_DIM_OFFSET)
               && ((0 - TYPES_DIM (g_type) + KNOWN_DIM_OFFSET) != TYPES_DIM (s_type))) {
        /* known dimension does not match specialized dim */
        spec_flag = SPECTYPE_NO;
    }
    DBUG_RETURN (spec_flag);
}

/******************************************************************************
 *
 * function:
 *   node *AddSpecializedFundef(node *fundefs, node *spec_fundef, node *gen_fundef)
 *
 * description:
 *   builds a new specialized fundef with spec_fundef declaration and gen_fundef
 *   body. adds generated fundef to list of fundefs
 *
 * remark: IMPORTANT
 *   this function dublicates the generic function gen_fundef and substitutes
 *   all types with the specialized one from spec_fundef. Because the first result
 *   type stores some data of the fundef node these data has to be copied manually.
 *   the identifier of all types is copied from the generic fundef typelist to
 *   avoid mixed variable identifiers in different specialized fundef to one
 *   generic body.
 *
 ******************************************************************************/

static node *
AddSpecializedFundef (node *fundefs, node *spec_fundef, node *gen_fundef)
{
    types *old_type;
    node *new_fundef;
    node *s_arg;
    node *n_arg;
    char *arg_name;

    DBUG_ENTER ("AddSpecializedFundef");
    NOTE (("Adding specialization for %s...\n", FUNDEF_NAME (gen_fundef)));

    /* copy complete fundef node */
    new_fundef = DupNode (gen_fundef);
    FUNDEF_ATTRIB (new_fundef) = ST_regular;

    /* adjust to specialized argtypes */
    s_arg = FUNDEF_ARGS (spec_fundef);
    n_arg = FUNDEF_ARGS (new_fundef);
    while (n_arg != NULL) {
        arg_name = StringCopy (ARG_NAME (n_arg)); /* make a copy of original arg. id */
        ARG_TYPE (n_arg) = FreeOneTypes (ARG_TYPE (n_arg));
        ARG_TYPE (n_arg) = DupTypes (ARG_TYPE (s_arg));
        ARG_NAME (n_arg)
          = Free (ARG_NAME (n_arg)); /* free identifier of spec declaration */
        ARG_NAME (n_arg) = arg_name; /* reset original arg. identifier */

        n_arg = ARG_NEXT (n_arg);
        s_arg = ARG_NEXT (s_arg);
    }

    /* adjust to specialized resulttypes */
    old_type = FUNDEF_TYPES (new_fundef);
    FUNDEF_TYPES (new_fundef) = DupTypes (FUNDEF_TYPES (spec_fundef));

    /* transfer fundef data stored in type and free old type */
    FUNDEF_NAME (new_fundef) = old_type->id;
    FUNDEF_MOD (new_fundef) = old_type->id_mod;
    FUNDEF_LINKMOD (new_fundef) = old_type->id_cmod;
    FUNDEF_STATUS (new_fundef) = old_type->status;
    FUNDEF_ATTRIB (new_fundef) = old_type->attrib;

    old_type = FreeAllTypes (old_type);

    /*
     * Mark specialized function as being exported in order to prevent its
     * elimination by dead function removal.
     */
    FUNDEF_STATUS (new_fundef) = ST_exported;

    /* add new_fundef to modules' list of fundefs */
    FUNDEF_NEXT (new_fundef) = fundefs;
    fundefs = new_fundef;

    DBUG_RETURN (fundefs);
}

/******************************************************************************
 *
 * function:
 *   node *IMPSPECarg(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
IMPSPECarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IMPSPECarg");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ScanParseSpecializationFile(char *modname )
 *
 * description:
 *   scan/parse of modules spec file
 *   returns the imported modspec node
 *
 ******************************************************************************/

static node *
ScanParseSpecializationFile (char *modname)
{
    char buffer[MAX_FILE_NAME];
    char *pathname, *abspathname, *old_filename;
    node *spec;

    DBUG_ENTER ("ScanParseSpecializationFile");

    strcpy (buffer, MODUL_NAME (syntax_tree));
    strcat (buffer, ".spec");

    pathname = FindFile (PATH, buffer);
    yyin = fopen (pathname, "r");

    if (yyin == NULL) {
        NOTE (("No additional specialization-file found !"));
        spec = 0;
    } else {
        abspathname = AbsolutePathname (pathname);

        NOTE (("Loading own specializations !"));
        NOTE (("  Parsing file \"%s\" ...", abspathname));

        linenum = 1;
        old_filename = filename; /* required for restauration */
        filename = buffer;
        start_token = PARSE_SPEC;
        My_yyparse ();
        fclose (yyin);

        if ((strcmp (MODSPEC_NAME (spec_tree), MODUL_NAME (syntax_tree)) != 0)
            || (NODE_TYPE (spec_tree) != N_modspec)) {
            SYSERROR (("File \"%s\" provides wrong specialization data", filename));
            ABORT_ON_ERROR;
        }
        spec = spec_tree;
    }

    DBUG_RETURN (spec);
}

/******************************************************************************
 *
 * function:
 *   node *ImportSpecialization( node *syntax_tree )
 *
 * description:
 *   - starts scan/parse of modules spec file
 *   - starts traversal of specialized fundefs to map each fundef to a generic
 *     implemented fundef
 *
 ******************************************************************************/

node *
ImportSpecialization (node *modul_node)
{
    node *arg_info;
    funtab *old_tab;
    node *modspec;

    DBUG_ENTER ("ImportSpecialization");

    modspec = ScanParseSpecializationFile (MODUL_NAME (modul_node));

    if (modspec != NULL) {
        /*
         * analyse specializations of fundefs
         * start traversal of spec fundefs
         */

        arg_info = MakeInfo ();
        INFO_IMPSPEC_SPECS (arg_info) = modspec;
        INFO_IMPSPEC_MODUL (arg_info) = modul_node;

        old_tab = act_tab;
        act_tab = impspec_tab;

        modspec = Trav (modspec, arg_info);

        syntax_tree = INFO_IMPSPEC_MODUL (arg_info);
        act_tab = old_tab;

        arg_info = Free (arg_info);

        FreeNode (modspec);
    }

    DBUG_RETURN (modul_node);
}
