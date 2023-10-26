/**
 *
 * @file namespaces.c
 * @brief function and type definitions for namespaces as uses throughout
 *        the compiler.
 * @author Stephan Herhut
 * @date 2005-07-11
 */

#include "namespaces.h"
#define DBUG_PREFIX "NS"
#include "debug.h"
#include "str_buffer.h"
#include "str.h"
#include "memory.h"
#include "filemgr.h"
#include "globals.h"

#define BLOCKSIZE 128

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
    namespace_t *block[BLOCKSIZE];
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
EqualsView (view_t *one, view_t *two)
{
    bool result = TRUE;

    DBUG_ENTER ();

    if (one == NULL && two != NULL)
        result = FALSE;
    else if (one != NULL && two == NULL)
        result = FALSE;
    else if (one != two)
        result = (one->id == two->id && STReq (one->name, two->name)
                  && EqualsView (one->next, two->next));

    DBUG_RETURN (result);
}

static nspool_t *
CreatePool (void)
{
    nspool_t *result;

    DBUG_ENTER ();

    result = (nspool_t *)MEMmalloc (sizeof (nspool_t));
    result->next = NULL;

    DBUG_RETURN (result);
}

static void
PutInPool (namespace_t *ns)
{
    int cnt;
    nspool_t *pos;

    DBUG_ENTER ();

    /*
     * create initial pool if necessary
     */
    if (pool == NULL)
        pool = CreatePool ();

    /*
     * go to block
     */
    pos = pool;

    for (cnt = 0; cnt < (ns->id / BLOCKSIZE); cnt++) {
        if (pos->next == NULL)
            pos->next = CreatePool ();

        pos = pos->next;
    }

    /*
     * insert ns
     */
    pos->block[ns->id % BLOCKSIZE] = ns;

    DBUG_RETURN ();
}

static namespace_t *
GetFromPool (int id)
{
    namespace_t *result;
    nspool_t *pos;
    int cnt;

    DBUG_ENTER ();

    pos = pool;

    for (cnt = 0; cnt < (id / BLOCKSIZE); cnt++)
        pos = pos->next;

    result = pos->block[id % BLOCKSIZE];

    DBUG_RETURN (result);
}

#if 0
static void
RemoveFromPool (int id)
{
  nspool_t *  p;
  size_t i;

  for (p = pool, i = 0; i < id / BLOCKSIZE; i++, p = p->next)
    ;

  p->block[id % BLOCKSIZE] = NULL;
}
#endif

static namespace_t *
FindInPool (const char *module, view_t *view)
{
    namespace_t *result = NULL;
    nspool_t *pos;
    int cnt;

    DBUG_ENTER ();

    pos = pool;

    for (cnt = 0; cnt < nextid; cnt++) {
        int idx = cnt % BLOCKSIZE;

        if (pos->block[idx] && STReq (pos->block[idx]->module, module)
            && EqualsView (pos->block[idx]->view, view)) {
            result = pos->block[idx];
            break;
        }

        if ((cnt % BLOCKSIZE) == (BLOCKSIZE - 1))
            pos = pos->next;
    }

    DBUG_RETURN (result);
}

static char *
BuildNamespaceName (namespace_t *ns)
{
    char *result;
    str_buf *buf;
    view_t *view;

    DBUG_ENTER ();

    buf = SBUFcreate (255);
    buf = SBUFprint (buf, ns->module);

    view = ns->view;

    while (view != NULL) {
        buf = SBUFprintf (buf, ":%s", view->name);
        view = view->next;
    }

    result = SBUF2str (buf);
    buf = SBUFfree (buf);

    DBUG_RETURN (result);
}

static namespace_t *
AddNamespaceToPool (const char *module, view_t *view)
{
    namespace_t *xnew;

    DBUG_ENTER ();

    xnew = (namespace_t *)MEMmalloc (sizeof (namespace_t));
    xnew->module = STRcpy (module);
    xnew->id = nextid++;
    xnew->view = view;
    xnew->name = BuildNamespaceName (xnew);

    PutInPool (xnew);

    DBUG_RETURN (xnew);
}

static view_t *
DupView (const view_t *src)
{
    view_t *result;

    DBUG_ENTER ();

    if (src == NULL)
        result = NULL;
    else {
        result = (view_t *)MEMmalloc (sizeof (view_t));
        result->id = src->id;
        result->name = STRcpy (src->name);
        result->next = DupView (src->next);
    }

    DBUG_RETURN (result);
}

static view_t *
FreeView (view_t *view)
{
    DBUG_ENTER ();

    if (view != NULL) {
        view->id = 0;
        view->name = MEMfree (view->name);
        view->next = FreeView (view->next);
        view = MEMfree (view);
    }

    DBUG_RETURN (view);
}

static view_t *
MakeView (const char *name, const view_t *views)
{
    view_t *result;

    DBUG_ENTER ();

    result = (view_t *)MEMmalloc (sizeof (view_t));
    result->name = STRcpy (name);
    result->id = nextviewid++;
    result->next = DupView (views);

    DBUG_RETURN (result);
}

namespace_t *
NSgetNamespace (const char *module)
{
    namespace_t *result;

    DBUG_ENTER ();

    if (module == NULL)
        /*
         * the empty namespace is mapped to the empty namespace
         * this merly is for convenience
         */

        result = NULL;
    else {
        result = FindInPool (module, NULL);

        if (result == NULL)
            result = AddNamespaceToPool (module, NULL);
    }

    DBUG_RETURN (result);
}

namespace_t *
NSgetRootNamespace (void)
{
    static namespace_t *result = NULL;

    DBUG_ENTER ();

    if (result == NULL)
        result = NSgetNamespace ("_MAIN");

    DBUG_RETURN (NSdupNamespace (result));
}

namespace_t *
NSgetInitNamespace (void)
{
    static namespace_t *initns;

    DBUG_ENTER ();

    if (initns == NULL)
        initns = AddNamespaceToPool (global.modulename, MakeView ("_INIT", NULL));

    DBUG_RETURN (NSdupNamespace (initns));
}

namespace_t *
NSgetCWrapperNamespace (void)
{
    static namespace_t *result;

    DBUG_ENTER ();

    if (result == NULL)
        result = NSgetNamespace ("_CWRAPPER");

    DBUG_RETURN (NSdupNamespace (result));
}

namespace_t *
NSgetMTNamespace (const namespace_t *orig)
{
    namespace_t *result;
    view_t *view;

    DBUG_ENTER ();

    view = MakeView ("_MT", orig->view);

    result = FindInPool (orig->name, view);

    if (result != NULL)
        view = FreeView (view);
    else
        result = AddNamespaceToPool (orig->module, view);

    DBUG_RETURN (result);
}

namespace_t *
NSgetSTNamespace (const namespace_t *orig)
{
    namespace_t *result;
    view_t *view;

    DBUG_ENTER ();

    view = MakeView ("_ST", orig->view);

    result = FindInPool (orig->name, view);

    if (result != NULL)
        view = FreeView (view);
    else
        result = AddNamespaceToPool (orig->module, view);

    DBUG_RETURN (result);
}

namespace_t *
NSgetXTNamespace (const namespace_t *orig)
{
    namespace_t *result;
    view_t *view;

    DBUG_ENTER ();

    view = MakeView ("_XT", orig->view);

    result = FindInPool (orig->name, view);

    if (result != NULL) {
        view = FreeView (view);
    } else {
        result = AddNamespaceToPool (orig->module, view);
    }

    DBUG_RETURN (result);
}

namespace_t *
NSdupNamespace (namespace_t *ns)
{
    DBUG_ENTER ();

    DBUG_RETURN (ns);
}

namespace_t *
NSfreeNamespace (namespace_t *ns)
{
    /* int id; */

    DBUG_ENTER ();

    /* XXX Why it is not being freed properly?  */
    /* ns = NULL; */

    /*if (ns)
      {
        int id = ns->id;
        xfree_namespace (ns);
        RemoveFromPool (id);
      }*/

    DBUG_RETURN (NULL);
}

void
NStouchNamespace (namespace_t *ns, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN ();
}

bool
NSequals (const namespace_t *one, const namespace_t *two)
{
    bool result;

    DBUG_ENTER ();

    result = (one == two);

    DBUG_RETURN (result);
}

const char *
NSgetName (const namespace_t *ns)
{
    const char *result;

    DBUG_ENTER ();

    if (ns == NULL)
        result = "--";
    else
        result = ns->name;

    DBUG_RETURN (result);
}

const char *
NSgetModule (const namespace_t *ns)
{
    DBUG_ENTER ();

    DBUG_ASSERT (ns != NULL, "called NSgetModule with argument NULL!");

    DBUG_RETURN (ns->module);
}

namespace_t *
NSbuildView (const namespace_t *orig)
{
    namespace_t *result;
    view_t *view;

    DBUG_ENTER ();

    view = MakeView (orig->name, orig->view);

    result = FindInPool (global.modulename, view);

    if (result != NULL)
        /*
         * we reuse the view constructed earlier
         */
        view = FreeView (view);
    else
        result = AddNamespaceToPool (global.modulename, view);

    DBUG_RETURN (result);
}

/*
 * serialisation support
 */

void
NSserializeNamespace (FILE *file, const namespace_t *ns)
{
    DBUG_ENTER ();

    /*
     * the MAPNS macro is defined by serialize.c
     * to use the namespace mapping table
     * generated during deserialisation
     */
    fprintf (file, "NSdeserializeNamespace( MAPNS( %d))", ns->id);

    DBUG_RETURN ();
}

namespace_t *
NSdeserializeNamespace (int id)
{
    namespace_t *result;

    DBUG_ENTER ();

    result = NSdupNamespace (GetFromPool (id));

    DBUG_RETURN (result);
}

void *
NSdeserializeView (const char *name, int id, void *_next)
{
    view_t *result;
    view_t *next = (view_t *)_next;

    DBUG_ENTER ();

    result = (view_t *)MEMmalloc (sizeof (view_t));

    result->name = STRcpy (name);
    result->id = id;
    result->next = next;

    DBUG_RETURN (result);
}

int
NSaddMapping (const char *module, void *_view)
{
    namespace_t *ns;
    view_t *view = (view_t *)_view;
    int result;

    DBUG_ENTER ();

    DBUG_PRINT ("adding new mapping for module '%s'...", module);

    ns = FindInPool (module, view);

    if (ns == NULL)
        ns = AddNamespaceToPool (module, view);

    result = ns->id;

    DBUG_PRINT ("...mapped '%s' to %d.", NSgetName (ns), result);

    DBUG_RETURN (result);
}

static void
GenerateNamespaceMapHead (FILE *file)
{
    DBUG_ENTER ();

    fprintf (file, "/* namespace mapping generated by sac2c %s */\n\n",
             global.version_id);
    fprintf (file, "#include \"sac_serialize.h\"\n\n");
    fprintf (file, "#include \"namespacemap.h\"\n\n");

    fprintf (file, "#ifdef __cplusplus\n"
                   "#  define EXTERNC extern \"C\"\n"
                   "#else\n"
                   "#  define EXTERNC \n"
                   "#endif\n\n");

    DBUG_RETURN ();
}

static void
GenerateNamespaceMapDeclaration (FILE *file)
{
    int cnt;

    DBUG_ENTER ();

    for (cnt = 0; cnt < nextid; cnt++)
        fprintf (file, "int __%s__nsmap_%d = 0;\n ", global.modulename, cnt);

    DBUG_RETURN ();
}

static void
GenerateViewConstructor (FILE *file, view_t *view)
{
    DBUG_ENTER ();

    if (view == NULL)
        fprintf (file, "NULL");
    else {
        fprintf (file, "NSdeserializeView( \"%s\", %d, ", view->name, view->id);

        GenerateViewConstructor (file, view->next);

        fprintf (file, ")");
    }

    DBUG_RETURN ();
}

static void
GenerateNamespaceMappingConstructor (FILE *file)
{
    nspool_t *pos;
    int cnt;

    DBUG_ENTER ();

    pos = pool;

    fprintf (file, "EXTERNC void __%s__MapConstructor( void) {\n", global.modulename);

    for (cnt = 0; cnt < nextid; cnt++) {
        fprintf (file, "MAPNS(%d) = NSaddMapping( \"%s\",", cnt,
                 NSgetModule (pos->block[cnt % BLOCKSIZE]));

        GenerateViewConstructor (file, pos->block[cnt % BLOCKSIZE]->view);

        fprintf (file, ");\n");

        if ((cnt % BLOCKSIZE) == (BLOCKSIZE - 1))
            pos = pos->next;
    }

    fprintf (file, "}\n");

    DBUG_RETURN ();
}

static void
GenerateNamespaceMappingHeader (FILE *file)
{
    int cnt;

    DBUG_ENTER ();

    fprintf (file, "#ifndef _NAMESPACEMAP_H_\n#define _NAMESPACEMAP_H_\n\n");
    fprintf (file, "#define MAPNS( x)  __%s__nsmap_##x\n\n", global.modulename);

    for (cnt = 0; cnt < nextid; cnt++)
        fprintf (file, "extern int __%s__nsmap_%d;\n ", global.modulename, cnt);

    fprintf (file, "#endif\n");

    DBUG_RETURN ();
}

void
NSgenerateNamespaceMap (void)
{
    FILE *file;

    DBUG_ENTER ();

    file = FMGRwriteOpen ("%s/namespacemap.c", global.tmp_dirname);

    GenerateNamespaceMapHead (file);
    GenerateNamespaceMapDeclaration (file);
    GenerateNamespaceMappingConstructor (file);

    fclose (file);

    file = FMGRwriteOpen ("%s/namespacemap.h", global.tmp_dirname);

    GenerateNamespaceMappingHeader (file);

    fclose (file);

    DBUG_RETURN ();
}

void
xfree_namespace (namespace_t *xnamespace)
{
    view_t *xview;

    if (!xnamespace)
        return;

    xview = xnamespace->view;
    while (xview) {
        view_t *t = xview;
        xview = xview->next;
        if (t->name)
            MEMfree (t->name);
        MEMfree (t);
    }

    if (xnamespace->name)
        MEMfree (xnamespace->name);
    if (xnamespace->module)
        MEMfree (xnamespace->module);

    MEMfree (xnamespace);
}

void
xfree_namespace_pool (void)
{
    int i;
    nspool_t *p = pool;

    for (i = 0; i < nextid; i++) {
        int idx = i % BLOCKSIZE;

        if (p->block[idx])
            xfree_namespace (p->block[idx]);
        p->block[idx] = NULL;

        if (idx == BLOCKSIZE - 1) {
            nspool_t *t = p;
            p = p->next;
            MEMfree (t);
        }
    }

    if (p)
        MEMfree (p);
}

#undef DBUG_PREFIX
