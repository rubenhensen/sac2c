#include "gtest/gtest.h"

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

#include "err.h"

// Some convenience macros.
#define make_vec_type(__x) TYmakeAKD (TYmakeSimpleType (__x), 1, SHmakeShape (0))
#define make_scalar_type(__x) TYmakeAKS (TYmakeSimpleType (__x), SHmakeShape (0))

#define int_akv_avis(__n, __v) TBmakeAvis (strdup (__n), \
        TYmakeAKV (TYmakeSimpleType (T_int), COmakeConstantFromInt (__v))) 
#define int_avis(__x) TBmakeAvis (strdup (__x), make_scalar_type (T_int))
#define int_vec_avis(__x) TBmakeAvis (strdup (__x), make_vec_type (T_int))

#define binary_prf(prf, arg1, arg2)                                                      \
    TBmakePrf (prf, TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)))
#define make_let(avis, rhs) TBmakeLet (TBmakeIds (avis, NULL), rhs)

extern "C" node *
SetupCompiler (int argc, char *argv[], tool_t tool, char *toolname);


static char *
read_from_fd (int fd)
{
    char buf[1024];
    ssize_t bytes_read;
    char *str = NULL;
    size_t len = 0;

    while (bytes_read = read (fd, buf, sizeof buf)) {
        if (bytes_read < 0) {
            if (errno == EAGAIN)
                continue;
            err (EXIT_FAILURE, "read");
            break;
        }
        str = (char *)realloc (str, len + bytes_read + 1);
        memcpy (str + len, buf, bytes_read);
        len += bytes_read;
        str[len] = '\0';
    }

    return str;
}


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
    GLOBinitializeGlobal (0, NULL, TOOL_sac2c, "test");


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

    int pipefd[2];
    char *s;

    pipe(pipefd);

    // FIXME(artem) I don't see an easy way to compare results of the
    //              traversal other than comparing its string output.
    //              However, as we only can print to a file, I pipe
    //              the stdout into the child process...  Improvements
    //              are welcome.
    if (fork () == 0) {
        close(pipefd[0]);

        dup2(pipefd[1], 1);
        dup2(pipefd[1], 2);

        close(pipefd[1]);

        // Print the block of the function to stderr.
        node * _2 = PRTdoPrintFile (stderr, FUNDEF_BODY (nfundef));
        fundef = FREEdoFreeTree (fundef);
        fundef = FREEdoFreeTree (nfundef);
    } else  {
        close(pipefd[1]);
        // Catch the output of the child process in the parent
        // process and store it in a string.
        s = read_from_fd (pipefd[0]);
    }
    
    // The output should be identical to the program we defined above.
    EXPECT_STREQ (s, out);
    free (s);
}
