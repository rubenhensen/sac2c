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
