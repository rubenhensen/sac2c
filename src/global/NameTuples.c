/*
 *
 * $Log$
 * Revision 1.2  2000/08/17 11:11:46  dkr
 * signature of PrintNT changed
 *
 * Revision 1.1  2000/08/17 10:16:08  dkr
 * Initial revision
 *
 */

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "NameTuples.h"

/*
 * These character arrays are the macro-name-parts used to select
 * array class and array uniqueness properties.
 */

char *nt_class_string[] = {
#define ATTRIB 1
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
};

char *nt_unq_string[] = {
#define ATTRIB 2
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
};

/******************************************************************************
 *
 * Function:
 *   void PrintNT( FILE *handle, char *name, types *type)
 *
 * Description:
 *   Prints name tuples.
 *
 ******************************************************************************/

void
PrintNT (FILE *handle, char *name, types *type)
{
    DBUG_ENTER ("PrintNT");

    DBUG_ASSERT ((type != NULL), "No type information found!");

    fprintf (handle, (compiler_phase < PH_compile) ? "%s__%s__%s" : "(%s,(%s,(%s,)))",
             name, nt_class_string[GetClassFromTypes (type)],
             nt_unq_string[GetUnqFromTypes (type)]);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   data_class_t GetClassFromTypes( types *type)
 *
 * description:
 *   Returns the Data Class of an object (usually an array) from its type.
 *
 ******************************************************************************/

data_class_t
GetClassFromTypes (types *type)
{
    data_class_t z;

    DBUG_ENTER ("GetClassFromTypes");

    DBUG_ASSERT ((type != NULL), "No type found!");

    /*
     * if a typedef exists for the given type, we have to use the type
     * definition for class inference!
     */
    if (TYPES_TDEF (type) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (TYPES_TDEF (type)) == N_typedef),
                     "TYPES_TDEF is not a N_typedef node!");
        type = TYPEDEF_TYPE (TYPES_TDEF (type));
    }

    if (IsHidden (type)) {
        z = C_hid;
    } else if (TYPES_DIM (type) == SCALAR) {
        z = C_scl;
    } else if (KNOWN_SHAPE (TYPES_DIM (type))) {
        z = C_aks;
    } else if (KNOWN_DIMENSION (TYPES_DIM (type))) {
        z = C_akd;
    } else {
        z = C_aud;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   unq_class_t GetUnqFromTypes( types *type)
 *
 * description:
 *   Returns the Uniqueness Class of an object (usually an array) from
 *   its type.
 *
 ******************************************************************************/

unq_class_t
GetUnqFromTypes (types *type)
{
    unq_class_t z;

    DBUG_ENTER ("GetUniFromTypes");

    DBUG_ASSERT ((type != NULL), "No type found!");

    if (IsUnique (type)) {
        z = C_unq;
    } else {
        z = C_nuq;
    }

    DBUG_RETURN (z);
}
