/*
 *
 * $Log$
 * Revision 1.6  1995/10/20 11:29:04  cg
 * DBUG_PRINT removed
 *
 * Revision 1.5  1995/10/18  12:51:58  cg
 * converted to new error macros
 *
 * Revision 1.4  1995/07/24  15:43:52  asi
 * itoa will now work correctly ;-)
 *
 * Revision 1.3  1995/07/24  09:01:48  asi
 * added function itoa
 *
 * Revision 1.2  1995/05/12  13:14:10  hw
 * changed tag of DBUG_PRINT in function Malloc to MEM
 *
 * Revision 1.1  1995/03/28  12:01:50  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <math.h>

#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"

/*
 *
 *  functionname  : Malloc
 *  arguments     :  1) size of memory to allocate
 *  description   : allocates memory, if there is enough
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
        SYSABORT (("Out of memory"));

    /*
     *  DBUG_PRINT("MEM",("new mem: "P_FORMAT,tmp));
     *
     *  DBUG_PRINT in Malloc makes usage of functions like ItemName or ModName
     *  from Error.c in other DBUG_PRINTs impossible.
     */

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

/*
 *
 *  functionname  : itoa
 *  arguments     : 1) number
 *		    R) result string
 *  description   : converts long to string
 *  global vars   :
 *  internal funs : Malloc
 *  external funs : sizeof
 *  macros        : DBUG..., NULL
 *
 *  remarks       :
 *
 */
char *
itoa (long number)
{
    char *str;
    int tmp;
    int length, i;

    DBUG_ENTER ("itoa");
    tmp = number;
    length = 1;
    while (9 < tmp) {
        tmp /= 10;
        length++;
    }
    str = (char *)Malloc (sizeof (char) * length + 1);
    str[length] = atoi ("\0");
    for (i = 0; (i < length); i++) {
        str[i] = ((int)'0') + (number / pow (10, (length - 1)));
        number = number % ((int)pow (10, (length - 1)));
    }
    DBUG_RETURN (str);
}
