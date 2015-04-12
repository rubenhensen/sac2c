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
printUnion (struct isl_union_set *pset, char *titl, struct isl_ctx *ctx, int verbose)
{
    isl_printer *p;

    if (verbose) {
        fprintf (stdout, "Union %s is:\n", titl);
        p = isl_printer_to_file (ctx, stdout);
        p = isl_printer_print_union_set (p, pset);
        p = isl_printer_end_line (p);
        isl_printer_free (p);
    }
    return;
}

struct isl_union_set *
doIntersect (struct isl_union_set *unionb, struct isl_union_set *unionc, char *titl,
             struct isl_ctx *ctx, int verbose)
{ // Intersect two sets and return the resulting set
    struct isl_union_set *intersectbc;

    intersectbc = isl_union_set_intersect (isl_union_set_copy (unionb),
                                           isl_union_set_copy (unionc));
    if (verbose) {
        printUnion (intersectbc, titl, ctx, verbose);
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
    // Poly_CWL is the CWL iv; Poly_PWL is the PWL bounds,step,width.
    // Poly_EQ is the condition that CWL iv matches PWL iv.
    // If the CWL is a subset of PWL, then we can fold immediately.
    //
    int z = POLY_RET_UNKNOWN;

#ifdef FIXME
    Poly_CWL = ReadPoly ("cwl", verbose);
    Poly_PWL = ReadPoly ("pwl", verbose);
    Poly_EQ = ReadPoly ("equality", verbose);

    Poly_PWLEQ = DomainIntersection (Poly_PWL, Poly_EQ, MAXCON);
    Poly_Z = DomainIntersection (Poly_PWLEQ, Poly_CWL, MAXCON);
    Matrix_Z = Polyhedron2Constraints (Poly_Z);
    if (verbose) {
        fprintf (stderr, "Poly_PWLEQ is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_PWLEQ);
        fprintf (stderr, "Poly_Z is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_Z);
    }

    z = PolyhedronIncludes (Poly_Z, Poly_CWL);
    if (verbose) {
        fprintf (stderr, "PolyhedronIncludes(Poly_CWL, Poly_Z is: %d\n", z);
    }
    z = z ? POLY_RET_YCONTAINSX : POLY_RET_UNKNOWN;
    printf ("%d\n", z); // This is the result that PHUT reads.

    Matrix_Print (stdout, "%4d", Matrix_Z); // This is the intersection polyhedron
    Matrix_Free (Matrix_Z);

    Domain_Free (Poly_CWL);
    Domain_Free (Poly_PWL);
    Domain_Free (Poly_EQ);
    Domain_Free (Poly_PWLEQ);
    Domain_Free (Poly_Z);
#endif // FIXME

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
    //
    //   If B and Y are matching sets, return POLY_RET_MATCH_BC.
    //   Else, intersect B,C,rfn, and return result of POLY_RET_EMPTYSET_BCR if the
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

    printUnion (unionb, "unionb", ctx, verbose);
    printUnion (unionc, "unionc", ctx, verbose);
    printUnion (unionrfn, "unionrfn", ctx, verbose);
    printUnion (unioncfn, "unioncfn", ctx, verbose);

    // I am guessing that looking for equality may be relatively fast, compared
    // to intersecting set. If not, rip out this code block.
    if (isl_union_set_is_equal (unionb, unionc)) {
        z = z | POLY_RET_MATCH_BC;
        z = z & ~POLY_RET_UNKNOWN;
        if (verbose) {
            fprintf (stderr, "b matches c\n");
        }
    } else {
        if (verbose) {
            fprintf (stderr, "b does not match c\n");
        }
    }

    if (POLY_RET_UNKNOWN == z) { // Intersect b,c
        intersectbc = doIntersect (unionb, unionc, "intersectbc", ctx, verbose);
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
        intersectbcr = doIntersect (intersectbc, unionrfn, "intersectbcr", ctx, verbose);
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
        intersectbcc = doIntersect (intersectbc, unioncfn, "intersectbcc", ctx, verbose);
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
