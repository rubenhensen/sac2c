/*
 *
 * $Log$
 * Revision 1.4  2002/07/11 13:58:57  dkr
 * AddNtTag() added
 *
 * Revision 1.3  2002/06/04 08:37:17  dkr
 * C_unknownc renamed into C_unknownd
 *
 * Revision 1.2  2002/06/02 21:42:42  dkr
 * symbols renamed
 *
 * Revision 1.1  2002/05/31 17:14:56  dkr
 * Initial revision
 *
 */

#include "dbug.h"
#include "internal_lib.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "NameTuplesUtils.h"

/******************************************************************************
 *
 * function:
 *   data_class_t GetDataClassFromTypes( types *type)
 *
 * description:
 *   Returns the Data Class of an object (usually an array) from its type.
 *
 ******************************************************************************/

data_class_t
GetDataClassFromTypes (types *type)
{
    data_class_t z;

    DBUG_ENTER ("GetDataClassFromTypes");

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
        z = C_unknownd;
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
 *   unq_class_t GetUnqClassFromTypes( types *type)
 *
 * description:
 *   Returns the Uniqueness Class of an object (usually an array) from
 *   its type.
 *
 ******************************************************************************/

unq_class_t
GetUnqClassFromTypes (types *type)
{
    unq_class_t z;

    DBUG_ENTER ("GetUnqClassFromTypes");

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

/******************************************************************************
 *
 * function:
 *   char *CreateNtTag( char *name, types *type)
 *
 * description:
 *   Creates the tag of an object (usually an array) from its type.
 *
 ******************************************************************************/

char *
CreateNtTag (char *name, types *type)
{
    char *res;

    DBUG_ENTER ("CreateNtTag");

    DBUG_ASSERT ((type != NULL), "No type found!");

    res = (char *)Malloc ((strlen (name) + 20) * sizeof (char));

    sprintf (res, "(%s, (%s, (%s,)))", name, nt_data_string[GetDataClassFromTypes (type)],
             nt_unq_string[GetUnqClassFromTypes (type)]);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *AddNtTag( node *id)
 *
 * description:
 *   Creates the tag of a N_id node.
 *
 ******************************************************************************/

node *
AddNtTag (node *id)
{
    DBUG_ENTER ("AddNtTag");

    DBUG_ASSERT ((ID_VARDEC (id) != NULL), "no vardec found!");

    ID_NT_TAG (id) = CreateNtTag (ID_NAME (id), ID_TYPE (id));

    DBUG_RETURN (id);
}
