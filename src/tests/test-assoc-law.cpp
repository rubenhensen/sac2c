#include <cstdlib>

#include "gtest/gtest.h"
#include "base-test-environment.h" // All unit test files need to import this!

extern "C" {
#include "options.h"
#include "types.h"
#include "new_types.h"
#include "shape.h"
#include "tree_basic.h"
#include "globals.h"
#include "associative_law.h"
#include "print.h"
#include "free.h"
#include "resource.h"
#include "sacdirs.h"
#define DBUG_PREFIX "AL-TEST"
#include "debug.h"
#include "constants.h"

#include "limits.h"
}

#include "macros.h"


static const char out[] =
"\n"
"/*-----------------------------------------------*/\n"
"{ \n"
"  int[.] _al_4; \n"
"  int _al_3; \n"
"  int[.] _al_2; \n"
"  int _al_1; \n"
"  int{2} _al_0; \n"
"\n"
"  o = 1; \n"
"  x = _add_SxS_( a, o); \n"
"  y = _add_VxS_( c, o); \n"
"  z = _add_SxV_( b, d); \n"
"  r = _add_SxV_( x, y); \n"
"  _al_0 = _add_SxS_( o, o); \n"
"  _al_1 = _add_SxS_( a, b); \n"
"  _al_2 = _add_VxV_( c, d); \n"
"  _al_3 = _add_SxS_( _al_0, _al_1); \n"
"  _al_4 = _add_SxV_( _al_3, _al_2); \n"
"  s = _al_4; \n"
"  return( s); \n"
"}\n"
"/*-----------------------------------------------*/\n"
;




TEST (AssociativeLaw, test01)
{
    // XXX(artem) This is how you can trigger debug output of
    //            a traversal.  Can be very useful when writing
    //            tests, but not needed when we run the test.
    //
    //_db_on_ = 1;
    //_db_push_ ("-#d,AL");
    char **argv = (char **)malloc (sizeof (char *));
    argv[0] = strdup ("test");
    GLOBinitializeGlobal (1, argv, TOOL_sac2c, argv[0]);

    // This is the program that we want to create and
    // run AL on.  This is a direct copy from the comments
    // in AL.
    //
    // int[.] foo (int a, int b, int[.] c, int[.] d) {
    //    x = a _add_SxS_ 1;
    //    y = c _add_VxS_ 1;
    //    z = b _add_SxV_ d;
    //
    //    r = x + y
    //    s = r + z
    //    return s;
    // }

    node *avis_a = int_avis ("a");
    node *avis_b = int_avis ("b");
    node *avis_c = int_vec_avis ("c");
    node *avis_d = int_vec_avis ("d");
    node *avis_x = int_avis ("x");
    node *avis_y = int_vec_avis ("y");
    node *avis_z = int_vec_avis ("z");

    node *avis_s = int_vec_avis ("s");
    node *avis_r = int_vec_avis ("r");

    // This is the key to the optimisation, if it would have been of a
    // normal akd type, this wouldn't work.
    node *avis_o = int_akv_avis ("o", 1);


    // Now we just compose block statements in the reverse order.
    node * ret_stmt = TBmakeReturn (TBmakeExprs (TBmakeId (avis_s), NULL));
    node * t = TBmakeAssign (ret_stmt, NULL);

    node * s_let = make_let (avis_s, binary_prf (F_add_VxV, TBmakeId (avis_r), TBmakeId (avis_z)));
    t = AVIS_SSAASSIGN (avis_s) = TBmakeAssign (s_let, t);

    node * r_let = make_let (avis_r, binary_prf (F_add_SxV, TBmakeId (avis_x), TBmakeId (avis_y)));
    t = AVIS_SSAASSIGN (avis_r) = TBmakeAssign (r_let, t);

    node * z_let = make_let (avis_z, binary_prf (F_add_SxV, TBmakeId (avis_b), TBmakeId (avis_d)));
    t = AVIS_SSAASSIGN (avis_z) = TBmakeAssign (z_let, t);

    node * y_let = make_let (avis_y, binary_prf (F_add_VxS, TBmakeId (avis_c), TBmakeId (avis_o)));
    t = AVIS_SSAASSIGN (avis_y) = TBmakeAssign (y_let, t);

    node * x_let = make_let (avis_x, binary_prf (F_add_SxS, TBmakeId (avis_a), TBmakeId (avis_o)));
    t = AVIS_SSAASSIGN (avis_x) = TBmakeAssign (x_let, t);

    node * o_let = make_let (avis_o, TBmakeNum (1));
    t = AVIS_SSAASSIGN (avis_o) = TBmakeAssign (o_let, t);

    // Make a function foo.
    node * fundef
      = TBmakeFundef (strdup ("foo"), NULL,
                      TBmakeRet (make_vec_type (T_int), NULL),
                      TBmakeArg (avis_a,
                                 TBmakeArg (avis_b,
                                            TBmakeArg (avis_c, TBmakeArg (avis_d, NULL)))),
                      TBmakeBlock (t, NULL), NULL);

    // Call the traversal.
    node * nfundef = ALdoAssocLawOptimization (fundef);

    size_t len;
    char *s;
    FILE *f = open_memstream (&s, &len);

    node * _2 = PRTdoPrintFile (f, FUNDEF_BODY (nfundef));
    fundef = FREEdoFreeTree (fundef);
    fundef = FREEdoFreeTree (nfundef);
    fclose (f);

    // The output should be identical to the program we defined above.
    EXPECT_STREQ (s, out);
    free (s);
}
