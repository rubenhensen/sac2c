/*
 * This is an interface to PolyLib, providing several services to CWL and POGO.
 *
 * Command-line option POLY_OPCODE_INTERSECT(I):
 *   Used by POlyhedral Guard Optimization (POGO.
 *
 *   Read four Polylib polyhedra (A,B,C,D) from stdin,
 *   If A and B are matching sets, return POLY_RET_MATCH_AB.
 *   Else, intersect A,B,C, and return result of POLY_RET_EMPTYSET_ABC if the
 *   intersect is a NULL set.
 *
 *   Else, intersect A,B,D and return result of POLY_RET_EMPTYSET_ABD if the
 *   intersect is a NULL set.
 *
 *   Else, return POLY_RET_UNKNOWN or POLY_RET_INVALID.
 *
 *
 * Command-line option POLY_OPCODE_PWLF(F):
 *   Used by Polyhedral with-loop folding (PWLF)
 *
 *   Read three PolyLib polyhedra from stdin.
 *
 *
 */

#include "stdlib.h"
#include "arithmetique.h"
#include "types.h" // This is the PolyLib file!
#include "vector.h"
#include "matrix.h"
#include "polyhedron.h"
#include "stdio.h"
#include "polyhedral_defs.h"

#define MAXCON 2000

int
isNullIntersect (Polyhedron *Poly_A, Polyhedron *Poly_B, Polyhedron **Poly_Z, int verbose)
{
    // Intersect Poly_A and Poly_B, placing the result in Poly_Z.
    // return TRUE(1) if the intersect is NULL; else FALSE(0).

    int z;

    *Poly_Z = DomainIntersection (Poly_A, Poly_B, MAXCON);
    z = (0 == (*Poly_Z)->NbRays) ? 1 : 0;
    if (verbose) {
        fprintf (stderr, "isNullIntersect result is %d; Poly_Z is:\n", z);
        Polyhedron_Print (stderr, "%4d", *Poly_Z);
        fprintf (stderr, "#rays ABC=%d\n", (*Poly_Z)->NbRays);
    }
    return (z);
}

int
isMatrixEmpty (Matrix *m)
{
    return (0 == m->NbRows);
}

Polyhedron *
ReadPoly (char *tag, int verbose)
{ // Read stdin and create a Polyhedron
    Polyhedron *p = NULL;
    Matrix *Matrix_R;

    Matrix_R = Matrix_Read ();
    if ((NULL != Matrix_R) && (!isMatrixEmpty (Matrix_R))) {
        p = Constraints2Polyhedron (Matrix_R, MAXCON);
        Matrix_Free (Matrix_R);
    }
    if (verbose) {
        fprintf (stderr, "Poly_%s is:\n", tag);
        Polyhedron_Print (stderr, "%4d", p);
    }

    return (p);
}

int
PolyhedralWLFIntersectCalc (int verbose)
{
    // Poly_CWL is the CWL iv; Poly_PWL is the PWL bounds,step,width.
    // Poly_EQ is the condition that CWL iv matches PWL iv.
    // If the CWL is a subset of PWL, then we can fold immediately.
    //
    Polyhedron *Poly_EQ = NULL;
    Polyhedron *Poly_CWL = NULL;
    Polyhedron *Poly_PWL = NULL;
    Polyhedron *Poly_PWLEQ = NULL;
    Polyhedron *Poly_Z = NULL;
    Matrix *Matrix_Z = NULL;
    int z = POLY_RET_UNKNOWN;

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

    return (z);
}

int
PolyhedralRelationalCalc (int verbose)
{ // Check for matching polyhedra in relationals for POGO
    //   Read four Polylib polyhedra (X,Y,F,C) from stdin,
    //   where X and Y are PRF_ARG1 and PRF_ARG2 for the relational,
    //   and where F represents the relational itself;
    //   C represents the complementary function for the relational.
    //   E.g., for X < Y, the relational is X >= Y.
    //
    //   If X and Y are matching sets, return POLY_RET_MATCH_XY.
    //   Else, intersect X,Y,F, and return result of POLY_RET_EMPTYSET_XYF if the
    //   intersect is a NULL set.
    //
    //   Else, intersect A,B,D and return result of POLY_RET_EMPTYSET_XYC if the
    //   intersect is a NULL set.
    //
    //   Else, return POLY_RET_UNKNOWN.

    Polyhedron *Poly_X = NULL;
    Polyhedron *Poly_Y = NULL;
    Polyhedron *Poly_RELFN = NULL;
    Polyhedron *Poly_C = NULL;
    Polyhedron *Poly_AF = NULL;
    Polyhedron *Poly_XY = NULL;
    Polyhedron *Poly_Z = NULL;
    Polyhedron *Poly_UF = NULL;
    Polyhedron *Poly_UC = NULL;
    Polyhedron *Poly_TEMP = NULL;
    int z = POLY_RET_UNKNOWN;
    int numrays = 0;

    Poly_X = ReadPoly ("X", verbose);
    Poly_Y = ReadPoly ("Y", verbose);
    Poly_RELFN = ReadPoly ("relfn", verbose);
    Poly_C = ReadPoly ("complementfn", verbose);
    Poly_UF = ReadPoly ("unionfn", verbose);
    Poly_UC = ReadPoly ("unioncomplentfn", verbose);

    // We may need to do a union on either of the functions.
    if (NULL != Poly_UF) {
        Poly_TEMP = Polyhedron_Copy (Poly_X);
        Domain_Free (Poly_X);
        Poly_X = DomainUnion (Poly_TEMP, Poly_UF, MAXCON);
    }

    if (PolyhedronIncludes (Poly_X, Poly_Y) && PolyhedronIncludes (Poly_Y, Poly_X)) {
        z = z | POLY_RET_MATCH_XY;
        z = z & ~POLY_RET_UNKNOWN;
    }

    if (POLY_RET_UNKNOWN == z) { // Intersect X,Y
        if (isNullIntersect (Poly_X, Poly_Y, &Poly_XY, verbose)) {
            z = z | POLY_RET_EMPTYSET_XY;
            z = z & ~POLY_RET_UNKNOWN;
            if (verbose) {
                fprintf (stderr, "X does not intersect Y\n");
            }
        }
    }

    if (POLY_RET_UNKNOWN == z) { // Intersect X,Y,F
        if (isNullIntersect (Poly_XY, Poly_RELFN, &Poly_Z, verbose)) {
            z = z | POLY_RET_EMPTYSET_XYF;
            z = z & ~POLY_RET_UNKNOWN;
            numrays = Poly_Z->NbRays;
            if (verbose) {
                fprintf (stderr, "no intersect in X,Y,F\n");
            }
        }
    }

    if (POLY_RET_UNKNOWN == z) { // Intersect X,Y,C
        Domain_Free (Poly_Z);
        if (isNullIntersect (Poly_XY, Poly_C, &Poly_Z, verbose)) {
            z = z | POLY_RET_EMPTYSET_XYC;
            z = z & ~POLY_RET_UNKNOWN;
            numrays = Poly_Z->NbRays;
            if (verbose) {
                fprintf (stderr, "no rays in A,B,D\n");
            }
        }
    }
    Domain_Free (Poly_X);
    Domain_Free (Poly_Y);
    Domain_Free (Poly_RELFN);
    Domain_Free (Poly_C);
    Domain_Free (Poly_AF);
    Domain_Free (Poly_XY);
    Domain_Free (Poly_Z);
    Domain_Free (Poly_UF);
    Domain_Free (Poly_UC);
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
