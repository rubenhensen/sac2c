/*
 *
 * $Log$
 * Revision 3.5  2002/04/16 21:09:40  dkr
 * dummy for GSCPrintMainEnd() no longer needed
 *
 * Revision 3.4  2001/11/22 10:34:01  sbs
 * string.h included (required on ALPHA)
 *
 * Revision 3.3  2001/11/21 11:05:03  dkr
 * include of icm2c_sched.h added
 *
 * Revision 3.2  2001/11/15 18:13:15  sbs
 * included stdlib.h instead of malloc.h now
 *
 * Revision 3.1  2000/11/20 18:00:59  sacbase
 * new release made
 *
 * Revision 2.6  2000/07/11 09:19:57  dkr
 * include icm2c_utils.h not needed
 *
 * Revision 2.4  1999/06/16 17:06:13  rob
 * Add hash-include "../runtime/sac.h".
 * Add include for icm2c_utils.h
 *
 * Revision 2.3  1999/05/20 07:59:16  cg
 * removed command line argument to enable boundary check
 * because this no longer affects the definition of ICMs.
 *
 * Revision 2.2  1999/05/12 14:39:05  cg
 * NO_TRACE renamed to TRACE_NONE
 *
 * Revision 2.1  1999/02/23 12:42:18  sacbase
 * new release made
 *
 * Revision 1.5  1998/05/19 10:05:21  cg
 * added 'clear-indent !' feature
 *
 * Revision 1.4  1998/05/13 09:26:07  dkr
 * added dummy function to fool the linker
 *
 * Revision 1.3  1998/05/13 07:14:06  cg
 * added icm2c_mt.h
 *
 * Revision 1.2  1998/05/03 13:08:39  dkr
 * added icm2c_wl.h
 *
 * Revision 1.1  1998/04/25 16:18:08  sbs
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbug.h"

#include "globals.h"

#include "icm2c_std.h"
#include "icm2c_wl.h"
#include "icm2c_mt.h"
#include "icm2c_sched.h"

/*
 * Variables needed for the code generated by "icm_betest.c":
 */
node *arg_info = NULL;

#define ICM_ALL
#include "icm_vars.c"
#undef ICM_ALL

/******************************************************************************
 *
 * Function:
 *   int main( int argc, char *argv[])
 *
 * Description:
 *   main function
 *
 ******************************************************************************/

int
main (int argc, char *argv[])
{
    char buffer[1024];
    int scanf_res;
    char c;

    DBUG_ENTER ("main");

    /*
     * preset some globals used for the compilation of ICM's
     */
    outfile = stdout;
    print_objdef_for_header_file = 0;
    indent = 0;

    printf ("#include \"../runtime/sac.h\"\n");
    scanf_res = scanf ("%s", buffer);
    while (scanf_res > 0) {
        if (buffer[strlen (buffer) - 1] == '(') {
            buffer[strlen (buffer) - 1] = '\0';
        }

        DBUG_PRINT ("BEtest", ("icm found: %s\n", buffer));

#define ICM_ALL
#include "icm_betest.c"
#undef ICM_ALL
        if (0 == strcmp (buffer, "clear-indent")) {
            indent = 0;
        } else {
            printf ("icm %s not defined!\n", buffer);

            /*
             * Now, we have to consume the rest of the line.
             */
            do {
                c = getchar ();
            } while (c != '\n');
            ungetc ((int)c, stdin);
        }

        scanf ("%s\n", buffer);
        scanf_res = scanf ("%s", buffer);
    }

    DBUG_RETURN (0);
}
