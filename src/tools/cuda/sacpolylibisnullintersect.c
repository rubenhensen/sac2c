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

typedef enum {
    INTERSECT_null,
    INTERSECT_fold,
    INTERSECT_cutleft,
    INTERSECT_cutright,
    INTERSECT_cutboth,
    INTERSECT_unknown
} intersect_type_t;

int
main (int argc, char *argv[])
{
    FILE *file;
    char opcode;
    Matrix *Matrix_A;
    Matrix *Matrix_B;
    Matrix *Matrix_C;
    Polyhedron *Poly_A;
    Polyhedron *Poly_B;
    Polyhedron *Poly_C;
    Polyhedron *Poly_I;
    Polyhedron *Poly_I2;
    int maxcon = 2000;
    int z = 0;

    if (argc > 1) {
        opcode = *argv[1];
        // printf("opcode is %c\n", opcode);
    } else {
        opcode = 'x';
    }

    Matrix_A = Matrix_Read ();
    Matrix_B = Matrix_Read ();
    Matrix_C = Matrix_Read ();

    Poly_A = Constraints2Polyhedron (Matrix_A, maxcon);
    Poly_B = Constraints2Polyhedron (Matrix_B, maxcon);
    Poly_C = Constraints2Polyhedron (Matrix_C, maxcon);

#ifdef VERBOSE
    fprintf (stderr, "Poly_A is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_A);
    fprintf (stderr, "Poly_B is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_B);
    fprintf (stderr, "Poly_C is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_C);
#endif // VERBOSE

    Poly_I = DomainIntersection (Poly_A, Poly_B, maxcon);
    Poly_I2 = DomainIntersection (Poly_I, Poly_C, maxcon);

    switch (opcode) {
    case 'P':
        fprintf (stderr, "Poly_I2 is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_I2);
        fprintf (stderr, "#rays=%d\n", Poly_I2->NbRays);
        break;

    case 'I':
#ifdef VERBOSE
        fprintf (stderr, "Poly_I2 is:\n");
        Polyhedron_Print (stderr, "%4d", Poly_I2);
        fprintf (stderr, "#rays=%d\n", Poly_I2->NbRays);
#endif                              // VERBOSE
        if (Poly_I2->NbRays == 0) { // Empty array joke.
            z = 1;                  // If no constraints, the polyhedra match
#ifdef VERBOSE
            fprintf (stderr, "no rays\n");
#endif // VERBOSE
        } else {
            if (PolyhedronIncludes (Poly_A, Poly_B)
                && PolyhedronIncludes (Poly_B, Poly_A)) {
#ifdef VERBOSE
                fprintf (stderr, "polyhedra Poly_A and Poly_B match\n");
#endif // VERBOSE
                z = 1;
            } else {
#ifdef VERBOSE
                fprintf (stderr, "polyhedra Poly_A and Poly_B do not match\n");
#endif // VERBOSE
                z = 0;
            }
        }
        break;

    case 'M':
        // to be continued

        // We assume that Poly_A is the CWL iv. If this matches the intersect,
        // then the PWLF can proceed immediately
        if ('M' == opcode) {
            if (PolyhedronIncludes (Poly_A, Poly_I)
                && PolyhedronIncludes (Poly_I, Poly_A)) {
                fprintf (stderr, "polyhedra Poly_A and Poly_I match\n");
                z = 1;
            } else {
                fprintf (stderr, "polyhedra Poly_A and Poly_I do not match\n");
                z = 0;
            }
        }
        break;
    default:
        fprintf (stderr, "caller is confused. We got opcode=%c\n", opcode);
        break;
    }

    Matrix_Free (Matrix_A);
    Matrix_Free (Matrix_B);
    Matrix_Free (Matrix_C);
    Domain_Free (Poly_A);
    Domain_Free (Poly_B);
    Domain_Free (Poly_C);
    Domain_Free (Poly_I);
    Domain_Free (Poly_I2);

    printf ("%d\n", z);

    return (z);
}
