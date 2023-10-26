#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "new_types.h"
#include "shape.h"
#include "user_types.h"
#include "globals.h"

/*
 * This module "user_type" implements a repository for user defined types.
 *
 * The repository keeps entries of the following kind:
 *
 *  udt# | type-name | type-module | defining type | base type | alias
 *    | line# | typedef
 *
 * All interfacing to that repository has to be made through the functions
 * defined in this module!
 *
 */

/*
 * First, we define the data structure for keeping a single user defined type.
 * While name, mod, and type are straightforward, "base" is introduced to
 * ease several phases of the compiler. It provides the so-called "base-type",
 * i.e., the SAC type in which the type actually will be stored!
 * "line" keeps the actual line number of the typedef which is only needed
 * for generating better error messages.
 */

typedef struct UDT_ENTRY {
    char *name;
    namespace_t *mod;
    ntype *type;
    ntype *base;
    usertype alias;
    size_t line;
    node *tdef;
    bool external;
    bool nested;
} udt_entry;

/*
 * For internal usage within this module only, we define the following
 * access macros:
 */

#define ENTRY_NAME(e) ((e)->name)
#define ENTRY_NS(e) ((e)->mod)
#define ENTRY_DEF(e) ((e)->type)
#define ENTRY_BASE(e) ((e)->base)
#define ENTRY_ALIAS(e) ((e)->alias)
#define ENTRY_LINE(e) ((e)->line)
#define ENTRY_TDEF(e) ((e)->tdef)
#define ENTRY_NESTED(e) ((e)->nested)
#define ENTRY_EXTERNAL(e) ((e)->external)

/*
 * We use a global datastructure "udt_rep", in order to keep all the information
 * regarding user defined types. It is implemented as an array of pointers to
 * the actual udt_entries. Choosing an array of entries eases the access;
 *
 * "usertype" simply refers to the position of the entry within "udt_rep"!!
 *
 * The choice of keeping an array of pointers rather than an array of entries
 * was made since the number of entries is not limited.
 */

static udt_entry **udt_rep = NULL;

/*
 * A global counter "udt_no" keeps track of the number of entries we currently have.
 * Since we only allow entries to be added, "udt_no" can also be used to determine
 * when to enlarge "udt_rep". This is done iff (udt_no % CHUNKSIZE) == 0 !!
 */

static int udt_no = 0;

#define CHUNKSIZE 20

/** <!-- ****************************************************************** -->
 * @fn usertype InsertIntoRepository( udt_entry *entry)
 *
 * @brief inserts the given udt_entry in the repository. the repository
 *        is enlarged whenever (udt_no % CHUNKSIZE) is 0.
 *
 * @param entry a new entry to be inserted
 *
 * @return the udt# that was assigned to the udt_entry
 ******************************************************************************/
static usertype
InsertIntoRepository (udt_entry *entry)
{
    udt_entry **new_rep;
    int i;

    DBUG_ENTER ();

    /*
     * Before putting the new entry into the repository, we have to make sure
     * the repository is big enough!!
     */
    if (udt_no % CHUNKSIZE == 0) {
        new_rep = (udt_entry **)MEMmalloc ((size_t)(udt_no + CHUNKSIZE) * sizeof (udt_entry *));
        for (i = 0; i < udt_no; i++) {
            new_rep[i] = udt_rep[i];
        }
        if (udt_rep != NULL) { /* to cope with the initial allocation */
            MEMfree (udt_rep);
        }
        udt_rep = new_rep;
    }
    udt_rep[udt_no] = entry;

    DBUG_RETURN (udt_no++);
}

/******************************************************************************
 *
 * function:
 *    usertype UTaddUserType( char *name, namespace_t *ns, ntype *type,
 *                            ntype *base, size_t lineno, node *tdef)
 *
 * description:
 *   adds a udt to the repository and enlarges it whenever (udt_no % CHUNKSIZE)
 *   equals 0. It returns the index of the udt_entry generated.
 *
 ******************************************************************************/

usertype
UTaddUserType (char *name, namespace_t *ns, ntype *type, ntype *base, size_t lineno,
               node *tdef, bool nested, bool external)
{
    udt_entry *entry;
    usertype result;

    DBUG_ENTER ();

    /*
     * First, we generate the desired entry:
     */
    entry = (udt_entry *)MEMmalloc (sizeof (udt_entry));
    ENTRY_NAME (entry) = name;
    ENTRY_NS (entry) = ns;
    ENTRY_DEF (entry) = type;
    ENTRY_BASE (entry) = base;
    ENTRY_LINE (entry) = lineno;
    ENTRY_TDEF (entry) = tdef;
    ENTRY_ALIAS (entry) = UT_NOT_DEFINED;
    ENTRY_NESTED (entry) = nested;
    ENTRY_EXTERNAL (entry) = external;

    result = InsertIntoRepository (entry);

    if (external) {
        TYsetHiddenUserType (TYgetScalar (type), result);  // self ref for external types!
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief adds a udt alias to the repository and  enlarges it whenever
 *        (udt_no % CHUNKSIZE) equals 0. It returns the index of the
 *        udt_entry generated.
 *        the defining type of the alias is set to AKS{ UDT{ alias} []
 *        and the basetype is taken from the alias.
 *
 * @param name name of alias
 * @param ns namespace of alias
 * @param alias type this is aliasing
 * @param lineno line number where the alias is defined
 * @param tdef the corresponding typedef node
 *
 * @return
 ******************************************************************************/
usertype
UTaddAlias (char *name, namespace_t *ns, usertype alias, size_t lineno, node *tdef)
{
    udt_entry *entry;
    int result;

    DBUG_ENTER ();
    DBUG_ASSERT (alias < udt_no, "alias in UTaddAlias out of range");

    entry = (udt_entry *)MEMmalloc (sizeof (udt_entry));
    ENTRY_NAME (entry) = name;
    ENTRY_NS (entry) = ns;
    ENTRY_DEF (entry) = TYmakeAKS (TYmakeUserType (alias), SHmakeShape (0));
    ENTRY_BASE (entry) = TYcopyType (UTgetBaseType (alias));
    ENTRY_LINE (entry) = lineno;
    ENTRY_TDEF (entry) = tdef;
    ENTRY_ALIAS (entry) = alias;

    result = InsertIntoRepository (entry);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *    usertype UTfindUserType( char *name, namespace_t *ns)
 *
 * description:
 *   looks up the udf ns::name. If it is not within the udt_rep, UT_NOT_DEFINED
 *   is returned.
 *   if ns is not yet given (i.e. ns == NULL), it is checked wether *:name
 *   matches more than one entry in udt_rep. If so, an ERROR message is posted
 *   and the most recent entry found is returned.
 *
 ******************************************************************************/

usertype
UTfindUserType (const char *name, const namespace_t *ns)
{
    int res, res2;

    DBUG_ENTER ();

    DBUG_ASSERT (UT_NOT_DEFINED == -1, "Invalid definition of UT_NOT_DEFINED");
    DBUG_ASSERT (name != NULL, "UTFindUserType called with NULL name!");

    res = udt_no - 1;
    if (ns == NULL) {
        while ((res >= 0) && !STReq (name, ENTRY_NAME (udt_rep[res]))) {
            res--;
        }
        res2 = res - 1;
        while ((res2 >= 0) && !STReq (name, ENTRY_NAME (udt_rep[res2]))) {
            res2--;
        }
        if (res2 >= 0) {
            CTIerror (LINE_TO_LOC (global.linenum),
                      "User defined type \"%s\" can not uniquely be determined",
                      name);
        }
    } else {
        while ((res >= 0)
               && ((!STReq (name, ENTRY_NAME (udt_rep[res])))
                   || (!NSequals (ns, ENTRY_NS (udt_rep[res]))))) {
            res--;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    int UTgetNumberOfUserTypes(void)
 *
 * description:
 *   returns the number of user defined types currently allocated.
 *
 ******************************************************************************/

int
UTgetNumberOfUserTypes (void)
{
    DBUG_ENTER ();

    DBUG_RETURN (udt_no);
}

/******************************************************************************
 *
 * function:
 *    char *UTgetNamespace( usertype udt)
 *    char *UTgetName( usertype udt)
 *    ntype *UTgetTypedef( usertype udt)
 *    ntype *UTgetBaseType( usertype udt)
 *    int UTgetLine( usertype udt)
 *    node *UTgetTdef( usertype udt)
 *
 * description:
 *   several functions for accessing the values of the definition of the user
 *   defined type udf.
 *
 ******************************************************************************/

const namespace_t *
UTgetNamespace (usertype udt)
{
    DBUG_ENTER ();
    DBUG_ASSERT (((udt < udt_no) && (udt > UT_NOT_DEFINED)),
                 "UTgetNamespace called with illegal udt!");

    DBUG_RETURN (ENTRY_NS (udt_rep[udt]));
}

char *
UTgetName (usertype udt)
{
    DBUG_ENTER ();
    DBUG_ASSERT (((udt < udt_no) && (udt > UT_NOT_DEFINED)),
                 "UTgetNamespace called with illegal udt!");

    DBUG_RETURN (ENTRY_NAME (udt_rep[udt]));
}

ntype *
UTgetTypedef (usertype udt)
{
    DBUG_ENTER ();
    DBUG_ASSERT (((udt < udt_no) && (udt > UT_NOT_DEFINED)),
                 "UTgetNamespace called with illegal udt!");

    DBUG_RETURN (ENTRY_DEF (udt_rep[udt]));
}

ntype *
UTgetBaseType (usertype udt)
{
    DBUG_ENTER ();
    DBUG_ASSERT (((udt < udt_no) && (udt > UT_NOT_DEFINED)),
                 "UTgetNamespace called with illegal udt!");

    DBUG_RETURN (ENTRY_BASE (udt_rep[udt]));
}

size_t
UTgetLine (usertype udt)
{
    DBUG_ENTER ();
    DBUG_ASSERT (((udt < udt_no) && (udt > UT_NOT_DEFINED)),
                 "UTgetNamespace called with illegal udt!");

    DBUG_RETURN (ENTRY_LINE (udt_rep[udt]));
}

node *
UTgetTdef (usertype udt)
{
    DBUG_ENTER ();
    DBUG_ASSERT (((udt < udt_no) && (udt > UT_NOT_DEFINED)),
                 "UTgetNamespace called with illegal udt!");

    DBUG_RETURN (ENTRY_TDEF (udt_rep[udt]));
}

usertype
UTgetAlias (usertype udt)
{
    usertype alias;

    DBUG_ENTER ();
    DBUG_ASSERT (((udt < udt_no) && (udt > UT_NOT_DEFINED)),
                 "UTgetNamespace called with illegal udt!");

    alias = ENTRY_ALIAS (udt_rep[udt]);

    DBUG_RETURN (alias);
}

usertype
UTgetUnAliasedType (usertype udt)
{
    usertype result;

    DBUG_ENTER ();
    DBUG_ASSERT (((udt < udt_no) && (udt > UT_NOT_DEFINED)),
                 "UTgetNamespace called with illegal udt!");

    if (ENTRY_ALIAS (udt_rep[udt]) != UT_NOT_DEFINED) {
        result = UTgetUnAliasedType (ENTRY_ALIAS (udt_rep[udt]));
    } else {
        result = udt;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *    ntype *UTsetTypedef( usertype udt)
 *    ntype *UTsetBaseType( usertype udt)
 *    void   UTsetName( usertype udt, const char *name)
 *    void   UTsetMod( usertype udt, const char *mod)
 *
 * description:
 *   several functions for changing the values of the definition of the user
 *   defined type udf.
 *
 ******************************************************************************/

void
UTsetTypedef (usertype udt, ntype *type)
{
    DBUG_ENTER ();
    DBUG_ASSERT (udt < udt_no, "UTsetTypedef called with illegal udt!");

    ENTRY_DEF (udt_rep[udt]) = type;

    DBUG_RETURN ();
}

void
UTsetBaseType (usertype udt, ntype *type)
{
    DBUG_ENTER ();
    DBUG_ASSERT (udt < udt_no, "UTsetBaseType called with illegal udt!");

    ENTRY_BASE (udt_rep[udt]) = type;
    DBUG_RETURN ();
}

void
UTsetName (usertype udt, const char *name)
{
    DBUG_ENTER ();
    DBUG_ASSERT (udt < udt_no, "UTsetName called with illegal udt!");

    ENTRY_NAME (udt_rep[udt]) = STRcpy (name);
    DBUG_RETURN ();
}

void
UTsetNamespace (usertype udt, namespace_t *ns)
{
    DBUG_ENTER ();
    DBUG_ASSERT (udt < udt_no, "UTsetNamespace called with illegal udt!");

    ENTRY_NS (udt_rep[udt]) = NSdupNamespace (ns);
    DBUG_RETURN ();
}

/** <!-- ****************************************************************** -->
 * @fn bool UTeq( usertype udt1, usertype udt2)
 *
 * @brief Checks whether the two udts are equal. This function takes
 *        aliasing of user defined types into account!
 *
 * @param udt1 first user-defined type
 * @param udt2 second user-defined type
 *
 * @return
 ******************************************************************************/
bool
UTeq (usertype udt1, usertype udt2)
{
    bool result;

    DBUG_ENTER ();

    result = (UTgetUnAliasedType (udt1) == UTgetUnAliasedType (udt2));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn bool UTisAlias( usertype udt)
 *
 * @brief checks whether the passed udt is an aliasing.
 *
 * @param udt
 *
 * @return
 ******************************************************************************/
bool
UTisAlias (usertype udt)
{
    DBUG_ENTER ();
    DBUG_ASSERT (udt < udt_no, "UTisAlias called with illegal udt!");

    DBUG_RETURN (ENTRY_ALIAS (udt_rep[udt]) != UT_NOT_DEFINED);
}

/** <!-- ****************************************************************** -->
 * @fn bool UTisNested( usertype udt)
 *
 * @brief checks whether the passed udt is a nested type
 *
 * @param udt
 *
 * @return
 ******************************************************************************/
bool
UTisNested (usertype udt)
{
    DBUG_ENTER ();

    DBUG_RETURN (ENTRY_NESTED (udt_rep[udt]));
}

/** <!-- ****************************************************************** -->
 * @fn bool UTisExternal( usertype udt)
 *
 * @brief checks whether the passed udt is an external type
 *
 * @param udt
 *
 * @return
 ******************************************************************************/
bool
UTisExternal (usertype udt)
{
    DBUG_ENTER ();

    DBUG_RETURN (ENTRY_EXTERNAL (udt_rep[udt]));
}

/******************************************************************************
 *
 * function:
 *    void UTprintRepository( FILE *outfile)
 *
 * description:
 *   prints the contents of udt_rep to outfile.
 *
 ******************************************************************************/

//                        module     name      def-type    base-type
#define UTPRINT_FORMAT "| %-10.10s | %-10.10s | %-25.25s | %-35.35s |"

void
UTprintRepository (FILE *outfile)
{
    int i, alias;

    DBUG_ENTER ();

    fprintf (outfile, "\n %4.4s " UTPRINT_FORMAT " %10s | %-7s | %-7s | %-7s | %-14s \n", "udt:", "module:",
             "name:", "defining type:", "base type:", "alias udt:", "nested:", "extern:", "line", "def node:");
    for (i = 0; i < udt_no; i++) {
        alias = UTgetAlias (i);
        if (alias == UT_NOT_DEFINED) {
            fprintf (outfile, " %4d " UTPRINT_FORMAT " %-10s | %-7s | %-7s | %-7zu | %-14p \n", i,
                     NSgetName (UTgetNamespace (i)), UTgetName (i),
                     TYtype2String (UTgetTypedef (i), TRUE, 0),
                     TYtype2String (UTgetBaseType (i), TRUE, 0), 
                     "---", (UTisNested (i)?"yes":""),
                     (UTisExternal (i)?"yes":""), UTgetLine (i),
                     (void *)UTgetTdef (i));
        } else {
            fprintf (outfile, " %4d " UTPRINT_FORMAT " %-10d | %-7s | %-7s | %-7zu | %-14p \n", i,
                     NSgetName (UTgetNamespace (i)), UTgetName (i),
                     TYtype2String (UTgetTypedef (i), TRUE, 0),
                     TYtype2String (UTgetBaseType (i), TRUE, 0), 
                     UTgetAlias (i), (UTisNested (i)?"yes":""),
                     (UTisExternal (i)?"yes":""), UTgetLine (i),
                     (void *)UTgetTdef (i));
        }
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
