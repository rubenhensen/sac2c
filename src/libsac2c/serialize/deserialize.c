/*
 *
 * $Id$
 *
 */

#include "deserialize.h"
#include "serialize.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "user_types.h"
#include "new_types.h"
#include "shape.h"
#include "type_utils.h"
#include "new_typecheck.h"
#include "LookUpTable.h"
#include "symboltable.h"
#include "stringset.h"
#include "modulemanager.h"
#include "namespaces.h"
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

    result = MEMmalloc (sizeof (info));

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

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * the state of the deserializer is stored here so we
 * do not have to pass it around. This makes the code
 * generation for serializer functions alot easier
 */
static info *DSstate = NULL;

/*
 * we store the aliasing lut outside of the state as
 * it needs to be preserved between multiple runs of
 * the deserializer and can be filled even though the
 * deserializer is not initialized.
 */
static lut_t *aliasinglut = NULL;

/*
 * we store aliasing structures in the LUT. this
 * allows us to stack aliasings!
 */
struct DS_ALIASING {
    node *alias;
    struct DS_ALIASING *next;
};

typedef struct DS_ALIASING ds_aliasing_t;

static ds_aliasing_t *
makeAliasing (node *target, ds_aliasing_t *next)
{
    ds_aliasing_t *result;

    DBUG_ENTER ("makeAliasing");

    result = MEMmalloc (sizeof (ds_aliasing_t));

    result->alias = target;
    result->next = next;

    DBUG_RETURN (result);
}

/*
 * functions handling the internal state
 */

static void
InsertIntoState (node *item)
{
    usertype udt, alias;

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
         * first add the type to the UDT repository. We have to
         * check for aliases here, as these are handeled differently
         * by the udt database.
         */
        if (TYPEDEF_ISALIAS (item)) {
            DBUG_ASSERT (TYisAKSUdt (TYPEDEF_NTYPE (item)), "invalid type alias found!");
            DBUG_ASSERT ((TYgetDim (TYPEDEF_NTYPE (item)) == 0),
                         "non scalar type as type alias found");

            alias = TYgetUserType (TYgetScalar (TYPEDEF_NTYPE (item)));

            udt = UTaddAlias (STRcpy (TYPEDEF_NAME (item)),
                              NSdupNamespace (TYPEDEF_NS (item)), alias, NODE_LINE (item),
                              item);
        } else {
            udt = UTaddUserType (STRcpy (TYPEDEF_NAME (item)),
                                 NSdupNamespace (TYPEDEF_NS (item)),
                                 TYcopyType (TYPEDEF_NTYPE (item)), NULL,
                                 NODE_LINE (item), item);
        }

        /*
         * now compute the basetype
         */
        TUcheckUdtAndSetBaseType (udt, NULL);

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

    MODULE_OBJS (module)
      = TCappendObjdef (MODULE_OBJS (module), INFO_DS_OBJDEFS (DSstate));

    global.dependencies = STRSjoin (global.dependencies, INFO_DS_DEPS (DSstate));

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
            /*
             * this is a hack somehow, but for the time
             * being the best solution. We never mark
             * specialised funs as imported to prevent them
             * to be added to the local wrapper and thus
             * resolve the multiple-instances by specialisation
             * problem.
             *
             * HACK: sah
             */
            if (!FUNDEF_ISSPECIALISATION (entry)) {
                FUNDEF_WASIMPORTED (entry) = TRUE;
            }
        }

        /*
         * it is always safe to mark a function as used, so
         * we can simply do so here
         */
        FUNDEF_WASUSED (entry) = TRUE;

        break;
    default:
        break;
    }
    DBUG_VOID_RETURN;
}

static char *
HeadSymbol2BodySymbol (const char *symbol)
{
    char *result;

    DBUG_ENTER ("HeadSymbol2BodySymbol");

    DBUG_ASSERT (((symbol[0] == 'S') && (symbol[1] == 'H') && (symbol[2] == 'D')),
                 "given symbol is not a function header symbol!");

    result = MEMmalloc (strlen (symbol) + 2);

    snprintf (result, strlen (symbol) + 2, "S%s", symbol);

    result[1] = 'B';
    result[2] = 'D';
    result[3] = 'Y';

    DBUG_RETURN (result);
}

/*
 * functions handling the aliasing
 */

static void
initAliasingLut ()
{
    DBUG_ENTER ("initAliasingLut");

    aliasinglut = LUTgenerateLut ();

    DBUG_VOID_RETURN;
}

void
DSaddAliasing (const char *symbol, node *target)
{
    ds_aliasing_t *oldalias;
    ds_aliasing_t *alias;
    void **search;

    DBUG_ENTER ("DSaddAliasing");

    if (aliasinglut == NULL) {
        initAliasingLut ();
    }

    DBUG_PRINT ("DS_ALIAS", ("adding new aliasing for %s", symbol));

    search = LUTsearchInLutS (aliasinglut, (char *)symbol);

    if (search != NULL) {
        oldalias = (ds_aliasing_t *)*search;
        DBUG_PRINT ("DS_ALIAS", (">>> will hide old alias"));

    } else {
        oldalias = NULL;
    }

    alias = makeAliasing (target, oldalias);

    aliasinglut = LUTinsertIntoLutS (aliasinglut, (char *)symbol, alias);

    DBUG_VOID_RETURN;
}

void
DSremoveAliasing (const char *symbol)
{
    ds_aliasing_t *oldalias;
    void **search;

    DBUG_ENTER ("DSremoveAliasing");

    DBUG_PRINT ("DS_ALIAS", ("removing aliasing for %s", symbol));

    DBUG_ASSERT ((aliasinglut != NULL),
                 "cannot remove a aliasing without ever defining one!");

    search = LUTsearchInLutS (aliasinglut, (char *)symbol);

    if (search != NULL) {
        /*
         * fetch the old alias which has been shadowed by the current
         * one. If this was the last aliasing, we will put a NULL pointer
         * into the LUT. As we cannot delete an entry anyways (that
         * would be too expensive), this will do as well.
         */
        oldalias = (ds_aliasing_t *)*search;
        oldalias = oldalias->next;

#ifndef DBUG_OFF
        if (oldalias != NULL) {
            DBUG_PRINT ("DS_ALIAS", (">>> this will unhide old alias"));
        }
#endif
    } else {
        DBUG_ASSERT (0, "no alias to remove found!");
        oldalias = NULL;
    }

    aliasinglut = LUTupdateLutS (aliasinglut, (char *)symbol, oldalias, NULL);

    DBUG_VOID_RETURN;
}

static node *
getAliasing (const char *symbol)
{
    ds_aliasing_t *alias = NULL;
    void **search;
    node *result;

    DBUG_ENTER ("getAliasing");

    search = LUTsearchInLutS (aliasinglut, (char *)symbol);

    if (search != NULL) {
        alias = (ds_aliasing_t *)*search;
    }

    if (alias == NULL) {
        result = NULL;
    } else {
        result = alias->alias;
        DBUG_PRINT ("DS_ALIAS", ("using alias for %s", symbol));
    }

    DBUG_RETURN (result);
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
            if (STReq (FUNDEF_SYMBOLNAME (fundefs), symbol)) {
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
            if (STReq (TYPEDEF_SYMBOLNAME (typedefs), symbol)) {
                result = typedefs;
            }
        }
        typedefs = TYPEDEF_NEXT (typedefs);
    }

    DBUG_RETURN (result);
}

static node *
FindSymbolInObjdefChain (const char *symbol, node *objdefs)
{
    node *result = NULL;

    DBUG_ENTER ("FindSymbolInObjdefChain");

    while ((result == NULL) && (objdefs != NULL)) {
        if (OBJDEF_SYMBOLNAME (objdefs) != NULL) {
            if (STReq (OBJDEF_SYMBOLNAME (objdefs), symbol)) {
                result = objdefs;
            }
        }
        objdefs = OBJDEF_NEXT (objdefs);
    }

    DBUG_RETURN (result);
}

static node *
FindSymbolInAst (const char *symbol)
{
    node *result = NULL;

    DBUG_ENTER ("FindSymbolInAst");

    result = getAliasing (symbol);

#ifndef DBUG_OFF
    if (result != NULL) {
        DBUG_PRINT ("DS_ALIAS",
                    ("using alias %s for symbol %s.", CTIitemName (result), symbol));
    }
#endif

    if (result == NULL) {
        result = FindSymbolInFundefChain (symbol, INFO_DS_FUNDEFS (DSstate));
    }

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

    if (result == NULL) {
        result = FindSymbolInObjdefChain (symbol, INFO_DS_OBJDEFS (DSstate));
    }

    if (result == NULL) {
        result = FindSymbolInObjdefChain (symbol, MODULE_OBJS (INFO_DS_MODULE (DSstate)));
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
             * fetch wrapper: as the instances are encoded within the wrapper type,
             *                we will automatically get all instances as well.
             *                as we have set INFO_DS_IMPORTMODE, those will be
             *                even imported.
             */
            entryp = serfun (DSstate);

            /*
             * free wrapper: as we only loaded the wrapper to make sure
             *               the instances are fetched, we can now free
             *               it again. note here, that it never was inserted
             *               into the ast, as we called the deserialiser function
             *               directly.
             */
            entryp = FREEdoFreeTree (entryp);

            INFO_DS_IMPORTMODE (DSstate) = FALSE;
        }
    }

    it = STentryIteratorRelease (it);
    mod = MODMunLoadModule (mod);

    DBUG_VOID_RETURN;
}

void
DSimportTypedefByName (const char *name, const char *module)
{
    node *orig_tdef;

    DBUG_ENTER ("DSimportTypedefByName");

    /*
     * first make sure the typedef to be imported is available
     */
    orig_tdef = DSaddSymbolByName (name, SET_typedef, module);

    /*
     * we allow the original typedef not to exist. This is mainly
     * to allow this function to be called without checking
     * whether a typedef by that name really exists. as we
     * always import entire symbols, it does not matter whether
     * we in fact imported a typedef or not.
     */
    if (orig_tdef != NULL) {
        node *new_tdef;
        usertype orig_udt;
        ntype *alias;
        node *exist_tdef;

        orig_udt = UTfindUserType (TYPEDEF_NAME (orig_tdef), TYPEDEF_NS (orig_tdef));

        /*
         * first check whether we have imported that type earlier
         * on.
         */
        exist_tdef = TCsearchTypedef (TYPEDEF_NAME (orig_tdef), global.modulenamespace,
                                      INFO_DS_TYPEDEFS (DSstate));
        if (exist_tdef == NULL) {
            exist_tdef
              = TCsearchTypedef (TYPEDEF_NAME (orig_tdef), global.modulenamespace,
                                 MODULE_TYPES (INFO_DS_MODULE (DSstate)));
        }

        /*
         * Check that either
         * - there is no typedef
         * - the typedef that exists is not an ALIAS
         * - if it is an ALIAS, it aliases a different type
         * In all three cases create a new user alias. Otherwise
         * the existing typedef will do.
         */
        if ((exist_tdef == NULL) || (!TYPEDEF_ISALIAS (exist_tdef))
            || (UTgetUnAliasedType (
                  TYgetUserType (TYgetScalar (TYPEDEF_NTYPE (exist_tdef))))
                != UTgetUnAliasedType (orig_udt))) {
            /*
             * as this is an alias, we set the declared type to
             * the original type.
             */
            alias = TYmakeAKS (TYmakeUserType (orig_udt), SHmakeShape (0));

            /*
             * construct the new typedef and mark it as an alias
             */
            new_tdef
              = TBmakeTypedef (STRcpy (TYPEDEF_NAME (orig_tdef)),
                               NSdupNamespace (global.modulenamespace), alias, NULL);
            TYPEDEF_ISALIAS (new_tdef) = TRUE;

            /*
             * we have to propagate the uniqueness property as well
             */
            TYPEDEF_ISUNIQUE (new_tdef) = TYPEDEF_ISUNIQUE (orig_tdef);

            /*
             * insert it into the state. we cannot use InsertIntoState here, as we
             * have created a new local typedef and InsertIntoState is meant for
             * adding non-local items to the AST only.
             */
            INFO_DS_TYPEDEFS (DSstate)
              = TCappendTypedef (INFO_DS_TYPEDEFS (DSstate), new_tdef);
        }
    }

    DBUG_VOID_RETURN;
}

void
DSimportObjdefByName (const char *name, const char *module)
{
    node *orig_objdef;

    DBUG_ENTER ("DSimportObjdefByName");

    /*
     * first make sure the objdef to be imported is available
     */
    orig_objdef = DSaddSymbolByName (name, SET_objdef, module);

    /*
     * we allow the original objdef not to exist. This is mainly
     * to allow this function to be called without checking
     * whether an objdef by that name really exists. as we
     * always import entire symbols, it does not matter whether
     * we in fact imported an objdef or not.
     */
    if (orig_objdef != NULL) {
        node *new_objdef;
        /*
         * construct the new objdef and mark it as an alias
         */
        new_objdef = TBmakeObjdef (TYcopyType (OBJDEF_TYPE (orig_objdef)),
                                   NSdupNamespace (global.modulenamespace),
                                   STRcpy (OBJDEF_NAME (orig_objdef)),
                                   TBmakeGlobobj (orig_objdef), NULL);
        OBJDEF_ISALIAS (new_objdef) = TRUE;

        /*
         * insert it into the state. we cannot use InsertIntoState here, as we
         * have created a new local objdef and InsertIntoState is meant for
         * adding non-local items to the AST only.
         */
        INFO_DS_OBJDEFS (DSstate)
          = TCappendObjdef (INFO_DS_OBJDEFS (DSstate), new_objdef);
    }

    DBUG_VOID_RETURN;
}

node *
DSloadFunctionBody (node *fundef)
{
    node *result = NULL;
    char *serfunname;
    serfun_p serfun;
    module_t *module;

    DBUG_ENTER ("DSloadFunctionBody");

    module = MODMloadModule (NSgetModule (FUNDEF_NS (fundef)));

    DBUG_ASSERT ((FUNDEF_SYMBOLNAME (fundef) != NULL),
                 "cannot load body for a function without symbolname!");

    serfunname = HeadSymbol2BodySymbol (FUNDEF_SYMBOLNAME (fundef));

    DBUG_PRINT ("DS_BODY", ("deserializing fundef body for symbol %s...", serfunname));

    SetCurrentFundefHead (fundef);

    serfun = MODMgetDeSerializeFunction (serfunname, module);

    serfunname = MEMfree (serfunname);

    SetCurrentFundefHead (fundef);

    DBUG_ASSERT ((serfun != NULL),
                 "deserializer not found. module seems to be inconsistent!");

    global.valid_ssaform = FALSE;
    result = serfun ();
    global.valid_ssaform = TRUE;

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
                && STReq (FUNDEF_NAME (fundefs), name)) {
                if (TUsignatureMatches (FUNDEF_ARGS (fundefs), argtypes, FALSE)) {
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

usertype
DSloadUserType (const char *symbid, const namespace_t *ns)
{
    node *tdef;
    usertype udt;

    DBUG_ENTER ("DSloadUserType");

    tdef = FindSymbolInAst (symbid);

    if (tdef == NULL) {
        tdef = DSaddSymbolById (symbid, NSgetModule (ns));
    }

    DBUG_ASSERT ((tdef != NULL), "deserialisation of typedef failed!");

    udt = UTfindUserType (TYPEDEF_NAME (tdef), ns);

    DBUG_ASSERT ((udt != UT_NOT_DEFINED), "typedef not in udt repository");

    DBUG_RETURN (udt);
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
        DBUG_PRINT ("DS",
                    ("Looking up function `%s:%s' in `%s'.", module, symbol, module));

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
DSlookupObject (const char *module, const char *symbol)
{
    node *result = NULL;
    serfun_p serfun;
    module_t *mod;

    DBUG_ENTER ("DSlookupObjdef");

    DBUG_PRINT ("DS", ("Looking up objdef `%s:%s' in ast.", module, symbol));

    result = FindSymbolInAst (symbol);

    if (result == NULL) {
        DBUG_PRINT ("DS", ("Looking up objdef `%s:%s' in `%s'.", module, symbol, module));

        mod = MODMloadModule (module);
        serfun = MODMgetDeSerializeFunction (symbol, mod);

        DBUG_ASSERT ((serfun != NULL),
                     "inconsistency in serialized module found. referenced "
                     "objdef does not exist");

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

/*
 * deserialize helpers
 */
double
DShex2Double (const char *string)
{
    double res;

    DBUG_ENTER ("DShex2Double");

    STRhex2Bytes ((unsigned char *)&res, string);

    DBUG_RETURN (res);
}

float
DShex2Float (const char *string)
{
    float res;

    DBUG_ENTER ("DShex2Float");

    STRhex2Bytes ((unsigned char *)&res, string);

    DBUG_RETURN (res);
}
