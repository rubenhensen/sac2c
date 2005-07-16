/**
 * @file namespaces.c
 * @brief function and type definitions for namespaces as uses throughout
 *        the compiler.
 * @author Stephan Herhut
 * @date 2005-07-11
 */

#include "namespaces.h"
#include "dbug.h"
#include "internal_lib.h"
#include "filemgr.h"

struct NAMESPACE {
    char *name;
    int id;
};

struct NSPOOL {
    namespace_t *block[100];
    struct NSPOOL *next;
};

typedef struct NSPOOL nspool_t;

/*
 * this pool is used to keep the namespaces
 * ordered by their id
 */
static nspool_t *pool = NULL;

/*
 * global namespace id counter
 */
static int nextid = 0;

void
PutInPool (namespace_t *ns)
{
    int cnt;
    nspool_t *pos;

    DBUG_ENTER ("PutInPool");

    /*
     * create initial pool if necessary
     */
    if (pool == NULL) {
        pool = ILIBmalloc (sizeof (nspool_t));
    }

    /*
     * go to block
     */
    pos = pool;

    for (cnt = 0; cnt < (ns->id / 100); cnt++) {
        if (pos->next == NULL) {
            pos->next = ILIBmalloc (sizeof (nspool_t));
        }

        pos = pos->next;
    }

    /*
     * insert ns
     */
    pos->block[ns->id % 100] = ns;

    DBUG_VOID_RETURN;
}

namespace_t *
GetFromPool (int id)
{
    namespace_t *result;
    nspool_t *pos;
    int cnt;

    DBUG_ENTER ("GetFromPool");

    pos = pool;

    for (cnt = 0; cnt < (id / 100); cnt++) {
        pos = pos->next;
    }

    result = pos->block[id % 100];

    DBUG_RETURN (result);
}

namespace_t *
FindInPool (const char *name)
{
    namespace_t *result = NULL;
    nspool_t *pos;
    int cnt;

    DBUG_ENTER ("GetFromPool");

    pos = pool;

    for (cnt = 0; cnt < nextid; cnt++) {
        if (ILIBstringCompare (pos->block[cnt]->name, name)) {
            result = pos->block[cnt];
            break;
        }

        if ((cnt % 100) == 99) {
            pos = pos->next;
        }
    }

    DBUG_RETURN (result);
}

namespace_t *
AddNamespaceToPool (const char *name)
{
    namespace_t *new;

    DBUG_ENTER ("AddNamespaceToPool");

    new = ILIBmalloc (sizeof (namespace_t));

    new->name = ILIBstringCopy (name);
    new->id = nextid++;

    PutInPool (new);

    DBUG_RETURN (new);
}

namespace_t *
NSgetNamespace (const char *name)
{
    namespace_t *result;

    DBUG_ENTER ("NSgetNamespace");

    if (name == NULL) {
        /*
         * the empty namespace is mapped to the empty namespace
         * this merly is for convenience
         */

        result = NULL;
    } else {
        result = FindInPool ((char *)name);

        if (result == NULL) {
            result = AddNamespaceToPool (name);
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

    /*
     * the MAPNS macro is defined by serialize.c
     * to use the namespace mapping table
     * generated during deserialisation
     */
    fprintf (file, "NSdeserializeNamespace( MAPNS( %d))", ns->id);

    DBUG_VOID_RETURN;
}

namespace_t *
NSdeserializeNamespace (int id)
{
    namespace_t *result;

    DBUG_ENTER ("NSdeserializeNamespace");

    result = NSdupNamespace (GetFromPool (id));

    DBUG_RETURN (result);
}

int
NSaddMapping (const char *name)
{
    namespace_t *ns;
    int result;

    DBUG_ENTER ("NSaddMapping");

    DBUG_PRINT ("NS", ("adding new mapping for '%s'...", name));

    ns = NSgetNamespace (name);

    result = ns->id;

    DBUG_PRINT ("NS", ("...mapped to %d.", result));

    DBUG_RETURN (result);
}

static void
GenerateNamespaceMapHead (FILE *file)
{
    DBUG_ENTER ("GenerateNamespaceMapHead");

    fprintf (file, "/* namespace mapping generated by sac2c %s */\n\n",
             global.version_id);
    fprintf (file, "#include \"sac_serialize.h\"\n\n");
    fprintf (file, "#include \"namespacemap.h\"\n\n");

    DBUG_VOID_RETURN;
}

static void
GenerateNamespaceMapDeclaration (FILE *file)
{
    int cnt;

    DBUG_ENTER ("GenerateNamespaceMapDeclaration");

    for (cnt = 0; cnt < nextid; cnt++) {
        fprintf (file, "int __%s__nsmap_%d = 0;\n ", global.modulename, cnt);
    }

    DBUG_VOID_RETURN;
}

static void
GenerateNamespaceMappingConstructor (FILE *file)
{
    nspool_t *pos;
    int cnt;

    DBUG_ENTER ("GenerateNamespaceMappingConstructor");

    pos = pool;

    fprintf (file, "void __%s__MapConstructor() {\n", global.modulename);

    for (cnt = 0; cnt < nextid; cnt++) {
        fprintf (file, "MAPNS(%d) = NSaddMapping( \"%s\");\n", cnt,
                 NSgetName (pool->block[cnt % 100]));

        if ((cnt % 100) == 99) {
            pos = pos->next;
        }
    }

    fprintf (file, "}\n");

    DBUG_VOID_RETURN;
}

static void
GenerateNamespaceMappingHeader (FILE *file)
{
    int cnt;

    DBUG_ENTER ("GenerateNamespaceMappingHeader");

    fprintf (file, "#ifndef _NAMESPACEMAP_H_\n#define _NAMESPACEMAP_H_\n\n");

    fprintf (file, "#define MAPNS( x)  __%s__nsmap_##x\n\n", global.modulename);

    for (cnt = 0; cnt < nextid; cnt++) {
        fprintf (file, "extern int __%s__nsmap_%d;\n ", global.modulename, cnt);
    }

    fprintf (file, "#endif\n");

    DBUG_VOID_RETURN;
}

void
NSgenerateNamespaceMap ()
{
    FILE *file;

    DBUG_ENTER ("NSgenerateNamespaceMap");

    file = FMGRwriteOpen ("%s/namespacemap.c", global.tmp_dirname);

    GenerateNamespaceMapHead (file);

    GenerateNamespaceMapDeclaration (file);

    GenerateNamespaceMappingConstructor (file);

    fclose (file);

    file = FMGRwriteOpen ("%s/namespacemap.h", global.tmp_dirname);

    GenerateNamespaceMappingHeader (file);

    fclose (file);

    DBUG_VOID_RETURN;
}
