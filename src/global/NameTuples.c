/*
 *
 * $Log$
 * Revision 3.2  2001/12/12 15:00:30  dkr
 * minor changes in PrintNT() done
 *
 * Revision 3.1  2000/11/20 17:59:26  sacbase
 * new release made
 *
 * Revision 1.3  2000/09/13 15:05:27  dkr
 * C_last? renamed into C_unknown?
 *
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

    fprintf (handle, (compiler_phase < PH_compile) ? "%s__%s_%s" : "(%s, (%s,(%s,)))",
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

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        z = C_unknownc;
    } else if (IsHidden (type)) {
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

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        z = C_unknownu;
    } else if (IsUnique (type)) {
        z = C_unq;
    } else {
        z = C_nuq;
    }

    DBUG_RETURN (z);
}
