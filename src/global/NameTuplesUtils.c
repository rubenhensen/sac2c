/*
 *
 * $Log$
 * Revision 1.1  2002/05/31 17:14:56  dkr
 * Initial revision
 *
 */

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "NameTuples.h"

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
