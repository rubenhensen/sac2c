/*
 *
 * $Log$
 * Revision 1.3  2004/11/26 21:18:50  sah
 * pour Bodo *<8-)
 *
 * Revision 1.2  2004/11/25 22:16:31  sah
 * COMPILES
 *
 * Revision 1.1  2004/11/23 22:40:28  sah
 * Initial revision
 */

#define NEW_INFO

#include "deserialize.h"
#include "serialize.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "user_types.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "symboltable.h"
#include "modulemanager.h"
#include "new2old.h"
#include "traverse.h"

#include <string.h>

/*
 * INFO structure
 */
struct INFO {
    node *ret;
    node *ssacounter;
    node *module;
    node *fundefs;
    node *fundecs;
    node *typedefs;
    node *objdefs;
    node *vardecs;
    node *args;
};

/*
 * INFO macros
 */
#define INFO_DS_RETURN(n) (n->ret)
#define INFO_DS_SSACOUNTER(n) (n->ssacounter)
#define INFO_DS_MODULE(n) (n->module)
#define INFO_DS_FUNDEFS(n) (n->fundefs)
#define INFO_DS_FUNDECS(n) (n->fundecs)
#define INFO_DS_TYPEDEFS(n) (n->typedefs)
#define INFO_DS_OBJDEFS(n) (n->objdefs)
#define INFO_DS_VARDECS(n) (n->vardecs)
#define INFO_DS_ARGS(n) (n->args)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_DS_RETURN (result) = NULL;
    INFO_DS_SSACOUNTER (result) = NULL;
    INFO_DS_MODULE (result) = NULL;
    INFO_DS_FUNDEFS (result) = NULL;
    INFO_DS_FUNDECS (result) = NULL;
    INFO_DS_TYPEDEFS (result) = NULL;
    INFO_DS_OBJDEFS (result) = NULL;
    INFO_DS_VARDECS (result) = NULL;
    INFO_DS_ARGS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

static info *DSstate = NULL;

/*
 * functions handling the internal state
 */

static void
InsertIntoState (node *item)
{
    usertype udt;

    DBUG_ENTER ("InsertIntoState");

    switch (NODE_TYPE (item)) {
    case N_fundef:
        /*
         * The fundef is neither local, nor is it provided/exported
         */
        FUNDEF_ISLOCAL (item) = FALSE;
        FUNDEF_ISEXPORTED (item) = FALSE;
        FUNDEF_ISPROVIDED (item) = FALSE;

        if (FUNDEF_ISEXTERN (item)) {
            INFO_DS_FUNDECS (DSstate) = TCappendFundef (INFO_DS_FUNDECS (DSstate), item);
        } else {
            INFO_DS_FUNDEFS (DSstate) = TCappendFundef (INFO_DS_FUNDEFS (DSstate), item);
        }
        break;
    case N_typedef:
        /*
         * reset the flags of the typedef
         */
        TYPEDEF_ISLOCAL (item) = FALSE;
        TYPEDEF_ISPROVIDED (item) = FALSE;
        TYPEDEF_ISEXPORTED (item) = FALSE;

        /*
         * first add the type to the UDT repository
         */
        udt = UTaddUserType (TYPEDEF_NAME (item), TYPEDEF_MOD (item),
                             TYPEDEF_NTYPE (item), NULL, NODE_LINE (item), item);
        /*
         * now compute the basetype
         */
        NTCcheckUdtAndSetBaseType (udt, NULL);

        INFO_DS_TYPEDEFS (DSstate) = TCappendTypedef (INFO_DS_TYPEDEFS (DSstate), item);
        break;
    case N_objdef:
        /*
         * reset the flags of the objdef
         */
        OBJDEF_ISLOCAL (item) = FALSE;
        OBJDEF_ISPROVIDED (item) = FALSE;
        OBJDEF_ISEXPORTED (item) = FALSE;

        /*
         * now insert it
         */
        INFO_DS_OBJDEFS (DSstate) = TCappendObjdef (INFO_DS_OBJDEFS (DSstate), item);
        break;
    default:
        DBUG_ASSERT (0, "Unhandeled node in InsertIntoState!");
        break;
    }

    DBUG_VOID_RETURN;
}

void
DSinitDeserialize (node *module)
{
    DBUG_ENTER ("DSinitDeserialize");

    DBUG_ASSERT ((DSstate == NULL),
                 "InitDeserialize called before last run was finished!");

    DSstate = MakeInfo ();

    INFO_DS_MODULE (DSstate) = module;

    DBUG_VOID_RETURN;
}

void
DSfinishDeserialize (node *module)
{
    DBUG_ENTER ("DSfinishDeserialize");

    MODULE_FUNS (module)
      = TCappendFundef (MODULE_FUNS (module), INFO_DS_FUNDEFS (DSstate));

    MODULE_FUNDECS (module)
      = TCappendFundef (MODULE_FUNDECS (module), INFO_DS_FUNDECS (DSstate));

    MODULE_TYPES (module)
      = TCappendTypedef (MODULE_TYPES (module), INFO_DS_TYPEDEFS (DSstate));

    DSstate = FreeInfo (DSstate);

    DBUG_VOID_RETURN;
}

/*
 * functions for symbol lookup in ast
 */

static node *
FindSymbolInFundefChain (const char *symbol, node *fundefs)
{
    node *result = NULL;

    DBUG_ENTER ("FindSymbolInFundefChain");

    while ((result == NULL) && (fundefs != NULL)) {
        if (FUNDEF_SYMBOLNAME (fundefs) != NULL) {
            if (ILIBstringCompare (FUNDEF_SYMBOLNAME (fundefs), symbol)) {
                result = fundefs;
            }
        }
        fundefs = FUNDEF_NEXT (fundefs);
    }

    DBUG_RETURN (result);
}

static node *
FindSymbolInTypedefChain (const char *symbol, node *typedefs)
{
    node *result = NULL;

    DBUG_ENTER ("FindSymbolInTypedefChain");

    while ((result == NULL) && (typedefs != NULL)) {
        if (TYPEDEF_SYMBOLNAME (typedefs) != NULL) {
            if (ILIBstringCompare (TYPEDEF_SYMBOLNAME (typedefs), symbol)) {
                result = typedefs;
            }
        }
        typedefs = TYPEDEF_NEXT (typedefs);
    }

    DBUG_RETURN (result);
}

static node *
FindSymbolInAst (const char *symbol)
{
    node *result = NULL;

    DBUG_ENTER ("FindSymbolInAst");

    result = FindSymbolInFundefChain (symbol, INFO_DS_FUNDEFS (DSstate));

    if (result == NULL) {
        result = FindSymbolInFundefChain (symbol, INFO_DS_FUNDECS (DSstate));
    }

    if (result == NULL) {
        result = FindSymbolInFundefChain (symbol, MODULE_FUNS (INFO_DS_MODULE (DSstate)));
    }

    if (result == NULL) {
        result
          = FindSymbolInFundefChain (symbol, MODULE_FUNDECS (INFO_DS_MODULE (DSstate)));
    }

    if (result == NULL) {
        result = FindSymbolInTypedefChain (symbol, INFO_DS_TYPEDEFS (DSstate));
    }

    if (result == NULL) {
        result
          = FindSymbolInTypedefChain (symbol, MODULE_TYPES (INFO_DS_MODULE (DSstate)));
    }

    DBUG_RETURN (result);
}

/*
 * functions for loading symbols
 */

static node *
AddEntryToAst (stentry_t *entry, stentrytype_t type, module_t *module)
{
    node *entryp = NULL;
    serfun_p serfun;

    DBUG_ENTER ("AddEntryToAst");

    if (STentryType (entry) == type) {

        switch (STentryType (entry)) {
        case SET_funbody:
        case SET_wrapperbody:
            /* these are ignored. All function bodies are loaded on
             * demand only!
             */
            break;
        case SET_funhead:
        case SET_wrapperhead:
        case SET_typedef:
        case SET_objdef:
            /* first check, whether it is already available */
            if (FindSymbolInAst (STentryName (entry)) == NULL) {
                serfun = MODMgetDeSerializeFunction (STentryName (entry), module);
                entryp = serfun (DSstate);
                /* add old types */
                entryp = NT2OTdoTransform (entryp);
                /* add to ast */
                InsertIntoState (entryp);
            }
            break;
        default:
            DBUG_ASSERT (0, "unhandeled STentrytype");
            break;
        }
    }

    DBUG_RETURN (entryp);
}

node *
SERaddSymbolByName (const char *symbol, stentrytype_t type, const char *module)
{
    node *result = NULL;
    module_t *mod;
    sttable_t *table;
    stentryiterator_t *it;

    DBUG_ENTER ("SERaddSymbolByName");

    DBUG_ASSERT ((DSstate != NULL),
                 "AddSymbolByName called without calling InitDeserialize.");

    mod = MODMloadModule (module);

    table = MODMgetSymbolTable (mod);
    it = STentryIteratorGet (symbol, table);

    while (STentryIteratorHasMore (it)) {
        node *tmp = AddEntryToAst (STentryIteratorNext (it), type, mod);
        if (tmp != NULL) {
            result = tmp;
        }
    }

    it = STentryIteratorRelease (it);
    table = STdestroy (table);
    mod = MODMunLoadModule (mod);

    DBUG_RETURN (result);
}

node *
SERaddSymbolById (const char *symbid, const char *module)
{
    module_t *mod;
    serfun_p fun;
    node *entryp;

    DBUG_ENTER ("SERaddSymbolById");

    mod = MODMloadModule (module);

    fun = MODMgetDeSerializeFunction (symbid, mod);

    entryp = fun ();

    /* add old types */
    entryp = NT2OTdoTransform (entryp);
    /* add to ast */
    InsertIntoState (entryp);

    mod = MODMunLoadModule (mod);

    DBUG_RETURN (entryp);
}

/*
 * hooks for the deserialisation process
 */

ntype *
DSloadUserType (const char *name, const char *mod)
{
    ntype *result;
    node *tdef;
    usertype udt;

    DBUG_ENTER ("DSloadUserType");

    tdef = SERaddSymbolByName (name, SET_typedef, mod);

    DBUG_ASSERT ((tdef != NULL), "deserialisation of typedef failed!");

    udt = UTfindUserType (name, mod);

    DBUG_ASSERT ((udt != UT_NOT_DEFINED), "typedef not in udt repository");

    result = TYmakeUserType (udt);

    DBUG_RETURN (result);
}

ntype *
DSloadSymbolType (const char *mod, const char *name)
{
    ntype *result;

    DBUG_ENTER ("DSloadSymbolType");

    DBUG_RETURN (result);
}

node *
DSlookupFunction (const char *module, const char *symbol)
{
    node *result = NULL;
    serfun_p serfun;
    module_t *mod;

    DBUG_ENTER ("DSlookupFunction");

    DBUG_PRINT ("DS", ("Looking up function `%s:%s' in ast.", module, symbol));

    result = FindSymbolInAst (symbol);

    if (result == NULL) {
        DBUG_PRINT ("DS", ("Looking up function `%s' in `%s'.", symbol, module));

        mod = MODMloadModule (module);
        serfun = MODMgetDeSerializeFunction (symbol, mod);
        result = serfun ();
        mod = MODMunLoadModule (mod);

        /* generate the old types */
        result = NT2OTdoTransform (result);

        InsertIntoState (result);
    }

    DBUG_RETURN (result);
}

/*
 * deserialize traversal functions
 */

static node *
LoadFunctionBody (node *fundef)
{
    node *result = NULL;
    module_t *module;
    sttable_t *table;
    serfun_p serfun;
    const char *serfunname;

    DBUG_ENTER ("LoadFunctionBody");

    DBUG_ASSERT ((DSstate != NULL),
                 "LoadFunctionBody called without calling InitDeserialize");

    module = MODMloadModule (FUNDEF_MOD (fundef));
    table = MODMgetSymbolTable (module);

    serfunname = SERgenerateSerFunName (SET_funbody, fundef);

    if (STcontainsEntry (serfunname, table)) {

        serfun = MODMgetDeSerializeFunction (serfunname, module);

        result = serfun (DSstate);
    }

    table = STdestroy (table);
    module = MODMunLoadModule (module);

    DBUG_RETURN (result);
}

node *
DSdoDeserialize (node *fundef)
{
    node *body;

    DBUG_ENTER ("DSdoDeserialize");

    DBUG_ASSERT ((DSstate != NULL),
                 "AddFunctionBodyToHead called without calling InitDeserialize");

    DBUG_PRINT ("DS", ("Adding function body to `%s:%s'.", FUNDEF_MOD (fundef),
                       FUNDEF_NAME (fundef)));

    body = LoadFunctionBody (fundef);

    FUNDEF_BODY (fundef) = body;

    TRAVpush (TR_ds);

    TRAVdo (fundef, DSstate);

    TRAVpop ();

    /* finaly, we have to do e new2old, as the old types are not
     * serialized.
     */

    fundef = NT2OTdoTransform (fundef);

    DBUG_RETURN (fundef);
}

/*
 * Helper functions for deserialize traversal
 */

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

static node *
LookUpArg (const char *name, node *args)
{
    DBUG_ENTER ("LookUpArg");

    while ((args != NULL) && (ARG_NAME (args) != NULL)
           && (strcmp (ARG_NAME (args), name))) {
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (args);
}

/*
 * traversal functions
 */

node *
DSids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSids");

    DBUG_PRINT ("DS", ("Restoring ids `%s'", IDS_NAME (arg_node)));

    /*
     * if there is no vardec for this node, it must have been
     * linked to an arg prior to deserialization!
     */
    if (IDS_DECL (arg_node) == NULL) {
        IDS_DECL (arg_node) = LookUpArg (IDS_NAME (arg_node), INFO_DS_ARGS (arg_info));

        DBUG_ASSERT ((IDS_DECL (arg_node) != NULL), "Cannot find vardec or arg for ids!");

        /* now update the avis link of the ids node to the args avis */
        if (IDS_AVIS (arg_node) == NULL) {
            IDS_AVIS (arg_node) = ARG_AVIS (IDS_DECL (arg_node));
        }
    }

    /* Finally make sure, that the backref avis->vardec is correct */
    DBUG_ASSERT ((AVIS_DECL (DECL_AVIS (IDS_DECL (arg_node))) == IDS_DECL (arg_node)),
                 "backref from avis to vardec is wrong!");

    DBUG_RETURN (arg_node);
}

node *
DSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSfundef");

    INFO_DS_ARGS (arg_info) = FUNDEF_ARGS (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    FUNDEF_RETURN (arg_node) = INFO_DS_RETURN (arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSreturn");

    INFO_DS_RETURN (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSblock");

    if (INFO_DS_SSACOUNTER (arg_info) == NULL) {
        /* we are the first block underneath the Fundef node */
        INFO_DS_SSACOUNTER (arg_info) = BLOCK_SSACOUNTER (arg_node);
    }

    if (INFO_DS_VARDECS (arg_info) == NULL) {
        /* no vardecs set yet, so we do it */
        INFO_DS_VARDECS (arg_info) = BLOCK_VARDEC (arg_node);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSarg");

    AVIS_SSACOUNT (ARG_AVIS (arg_node))
      = LookUpSSACounter (INFO_DS_SSACOUNTER (arg_info), arg_node);

    arg_node = TRAVdo (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
