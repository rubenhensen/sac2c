/*
 * $Log$
 * Revision 1.1  1999/10/12 15:38:15  sbs
 * Initial revision
 *
 *
 */

#include <string.h>
#include "Error.h"
#include "dbug.h"

#include "free.h"
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
    char *mod;
    ntype *type;
    ntype *base;
    int line;
} udt_entry;

/*
 * For internal usage within this module only, we define the following
 * access macros:
 */

#define ENTRY_NAME(e) (e->name)
#define ENTRY_MOD(e) (e->mod)
#define ENTRY_DEF(e) (e->type)
#define ENTRY_BASE(e) (e->base)
#define ENTRY_LINE(e) (e->line)

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
 *    usertype UTAddUserType( char *name, char *mod, ntype *type, ntype *base,
 *                            int lineno)
 *
 * description:
 *   adds a udt to the repository and enlarges it whenever (udt_no % CHUNKSIZE)
 *   equals 0. It returns the index of the udt_entry generated.
 *
 ******************************************************************************/

usertype
UTAddUserType (char *name, char *mod, ntype *type, ntype *base, int lineno)
{
    udt_entry *entry;
    udt_entry **new_rep;
    int i;

    DBUG_ENTER ("UTAddUserType");

    /*
     * First, we generate the desired entry:
     */
    entry = (udt_entry *)MALLOC (sizeof (udt_entry));
    ENTRY_NAME (entry) = name;
    ENTRY_MOD (entry) = mod;
    ENTRY_DEF (entry) = type;
    ENTRY_BASE (entry) = base;
    ENTRY_LINE (entry) = lineno;

    /*
     * Before putting the new entry into the repository, we have to make sure
     * the repository is big enough!!
     */
    if (udt_no % CHUNKSIZE == 0) {
        new_rep = (udt_entry **)MALLOC ((udt_no + CHUNKSIZE) * sizeof (udt_entry *));
        for (i = 0; i < udt_no; i++) {
            new_rep[i] = udt_rep[i];
        }
        if (udt_rep != NULL) { /* to cope with the initial allocation */
            FREE (udt_rep);
        }
        udt_rep = new_rep;
    }
    udt_rep[udt_no] = entry;

    DBUG_RETURN (udt_no++);
}

/******************************************************************************
 *
 * function:
 *    usertype UTFindUserType( char *name, char *mod)
 *
 * description:
 *   looks up the udf mod:name. If it is not within the udt_rep, UT_NOT_DEFINED
 *   is returned.
 *   if mod is not yet given (i.e. mod == NULL), it is checked wether *:name
 *   matches more than one entry in udt_rep. If so, an ERROR message is posted
 *   and the most recent entry found is returned.
 *
 ******************************************************************************/

usertype
UTFindUserType (char *name, char *mod)
{
    int res, res2;

    DBUG_ENTER ("UTFindUserType");

    DBUG_ASSERT ((UT_NOT_DEFINED == -1), "Invalid definition of UT_NOT_DEFINED");
    DBUG_ASSERT ((name != NULL), "UTFindUserType called with NULL name!");

    res = udt_no - 1;
    if (mod == NULL) {
        while ((res >= 0) && (strcmp (name, ENTRY_NAME (udt_rep[res])) != 0)) {
            res--;
        }
        res2 = res - 1;
        while ((res2 >= 0) && (strcmp (name, ENTRY_NAME (udt_rep[res2])) != 0)) {
            res2--;
        }
        if (res2 >= 0) {
            ERROR (linenum,
                   ("user defined type \"%s\" can not uniquely be determined", name));
        }
    } else {
        while ((res >= 0)
               && ((strcmp (name, ENTRY_NAME (udt_rep[res])) != 0)
                   || (strcmp (mod, ENTRY_MOD (udt_rep[res])) != 0))) {
            res--;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    int UTGetNumberOfUserTypes()
 *
 * description:
 *   returns the number of user defined types currently allocated.
 *
 ******************************************************************************/

int
UTGetNumberOfUserTypes ()
{
    DBUG_ENTER ("UTGetNumberOfUserTypes");

    DBUG_RETURN (udt_no);
}

/******************************************************************************
 *
 * function:
 *    char *UTGetMod( usertype udt)
 *    char *UTGetName( usertype udt)
 *    ntype *UTGetTypedef( usertype udt)
 *    ntype *UTGetBaseType( usertype udt)
 *    int UTGetLine( usertype udt)
 *
 * description:
 *   several functions for accessing the values of the definition of the user
 *   defined type udf.
 *
 ******************************************************************************/

char *
UTGetMod (usertype udt)
{
    DBUG_ENTER ("UTGetMod");
    DBUG_ASSERT ((udt < udt_no), "UTGetMod called with illegal udt!");

    DBUG_RETURN (ENTRY_MOD (udt_rep[udt]));
}

char *
UTGetName (usertype udt)
{
    DBUG_ENTER ("UTGetName");
    DBUG_ASSERT ((udt < udt_no), "UTGetName called with illegal udt!");

    DBUG_RETURN (ENTRY_NAME (udt_rep[udt]));
}

ntype *
UTGetTypedef (usertype udt)
{
    DBUG_ENTER ("UTGetTypedef");
    DBUG_ASSERT ((udt < udt_no), "UTGetTypedef called with illegal udt!");

    DBUG_RETURN (ENTRY_DEF (udt_rep[udt]));
}

ntype *
UTGetBaseType (usertype udt)
{
    DBUG_ENTER ("UTGetBaseType");
    DBUG_ASSERT ((udt < udt_no), "UTGetBaseType called with illegal udt!");

    DBUG_RETURN (ENTRY_BASE (udt_rep[udt]));
}

int
UTGetLine (usertype udt)
{
    DBUG_ENTER ("UTGetLine");
    DBUG_ASSERT ((udt < udt_no), "UTGetLine called with illegal udt!");

    DBUG_RETURN (ENTRY_LINE (udt_rep[udt]));
}

/******************************************************************************
 *
 * function:
 *    ntype *UTSetTypedef( usertype udt)
 *    ntype *UTSetBaseType( usertype udt)
 *
 * description:
 *   several functions for changing the values of the definition of the user
 *   defined type udf.
 *
 ******************************************************************************/

void
UTSetTypedef (usertype udt, ntype *type)
{
    DBUG_ENTER ("UTSetTypedef");
    DBUG_ASSERT ((udt < udt_no), "UTSetTypedef called with illegal udt!");

    ENTRY_DEF (udt_rep[udt]) = type;

    DBUG_VOID_RETURN;
}

void
UTSetBaseType (usertype udt, ntype *type)
{
    DBUG_ENTER ("UTSetBaseType");
    DBUG_ASSERT ((udt < udt_no), "UTSetBaseType called with illegal udt!");

    ENTRY_BASE (udt_rep[udt]) = type;
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void UTPrintRepository( FILE *outfile)
 *
 * description:
 *   prints the contents of udt_rep to outfile.
 *
 ******************************************************************************/

#define UTPRINT_FORMAT "| %-10.10s | %-10.10s | %-89.89s | %-89.89s |"

void
UTPrintRepository (FILE *outfile)
{
    int i;

    DBUG_ENTER ("UTPrintRepository");

    fprintf (outfile, "\n %4.4s " UTPRINT_FORMAT " %6s \n",
             "udt:", "module:", "name:", "defining type:", "base type:", "line:");
    for (i = 0; i < udt_no; i++) {
        fprintf (outfile, " %4d " UTPRINT_FORMAT " %6d \n", i, UTGetMod (i),
                 UTGetName (i), TYType2DebugString (UTGetTypedef (i)),
                 TYType2DebugString (UTGetBaseType (i)), UTGetLine (i));
    }

    DBUG_VOID_RETURN;
}
