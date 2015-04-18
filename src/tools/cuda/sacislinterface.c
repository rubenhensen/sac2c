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
#include "polyhedral_defs.h"

void
printBasicSet (struct isl_basic_set *pset, char *titl)
{ // Print a BasicSet to stdout

    isl_printer *p;
    struct isl_ctx *ctx;

    printf ("Printing %s\n", titl);
    ctx = isl_basic_set_get_ctx (pset);
    p = isl_printer_to_file (ctx, stdout);
    p = isl_printer_print_basic_set (p, pset);
    p = isl_printer_end_line (p);
    isl_printer_free (p);
    printf ("Done printing %s\n", titl);
    return;
}

void
printUnion (struct _IO_FILE *fd, struct isl_union_set *pset, char *titl, int verbose,
            int fmt)
{
    // Print a union_set. Use fmt as format, if non-zero.
    isl_printer *p;
    struct isl_ctx *ctx;

    if (verbose) {
        ctx = isl_union_set_get_ctx (pset);
        fprintf (fd, "Union %s is:\n", titl);
        p = isl_printer_to_file (ctx, fd);
        if (0 != fmt) {
            p = isl_printer_set_output_format (p, fmt);
        }
        p = isl_printer_print_union_set (p, pset);
        p = isl_printer_end_line (p);
        isl_printer_free (p);
    }
    return;
}

struct isl_union_set *
doIntersect (struct isl_union_set *unionb, struct isl_union_set *unionc, char *titl,
             int verbose)
{ // Intersect two sets and return the resulting set
    struct isl_union_set *intersectbc;

    intersectbc = isl_union_set_intersect (isl_union_set_copy (unionb),
                                           isl_union_set_copy (unionc));
    if (verbose) {
        printUnion (stderr, intersectbc, titl, verbose, 0);
    }

    return (intersectbc);
}

isl_union_set *
ReadUnionFromFile (struct _IO_FILE *fd, struct isl_ctx *ctx)
{ // Read one PolyLib-format union from file.
    struct isl_basic_set *bset;
    struct isl_union_set *bunion;

    bset = isl_basic_set_read_from_file (ctx, fd);
    bunion = isl_union_set_from_basic_set (bset);

    return (bunion);
}

int
PolyhedralWLFIntersectCalc (int verbose)
{
    // cwl is the consumerWL index vector's affine expn tree;
    // pwl is the producerWL bounds, step, width affine expn tree.
    //
    // If the intersect of cwl and pwl is cwl, then the fold
    // can proceed immediately. If the intersect is NULL, then
    // no folding can happen. Otherwise, the cwl must be sliced.
    //
    int z = POLY_RET_UNKNOWN;
    struct isl_ctx *ctx = isl_ctx_alloc ();
    struct isl_union_set *pwl = NULL;
    struct isl_union_set *cwl = NULL;
    struct isl_union_set *peq = NULL;
    struct isl_union_set *intr = NULL;
    struct isl_union_set *cwleq = NULL;
    struct isl_union_set *pwleq = NULL;

    pwl = ReadUnionFromFile (stdin, ctx);
    cwl = ReadUnionFromFile (stdin, ctx);
    peq = ReadUnionFromFile (stdin, ctx);
    printUnion (stderr, pwl, "pwl", verbose, 0);
    printUnion (stderr, cwl, "cwl", verbose, 0);
    printUnion (stderr, peq, "peq", verbose, 0);

    // ISL: Is (cwl * eq) = ( cwl * eq * pwl)?
    cwleq = doIntersect (cwl, peq, "cwleq", verbose);
    pwleq = doIntersect (pwl, peq, "pwleq", verbose);
    intr = doIntersect (cwleq, pwleq, "intr", verbose);
    if (isl_union_set_is_subset (cwleq, intr)) {
        z = POLY_RET_CCONTAINSB;
        if (verbose) {
            fprintf (stderr, "cwleq is subset of intersect( cwleq, pwleq)\n");
        }
    }

    fprintf (stdout, "%d\n", z); // This is the result that PHUT reads.
    printUnion (stdout, intr, "intersect", 1, ISL_FORMAT_ISL);

    isl_union_set_free (pwl);
    isl_union_set_free (cwl);
    isl_union_set_free (peq);
    isl_union_set_free (intr);
    isl_union_set_free (cwleq);
    isl_union_set_free (pwleq);
    isl_ctx_free (ctx);

    return (z);
}

int
PolyhedralRelationalCalc (int verbose)
{ // Check for matching polyhedra in relationals for POGO
    //   Read four Polylib polyhedra (B,C,rfn,cfn) from stdin,
    //   where B and C are PRF_ARG1 and PRF_ARG2 for the relational,
    //   and where rfn represents the relational itself;
    //   cfn represents the complementary function for the relational.
    //   E.g., for B < C, the complementary relational is B >= C.
    //   Intersect B,C,rfn, and return result of POLY_RET_EMPTYSET_BCR if the
    //   intersect is a NULL set.
    //
    //   Else, intersect B,C,cfn and return result of POLY_RET_EMPTYSET_BCC if the
    //   intersect is a NULL set.
    //
    //   Else, return POLY_RET_UNKNOWN.

    int z = POLY_RET_UNKNOWN;

    struct isl_union_set *unionb = NULL;
    struct isl_union_set *unionc = NULL;
    struct isl_union_set *unionrfn = NULL;
    struct isl_union_set *unioncfn = NULL;
    struct isl_union_set *intersectbc = NULL;
    struct isl_union_set *intersectbcr = NULL;
    struct isl_union_set *intersectbcc = NULL;
    struct isl_ctx *ctx = isl_ctx_alloc ();

    unionb = ReadUnionFromFile (stdin, ctx);   // PRF_ARG1 affine exprs tree
    unionc = ReadUnionFromFile (stdin, ctx);   // PRF_ARG2 affine exprs tree
    unionrfn = ReadUnionFromFile (stdin, ctx); // Relational fn
    unioncfn = ReadUnionFromFile (stdin, ctx); // Complementary relational fn

    printUnion (stderr, unionb, "unionb", verbose, 0);
    printUnion (stderr, unionc, "unionc", verbose, 0);
    printUnion (stderr, unionrfn, "unionrfn", verbose, 0);
    printUnion (stderr, unioncfn, "unioncfn", verbose, 0);

#ifdef RUBBISH
    if (isl_union_set_is_subset (unionb, unionc)
        && isl_union_set_is_subset (unionc, unionb)) {
        z = z | POLY_RET_MATCH_BC;
        z = z & ~POLY_RET_UNKNOWN;
        if (verbose) {
            fprintf (stderr, "non-empty b matches c\n");
        }
    } else {
        if (verbose) {
            fprintf (stderr, "b does not match c\n");
        }
    }
#endif // RUBBISH

    if (POLY_RET_UNKNOWN == z) { // Intersect b,c
        intersectbc = doIntersect (unionb, unionc, "intersectbc", verbose);
        if (isl_union_set_is_empty (intersectbc)) {
            z = z | POLY_RET_EMPTYSET_BC;
            z = z & ~POLY_RET_UNKNOWN;
            if (verbose) {
                fprintf (stderr, "b does not intersect c\n");
            }
        } else {
            if (verbose) {
                fprintf (stderr, "b intersects c\n");
            }
        }
    }

    if (POLY_RET_UNKNOWN == z) { // Intersect b,c,rfn
        intersectbcr = doIntersect (intersectbc, unionrfn, "intersectbcr", verbose);
        if (isl_union_set_is_empty (intersectbcr)) {
            z = z | POLY_RET_EMPTYSET_BCR;
            z = z & ~POLY_RET_UNKNOWN;
            if (verbose) {
                fprintf (stderr, "no intersect in b,c,rfn\n");
            }
        } else {
            if (verbose) {
                fprintf (stderr, "(b,c) intersects rfn\n");
            }
        }
    }

    if (POLY_RET_UNKNOWN == z) { // Intersect b,c,cfn
        intersectbcc = doIntersect (intersectbc, unioncfn, "intersectbcc", verbose);
        if (isl_union_set_is_empty (intersectbcc)) {
            if (verbose) {
                fprintf (stderr, "no intersect in b,c,cfn\n");
            }
            z = z | POLY_RET_EMPTYSET_BCC;
            z = z & ~POLY_RET_UNKNOWN;
        } else {
            if (verbose) {
                fprintf (stderr, "(b,c) intersects cfn\n");
            }
        }
    }

    isl_union_set_free (unionb);
    isl_union_set_free (unionc);
    isl_union_set_free (unionrfn);
    isl_union_set_free (unioncfn);
    isl_union_set_free (intersectbc);
    isl_union_set_free (intersectbcr);
    isl_union_set_free (intersectbcc);
    isl_ctx_free (ctx);

    printf ("%d\n", z); // This is the result that PHUT reads.

    return (z);
}

int
main (int argc, char *argv[])
{
    FILE *file;
    char opcode;
    int z = POLY_RET_UNKNOWN;
    int res;
    int verbose = 0;

    if (argc > 1) {
        opcode = *argv[1];
    } else {
        fprintf (stderr, "Specify opcode as command-line argument, please");
        exit (EXIT_FAILURE);
    }

    if (argc > 2) { // Any second argument forces verbosity. This is crude, I admit.
        verbose = 1;
        fprintf (stderr, "Running in verbose mode\n");
    }

    switch (opcode) {

    case POLY_OPCODE_INTERSECT: // POlyhedral Guard Optimization (POGO)
        z = PolyhedralRelationalCalc (verbose);
        break;

    case POLY_OPCODE_PWLF: // Polyhedral WLF (PWLF)
        z = PolyhedralWLFIntersectCalc (verbose);
        break;

    default:
        fprintf (stderr, "caller is confused. We got opcode=%c\n", opcode);
        break;
    }

    if (verbose) {
        fprintf (stderr, "result is %d\n", z);
    }

    return (z);
}
