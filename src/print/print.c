/*
 *
 * $Log$
 * Revision 1.18  1994/12/14 10:18:39  sbs
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

#define INDENT                                                                           \
    {                                                                                    \
        int j;                                                                           \
        for (j = 0; j < indent; j++)                                                     \
            fprintf (outfile, "  ");                                                     \
    }

extern FILE *outfile; /* outputfile for PrintTree defined in main.c*/
extern funptr print_tab[];

static int indent = 0;

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

    fprintf (outfile, "typedef %s %s;\n", Type2String (arg_node->info.types, 0),
             arg_node->info.types->id);

    if (1 == arg_node->nnode)
        Trav (arg_node->node[0], arg_info); /* traverse next typedef/fundef */

    DBUG_RETURN (arg_node);
}

node *
PrintFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintFundef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    fprintf (outfile, "\n%s %s", Type2String (arg_node->info.types, 0),
             arg_node->info.types->id);
    fprintf (outfile, "(");
    if (3 == arg_node->nnode)
        Trav (arg_node->node[2], arg_info); /* print args of function */
    fprintf (outfile, ")\n");

    Trav (arg_node->node[0], arg_info); /* traverse functionbody */

    if (2 <= arg_node->nnode)
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
        Trav (arg_node->node[0], arg_info);
        fprintf (outfile, " %s ", prf_string[arg_node->info.prf]);
        Trav (arg_node->node[1], arg_info);
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
        int i = 0;

        fprintf (outfile, "%s( ", prf_string[arg_node->info.prf]);
        for (i = 0; i < arg_node->nnode; i++) {
            Trav (arg_node->node[i], arg_info);
            if ((arg_node->nnode - 1) != i)
                fprintf (outfile, ", ");
        }

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

    fprintf (outfile, "%s(", arg_node->info.id);
    if (arg_node->node[0])
        Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ")");

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
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, "\n");
    Trav (arg_node->node[1], arg_info);
    fprintf (outfile, "\n");
    indent--;
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
