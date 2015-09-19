/*
 * This function is a predicate for NULL intersect of two polyhedra.
 *
 * Read two Polylib polyhedra from stdin,
 * compute their intersection,
 * and return 1 if their intersection is NULL; 0 otherwise.
 *
 */

#define VERBOSE

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
main ()
{
    FILE *file;
    Matrix *Matrix_A;
    Matrix *Matrix_B;
    Polyhedron *Poly_A;
    Polyhedron *Poly_B;
    Polyhedron *Poly_I;
    int maxcon = 2000;
    int z = 0;

    Matrix_A = Matrix_Read ();
    Matrix_B = Matrix_Read ();

    Poly_A = Constraints2Polyhedron (Matrix_A, maxcon);
    Poly_B = Constraints2Polyhedron (Matrix_B, maxcon);

#ifdef VERBOSE
    fprintf (stderr, "Poly_A is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_A);
    fprintf (stderr, "Poly_B is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_B);
#endif // VERBOSE

    Poly_I = DomainIntersection (Poly_A, Poly_B, maxcon);

#ifdef VERBOSE
    fprintf (stderr, "Poly_I is:\n");
    Polyhedron_Print (stderr, "%4d", Poly_I);
    fprintf (stderr, "#rays=%d\n", Poly_I->NbRays);
#endif // VERBOSE

    if (Poly_I->NbRays == 0) { // Empty array joke.
        z = 1;                 // If no constraints, the polyhedra match

#ifdef VERBOSE
        fprintf (stderr, "no rays\n");
#endif // VERBOSE

    } else {
        if (PolyhedronIncludes (Poly_A, Poly_B) && PolyhedronIncludes (Poly_B, Poly_A)) {
#ifdef VERBOSE
            fprintf (stderr, "polyhedra match\n");
#endif // VERBOSE
            z = 1;
        } else {
#ifdef VERBOSE
            fprintf (stderr, "polyhedral do not match\n");
#endif // VERBOSE
            z = 0;
        }
    }

    Matrix_Free (Matrix_A);
    Matrix_Free (Matrix_B);
    Domain_Free (Poly_A);
    Domain_Free (Poly_B);
    Domain_Free (Poly_I);

    printf ("%d\n", z);

    return (z);
}
