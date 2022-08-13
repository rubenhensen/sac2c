#include "gtest/gtest.h"
#include "base-test-environment.h" // All unit test files need to import this!
testing::Environment* base_test_env = testing::AddGlobalTestEnvironment(new BaseEnvironment);

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
#include "compile.h"
#include "functionprecompile.h"

#include "limits.h"
}

#include "macros.h"

// Create a trivial function that we will pass to compile.c:
// int <fname> (int a) { return a; }
node *make_test_fundef (const char *fname)
{
    node *avis_a = int_avis ("a");

    node * ret_stmt = TBmakeReturn (TBmakeExprs (TBmakeId (avis_a), NULL));
    node * t = TBmakeAssign (ret_stmt, NULL);

    node * ret = TBmakeRet (make_scalar_type (T_int), NULL);
    // This is needed for the compilation phase.
    RET_LINKSIGN (ret) = 0;
    RET_ISREFCOUNTED (ret) = FALSE;

    node * arg = TBmakeArg (avis_a, NULL);
    // This is needed for the compilation phase.
    ARG_LINKSIGN (arg) = 1;

    node * fundef = TBmakeFundef (strdup (fname), NULL, ret,
                                  arg, TBmakeBlock (t, NULL), NULL);
    return fundef;
}

// External declaration to the icm-related function.
extern "C" void PrintND_FUN_DECL (node *, void *);


// This test verifies that when creating a function declaration of
// a simple function, the Print<icm-name> function succeeds.
TEST (CompileICM, ICM_ND_FUN_DECL)
{
    char **argv = (char **)malloc (sizeof (char *));
    argv[0] = strdup ("test");
    GLOBinitializeGlobal (1, argv, TOOL_sac2c, argv[0]);

    node *fn = make_test_fundef ("foo");
    // Kill the body, as it is declaration.
    FUNDEF_BODY (fn) = FREEdoFreeTree (FUNDEF_BODY (fn));
    // Mark it as wrapper, so that we trigger one of the
    // ND_FUN_DEC cases in compile.c
    FUNDEF_ISWRAPPERENTRYFUN (fn) = TRUE;
    // Mark function as declaraion-only.
    FUNDEF_ISEXTERN (fn) = true;

    // Create a stupid argtab of the function
    fn = FPCdoFunctionPrecompile (fn);

    node *fn_icm = COMPdoCompile (fn);

    global.outfile = stdout;
    PrintND_FUN_DECL (ICM_ARGS (FUNDEF_ICMDECL (fn_icm)), NULL);
    printf ("\n");

    // Here is how we can check that our AST is being printed
    // correctly via PRTdoPrint.  But this opens a file, writes
    // to it and executes a lot of unrelated things.
    //global.compiler_subphase = PH_cg_prt;
    //global.targetdir = strdup (".");
    //global.outfilename = strdup ("somename");
    //global.config.cext = strdup (".c");
    //PRTdoPrint (FUNDEF_ICMDECL (fn_icm));

    FREEdoFreeTree (fn_icm);
    free (global.targetdir);
    free (global.outfilename);
    free (global.config.cext);
    free (argv[0]);
    free (argv);
}
