/*
 * $Log$
 * Revision 1.2  2000/07/21 15:13:06  nmw
 * specfile parsing integrated
 *
 * Revision 1.1  2000/07/21 08:18:37  nmw
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "filemgr.h"
#include "globals.h"
#include "free.h"
#include "resource.h"
#include "scnprs.h"
#include "import_specialization.h"

node *
IMPSPECfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IMPSPECfundef");

    DBUG_RETURN (arg_node);
}

node *
IMPSPECmodspec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IMPSPECmodspec");

    DBUG_RETURN (arg_node);
}

node *
IMPSPECarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("IMPSPECarg");

    DBUG_RETURN (arg_node);
}

node *
ImportSpecialization (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    char buffer[MAX_FILE_NAME];
    node *spec = NULL;
    char *pathname, *abspathname, *old_filename;

    DBUG_ENTER ("ImportSpecialization");

    strcpy (buffer, MODUL_NAME (syntax_tree));
    strcat (buffer, ".spec");

    pathname = FindFile (PATH, buffer);
    yyin = fopen (pathname, "r");

    if (yyin == NULL) {
        NOTE (("No additional specialization-file found !"));
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

        /* analyse specializations of fundefs
         * start traversal of spec fundefs
         */

        arg_info = MakeInfo ();
        INFO_IMPSPEC_SPECS (arg_info) = spec;
        INFO_IMPSPEC_MODUL (arg_info) = syntax_tree;

        old_tab = act_tab;
        act_tab = impspec_tab;

        spec = Trav (spec, arg_info);

        syntax_tree = INFO_IMPSPEC_MODUL (arg_info);
        act_tab = old_tab;

        FREE (arg_info);

        FreeNode (spec);
    }

    DBUG_RETURN (syntax_tree);
}
