#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "type_utils.h"
#include "globals.h"

#include "NameTuplesUtils.h"

/** <!--********************************************************************-->
 *
 * @fn mutcStorageClass simpletype2mutcStorageClass( simpletype st)
 *
 *   @brief  Convert a simpletype into mutcStorageClass
 *
 *   @param st    simpletype to convert
 *   @return      mutcStorageClass that can hold simpletype
 *
 *****************************************************************************/

static mutcStorageClass
simpletype2mutcStorageClass (simpletype st)
{
    mutcStorageClass ret = (mutcStorageClass)0;

    DBUG_ENTER ();

#define TYP_IF(it_name, it_db_str, it_pr_str, it_funr_str, it_cv2scal, it_cv2cv,         \
               it_cv2str, it_size, it_zipcv, it_basecv, it_mutc_sc, unused2,             \
               it_ntcbasetype)                                                           \
    case it_name:                                                                        \
        ret = it_mutc_sc;                                                                \
        break;

    switch (st) {
#include "type_info.mac"
    }

    DBUG_RETURN (ret);
}

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

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_UNREACHABLE ("illegal data class found!");
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

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_UNREACHABLE ("illegal data class found!");
        z = C_unknownh;
    } else if (TCisNested (type)) {
        z = C_hns;
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

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_UNREACHABLE ("illegal data class found!");
        z = C_unknownu;
    } else if (TCisUnique (type)) {
        z = C_unq;
    } else if (TYPES_UNIQUE (type)) {
        z = C_unq;
    } else {
        z = C_nuq;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   mutc_storage_class_class_t NTUMutcgetStorageClassFromTypes( types *type)
 *
 * description:
 *
 ******************************************************************************/

mutc_storage_class_class_t
NTUgetMutcStorageClassFromTypes (types *type)
{
    mutc_storage_class_class_t z;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_UNREACHABLE ("illegal data class found!");
        z = C_unknownc;
    } else {
        switch (simpletype2mutcStorageClass (TYPES_BASETYPE (type))) {
        case MUTC_SC_INT:
            z = C_int;
            break;
        case MUTC_SC_FLOAT:
            z = C_float;
            break;
        default:
            z = C_unknownc;
        }
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   mutc_scope_class_t NTUMutcgetScopeFromTypes( types *type)
 *
 * description:
 *
 ******************************************************************************/

mutc_scope_class_t
NTUgetMutcScopeFromTypes (types *type)
{
    mutc_scope_class_t z;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_UNREACHABLE ("illegal scope found!");
        z = C_unknowno;
    } else {
        switch (TYPES_MUTC_SCOPE (type)) {
        case MUTC_SHARED:
            z = C_shared;
            break;
        default:
            z = C_global;
        }
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   mutc_usage_class_t NTUgetMutcUsageFromTypes( types *type)
 *
 * description:
 *
 ******************************************************************************/

mutc_usage_class_t
NTUgetMutcUsageFromTypes (types *type)
{
    mutc_usage_class_t z;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_UNREACHABLE ("illegal usage found!");
        z = C_unknowna;
    } else {
        switch (TYPES_MUTC_USAGE (type)) {
        case MUTC_US_THREADPARAMIO:
            z = C_threadparamio;
            break;
        case MUTC_US_THREADPARAM:
            z = C_threadparam;
            break;
        case MUTC_US_FUNPARAMIO:
            z = C_funparamio;
            break;
        case MUTC_US_FUNPARAM:
            z = C_funparam;
            break;
        case MUTC_US_FUNARG:
            z = C_funarg;
            break;
        default:
            z = C_none;
            break;
        }
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   bitarray_class_t NTUgetBitarrayFromTypes( types *type)
 *
 * description:
 *
 ******************************************************************************/

bitarray_class_t
NTUgetBitarrayFromTypes (types *type)
{
#ifdef DBUG_OFF
    (void)type;
#endif
    bitarray_class_t z;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    z = C_sparse;

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   distributed_class_t NTUgetDistributedFromTypes( types *type)
 *
 * description:
 *
 ******************************************************************************/

distributed_class_t
NTUgetDistributedFromTypes (types *type)
{
    distributed_class_t d;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_UNREACHABLE ("illegal distributed attribute found!");
        d = C_unknownd;
    } else {
        if (TYPES_DISTRIBUTED (type) == distmem_dis_dis) {
            d = C_distr;
        } else if (TYPES_DISTRIBUTED (type) == distmem_dis_dsm) {
            d = C_distmem;
        } else {
            d = C_notdistr;
        }
    }

    DBUG_RETURN (d);
}

/******************************************************************************
 *
 * function:
 *   distributed_class_t NTUgetDistributedFromNType( ntype *type)
 *
 * description:
 *
 ******************************************************************************/

distributed_class_t
NTUgetDistributedFromNType (ntype *type)
{
    distributed_class_t d;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if (TYgetDistributed (type) == distmem_dis_dis) {
        d = C_distr;
    } else if (TYgetDistributed (type) == distmem_dis_dsm) {
        d = C_distmem;
    } else {
        d = C_notdistr;
    }

    DBUG_RETURN (d);
}

/******************************************************************************
 *
 * function:
 *   cbasetype_class_t NTUgetCBasetypeFromTypes( types *type)
 *
 * description:
 *
 * Note:    This has to be kept in sync with GetBasetypeStr
 *          (compile.c) !!!
 *
 ******************************************************************************/

cbasetype_class_t
NTUgetCBasetypeFromTypes (types *type)
{
    cbasetype_class_t b;
    simpletype basetype;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    if ((TYPES_BASETYPE (type) == T_user) && (TYPES_TDEF (type) == NULL)) {
        /*
         * the TC has probably not been called yet :-(
         */
        DBUG_UNREACHABLE ("illegal C basetype attribute found!");
        b = C_unknownt;
    } else {
        basetype = TCgetBasetype (type);
        b = global.type_cbasetype[basetype];

        /*
         * If the enforce_float flag is set,
         * we change all doubles to floats.
         */
        if (b == C_btdouble && global.enforce_float) {
            b = C_btfloat;
        }
    }

    DBUG_RETURN (b);
}

/******************************************************************************
 *
 * function:
 *   cbasetype_class_t NTUgetCBasetypeFromNType( ntype *type)
 *
 * description:
 *
 * Note:    This has to be kept in sync with GetBasetypeStr
 *          (compile.c) !!!
 *
 ******************************************************************************/

cbasetype_class_t
NTUgetCBasetypeFromNType (ntype *type)
{
    cbasetype_class_t b;
    simpletype basetype;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    basetype = TUgetSimpleImplementationType (type);
    b = global.type_cbasetype[basetype];

    /*
     * If the enforce_float flag is set,
     * we change all doubles to floats.
     */
    if (b == C_btdouble && global.enforce_float) {
        b = C_btfloat;
    }

    DBUG_RETURN (b);
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
    mutc_storage_class_class_t storage;
    mutc_scope_class_t scope;
    mutc_usage_class_t usage;
    bitarray_class_t bitarray;
    distributed_class_t distr;
    cbasetype_class_t cbasetype;
    char *res;

    DBUG_ENTER ();

    DBUG_ASSERT (type != NULL, "No type found!");

    sc = NTUgetShapeClassFromTypes (type);
    hc = NTUgetHiddenClassFromTypes (type);
    uc = NTUgetUniqueClassFromTypes (type);

    storage = NTUgetMutcStorageClassFromTypes (type);
    scope = NTUgetMutcScopeFromTypes (type);
    usage = NTUgetMutcUsageFromTypes (type);

    bitarray = NTUgetBitarrayFromTypes (type);

    distr = NTUgetDistributedFromTypes (type);

    cbasetype = NTUgetCBasetypeFromTypes (type);

    /*
     * Allocate enough space for the textual representation of the type tuple.
     * The total length is the length of all textual representations combined
     * plus some space for administration:
     *
     * - 10 is the number of elements (including the name)
     * - 4 is the number of chars each element takes aside of its name: "( ,)"
     * - 1 is the terminating \0 byte.
     */
    res = (char *)MEMmalloc (
      (STRlen (name) + STRlen (global.nt_shape_string[sc])
       + STRlen (global.nt_hidden_string[hc]) + STRlen (global.nt_unique_string[uc])
       + STRlen (global.nt_mutc_storage_class_string[storage])
       + STRlen (global.nt_mutc_scope_string[scope])
       + STRlen (global.nt_mutc_usage_string[usage])
       + STRlen (global.nt_bitarray_string[bitarray])
       + STRlen (global.nt_distributed_string[distr])
       + STRlen (global.nt_cbasetype_string[cbasetype]) + (10 * 4 + 1))
      * sizeof (char));

    sprintf (res, "(%s, (%s, (%s, (%s, (%s, (%s, (%s, (%s, (%s, (%s, ))))))))))", name,
             global.nt_shape_string[sc], global.nt_hidden_string[hc],
             global.nt_unique_string[uc], global.nt_mutc_storage_class_string[storage],
             global.nt_mutc_scope_string[scope], global.nt_mutc_usage_string[usage],
             global.nt_bitarray_string[bitarray], global.nt_distributed_string[distr],
             global.nt_cbasetype_string[cbasetype]);

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

    DBUG_ENTER ();

    avis = ID_AVIS (id);

    DBUG_ASSERT (avis != NULL, "no avis found!");

    switch (NODE_TYPE (AVIS_DECL (avis))) {
    case N_vardec:
        ID_NT_TAG (id)
          = NTUcreateNtTag (AVIS_NAME (avis), VARDEC_TYPE (AVIS_DECL (avis)));
        break;
    case N_arg:
        ID_NT_TAG (id) = NTUcreateNtTag (AVIS_NAME (avis), ARG_TYPE (AVIS_DECL (avis)));
        break;
    default:
        DBUG_UNREACHABLE ("illegal decl in avis node");
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

    DBUG_ENTER ();

    DBUG_ASSERT (ntype != NULL, "No type found!");

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
        DBUG_UNREACHABLE ("Illegal Shape Class");
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

    DBUG_ENTER ();

    DBUG_ASSERT (ntype != NULL, "No type found!");

    if (TUisNested (ntype)) {
        z = C_hns;
    } else if (TUisHidden (ntype)) {
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

    DBUG_ENTER ();

    DBUG_ASSERT (ntype != NULL, "No type found!");

    if (TUisUniqueUserType (ntype)) {
        z = C_unq;
    } else {
        z = C_nuq;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   mutc_storage_class_class_t NTUgetMutcStorageClassFromNType( ntype *ntype)
 *
 * description:
 *
 ******************************************************************************/

mutc_storage_class_class_t
NTUgetMutcStorageClassFromNType (ntype *ntype)
{
    mutc_storage_class_class_t z;

    DBUG_ENTER ();

    DBUG_ASSERT (ntype != NULL, "No type found!");

    switch (simpletype2mutcStorageClass (TUgetBaseSimpleType (ntype))) {
    case MUTC_SC_INT:
        z = C_int;
        break;
    case MUTC_SC_FLOAT:
        z = C_float;
        break;
    default:
        z = C_unknownc;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   mutc_scope_class_t NTUgetMutcScopeFromNType( ntype *ntype)
 *
 * description:
 *
 ******************************************************************************/

mutc_scope_class_t
NTUgetMutcScopeFromNType (ntype *ntype)
{
    mutc_scope_class_t z;

    DBUG_ENTER ();

    DBUG_ASSERT (ntype != NULL, "No type found!");

    switch (TYgetMutcScope (ntype)) {
    case MUTC_GLOBAL:
        z = C_global;
        break;
    case MUTC_SHARED:
        z = C_shared;
        break;
    case MUTC_LOCAL:
        z = C_shared;
        break;
    default:
        z = C_unknowno;
        break;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   mutc_usage_class_t NTUgetMutcUsageFromNType( ntype *ntype)
 *
 * description:
 *
 ******************************************************************************/

mutc_usage_class_t
NTUgetMutcUsageFromNType (ntype *ntype)
{
    mutc_usage_class_t z;

    DBUG_ENTER ();

    DBUG_ASSERT (ntype != NULL, "No type found!");

    switch (TYgetMutcUsage (ntype)) {
    case MUTC_US_THREADPARAMIO:
        z = C_threadparamio;
        break;
    case MUTC_US_THREADPARAM:
        z = C_threadparam;
        break;
    case MUTC_US_FUNPARAMIO:
        z = C_funparamio;
        break;
    case MUTC_US_FUNPARAM:
        z = C_funparam;
        break;
    case MUTC_US_FUNARG:
        z = C_funarg;
        break;
    default:
        z = C_none;
        break;
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   bitarray_class_t NTUgetBitarrayFromNType( ntype *ntype)
 *
 * description:
 *
 ******************************************************************************/

bitarray_class_t
NTUgetBitarrayFromNType (ntype *ntype)
{
#ifdef DBUG_OFF
    (void)ntype;
#endif
    bitarray_class_t z;

    DBUG_ENTER ();

    DBUG_ASSERT (ntype != NULL, "No type found!");

    /* Place-holder code: always assume regular. */
    z = C_sparse;

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
    mutc_storage_class_class_t storage;
    mutc_scope_class_t scope;
    mutc_usage_class_t usage;
    bitarray_class_t bitarray;
    distributed_class_t distr;
    cbasetype_class_t cbasetype;

    DBUG_ENTER ();

    DBUG_ASSERT (ntype != NULL, "No type found!");

    sc = NTUgetShapeClassFromNType (ntype);
    hc = NTUgetHiddenClassFromNType (ntype);
    uc = NTUgetUniqueClassFromNType (ntype);

    storage = NTUgetMutcStorageClassFromNType (ntype);
    scope = NTUgetMutcScopeFromNType (ntype);
    usage = NTUgetMutcUsageFromNType (ntype);

    bitarray = NTUgetBitarrayFromNType (ntype);

    distr = NTUgetDistributedFromNType (ntype);

    cbasetype = NTUgetCBasetypeFromNType (ntype);

    res = (char *)MEMmalloc (
      (STRlen (name) + STRlen (global.nt_shape_string[sc])
       + STRlen (global.nt_hidden_string[hc]) + STRlen (global.nt_unique_string[uc])
       + STRlen (global.nt_mutc_storage_class_string[storage])
       + STRlen (global.nt_mutc_scope_string[scope])
       + STRlen (global.nt_mutc_usage_string[usage])
       + STRlen (global.nt_bitarray_string[bitarray])
       + STRlen (global.nt_distributed_string[distr])
       + STRlen (global.nt_cbasetype_string[cbasetype]) + (10 * 4) + 1)
      * sizeof (char));

    sprintf (res, "(%s, (%s, (%s, (%s, (%s, (%s, (%s, (%s, (%s, (%s, ))))))))))", name,
             global.nt_shape_string[sc], global.nt_hidden_string[hc],
             global.nt_unique_string[uc], global.nt_mutc_storage_class_string[storage],
             global.nt_mutc_scope_string[scope], global.nt_mutc_usage_string[usage],
             global.nt_bitarray_string[bitarray], global.nt_distributed_string[distr],
             global.nt_cbasetype_string[cbasetype]);

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
