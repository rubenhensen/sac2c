/*
 *
 * $Log$
 * Revision 1.1  1998/05/03 14:06:09  dkr
 * Initial revision
 *
 *
 *
 */

#include <malloc.h>
#include <stdio.h>

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "main.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_BEGIN( char *offset, int dims, char **args)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_BEGIN( offset, dims, [ idx_scalar, shape ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_BEGIN (char *array, char *offset, int dims, char **idx_scalar)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_BEGIN");

#define WL_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_BEGIN

    INDENT;
    fprintf (outfile, "{\n");

    indent++;
    INDENT;
    fprintf (outfile, "int %s = 0;\n", offset);

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int __stop_%s = ", idx_scalar[i]);
        AccessShape (array, i);
        fprintf (outfile, ";\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_END( char *offset, int dims, char **args)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_END( offset, dims, [ idx_scalar, shape ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_END (char *array, char *offset, int dims, char **idx_scalar)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_END");

#define WL_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_END

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}
