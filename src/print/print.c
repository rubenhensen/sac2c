/*
 *
 * $Log$
 * Revision 2.6  1999/03/17 22:22:22  bs
 * DbugPrintArray modified. Printing of character- and boolean arrays possible.
 *
 * Revision 2.5  1999/03/17 15:37:09  bs
 * DbugPrintArray modified.
 *
 * Revision 2.4  1999/03/15 15:57:29  sbs
 * braces for if's inserted which might cause ambiguities.
 *
 * Revision 2.3  1999/03/15 14:19:19  bs
 * Access macros renamed (take a look at tree_basic.h).
 *
 * Revision 2.2  1999/03/09 11:21:23  bs
 * Debugging-information in PrintArray and PrintId added. Using the debug flag
 * "ARRAY_FLAT" the compact form of integer vectors will be printed.
 *
 * Revision 2.1  1999/02/23 12:40:24  sacbase
 * new release made
 *
 * Revision 1.261  1999/02/15 15:11:50  cg
 * Defines with function name before each function definition
 * moved into ND_FUNDEC ICM.
 *
 * Revision 1.260  1999/02/10 10:00:20  srs
 * bugfix in PrintAssign()
 *
 * Revision 1.259  1999/02/10 09:19:41  srs
 * added DBUG output for WLI
 *
 * Revision 1.258  1999/02/10 08:44:22  cg
 * bug fixed in PrintModul(), added some useful comments.
 *
 * Revision 1.257  1999/02/09 17:27:57  dkr
 * fixed a bug in PrintModul:
 *   no printf-comands on closed files ... :^)
 *
 * Revision 1.255  1999/02/04 17:23:37  srs
 * added check if arg_info is not NULL.
 *
 * Revision 1.254  1999/01/07 14:00:18  sbs
 * MRD list printing inserted.
 *
 * Revision 1.253  1998/12/21 10:52:25  sbs
 * MRD lists now can be printed by using #d,MRD,MASK !
 *
 * Revision 1.252  1998/12/03 09:21:31  sbs
 * ftoi, ftod, and friends eliminated
 *
 * Revision 1.251  1998/10/30 09:51:32  cg
 * primtive functions min and max or no longer printed
 * in infix notation.
 *
 * Revision 1.250  1998/10/26 18:04:48  dkr
 * with -> old_with
 * new_with -> with
 *
 * Revision 1.249  1998/08/11 14:35:17  dkr
 * PrintWLsegVar changed
 *
 * Revision 1.248  1998/08/11 12:09:01  dkr
 * PrintNodeTree extended
 *
 * Revision 1.247  1998/08/11 00:04:04  dkr
 * PrintNodeTree extended
 *
 * Revision 1.246  1998/08/07 14:38:20  dkr
 * PrintWLsegVar added
 *
 * Revision 1.245  1998/08/06 15:30:40  dkr
 * fixed a minor bug in PrintNodeTree
 *
 * Revision 1.244  1998/07/24 00:27:42  dkr
 * fixed some minor bugs in PrintNodeTree
 *
 * Revision 1.243  1998/07/20 16:51:28  dkr
 * PrintNodeTree extended
 *
 * Revision 1.242  1998/07/16 20:42:04  dkr
 * extended PrintNodeTree
 *
 * Revision 1.241  1998/07/03 10:17:22  cg
 * printing of N_spmd node completely changed.
 *
 * Revision 1.240  1998/06/25 08:02:45  cg
 * printing of spmd-functions modified
 *
 * Revision 1.239  1998/06/23 12:44:34  cg
 * bug fixed in indentation mechanism for ICMs
 *
 * Revision 1.238  1998/06/18 13:44:04  cg
 * file is now able to deal correctly with data objects of
 * the abstract data type for the representation of schedulings.
 *
 * Revision 1.237  1998/06/05 15:27:49  cg
 * global variable mod_name_con and macros MOD_NAME_CON MOD MOD_NAME MOD_CON removed
 * Now, module name and symbol name are combined correctly by ':'.
 * Only when it really comes to the generation of C code, the ':' is
 * replaced by '__'. This is done by the renaming of all identifiers
 * during the precompilation phase.
 *
 * Revision 1.236  1998/06/03 14:33:13  cg
 * The preprocessor conditional around spmd-functions is now
 * printed directly by PrintFundef()
 *
 * Revision 1.235  1998/05/28 23:47:45  dkr
 * fixed a bug with ICM_INDENT
 * changed output in PrintNwithop
 *
 * Revision 1.234  1998/05/28 16:33:45  dkr
 * changed output for N_Nwith, N_Nwith2, N_spmd, N_sync
 * added an indent-mechanismus for H-ICMs in PrintIcm
 *
 * Revision 1.233  1998/05/27 11:19:44  cg
 * global variable 'filename' which contains the current file name in order
 * to provide better error messages is now handled correctly.
 *
 * Revision 1.232  1998/05/24 00:40:34  dkr
 * removed WLGRID_CODE_TEMPLATE
 *
 * Revision 1.231  1998/05/21 14:45:45  dkr
 * added a \n in PrintModarray, PrintGenarray, PrintFold...
 *
 * Revision 1.230  1998/05/19 15:40:41  dkr
 * fixed a bug in PrintWLgridVar
 *
 * Revision 1.229  1998/05/17 00:09:34  dkr
 * changed PrintWLgrid, PrintWLgridVar:
 *   WLGRID_CEXPR_TEMPLATE is now WLGRID_CODE_TEMPLATE
 *
 * Revision 1.228  1998/05/15 23:53:15  dkr
 * added a comment
 *
 * ... [eliminated] ...
 *
 * Revision 1.6  1994/11/10  15:34:26  sbs
 * RCS-header inserted
 *
 *
 */

/*
 * use of arg_info in this file:
 * - node[0]: is used for storing the current fundef node.
 * - node[1]: profile macros  (?)
 * - node[2]: determines which syntax of the new WLs is printed. If it's
 *   NULL then the internal syntax is uses which allows to state more than
 *   one Npart. Else the last (and hopefully only) Npart returns the
 *   last expr in node[2].
 * - node[3]: is used while printing the old WLs to return the main
 *   expr from the block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"

#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "optimize.h"
#include "generatemasks.h"
#include "filemgr.h"
#include "globals.h"
#include "gen_startup_code.h"
#include "WithloopFolding.h"
#include "scheduling.h"

/******************************************************************************/

static int print_separate = 0;

/******************************************************************************/

/*
 * First, we generate the external declarations for all functions that
 * expand ICMs to C.
 */

#define ICM_ALL
#define ICM_DEF(prf, trf) extern void Print##prf (node *ex, node *arg_info);
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_VARINT(dim, name)
#define ICM_END(prf, args)
#include "icm.data"
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
#undef ICM_ALL

/******************************************************************************/

#define PRF_IF(n, s, x, y) x

char *prf_string[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

/******************************************************************************
 *
 * function:
 *  void DbugPrintArray(node *arg_node)
 *
 * description:
 *   - this function is only used for printing of debugging informations in
 *     PrintArray and PrintId.
 *
 ******************************************************************************/

static void
DbugPrintArray (node *arg_node)
{
    int *intptr, length, i;
    char *chrptr;
    float *fltptr;
    double *dblptr;

    if (NODE_TYPE (arg_node) == N_array) {
        length = ARRAY_VECLEN (arg_node);
        switch (ARRAY_VECTYPE (arg_node)) {
        case T_int:
            intptr = ARRAY_INTVEC (arg_node);
            if ((intptr == NULL) || (length < 1))
                return;
            else {
                fprintf (outfile, ":[%d", intptr[0]);
                for (i = 1; i < ((length < 10) ? (length) : (10)); i++)
                    fprintf (outfile, ",%d", intptr[i]);
            }
            break;
        case T_float:
            fltptr = ARRAY_FLOATVEC (arg_node);
            if ((fltptr == NULL) || (length < 1))
                return;
            else {
                fprintf (outfile, ":[%f", fltptr[0]);
                for (i = 1; i < ((length < 10) ? (length) : (10)); i++)
                    fprintf (outfile, ",%f", fltptr[i]);
            }
            break;
        case T_double:
            dblptr = ARRAY_DOUBLEVEC (arg_node);
            if ((dblptr == NULL) || (length < 1))
                return;
            else {
                fprintf (outfile, ":[%f", dblptr[0]);
                for (i = 1; i < ((length < 10) ? (length) : (10)); i++)
                    fprintf (outfile, ",%f", dblptr[i]);
            }
            break;
        case T_bool:
            intptr = ARRAY_INTVEC (arg_node);
            if ((intptr == NULL) || (length < 1))
                return;
            else {
                fprintf (outfile, ":[%s", ((intptr[0] == 0) ? ("false") : ("true")));
                for (i = 1; i < ((length < 10) ? (length) : (10)); i++)
                    fprintf (outfile, ",%s", ((intptr[i] == 0) ? ("false") : ("true")));
            }
            break;
        case T_char:
            chrptr = ARRAY_CHARVEC (arg_node);
            if ((chrptr == NULL) || (length < 1))
                return;
            else {
                fprintf (outfile, ":[");
                if ((chrptr[0] >= ' ') && (chrptr[0] <= '~') && (chrptr[0] != '\'')) {
                    fprintf (outfile, ",'%c'", chrptr[0]);
                } else {
                    fprintf (outfile, ",'\\%o'", chrptr[0]);
                }
                for (i = 1; i < ((length < 10) ? (length) : (10)); i++)
                    if ((chrptr[i] >= ' ') && (chrptr[i] <= '~') && (chrptr[i] != '\'')) {
                        fprintf (outfile, ",'%c'", chrptr[i]);
                    } else {
                        fprintf (outfile, ",'\\%o'", chrptr[i]);
                    }
            }
            break;
        default:
            return;
        }
    } else /* (NODE_TYPE(arg_node) == N_id) */ {
        length = ID_VECLEN (arg_node);
        intptr = ID_INTVEC (arg_node);
        if ((intptr == NULL) || (length < 1))
            return;
        else {
            fprintf (outfile, ":[%d", intptr[0]);
            for (i = 1; i < ((length < 10) ? (length) : (10)); i++)
                fprintf (outfile, ",%d", intptr[i]);
        }
    }
    if (length > 10)
        fprintf (outfile, ",..]");
    else
        fprintf (outfile, "]");
    return;
}

/******************************************************************************/

void
PrintIds (ids *arg)
{
    DBUG_ENTER ("PrintIds");

    do {
        DBUG_PRINT ("PRINT", ("%s", IDS_NAME (arg)));

        if (IDS_MOD (arg) != NULL) {
            fprintf (outfile, "%s:", IDS_MOD (arg));
        }
        fprintf (outfile, "%s", IDS_NAME (arg));
        if ((IDS_REFCNT (arg) != -1) && show_refcnt) {
            fprintf (outfile, ":%d", IDS_REFCNT (arg));
        }
        if (show_idx && IDS_USE (arg)) {
            Trav (IDS_USE (arg), NULL);
        }
        if (NULL != IDS_NEXT (arg)) {
            fprintf (outfile, ", ");
        }
        arg = IDS_NEXT (arg);
    } while (NULL != arg);

    DBUG_VOID_RETURN;
}

/******************************************************************************/

void
PrintNums (nums *n)
{
    DBUG_ENTER ("PrintNums");

    while (n != NULL) {
        fprintf (outfile, "%d", NUMS_NUM (n));

        if (NUMS_NEXT (n) != NULL) {
            fprintf (outfile, ", ");
        }

        n = NUMS_NEXT (n);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************/

node *
PrintAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAssign");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - assign : %s(%p)\n",
                                   mdb_nodetype[NODE_TYPE (ASSIGN_INSTR (arg_node))],
                                   ASSIGN_INSTR (arg_node));
                  PrintMasks (arg_node, arg_info););

    DBUG_EXECUTE ("WLI", if (N_let == NODE_TYPE (ASSIGN_INSTR (arg_node))
                             && F_psi == PRF_PRF (LET_EXPR (ASSIGN_INSTR (arg_node))))
                           DbugIndexInfo (ASSIGN_INDEX (arg_node)););

    if (N_icm == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        PrintIcm (ASSIGN_INSTR (arg_node), arg_info);
        fprintf (outfile, "\n");
        if (ASSIGN_NEXT (arg_node))
            Trav (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        DBUG_EXECUTE ("LINE", fprintf (outfile, "/*%03d*/", arg_node->lineno););

        if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return)
            || ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_return)
                && (RETURN_EXPRS (ASSIGN_INSTR (arg_node)) != NULL))) {
            INDENT;
            Trav (ASSIGN_INSTR (arg_node), arg_info);
            fprintf (outfile, "\n");
        }

        if (ASSIGN_NEXT (arg_node)) {
            Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintBlock (node *arg_node, node *arg_info)
{
    static int not_yet_done_print_main_begin = 1;
    /*
     * This static variable assures that only once for the outer block of
     * the main() function initialization code is generated, but not for
     * subsequent blocks of perhaps loops or conditionals.
     */

    DBUG_ENTER ("PrintBlock");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    INDENT;
    fprintf (outfile, "{ \n");
    indent++;

    if (BLOCK_VARDEC (arg_node) != NULL) {
        Trav (BLOCK_VARDEC (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if (arg_info && /* arg_info may be NULL if Print() is called */
                    /* from within a debugger (PrintFundef did */
                    /* not craete an info node). */
        not_yet_done_print_main_begin && (NODE_TYPE (arg_info) == N_info)
        && (INFO_PRINT_FUNDEF (arg_info) != NULL)
        && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)
        && (compiler_phase == PH_genccode)) {
        GSCPrintMainBegin ();
        not_yet_done_print_main_begin = 0;
    }

    if (BLOCK_INSTR (arg_node)) {
        Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintLet");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if (LET_IDS (arg_node)) {
        PrintIds (LET_IDS (arg_node));
        fprintf (outfile, " = ");
    }
    Trav (LET_EXPR (arg_node), arg_info);
    fprintf (outfile, "; ");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintAnnotate (node *arg_node, node *arg_info)
{
    static char strbuffer1[256];
    static char strbuffer2[256];

    DBUG_ENTER ("PrintAnnotate");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if (ANNOTATE_TAG (arg_node) & CALL_FUN) {
        sprintf (strbuffer1, "PROFILE_BEGIN_UDF( %d ,%d )", ANNOTATE_FUNNUMBER (arg_node),
                 ANNOTATE_FUNAPNUMBER (arg_node));
    } else {
        if (ANNOTATE_TAG (arg_node) & RETURN_FROM_FUN) {
            sprintf (strbuffer1, "PROFILE_END_UDF( %d ,%d )",
                     ANNOTATE_FUNNUMBER (arg_node), ANNOTATE_FUNAPNUMBER (arg_node));
        } else {
            DBUG_ASSERT ((1 == 0), "wrong tag at N_annotate");
        }
    }

    if (ANNOTATE_TAG (arg_node) & INL_FUN) {
        sprintf (strbuffer2, "PROFILE_INLINE( %s )", strbuffer1);
    } else {
        strcpy (strbuffer2, strbuffer1);
    }

    if (ANNOTATE_TAG (arg_node) & LIB_FUN) {
        sprintf (strbuffer1, "PROFILE_LIBRARY( %s )", strbuffer2);
    } else {
        strcpy (strbuffer1, strbuffer2);
    }

    fprintf (outfile, "%s;", strbuffer1);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintModul");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if (print_separate) {
        /*
         * In this case, we print a module or class implementation and we want
         * each function to appear in a separate file to create a real archive
         * for later linking.
         *
         * So we produce several files in the temporary directory:
         *   header.h   contains all type definitions and an external declaration
         *              for each global object and function. This header file
         *              is included by all other files.
         *   globals.c  contains the definitions of the global objects.
         *   fun<n>.c   contains the definition of the nth function.
         */
        outfile = WriteOpen ("%s/header.h", tmp_dirname);
        GSCPrintFileHeader (arg_node);

        if (NULL != MODUL_TYPES (arg_node)) {
            fprintf (outfile, "\n\n");
            Trav (MODUL_TYPES (arg_node), arg_info); /* print typedefs */
        }

        if (NULL != MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            Trav (MODUL_FUNS (arg_node), arg_node); /* print function declarations */
                                                    /*
                                                     * Here, we do print only function declarations. This is done by traversing
                                                     * with arg_info != NULL !!
                                                     */
        }

        if (NULL != MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = 1;
            Trav (MODUL_OBJS (arg_node), arg_info); /* print object declarations */
        }

        fclose (outfile);

        outfile = WriteOpen ("%s/globals.c", tmp_dirname);
        fprintf (outfile, "#include \"header.h\"\n\n");
        fprintf (outfile, "int __dummy_value_which_is_completely_useless=0;\n\n");

        if (NULL != MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = 0;
            Trav (MODUL_OBJS (arg_node), arg_info); /* print object definitions */
        }

        fclose (outfile);
        /*
         *  Maybe there's nothing to compile in this module because all functions
         *  deal with shape-independent arrays which are removed after writing
         *  the SIB.
         *
         *  Unfortunately, things are not as easy as they should be.
         *  If there is no symbol declared in this file then gcc creates an
         *  object file which does not contain any objects. This causes ranlib
         *  not (!!) to produce an empty symbol table, but to produce no symbol
         *  table at all. Finding no symbol table lets the linker give some
         *  nasty warnings. These are suppressed by the above dummy symbol.
         */

        if (NULL != MODUL_FUNS (arg_node)) {
            Trav (MODUL_FUNS (arg_node), NULL); /* print function definitions */
                                                /*
                                                 * Here, we do print full function definitions. This is done by traversing
                                                 * with arg_info == NULL !!
                                                 *
                                                 * Note that in this case a separate file is created for each function.
                                                 * These files are opened and closed in PrintFundef().
                                                 */
        }
    } else {
        switch (MODUL_FILETYPE (arg_node)) {
        case F_modimp:
            fprintf (outfile, "\n/*\n *  Module %s :\n */\n", MODUL_NAME (arg_node));
            break;
        case F_classimp:
            fprintf (outfile, "\n/*\n *  Class %s :\n", MODUL_NAME (arg_node));
            if (MODUL_CLASSTYPE (arg_node) != NULL) {
                fprintf (outfile, " *  classtype %s;\n",
                         Type2String (MODUL_CLASSTYPE (arg_node), 0));
            }
            fprintf (outfile, " */\n");
            break;
        case F_prog:
            fprintf (outfile, "\n/*\n *  SAC-Program %s :\n */\n", puresacfilename);
            break;
        default:;
        }

        if (MODUL_IMPORTS (arg_node)) {
            fprintf (outfile, "\n");
            Trav (MODUL_IMPORTS (arg_node), arg_info); /* print import-list */
        }

        if (MODUL_TYPES (arg_node)) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  type definitions\n");
            fprintf (outfile, " */\n\n");
            Trav (MODUL_TYPES (arg_node), arg_info); /* print typedefs */
        }

        if (MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  function declarations\n");
            fprintf (outfile, " */\n\n");
            Trav (MODUL_FUNS (arg_node), arg_node); /* print functions */
        }

        if (MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  global objects\n");
            fprintf (outfile, " */\n\n");
            Trav (MODUL_OBJS (arg_node), arg_info); /* print objdefs */
        }

        if (MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  function definitions\n");
            fprintf (outfile, " */\n");
            Trav (MODUL_FUNS (arg_node), NULL); /* print functions */
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintImplist (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintImplist");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    fprintf (outfile, "import %s: ", IMPLIST_NAME (arg_node));

    if ((IMPLIST_ITYPES (arg_node) == NULL) && (IMPLIST_ETYPES (arg_node) == NULL)
        && (IMPLIST_FUNS (arg_node) == NULL) && (IMPLIST_OBJS (arg_node) == NULL)) {
        fprintf (outfile, "all;\n");
    } else {
        fprintf (outfile, "{");
        if (IMPLIST_ITYPES (arg_node) != NULL) {
            fprintf (outfile, "\n  implicit types: ");
            PrintIds (IMPLIST_ITYPES (arg_node)); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_ETYPES (arg_node) != NULL) {
            fprintf (outfile, "\n  explicit types: ");
            PrintIds (IMPLIST_ETYPES (arg_node)); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_OBJS (arg_node) != NULL) {
            fprintf (outfile, "\n  global objects: ");
            PrintIds (IMPLIST_OBJS (arg_node)); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_FUNS (arg_node) != NULL) {
            fprintf (outfile, "\n  funs: ");
            PrintIds (IMPLIST_FUNS (arg_node)); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        fprintf (outfile, "\n}\n");
    }

    if (IMPLIST_NEXT (arg_node))
        Trav (IMPLIST_NEXT (arg_node), arg_info); /* print further imports */

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintTypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintTypedef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    fprintf (outfile, "typedef %s ", Type2String (TYPEDEF_TYPE (arg_node), 0));
    if (TYPEDEF_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", TYPEDEF_MOD (arg_node));
    }
    fprintf (outfile, "%s;\n", TYPEDEF_NAME (arg_node));

    if (TYPEDEF_COPYFUN (arg_node) != NULL) {
        fprintf (outfile, "\nextern void *%s(void *);\n", TYPEDEF_COPYFUN (arg_node));
        fprintf (outfile, "extern void %s(void *);\n\n", TYPEDEF_FREEFUN (arg_node));
    }

    if (TYPEDEF_NEXT (arg_node)) {
        Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintObjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintObjdef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if ((OBJDEF_ICM (arg_node) != NULL) && (NODE_TYPE (OBJDEF_ICM (arg_node)) == N_icm)) {
        Trav (OBJDEF_ICM (arg_node), arg_info);
        fprintf (outfile, "\n");
    } else {
        if ((OBJDEF_STATUS (arg_node) == ST_imported) || print_objdef_for_header_file) {
            fprintf (outfile, "extern ");
        }

        fprintf (outfile, "%s ", Type2String (OBJDEF_TYPE (arg_node), 0));

        if (OBJDEF_MOD (arg_node) != NULL) {
            fprintf (outfile, "%s:", OBJDEF_MOD (arg_node));
        }

        fprintf (outfile, "%s", OBJDEF_NAME (arg_node));

        if (OBJDEF_EXPR (arg_node) != NULL) {
            fprintf (outfile, " = ");
            Trav (OBJDEF_EXPR (arg_node), arg_info);
        }

        fprintf (outfile, ";\n");

        if (OBJDEF_PRAGMA (arg_node) != NULL) {
            Trav (OBJDEF_PRAGMA (arg_node), arg_info);
        }
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

void
PrintFunctionHeader (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintFunctionHeader");

    if (0 != FUNDEF_INLINE (arg_node)) {
        fprintf (outfile, "inline ");
    }

    fprintf (outfile, "%s ", Type2String (FUNDEF_TYPES (arg_node), 0));

    if (FUNDEF_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", FUNDEF_MOD (arg_node));
    }

    fprintf (outfile, "%s(", FUNDEF_NAME (arg_node));

    if (FUNDEF_ARGS (arg_node) != NULL) {
        Trav (FUNDEF_ARGS (arg_node), arg_info); /* print args of function */
    }

    fprintf (outfile, ")");

    DBUG_VOID_RETURN;
}

/******************************************************************************/

/*
 * Remark for PrintFundef:
 *
 *  arg_info is used as flag :
 *  arg_info == NULL: print function definitions (with body)
 *  arg_info != NULL: print function declarations (without body)
 *
 *  If C-code is to be generated, which means that an N_icm node already
 *  hangs on node[3], additional extern declarations for function
 *  definitions are printed.
 */

node *
PrintFundef (node *arg_node, node *arg_info)
{
    node *new_info;

    DBUG_ENTER ("PrintFundef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    new_info = MakeInfo ();
    new_info->varno = FUNDEF_VARNO (arg_node);
    /*
     * needed for the introduction of PROFILE_... MACROS in the
     *  function body.
     */
    INFO_PRINT_FUNDEF (new_info) = arg_node;
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - function\n");
                  PrintMasks (arg_node, new_info););

    if (arg_info == NULL) {
        /*
         * print function definition
         */

        if (FUNDEF_BODY (arg_node) != NULL) {
            if (print_separate) {
                outfile = WriteOpen ("%s/fun%d.c", tmp_dirname, function_counter);

                fprintf (outfile, "#include \"header.h\"\n");
            }

            fprintf (outfile, "\n");

            if ((FUNDEF_STATUS (arg_node) == ST_spmdfun)
                && (compiler_phase == PH_genccode)) {
                fprintf (outfile, "#if SAC_DO_MULTITHREAD\n\n");
            }

            if ((FUNDEF_ICM (arg_node) != NULL)
                && (N_icm == NODE_TYPE (FUNDEF_ICM (arg_node)))) {
                Trav (FUNDEF_ICM (arg_node), new_info); /* print N_icm ND_FUN_DEC */
            } else {
                PrintFunctionHeader (arg_node, new_info);
            }

            fprintf (outfile, "\n");
            Trav (FUNDEF_BODY (arg_node), new_info); /* traverse function body */

            if (FUNDEF_PRAGMA (arg_node) != NULL) {
                Trav (FUNDEF_PRAGMA (arg_node), NULL);
            }

            if ((FUNDEF_STATUS (arg_node) == ST_spmdfun)
                && (compiler_phase == PH_genccode)) {
                fprintf (outfile, "\n#endif  /* SAC_DO_MULTITHREAD */\n\n");
            }

            if (print_separate) {
                fclose (outfile);
                function_counter++;
            }
        }
    } else {
        /*
         * print function declaration
         */

        if (FUNDEF_STATUS (arg_node) != ST_spmdfun) {
            if ((FUNDEF_BODY (arg_node) == NULL)
                || ((NULL != FUNDEF_RETURN (arg_node))
                    && (N_icm == NODE_TYPE (FUNDEF_RETURN (arg_node)))
                    && (0 != strcmp (FUNDEF_NAME (arg_node), "main")))) {

                fprintf (outfile, "extern ");

                if ((NULL != FUNDEF_ICM (arg_node))
                    && (N_icm == NODE_TYPE (FUNDEF_ICM (arg_node)))
                    && (FUNDEF_STATUS (arg_node) != ST_spmdfun)) {
                    Trav (FUNDEF_ICM (arg_node), new_info); /* print N_icm ND_FUN_DEC */
                } else {
                    PrintFunctionHeader (arg_node, new_info);
                }

                fprintf (outfile, ";\n");

                if (FUNDEF_PRAGMA (arg_node) != NULL) {
                    Trav (FUNDEF_PRAGMA (arg_node), NULL);
                }
            }
        }
    }

    FREE (new_info);

    if (FUNDEF_NEXT (arg_node)) {
        Trav (FUNDEF_NEXT (arg_node), arg_info); /* traverse next function */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintPrf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPrf");

    DBUG_PRINT ("PRINT", ("%s (%s)" P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)],
                          mdb_prf[PRF_PRF (arg_node)], arg_node));

    switch (PRF_PRF (arg_node)) {
    case F_psi:
    case F_take:
    case F_drop:
    case F_shape:
    case F_reshape:
    case F_cat:
    case F_dim:
    case F_rotate:
    case F_not:
    case F_toi:
    case F_toi_A:
    case F_tof:
    case F_tof_A:
    case F_tod:
    case F_tod_A:
    case F_idx_psi:
    case F_modarray:
    case F_genarray:
    case F_idx_modarray:
    case F_min:
    case F_max: {
        /* primitive functions that are printed as function application */
        fprintf (outfile, "%s( ", prf_string[PRF_PRF (arg_node)]);
        Trav (PRF_ARGS (arg_node), arg_info);
        fprintf (outfile, " )");
        break;
    }
    default: {
        /* primitive functions in infix notation */
        fprintf (outfile, "(");
        Trav (EXPRS_EXPR (PRF_ARGS (arg_node)), arg_info);
        fprintf (outfile, " %s ", prf_string[PRF_PRF (arg_node)]);
        if (NULL != EXPRS_NEXT (PRF_ARGS (arg_node))) {
            DBUG_ASSERT ((EXPRS_NEXT (EXPRS_NEXT (PRF_ARGS (arg_node))) == NULL),
                         "more than two args found");
            Trav (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info);
        }
        fprintf (outfile, ")");
        break;
    }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintStr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintStr");

    DBUG_ASSERT ((N_str == NODE_TYPE (arg_node)), "wrong node type");

    fprintf (outfile, "\"%s\"", STR_STRING (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintId (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintId");

    DBUG_ASSERT ((N_id == NODE_TYPE (arg_node)), "wrong node type");

    if ((ID_ATTRIB (arg_node) == ST_global) && (ID_MOD (arg_node) != NULL)) {
        fprintf (outfile, "%s:", ID_MOD (arg_node));
    }

    if ((0 == show_refcnt) || (-1 == ID_REFCNT (arg_node))) {
        fprintf (outfile, "%s", ID_NAME (arg_node));
    } else {
        fprintf (outfile, "%s:%d", ID_NAME (arg_node), ID_REFCNT (arg_node));
    }

    if (compiler_phase != PH_genccode) {
        DBUG_EXECUTE ("ARRAY_FLAT", DbugPrintArray (arg_node););
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintNum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNum");

    fprintf (outfile, "%d", NUM_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintChar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintChar");

    if ((CHAR_VAL (arg_node) >= ' ') && (CHAR_VAL (arg_node) <= '~')
        && (CHAR_VAL (arg_node) != '\'')) {
        fprintf (outfile, "'%c'", CHAR_VAL (arg_node));
    } else {
        fprintf (outfile, "'\\%o'", CHAR_VAL (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintFloat (node *arg_node, node *arg_info)
{
    char *tmp_string;

    DBUG_ENTER ("PrintFloat");

    tmp_string = Float2String (FLOAT_VAL (arg_node));
    fprintf (outfile, "%s", tmp_string);
    FREE (tmp_string);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintDouble (node *arg_node, node *arg_info)
{
    char *tmp_string;

    DBUG_ENTER ("PrintDouble");

    tmp_string = Double2String (DOUBLE_VAL (arg_node));
    fprintf (outfile, "%s", tmp_string);
    FREE (tmp_string);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintBool (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintBool");

    if (0 == BOOL_VAL (arg_node)) {
        fprintf (outfile, "false");
    } else
        fprintf (outfile, "true");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintReturn");

    if (RETURN_EXPRS (arg_node) && (!RETURN_INWITH (arg_node))) {
        if (arg_info && /* may be NULL if call from within debugger */
            (NODE_TYPE (arg_info) = N_info) && (INFO_PRINT_FUNDEF (arg_info) != NULL)
            && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)
            && (compiler_phase == PH_genccode)) {
            GSCPrintMainEnd ();
            INDENT;
        }

        fprintf (outfile, "return( ");
        Trav (RETURN_EXPRS (arg_node), arg_info);
        fprintf (outfile, " );");
    }

    if (RETURN_INWITH (arg_node)) {
        INFO_PRINT_WITH_RET (arg_info) = arg_node;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAp");

    if (AP_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", AP_MOD (arg_node));
    }
    fprintf (outfile, "%s(", AP_NAME (arg_node));
    if (AP_ARGS (arg_node)) {
        Trav (AP_ARGS (arg_node), arg_info);
    }
    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintCast (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCast");

    fprintf (outfile, "(: %s) ", Type2String (CAST_TYPE (arg_node), 0));
    Trav (CAST_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintExprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintExprs");

    Trav (EXPRS_EXPR (arg_node), arg_info);

    if (EXPRS_NEXT (arg_node) != NULL) {
        fprintf (outfile, ", ");
        Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintArg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintArg");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**Number %d -> ", ARG_VARNO (arg_node)););

    fprintf (outfile, "%s",
             Type2String (ARG_TYPE (arg_node), (arg_info == NULL) ? 0 : 1));

    if ((1 == show_refcnt) && (-1 != ARG_REFCNT (arg_node))) {
        fprintf (outfile, ":%d", ARG_REFCNT (arg_node));
    }

    if (ARG_COLCHN (arg_node) && show_idx) {
        Trav (ARG_COLCHN (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ", ");
        Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintVardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintVardec");

    INDENT;

    DBUG_EXECUTE ("MASK", fprintf (outfile, "**Number %d -> ", VARDEC_VARNO (arg_node)););

    fprintf (outfile, "%s", Type2String (VARDEC_TYPE (arg_node), 1));
    if (VARDEC_COLCHN (arg_node) && show_idx) {
        Trav (VARDEC_COLCHN (arg_node), arg_info);
    }
    fprintf (outfile, ";\n");
    if (VARDEC_NEXT (arg_node)) {
        Trav (VARDEC_NEXT (arg_node), arg_info);
    } else {
        fprintf (outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintDo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDo");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - do body\n");
                  PrintMasks (DO_BODY (arg_node), arg_info););

    fprintf (outfile, "do\n");
    if (NULL != DO_BODY (arg_node)) {
        indent++;
        Trav (DO_BODY (arg_node), arg_info); /* traverse body of loop */
        indent--;
    }

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (DO_MASK (arg_node, 1), VARNO);
                  fprintf (outfile, "**Used Variables (do-cnd) : %s\n", text);
                  FREE (text););

    INDENT;
    fprintf (outfile, "while( ");
    Trav (DO_COND (arg_node), arg_info);
    fprintf (outfile, " );\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintEmpty (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintEmpty");

    INDENT;
    fprintf (outfile, "/* empty */\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintWhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWhile");

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (WHILE_MASK (arg_node, 1), VARNO);
                  fprintf (outfile, "**Used Variables (while-cnd) : %s\n", text);
                  FREE (text););

    fprintf (outfile, "while( ");
    Trav (WHILE_COND (arg_node), arg_info);
    fprintf (outfile, " )\n");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - while body\n");
                  PrintMasks (WHILE_BODY (arg_node), arg_info););

    Trav (WHILE_BODY (arg_node), arg_info); /* traverse body of loop */

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCond");

    fprintf (outfile, "if (");

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (COND_MASK (arg_node, 1), VARNO);
                  fprintf (outfile, "**Used Variables (Cond) : %s\n", text);
                  FREE (text););

    Trav (COND_COND (arg_node), arg_info);
    fprintf (outfile, ")\n");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - then\n");
                  PrintMasks (COND_THEN (arg_node), arg_info););

    Trav (COND_THEN (arg_node), arg_info);

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - else\n");
                  PrintMasks (COND_ELSE (arg_node), arg_info););

    INDENT;
    fprintf (outfile, "else\n");
    Trav (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintWith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWith");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - generator\n");
                  PrintMasks (arg_node->node[0], arg_info););
    fprintf (outfile, "old_with (");
    Trav (WITH_GEN (arg_node), arg_info);
    fprintf (outfile, ") ");

    DBUG_EXECUTE ("MASK", char *text;
                  text = PrintMask (arg_node->node[1]->mask[1], VARNO);
                  fprintf (outfile, "\n\n**Used Variables (gen-,modarray) : %s\n", text);
                  FREE (text););

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - with body\n");
                  PrintMasks (arg_node, arg_info););
    Trav (WITH_OPERATOR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintGenator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintGenator");

    Trav (GEN_LEFT (arg_node), arg_info);
    if ((-1 == arg_node->info.ids->refcnt) || (0 == show_refcnt)) {
        fprintf (outfile, " <= %s <= ", arg_node->info.ids->id);
    } else {
        fprintf (outfile, " <= %s:%d <= ", arg_node->info.ids->id,
                 arg_node->info.ids->refcnt);
    }
    Trav (GEN_RIGHT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintGenarray (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintGenarray");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (GENARRAY_BODY (arg_node)))) != N_return) {
        /* right now INFO_PRINT_WITH_RET(arg_info) is NULL, but in PrintReturn it
           will be replaced by a pointer to an N_return node instead of
           printing it. */
        fprintf (outfile, "\n");
        Trav (GENARRAY_BODY (arg_node), arg_info);
        ret_node = INFO_PRINT_WITH_RET (arg_info);

        INDENT;
    } else {
        fprintf (outfile, "\n");
        INDENT;
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (GENARRAY_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "genarray without return-statement");

    fprintf (outfile, "genarray( ");
    Trav (GENARRAY_ARRAY (arg_node), arg_info);
    fprintf (outfile, ", ");
    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintModarray (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintModarray");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (MODARRAY_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (MODARRAY_BODY (arg_node), arg_info);
        ret_node = INFO_PRINT_WITH_RET (arg_info);

        INDENT;
    } else {
        fprintf (outfile, "\n");
        INDENT;
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (MODARRAY_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "modarray without return-statement");

    fprintf (outfile, "modarray( ");
    Trav (MODARRAY_ARRAY (arg_node), arg_info);
    fprintf (outfile, ", %s, ", MODARRAY_ID (arg_node));
    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintFoldfun (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintFold");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (FOLDFUN_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (FOLDFUN_BODY (arg_node), arg_info);
        ret_node = INFO_PRINT_WITH_RET (arg_info);

        INDENT;
    } else {
        fprintf (outfile, "\n");
        INDENT;
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (FOLDFUN_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "foldfun without return-statement");

    fprintf (outfile, "fold( ");
    if (NULL != FOLDFUN_MOD (arg_node)) {
        fprintf (outfile, "%s:", FOLDFUN_MOD (arg_node));
    }
    fprintf (outfile, "%s, ", FOLDFUN_NAME (arg_node));

    Trav (FOLDFUN_NEUTRAL (arg_node), arg_info);
    fprintf (outfile, ", ");
    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, " )");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintFoldprf (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintFold");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (FOLDPRF_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (FOLDPRF_BODY (arg_node), arg_info);
        ret_node = INFO_PRINT_WITH_RET (arg_info);

        INDENT;
    } else {
        fprintf (outfile, "\n");
        INDENT;
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (FOLDPRF_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "foldprf without return-statement");

    fprintf (outfile, "fold( %s, ", prf_string[FOLDPRF_PRF (arg_node)]);

    if (FOLDPRF_NEUTRAL (arg_node) != NULL) {
        Trav (FOLDPRF_NEUTRAL (arg_node), arg_info);
        fprintf (outfile, ", ");
    }

    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, " )");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintArray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintArray");

    fprintf (outfile, "[ ");
    Trav (ARRAY_AELEMS (arg_node), arg_info);
    fprintf (outfile, " ]");

    if (compiler_phase != PH_genccode) {
        DBUG_EXECUTE ("ARRAY_FLAT", DbugPrintArray (arg_node););
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintDec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDec");

    fprintf (outfile, "--");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintInc (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintInc");

    fprintf (outfile, "++");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintPost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPost");

    PrintIds (arg_node->info.ids);
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintPre (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPre");

    Trav (arg_node->node[0], arg_info);
    PrintIds (arg_node->info.ids);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintVectInfo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintVectInfo");
    if (show_idx) {
        if (arg_node->info.use == VECT)
            fprintf (outfile, ":VECT");
        else {
            fprintf (outfile, ":IDX(%s)", Type2String ((types *)arg_node->node[1], 0));
        }
        if (arg_node->node[0]) {
            Trav (arg_node->node[0], arg_info);
        }
        fprintf (outfile, " ");
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintIcm (node *arg_node, node *arg_info)
{
    int compiled_icm = 0;

    DBUG_ENTER ("PrintIcm");

    DBUG_PRINT ("PRINT", ("icm-node %s\n", ICM_NAME (arg_node)));

    if (ICM_INDENT (arg_node) < 0) {
        indent += ICM_INDENT (arg_node);
    }

    INDENT
    if (show_icm == 0) {
#define ICM_ALL
#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (ICM_NAME (arg_node), #prf) == 0) {                                       \
        Print##prf (ICM_ARGS (arg_node), arg_info);                                      \
        compiled_icm = 1;                                                                \
    } else
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_VARINT(dim, name)
#define ICM_END(prf, args)
#include "icm.data"
#undef ICM_ALL
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
        if (strcmp (ICM_NAME (arg_node), "NOOP") == 0) {
            compiled_icm = 1;
        }
    }

    if ((show_icm == 1) || (compiled_icm == 0)) {

        if ((strcmp (ICM_NAME (arg_node), "ND_FUN_RET") == 0)
            && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)
            && (compiler_phase == PH_genccode)) {
            GSCPrintMainEnd ();
            INDENT;
        }

        fprintf (outfile, "SAC_%s(", ICM_NAME (arg_node));
        if (NULL != ICM_ARGS (arg_node)) {
            Trav (ICM_ARGS (arg_node), arg_info);
        }
        fprintf (outfile, ")");
    }

    if (NULL != ICM_NEXT (arg_node)) {
        if ((1 == show_icm) || (0 == compiled_icm)) {
            if (0 == strcmp (ICM_NAME (arg_node), "ND_TYPEDEF_ARRAY")) {
                /*
                 * ICM's within the typedef-chain are connected via ICM_NEXT!
                 */
                fprintf (outfile, "\n");
                INDENT;
            } else {
                /*
                 * ICM's that handle the arguments in fun-decl's are linked
                 * via ICM_NEXT as well!! These have to be connected by colons!
                 */
                fprintf (outfile, ", ");
            }

            Trav (ICM_NEXT (arg_node), arg_info);
        }
    }

    if (ICM_INDENT (arg_node) > 0) {
        indent += ICM_INDENT (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintPragma (node *arg_node, node *arg_info)
{
    int i, first;

    DBUG_ENTER ("PrintPragma");

    if (PRAGMA_LINKNAME (arg_node) != NULL) {
        fprintf (outfile, "#pragma linkname \"%s\"\n", PRAGMA_LINKNAME (arg_node));
    }

    if (PRAGMA_LINKSIGN (arg_node) != NULL) {
        fprintf (outfile, "#pragma linksign [%d", PRAGMA_LS (arg_node, 0));

        for (i = 1; i < PRAGMA_NUMPARAMS (arg_node); i++) {
            fprintf (outfile, ", %d", PRAGMA_LS (arg_node, i));
        }

        fprintf (outfile, "]\n");
    }

    if (PRAGMA_REFCOUNTING (arg_node) != NULL) {
        fprintf (outfile, "#pragma refcounting [");
        first = 1;

        for (i = 0; i < PRAGMA_NUMPARAMS (arg_node); i++) {
            if (PRAGMA_RC (arg_node, i)) {
                if (first) {
                    fprintf (outfile, "%d", i);
                    first = 0;
                } else {
                    fprintf (outfile, ", %d", i);
                }
            }
        }

        fprintf (outfile, "]\n");
    }

    if (PRAGMA_READONLY (arg_node) != NULL) {
        fprintf (outfile, "#pragma readonly [");
        first = 1;

        for (i = 0; i < PRAGMA_NUMPARAMS (arg_node); i++) {
            if (PRAGMA_RO (arg_node, i)) {
                if (first) {
                    fprintf (outfile, "%d", i);
                    first = 0;
                } else {
                    fprintf (outfile, ", %d", i);
                }
            }
        }

        fprintf (outfile, "]\n");
    }

    if (PRAGMA_EFFECT (arg_node) != NULL) {
        fprintf (outfile, "#pragma effect ");
        PrintIds (PRAGMA_EFFECT (arg_node));
        fprintf (outfile, "\n");
    }

    if (PRAGMA_TOUCH (arg_node) != NULL) {
        fprintf (outfile, "#pragma touch ");
        PrintIds (PRAGMA_TOUCH (arg_node));
        fprintf (outfile, "\n");
    }

    if (PRAGMA_COPYFUN (arg_node) != NULL) {
        fprintf (outfile, "#pragma copyfun \"%s\"\n", PRAGMA_COPYFUN (arg_node));
    }

    if (PRAGMA_FREEFUN (arg_node) != NULL) {
        fprintf (outfile, "#pragma freefun \"%s\"\n", PRAGMA_FREEFUN (arg_node));
    }

    if (PRAGMA_INITFUN (arg_node) != NULL) {
        fprintf (outfile, "#pragma initfun \"%s\"\n", PRAGMA_INITFUN (arg_node));
    }

    fprintf (outfile, "\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintSpmd (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSpmd");

    fprintf (outfile, "\n");

    if (SPMD_ICM_BEGIN (arg_node) == NULL) {
        /*
         * The SPMD-block has not yet been compiled.
         */

        INDENT;
        fprintf (outfile, "/*** begin of SPMD region ***\n");

        INDENT
        fprintf (outfile, " * in:");
        DFMPrintMask (outfile, " %s", SYNC_IN (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " * inout:");
        DFMPrintMask (outfile, " %s", SYNC_INOUT (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " * out:");
        DFMPrintMask (outfile, " %s", SYNC_OUT (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " * local:");
        DFMPrintMask (outfile, " %s", SYNC_LOCAL (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " */\n");

        Trav (SPMD_REGION (arg_node), arg_info);

        INDENT
        fprintf (outfile, "/*** end of SPMD region ***/\n");
    } else {
        /*
         * print ICMs
         */
        Trav (SPMD_ICM_BEGIN (arg_node), arg_info);
        fprintf (outfile, "\n");
        Trav (SPMD_ICM_PARALLEL (arg_node), arg_info);
        Trav (SPMD_ICM_ALTSEQ (arg_node), arg_info);
        fprintf (outfile, "\n");
        Trav (SPMD_ICM_SEQUENTIAL (arg_node), arg_info);
        Trav (SPMD_ICM_END (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintSync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSync");

    fprintf (outfile, "\n");
    INDENT;

    fprintf (outfile, "/*** begin of sync region ***\n");

    INDENT
    fprintf (outfile, " * in:");
    DFMPrintMask (outfile, " %s", SYNC_IN (arg_node));
    fprintf (outfile, "\n");

    INDENT
    fprintf (outfile, " * inout:");
    DFMPrintMask (outfile, " %s", SYNC_INOUT (arg_node));
    fprintf (outfile, "\n");

    INDENT
    fprintf (outfile, " * out:");
    DFMPrintMask (outfile, " %s", SYNC_OUT (arg_node));
    fprintf (outfile, "\n");

    INDENT
    fprintf (outfile, " * local:");
    DFMPrintMask (outfile, " %s", SYNC_LOCAL (arg_node));
    fprintf (outfile, "\n");

    if (SYNC_SCHEDULING (arg_node) != NULL) {
        INDENT
        fprintf (outfile, " * scheduling: ");
        SCHPrintScheduling (outfile, SYNC_SCHEDULING (arg_node));
        fprintf (outfile, "\n");
    }

    INDENT
    fprintf (outfile, " */\n");

    indent++;
    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);
    indent--;

    INDENT
    fprintf (outfile, "/*** end of sync region ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   prints Nwith node.
 *
 * remarks: there are syntactic alternatives to print the new WLs.
 * If only one Npart node exists the WL is printed in the way the
 * scanner can handle it. This is essential because the SIBs (which are
 * written with this code) have to be scanned again.
 * If a complete partition exists (more than one Npart) an internal
 * syntax is used.
 *
 * INFO_PRINT_INT_SYN(arg_info) is NULL for the internal syntax or != NULL
 * if 'PrintNpart' shall return the last expr.
 *
 ******************************************************************************/

node *
PrintNwith (node *arg_node, node *arg_info)
{
    node *buffer;

    DBUG_ENTER ("PrintNwith");

    DBUG_ASSERT (arg_info, "arg_info is NULL");
    buffer = INFO_PRINT_INT_SYN (arg_info);

    DBUG_EXECUTE ("WLI",
                  fprintf (outfile,
                           "\n** WLI N_Nwith : "
                           "(PARTS %d, REF %d(%d,%d), CPLX %d, FOLDABLE %d, NO_CHANCE "
                           "%d)\n",
                           NWITH_PARTS (arg_node), NWITH_REFERENCED (arg_node),
                           NWITH_REFERENCED_FOLD (arg_node),
                           NWITH_REFERENCES_FOLDED (arg_node), NWITH_COMPLEX (arg_node),
                           NWITH_FOLDABLE (arg_node), NWITH_NO_CHANCE (arg_node)););

    indent += 2;

    /*
     * check wether to use output format 1 (multiple NParts)
     * or 2 (only one NPart) and use INFO_PRINT_INT_SYN(arg_info)
     * as flag for traversal.
     */
    if (NPART_NEXT (NWITH_PART (arg_node)) != NULL) {
        /* output format 1 */
        INFO_PRINT_INT_SYN (arg_info) = NULL;
        fprintf (outfile, "with\n");
        indent++;
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        indent--;
    } else {
        /* output format 2 */
        INFO_PRINT_INT_SYN (arg_info) = (node *)!NULL; /* set != NULL */
        fprintf (outfile, "with ");
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    if (NPART_NEXT (NWITH_PART (arg_node)) == NULL) {
        /*
         * output format 2: now we have in
         * INFO_PRINT_INT_SYN(arg_info) the last expr.
         */
        if (WO_modarray == NWITH_TYPE (arg_node))
            fprintf (outfile, ", dummy, ");
        else
            fprintf (outfile, ", ");
        Trav (INFO_PRINT_INT_SYN (arg_info), arg_info);
    }
    fprintf (outfile, ")");

    indent -= 2;

    INFO_PRINT_INT_SYN (arg_info) = buffer;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Nwithid-nodes
 *
 ******************************************************************************/

node *
PrintNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNwithid");

    if (NWITHID_VEC (arg_node)) {
        PrintIds (NWITHID_VEC (arg_node));
        if (NWITHID_IDS (arg_node)) {
            fprintf (outfile, "=");
        }
    }

    if (NWITHID_IDS (arg_node)) {
        fprintf (outfile, "[");
        PrintIds (NWITHID_IDS (arg_node));
        fprintf (outfile, "]");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNgenerator(node *gen, node *idx, node *arg_info)
 *
 * description:
 *   prints a generator
 *
 *   ATTENTION: this function is not being used by the conventional
 *   traversation algorithm but from within PrintNPart.
 *
 ******************************************************************************/

node *
PrintNgenerator (node *gen, node *idx, node *arg_info)
{
    DBUG_ENTER ("PrintNgenerator");

    fprintf (outfile, "(");

    /* print upper bound and first operator*/
    if (NGEN_BOUND1 (gen))
        Trav (NGEN_BOUND1 (gen), arg_info);
    else
        fprintf (outfile, ".");
    fprintf (outfile, " %s ", prf_string[NGEN_OP1 (gen)]);

    /* print indices */
    idx = Trav (idx, arg_info);

    /* print second operator and lower bound */
    fprintf (outfile, " %s ", prf_string[NGEN_OP2 (gen)]);
    if (NGEN_BOUND2 (gen))
        Trav (NGEN_BOUND2 (gen), arg_info);
    else
        fprintf (outfile, " .");

    /* print step and width */
    if (NGEN_STEP (gen)) {
        fprintf (outfile, " step ");
        Trav (NGEN_STEP (gen), arg_info);
    }
    if (NGEN_WIDTH (gen)) {
        fprintf (outfile, " width ");
        Trav (NGEN_WIDTH (gen), arg_info);
    }

    fprintf (outfile, ")");

    DBUG_RETURN ((node *)NULL);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Ncode-nodes
 *
 ******************************************************************************/

node *
PrintNcode (node *arg_node, node *arg_info)
{
    node *block;

    DBUG_ENTER ("PrintNcode");
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - Ncode\n");
                  PrintMasks (arg_node, arg_info););

    /* print the code section; first the body */
    block = NCODE_CBLOCK (arg_node);
    fprintf (outfile, " {");
    if (block != NULL) {
        fprintf (outfile, "\n");
        indent++;

        if (BLOCK_VARDEC (block) != NULL) {
            BLOCK_VARDEC (block) = Trav (BLOCK_VARDEC (block), arg_info);
            fprintf (outfile, "\n");
        }

        if (BLOCK_INSTR (block) != NULL) {
            BLOCK_INSTR (block) = Trav (BLOCK_INSTR (block), arg_info);
        }

        indent--;
        INDENT;
    }
    fprintf (outfile, "}");

    /*
     * print the expression if internal syntax should be used.
     * else return expr in INFO_PRINT_INT_SYN(arg_info)
     */
    if (NCODE_CEXPR (arg_node) != NULL) {
        if (INFO_PRINT_INT_SYN (arg_info) != NULL) {
            INFO_PRINT_INT_SYN (arg_info) = NCODE_CEXPR (arg_node);
        } else {
            fprintf (outfile, " : ");
            NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Npart nodes
 *
 ******************************************************************************/

node *
PrintNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNpart");
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - Npart\n");
                  PrintMasks (arg_node, arg_info););

    /* print generator */
    if (INFO_PRINT_INT_SYN (arg_info) == NULL) {
        INDENT; /* each gen in a new line. */
    }
    PrintNgenerator (NPART_GEN (arg_node), NPART_WITHID (arg_node), arg_info);

    DBUG_ASSERT ((NPART_CODE (arg_node) != NULL),
                 "part within WL without pointer to N_Ncode");
    NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);

    if (NPART_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",\n");
        /*
         * continue with other parts
         */
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    } else {
        fprintf (outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwithop(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Nwithop-nodes
 *
 *   ATTENTION: the closed bracket ) is not printed,
 *              because PrintNwith must append the last expr.
 *
 ******************************************************************************/

node *
PrintNwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNwithop");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - Nwithop\n");
                  PrintMasks (arg_node, arg_info););

    INDENT;
    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        fprintf (outfile, "genarray( ");
        Trav (NWITHOP_SHAPE (arg_node), arg_info);
        break;
    case WO_modarray:
        fprintf (outfile, "modarray( ");
        Trav (NWITHOP_ARRAY (arg_node), arg_info);
        break;
    case WO_foldfun:
        if (NWITHOP_MOD (arg_node) == NULL) {
            fprintf (outfile, "fold/*fun*/( %s, ", NWITHOP_FUN (arg_node));
        } else {
            fprintf (outfile, "fold/*fun*/( %s:%s, ", NWITHOP_MOD (arg_node),
                     NWITHOP_FUN (arg_node));
        }
        Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        break;
    case WO_foldprf:
        fprintf (outfile, "fold/*prf*/( %s", prf_string[NWITHOP_PRF (arg_node)]);
        if (NWITHOP_NEUTRAL (arg_node)) {
            fprintf (outfile, ", ");
            Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwith2(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Nwith2-nodes
 *
 ******************************************************************************/

node *
PrintNwith2 (node *arg_node, node *arg_info)
{
    node *code, *tmp_nwith2;

    DBUG_ENTER ("PrintNwith2");

    tmp_nwith2 = INFO_PRINT_NWITH2 (arg_info);
    INFO_PRINT_NWITH2 (arg_info) = arg_node;
    indent += 2;

    fprintf (outfile, "with2 (");
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    fprintf (outfile, ")\n");

    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        INDENT
        fprintf (outfile, "/* scheduling :");
        SCHPrintScheduling (outfile, NWITH2_SCHEDULING (arg_node));
        fprintf (outfile, " */\n");
    }

    INDENT
    fprintf (outfile, "/********** operators: **********/\n");
    code = NWITH2_CODE (arg_node);
    while (code != NULL) {
        INDENT
        fprintf (outfile, "op_%d =", NCODE_NO (code));
        indent++;
        code = Trav (code, arg_info);
        indent--;
        code = NCODE_NEXT (code);

        if (code != NULL) {
            fprintf (outfile, ",\n");
        } else {
            fprintf (outfile, "\n");
        }
    }

    if (NWITH2_SEGS (arg_node) != NULL) {
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    }

    INDENT
    fprintf (outfile, "/********** conexpr: **********/\n");
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    fprintf (outfile, ")");

    indent -= 2;
    INFO_PRINT_NWITH2 (arg_info) = tmp_nwith2;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLseg(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLseg-nodes
 *
 ******************************************************************************/

node *
PrintWLseg (node *arg_node, node *arg_info)
{
    node *seg;
    int i = 0;

    DBUG_ENTER ("PrintWLseg");

    seg = arg_node;
    while (seg != NULL) {
        if (WLSEG_SCHEDULING (seg) == NULL) {
            INDENT
            fprintf (outfile, "/********** segment %d: **********/\n", i++);
        } else {
            INDENT
            fprintf (outfile, "/********** segment %d: **********\n", i++);
            INDENT
            fprintf (outfile, " * scheduling: ");
            SCHPrintScheduling (outfile, WLSEG_SCHEDULING (seg));
            fprintf (outfile, "\n");
            INDENT
            fprintf (outfile, " */\n");
        }

        WLSEG_CONTENTS (seg) = Trav (WLSEG_CONTENTS (seg), arg_info);
        seg = WLSEG_NEXT (seg);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLblock(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLblock-nodes
 *
 ******************************************************************************/

node *
PrintWLblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLblock");

    INDENT
    fprintf (outfile, "(%d -> %d), block%d[%d] %d: ", WLBLOCK_BOUND1 (arg_node),
             WLBLOCK_BOUND2 (arg_node), WLBLOCK_LEVEL (arg_node), WLBLOCK_DIM (arg_node),
             WLBLOCK_STEP (arg_node));

    if (WLBLOCK_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLBLOCK_NEXTDIM (arg_node) = Trav (WLBLOCK_NEXTDIM (arg_node), arg_info);
        indent--;
    }

    if (WLBLOCK_CONTENTS (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLBLOCK_CONTENTS (arg_node) = Trav (WLBLOCK_CONTENTS (arg_node), arg_info);
        indent--;
    }

    if (WLBLOCK_NEXT (arg_node) != NULL) {
        WLBLOCK_NEXT (arg_node) = Trav (WLBLOCK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLublock(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLublock-nodes
 *
 ******************************************************************************/

node *
PrintWLublock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLublock");

    INDENT
    fprintf (outfile, "(%d -> %d), ublock%d[%d] %d: ", WLUBLOCK_BOUND1 (arg_node),
             WLUBLOCK_BOUND2 (arg_node), WLUBLOCK_LEVEL (arg_node),
             WLUBLOCK_DIM (arg_node), WLUBLOCK_STEP (arg_node));

    if (WLUBLOCK_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLUBLOCK_NEXTDIM (arg_node) = Trav (WLUBLOCK_NEXTDIM (arg_node), arg_info);
        indent--;
    }

    if (WLUBLOCK_CONTENTS (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLUBLOCK_CONTENTS (arg_node) = Trav (WLUBLOCK_CONTENTS (arg_node), arg_info);
        indent--;
    }

    if (WLUBLOCK_NEXT (arg_node) != NULL) {
        WLUBLOCK_NEXT (arg_node) = Trav (WLUBLOCK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLstride(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLstride-nodes
 *
 * remark:
 *   N_WLstride, N_WLstriVar differs in output.
 *   The former prints '->', the latter '=>' !!!
 *
 ******************************************************************************/

node *
PrintWLstride (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLstride");

    INDENT
    fprintf (outfile, "(%d -> %d), step%d[%d] %d\n", WLSTRIDE_BOUND1 (arg_node),
             WLSTRIDE_BOUND2 (arg_node), WLSTRIDE_LEVEL (arg_node),
             WLSTRIDE_DIM (arg_node), WLSTRIDE_STEP (arg_node));

    indent++;
    WLSTRIDE_CONTENTS (arg_node) = Trav (WLSTRIDE_CONTENTS (arg_node), arg_info);
    indent--;

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        WLSTRIDE_NEXT (arg_node) = Trav (WLSTRIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLgrid(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLgrid-nodes
 *
 * remark:
 *   N_WLgrid, N_WLgridVar differs in output.
 *   The former prints '->', the latter '=>' !!!
 *
 ******************************************************************************/

node *
PrintWLgrid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLgrid");

    INDENT
    fprintf (outfile, "(%d -> %d): ", WLGRID_BOUND1 (arg_node), WLGRID_BOUND2 (arg_node));

    indent++;
    if (WLGRID_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        WLGRID_NEXTDIM (arg_node) = Trav (WLGRID_NEXTDIM (arg_node), arg_info);
    } else {
        if (WLGRID_CODE (arg_node) != NULL) {
            fprintf (outfile, "op_%d\n", NCODE_NO (WLGRID_CODE (arg_node)));
        } else {
            switch (NWITH2_TYPE (INFO_PRINT_NWITH2 (arg_info))) {
            case WO_genarray:
                fprintf (outfile, "init\n");
                break;
            case WO_modarray:
                fprintf (outfile, "copy\n");
                break;
            case WO_foldfun:
                /* here is no break missing! */
            case WO_foldprf:
                fprintf (outfile, "noop\n");
                break;
            default:
                DBUG_ASSERT ((0), "wrong with-loop type found");
            }
        }
    }
    indent--;

    if (WLGRID_NEXT (arg_node) != NULL) {
        WLGRID_NEXT (arg_node) = Trav (WLGRID_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLsegVar(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLsegVar-nodes
 *
 ******************************************************************************/

node *
PrintWLsegVar (node *arg_node, node *arg_info)
{
    node *seg;
    int i = 0;

    DBUG_ENTER ("PrintWLsegVar");

    seg = arg_node;
    while (seg != NULL) {
        if (WLSEGVAR_SCHEDULING (seg) == NULL) {
            INDENT
            fprintf (outfile, "/********** (var.) segment %d: **********/\n", i++);
        } else {
            INDENT
            fprintf (outfile, "/********** (var.) segment %d: **********\n", i++);
            INDENT
            fprintf (outfile, " * scheduling: ");
            SCHPrintScheduling (outfile, WLSEGVAR_SCHEDULING (seg));
            fprintf (outfile, "\n");
            INDENT
            fprintf (outfile, " */\n");
        }

        WLSEGVAR_CONTENTS (seg) = Trav (WLSEGVAR_CONTENTS (seg), arg_info);
        seg = WLSEG_NEXT (seg);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   void PrintWLvar(node *arg_node, node *arg_info)
 *
 * description:
 *   prints a son of N_WLstriVar- and N_WLgridVar-nodes.
 *
 ******************************************************************************/

void
PrintWLvar (node *arg_node, int dim)
{
    DBUG_ENTER ("PrintWLvar");

    switch (NODE_TYPE (arg_node)) {
    case N_num:

        fprintf (outfile, "%d", NUM_VAL (arg_node));
        break;

    case N_id:

        fprintf (outfile, "%s[%d]", ID_NAME (arg_node), dim);
        break;

    default:

        DBUG_ASSERT ((0), "wrong node type found");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLstriVar(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLstriVar-nodes
 *
 * remark:
 *   N_WLstride, N_WLstriVar differs in output.
 *   The former prints '->', the latter '=>' !!!
 *
 ******************************************************************************/

node *
PrintWLstriVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLstriVar");

    INDENT
    fprintf (outfile, "(");
    PrintWLvar (WLSTRIVAR_BOUND1 (arg_node), WLSTRIVAR_DIM (arg_node));
    fprintf (outfile, " => ");
    PrintWLvar (WLSTRIVAR_BOUND2 (arg_node), WLSTRIVAR_DIM (arg_node));
    fprintf (outfile, "), step%d[%d] ", WLSTRIVAR_LEVEL (arg_node),
             WLSTRIVAR_DIM (arg_node));
    PrintWLvar (WLSTRIVAR_STEP (arg_node), WLSTRIVAR_DIM (arg_node));
    fprintf (outfile, "\n");

    indent++;
    WLSTRIVAR_CONTENTS (arg_node) = Trav (WLSTRIVAR_CONTENTS (arg_node), arg_info);
    indent--;

    if (WLSTRIVAR_NEXT (arg_node) != NULL) {
        WLSTRIVAR_NEXT (arg_node) = Trav (WLSTRIVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLgridVar( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLgrid-nodes
 *
 * remark:
 *   N_WLgrid, N_WLgridVar differs in output.
 *   The former prints '->', the latter '=>' !!!
 *
 ******************************************************************************/

node *
PrintWLgridVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLgridVar");

    INDENT
    fprintf (outfile, "(");
    PrintWLvar (WLGRIDVAR_BOUND1 (arg_node), WLGRIDVAR_DIM (arg_node));
    fprintf (outfile, " => ");
    PrintWLvar (WLGRIDVAR_BOUND2 (arg_node), WLGRIDVAR_DIM (arg_node));
    fprintf (outfile, "): ");

    indent++;
    if (WLGRIDVAR_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        WLGRIDVAR_NEXTDIM (arg_node) = Trav (WLGRIDVAR_NEXTDIM (arg_node), arg_info);
    } else {
        if (WLGRIDVAR_CODE (arg_node) != NULL) {
            fprintf (outfile, "op_%d\n", NCODE_NO (WLGRIDVAR_CODE (arg_node)));
        } else {
            switch (NWITH2_TYPE (INFO_PRINT_NWITH2 (arg_info))) {
            case WO_genarray:
                fprintf (outfile, "init\n");
                break;
            case WO_modarray:
                fprintf (outfile, "copy\n");
                break;
            case WO_foldfun:
                /* here is no break missing! */
            case WO_foldprf:
                fprintf (outfile, "noop\n");
                break;
            default:
                DBUG_ASSERT ((0), "wrong with-loop type found");
            }
        }
    }
    indent--;

    if (WLGRIDVAR_NEXT (arg_node) != NULL) {
        WLGRIDVAR_NEXT (arg_node) = Trav (WLGRIDVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Print(node *syntax_tree)
 *
 * description:
 *   initiates print of (sub-)tree
 *
 ******************************************************************************/

node *
Print (node *syntax_tree)
{
    funptr *old_tab;

    DBUG_ENTER ("Print");

    /* save act_tab to restore it afterwards. This could be useful if
       Print() is called from inside a debugger. */
    old_tab = act_tab;

    act_tab = print_tab;

    indent = 0;

    if (compiler_phase == PH_genccode) {
        switch (linkstyle) {
        case 0:
            /*
             * The current file is a SAC program.
             * Therefore, the C file is generated within the target directory.
             */

            outfile = WriteOpen ("%s%s", targetdir, cfilename);
            NOTE (("Writing file \"%s%s\"", targetdir, cfilename));
            GSCPrintFileHeader (syntax_tree);
            syntax_tree = Trav (syntax_tree, NULL);
            fclose (outfile);
            break;

        case 1:
            /*
             * The current file is a module/class implementation, but the functions
             * are nevertheless not compiled separatly to the archive.
             * Therefore, the C file is generated within the temprorary directory.
             */

            outfile = WriteOpen ("%s/%s", tmp_dirname, cfilename);
            NOTE (("Writing file \"%s%s\"", targetdir, cfilename));
            GSCPrintFileHeader (syntax_tree);
            syntax_tree = Trav (syntax_tree, NULL);
            fclose (outfile);
            break;

        case 2:
            /*
             * The current file is a module/class implementation. The functions and
             * global objects are all printed to separate files allowing for separate
             * compilation and the building of an archive. An additional header file
             * is generated for global variable and type declarations as well as
             * function prototypes.
             */
            print_separate = 1;
            syntax_tree = Trav (syntax_tree, NULL);
            break;
        }
    }

    else {
        outfile = stdout;
        fprintf (outfile, "\n-----------------------------------------------\n");
        syntax_tree = Trav (syntax_tree, NULL);
        fprintf (outfile, "\n-----------------------------------------------\n");
    }

    act_tab = old_tab;

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   void PrintNodeTreeSon( int num, node *node)
 *
 * description:
 *   This function is call from 'PrintNodeTree' only.
 *   Prints a son or attribute containing a hole sub-tree.
 *
 ******************************************************************************/

void
PrintNodeTreeSon (int num, node *node)
{
    int j;

    DBUG_ENTER ("PrintNodeTreeSon");

    for (j = 0; j < indent; j++) {
        if (j % 4) {
            fprintf (outfile, "  ");
        } else {
            fprintf (outfile, "| ");
        }
    }

    if (num >= 0) {
        fprintf (outfile, "%i-", num);
    } else {
        fprintf (outfile, "+-");
    }
    PrintNodeTree (node);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintNodeTreeIds( ids *vars)
 *
 * description:
 *   This function is call from 'PrintNodeTree' only.
 *   Prints a 'ids'-chain.
 *
 ******************************************************************************/

void
PrintNodeTreeIds (ids *vars)
{
    DBUG_ENTER ("PrintNodeTreeIds");

    fprintf (outfile, "( ");
    while (vars != NULL) {
        fprintf (outfile, "%s:%d ", IDS_NAME (vars), IDS_REFCNT (vars));
        vars = IDS_NEXT (vars);
    }
    fprintf (outfile, ")");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintNodeTree( node *node)
 *
 * description:
 *   this function is for debug assistance.
 *   It prints the syntax tree without any interpretation.
 *   Some attribues of interest are printed inside of parenthesizes behind
 *   the node name.
 *
 ******************************************************************************/

void
PrintNodeTree (node *node)
{
    int i, d;

    DBUG_ENTER ("PrintNodeTree");

    outfile = stdout;

    if (node) {
        /* print node name */
        fprintf (outfile, "%s  ", mdb_nodetype[NODE_TYPE (node)]);

        /* print additional information to nodes */
        switch (NODE_TYPE (node)) {
        case N_pragma:
            if (PRAGMA_WLCOMP_APS (node) != NULL) {
                fprintf (outfile, "(wlcomp)\n");
                indent++;
                PrintNodeTreeSon (0, PRAGMA_WLCOMP_APS (node));
                indent--;
            } else {
                fprintf (outfile, "\n");
            }
            break;
        case N_let:
            PrintNodeTreeIds (LET_IDS (node));
            fprintf (outfile, "\n");
            break;
        case N_id:
            fprintf (outfile, "(%s:%d)\n", ID_NAME (node), ID_REFCNT (node));
            break;
        case N_num:
            fprintf (outfile, "(%i)\n", NUM_VAL (node));
            break;
        case N_prf:
            fprintf (outfile, "(%s)\n", mdb_prf[PRF_PRF (node)]);
            break;
        case N_ap:
            fprintf (outfile, "(%s)\n", AP_NAME (node));
            break;
        case N_arg:
            fprintf (outfile, "(%s %s:%d)\n", mdb_type[TYPES_BASETYPE (ARG_TYPE (node))],
                     (ARG_NAME (node) != NULL) ? ARG_NAME (node) : "?",
                     ARG_REFCNT (node));
            break;
        case N_vardec:
            fprintf (outfile, "(%s %s:%d)\n",
                     mdb_type[TYPES_BASETYPE (VARDEC_TYPE (node))], VARDEC_NAME (node),
                     VARDEC_REFCNT (node));
            break;
        case N_fundef:
            fprintf (outfile, "(%s)\n", FUNDEF_NAME (node));
            break;
        case N_Nwith:
            fprintf (outfile, "\n");
            if (NWITH_PRAGMA (node) != NULL) {
                indent++;
                PrintNodeTreeSon (-1, NWITH_PRAGMA (node));
                indent--;
            }
            break;
        case N_Nwithid:
            fprintf (outfile, "( ");
            if (NWITHID_VEC (node) != NULL) {
                fprintf (outfile, "%s:%d", IDS_NAME (NWITHID_VEC (node)),
                         IDS_REFCNT (NWITHID_VEC (node)));
            } else {
                fprintf (outfile, "?");
            }
            fprintf (outfile, " = ");
            if (NWITHID_IDS (node) != NULL) {
                PrintNodeTreeIds (NWITHID_IDS (node));
            } else {
                fprintf (outfile, "?");
            }
            fprintf (outfile, " )\n");
            break;
        case N_Nwithop:
            fprintf (outfile, "( ");
            if (NWITHOP_FUNDEF (node) != NULL) {
                fprintf (outfile, "%s", FUNDEF_NAME (NWITHOP_FUNDEF (node)));
            }
            fprintf (outfile, " )\n");
            break;
        case N_Npart:
            if (NPART_CODE (node) != NULL) {
                fprintf (outfile, "(code used: 0x%p)\n", NPART_CODE (node));
            } else {
                fprintf (outfile, "(no code)\n");
            }
            break;
        case N_Ncode:
            fprintf (outfile, "(adr: 0x%p, used: %d)\n", node, NCODE_USED (node));
            break;
        case N_WLseg:
            fprintf (outfile, "(sv: [ ");
            for (d = 0; d < WLSEG_DIMS (node); d++) {
                fprintf (outfile, "%li ", (WLSEG_SV (node))[d]);
            }
            for (i = 0; i < WLSEG_BLOCKS (node); i++) {
                fprintf (outfile, "], bv%i: [ ", i);
                for (d = 0; d < WLSEG_DIMS (node); d++) {
                    fprintf (outfile, "%li ", (WLSEG_BV (node, i))[d]);
                }
            }
            fprintf (outfile, "], ubv: [ ");
            for (d = 0; d < WLSEG_DIMS (node); d++) {
                fprintf (outfile, "%li ", (WLSEG_UBV (node))[d]);
            }
            fprintf (outfile, "])\n");
            break;
        case N_WLsegVar:
            fprintf (outfile, "(sv: [ ");
            for (d = 0; d < WLSEGVAR_DIMS (node); d++) {
                fprintf (outfile, "%li ", (WLSEGVAR_SV (node))[d]);
            }
            for (i = 0; i < WLSEGVAR_BLOCKS (node); i++) {
                fprintf (outfile, "], bv%i: [ ", i);
                for (d = 0; d < WLSEGVAR_DIMS (node); d++) {
                    fprintf (outfile, "%li ", (WLSEGVAR_BV (node, i))[d]);
                }
            }
            fprintf (outfile, "], ubv: [ ");
            for (d = 0; d < WLSEGVAR_DIMS (node); d++) {
                fprintf (outfile, "%li ", (WLSEGVAR_UBV (node))[d]);
            }
            fprintf (outfile, "])\n");
            break;
        case N_WLblock:
            fprintf (outfile, "(%d->%d block%d[%d] %d)\n", WLBLOCK_BOUND1 (node),
                     WLBLOCK_BOUND2 (node), WLBLOCK_LEVEL (node), WLBLOCK_DIM (node),
                     WLBLOCK_STEP (node));
            break;
        case N_WLublock:
            fprintf (outfile, "(%d->%d ublock%d[%d] %d)\n", WLUBLOCK_BOUND1 (node),
                     WLUBLOCK_BOUND2 (node), WLUBLOCK_LEVEL (node), WLUBLOCK_DIM (node),
                     WLUBLOCK_STEP (node));
            break;
        case N_WLstride:
            fprintf (outfile, "(%d->%d step%d[%d] %d)\n", WLSTRIDE_BOUND1 (node),
                     WLSTRIDE_BOUND2 (node), WLSTRIDE_LEVEL (node), WLSTRIDE_DIM (node),
                     WLSTRIDE_STEP (node));
            break;
        case N_WLgrid:
            fprintf (outfile, "(%d->%d [%d])", WLUBLOCK_BOUND1 (node),
                     WLUBLOCK_BOUND2 (node), WLUBLOCK_DIM (node));
            if (WLGRID_CODE (node) != NULL) {
                fprintf (outfile, ": op\n");
            } else {
                fprintf (outfile, ": ..\n");
            }
            break;
        case N_WLstriVar:
            fprintf (outfile, "(");
            PrintWLvar (WLSTRIVAR_BOUND1 (node), WLSTRIVAR_DIM (node));
            fprintf (outfile, "->");
            PrintWLvar (WLSTRIVAR_BOUND2 (node), WLSTRIVAR_DIM (node));
            fprintf (outfile, ", step%d[%d] ", WLSTRIVAR_LEVEL (node),
                     WLSTRIVAR_DIM (node));
            PrintWLvar (WLSTRIVAR_STEP (node), WLSTRIVAR_DIM (node));
            fprintf (outfile, ")\n");
            break;
        case N_WLgridVar:
            fprintf (outfile, "(");
            PrintWLvar (WLGRIDVAR_BOUND1 (node), WLGRIDVAR_DIM (node));
            fprintf (outfile, "->");
            PrintWLvar (WLGRIDVAR_BOUND2 (node), WLGRIDVAR_DIM (node));
            fprintf (outfile, " [%d])\n", WLGRIDVAR_DIM (node));
            break;
        case N_icm:
            fprintf (outfile, "(%s)\n", ICM_NAME (node));
            break;
        default:
            fprintf (outfile, "\n");
        }

        indent++;
        for (i = 0; i < nnode[NODE_TYPE (node)]; i++)
            if (node->node[i]) {
                PrintNodeTreeSon (i, node->node[i]);
            }
        indent--;
    } else
        fprintf (outfile, "NULL\n");

    DBUG_VOID_RETURN;
}
