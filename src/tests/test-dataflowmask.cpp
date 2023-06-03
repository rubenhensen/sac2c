#include "gtest/gtest.h"
#include "base-test-environment.h" // All unit test files need to import this!
testing::Environment* base_test_env = testing::AddGlobalTestEnvironment(new BaseEnvironment);

#include "config.h"

extern "C" {
#include "DataFlowMask.h"

#include "types.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "memory.h"
#include "str.h"
#include "DupTree.h"
}

#define NUM_AVIS 10

static void
freeNodes (node *args, node *vardecs)
{
    node *tmp;

    FREEdoFreeTree (args);
    while (vardecs != NULL) {
        tmp = VARDEC_NEXT (vardecs);
        MEMfree (vardecs);
        vardecs = tmp;
    }
}

class DFMTest : public ::testing::Test {
    protected:
#if __cplusplus >= 201103L
        void SetUp() override
#else
        void SetUp()
#endif
        {
            node *avis;
            ntype *type;
            char numstr[32];

            args = NULL;
            vardecs = NULL;
            for (int i = 0; i < NUM_AVIS; i++) {
                type = TYmakeSimpleType (T_int);
                snprintf (numstr, 32, "val_%d", i);
                avis = TBmakeAvis (STRncpy (numstr, 32), type);
                vardecs = TBmakeVardec (avis, vardecs);
                AVIS_DECL (avis) = vardecs;
                args = TBmakeArg (avis, args);
            }
        }
#if __cplusplus >= 201103L
        void TearDown() override
#else
        void TearDown()
#endif
        {
            freeNodes (args, vardecs);
        }

    node *args;
    node *vardecs;
};

TEST_F (DFMTest, CreateMaskBase)
{
    dfmask_base_t *base;

    base = DFMgenMaskBase (args, vardecs);

    ASSERT_EQ (DFMnumDecls (base), NUM_AVIS * 2);
    ASSERT_EQ (DFMnumBitFields (base), (NUM_AVIS / (sizeof (unsigned int) * 8)) + 1);

    DFMremoveMaskBase (base);
}

TEST_F (DFMTest, UpdateMaskBase)
{
    dfmask_base_t *base;
    node *avis, *nargs, *nvardecs;

    // copy nodes and update them
    nargs = DUPdoDupTree (args);
    nvardecs = DUPdoDupTree (vardecs);

    base = DFMgenMaskBase (nargs, nvardecs);

    avis = TBmakeAvis (STRcpy ("random1"), TYmakeSimpleType (T_int));
    nvardecs = TBmakeVardec (avis, nvardecs);
    AVIS_DECL (avis) = nvardecs;
    nargs = TBmakeArg (avis, nargs);

    base = DFMupdateMaskBase (base, nargs, nvardecs);

    ASSERT_EQ (DFMnumDecls (base), (NUM_AVIS + 1) * 2);
    ASSERT_EQ (DFMnumBitFields (base), ((NUM_AVIS + 1) / (sizeof (unsigned int) * 8)) + 1);

    DFMremoveMaskBase (base);
    freeNodes (nargs, nvardecs);
}

#undef NUM_AVIS
