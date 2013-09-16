

/*
 * This file is used to build libcygcompat.a in $SAC2CBASE/lib. You can find the
 * rule for building this in build.mkv under the rules for libsac2c.
 *
 * The purpose of this library is to allow an interface between sac2c and a
 * compiled sac module's Tree library that satisfies the need to have all symbols
 * defined at link time under windows, and ensures that both the product version
 * and developer version of sac2c are compatible.
 *
 * It works by housing a list of dummy functions in libcygcompat.a which ensures
 * that the symbols get defined when building the Tree library. Each dummy
 * function simply calls a function pointer which corresponds with the function
 * in the currently loaded libsac2c library (either libsac2c.p.dll or
 * libsac2c.d.dll). The loaded library must somehow have access to these function
 * pointers. When each Tree library is loaded by sac2c it is passed a
 * pointer to a global struct containing these function pointers. This means at
 * runtime when the compiled module calls back to libsac2c it calls back to the
 * correct function.
 *
 * The downside to this approach is that a conclusive list of functions needs to
 * be maintained. This turns out not to be that difficult because relatively few
 * functions are called from a Tree lib.
 *
 * If when compiling a sac module you discover that lib[modulename]Tree.dll fails
 * to compile because of an undefined symbol, then you must add that function to
 * the list. This requires 3 simple additions:
 *  1. In types.h: add a function pointer declaration to the CYG_FUN_TABLE struct
 *     for the function in question. e.g int (*FOObar_fp)( void);
 *  2. In cygwinhelpers.c: in the CYGHinitFunTable function add an entry to assign
 *     the function pointer with the correct value. e.g tab->FOObar_fp = &FOObar;
 *  3. In this file add a dummy function which calls the function pointer.
 *
 * In the case that the function in question takes a variable number of arguments
 * (i.e ...) you cannot pass the arguments directly to that function from the dummy
 * function. You need to add a function which takes a va_list as an argument
 * instead. You only want this to occur under cygwin because in any other case it
 * is unnecessary and wasteful. See SHcreateShape in shape.c as an example.
 *
 * NOTES:
 * Each function pointer in the table has the suffix `_fp' to show that it is a
 * function pointer and nothing else.
 */

#include "cygcompat.h"

#if IS_CYGWIN
#include "types.h"

cyg_fun_table_t *table = NULL;
#define CALL_FROM_TABLE(_tocall_) table->_tocall_

/** <!--********************************************************************-->
 *
 * @fn void CYGFpassFunTable( cyg_fun_table_t *tab)
 *
 * @brief used to pass the list of function pointers to the library after
 *        a dlopen call.
 *
 *****************************************************************************/
void
CYGFpassFunTable (cyg_fun_table_t *tab)
{
    table = tab;
}

/*
 * These functions are dummies. Each should only call
 * their corresponding function pointer from the table.
 */

char *
STRcpy (const char *source)
{
    return CALL_FROM_TABLE (STRcpy_fp (source));
}

shape *
SHcreateShape (int dim, ...)
{
    shape *result;
    va_list Argp;

    va_start (Argp, dim);
    result = CALL_FROM_TABLE (SHcreateShapeVa_fp (dim, Argp));
    va_end (Argp);

    return result;
}

ntype *
TYdeserializeType (int _con, ...)
{
    ntype *result = NULL;
    va_list Argp;

    va_start (Argp, _con);
    result = CALL_FROM_TABLE (TYdeserializeTypeVa_fp (_con, Argp));
    va_end (Argp);

    return (result);
}

namespace_t *
NSdeserializeNamespace (int id)
{
    return CALL_FROM_TABLE (NSdeserializeNamespace_fp (id));
}

int
NSaddMapping (const char *module, void *_view)
{
    return CALL_FROM_TABLE (NSaddMapping_fp (module, _view));
}

void *
NSdeserializeView (const char *module, int id, void *_next)
{
    return CALL_FROM_TABLE (NSdeserializeView_fp (module, id, _next));
}

node *
SHLPmakeNode (int _node_type, char *sfile, size_t lineno, size_t col, ...)
{
    node *result;
    va_list Argp;

    va_start (Argp, col);
    result = CALL_FROM_TABLE (SHLPmakeNodeVa_fp (_node_type, sfile, lineno, col, Argp));
    va_end (Argp);

    return result;
}

void
SHLPfixLink (serstack_t *stack, int from, int no, int to)
{
    CALL_FROM_TABLE (SHLPfixLink_fp (stack, from, no, to));
}

serstack_t *
SERbuildSerStack (node *arg_node)
{
    return CALL_FROM_TABLE (SERbuildSerStack_fp (arg_node));
}

constant *
COdeserializeConstant (simpletype type, shape *shp, int vlen, char *vec)
{
    return CALL_FROM_TABLE (COdeserializeConstant_fp (type, shp, vlen, vec));
}

node *
DSlookupFunction (const char *module, const char *symbol)
{
    return CALL_FROM_TABLE (DSlookupFunction_fp (module, symbol));
}

node *
DSlookupObject (const char *module, const char *symbol)
{
    return CALL_FROM_TABLE (DSlookupObject_fp (module, symbol));
}

node *
DSfetchArgAvis (int pos)
{
    return CALL_FROM_TABLE (DSfetchArgAvis_fp (pos));
}

double
DShex2Double (const char *string)
{
    return CALL_FROM_TABLE (DShex2Double_fp (string));
}

float
DShex2Float (const char *string)
{
    return CALL_FROM_TABLE (DShex2Float_fp (string));
}

sttable_t *
STinit (void)
{
    return CALL_FROM_TABLE (STinit_fp ());
}

void
STadd (const char *symbol, int visbility, const char *name, int type, void *tab,
       unsigned argc)
{
    CALL_FROM_TABLE (STadd_fp (symbol, visbility, name, type, tab, argc));
}

stringset_t *
STRSadd (const char *string, strstype_t kind, stringset_t *set)
{
    return CALL_FROM_TABLE (STRSadd_fp (string, kind, set));
}
#else  /* IS_CYGWIN */
static UNUSED int dummy; /* Silence empty source file warning. */
#endif /* IS_CYGWIN */
