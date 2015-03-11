/*
 * This function is a predicate for NULL intersect of three polyhedra.
 *
 * Read three Polylib polyhedra from stdin, intersect them, and then
 * and do something with the result, based on the command-line argument:
 *
 *  I:  return 1 if the polyhedral intersection is NULL; else 0.
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
#include "sacpolylibisnullintersect.h"

int
main (int argc, char *argv[])
{
    FILE *file;
    char opcode;
    Matrix *Matrix_A;
    Matrix *Matrix_B;
    Matrix *Matrix_C;
    Matrix *Matrix_D;
    Polyhedron *Poly_A;
    Polyhedron *Poly_B;
    Polyhedron *Poly_C;
    Polyhedron *Poly_D;
    Polyhedron *Poly_I;
    Polyhedron *Poly_I2;
    int maxcon = 2000;
    int z = EMPTYSET_UNKNOWN;
    int numrays = 0;

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

    Poly_A = Constraints2Polyhedron (Matrix_A, maxcon);
    Poly_B = Constraints2Polyhedron (Matrix_B, maxcon);
    Poly_C = Constraints2Polyhedron (Matrix_C, maxcon);
    Poly_D = Constraints2Polyhedron (Matrix_D, maxcon);

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
        Poly_I = DomainIntersection (Poly_A, Poly_B, maxcon);
        Poly_I2 = DomainIntersection (Poly_I, Poly_C, maxcon);
        fprintf (stderr, "Poly_I2 is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_I2);
        fprintf (stderr, "# PolyI2 rays=%d\n", Poly_I2->NbRays);
        break;

    case 'I':
        Poly_I = DomainIntersection (Poly_A, Poly_B, maxcon);
        if (0 == Poly_I->NbRays) { // Stop here if already empty
            z = z | EMPTYSET_ABC;
            z = z & ~EMPTYSET_UNKNOWN;
            numrays = 0;
#ifdef VERBOSE
            fprintf (stderr, "no rays in A,B\n");
#endif // VERBOSE
        } else {
            Poly_I2 = DomainIntersection (Poly_I, Poly_C, maxcon);
            numrays = Poly_I2->NbRays;

#ifdef VERBOSE
            fprintf (stderr, "Poly_I2 is:\n");
            Polyhedron_Print (stderr, "%4d", Poly_I2);
            fprintf (stderr, "#rays ABC=%d\n", Poly_I2->NbRays);
#endif // VERBOSE
        }
        if (0 == numrays) { // Empty set implies all is good
            z = z | EMPTYSET_ABC;
            z = z & ~EMPTYSET_UNKNOWN;
#ifdef VERBOSE
            fprintf (stderr, "no rays in A,B,C\n");
#endif // VERBOSE
        }
#ifdef VERBOSE
        fprintf (stderr, "Poly_I2 is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_I2);
        fprintf (stderr, "#rays ABC=%d\n", Poly_I2->NbRays);
#endif // VERBOSE

        // Compute A * B * D
        Domain_Free (Poly_I2);
        Poly_I2 = DomainIntersection (Poly_I, Poly_D, maxcon);
        numrays = Poly_I2->NbRays;
#ifdef VERBOSE
        fprintf (stderr, "ABD is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_I2);
        fprintf (stderr, "#rays ABD=%d\n", Poly_I2->NbRays);
#endif // VERBOSE
        if (0 == numrays) {
            z = z | EMPTYSET_ABD;
            z = z & ~EMPTYSET_UNKNOWN;
#ifdef VERBOSE
            fprintf (stderr, "no rays in A,B,D\n");
#endif // VERBOSE
        }
        break;

    case 'M':
        // to be continued
        Poly_I = DomainIntersection (Poly_A, Poly_B, maxcon);
        Poly_I2 = DomainIntersection (Poly_I, Poly_C, maxcon);

        // We assume that Poly_A is the CWL iv. If this matches the intersect,
        // then the PWLF can proceed immediately
#ifdef NEEDSWORKORBURNING
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
    Domain_Free (Poly_I);
    Domain_Free (Poly_I2);

#ifdef VERBOSE
    fprintf (stderr, "result is %d\n", z);
#endif // VERBOSE
    printf ("%d\n", z);

    return (z);
}
