/*
 *
 * $Log$
 * Revision 1.9  2004/11/21 12:48:08  sah
 * removed some old ast info
 *
 * Revision 1.8  2004/11/17 19:50:13  sah
 * interface changes
 *
 * Revision 1.7  2004/11/14 15:25:38  sah
 * implemented support for udts
 * some cleanup
 *
 * Revision 1.6  2004/11/07 18:11:04  sah
 * brushed the code
 *
 * Revision 1.5  2004/11/01 21:52:40  sah
 * NWITHID_VEC ids are processed now as well
 *
 * Revision 1.4  2004/10/28 17:20:46  sah
 * now deserialize as an internal state
 *
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
#include "user_types.h"
#include "new_typecheck.h"

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

    info = Free (info);

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
        SET_FLAG (FUNDEF, item, IS_LOCAL, FALSE);
        SET_FLAG (FUNDEF, item, IS_EXPORTED, FALSE);
        SET_FLAG (FUNDEF, item, IS_PROVIDED, FALSE);

        if (FUNDEF_STATUS (item) == ST_Cfun) {
            INFO_DS_FUNDECS (DSstate) = AppendFundef (INFO_DS_FUNDECS (DSstate), item);
        } else {
            INFO_DS_FUNDEFS (DSstate) = AppendFundef (INFO_DS_FUNDEFS (DSstate), item);
        }
        break;
    case N_typedef:
        /*
         * reset the flags of the typedef
         */
        SET_FLAG (TYPEDEF, item, IS_LOCAL, FALSE);
        SET_FLAG (TYPEDEF, item, IS_PROVIDED, FALSE);
        SET_FLAG (TYPEDEF, item, IS_EXPORTED, FALSE);

        /*
         * first add the type to the UDT repository
         */
        udt = UTAddUserType (TYPEDEF_NAME (item), TYPEDEF_MOD (item),
                             TYPEDEF_NTYPE (item), NULL, NODE_LINE (item), item);
        /*
         * now compute the basetype
         */
        CheckUdtAndSetBaseType (udt, NULL);

        INFO_DS_TYPEDEFS (DSstate) = AppendTypedef (INFO_DS_TYPEDEFS (DSstate), item);
        break;
    case N_objdef:
        /*
         * reset the flags of the objdef
         */
        SET_FLAG (OBJDEF, item, IS_LOCAL, FALSE);
        SET_FLAG (OBJDEF, item, IS_PROVIDED, FALSE);
        SET_FLAG (OBJDEF, item, IS_EXPORTED, FALSE);

        /*
         * now insert it
         */
        INFO_DS_OBJDEFS (DSstate) = AppendObjdef (INFO_DS_OBJDEFS (DSstate), item);
        break;
    default:
        DBUG_ASSERT (0, "Unhandeled node in InsertIntoState!");
        break;
    }

    DBUG_VOID_RETURN;
}

void
InitDeserialize (node *module)
{
    DBUG_ENTER ("InitDeserialize");

    DBUG_ASSERT ((DSstate == NULL),
                 "InitDeserialize called before last run was finished!");

    DSstate = MakeInfo ();

    INFO_DS_MODULE (DSstate) = module;

    DBUG_VOID_RETURN;
}

void
FinishDeserialize (node *module)
{
    DBUG_ENTER ("FinishDeserialize");

    MODUL_FUNS (module) = AppendFundef (MODUL_FUNS (module), INFO_DS_FUNDEFS (DSstate));

    MODUL_FUNDECS (module)
      = AppendFundef (MODUL_FUNDECS (module), INFO_DS_FUNDECS (DSstate));

    MODUL_TYPES (module)
      = AppendTypedef (MODUL_TYPES (module), INFO_DS_TYPEDEFS (DSstate));

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
            if (!strcmp (FUNDEF_SYMBOLNAME (fundefs), symbol)) {
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
            if (!strcmp (TYPEDEF_SYMBOLNAME (typedefs), symbol)) {
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
        result = FindSymbolInFundefChain (symbol, MODUL_FUNS (INFO_DS_MODULE (DSstate)));
    }

    if (result == NULL) {
        result
          = FindSymbolInFundefChain (symbol, MODUL_FUNDECS (INFO_DS_MODULE (DSstate)));
    }

    if (result == NULL) {
        result = FindSymbolInTypedefChain (symbol, INFO_DS_TYPEDEFS (DSstate));
    }

    if (result == NULL) {
        result
          = FindSymbolInTypedefChain (symbol, MODUL_TYPES (INFO_DS_MODULE (DSstate)));
    }

    DBUG_RETURN (result);
}

/*
 * functions for loading symbols
 */

static node *
AddEntryToAst (STentry_t *entry, STentrytype_t type, module_t *module)
{
    node *entryp = NULL;
    serfun_p serfun;

    DBUG_ENTER ("AddEntryToAst");

    if (STEntryType (entry) == type) {

        switch (STEntryType (entry)) {
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
            if (FindSymbolInAst (STEntryName (entry)) == NULL) {
                serfun = GetDeSerializeFunction (STEntryName (entry), module);
                entryp = serfun (DSstate);
                /* add old types */
                entryp = NT2OTTransform (entryp);
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
AddSymbolByName (const char *symbol, STentrytype_t type, const char *module)
{
    node *result = NULL;
    module_t *mod;
    STtable_t *table;
    STentryiterator_t *it;

    DBUG_ENTER ("AddSymbolByName");

    DBUG_ASSERT ((DSstate != NULL),
                 "AddSymbolByName called without calling InitDeserialize.");

    mod = LoadModule (module);

    table = GetSymbolTable (mod);
    it = STEntryIteratorGet (symbol, table);

    while (STEntryIteratorHasMore (it)) {
        node *tmp = AddEntryToAst (STEntryIteratorNext (it), type, mod);
        if (tmp != NULL) {
            result = tmp;
        }
    }

    it = STEntryIteratorRelease (it);
    table = STDestroy (table);
    mod = UnLoadModule (mod);

    DBUG_RETURN (result);
}

node *
AddSymbolById (const char *symbid, const char *module)
{
    module_t *mod;
    serfun_p fun;
    node *entryp;

    DBUG_ENTER ("AddSymbolById");

    mod = LoadModule (module);

    fun = GetDeSerializeFunction (symbid, mod);

    entryp = fun ();

    /* add old types */
    entryp = NT2OTTransform (entryp);
    /* add to ast */
    InsertIntoState (entryp);

    mod = UnLoadModule (mod);

    DBUG_RETURN (entryp);
}

/*
 * hooks for the deserialisation process
 */

ntype *
DeserializeLoadUserType (const char *name, const char *mod)
{
    ntype *result;
    node *tdef;
    usertype udt;

    DBUG_ENTER ("DeserializeLoadUserType");

    tdef = AddSymbolByName (name, SET_typedef, mod);

    DBUG_ASSERT ((tdef != NULL), "deserialisation of typedef failed!");

    udt = UTFindUserType (name, mod);

    DBUG_ASSERT ((udt != UT_NOT_DEFINED), "typedef not in udt repository");

    result = TYMakeUserType (udt);

    DBUG_RETURN (result);
}

ntype *
DeserializeLoadSymbolType (const char *mod, const char *name)
{
    ntype *result;

    DBUG_ENTER ("DeserializeLoadSymbolType");

    DBUG_RETURN (result);
}

node *
DeserializeLookupFunction (const char *module, const char *symbol)
{
    node *result = NULL;
    serfun_p serfun;
    module_t *mod;

    DBUG_ENTER ("DeserializeLookupFunction");

    DBUG_PRINT ("DS", ("Looking up function `%s:%s' in ast.", module, symbol));

    result = FindSymbolInAst (symbol);

    if (result == NULL) {
        DBUG_PRINT ("DS", ("Looking up function `%s' in `%s'.", symbol, module));

        mod = LoadModule (module);
        serfun = GetDeSerializeFunction (symbol, mod);
        result = serfun ();

        /* generate the old types */
        result = NT2OTTransform (result);

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
    STtable_t *table;
    serfun_p serfun;
    const char *serfunname;

    DBUG_ENTER ("LoadFunctionBody");

    DBUG_ASSERT ((DSstate != NULL),
                 "LoadFunctionBody called without calling InitDeserialize");

    module = LoadModule (FUNDEF_MOD (fundef));
    table = GetSymbolTable (module);

    serfunname = GenerateSerFunName (SET_funbody, fundef);

    if (STContainsEntry (serfunname, table)) {

        serfun = GetDeSerializeFunction (serfunname, module);

        result = serfun (DSstate);
    }

    table = STDestroy (table);
    module = UnLoadModule (module);

    DBUG_RETURN (result);
}

node *
AddFunctionBodyToHead (node *fundef)
{
    funtab *store_tab;
    node *body;

    DBUG_ENTER ("AddFunctionBodyToHead");

    DBUG_ASSERT ((DSstate != NULL),
                 "AddFunctionBodyToHead called without calling InitDeserialize");

    DBUG_PRINT ("DS", ("Adding function body to `%s:%s'.", FUNDEF_MOD (fundef),
                       FUNDEF_NAME (fundef)));

    body = LoadFunctionBody (fundef);

    FUNDEF_BODY (fundef) = body;

    store_tab = act_tab;
    act_tab = ds_tab;

    Trav (fundef, DSstate);

    act_tab = store_tab;

    /* finaly, we have to do e new2old, as the old types are not
     * serialized.
     */

    fundef = NT2OTTransform (fundef);

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
LookUpVardec (ids *ids, node *vardecs)
{
    DBUG_ENTER ("LookUpVardec");

    while ((vardecs != NULL) && (strcmp (VARDEC_NAME (vardecs), IDS_NAME (ids)))) {
        vardecs = VARDEC_NEXT (vardecs);
    }

    DBUG_RETURN (vardecs);
}

static node *
LookUpArg (ids *ids, node *args)
{
    DBUG_ENTER ("LookUpArg");

    while ((args != NULL) && (ARG_NAME (args) != NULL)
           && (strcmp (ARG_NAME (args), IDS_NAME (ids)))) {
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (args);
}

/*
 * traversal functions
 */

static ids *
DSIds (ids *arg_ids, info *arg_info)
{
    DBUG_ENTER ("DSIds");

    DBUG_PRINT ("DS", ("Restoring ids `%s'", IDS_NAME (arg_ids)));

    /* First try the vardecs */
    if (IDS_VARDEC (arg_ids) == NULL) {
        IDS_VARDEC (arg_ids) = LookUpVardec (arg_ids, INFO_DS_VARDECS (arg_info));
    }

    /* Second try the args */
    if (IDS_VARDEC (arg_ids) == NULL) {
        IDS_VARDEC (arg_ids) = LookUpArg (arg_ids, INFO_DS_ARGS (arg_info));
    }

    DBUG_ASSERT ((IDS_VARDEC (arg_ids) != NULL), "Cannot find vardec or arg for ids!");

    /* now update the avis link of the ids node to the vardecs avis */
    if (IDS_AVIS (arg_ids) == NULL) {
        IDS_AVIS (arg_ids) = VARDEC_OR_ARG_AVIS (IDS_VARDEC (arg_ids));
    }

    /* Finally make sure, that the backref avis->vardec is correct */
    DBUG_ASSERT ((AVIS_VARDECORARG (VARDEC_OR_ARG_AVIS (IDS_VARDEC (arg_ids)))
                  == IDS_VARDEC (arg_ids)),
                 "backref from avis to vardec is wrong!");

    DBUG_RETURN (arg_ids);
}

node *
DSFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSFundef");

    INFO_DS_ARGS (arg_info) = FUNDEF_ARGS (arg_node);

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

    if (INFO_DS_VARDECS (arg_info) == NULL) {
        /* no vardecs set yet, so we do it */
        INFO_DS_VARDECS (arg_info) = BLOCK_VARDEC (arg_node);
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

    /* Trav into Ids */
    ID_IDS (arg_node) = DSIds (ID_IDS (arg_node), arg_info);

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSLet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSLet");

    /* Trav into Ids */
    LET_IDS (arg_node) = DSIds (LET_IDS (arg_node), arg_info);

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
DSNWithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DSNWithid");

    /* Trav into Ids */
    NWITHID_IDS (arg_node) = DSIds (NWITHID_IDS (arg_node), arg_info);
    NWITHID_VEC (arg_node) = DSIds (NWITHID_VEC (arg_node), arg_info);

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
