/*
 *
 * $Log$
 * Revision 1.3  1994/12/05 15:08:48  hw
 * changed output of function Error
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#include "Error.h"

void
Error (char *string, int status)
{
    fprintf (stderr, "\n%s\n", string);
    exit (status);
}

/*
void postmortem(char *s)
{
   fprintf(stderr,"\n\nPostMortem: %s \n",s);
   exit(99);
}
*/
