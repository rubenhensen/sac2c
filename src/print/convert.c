/*
 *
 * $Log$
 * Revision 1.3  1994/12/19 16:51:48  hw
 * changed TYPE_LENGTH & INT_STRING_LENGTH
 *
 * Revision 1.2  1994/12/14  16:35:39  sbs
 * userdef types integrated
 *
 * Revision 1.1  1994/12/05  13:20:47  hw
 * Initial revision
 *
 *
 *
 */

#include <string.h>
#include <stdlib.h>

#include "tree.h"
#include "dbug.h"
#include "convert.h"

#define TYPE_LENGTH 256      /* dimension of array of char */
#define INT_STRING_LENGTH 16 /* dimension of array of char */

char *type_string[] = {"int", "float", "bool"}; /* strings for primitve types */

/*
 *
 *  functionname  : Type2String
 *  arguments     :  1) pointer to type-structure
 *                   2) flag
 *  description   : convertes the infomation in type to a string
 *                  if flag ==1 then the identifier type->id is also put into
 *                     the resulting string
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcat, malloc, sprintf
 *  macros        : DBUG...
 *
 *  remarks       :
 *
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
            strcat (tmp_string, SIMPLE2STR (type));
        else if (-1 == type->dim) {
            strcat (tmp_string, SIMPLE2STR (type));
            strcat (tmp_string, "[]");
        } else {
            int i;
            static char int_string[INT_STRING_LENGTH];

            strcat (tmp_string, SIMPLE2STR (type));
            strcat (tmp_string, "[ ");
            for (i = 0; i < type->dim; i++)
                if (i != (type->dim - 1)) {
                    DBUG_PRINT ("PRINT", ("shp[%d]=%d", i, type->shpseg->shp[i]));
                    sprintf (int_string, "%d, ", type->shpseg->shp[i]);
                    strcat (tmp_string, int_string);
                } else {
                    DBUG_PRINT ("PRINT", ("shp[%d]=%d", i, type->shpseg->shp[i]));
                    sprintf (int_string, "%d ]", type->shpseg->shp[i]);
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
