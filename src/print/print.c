/*
 *
 * $Log$
 * Revision 1.35  1995/03/01 16:04:41  asi
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

#include "tree.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "print.h"
#include "Error.h"
#include "convert.h"
#include "optimize.h"

#define INDENT                                                                           \
    {                                                                                    \
        int j;                                                                           \
        for (j = 0; j < indent; j++)                                                     \
            fprintf (outfile, "  ");                                                     \
    }

extern FILE *outfile; /* outputfile for PrintTree defined in main.c*/

static int indent = 0;

#define PRF_IF(n, s, x) x

char *prf_string[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

/*
 *  Prints all masks
 */

void
PrintMasks (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintMasks");
    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[0], VARNO);
                  DBUG_PRINT ("MASK", ("Def. Variables : %s", text)); free (text););
    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  DBUG_PRINT ("MASK", ("Used Variables : %s", text)); free (text););
    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[2], VARNO);
                  DBUG_PRINT ("MASK", ("Spz. Variables : %s", text)); free (text););
    DBUG_VOID_RETURN;
}

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
            fprintf (outfile, "%s, ", ids->id);
        else
            fprintf (outfile, "%s ", ids->id);
        ids = ids->next;
    } while (NULL != ids);

    DBUG_VOID_RETURN;
}

node *
PrintAssign (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("PrintAssign");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    DBUG_EXECUTE ("MASK", PrintMasks (arg_node, arg_info););

    for (i = 0; i < arg_node->nnode; i++) {
        INDENT;
        if (1 == i)
            fprintf (outfile, "\n");
        Trav (arg_node->node[i], arg_info);
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
    DBUG_ENTER ("PrintFundef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));
    arg_info = MakeNode (N_info);
    VARNO = arg_node->varno;
    DBUG_EXECUTE ("MASK", PrintMasks (arg_node, arg_info););

    fprintf (outfile, "\n");
    if (arg_node->node[0] == NULL) /* pure fundec! */
        fprintf (outfile, "extern ");
    fprintf (outfile, "%s ", Type2String (arg_node->info.types, 0));
    if (arg_node->info.types->id_mod != NULL)
        fprintf (outfile, "%s" MOD_NAME_CON, arg_node->info.types->id_mod);
    fprintf (outfile, "%s(", arg_node->info.types->id);
    if (arg_node->node[2] != NULL)
        Trav (arg_node->node[2], arg_info); /* print args of function */
    fprintf (outfile, ")");

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
    case F_add:
    case F_sub:
    case F_mul:
    case F_div:
    case F_and:
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_ge:
    case F_gt:
    case F_neq: {
        fprintf (outfile, "(");
        Trav (arg_node->node[0]->node[0], arg_info);
        fprintf (outfile, " %s ", prf_string[arg_node->info.prf]);
        Trav (arg_node->node[0]->node[1]->node[0], arg_info);
        fprintf (outfile, ")");
        break;
    }
    case F_take:
    case F_drop:
    case F_psi:
    case F_shape:
    case F_reshape:
    case F_cat:
    case F_dim:
    case F_rotate:
    case F_not: {
        fprintf (outfile, "%s( ", prf_string[arg_node->info.prf]);
        Trav (arg_node->node[0], arg_info);
        fprintf (outfile, " )");
        break;
    }
    }

    DBUG_RETURN (arg_node);
}

node *
PrintId (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintId");

    fprintf (outfile, "%s", arg_node->info.id);

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
        fprintf (outfile, "FALSE");
    else
        fprintf (outfile, "TRUE");

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
    DBUG_PRINT ("MASK", ("Number= : %d", arg_node->varno));

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
    DBUG_PRINT ("MASK", ("Number= : %d", arg_node->varno));

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

    fprintf (outfile, "do\n");
    if (NULL != arg_node->node[1]) {
        indent++;
        Trav (arg_node->node[1], arg_info); /* traverse body of loop */
        indent--;
    }
    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  DBUG_PRINT ("MASK", ("Used Variables (do-cnd) : %s", text));
                  free (text););
    INDENT;
    fprintf (outfile, "while( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " )\n");

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
                  DBUG_PRINT ("MASK", ("Used Variables (while-cnd) : %s", text));
                  free (text););

    fprintf (outfile, "while( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " )\n");
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

    fprintf (outfile, "if ");
    indent++;

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  DBUG_PRINT ("MASK", ("Used Variables (Cond): %s", text)); free (text););

    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, "\n");

    DBUG_EXECUTE ("MASK", PrintMasks (arg_node->node[1], arg_info););

    Trav (arg_node->node[1], arg_info);
    fprintf (outfile, "\n");
    indent--;

    DBUG_EXECUTE ("MASK", PrintMasks (arg_node->node[2], arg_info););

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

    fprintf (outfile, "with (");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ")\n");
    Trav (arg_node->node[1], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintGenator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintGenator");

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[0], VARNO);
                  DBUG_PRINT ("MASK", ("Bound Variable (generator) : %s", text));
                  free (text););

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  DBUG_PRINT ("MASK", ("Used Variables (generator) : %s", text));
                  free (text););

    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " <= %s <= ", arg_node->info.id);
    Trav (arg_node->node[1], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintConexpr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintConexpr");

    INDENT;
    if (N_genarray == arg_node->nodetype)
        fprintf (outfile, "genarray( ");
    else
        fprintf (outfile, "modarray( ");
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
    else
        fprintf (outfile, "fold( %s )\n", arg_node->info.id);
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
    fprintf (outfile, "%s;", arg_node->info.id);

    DBUG_RETURN (arg_node);
}

node *
Print (node *arg_node)
{

    DBUG_ENTER ("Print");

    act_tab = print_tab;

    DBUG_RETURN (Trav (arg_node, NULL));
}
