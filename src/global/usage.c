#include "usage.h"
#include <stdio.h>
#include "dbug.h"

void
usage (char *prg_name)
{
    DBUG_ENTER ("usage");

    printf ("\nusage: %s [options] [filename]\n\n", prg_name);
    printf ("options: -h\t\t\thelptext\n");
    printf ("\t -#string\t\toptions (string) for DBUG information\n");
    printf ("\t -o outfilename\t\tset output to outfilename\n");
    printf ("\t -p\t\toutput infile\n");

    DBUG_VOID_RETURN;
}
