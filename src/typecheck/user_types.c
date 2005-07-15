/*
 * $Log$
 * Revision 3.10  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 3.9  2005/01/11 14:20:44  cg
 * Converted output generation from Error.h to ctinfo.c
 *
 * Revision 3.8  2005/01/10 17:27:06  cg
 * Converted error messages from Error.h to ctinfo.c
 *
 * Revision 3.7  2004/11/27 02:12:28  sah
 * ...
 *
 * Revision 3.6  2004/11/23 13:25:10  sbs
 * SacDevCamp04: compiles again
 *
 * Revision 3.5  2004/11/17 19:46:19  sah
 * changed arguments from char to const char
 *
 * Revision 3.4  2002/10/18 14:30:12  sbs
 * made the type definition node part of the repository record
 *
 * Revision 3.3  2002/08/05 17:01:02  sbs
 * minor bugs fixed
 *
 * Revision 3.2  2001/05/17 09:20:42  sbs
 * MALLOC FREE aliminated
 *
 * Revision 3.1  2000/11/20 18:00:23  sacbase
 * new release made
 *
 * Revision 1.1  1999/10/12 15:38:15  sbs
 * Initial revision
 *
 *
 */

#include <string.h>

#include "dbug.h"
#include "ctinfo.h"
#include "free.h"
#include "internal_lib.h"
#include "namespaces.h"
#include "new_types.h"
#include "user_types.h"

/*
 * This module "user_type" implements a repository for user defined types.
 *
 * The repository keeps entries of the following kind:
 *
 *  udt# | type-name | type-module | defining type | base type | line#
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
    int line;
    node *tdef;
} udt_entry;

/*
 * For internal usage within this module only, we define the following
 * access macros:
 */

#define ENTRY_NAME(e) (e->name)
#define ENTRY_NS(e) (e->mod)
#define ENTRY_DEF(e) (e->type)
#define ENTRY_BASE(e) (e->base)
#define ENTRY_LINE(e) (e->line)
#define ENTRY_TDEF(e) (e->tdef)

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

/******************************************************************************
 *
 * function:
 *    usertype UTaddUserType( char *name, namespace_t *ns, ntype *type,
 *                            ntype *base, int lineno, node *tdef)
 *
 * description:
 *   adds a udt to the repository and enlarges it whenever (udt_no % CHUNKSIZE)
 *   equals 0. It returns the index of the udt_entry generated.
 *
 ******************************************************************************/

usertype
UTaddUserType (char *name, namespace_t *ns, ntype *type, ntype *base, int lineno,
               node *tdef)
{
    udt_entry *entry;
    udt_entry **new_rep;
    int i;

    DBUG_ENTER ("UTaddUserType");

    /*
     * First, we generate the desired entry:
     */
    entry = (udt_entry *)ILIBmalloc (sizeof (udt_entry));
    ENTRY_NAME (entry) = name;
    ENTRY_NS (entry) = ns;
    ENTRY_DEF (entry) = type;
    ENTRY_BASE (entry) = base;
    ENTRY_LINE (entry) = lineno;
    ENTRY_TDEF (entry) = tdef;

    /*
     * Before putting the new entry into the repository, we have to make sure
     * the repository is big enough!!
     */
    if (udt_no % CHUNKSIZE == 0) {
        new_rep = (udt_entry **)ILIBmalloc ((udt_no + CHUNKSIZE) * sizeof (udt_entry *));
        for (i = 0; i < udt_no; i++) {
            new_rep[i] = udt_rep[i];
        }
        if (udt_rep != NULL) { /* to cope with the initial allocation */
            ILIBfree (udt_rep);
        }
        udt_rep = new_rep;
    }
    udt_rep[udt_no] = entry;

    DBUG_RETURN (udt_no++);
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

    DBUG_ENTER ("UTfindUserType");

    DBUG_ASSERT ((UT_NOT_DEFINED == -1), "Invalid definition of UT_NOT_DEFINED");
    DBUG_ASSERT ((name != NULL), "UTFindUserType called with NULL name!");

    res = udt_no - 1;
    if (ns == NULL) {
        while ((res >= 0) && (strcmp (name, ENTRY_NAME (udt_rep[res])) != 0)) {
            res--;
        }
        res2 = res - 1;
        while ((res2 >= 0) && (strcmp (name, ENTRY_NAME (udt_rep[res2])) != 0)) {
            res2--;
        }
        if (res2 >= 0) {
            CTIerrorLine (global.linenum,
                          "User defined type \"%s\" can not uniquely be determined",
                          name);
        }
    } else {
        while ((res >= 0)
               && ((!ILIBstringCompare (name, ENTRY_NAME (udt_rep[res])))
                   || (!NSequals (ns, ENTRY_NS (udt_rep[res]))))) {
            res--;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    int UTgetNumberOfUserTypes()
 *
 * description:
 *   returns the number of user defined types currently allocated.
 *
 ******************************************************************************/

int
UTgetNumberOfUserTypes ()
{
    DBUG_ENTER ("UTgetNumberOfUserTypes");

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
    DBUG_ENTER ("UTgetNamespace");
    DBUG_ASSERT ((udt < udt_no), "UTgetNamespace called with illegal udt!");

    DBUG_RETURN (ENTRY_NS (udt_rep[udt]));
}

char *
UTgetName (usertype udt)
{
    DBUG_ENTER ("UTgetName");
    DBUG_ASSERT ((udt < udt_no), "UTgetName called with illegal udt!");

    DBUG_RETURN (ENTRY_NAME (udt_rep[udt]));
}

ntype *
UTgetTypedef (usertype udt)
{
    DBUG_ENTER ("UTgetTypedef");
    DBUG_ASSERT ((udt < udt_no), "UTgetTypedef called with illegal udt!");

    DBUG_RETURN (ENTRY_DEF (udt_rep[udt]));
}

ntype *
UTgetBaseType (usertype udt)
{
    DBUG_ENTER ("UTgetBaseType");
    DBUG_ASSERT ((udt < udt_no), "UTgetBaseType called with illegal udt!");

    DBUG_RETURN (ENTRY_BASE (udt_rep[udt]));
}

int
UTgetLine (usertype udt)
{
    DBUG_ENTER ("UTgetLine");
    DBUG_ASSERT ((udt < udt_no), "UTgetLine called with illegal udt!");

    DBUG_RETURN (ENTRY_LINE (udt_rep[udt]));
}

node *
UTgetTdef (usertype udt)
{
    DBUG_ENTER ("UTgetTdef");
    DBUG_ASSERT ((udt < udt_no), "UTgetTdef called with illegal udt!");

    DBUG_RETURN (ENTRY_TDEF (udt_rep[udt]));
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
    DBUG_ENTER ("UTsetTypedef");
    DBUG_ASSERT ((udt < udt_no), "UTsetTypedef called with illegal udt!");

    ENTRY_DEF (udt_rep[udt]) = type;

    DBUG_VOID_RETURN;
}

void
UTsetBaseType (usertype udt, ntype *type)
{
    DBUG_ENTER ("UTsetBaseType");
    DBUG_ASSERT ((udt < udt_no), "UTsetBaseType called with illegal udt!");

    ENTRY_BASE (udt_rep[udt]) = type;
    DBUG_VOID_RETURN;
}

void
UTsetName (usertype udt, const char *name)
{
    DBUG_ENTER ("UTsetName");
    DBUG_ASSERT ((udt < udt_no), "UTsetName called with illegal udt!");

    ENTRY_NAME (udt_rep[udt]) = ILIBstringCopy (name);
    DBUG_VOID_RETURN;
}

void
UTsetNamespace (usertype udt, const namespace_t *ns)
{
    DBUG_ENTER ("UTsetNamespace");
    DBUG_ASSERT ((udt < udt_no), "UTsetNamespace called with illegal udt!");

    ENTRY_NS (udt_rep[udt]) = NSdupNamespace (ns);
    DBUG_VOID_RETURN;
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

#define UTPRINT_FORMAT "| %-10.10s | %-10.10s | %-20.20s | %-20.20s |"

void
UTprintRepository (FILE *outfile)
{
    int i;

    DBUG_ENTER ("UTprintRepository");

    fprintf (outfile, "\n %4.4s " UTPRINT_FORMAT " %6s | %9s\n", "udt:", "module:",
             "name:", "defining type:", "base type:", "line:", "def node:");
    for (i = 0; i < udt_no; i++) {
        fprintf (outfile, " %4d " UTPRINT_FORMAT " %6d |  %8p\n", i,
                 NSgetName (UTgetNamespace (i)), UTgetName (i),
                 TYtype2String (UTgetTypedef (i), TRUE, 0),
                 TYtype2String (UTgetBaseType (i), TRUE, 0), UTgetLine (i),
                 UTgetTdef (i));
    }

    DBUG_VOID_RETURN;
}
