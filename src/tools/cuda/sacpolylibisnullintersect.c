/*
 * This function is a predicate for NULL intersects of three polyhedra.
 *
 * Command-line option I:
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
 *  F: Used by PWLF. Poly_A is Consumer-WL; Poly_B is Producer_WL.
 *     If PolyMatch( Poly_A,( Poly_A intersect Poly_B)), then
 *     the fold can happen immediately.
 *     If isNullIntersect( Poly_A, Poly_B), then no folding can occur.
 *     Otherwise, FIXME we have to decode the intersect and slice the CWL.
 *  I: Used by POGO. Checks for PolyMatch( Poly_A, Poly_B).
 *  M:  return 1 if the polyhedral intersection matches Poly_A; else 0.
 *  P:  print the intersection of the polyhedra on stdout
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
isNullIntersect (Polyhedron *Poly_A, Polyhedron *Poly_B, Polyhedron **Poly_Z)
{
    // Intersect Poly_A and Poly_B, placing the result in Poly_Z.
    // return TRUE(1) if the intersect is NULL; else FALSE(0).

    int z;

    *Poly_Z = DomainIntersection (Poly_A, Poly_B, MAXCON);
    z = (0 == (*Poly_Z)->NbRays) ? 1 : 0;
#ifdef VERBOSE
    fprintf (stderr, "Poly_Z is:\n");
    Polyhedron_Print (stderr, "%4d", *Poly_Z);
    fprintf (stderr, "#rays ABC=%d\n", (*Poly_Z)->NbRays);
#endif // VERBOSE
    return (z);
}

int
main (int argc, char *argv[])
{
    FILE *file;
    char opcode;
    Matrix *Matrix_A = NULL;
    Matrix *Matrix_B = NULL;
    Matrix *Matrix_C = NULL;
    Matrix *Matrix_D = NULL;
    Polyhedron *Poly_A = NULL;
    Polyhedron *Poly_B = NULL;
    Polyhedron *Poly_C = NULL;
    Polyhedron *Poly_D = NULL;
    Polyhedron *Poly_AB = NULL;
    Polyhedron *Poly_Z = NULL;
    int z = POLY_RET_UNKNOWN;
    int z0;
    int z1;
    int res;
    int numrays = 0;
    int finished = 0;
    int verbose = 0;

    if (argc > 1) {
        opcode = *argv[1];
    } else {
        fprintf (stderr, "Specify opcode as command-line argument, please");
        exit (EXIT_FAILURE);
    }

    if (argc > 2) { // Any second argument is fine. This is crude, I admit.
        verbose = 1;
    }

    Matrix_A = Matrix_Read ();
    Matrix_B = Matrix_Read ();
    Matrix_C = Matrix_Read ();
    Matrix_D = Matrix_Read ();

    Poly_A = Constraints2Polyhedron (Matrix_A, MAXCON);
    Poly_B = Constraints2Polyhedron (Matrix_B, MAXCON);
    Poly_C = Constraints2Polyhedron (Matrix_C, MAXCON);
    Poly_D = Constraints2Polyhedron (Matrix_D, MAXCON);

    if (verbose) {
        fprintf (stderr, "Poly_A is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_A);
        fprintf (stderr, "Poly_B is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_B);
        fprintf (stderr, "Poly_C is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_C);
        fprintf (stderr, "Poly_D is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_D);
    }

    switch (opcode) {
    case POLY_OPCODE_PRINT:
        Poly_AB = DomainIntersection (Poly_A, Poly_B, MAXCON);
        Poly_Z = DomainIntersection (Poly_AB, Poly_C, MAXCON);
        fprintf (stderr, "Poly_Z(ABC) is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_Z);
        fprintf (stderr, "# Poly_Z rays=%d\n", Poly_Z->NbRays);
        break;

    case POLY_OPCODE_INTERSECT:
        // check for matching polyhedra A and B.
        if (PolyhedronIncludes (Poly_A, Poly_B) && PolyhedronIncludes (Poly_B, Poly_A)) {
            z = z | POLY_RET_MATCH_AB;
            z = z & ~POLY_RET_UNKNOWN;
            finished = 1;
        }

        if (!finished) { // Intersect A,B
            if (isNullIntersect (Poly_A, Poly_B, &Poly_AB)) {
                z = z | POLY_RET_EMPTYSET_AB;
                z = z & ~POLY_RET_UNKNOWN;
                finished = 1;
                if (verbose) {
                    fprintf (stderr, "no rays in A,B\n");
                }
            }
        }

        if (!finished) { // Intersect ABC
            if (isNullIntersect (Poly_AB, Poly_C, &Poly_Z)) {
                z = z | POLY_RET_EMPTYSET_ABC;
                z = z & ~POLY_RET_UNKNOWN;
                numrays = Poly_Z->NbRays;
                finished = 1;
                if (verbose) {
                    fprintf (stderr, "no rays in A,B,C\n");
                }
            }
        }

        if (!finished) { // Intersect ABD
            Domain_Free (Poly_Z);
            if (isNullIntersect (Poly_AB, Poly_D, &Poly_Z)) {
                z = z | POLY_RET_EMPTYSET_ABD;
                z = z & ~POLY_RET_UNKNOWN;
                numrays = Poly_Z->NbRays;
                finished = 1;
                if (verbose) {
                    fprintf (stderr, "no rays in A,B,D\n");
                }
            }
        }
        break;

    case POLY_OPCODE_PWLF: // Polyhedral WLF
        // Poly_A is the CWL iv; Poly_B is the PWL bounds,step,width.
        // Poly_C is the condition that CWL iv matches PWL iv.
        // If CWL is a subset of PWL, then we can fold immediately.
        // If their intersection matches CWL, then this is the case.
        Poly_AB = DomainIntersection (Poly_A, Poly_B, MAXCON);
        Poly_Z = DomainIntersection (Poly_AB, Poly_C, MAXCON);
        if (verbose) {
            fprintf (stderr, "Poly_AB is:\n");
            Polyhedron_Print (stderr, "%4d", Poly_AB);
            fprintf (stderr, "Poly_Z is:\n");
            Polyhedron_Print (stderr, "%4d", Poly_Z);
        }

        z0 = (PolyhedronIncludes (Poly_Z, Poly_A)) ? 1 : 0;
        if (verbose) {
            fprintf (stderr, "PolyhedronIncludes(Poly_Z, Poly_A is: %d\n", z0);
        }
        z1 = (PolyhedronIncludes (Poly_A, Poly_Z)) ? 1 : 0;
        if (verbose) {
            fprintf (stderr, "PolyhedronIncludes(Poly_A, Poly_Z is: %d\n", z1);
        }
        z = (z0 && z1) ? POLY_RET_BCONTAINSA : POLY_RET_UNKNOWN;
        // FIXME - more coding needed if !z
        break;

    case POLY_OPCODE_MATCH:
        if (PolyhedronIncludes (Poly_A, Poly_B) && PolyhedronIncludes (Poly_B, Poly_A)) {
            z = POLY_RET_MATCH_AB;
        } else {
            z = POLY_RET_UNKNOWN;
        }
        break;

    default:
        fprintf (stderr, "caller is confused. We got opcode=%c\n", opcode);
        break;
    }

    Matrix_Free (Matrix_A);
    Matrix_Free (Matrix_B);
    Matrix_Free (Matrix_C);
    Matrix_Free (Matrix_D);
    Domain_Free (Poly_A);
    Domain_Free (Poly_B);
    Domain_Free (Poly_C);
    Domain_Free (Poly_D);
    Domain_Free (Poly_AB);
    Domain_Free (Poly_Z);

    if (verbose) {
        fprintf (stderr, "result is %d\n", z);
    }
    printf ("%d\n", z);

    return (z);
}
