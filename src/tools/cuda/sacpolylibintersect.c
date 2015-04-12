this is rubbish

/*
 * Read two Polylib polyhedra from stdin,
 * compute their intersection,
 * and return 0 if their intersection is NULL; 1 otherwise.
 *
 */

#include "arithmetique.h"
#include "types.h"
#include "vector.h"
#include "matrix.h"
#include "polyhedron.h"

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
    int z = 1;

    Matrix_A = Matrix_Read ();
    Matrix_B = Matrix_Read ();

    Poly_A = Constraints2Polyhedron (Matrix_A, maxcon);
    Poly_B = Constraints2Polyhedron (Matrix_B, maxcon);
    printf ("Poly_A is:\n");
    Polyhedron_Print (stdout, "%4d", Poly_A);
    printf ("Poly_B is:\n");
    Polyhedron_Print (stdout, "%4d", Poly_B);

    Poly_I = DomainIntersection (Poly_A, Poly_B, maxcon);

    printf ("Poly_I is:\n");
    Polyhedron_Print (stdout, "%4d", Poly_I);

    printf ("#rays=%d\n", Poly_I->NbRays);
    if (Poly_I->NbRays == 0) { // Empty array joke.
        z = 0;                 // If no constraints, the polyhedra match
        printf ("no rays\n");
    } else {
        if (PolyhedronIncludes (Poly_A, Poly_B) && PolyhedronIncludes (Poly_B, Poly_A)) {
            printf ("polyhedra match\n");
            z = 0;
        } else {
            printf ("polyhedral do not match\n");
            z = 1;
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
