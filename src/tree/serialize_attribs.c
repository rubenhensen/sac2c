/*
 *
 * $Log$
 * Revision 1.9  2004/10/19 14:04:49  sah
 * implemented SerializeIdsAttrib
 *
 * Revision 1.8  2004/10/17 17:03:20  sah
 * noew the generated code no more contains
 * any sac2c specific types o [3~´´r enums (hopefully)
 *
 * Revision 1.7  2004/09/30 20:15:10  sah
 * fixed signature of SerializeRCCounterAttrib
 *
 * Revision 1.6  2004/09/30 19:52:43  sah
 * added RCCounter support
 *
 * Revision 1.5  2004/09/27 13:18:12  sah
 * implemented new serialization scheme
 *
 * Revision 1.4  2004/09/24 20:21:53  sah
 * intermediate version
 *
 * Revision 1.3  2004/09/23 21:12:25  sah
 * ongoing implementation
 *
 * Revision 1.2  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
 *
 *
 *
 */

/**
 * @file serialize_attribs.c
 *
 * Functions needed by serialize traversal to serialize attributes
 *
 */

/**
 * @defgroup serialize Serialize Tree Functions.
 *
 * Functions needed by serialize traversal.
 *
 * @{
 */

#define NEW_INFO

#include "serialize_attribs.h"
#include "serialize_info.h"
#include "serialize_stack.h"
#include "serialize.h"
#include "constants.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

/** <!--******************************************************************-->
 *
 * @fn SerializeStringAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeStringAttrib (info *info, char *attr, node *parent)
{
    DBUG_ENTER ("SerializeStringAttrib");

    if (attr == NULL) {
        DBUG_PRINT ("SET", ("Processing String (null)"));

        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        DBUG_PRINT ("SET", ("Processing String `%s'", attr));

        fprintf (INFO_SER_FILE (info), "\"%s\"", attr);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSharedStringAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSharedStringAttrib (info *info, char *attr, node *parent)
{
    DBUG_ENTER ("SerializeSharedStringAttrib");

    if (attr == NULL) {
        DBUG_PRINT ("SET", ("Processing String (null)"));

        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        DBUG_PRINT ("SET", ("Processing String `%s'", attr));

        fprintf (INFO_SER_FILE (info), "\"%s\"", attr);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerAttrib (info *info, int attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLongAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLongAttrib (info *info, long attr, node *parent)
{
    DBUG_ENTER ("SerializeLongAttrib");

    fprintf (INFO_SER_FILE (info), "%ld", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeBoolAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeBoolAttrib (info *info, bool attr, node *parent)
{
    DBUG_ENTER ("SerializeBoolAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeFloatAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeFloatAttrib (info *info, float attr, node *parent)
{
    DBUG_ENTER ("SerializeFloatAttrib");

    fprintf (INFO_SER_FILE (info), "%f", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDoubleAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDoubleAttrib (info *info, double attr, node *parent)
{
    DBUG_ENTER ("SerializeDoubleAttrib");

    fprintf (INFO_SER_FILE (info), "%f", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeCharAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeCharAttrib (info *info, char attr, node *parent)
{
    DBUG_ENTER ("SerializeCharAttrib");

    fprintf (INFO_SER_FILE (info), "'%c'", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeOldTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeOldTypeAttrib (info *info, types *attr, node *parent)
{
    DBUG_ENTER ("SerializeOldTypeAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        fprintf (INFO_SER_FILE (info), "MakeTypes1( %d)", T_unknown);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodeAttrib (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeNodeAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        /* we have to eat the additional , created by SET traversal */
        fprintf (INFO_SER_FILE (info), "DROP( x");
        Trav (attr, info);
        fprintf (INFO_SER_FILE (info), ")");
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLinkAttrib (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeLinkAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeApLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeApLinkAttrib (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeApLinkAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerArrayAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerArrayAttrib (info *info, int *attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerArrayAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNumsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNumsAttrib (info *info, nums *attr, node *parent)
{
    DBUG_ENTER ("SerializeNumsAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeFileTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeFileTypeAttrib (info *info, file_type attr, node *parent)
{
    DBUG_ENTER ("SerializeFileTypeAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLUTAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLUTAttrib (info *info, LUT_t attr, node *parent)
{
    DBUG_ENTER ("SerializeLUTAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeWithOpTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeWithOpTypeAttrib (info *info, WithOpType attr, node *parent)
{
    DBUG_ENTER ("SerializeWithOpTypeAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializePrfAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializePrfAttrib (info *info, prf attr, node *parent)
{
    DBUG_ENTER ("SerializePrfAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param pos    position within the current array
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeMaskAttrib (info *info, int pos, long *attr, node *parent)
{
    DBUG_ENTER ("SerializeMaskAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDepsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDepsAttrib (info *info, deps *attr, node *parent)
{
    DBUG_ENTER ("SerializeDepsAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIdsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIdsAttrib (info *info, ids *attr, node *parent)
{
    DBUG_ENTER ("SerializeIdsAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL ");
    } else {
        fprintf (INFO_SER_FILE (info), "CreateIds( \"%s\", ", IDS_NAME (attr));

        if (IDS_MOD (attr) != NULL) {
            fprintf (INFO_SER_FILE (info), "\"%s\", ", IDS_MOD (attr));
        } else {
            fprintf (INFO_SER_FILE (info), "NULL, ");
        }

        fprintf (INFO_SER_FILE (info), "%d, %d, ", IDS_STATUS (attr), IDS_ATTRIB (attr));

        fprintf (INFO_SER_FILE (info), "%d, %d, ", IDS_REFCNT (attr),
                 IDS_NAIVE_REFCNT (attr));

        SerializeIdsAttrib (info, IDS_NEXT (attr), parent);

        fprintf (INFO_SER_FILE (info), ")");
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeStatusTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeStatusTypeAttrib (info *info, statustype attr, node *parent)
{
    DBUG_ENTER ("SerializeStatusTypeAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodeListAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodeListAttrib (info *info, nodelist *attr, node *parent)
{
    DBUG_ENTER ("SerializeNodeListAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSharedNodeListAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSharedNodeListAttrib (info *info, nodelist *attr, node *parent)
{
    DBUG_ENTER ("SerializeSharedNodeListAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializePragmaLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializePragmaLinkAttrib (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializePragmaLinkAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMMaskAttrib (info *info, DFMmask_t attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMMaskAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMMaskBaseAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMMaskBaseAttrib (info *info, DFMmask_base_t attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMMaskBaseAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMFoldMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMFoldMaskAttrib (info *info, DFMfoldmask_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMFoldMaskAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNewTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNewTypeAttrib (info *info, ntype *attr, node *parent)
{
    DBUG_ENTER ("SerializeNewTypeAttrib");

    DBUG_PRINT ("SET", ("Starting traversal for ntype attribute"));

    TYSerializeType (INFO_SER_FILE (info), attr);

    DBUG_PRINT ("SET", ("Finished traversal for ntype attribute"));

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeArgTabAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeArgTabAttrib (info *info, argtab_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeArgTabAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIndexPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIndexPointerAttrib (info *info, index_info *attr, node *parent)
{
    DBUG_ENTER ("SerializeIndexPointerAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeShapeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeShapeAttrib (info *info, shape *attr, node *parent)
{
    DBUG_ENTER ("SerializeShapeAttrib");

    SHSerializeShape (INFO_SER_FILE (info), attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSimpleTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSimpleTypeAttrib (info *info, simpletype attr, node *parent)
{
    DBUG_ENTER ("SerializeSimpleTypeAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeConstVecPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeConstVecPointerAttrib (info *info, void *attr, node *parent)
{
    DBUG_ENTER ("SerializeConstVecPointerAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeUseFlagAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeUseFlagAttrib (info *info, useflag attr, node *parent)
{
    DBUG_ENTER ("SerializeUseFlagAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeAccessInfoAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeAccessInfoAttrib (info *info, access_info_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeAccessInfoAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeShpSegAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeShpSegAttrib (info *info, shpseg *attr, node *parent)
{
    DBUG_ENTER ("SerializeShpSegAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerPointerAttrib (info *info, int *attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerPointerAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerPointerArrayAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param pos    position within the current array
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerPointerArrayAttrib (info *info, int pos, int *attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerPointerArrayAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSchedulingAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSchedulingAttrib (info *info, SCHsched_t attr, node *parent)
{
    DBUG_ENTER ("SerializeSchedulingAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeTaskSelAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeTaskSelAttrib (info *info, SCHtasksel_t attr, node *parent)
{
    DBUG_ENTER ("SerializeTaskSelAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodePointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodePointerAttrib (info *info, node **attr, node *parent)
{
    DBUG_ENTER ("SerializeNodePointerAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSSAPhiAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSSAPhiAttrib (info *info, ssaphit_t attr, node *parent)
{
    DBUG_ENTER ("SerializeSSAPhiAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeBitFieldAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeBitFieldAttrib (info *info, int attr, node *parent)
{
    DBUG_ENTER ("SerializeBitFieldAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeConstantAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeConstantAttrib (info *info, constant *attr, node *parent)
{
    DBUG_ENTER ("SerializeConstantAttrib");

    COSerializeConstant (INFO_SER_FILE (info), attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeMTExecModeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeMTExecModeAttrib (info *info, mtexecmode_t attr, node *parent)
{
    DBUG_ENTER ("SerializeMTExecModeAttrib");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeRCCounterAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeRCCounterAttrib (info *info, rc_counter *attr, node *parent)
{
    DBUG_ENTER ("SerializeMTExecModeAttrib");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/**
 * @}
 */
