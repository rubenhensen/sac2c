/*
 *
 * $Log$
 * Revision 1.2  1994/11/10 15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#include "Error.h"

void
Error (char *string, int status)
{
    fprintf (stderr, "%s\n", string);
    exit (status);
}

/*
void postmortem(char *s)
{
   fprintf(stderr,"\n\nPostMortem: %s \n",s);
   exit(99);
}
*/
