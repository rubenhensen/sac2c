/*
 *
 * $Log$
 * Revision 1.6  1994/11/10 15:34:26  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>

#include "tree.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "print.h"

#define TYPE_LENGTH 80      /* dimension of array of char */
#define FUN_HEAD_LENGTH 120 /* dimension of array of char */
#define INT_STRING_LENGTH 5 /* dimension of array of char */

extern FILE *outfile; /* outputfile for PrintTree defined in main.c*/
extern funptr print_tab[];

char *type_string[] = {"int", "float", "bool"};

#define PRF_IF(n, s, x) x

char *prf_string[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

/*
 *  Umwandlung der Information eines Typ-Deskriptors
 *  in einen String
 */
char *
Type2String (types *type, int print_id)
{
    char *tmp_string;

    DBUG_ENTER ("Type2String");

    tmp_string = (char *)malloc (sizeof (char) * TYPE_LENGTH);
    tmp_string[0] = '\0';

    do {
        if (0 == type->dim)
            strcat (tmp_string, type_string[type->simpletype]);
        else {
            int i;
            static char int_string[INT_STRING_LENGTH];

            strcat (tmp_string, type_string[type->simpletype]);
            strcat (tmp_string, "[ ");
            for (i = 0; i < type->dim; i++)
                if (i != (type->dim - 1)) {
                    DBUG_PRINT ("PRINT", ("shp[%d]=%d", i, type->shpseg->shp[i]));
                    sprintf (int_string, "%d, ", type->shpseg->shp[i]);
                    strcat (tmp_string, int_string);
                } else {
                    DBUG_PRINT ("PRINT", ("shp[%d]=%d", i, type->shpseg->shp[i]));
                    sprintf (int_string, "%d ] ", type->shpseg->shp[i]);
                    strcat (tmp_string, int_string);
                }
        }
        if ((NULL != type->id) && print_id) {
            strcat (tmp_string, " ");
            strcat (tmp_string, type->id);
        }

        type = type->next;
        if (NULL != type)
            strcat (tmp_string, ", ");
    } while (NULL != type);

    DBUG_RETURN (tmp_string);
}

/*
 * prints ids-information to outfile
 *
 */
void *
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
        if (1 == i)
            fprintf (outfile, "\n");
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PrintBlock (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("PrintBlock");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    fprintf (outfile, "{ \n");
    for (i = arg_node->nnode - 1; 0 <= i; i--)
        Trav (arg_node->node[i], arg_info);
    fprintf (outfile, "\n} \n");

    DBUG_RETURN (arg_node);
}

node *
PrintLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintLet");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    PrintIds (arg_node->info.ids);
    fprintf (outfile, " = ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, "; ");

    DBUG_RETURN (arg_node);
}

node *
PrintFundef (node *arg_node, node *arg_info)
{
    char fun_string[FUN_HEAD_LENGTH];
    int i;

    DBUG_ENTER ("PrintFundef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    fprintf (outfile, "%s %s", Type2String (arg_node->info.types, 0),
             arg_node->info.types->id);
    fprintf (outfile, "( ");
    if (2 <= arg_node->nnode)
        Trav (arg_node->node[1], arg_info); /* print args of function */
    fprintf (outfile, " )\n");

    Trav (arg_node->node[0], arg_info); /* traverse functionbody */

    if (3 == arg_node->nnode)
        Trav (arg_node->node[2], arg_info); /* traverse next function */

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
        fprintf (outfile, "( ");
        Trav (arg_node->node[0], arg_info);
        fprintf (outfile, " %s ", prf_string[arg_node->info.prf]);
        Trav (arg_node->node[1], arg_info);
        fprintf (outfile, " )");
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

        fprintf (outfile, " %s( ", prf_string[arg_node->info.prf]);
        for (i = 0; i < arg_node->nnode; i++) {
            Trav (arg_node->node[i], arg_info);
            if ((arg_node->nnode - 1) != i)
                fprintf (outfile, ", ");
        }

        fprintf (outfile, " ) ");
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

    fprintf (outfile, "%s", arg_node->info.cint);

    DBUG_RETURN (arg_node);
}

node *
PrintReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintReturn");

    fprintf (outfile, "return( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " );\n");

    DBUG_RETURN (arg_node);
}

node *
PrintAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAp");

    fprintf (outfile, "%s( ", arg_node->info.id);
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " )");

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

    fprintf (outfile, "%s;\n", Type2String (arg_node->info.types, 1));
    if (1 == arg_node->nnode)
        Trav (arg_node->node[0], arg_info);
    else
        fprintf (outfile, "\n");

    DBUG_RETURN (arg_node);
}

node *
PrintFor (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("PrintFor");

    fprintf (outfile, "for( ");
    for (i = 0; arg_node->nnode > i; i++) {
        Trav (arg_node->node[i], arg_info);
        if (2 == i)
            fprintf (outfile, " )\n");
        else if (1 == i)
            fprintf (outfile, "; ");
    }

    DBUG_RETURN (arg_node);
}

node *
PrintDo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDo");

    fprintf (outfile, "do\n");
    if (NULL != arg_node->node[1])
        Trav (arg_node->node[1], arg_info); /* traverse body of loop */
    fprintf (outfile, "while( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " )\n");

    DBUG_RETURN (arg_node);
}

node *
PrintEmpty (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintEmpty");

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
    }
    Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCond");

    fprintf (outfile, " if ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, "\n\t");
    Trav (arg_node->node[1], arg_info);
    fprintf (outfile, ";\n else\n\t ");
    Trav (arg_node->node[2], arg_info);

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

    if (N_genarray == arg_node->nodetype)
        fprintf (outfile, "genarray( %s )", arg_node->info.id);
    else
        fprintf (outfile, "modaray( %s )", arg_node->info.id);
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
Print (node *arg_node)
{

    DBUG_ENTER ("Print");

    act_tab = print_tab;

    DBUG_RETURN (Trav (arg_node, NULL));
}
