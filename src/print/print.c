/*
 *
 * $Log$
 * Revision 1.66  1995/05/30 07:04:37  hw
 * changed PrintFold , because N_foldfun has now two child nodes
 *
 * Revision 1.65  1995/05/29  09:47:42  sbs
 * braces around cond's predicates inserted.
 *
 * Revision 1.64  1995/05/24  15:25:15  sbs
 * trace.h included
 *
 * Revision 1.63  1995/05/22  18:08:19  sbs
 * __trace_buffer inswerted
 *
 * Revision 1.62  1995/05/22  15:51:12  sbs
 * __trace_mem_cnt inserted
 *
 * Revision 1.61  1995/05/22  15:09:02  sbs
 * TRACE_MEM included
 *
 * Revision 1.60  1995/05/17  14:49:27  hw
 * changed PrintBool (TRUE => true; FALSE =>false )
 *
 * Revision 1.59  1995/05/04  11:41:20  sbs
 * ICM-macros adjusted to trf's
 *
 * Revision 1.58  1995/04/28  15:25:20  hw
 * changed PrintGenator ( refcount of index_vector will be shown )
 *
 * Revision 1.57  1995/04/15  15:11:44  asi
 * debug option LINE added, for linenumber output
 *
 * Revision 1.56  1995/04/11  15:08:50  hw
 * changed PrintFundef
 *
 * Revision 1.55  1995/04/11  11:34:45  asi
 * added 'flag' to struct 'node'
 *
 * Revision 1.54  1995/04/05  07:39:43  hw
 * extended PrintIcm
 *
 * Revision 1.53  1995/04/04  11:33:49  sbs
 * ";" at the end of ICM-Assigns eleminated; include inserted
 *
 * Revision 1.52  1995/04/04  09:34:26  sbs
 * parameter to ICM_END macro inserted
 *
 * Revision 1.51  1995/04/03  14:02:56  sbs
 * show_icm inserted
 *
 * Revision 1.50  1995/03/31  15:45:41  hw
 * changed PrintAssign
 *
 * Revision 1.49  1995/03/29  12:01:34  hw
 * PrintIcm added
 * changed PrintAssign (to use it with N_icm)
 *
 * Revision 1.48  1995/03/16  17:46:20  asi
 * output for arguments and variable numbers changed
 *
 * Revision 1.47  1995/03/16  17:22:19  asi
 * Output for Used Variables (gen-,modarray) modified
 *
 * Revision 1.46  1995/03/15  14:14:25  asi
 * output for masks modified
 *
 * Revision 1.45  1995/03/14  15:46:01  asi
 * printing of basic block numbers
 *
 * Revision 1.44  1995/03/14  10:54:29  hw
 * changed PrintId
 *
 * Revision 1.43  1995/03/13  16:54:00  asi
 * changed PrintPost and PrintPre
 *
 * Revision 1.42  1995/03/13  16:44:07  asi
 * changed PrintId
 *
 * Revision 1.41  1995/03/10  13:10:01  hw
 * - changed PrintId , PrintIds ( now refcounts can be printed)
 *
 * Revision 1.40  1995/03/08  14:40:10  sbs
 * include "tree.h" moved from tree.c to tree.h
 *
 * Revision 1.39  1995/03/08  14:04:01  sbs
 * INDENT & indent exported!
 *
 * Revision 1.38  1995/03/08  14:02:20  hw
 * changed PrintPrf
 *
 * Revision 1.37  1995/03/03  17:22:29  asi
 * debug-output for Generator added
 *
 * Revision 1.36  1995/03/01  16:29:50  hw
 * changed PrintGenerator ( name of index-vector is in info->ids)
 *
 * Revision 1.35  1995/03/01  16:04:41  asi
 * debug-output for with loops added
 *
 * Revision 1.34  1995/02/28  18:26:12  asi
 * added argument to functioncall PrintMask
 * added function PrintMasks
 *
 * Revision 1.33  1995/02/14  12:22:37  sbs
 * PrintFold inserted
 *
 * Revision 1.32  1995/02/14  10:12:05  sbs
 * superfluous "i"-declaration in PrintPrf removed
 *
 * Revision 1.31  1995/02/02  14:56:39  hw
 * changed PrintPrf, because N_prf has been changed
 *
 * Revision 1.30  1995/01/16  17:26:38  asi
 * Added PrintMask to PrintDo and PrintWhile
 *
 * Revision 1.29  1995/01/16  11:00:29  asi
 * decleration of print_tab removed, no longer needed
 *
 * Revision 1.28  1995/01/12  13:16:34  asi
 * DBUG_PRINT("MASK",... for optimizing routines degugging inserted
 *
 * Revision 1.27  1995/01/06  14:57:10  asi
 * changed OPTP to MASK and spezial Variable output added
 *
 * Revision 1.26  1995/01/05  15:28:00  asi
 * DBUG_PRINT( MASK,... added - defined and used variables
 * will be printed
 *
 * Revision 1.25  1995/01/05  11:51:25  sbs
 * MOD_NAME_CON macro inserted for mod-name generation for
 * types and functions.
 *
 * Revision 1.24  1995/01/02  19:45:20  sbs
 * PrintTypedef extended for types->id_mod!
 *
 * Revision 1.23  1994/12/31  14:07:01  sbs
 * types->id_mod prepanded at function declarations.
 * This is needed for external functions only!
 *
 * Revision 1.22  1994/12/31  13:54:17  sbs
 * id_mod for N_ap inserted
 *
 * Revision 1.21  1994/12/21  13:25:39  sbs
 * extern declaration for functions inserted
 *
 * Revision 1.20  1994/12/21  13:18:42  sbs
 * PrintFundef changed (works for empty function bodies too
 *
 * Revision 1.19  1994/12/15  17:14:03  sbs
 * PrintCast inserted
 *
 * Revision 1.18  1994/12/14  10:18:39  sbs
 * PrintModul & PrintImportlist & PrintTypedef inserted
 *
 * Revision 1.17  1994/12/08  14:23:41  hw
 * changed PrintAp & PrintFundef
 *
 * Revision 1.16  1994/12/05  11:17:43  hw
 * moved function Type2String to convert.c
 *
 * Revision 1.15  1994/12/02  11:11:55  hw
 * deleted declaration of char *type_string[], because it is defined in
 * my_debug.c now.
 *
 * Revision 1.14  1994/11/22  13:27:27  sbs
 * Typo in return value (previously void *) of PrintIds corrected
 *
 * Revision 1.13  1994/11/22  11:45:48  hw
 * - deleted PrintFor
 * - changed Type2string
 *
 * Revision 1.12  1994/11/17  16:49:03  hw
 * added ";" to output in PrintPost, PrintPre
 *
 * Revision 1.11  1994/11/15  10:06:15  sbs
 * typo for modarray eliminated
 *
 * Revision 1.10  1994/11/14  16:57:44  hw
 * make nicer output
 *
 * Revision 1.9  1994/11/11  16:43:07  hw
 * embellish output
 *
 * Revision 1.8  1994/11/11  13:55:56  hw
 * added new Print-functions: PrintInc PrintDec PrintPost PrintPre
 * embellish output
 *
 * Revision 1.7  1994/11/10  16:59:34  hw
 * added makro INDENT to have a nice output
 *
 * Revision 1.6  1994/11/10  15:34:26  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "optimize.h"
#include "trace.h"

extern show_refcnt;   /* imported from main.c */
extern show_icm;      /* imported from main.c */
extern FILE *outfile; /* outputfile for PrintTree defined in main.c*/

int indent = 0;

/* First, we generate the external declarations for all functions that
 * expand ICMs to C.
 */

#define ICM_ALL
#define ICM_DEF(prf, trf) extern void Print##prf (node *ex);
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_END(prf)
#include "icm.data"
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
#undef ICM_ALL

#define PRF_IF(n, s, x) x

char *prf_string[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

/*
 * prints ids-information to outfile
 *
 */
void
PrintIds (ids *ids)
{
    DBUG_ENTER ("PrintIds");

    do {
        DBUG_PRINT ("PRINT", ("%s", ids->id));

        if (NULL != ids->next)
            if ((-1 == ids->refcnt) || (0 == show_refcnt))
                fprintf (outfile, "%s, ", ids->id);
            else
                fprintf (outfile, "%s:%d, ", ids->id, ids->refcnt);
        else if ((-1 == ids->refcnt) || (0 == show_refcnt))
            fprintf (outfile, "%s ", ids->id);
        else
            fprintf (outfile, "%s:%d ", ids->id, ids->refcnt);
        ids = ids->next;
    } while (NULL != ids);

    DBUG_VOID_RETURN;
}

node *
PrintAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAssign");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    DBUG_EXECUTE ("BBLOCK", fprintf (outfile, "[%d]", arg_node->bblock););

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - assign : %s\n",
                                   mdb_nodetype[arg_node->node[0]->nodetype]);
                  PrintMasks (arg_node, arg_info););

    DBUG_PRINT ("FLAG", ("\nflag = %d", arg_node->flag));

    if (N_icm == arg_node->node[0]->nodetype) {
        PrintIcm (arg_node->node[0], arg_info);
        fprintf (outfile, "\n");
        if (2 == arg_node->nnode)
            Trav (arg_node->node[1], arg_info);
    } else {
        INDENT;
        DBUG_EXECUTE ("LINE", fprintf (outfile, "[%d]", arg_node->lineno););
        Trav (arg_node->node[0], arg_info);
        if (2 == arg_node->nnode) {
            fprintf (outfile, "\n");
            Trav (arg_node->node[1], arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
PrintBlock (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("PrintBlock");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    INDENT;
    fprintf (outfile, "{ \n");
    indent++;
    for (i = arg_node->nnode - 1; 0 <= i; i--)
        Trav (arg_node->node[i], arg_info);
    indent--;
    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_RETURN (arg_node);
}

node *
PrintLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintLet");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    PrintIds (arg_node->info.ids);
    fprintf (outfile, "= ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, "; ");

    DBUG_RETURN (arg_node);
}

node *
PrintModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintModul");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    if (arg_node->info.id != NULL)
        fprintf (outfile, "Modul %s\n", arg_node->info.id);
    if (NULL != arg_node->node[0]) {
        fprintf (outfile, "\n");
        Trav (arg_node->node[0], arg_info); /* print import-list */
    }
    if (NULL != arg_node->node[1]) {
        fprintf (outfile, "\n");
        Trav (arg_node->node[1], arg_info); /* print typedefs */
    }

    Trav (arg_node->node[2], arg_info); /* traverse functions */

    DBUG_RETURN (arg_node);
}

node *
PrintImplist (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintImplist");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    fprintf (outfile, "import %s: ", arg_node->info.id);

    if ((arg_node->node[1] == NULL) && (arg_node->node[1] == NULL)
        && (arg_node->node[1] == NULL))
        fprintf (outfile, "all\n");
    else {
        fprintf (outfile, "{");
        if (arg_node->node[1] != NULL) {
            fprintf (outfile, "\n  implicit types: ");
            PrintIds ((ids *)arg_node->node[1]); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (arg_node->node[2] != NULL) {
            fprintf (outfile, "\n  explicit types: ");
            PrintIds ((ids *)arg_node->node[2]); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (arg_node->node[3] != NULL) {
            fprintf (outfile, "\n  funs: ");
            PrintIds ((ids *)arg_node->node[3]); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        fprintf (outfile, "\n}\n");
    }
    if (1 == arg_node->nnode)
        Trav (arg_node->node[0], arg_info); /* print further imports */

    DBUG_RETURN (arg_node);
}

node *
PrintTypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintTypedef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    fprintf (outfile, "typedef %s ", Type2String (arg_node->info.types, 0));
    if (arg_node->info.types->id_mod != NULL)
        fprintf (outfile, "%s" MOD_NAME_CON, arg_node->info.types->id_mod);
    fprintf (outfile, "%s;\n", arg_node->info.types->id);

    if (1 == arg_node->nnode)
        Trav (arg_node->node[0], arg_info); /* traverse next typedef/fundef */

    DBUG_RETURN (arg_node);
}

node *
PrintFundef (node *arg_node, node *arg_info)
{
    int print_icm = 0;

    DBUG_ENTER ("PrintFundef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));
    arg_info = MakeNode (N_info);
    VARNO = arg_node->varno;
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - function\n");
                  PrintMasks (arg_node, arg_info););

    fprintf (outfile, "\n");
    if (NULL != arg_node->node[3])
        /* after typechecking arg_node->node[3] contains a pointer to
         * corresponding N_return. So we have to look wether this node
         * is a N_icm
         */
        if (N_icm == arg_node->node[3]->nodetype)
            print_icm = 1;

    if (arg_node->node[0] == NULL) /* pure fundec! */
        fprintf (outfile, "extern ");
    if (0 == print_icm) {
        fprintf (outfile, "%s ", Type2String (arg_node->info.types, 0));
        if (arg_node->info.types->id_mod != NULL)
            fprintf (outfile, "%s" MOD_NAME_CON, arg_node->info.types->id_mod);
        fprintf (outfile, "%s(", arg_node->info.types->id);
        if (arg_node->node[2] != NULL)
            Trav (arg_node->node[2], arg_info); /* print args of function */
        fprintf (outfile, ")");
    } else
        Trav (arg_node->node[3], arg_info); /* print N_icm ND_FUN_DEC */

    if (arg_node->node[0] == NULL) {
        fprintf (outfile, ";\n");
    } else {
        fprintf (outfile, "\n");
        Trav (arg_node->node[0], arg_info); /* traverse functionbody */
    }

    free (arg_info);

    if (arg_node->node[1] != NULL)
        Trav (arg_node->node[1], arg_info); /* traverse next function */

    DBUG_RETURN (arg_node);
}

node *
PrintPrf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPrf");

    DBUG_PRINT ("PRINT", ("%s (%s)" P_FORMAT, mdb_nodetype[arg_node->nodetype],
                          mdb_prf[arg_node->info.prf], arg_node));

    switch (arg_node->info.prf) {
    case F_take:
    case F_drop:
    case F_psi:
    case F_shape:
    case F_reshape:
    case F_cat:
    case F_dim:
    case F_rotate:
    case F_not: {
        /* primitive functions that are printed as function application */
        fprintf (outfile, "%s( ", prf_string[arg_node->info.prf]);
        Trav (arg_node->node[0], arg_info);
        fprintf (outfile, " )");
        break;
    }
    default: {
        /* primitive functions in infix notation */
        fprintf (outfile, "(");
        Trav (arg_node->node[0]->node[0], arg_info);
        fprintf (outfile, " %s ", prf_string[arg_node->info.prf]);
        Trav (arg_node->node[0]->node[1]->node[0], arg_info);
        fprintf (outfile, ")");
        break;
    }
    }

    DBUG_RETURN (arg_node);
}

node *
PrintId (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintId");

    if ((0 == show_refcnt) || (-1 == arg_node->refcnt))
        fprintf (outfile, "%s", arg_node->info.ids->id);
    else
        fprintf (outfile, "%s:%d", arg_node->info.ids->id, arg_node->refcnt);

    DBUG_RETURN (arg_node);
}

node *
PrintNum (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("PrintNum");

    fprintf (outfile, "%d", arg_node->info.cint);

    DBUG_RETURN (arg_node);
}

node *
PrintFloat (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("PrintFloat");

    fprintf (outfile, "%f", arg_node->info.cfloat);

    DBUG_RETURN (arg_node);
}

node *
PrintBool (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("PrintBool");

    if (0 == arg_node->info.cint)
        fprintf (outfile, "false");
    else
        fprintf (outfile, "true");

    DBUG_RETURN (arg_node);
}

node *
PrintReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintReturn");

    fprintf (outfile, "return( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " );");

    DBUG_RETURN (arg_node);
}

node *
PrintAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAp");

    if (arg_node->info.fun_name.id_mod != NULL)
        fprintf (outfile, "%s" MOD_NAME_CON, arg_node->info.fun_name.id_mod);
    fprintf (outfile, "%s(", arg_node->info.fun_name.id);
    if (arg_node->node[0])
        Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

node *
PrintCast (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCast");

    fprintf (outfile, "(: %s) ", Type2String (arg_node->info.types, 0));
    Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintExprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintExprs");

    Trav (arg_node->node[0], arg_info);

    if (2 == arg_node->nnode) {
        fprintf (outfile, ", ");
        Trav (arg_node->node[1], arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PrintArg (node *arg_node, node *info_node)
{
    DBUG_ENTER ("PrintArg");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**Number %d -> ", arg_node->varno););

    fprintf (outfile, "%s", Type2String (arg_node->info.types, 1));

    if (1 == arg_node->nnode) {
        fprintf (outfile, ", ");
        Trav (arg_node->node[0], info_node);
    }

    DBUG_RETURN (arg_node);
}

node *
PrintVardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintVardec");

    INDENT;

    DBUG_EXECUTE ("MASK", fprintf (outfile, "**Number %d -> ", arg_node->varno););

    fprintf (outfile, "%s;\n", Type2String (arg_node->info.types, 1));
    if (1 == arg_node->nnode)
        Trav (arg_node->node[0], arg_info);
    else
        fprintf (outfile, "\n");

    DBUG_RETURN (arg_node);
}

node *
PrintDo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDo");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - do body\n");
                  PrintMasks (arg_node->node[1], arg_info););

    fprintf (outfile, "do\n");
    if (NULL != arg_node->node[1]) {
        indent++;
        Trav (arg_node->node[1], arg_info); /* traverse body of loop */
        indent--;
    }

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  fprintf (outfile, "**Used Variables (do-cnd) : %s\n", text);
                  free (text););

    INDENT;
    fprintf (outfile, "while( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " );\n");

    DBUG_RETURN (arg_node);
}

node *
PrintEmpty (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintEmpty");

    INDENT;
    fprintf (outfile, "\t;\n");

    DBUG_RETURN (arg_node);
}

node *
PrintWhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWhile");

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  fprintf (outfile, "**Used Variables (while-cnd) : %s\n", text);
                  free (text););

    fprintf (outfile, "while( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " )\n");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - while body\n");
                  PrintMasks (arg_node->node[1], arg_info););

    Trav (arg_node->node[1], arg_info); /* traverse body of loop */

    DBUG_RETURN (arg_node);
}

node *
PrintLeton (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAddon");

    switch (arg_node->nodetype) {
    case N_addon:
        fprintf (outfile, " %s += ", arg_node->info.ids->id);
        break;
    case N_subon:
        fprintf (outfile, " %s -= ", arg_node->info.ids->id);
        break;
    case N_mulon:
        fprintf (outfile, " %s *= ", arg_node->info.ids->id);
        break;
    case N_divon:
        fprintf (outfile, " %s /= ", arg_node->info.ids->id);
        break;
    default:
        Error ("wrong nodetype in PrintLeton", 1);
        break;
    }
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

node *
PrintCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCond");

    fprintf (outfile, "if (");
    indent++;

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  fprintf (outfile, "**Used Variables (Cond) : %s\n", text);
                  free (text););

    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ")\n");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - then\n");
                  PrintMasks (arg_node->node[1], arg_info););

    Trav (arg_node->node[1], arg_info);
    fprintf (outfile, "\n");
    indent--;

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - else\n");
                  PrintMasks (arg_node->node[2], arg_info););

    INDENT;
    fprintf (outfile, "else\n");
    indent++;
    Trav (arg_node->node[2], arg_info);
    indent--;

    DBUG_RETURN (arg_node);
}

node *
PrintWith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWith");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - generator\n");
                  PrintMasks (arg_node->node[0], arg_info););
    fprintf (outfile, "with (");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ")\n");

    DBUG_EXECUTE ("MASK", char *text;
                  text = PrintMask (arg_node->node[1]->mask[1], VARNO);
                  fprintf (outfile, "**Used Variables (gen-,modarray) : %s\n", text);
                  free (text););

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - with body\n");
                  PrintMasks (arg_node, arg_info););
    Trav (arg_node->node[1], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintGenator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintGenator");

    Trav (arg_node->node[0], arg_info);
    if ((-1 == arg_node->info.ids->refcnt) || (0 == show_refcnt))
        fprintf (outfile, " <= %s <= ", arg_node->info.ids->id);
    else
        fprintf (outfile, " <= %s:%d <= ", arg_node->info.ids->id,
                 arg_node->info.ids->refcnt);
    Trav (arg_node->node[1], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintConexpr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintConexpr");

    INDENT;

    if (N_genarray == arg_node->nodetype) {
        fprintf (outfile, "genarray( ");
    } else {
        fprintf (outfile, "modarray( ");
    }
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " )\n");
    Trav (arg_node->node[1], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintFold (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintFold");

    INDENT;
    if (N_foldprf == arg_node->nodetype)
        fprintf (outfile, "fold( %s )\n", prf_string[arg_node->info.prf]);
    else {
        if (NULL != arg_node->info.fun_name.id_mod)
            fprintf (outfile, "fold( %s%s%s, ", arg_node->info.fun_name.id_mod,
                     MOD_NAME_CON, arg_node->info.fun_name.id);
        else
            fprintf (outfile, "fold( %s, ", arg_node->info.fun_name.id);
        Trav (arg_node->node[1], arg_info);
        fprintf (outfile, " )\n");
    }

    Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintArray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintArray");

    fprintf (outfile, "[ ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " ]");

    DBUG_RETURN (arg_node);
}

node *
PrintDec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDec");

    fprintf (outfile, "--");

    DBUG_RETURN (arg_node);
}

node *
PrintInc (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintInc");

    fprintf (outfile, "++");

    DBUG_RETURN (arg_node);
}

node *
PrintPost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPost");

    PrintIds (arg_node->info.ids);
    fprintf (outfile, "%s", arg_node->info.id);
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

node *
PrintPre (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPre");

    Trav (arg_node->node[0], arg_info);
    PrintIds (arg_node->info.ids);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

node *
PrintIcm (node *arg_node, node *arg_info)
{
    int compiled_icm = 0;
    DBUG_ENTER ("PrintIcm");
    DBUG_PRINT ("PRINT", ("icm-node %s\n", arg_node->info.fun_name.id));

    if (show_icm == 0)
#define ICM_ALL
#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (arg_node->info.fun_name.id, #prf) == 0) {                                \
        Print##prf (arg_node->node[0]);                                                  \
        compiled_icm = 1;                                                                \
    } else
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_END(prf)
#include "icm.data"
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
#undef ICM_ALL
        if (strcmp (arg_node->info.fun_name.id, "NOOP") == 0)
            compiled_icm = 1;

    if ((show_icm == 1) || (compiled_icm == 0)) {
        INDENT;
        fprintf (outfile, "%s(", arg_node->info.fun_name.id);
        if (NULL != arg_node->node[0])
            Trav (arg_node->node[0], arg_info);
        fprintf (outfile, ")");
    }

    if (NULL != arg_node->node[1]) {
        if ((1 == show_icm) || (0 == compiled_icm))
            fprintf (outfile, ", ");
        Trav (arg_node->node[1], arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
Print (node *arg_node)
{

    DBUG_ENTER ("Print");

    act_tab = print_tab;
    if (show_icm == 0) {
        if (traceflag != 0) {
            fprintf (outfile, "#include <stdio.h>\n");
            fprintf (outfile, "\nchar __trace_buffer[128];\n\n");
            if (traceflag & TRACE_MEM) {
                fprintf (outfile, "#define TRACE_MEM\n");
                fprintf (outfile, "\nint __trace_mem_cnt=0;\n\n");
            }
            if (traceflag & TRACE_REF)
                fprintf (outfile, "#define TRACE_REF\n");
        }
        fprintf (outfile, "#include \"icm2c.h\"\n");
    }

    DBUG_RETURN (Trav (arg_node, NULL));
}
