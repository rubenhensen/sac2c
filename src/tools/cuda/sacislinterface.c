/*
 * Interface between sac2c and isl
 *
 */

// #include <stdint.h> does not work today, so we do it this way.
typedef unsigned int uint32_t;

//#include "isl_map_private.h"
#include <isl/ctx.h>
#include <isl/vec.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <isl/obj.h>
#include <isl/stream.h>
#include <isl/set.h>
#include <isl/map.h>
#include <isl/vertices.h>
#include <isl/flow.h>
#include <isl/band.h>
#include <isl/schedule.h>
#include <isl/ast_build.h>
#include <barvinok/isl.h>
#include <barvinok/options.h>

void
printBasicSet (struct isl_basic_set *pset, char *titl, struct isl_ctx *ctx)
{ // Print a BasicSet to stdout
    isl_printer *p;
    printf ("Printing %s\n", titl);
    p = isl_printer_to_file (ctx, stdout);
    p = isl_printer_print_basic_set (p, pset);
    p = isl_printer_end_line (p);
    isl_printer_free (p);
    printf ("Done printing %s\n", titl);
    return;
}

void
printUnion (struct isl_union_set *pset, char *titl, struct isl_ctx *ctx)
{
    isl_printer *p;
    printf ("Printing %s\n", titl);
    p = isl_printer_to_file (ctx, stdout);
    p = isl_printer_print_union_set (p, pset);
    p = isl_printer_end_line (p);
    isl_printer_free (p);
    printf ("Done printing %s\n", titl);
    return;
}

struct isl_union_set *
doIntersect (struct isl_union_set *unionb, struct isl_union_set *unionc, char *titl,
             struct isl_ctx *ctx)
{ // Intersect two sets and return the resulting set
    struct isl_union_set *intersectbc;

    intersectbc = isl_union_set_intersect (isl_union_set_copy (unionb),
                                           isl_union_set_copy (unionc));
    printUnion (intersectbc, titl, ctx);
    if (isl_union_set_is_empty (intersectbc)) {
        printf ("%s is empty\n", titl);
    } else {
        printf ("%s is not empty\n", titl);
    }
    printUnion (intersectbc, titl, ctx);

    return (intersectbc);
}

isl_union_set *
ReadUnionFromFile (struct _IO_FILE *fd, struct isl_ctx *ctx)
{ // Read one PolyLib-format union from file.
    struct isl_basic_set *bset;
    struct isl_union_set *bunion;

    printf ("Reading union\n");
    bset = isl_basic_set_read_from_file (ctx, fd);
    bunion = isl_union_set_from_basic_set (bset);
    printf ("Done reading union\n");

    return (bunion);
}

int
main (int argc, char **argv)
{
    struct isl_union_set *unionb;
    struct isl_union_set *unionc;
    struct isl_union_set *uniond;
    struct isl_union_set *unione;
    struct isl_union_set *intersectbc;
    struct isl_union_set *intersectbcd;
    struct isl_union_set *intersectbce;
    struct isl_ctx *ctx = isl_ctx_alloc ();

    unionb = ReadUnionFromFile (stdin, ctx);
    unionc = ReadUnionFromFile (stdin, ctx);
    uniond = ReadUnionFromFile (stdin, ctx);
    unione = ReadUnionFromFile (stdin, ctx);
    printUnion (unionb, "unionb", ctx);
    printUnion (unionc, "unionc", ctx);
    printUnion (uniond, "uniond", ctx);
    printUnion (unione, "unione", ctx);

    intersectbc = doIntersect (unionb, unionc, "intersectbc", ctx);
    intersectbcd = doIntersect (intersectbc, uniond, "intersectbcd", ctx);
    intersectbce = doIntersect (intersectbc, unione, "intersectbce", ctx);

    isl_union_set_free (unionb);
    isl_union_set_free (unionc);
    isl_union_set_free (uniond);
    isl_union_set_free (unione);
    isl_union_set_free (intersectbc);
    isl_union_set_free (intersectbcd);
    isl_union_set_free (intersectbce);

    isl_ctx_free (ctx);

    return 0;
}
