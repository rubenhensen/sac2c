/*
 *
 * $Log$
 * Revision 1.8  2003/03/13 15:49:09  dkr
 * handling of -minarrayrep option added
 *
 * Revision 1.7  2002/07/31 15:35:02  dkr
 * new hidden tag added
 *
 * Revision 1.6  2002/07/15 18:40:39  dkr
 * Get...ClassFromTypes(): DBUG_ASSERT added
 *
 * Revision 1.5  2002/07/12 21:37:58  dkr
 * fixed a bug in GetDataClassFromTypes()
 *
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
 *   shape_class_t GetShapeClassFromTypes( types *type)
 *
 * description:
 *   Returns the Shape Class of an object (usually an array) from its type.
 *
 ******************************************************************************/

shape_class_t
GetShapeClassFromTypes (types *type)
{
    shape_class_t z;

    DBUG_ENTER ("GetShapeClassFromTypes");

    DBUG_ASSERT ((type != NULL), "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_ASSERT ((0), "illegal data class found!");
        z = C_unknowns;
    } else {
        int dim = GetShapeDim (type);

        if ((dim == SCALAR) && (min_array_rep <= MIN_ARRAY_REP_SCL_AUD)) {
            z = C_scl;
        } else if (KNOWN_SHAPE (dim) && (min_array_rep <= MIN_ARRAY_REP_SCL_AKS)) {
            z = C_aks;
        } else if (KNOWN_DIMENSION (dim) && (min_array_rep <= MIN_ARRAY_REP_SCL_AKD)) {
            z = C_akd;
        } else {
            z = C_aud;
        }
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   hidden_class_t GetHiddenClassFromTypes( types *type)
 *
 * description:
 *   Returns the Hiddenness Class of an object (usually an array) from
 *   its type.
 *
 ******************************************************************************/

hidden_class_t
GetHiddenClassFromTypes (types *type)
{
    hidden_class_t z;

    DBUG_ENTER ("GetHiddenClassFromTypes");

    DBUG_ASSERT ((type != NULL), "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_ASSERT ((0), "illegal data class found!");
        z = C_unknownh;
    } else if (IsHidden (type)) {
        z = C_unq;
    } else {
        z = C_nuq;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   unique_class_t GetUniqueClassFromTypes( types *type)
 *
 * description:
 *   Returns the Uniqueness Class of an object (usually an array) from
 *   its type.
 *
 ******************************************************************************/

unique_class_t
GetUniqueClassFromTypes (types *type)
{
    unique_class_t z;

    DBUG_ENTER ("GetUniqueClassFromTypes");

    DBUG_ASSERT ((type != NULL), "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_ASSERT ((0), "illegal data class found!");
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
    shape_class_t sc;
    hidden_class_t hc;
    unique_class_t uc;
    char *res;

    DBUG_ENTER ("CreateNtTag");

    DBUG_ASSERT ((type != NULL), "No type found!");

    sc = GetShapeClassFromTypes (type);
    hc = GetHiddenClassFromTypes (type);
    uc = GetUniqueClassFromTypes (type);

    res = (char *)Malloc ((strlen (name) + strlen (nt_shape_string[sc])
                           + strlen (nt_hidden_string[hc]) + strlen (nt_unique_string[uc])
                           + 16)
                          * sizeof (char));

    sprintf (res, "(%s, (%s, (%s, (%s,))))", name, nt_shape_string[sc],
             nt_hidden_string[hc], nt_unique_string[uc]);

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
