/*
 * $Log$
 * Revision 3.1  2000/11/20 18:03:42  sacbase
 * new release made
 *
 * Revision 1.20  2000/08/02 14:33:25  nmw
 * split code for PIH and PIW traversal in separate files
 * added comments about cinterface
 *
 * Revision 1.19  2000/08/02 13:34:10  nmw
 * comment for simple linker command line added
 *
 * Revision 1.18  2000/08/02 09:19:06  nmw
 * order of init function calls for interface and module changed
 *
 * Revision 1.17  2000/08/01 13:25:24  nmw
 * startup-code generation modified to handle PHM in c-library
 *
 * Revision 1.16  2000/07/28 14:47:07  nmw
 * handling of void functions added
 * handling of T_users types added
 *
 * Revision 1.15  2000/07/24 15:00:37  nmw
 * refcount check in cwrapper added
 * generation of separate c-files with object initflags added
 * code beautifying
 *
 * Revision 1.14  2000/07/20 12:09:49  nmw
 * debug print generated code removed
 *
 * Revision 1.13  2000/07/20 11:40:15  nmw
 * wrapperfunctions check for initialized module and register themselves
 * at the free_interface_hanlder to be freed at cleanup
 *
 * Revision 1.12  2000/07/19 16:44:21  nmw
 * made a variable a bit more static...
 *
 * Revision 1.11  2000/07/19 08:32:33  nmw
 * link order in comment adjusted
 *
 * Revision 1.10  2000/07/13 15:34:09  nmw
 * add comments in generated headerfile
 *
 * Revision 1.9  2000/07/13 14:52:39  nmw
 * handling for global objects and startup code generation added
 *
 * Revision 1.8  2000/07/12 15:52:35  nmw
 * add comment which libraries to link with
 *
 * Revision 1.7  2000/07/12 10:09:08  nmw
 * RCS-header added
 *
 * Revision 1.6  2000/07/12 09:24:46  nmw
 * traversal of returntypes modified. now using TYPE_STATUS()
 *
 * Revision 1.5  2000/07/07 15:34:56  nmw
 * beautyfing of generated code
 *
 * Revision 1.4  2000/07/06 15:56:10  nmw
 * debugging in generated cwrapper code
 *
 * Revision 1.3  2000/07/05 15:33:25  nmw
 * filenames modified according to cccalls.c
 *
 * Revision 1.2  2000/07/05 12:54:30  nmw
 * filenames for printed includes adjusted
 *
 * Revision 1.1  2000/07/05 11:39:33  nmw
 * Initial revision
 *
 *
 * The c-interface allows you to generate a c-library from your SAC-module.
 * at this time, only functions with a fixed shape exported.
 * When you compile your SAC-module with sac2c and the option '-genlib c'
 * you get an headerfile (myMod.h) with comments to all exported functions
 * (the shapes they accept and return) and list of library files to link with.
 * The libary is named libmyMod.a.
 *
 * Involved compiler parts for this c-interface:
 * map_cwrapper.[ch]: traversing the AST, looking for overloaded SAC-functions
 *                    and building up a set of wrapper functions, witch simulate
 *                    this overloading.
 *
 * import_specialization.[ch]: importing an additional myMod.spec file, that contains
 *                    declarations for specializations for generic functions
 *                    defined in myMod.sac. These specializations are defined
 *                    and exported in the c library.
 *                    If no spec file is available, only the functions defined
 *                    in mytMod.sac are exported.
 *
 * print_interface.[ch]: starts traversal for interface headerfile and wrapper code
 *                    it also generates code for initializing the sac-runtime
 *                    system (heapmangement, ...)

 * print_interface_header.[ch]: generating the c-code for the module headerfile
 *
 * print_interface_wrapper.[ch]: generating c-code for the module
 *                    wrapper functions (calls cwrapper.c). The generated
 *                    code uses macros, defined in runtime/sac_cwrapper.h
 *                    It also generated files for global objects
 *
 * libsac/sac_arg.c, runtime/sac_arg.h: implements the abstract datatype used
 *                    be the c-interface functions and the wrapper functions
 *                    these functions are included in the sac-runtime-system
 *
 * libsac/cinterface.c, runtime/sac_cinterface.c: implements the interface
 *                    functions used by the c-programmer for converting c-datatypes
 *                    to sac-datatypes and vice versa
 *
 * modules/cccall.c:  compiles and generates the c-library.
 *
 * known limitations:
 *   - user defined types are always abstract datatypes without any conversion
 *     function you might get them as a result from a sac-functions and use it
 *     as an argument for another sac-function. but you cannot access the type
 *     directly
 *   - no multithreading in an c-library (yet)
 *   - you have to take care of one set of switches for all modules used in one
 *     c executable, concerning private heap management, checks and profiling
 *
 */

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
#include "gen_startup_code.h"

/* function for only local usage */
static void PrintSACRuntimeInitExit (node *arg_node);

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
    DBUG_PRINT ("PIH", ("Generating c library interface header\n"));

    arg_info = MakeInfo ();
    old_tab = act_tab;

    /* do PIH traversal */
    act_tab = pih_tab;
    syntax_tree = Trav (syntax_tree, arg_info);

    DBUG_PRINT ("PIW", ("Generating c library interface wrapper\n"));
    /* do PIW traversal */
    act_tab = piw_tab;
    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;
    FREE (arg_info);

    PrintSACRuntimeInitExit (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *    void PrintSACRuntimeInitExit(node *arg_node)
 *
 * description:
 *   prints code in internal_runtime_init.c which inits the heapmanager
 *   and the multithreading parts of the runtime system
 *   this function calls SAC_InitCInterface
 *
 ******************************************************************************/
static void
PrintSACRuntimeInitExit (node *arg_node)
{
    FILE *old_outfile;

    DBUG_ENTER ("PrintInternalRuntimeInit");

    old_outfile = outfile; /* save, might be in use */

    /* open internal_runtime_init.c in tmpdir for writing*/
    outfile = WriteOpen ("%s/internal_runtime_init.c", tmp_dirname);
    fprintf (outfile, "/* Interface SAC <-> C \n"
                      " * this code initializes the internal data structures\n"
                      " * of the SAC runtime system and calls the CInterface\n"
                      " * init function\n"
                      " */\n\n"
                      "extern void SAC_InitCInterface();\n"
                      "extern void SAC_ExitCInterface();\n\n");

    /* general preload for codefile */
    fprintf (outfile, "/* startup functions and global code */\n\n");
    GSCPrintInternalInitFileHeader (arg_node);
    fprintf (outfile, "void SAC_InitRuntimeSystem()\n"
                      "{\n"
                      "  int __argc=0;\n"
                      "  char **__argv=NULL;\n"
                      "  SAC_MT_SETUP_INITIAL();\n"
                      "  SAC_PF_SETUP();\n"
                      "  SAC_HM_SETUP();\n"
                      "  SAC_MT_SETUP();\n"
                      "  SAC_CS_SETUP();\n"
                      "  SAC_InitCInterface();\n"
                      "}\n\n");
    fprintf (outfile, "void SAC_FreeRuntimeSystem()\n"
                      "{\n"
                      "SAC_ExitCInterface();\n");
    GSCPrintMainEnd ();
    fprintf (outfile, "\n}\n\n");

    fprintf (outfile, "/* generated codefile, please do not modify */\n");
    fclose (outfile);

    outfile = old_outfile; /* restore old filehandle */

    DBUG_VOID_RETURN;
}
