#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_ia.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"
#include "free.h"
#include "str.h"
#include "memory.h"

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_CREATE__IRREGULAR__ARRAY__DATA( char *to_NT, int to_sdim,
 *                                                     int val_size, char **vals_ANY,
 *                                                     char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_CREATE__VEC__DATAT( to_NT, to_sdim, val_size, [ vals_ANY ]* , copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_CREATE__IRREGULAR__ARRAY__DATA (char *to_NT, int to_sdim, int val_size,
                                             char **vals_ANY, char *copyfun)
{
    int i;

    DBUG_ENTER ();

#define ND_CREATE__IRREGULAR__ARRAY__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_CREATE__IRREGULAR__ARRAY__DATA

    /* USE:
     *
     * SAC_ND_A_DEC   <-- descriptor
     * SAC_ND_A_FIELD <-- data
     */

    for (i = 0; i < 0 * val_size; i++) {
        fprintf (global.outfile,
                 "SAC_ND_A_DEC("
                 "%s)\n",
                 vals_ANY[i]);
    }

    for (i = 0; i < val_size; i++) {
        fprintf (global.outfile, "%s\n", vals_ANY[i]);
    }
    fprintf (global.outfile, "%d\n", to_sdim);
    fprintf (global.outfile, "%s\n", to_NT);
    fprintf (global.outfile, "%s\n", copyfun);

    // DBUG_UNREACHABLE ("O YES!");
    fprintf (global.outfile, "FOOBAR!!!!!!!!!!");

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
