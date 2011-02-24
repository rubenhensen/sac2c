#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_ia.h"

#include "dbug.h"
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

    DBUG_ENTER ("ICMCompileND_CREATE__IRREGULAR__ARRAY__DATA");

    if (val_size > 0) {
        BLOCK_VARDECS (fprintf (global.outfile, "int SAC_j, SAC_i = 0;");
                       , for (i = 0; i < val_size; i++) {
                           FOR_LOOP_INC (fprintf (global.outfile, "SAC_j");
                                         , fprintf (global.outfile, "0");
                                         , fprintf (global.outfile, "SAC_ND_A_SIZE( %s)",
                                                    vals_ANY[i]);
                                         , INDENT;
                                         fprintf (global.outfile,
                                                  "SAC_ND_WRITE_READ_COPY("
                                                  " %s, SAC_i, %s, SAC_j, %s)\n",
                                                  to_NT, vals_ANY[i], copyfun);
                                         INDENT; fprintf (global.outfile, "SAC_i++;\n"););
                       });
    }

    for (i = 0; i < val_size; i++) {
        fprintf (global.outfile, "%s\n", vals_ANY[i]);
    }
    fprintf (global.outfile, "%d\n", to_sdim);
    fprintf (global.outfile, "%s\n", to_NT);
    fprintf (global.outfile, "%s\n", copyfun);

    // DBUG_ASSERT( FALSE, "O YES!");
    fprintf (global.outfile, "FOOBAR!!!!!!!!!!");

    DBUG_VOID_RETURN;
}
