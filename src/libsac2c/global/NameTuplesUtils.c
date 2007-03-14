/*
 *
 * $Id$
 *
 */

#include <string.h>

#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "type_utils.h"
#include "globals.h"

#include "NameTuplesUtils.h"

/******************************************************************************
 *
 * function:
 *   shape_class_t NTUgetShapeClassFromTypes( types *type)
 *
 * description:
 *   Returns the Shape Class of an object (usually an array) from its type.
 *
 ******************************************************************************/

shape_class_t
NTUgetShapeClassFromTypes (types *type)
{
    shape_class_t z;

    DBUG_ENTER ("NTUgetShapeClassFromTypes");

    DBUG_ASSERT ((type != NULL), "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_ASSERT ((0), "illegal data class found!");
        z = C_unknowns;
    } else {
        int dim = TCgetShapeDim (type);

        if ((dim == SCALAR)
            && ((global.min_array_rep <= MAR_scl_aud) || TCisHidden (type))) {
            /*
             * C_scl can not be deactivated for hidden objects in order to prevent
             * inconsistency with the implementation of the hidden type.
             */
            z = C_scl;
        } else if (KNOWN_SHAPE (dim) && (global.min_array_rep <= MAR_scl_aks)) {
            z = C_aks;
        } else if (KNOWN_DIMENSION (dim) && (global.min_array_rep <= MAR_scl_akd)) {
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
 *   hidden_class_t NTUgetHiddenClassFromTypes( types *type)
 *
 * description:
 *   Returns the Hiddenness Class of an object (usually an array) from
 *   its type.
 *
 ******************************************************************************/

hidden_class_t
NTUgetHiddenClassFromTypes (types *type)
{
    hidden_class_t z;

    DBUG_ENTER ("NTUgetHiddenClassFromTypes");

    DBUG_ASSERT ((type != NULL), "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_ASSERT ((0), "illegal data class found!");
        z = C_unknownh;
    } else if (TCisHidden (type)) {
        z = C_hid;
    } else {
        z = C_nhd;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   unique_class_t NTUgetUniqueClassFromTypes( types *type)
 *
 * description:
 *   Returns the Uniqueness Class of an object (usually an array) from
 *   its type.
 *
 ******************************************************************************/

unique_class_t
NTUgetUniqueClassFromTypes (types *type)
{
    unique_class_t z;

    DBUG_ENTER ("NTUgetUniqueClassFromTypes");

    DBUG_ASSERT ((type != NULL), "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_ASSERT ((0), "illegal data class found!");
        z = C_unknownu;
    } else if (TCisUnique (type)) {
        z = C_unq;
    } else {
        z = C_nuq;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   char *NTUcreateNtTag( const char *name, types *type)
 *
 * description:
 *   Creates the tag of an object (usually an array) from its type.
 *
 ******************************************************************************/

char *
NTUcreateNtTag (const char *name, types *type)
{
    shape_class_t sc;
    hidden_class_t hc;
    unique_class_t uc;
    char *res;

    DBUG_ENTER ("NTUcreateNtTag");

    DBUG_ASSERT ((type != NULL), "No type found!");

    sc = NTUgetShapeClassFromTypes (type);
    hc = NTUgetHiddenClassFromTypes (type);
    uc = NTUgetUniqueClassFromTypes (type);

    res = (char *)MEMmalloc ((strlen (name) + strlen (global.nt_shape_string[sc])
                              + strlen (global.nt_hidden_string[hc])
                              + strlen (global.nt_unique_string[uc]) + 16)
                             * sizeof (char));

    sprintf (res, "(%s, (%s, (%s, (%s,))))", name, global.nt_shape_string[sc],
             global.nt_hidden_string[hc], global.nt_unique_string[uc]);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *NTUaddNtTag( node *id)
 *
 * description:
 *   Creates the tag of a N_id node.
 *
 ******************************************************************************/

node *
NTUaddNtTag (node *id)
{
    node *avis;

    DBUG_ENTER ("NTUaddNtTag");

    avis = ID_AVIS (id);

    DBUG_ASSERT ((avis != NULL), "no avis found!");

    switch (NODE_TYPE (AVIS_DECL (avis))) {
    case N_vardec:
        ID_NT_TAG (id)
          = NTUcreateNtTag (AVIS_NAME (avis), VARDEC_TYPE (AVIS_DECL (avis)));
        break;
    case N_arg:
        ID_NT_TAG (id) = NTUcreateNtTag (AVIS_NAME (avis), ARG_TYPE (AVIS_DECL (avis)));
        break;
    default:
        DBUG_ASSERT ((FALSE), "illegal decl in avis node");
    }

    DBUG_RETURN (id);
}

/******************************************************************************
 *
 * Name Tuples Utils
 *
 * ---- NTYPE VERSION ----
 *
 * Prefix: NTU
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   shape_class_t NTUgetShapeClassFromNType( ntype *ntype)
 *
 * description:
 *   Returns the Shape Class of an object (usually an array) from its ntype.
 *
 ******************************************************************************/

shape_class_t
NTUgetShapeClassFromNType (ntype *ntype)
{
    shape_class_t z;

    DBUG_ENTER ("NTUgetShapeClassFromNType");

    DBUG_ASSERT ((ntype != NULL), "No type found!");

    switch (TYgetConstr (ntype)) {
    case TC_akv:
    case TC_aks:
        if (TYgetDim (ntype) == 0) {
            z = C_scl;
        } else {
            z = C_aks;
        }
        break;

    case TC_akd:
        if (TYgetDim (ntype) == 0) {
            z = C_scl;
        } else {
            z = C_akd;
        }
        break;

    case TC_aud:
    case TC_audgz:
        z = C_aud;
        break;

    default:
        DBUG_ASSERT ((0), "Illegal Shape Class");
        z = C_unknowns;
        break;
    }

    /*
     * Adapt to minimal array representation
     *
     * C_scl can not be deactivated for hidden objects in order to prevent
     * inconsistency with the implementation of the hidden type.
     */
    if (!TUisHidden (ntype)) {

        switch (global.min_array_rep) {
        case MAR_aud:
            z = C_aud;
            break;

        case MAR_scl_aud:
            if (z != C_scl) {
                z = C_aud;
            }
            break;

        case MAR_scl_akd:
            if (z == C_aks) {
                z = C_aks;
            }

        case MAR_scl_aks:
        default:
            break;
        }
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   hidden_class_t NTUgetHiddenClassFromNType( ntype *ntype)
 *
 * description:
 *   Returns the Hiddenness Class of an object (usually an array) from
 *   its type.
 *
 ******************************************************************************/

hidden_class_t
NTUgetHiddenClassFromNType (ntype *ntype)
{
    hidden_class_t z;

    DBUG_ENTER ("NTUgetHiddenClassFromNType");

    DBUG_ASSERT ((ntype != NULL), "No type found!");

    if (TUisHidden (ntype)) {
        z = C_hid;
    } else {
        z = C_nhd;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   unique_class_t NTUgetUniqueClassFromNType( ntype *ntype)
 *
 * description:
 *   Returns the Uniqueness Class of an object (usually an array) from
 *   its type.
 *
 ******************************************************************************/

unique_class_t
NTUgetUniqueClassFromNType (ntype *ntype)
{
    unique_class_t z;

    DBUG_ENTER ("NTUgetUniqueClassFromNType");

    DBUG_ASSERT ((ntype != NULL), "No type found!");

    if (TUisUniqueUserType (ntype)) {
        z = C_unq;
    } else {
        z = C_nuq;
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 * @brief Creates the tag of an object (usually an array) from its type.
 *
 * @param name name of the id/object to create a tag for
 * @param ntype type of the id/object
 *
 * @return tag
 ******************************************************************************/
char *
NTUcreateNtTagFromNType (const char *name, ntype *ntype)
{
    shape_class_t sc;
    hidden_class_t hc;
    unique_class_t uc;
    char *res;

    DBUG_ENTER ("NTUcreateNtTagFromNType");

    DBUG_ASSERT ((ntype != NULL), "No type found!");

    sc = NTUgetShapeClassFromNType (ntype);
    hc = NTUgetHiddenClassFromNType (ntype);
    uc = NTUgetUniqueClassFromNType (ntype);

    res = (char *)MEMmalloc ((strlen (name) + strlen (global.nt_shape_string[sc])
                              + strlen (global.nt_hidden_string[hc])
                              + strlen (global.nt_unique_string[uc]) + 16)
                             * sizeof (char));

    sprintf (res, "(%s, (%s, (%s, (%s,))))", name, global.nt_shape_string[sc],
             global.nt_hidden_string[hc], global.nt_unique_string[uc]);

    DBUG_RETURN (res);
}