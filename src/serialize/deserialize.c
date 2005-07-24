/*
 *
 * $Log$
 * Revision 1.23  2005/07/24 20:01:50  sah
 * moved all the preparations for typechecking
 * into a different phase
 *
 * Revision 1.22  2005/07/22 13:10:52  sah
 * extracted some functionality from
 * deserialize into add_function_body
 *
 * Revision 1.21  2005/07/21 12:02:59  sbs
 * fixed bug in imports
 * free wheeling zombies caught and properly disposed of.
 *
 * Revision 1.20  2005/07/20 13:16:50  ktr
 * removed FUNDEF_EXT_ASSIGM
 *
 * Revision 1.19  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 1.18  2005/06/28 16:23:57  sah
 * cleanup
 *
 * Revision 1.17  2005/06/18 18:07:51  sah
 * added DSdispatchFunCall
 *
 * Revision 1.16  2005/06/18 13:14:42  sah
 * fixed incompatiblity
 *
 * Revision 1.15  2005/06/06 09:01:55  sah
 * fixed a minor bug in findEntryInAST
 *
 * Revision 1.14  2005/05/31 19:27:02  sah
 * when adding usertypes, all info is explicitly copied to avoid sharing
 *
 * Revision 1.13  2005/05/26 08:51:54  sbs
 * missing DBUG_VOID_RETURN added in updateContextInformation
 *
 * Revision 1.12  2005/05/25 20:26:35  sah
 * FUNDEF_EXT_ASSIGN is restored now
 * during deserialisation
 *
 * Revision 1.11  2005/05/24 19:40:14  sah
 * fixed handling of WASUSED/WASIMPORTED on
 * deserialization
 *
 * Revision 1.10  2005/05/22 19:45:53  sah
 * added first implementation steps for import
 *
 * Revision 1.9  2005/05/19 11:10:44  sah
 * added special import mode
 *
 * Revision 1.8  2005/05/18 13:56:51  sah
 * enabled caching of symboltables which
 * leads to a huge speedup when analysing use and import
 * from big modules
 *
 * Revision 1.7  2005/05/17 16:20:50  sah
 * added some reasonable error messages
 *
 * Revision 1.6  2005/02/16 22:29:13  sah
 * changed link handling
 *
 * Revision 1.5  2004/12/19 18:09:33  sah
 * post dk fixes
 *
 * Revision 1.4  2004/11/27 02:48:08  sah
 * fix
 *
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
#include "type_utils.h"
#include "new_typecheck.h"
#include "symboltable.h"
#include "stringset.h"
#include "modulemanager.h"
#include "namespaces.h"
#include "new2old.h"
#include "traverse.h"
#include "ctinfo.h"
#include "free.h"

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
    node *funhead;
    node *lastassign;
    bool importmode;
    stringset_t *dependencies;
};

/*
 * INFO macros
 */
#define INFO_DS_RETURN(n) ((n)->ret)
#define INFO_DS_SSACOUNTER(n) ((n)->ssacounter)
#define INFO_DS_MODULE(n) ((n)->module)
#define INFO_DS_FUNDEFS(n) ((n)->fundefs)
#define INFO_DS_FUNDECS(n) ((n)->fundecs)
#define INFO_DS_TYPEDEFS(n) ((n)->typedefs)
#define INFO_DS_OBJDEFS(n) ((n)->objdefs)
#define INFO_DS_VARDECS(n) ((n)->vardecs)
#define INFO_DS_ARGS(n) ((n)->args)
#define INFO_DS_FUNHEAD(n) ((n)->funhead)
#define INFO_DS_LASTASSIGN(n) ((n)->lastassign)
#define INFO_DS_IMPORTMODE(n) ((n)->importmode)
#define INFO_DS_DEPS(n) ((n)->dependencies)

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
    INFO_DS_FUNHEAD (result) = NULL;
    INFO_DS_LASTASSIGN (result) = NULL;
    INFO_DS_IMPORTMODE (result) = FALSE;
    INFO_DS_DEPS (result) = NULL;

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
         * furthermore we reset the WASUSED/WASIMPORTED here
         * as these informations refer to the context where
         * the fundef has been serialized. Functions calling this
         * function should set the flags appropriate again
         */
        FUNDEF_ISLOCAL (item) = FALSE;
        FUNDEF_ISEXPORTED (item) = FALSE;
        FUNDEF_ISPROVIDED (item) = FALSE;
        FUNDEF_WASUSED (item) = FALSE;
        FUNDEF_WASIMPORTED (item) = FALSE;

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
        udt = UTaddUserType (ILIBstringCopy (TYPEDEF_NAME (item)),
                             NSdupNamespace (TYPEDEF_NS (item)),
                             TYcopyType (TYPEDEF_NTYPE (item)), NULL, NODE_LINE (item),
                             item);
#if 0
      /*
       * now compute the basetype
       */
      NTCcheckUdtAndSetBaseType( udt, NULL);
#endif

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

static node *
getCurrentFundefHead ()
{
    DBUG_ENTER ("getCurrentFundefHead");

    DBUG_ASSERT ((DSstate != NULL), "called getCurrentFundefHead without starting DS...");

    DBUG_ASSERT ((INFO_DS_FUNHEAD (DSstate) != NULL),
                 "called getCurrentFundefHead but there is none!");

    DBUG_RETURN (INFO_DS_FUNHEAD (DSstate));
}

static void
SetCurrentFundefHead (node *fundef)
{
    DBUG_ENTER ("SetCurrentFundefHead");

    DBUG_ASSERT ((DSstate != NULL), "called SetCurrentFundefHead without starting DS...");

    INFO_DS_FUNHEAD (DSstate) = fundef;

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

    DBUG_ASSERT ((DSstate != NULL), "called DSfinishDeserialize without starting DS...");

    MODULE_FUNS (module)
      = TCappendFundef (MODULE_FUNS (module), INFO_DS_FUNDEFS (DSstate));

    MODULE_FUNDECS (module)
      = TCappendFundef (MODULE_FUNDECS (module), INFO_DS_FUNDECS (DSstate));

    MODULE_TYPES (module)
      = TCappendTypedef (MODULE_TYPES (module), INFO_DS_TYPEDEFS (DSstate));

    MODULE_DEPENDENCIES (module)
      = STRSjoin (MODULE_DEPENDENCIES (module), INFO_DS_DEPS (DSstate));

    DSstate = FreeInfo (DSstate);

    DBUG_VOID_RETURN;
}

/*
 * helper functions
 */

static void
updateContextInformation (node *entry)
{
    DBUG_ENTER ("updateContextInformation");

    switch (NODE_TYPE (entry)) {
    case N_fundef:
        /*
         * we have to update the WASUSED/WASIMPORTED
         * information for fundefs
         */
        if (INFO_DS_IMPORTMODE (DSstate)) {
            FUNDEF_WASIMPORTED (entry) = TRUE;
        } else {
            FUNDEF_WASUSED (entry) = TRUE;
        }
        break;
    default:
        break;
    }
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
        result = FindSymbolInFundefChain (symbol, INFO_DS_FUNDEFS (DSstate));
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

                DBUG_ASSERT ((serfun != NULL),
                             "module is inconsistent. cannot find function referenced in"
                             "symbol table");

                entryp = serfun (DSstate);
                /* add to ast */
                InsertIntoState (entryp);

                updateContextInformation (entryp);
            }
            break;
        default:
            DBUG_ASSERT (0, "unhandeled STentrytype");
            break;
        }
    }

    DBUG_RETURN (entryp);
}
/**
 * @brief adds the given symbol from the given module to the AST.
 *
 * Be aware, that the return value is any of the inserted symbols. In
 * case of wrapper functions, this may be any of the inserted wrappers.
 * So never rely on the returned pointer to point to the actually needed
 * wrapper!
 *
 * @param symbol the name of the symbol to be loaded
 * @param type the type of symbol to be loaded
 * @param module the module to load the symbol from
 *
 * @return a pointer to one of the loaded symbols
 */
node *
DSaddSymbolByName (const char *symbol, stentrytype_t type, const char *module)
{
    node *result = NULL;
    module_t *mod;
    const sttable_t *table;
    stentryiterator_t *it;

    DBUG_ENTER ("DSaddSymbolByName");

    DBUG_ASSERT ((DSstate != NULL),
                 "DSaddSymbolByName called without calling InitDeserialize.");

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
    mod = MODMunLoadModule (mod);

    DBUG_RETURN (result);
}

node *
DSaddSymbolById (const char *symbid, const char *module)
{
    module_t *mod;
    serfun_p fun;
    node *entryp;

    DBUG_ENTER ("DSaddSymbolById");

    mod = MODMloadModule (module);

    fun = MODMgetDeSerializeFunction (symbid, mod);

    DBUG_ASSERT ((fun != NULL), "requested symbol does not exist!");

    entryp = fun ();

    /* add to ast */
    InsertIntoState (entryp);

    updateContextInformation (entryp);

    mod = MODMunLoadModule (mod);

    DBUG_RETURN (entryp);
}

void
DSimportInstancesByName (const char *name, const char *module)
{
    module_t *mod;
    const sttable_t *table;
    stentryiterator_t *it;
    stentry_t *symbol;
    node *entryp;
    serfun_p serfun;

    DBUG_ENTER ("DSimportInstancesByName");

    DBUG_ASSERT ((DSstate != NULL),
                 "DSimportInstancesByName called without calling InitDeserialize.");

    DBUG_PRINT ("DS", ("processing import of '%s:%s'...", module, name));

    mod = MODMloadModule (module);

    table = MODMgetSymbolTable (mod);
    it = STentryIteratorGet (name, table);

    while (STentryIteratorHasMore (it)) {
        /*
         * fetch all wrappers matching the given symbol name
         */
        symbol = STentryIteratorNext (it);

        if (STentryType (symbol) == SET_wrapperhead) {
            INFO_DS_IMPORTMODE (DSstate) = TRUE;

            DBUG_PRINT ("DS", ("fetching instances for '%s:%s'...", module,
                               STentryName (symbol)));

            serfun = MODMgetDeSerializeFunction (STentryName (symbol), mod);

            DBUG_ASSERT ((serfun != NULL),
                         "found inconsistency between module and its symbol table");

            /*
             * fetch wrapper
             */
            entryp = serfun (DSstate);
            /*
             * and throw it away again
             */
            entryp = FREEdoFreeNode (entryp);
            entryp = FREEremoveAllZombies (entryp);

            INFO_DS_IMPORTMODE (DSstate) = FALSE;
        }
    }

    it = STentryIteratorRelease (it);
    mod = MODMunLoadModule (mod);

    DBUG_VOID_RETURN;
}

node *
DSloadFunctionBody (node *fundef)
{
    node *result = NULL;
    const char *serfunname;
    serfun_p serfun;
    module_t *module;

    DBUG_ENTER ("DSloadFunctionBody");

    module = MODMloadModule (NSgetModule (FUNDEF_NS (fundef)));

    serfunname = SERgenerateSerFunName ((FUNDEF_ISWRAPPERFUN (fundef) ? SET_wrapperbody
                                                                      : SET_funbody),
                                        fundef);

    serfun = MODMgetDeSerializeFunction (serfunname, module);

    SetCurrentFundefHead (fundef);

    result = serfun ();

    SetCurrentFundefHead (NULL);

    module = MODMunLoadModule (module);

    DBUG_RETURN (result);
}

/*
 * general deserialise and dispatch functions
 */

static node *
doDispatchFunCall (node *fundefs, const namespace_t *ns, const char *name,
                   ntype *argtypes)
{
    node *result = NULL;

    DBUG_ENTER ("doDispatchFunCall");

    while ((fundefs != NULL) && (result == NULL)) {

        if (FUNDEF_ISWRAPPERFUN (fundefs)) {
            if (NSequals (FUNDEF_NS (fundefs), ns)
                && ILIBstringCompare (FUNDEF_NAME (fundefs), name)) {
                if (TUsignatureMatches (FUNDEF_ARGS (fundefs), argtypes)) {
                    result = fundefs;
                }
            }
        }

        fundefs = FUNDEF_NEXT (fundefs);
    }

    DBUG_RETURN (result);
}

/**
 * @brief dispatches the given function call to the matching wrapper
 *        of the given function.
 *
 * @param mod module name
 * @param name function name
 * @param args N_exprs chain containing the arguments of the function call
 *
 * @return N_ap node encoding the call or NULL in case of failure
 */
node *
DSdispatchFunCall (const namespace_t *ns, const char *name, node *args)
{
    node *result = NULL;
    node *fundef;
    ntype *argtypes;

    DBUG_ENTER ("DSdispatchFunCall");

    DBUG_ASSERT ((DSstate != NULL),
                 "called doDispatchFunCall without initialising deserialise engine!");

    /*
     * first make sure the needed function is indeed available
     */
    DSaddSymbolByName (name, SET_wrapperhead, NSgetModule (ns));

    /*
     * now walk through the fundef chains and try to find
     * the correct wrapper or instance of that function
     * and do the dispatch!
     */

    argtypes = TUactualArgs2Ntype (args);

    fundef = doDispatchFunCall (INFO_DS_FUNDECS (DSstate), ns, name, argtypes);

    if (fundef == NULL) {
        fundef = doDispatchFunCall (INFO_DS_FUNDEFS (DSstate), ns, name, argtypes);

        if (fundef == NULL) {
            fundef = doDispatchFunCall (MODULE_FUNDECS (INFO_DS_MODULE (DSstate)), ns,
                                        name, argtypes);

            if (fundef == NULL) {
                fundef = doDispatchFunCall (MODULE_FUNS (INFO_DS_MODULE (DSstate)), ns,
                                            name, argtypes);
            }
        }
    }

    argtypes = TYfreeType (argtypes);

    if (fundef != NULL) {
        /*
         * build the Ap Node and add the corresponding module to the
         * dependencies.
         */
        result = TBmakeAp (fundef, args);

        INFO_DS_DEPS (DSstate)
          = STRSadd (NSgetModule (ns), STRS_saclib, INFO_DS_DEPS (DSstate));
    }

    DBUG_RETURN (result);
}

/*
 * hooks for the deserialisation process
 */

ntype *
DSloadUserType (const char *name, const namespace_t *ns)
{
    ntype *result;
    node *tdef;
    usertype udt;

    DBUG_ENTER ("DSloadUserType");

    tdef = DSaddSymbolByName (name, SET_typedef, NSgetModule (ns));

    DBUG_ASSERT ((tdef != NULL), "deserialisation of typedef failed!");

    udt = UTfindUserType (name, ns);

    DBUG_ASSERT ((udt != UT_NOT_DEFINED), "typedef not in udt repository");

    result = TYmakeUserType (udt);

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

        DBUG_ASSERT ((serfun != NULL),
                     "inconsistency in serialized module found. referenced "
                     "function does not exist");

        result = serfun ();
        mod = MODMunLoadModule (mod);

        InsertIntoState (result);
    }

    DBUG_ASSERT ((result != NULL), "lookup failed.");

    updateContextInformation (result);

    DBUG_RETURN (result);
}

node *
DSfetchArgAvis (int pos)
{
    node *arg;

    DBUG_ENTER ("DSfetchArgAvis");

    arg = FUNDEF_ARGS (getCurrentFundefHead ());

    while ((arg != NULL) && (pos != 0)) {
        pos--;
        arg = ARG_NEXT (arg);
    }

    DBUG_ASSERT ((pos == 0), "Referenced arg does not exist!");

    DBUG_RETURN (ARG_AVIS (arg));
}
