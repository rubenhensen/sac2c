#include "gtest/gtest.h"
#include "config.h"

extern "C" {
#include "new_types.h"
#define DBUG_PREFIX "TEST-NTY"
#include "debug.h"
}

TEST (NEW_TYPES, FreeType)
{
    ntype *simp = NULL;

    simp = TYmakeSimpleType (T_int);
    ASSERT_FALSE (simp == NULL);

    /* running this through valgrind shows that we leak memory.
     * a closer look at TYfreeTypeConstructor shows that when
     * dealing with a SimpleType, we never free it... instead we
     * set the ntype to NULL (WTF?!). Evdiently this has something to do
     * with sharing...
     */
    simp = TYfreeType (simp);
    ASSERT_TRUE (simp == NULL);
}
