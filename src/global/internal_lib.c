/*
 *
 * $Log$
 * Revision 1.1  1995/03/28 12:01:50  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"

/*
 *
 *  functionname  : Malloc
 *  arguments     :  1) size of memory to allocate
 *  description   : allociates memory, if there is enough
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : malloc, Error
 *  macros        : DBUG...,NULL
 *
 *  remarks       : exit if no there is not enough memory
 *
 */
void *
Malloc (int size)
{
    void *tmp;

    DBUG_ENTER ("Malloc");

    tmp = malloc (size);
    if (NULL == tmp)
        Error ("out of memory", 1);
    DBUG_PRINT ("TYPE", ("new mem: " P_FORMAT, tmp));

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : StringCopy
 *  arguments     : 1) source string
 *  description   : allociates memory and returns a pointer to the copy of 1)
 *  global vars   :
 *  internal funs : Malloc
 *  external funs : sizeof
 *  macros        : DBUG..., NULL
 *
 *  remarks       :
 *
 */
char *
StringCopy (char *source)
{
    char *str;
    int length, i;

    DBUG_ENTER ("StringCopy");

    if (NULL != source) {
        for (length = 0; source[length] != '\0'; length++)
            ;
        length++;
        str = (char *)Malloc (sizeof (char) * length);
        for (i = 0; i <= length; i++)
            str[i] = source[i];
    } else
        str = NULL;
    DBUG_RETURN (str);
}
