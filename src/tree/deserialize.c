/*
 *
 * $Log$
 * Revision 1.3  2004/10/26 09:36:20  sah
 * ongoing implementation
 *
 * Revision 1.2  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.1  2004/09/23 21:12:03  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "deserialize.h"
#include <strings.h>
#include "internal_lib.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "symboltable.h"
#include "serialize.h"
#include "new2old.h"

#include "deserialize_info.h"

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_DS_RETURN (result) = NULL;
    INFO_DS_SSACOUNTER (result) = NULL;
    INFO_DS_AST (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

node *
LoadFunctionBody (node *fundef, node *modnode)
{
    node *result;
    module_t *module;
    info *info;
    serfun_p serfun;

    DBUG_ENTER ("LoadFunctionBody");

    info = MakeInfo ();
    INFO_DS_AST (info) = modnode;

    module = LoadModule (FUNDEF_MOD (fundef));

    serfun = GetDeSerializeFunction (GenerateSerFunName (SET_funbody, fundef), module);

    result = serfun (info);

    info = FreeInfo (info);

    module = UnLoadModule (module);

    DBUG_RETURN (result);
}

static void
AddEntryToAst (STentry_t *entry, module_t *module, node *ast)
{
    node *entryp;
    info *info;
    serfun_p serfun;

    DBUG_ENTER ("AddEntryToAst");

    info = MakeInfo ();

    INFO_DS_AST (info) = ast;

    switch (STEntryType (entry)) {
    case SET_funhead:
    case SET_funbody:
    case SET_wrapperbody:
        /* these are ignored. All functions are loaded as a dependency
         * of the wrapperfunction anyways and bodies are loaded on
         * demand only!
         */
        break;
    case SET_wrapperhead:
        serfun = GetDeSerializeFunction (STEntryName (entry), module);
        entryp = serfun (info);

        /* add old types */
        entryp = NT2OTTransform (entryp);

        FUNDEF_NEXT (entryp) = MODUL_FUNS (ast);
        MODUL_FUNS (ast) = entryp;
        break;
    default:
        break;
    }

    info = FreeInfo (info);

    DBUG_VOID_RETURN;
}

void
AddSymbolToAst (const char *symbol, module_t *module, node *ast)
{
    STtable_t *table;
    STentryiterator_t *it;

    DBUG_ENTER ("AddSymbolToAst");

    table = GetSymbolTable (module);
    it = STEntryIteratorGet (symbol, table);

    while (STEntryIteratorHasMore (it)) {
        AddEntryToAst (STEntryIteratorNext (it), module, ast);
    }

    it = STEntryIteratorRelease (it);
    table = STDestroy (table);

    DBUG_VOID_RETURN;
}

node *
DeserializeLookupFunction (const char *module, const char *symbol, info *info)
{
    node *fundefs = NULL;
    node *result = NULL;
    serfun_p serfun;
    module_t *mod;

    DBUG_ENTER ("DeserializeLookupFunction");

    fundefs = MODUL_FUNS (INFO_DS_AST (info));

    while ((result == NULL) && (fundefs != NULL)) {
        if (FUNDEF_SYMBOLNAME (fundefs) != NULL) {
            if (!strcmp (FUNDEF_SYMBOLNAME (fundefs), symbol)) {
                result = fundefs;
            }
        }
        fundefs = FUNDEF_NEXT (fundefs);
    }

    if (result == NULL) {
        mod = LoadModule (module);
        serfun = GetDeSerializeFunction (symbol, mod);
        result = serfun (info);

        /* generate the old types */
        result = NT2OTTransform (result);

        FUNDEF_NEXT (result) = MODUL_FUNS (INFO_DS_AST (info));
        MODUL_FUNS (INFO_DS_AST (info)) = result;
    }

    DBUG_RETURN (result);
}

node *
AddFunctionBodyToHead (node *fundef, node *module)
{
    funtab *store_tab;
    info *info;
    node *body;

    DBUG_ENTER ("CombineFunctionHeadAndBody");

    info = MakeInfo ();
    body = LoadFunctionBody (fundef, module);

    FUNDEF_BODY (fundef) = body;

    store_tab = act_tab;
    act_tab = ds_tab;

    Trav (fundef, info);

    act_tab = store_tab;

    info = FreeInfo (info);

    /* finaly, we have to do e new2old, as the old types are not
     * serialized.
     */

    fundef = NT2OTTransform (fundef);

    DBUG_RETURN (fundef);
}

static node *
LookUpSSACounter (node *cntchain, node *arg)
{
    node *result = NULL;
    DBUG_ENTER ("LookUpSSACounter");

    while ((cntchain != NULL) && (result == NULL)) {
        if (!strcmp (SSACNT_BASEID (cntchain), ARG_NAME (arg))) {
            result = cntchain;
        }

        cntchain = SSACNT_NEXT (cntchain);
    }

    DBUG_RETURN (result);
}

node *
DSFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSFundef");

    arg_node = TravSons (arg_node, arg_info);

    FUNDEF_RETURN (arg_node) = INFO_DS_RETURN (arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSReturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSReturn");

    INFO_DS_RETURN (arg_info) = arg_node;

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSBlock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSBlock");

    if (INFO_DS_SSACOUNTER (arg_info) == NULL) {
        /* we are the first block underneath the Fundef node */
        INFO_DS_SSACOUNTER (arg_info) = BLOCK_SSACOUNTER (arg_node);
    }

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSArg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSArg");

    AVIS_SSACOUNT (ARG_AVIS (arg_node))
      = LookUpSSACounter (INFO_DS_SSACOUNTER (arg_info), arg_node);

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSVardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSVardec");

    /* nothing to be done here */

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSId");

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSLet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSLet");

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSNWithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSNWithid");

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
