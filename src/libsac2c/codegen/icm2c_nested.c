#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_nested.h"
#include "icm2c_std.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"
#include "free.h"
#include "str.h"
#include "memory.h"

void
ICMCompileND_ENCLOSE (char *to_NT, int to_DIM, char *from_NT, int from_DIM)
{
    DBUG_ENTER ();

    for (int i = 0; i < from_DIM; i++) {
        fprintf (global.outfile,
                 "DESC_SHAPE( SAC_ND_A_DESC(%s), %d) = SAC_ND_A_SHAPE( %s, %d);\n",
                 from_NT, i, from_NT, i);
        INDENT;
    }
    /* Assign the size */
    fprintf (global.outfile, "DESC_SIZE( SAC_ND_A_DESC(%s) ) = SAC_ND_A_SIZE( %s);\n",
             from_NT, from_NT);

    INDENT;

    /* Assign the dimension */
    fprintf (global.outfile, "DESC_DIM( SAC_ND_A_DESC(%s) ) = SAC_ND_A_DIM( %s);\n",
             from_NT, from_NT);

    INDENT;

    /* Assign the descriptor */
    fprintf (global.outfile, "SAC_ND_A_DESC( %s ) = SAC_ND_A_DESC( %s);\n", to_NT,
             from_NT);

    INDENT;
    /* Assign the data */
    fprintf (global.outfile, "SAC_ND_A_DATA_NESTED__SCL( %s) = SAC_ND_A_FIELD( %s);\n",
             to_NT, from_NT);

    DBUG_RETURN ();
}

void
ICMCompileND_DISCLOSE (char *to_NT, int to_DIM, char *from_NT, int from_DIM)
{
    int i;

    DBUG_ENTER ();

    fprintf (global.outfile, "SAC_ND_A_DESC( %s) = SAC_ND_A_DESC( %s);\n", to_NT,
             from_NT);

    INDENT;
    fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = SAC_ND_A_DATA_NESTED__SCL( %s);\n",
             to_NT, from_NT);

    if (to_DIM < -2) {
        for (i = 0; i < -(to_DIM + 2); i++) {
            INDENT;
            fprintf (global.outfile,
                     "SAC_ND_A_SHAPE(%s, %d) = DESC_SHAPE( SAC_ND_A_DESC(%s), %d);\n",
                     to_NT, i, to_NT, i);
        }

        INDENT;
        fprintf (global.outfile, "SAC_ND_A_SIZE(%s) = DESC_SIZE( SAC_ND_A_DESC(%s) );\n",
                 to_NT, to_NT);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_DECL( char *var_NT, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_DECL( var_NT, basetype, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileND_DECL_NESTED (char *var_NT, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ();

#define ND_DECL_NESTED
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_DECL_NESTED

    INDENT;
    fprintf (global.outfile, "SAC_ND_DECL_NESTED__DATA( %s, %s, )\n", var_NT, basetype);

    INDENT;
    fprintf (global.outfile, "SAC_ND_DECL_NESTED__DESC( %s, )\n", var_NT);

    ICMCompileND_DECL__MIRROR (var_NT, sdim, shp, 0);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
