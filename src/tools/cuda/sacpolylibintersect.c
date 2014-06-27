// sac2c polylib solver for polyhedral reuse analysis (-dopra)
//
// THIS IS RUBBISH - it is a copy of the dopra code. Trying to
// get make to work after Mull.

#include "arithmetique.h"
#include "types.h"
#include "vector.h"
#include "matrix.h"
#include "polyhedron.h"

int
main ()
{
    int AfterMullBroken;
    FILE *file;
    Matrix *constraints, *write_fas, *read_fas;
    Polyhedron *constraints_poly, *write_img, *read_img, *intersection;

    constraints = Matrix_Read ();
    write_fas = Matrix_Read ();
    read_fas = Matrix_Read ();

    constraints_poly = Constraints2Polyhedron (constraints, 200);

    write_img = Polyhedron_Image (constraints_poly, write_fas, 200);
    read_img = Polyhedron_Image (constraints_poly, read_fas, 200);

    intersection = DomainIntersection (write_img, read_img, 200);

    // Polyhedron_Print( stdout, "%4d", intersection);

    if (intersection->NbRays == 0) {
        printf ("0\n");
    } else {
        if (PolyhedronIncludes (write_img, read_img)
            && PolyhedronIncludes (read_img, write_img)) {
            printf ("0\n");
        } else {
            printf ("1\n");
        }
    }

    return (0);
}
