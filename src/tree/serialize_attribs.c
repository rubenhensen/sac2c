/*
 *
 * $Log: *
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

#include "serialize_attribs.h"

/** <!--******************************************************************-->
 *
 * @fn SerializeStringAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeStringAttrib (char *vname, info *info, char *attr, node *parent)
{
    DBUG_ENTER ("SerializeStringAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSharedStringAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSharedStringAttrib (char *vname, info *info, char *attr, node *parent)
{
    DBUG_ENTER ("SerializeSharedStringAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerAttrib (char *vname, info *info, int attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLongAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLongAttrib (char *vname, info *info, long attr, node *parent)
{
    DBUG_ENTER ("SerializeLongAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeBoolAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeBoolAttrib (char *vname, info *info, bool attr, node *parent)
{
    DBUG_ENTER ("SerializeBoolAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeFloatAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeFloatAttrib (char *vname, info *info, float attr, node *parent)
{
    DBUG_ENTER ("SerializeFloatAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDoubleAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDoubleAttrib (char *vname, info *info, double attr, node *parent)
{
    DBUG_ENTER ("SerializeDoubleAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeCharAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeCharAttrib (char *vname, info *info, char attr, node *parent)
{
    DBUG_ENTER ("SerializeCharAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeOldTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeOldTypeAttrib (char *vname, info *info, types *attr, node *parent)
{
    DBUG_ENTER ("SerializeOldTypeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodeAttrib (char *vname, info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeNodeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLinkAttrib (char *vname, info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeLinkAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeApLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeApLinkAttrib (char *vname, info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeApLinkAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerArrayAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerArrayAttrib (char *vname, info *info, int *attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerArrayAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNumsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNumsAttrib (char *vname, info *info, nums *attr, node *parent)
{
    DBUG_ENTER ("SerializeNumsAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeFileTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeFileTypeAttrib (char *vname, info *info, file_type attr, node *parent)
{
    DBUG_ENTER ("SerializeFileTypeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLUTAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLUTAttrib (char *vname, info *info, LUT_t attr, node *parent)
{
    DBUG_ENTER ("SerializeLUTAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeWithOpTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeWithOpTypeAttrib (char *vname, info *info, WithOpType attr, node *parent)
{
    DBUG_ENTER ("SerializeWithOpTypeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializePrfAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializePrfAttrib (char *vname, info *info, prf attr, node *parent)
{
    DBUG_ENTER ("SerializePrfAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param pos    position within the current array
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeMaskAttrib (char *vname, info *info, int pos, long *attr, node *parent)
{
    DBUG_ENTER ("SerializeMaskAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDepsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDepsAttrib (char *vname, info *info, deps *attr, node *parent)
{
    DBUG_ENTER ("SerializeDepsAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIdsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIdsAttrib (char *vname, info *info, ids *attr, node *parent)
{
    DBUG_ENTER ("SerializeIdsAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeStatusTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeStatusTypeAttrib (char *vname, info *info, statustype attr, node *parent)
{
    DBUG_ENTER ("SerializeStatusTypeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodeListAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodeListAttrib (char *vname, info *info, nodelist *attr, node *parent)
{
    DBUG_ENTER ("SerializeNodeListAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSharedNodeListAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSharedNodeListAttrib (char *vname, info *info, nodelist *attr, node *parent)
{
    DBUG_ENTER ("SerializeSharedNodeListAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializePragmaLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializePragmaLinkAttrib (char *vname, info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializePragmaLinkAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMMaskAttrib (char *vname, info *info, DFMmask_t attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMMaskAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMMaskBaseAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMMaskBaseAttrib (char *vname, info *info, DFMmask_base_t attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMMaskBaseAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMFoldMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMFoldMaskAttrib (char *vname, info *info, DFMfoldmask_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMFoldMaskAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNewTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNewTypeAttrib (char *vname, info *info, ntype *attr, node *parent)
{
    DBUG_ENTER ("SerializeNewTypeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeArgTabAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeArgTabAttrib (char *vname, info *info, argtab_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeArgTabAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIndexPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIndexPointerAttrib (char *vname, info *info, index_info *attr, node *parent)
{
    DBUG_ENTER ("SerializeIndexPointerAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeShapeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeShapeAttrib (char *vname, info *info, shape *attr, node *parent)
{
    DBUG_ENTER ("SerializeShapeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSimpleTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSimpleTypeAttrib (char *vname, info *info, simpletype attr, node *parent)
{
    DBUG_ENTER ("SerializeSimpleTypeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeConstVecPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeConstVecPointerAttrib (char *vname, info *info, void *attr, node *parent)
{
    DBUG_ENTER ("SerializeConstVecPointerAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeUseFlagAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeUseFlagAttrib (char *vname, info *info, useflag attr, node *parent)
{
    DBUG_ENTER ("SerializeUseFlagAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeAccessInfoAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeAccessInfoAttrib (char *vname, info *info, access_info_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeAccessInfoAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeShpSegAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeShpSegAttrib (char *vname, info *info, shpseg *attr, node *parent)
{
    DBUG_ENTER ("SerializeShpSegAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerPointerAttrib (char *vname, info *info, int *attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerPointerAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerPointerArrayAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param pos    position within the current array
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerPointerArrayAttrib (char *vname, info *info, int pos, int *attr,
                                    node *parent)
{
    DBUG_ENTER ("SerializeIntegerPointerArrayAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSchedulingAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSchedulingAttrib (char *vname, info *info, SCHsched_t attr, node *parent)
{
    DBUG_ENTER ("SerializeSchedulingAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeTaskSelAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeTaskSelAttrib (char *vname, info *info, SCHtasksel_t attr, node *parent)
{
    DBUG_ENTER ("SerializeTaskSelAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodePointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodePointerAttrib (char *vname, info *info, node **attr, node *parent)
{
    DBUG_ENTER ("SerializeNodePointerAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSSAPhiAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSSAPhiAttrib (char *vname, info *info, ssaphit_t attr, node *parent)
{
    DBUG_ENTER ("SerializeSSAPhiAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeBitFieldAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeBitFieldAttrib (char *vname, info *info, int attr, node *parent)
{
    DBUG_ENTER ("SerializeBitFieldAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeConstantAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeConstantAttrib (char *vname, info *info, constant *attr, node *parent)
{
    DBUG_ENTER ("SerializeConstantAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeMTExecModeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeMTExecModeAttrib (char *vname, info *info, mtexecmode_t attr, node *parent)
{
    DBUG_ENTER ("SerializeMTExecModeAttrib");
    DBUG_VOID_RETURN;
}

/**
 * @}
 */
