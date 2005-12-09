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

struct VIEW {
    char *name;
    int id;
    struct VIEW *next;
};

struct NAMESPACE {
    char *name;
    char *module;
    int id;
    view_t *view;
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

/*
 * global view id counter
 */
static int nextviewid = 0;

static bool
equalsView (view_t *one, view_t *two)
{
    bool result = TRUE;

    DBUG_ENTER ("equalsView");

    if ((one == NULL) && (two != NULL)) {
        result = FALSE;
    } else if ((one != NULL) && (two == NULL)) {
        result = FALSE;
    } else if (one != two) {
        result = ((one->id == two->id) && (ILIBstringCompare (one->name, two->name))
                  && (equalsView (one->next, two->next)));
    }

    DBUG_RETURN (result);
}

static void
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

static namespace_t *
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

static namespace_t *
FindInPool (const char *module, view_t *view)
{
    namespace_t *result = NULL;
    nspool_t *pos;
    int cnt;

    DBUG_ENTER ("GetFromPool");

    pos = pool;

    for (cnt = 0; cnt < nextid; cnt++) {
        if ((ILIBstringCompare (pos->block[cnt]->module, module))
            && (equalsView (pos->block[cnt]->view, view))) {
            result = pos->block[cnt];
            break;
        }

        if ((cnt % 100) == 99) {
            pos = pos->next;
        }
    }

    DBUG_RETURN (result);
}

static char *
buildNamespaceName (namespace_t *ns)
{
    char *result;
    str_buf *buf;
    view_t *view;

    DBUG_ENTER ("buildNamespaceName");

    buf = ILIBstrBufCreate (255);

    buf = ILIBstrBufPrint (buf, ns->module);

    view = ns->view;

    while (view != NULL) {
        buf = ILIBstrBufPrintf (buf, ":%s", view->name);
        view = view->next;
    }

    result = ILIBstrBuf2String (buf);
    buf = ILIBstrBufFree (buf);

    DBUG_RETURN (result);
}

static namespace_t *
AddNamespaceToPool (const char *module, view_t *view)
{
    namespace_t *new;

    DBUG_ENTER ("AddNamespaceToPool");

    new = ILIBmalloc (sizeof (namespace_t));

    new->module = ILIBstringCopy (module);
    new->id = nextid++;
    new->view = view;
    new->name = buildNamespaceName (new);

    PutInPool (new);

    DBUG_RETURN (new);
}

static view_t *
dupView (const view_t *src)
{
    view_t *result;

    DBUG_ENTER ("dupView");

    if (src == NULL) {
        result = NULL;
    } else {
        result = ILIBmalloc (sizeof (view_t));

        result->id = src->id;
        result->name = ILIBstringCopy (src->name);
        result->next = dupView (src->next);
    }

    DBUG_RETURN (result);
}

static view_t *
makeView (const char *name, const view_t *views)
{
    view_t *result;

    DBUG_ENTER ("makeView");

    result = ILIBmalloc (sizeof (view_t));

    result->name = ILIBstringCopy (name);
    result->id = nextviewid++;
    result->next = dupView (views);

    DBUG_RETURN (result);
}

namespace_t *
NSgetNamespace (const char *module)
{
    namespace_t *result;

    DBUG_ENTER ("NSgetNamespace");

    if (module == NULL) {
        /*
         * the empty namespace is mapped to the empty namespace
         * this merly is for convenience
         */

        result = NULL;
    } else {
        result = FindInPool (module, NULL);

        if (result == NULL) {
            result = AddNamespaceToPool (module, NULL);
        }
    }

    DBUG_RETURN (result);
}

namespace_t *
NSgetRootNamespace ()
{
    static namespace_t *result;

    DBUG_ENTER ("NSgetRootNamespace");

    if (result == NULL) {
        result = NSgetNamespace ("_MAIN");
    }

    DBUG_RETURN (NSdupNamespace (result));
}

namespace_t *
NSgetInitNamespace ()
{
    static namespace_t *initns;

    DBUG_ENTER ("NSgetInitNamespace");

    if (initns == NULL) {
        initns = AddNamespaceToPool (global.modulename, makeView ("_INIT", NULL));
    }

    DBUG_RETURN (NSdupNamespace (initns));
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

    DBUG_RETURN (ns->module);
}

namespace_t *
NSbuildView (const namespace_t *orig)
{
    namespace_t *result;
    view_t *view;

    DBUG_ENTER ("NSbuildView");

    view = makeView (orig->name, orig->view);

    result = AddNamespaceToPool (global.modulename, view);

    DBUG_RETURN (result);
}

bool
NSisView (const namespace_t *ns)
{
    bool result;

    DBUG_ENTER ("NSisView");

    result = (ns->view != NULL);

    DBUG_RETURN (result);
}

/*
 * serialisation support
 */

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

view_t *
NSdeserializeView (const char *name, int id, view_t *next)
{
    view_t *result;

    DBUG_ENTER ("NSdeserializeView");

    result = ILIBmalloc (sizeof (view_t));

    result->name = ILIBstringCopy (name);
    result->id = id;
    result->next = next;

    DBUG_RETURN (result);
}

int
NSaddMapping (const char *module, view_t *view)
{
    namespace_t *ns;
    int result;

    DBUG_ENTER ("NSaddMapping");

    DBUG_PRINT ("NS", ("adding new mapping for '%s'...", module));

    ns = FindInPool (module, view);

    if (ns == NULL) {
        ns = AddNamespaceToPool (module, view);
    }

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
GenerateViewConstructor (FILE *file, view_t *view)
{
    DBUG_ENTER ("GenerateViewConstructor");

    if (view == NULL) {
        fprintf (file, "NULL");
    } else {
        fprintf (file, "NSdeserializeView( \"%s\", %d, ", view->name, view->id);

        GenerateViewConstructor (file, view->next);

        fprintf (file, ")");
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
        fprintf (file, "MAPNS(%d) = NSaddMapping( \"%s\",", cnt,
                 NSgetModule (pool->block[cnt % 100]));

        GenerateViewConstructor (file, pool->block[cnt % 100]->view);

        fprintf (file, ");\n");

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
