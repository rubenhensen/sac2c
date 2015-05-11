#include <stdio.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"
#include "icm2c_distmem.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"
#include "globals.h"
#include "print.h"
#include "tree_basic.h"
#include "str.h"
#include "memory.h"

#ifndef BEtest
#include "scnprs.h"   /* for big magic access to syntax tree      */
#include "traverse.h" /* for traversal of fold operation function */
#include "compile.h"  /* for GetFoldCode()                        */
#include "free.h"
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   void ICMCompileDISTMEM_DECL( char *var_NT, char *basetype, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   DISTMEM_DECL( var_NT, basetype, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileDISTMEM_DECL (char *var_NT, char *basetype, int sdim, int *shp)
{
    DBUG_ENTER ();

#define DISTMEM_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef DISTMEM_DECL

    indout ("/* TESTTESTTEST DISTMEM*/");
    indout ("SAC_ND_DECL__DATA( %s, %s, )\n", var_NT, basetype);
    indout ("SAC_ND_DECL__DESC( %s, )\n", var_NT);
    ICMCompileDISTMEM_DECL__MIRROR (var_NT, sdim, shp);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileDISTMEM_DECL__MIRROR( char *var_NT, int sdim, int *shp)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   DISTMEM_DECL__MIRROR( var_NT, sdim, [ shp ]* )
 *
 ******************************************************************************/

void
ICMCompileDISTMEM_DECL__MIRROR (char *var_NT, int sdim, int *shp)
{
    int size, i;
    shape_class_t sc = ICUGetShapeClass (var_NT);
    int dim = DIM_NO_OFFSET (sdim);

    DBUG_ENTER ();

#define DISTMEM_DECL__MIRROR
#include "icm_comment.c"
#include "icm_trace.c"
#undef DISTMEM_DECL__MIRROR

    switch (sc) {
    case C_scl:
        indout ("SAC_NOTHING()\n");
        break;

    case C_aks:
        size = 1;
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            indout ("const int SAC_ND_A_MIRROR_SHAPE( %s, %d) = %d;\n", var_NT, i,
                    shp[i]);
            size *= shp[i];
            DBUG_ASSERT (size >= 0, "array with size <0 found!");
        }

        indout ("const int SAC_ND_A_MIRROR_SIZE( %s) = %d;\n", var_NT, size);
        indout ("const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);

        indout ("const bool SAC_DISTMEM_MIRROR_IS_DIST( %s) = "
                "SAC_DISTMEM_DET_DO_DISTR_ARR( %d, %d);\n",
                var_NT, size, shp[0]);

        break;

    case C_akd:
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
        for (i = 0; i < dim; i++) {
            indout ("int SAC_ND_A_MIRROR_SHAPE( %s, %d);\n", var_NT, i);
        }

        indout ("int SAC_ND_A_MIRROR_SIZE( %s);\n", var_NT);
        indout ("const int SAC_ND_A_MIRROR_DIM( %s) = %d;\n", var_NT, dim);
        break;

    case C_aud:
        indout ("int SAC_ND_A_MIRROR_SIZE( %s) = 0;\n", var_NT);
        indout ("int SAC_ND_A_MIRROR_DIM( %s) = 0;\n", var_NT);
        break;

    default:
        DBUG_UNREACHABLE ("Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
