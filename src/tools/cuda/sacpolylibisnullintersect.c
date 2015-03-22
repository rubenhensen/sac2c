/*
 * This function is a predicate for NULL intersects of three polyhedra.
 *
 * Command-line option I:
 *   Read four Polylib polyhedra (A,B,C,D) from stdin,
 *   If A and B are matching sets, return POLY_MATCH_AB.
 *   Else, intersect A,B,C, and return result of POLY_EMPTYSET_ABC if the
 *   intersect is a NULL set.
 *
 *   Else, intersect A,B,D and return result of POLY_EMPTYSET_ABD if the
 *   intersect is a NULL set.
 *
 *   Else, return POLY_UNKNOWN or POLY_INVALID.
 *
 *  FIXME: these options are broken.
 *  M:  return 1 if the polyhedral intersection matches Poly_A; else 0.
 *  P:  print the intersection of the polyhedra on stdout
 *
 */

#include "arithmetique.h"
#include "types.h"
#include "vector.h"
#include "matrix.h"
#include "polyhedron.h"
#include "stdio.h"
#include "polyhedral_utilities.h"

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
    int z = POLY_UNKNOWN;
    int res;
    int numrays = 0;
    int finished = 0;

    if (argc > 1) {
        opcode = *argv[1];
        // printf("opcode is %c\n", opcode);
    } else {
        opcode = 'x';
    }

    Matrix_A = Matrix_Read ();
    Matrix_B = Matrix_Read ();
    Matrix_C = Matrix_Read ();
    Matrix_D = Matrix_Read ();

    Poly_A = Constraints2Polyhedron (Matrix_A, MAXCON);
    Poly_B = Constraints2Polyhedron (Matrix_B, MAXCON);
    Poly_C = Constraints2Polyhedron (Matrix_C, MAXCON);
    Poly_D = Constraints2Polyhedron (Matrix_D, MAXCON);

#ifdef VERBOSE
    fprintf (stderr, "Poly_A is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_A);
    fprintf (stderr, "Poly_B is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_B);
    fprintf (stderr, "Poly_C is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_C);
    fprintf (stderr, "Poly_D is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_D);
#endif // VERBOSE

    switch (opcode) {
    case 'P':
        Poly_AB = DomainIntersection (Poly_A, Poly_B, MAXCON);
        Poly_Z = DomainIntersection (Poly_AB, Poly_C, MAXCON);
        fprintf (stderr, "Poly_Z(ABC) is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_Z);
        fprintf (stderr, "# Poly_Z rays=%d\n", Poly_Z->NbRays);
        break;

    case 'I':
        // check for matching polyhedra A and B.
        if (PolyhedronIncludes (Poly_A, Poly_B) && PolyhedronIncludes (Poly_B, Poly_A)) {
            z = z | POLY_MATCH_AB;
            z = z & ~POLY_UNKNOWN;
            finished = 1;
        }

        if (!finished) { // Intersect A,B
            if (isNullIntersect (Poly_A, Poly_B, &Poly_AB)) {
                z = z | POLY_EMPTYSET_AB;
                z = z & ~POLY_UNKNOWN;
                finished = 1;
#ifdef VERBOSE
                fprintf (stderr, "no rays in A,B\n");
#endif // VERBOSE
            }
        }

        if (!finished) { // Intersect ABC
            if (isNullIntersect (Poly_AB, Poly_C, &Poly_Z)) {
                z = z | POLY_EMPTYSET_ABC;
                z = z & ~POLY_UNKNOWN;
                numrays = Poly_Z->NbRays;
                finished = 1;
#ifdef VERBOSE
                fprintf (stderr, "no rays in A,B,C\n");
#endif // VERBOSE
            }
        }

        if (!finished) { // Intersect ABD
            Domain_Free (Poly_Z);
            if (isNullIntersect (Poly_AB, Poly_D, &Poly_Z)) {
                z = z | POLY_EMPTYSET_ABD;
                z = z & ~POLY_UNKNOWN;
                numrays = Poly_Z->NbRays;
                finished = 1;
#ifdef VERBOSE
                fprintf (stderr, "no rays in A,B,D\n");
#endif // VERBOSE
            }
        }
        break;

#ifdef NEEDSWORKORBURNING
    case 'M':
        // to be continued
        Poly_I = DomainIntersection (Poly_A, Poly_B, MAXCON);
        Poly_I2 = DomainIntersection (Poly_I, Poly_C, MAXCON);

        // We assume that Poly_A is the CWL iv. If this matches the intersect,
        // then the PWLF can proceed immediately
        if ('M' == opcode) {
            if (PolyhedronIncludes (Poly_A, Poly_I)
                && PolyhedronIncludes (Poly_I, Poly_A)) {
                fprintf (stderr, "polyhedra Poly_A and Poly_I match\n");
                z = INTERSECT_fold;
            } else {
                fprintf (stderr, "polyhedra Poly_A and Poly_I do not match\n");
                z = INTERSECT_xxx;
            }
        }
#endif // NEEDSWORKORBURNING
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

#ifdef VERBOSE
    fprintf (stderr, "result is %d\n", z);
#endif // VERBOSE
    printf ("%d\n", z);

    return (z);
}
