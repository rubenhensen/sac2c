/**
 * @file namespaces.c
 * @brief function and type definitions for namespaces as uses throughout
 *        the compiler.
 * @author Stephan Herhut
 * @date 2005-07-11
 */

#include "namespaces.h"
#include "dbug.h"
#include "LookUpTable.h"
#include "internal_lib.h"

struct NAMESPACE {
    char *name;
};

/*
 * this LookUpTable is used to match namespace names
 * to namespace records, as these are implicitly
 * shared.
 */
static lut_t *namespaces = NULL;

namespace_t *
AddNamespaceToPool (const char *name)
{
    namespace_t *new;

    DBUG_ENTER ("AddNamespaceToPool");

    new = ILIBmalloc (sizeof (namespace_t));

    new->name = ILIBstringCopy (name);

    if (namespaces == NULL) {
        namespaces = LUTgenerateLut ();
    }

    namespaces = LUTinsertIntoLutS (namespaces, (char *)name, new);

    DBUG_RETURN (new);
}

namespace_t *
NSgetNamespace (const char *name)
{
    namespace_t *result;
    void **search = NULL;

    DBUG_ENTER ("NSgetNamespace");

    if (name == NULL) {
        /*
         * the empty namespace is mapped to the empty namespace
         * this merly is for convenience
         */

        result = NULL;
    } else {
        if (namespaces != NULL) {
            search = LUTsearchInLutS (namespaces, (char *)name);
        }

        if (search == NULL) {
            result = AddNamespaceToPool (name);
        } else {
            result = (namespace_t *)*search;
        }
    }

    DBUG_RETURN (result);
}

namespace_t *
NSgetRootNamespace ()
{
    namespace_t *result;

    DBUG_ENTER ("NSgetRootNamespace");

    result = NSgetNamespace ("_MAIN");

    DBUG_RETURN (result);
}

namespace_t *
NSdupNamespace (const namespace_t *ns)
{
    namespace_t *result;

    DBUG_ENTER ("NSdupNamespace");

    result = (namespace_t *)ns;

    DBUG_RETURN (result);
}

namespace_t *
NSfreeNamespace (namespace_t *ns)
{
    DBUG_ENTER ("NSfreeNamespace");

    ns = NULL;

    DBUG_RETURN (ns);
}

bool
NSequals (const namespace_t *one, const namespace_t *two)
{
    bool result;

    DBUG_ENTER ("NSequals");

    result = (one == two);

    DBUG_RETURN (result);
}

const char *
NSgetName (const namespace_t *ns)
{
    const char *result;

    DBUG_ENTER ("NSgetName");

    if (ns == NULL) {
        result = "--";
    } else {
        result = ns->name;
    }

    DBUG_RETURN (result);
}

const char *
NSgetModule (const namespace_t *ns)
{
    DBUG_ENTER ("NSgetModule");

    DBUG_ASSERT ((ns != NULL), "called NSgetModule with argument NULL!");

    DBUG_RETURN (ns->name);
}

void
NSserializeNamespace (FILE *file, const namespace_t *ns)
{
    DBUG_ENTER ("NSserializeNamespace");

    fprintf (file, "NSdeserializeNamespace( \"%s\")", NSgetName (ns));

    DBUG_VOID_RETURN;
}

namespace_t *
NSdeserializeNamespace (char *name)
{
    namespace_t *result;

    DBUG_ENTER ("NSdeserializeNamespace");

    result = NSgetNamespace (name);

    DBUG_RETURN (result);
}
